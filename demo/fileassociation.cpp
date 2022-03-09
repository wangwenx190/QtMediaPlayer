/*
 * MIT License
 *
 * Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "fileassociation.h"
#include <dummyplayer.h>
#ifdef Q_OS_WINDOWS
#  if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
#    include <QtCore/qoperatingsystemversion.h>
#  else
#    include <QtCore/qsysinfo.h>
#  endif
#  include <QtCore/qdir.h>
#  include <QtCore/qfileinfo.h>
#  include <QtCore/qsettings.h>
#  include <QtGui/qguiapplication.h>
#  include <QtCore/private/qwinregistry_p.h>
#  include <QtCore/qt_windows.h>
#  include <atlbase.h>
#  include <shellapi.h>
#  include <shlobj_core.h>
#  include <shobjidl.h>
#endif

#ifdef Q_OS_WINDOWS
static const QString kGroupPolicyKeyPath = QStringLiteral(R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System)");
static const QString kEnableLUAKeyName = QStringLiteral("EnableLUA");

static const QString kDefault = QStringLiteral("Default");

// Only extensionName can't be empty. CLSID is used when you want to register MIME types.
struct FileAssocId
{
    QString extensionName = {};
    QString description = {};
    QString appUserModelID = {};
    QString friendlyTypeName = {};
    QString mimeType = {};
    QString clsid = {};
    QString defaultIcon = {};
    QString operation = {};
    QString command = {};
};
using FileAssocIdList = QList<FileAssocId>;

class COMCleaner
{
    Q_DISABLE_COPY_MOVE(COMCleaner)

public:
    explicit COMCleaner() {
        hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    }

    ~COMCleaner() {
        if (isInitialized()) {
            CoUninitialize();
        }
    }

    [[nodiscard]] inline bool isInitialized() const {
        return SUCCEEDED(hr);
    }

private:
    HRESULT hr = E_FAIL;
};

[[nodiscard]] static inline bool IsWin10OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10);
#else
    static const bool result = (QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS10);
#endif
    return result;
}

[[nodiscard]] static inline QString QuoteFilePath(const QString &path)
{
    if (path.isEmpty()) {
        return {};
    }
    QString result = QDir::toNativeSeparators(path);
    if (!result.startsWith(u'"', Qt::CaseInsensitive)) {
        result.prepend(u'"');
    }
    if (!result.endsWith(u'"', Qt::CaseInsensitive)) {
        result.append(u'"');
    }
    return result;
}

[[nodiscard]] static inline bool IsCurrentProcessElevated()
{
    // The token of the current process can't change during runtime,
    // so it should be safe to cache the result.
    static const bool result = []() -> bool {
        SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
        PSID AdministratorsGroup = nullptr;
        AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                 DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup);
        BOOL isMember = FALSE;
        CheckTokenMembership(nullptr, AdministratorsGroup, &isMember);
        FreeSid(AdministratorsGroup);
        return (isMember != FALSE);
    }();
    return result;
}

[[nodiscard]] static inline bool RunExecutableAsElevated
    (const QString &path, const QStringList &params)
{
    // This function uses UAC to ask for admin privileges. If the
    // user is no administrator yet and the computer's policies are set to not
    // use UAC (which is the case in some corporate networks), the call to
    // this function will simply succeed and not at all launch the child process. To
    // avoid this, we detect this situation here and return early.
    if (!IsCurrentProcessElevated()) {
        QWinRegistryKey reg(HKEY_LOCAL_MACHINE, kGroupPolicyKeyPath);
        if (!reg.isValid()) {
            return false;
        }
        const auto ret = reg.dwordValue(kEnableLUAKeyName);
        reg.close();
        if (!ret.second) {
            return false;
        }
        if (ret.first == 0) {
            return false;
        }
    }
    Q_ASSERT(!path.isEmpty());
    if (path.isEmpty()) {
        qWarning() << "Can't execute an evaluated process due to the file path is empty.";
        return false;
    }
    const QFileInfo fi(path);
    if (!fi.exists() || !fi.isExecutable()) {
        qWarning() << "Can't execute an evaluated process due to the file doesn't exist or it's not an executable.";
        return false;
    }
    const COMCleaner comCleaner{};
    if (!comCleaner.isInitialized()) {
        return false;
    }
    SHELLEXECUTEINFOW sei;
    SecureZeroMemory(&sei, sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOASYNC; // Wait for the execute operation to complete before returning.
    sei.lpVerb = L"runas"; // Launches an application as Administrator.
    sei.lpFile = qUtf16Printable(QDir::toNativeSeparators(fi.canonicalFilePath()));
    // The default working directory will be the calling process's directory,
    // but we don't want this behavior. Fix it by setting the working directory
    // to the executable's directory explicitly.
    sei.lpDirectory = qUtf16Printable(QDir::toNativeSeparators(fi.canonicalPath()));
    if (!params.isEmpty()) {
        // The parameters must be separated by a single whitespace.
        sei.lpParameters = qUtf16Printable(params.join(u' '));
    }
    return (ShellExecuteExW(&sei) != FALSE);
}

[[nodiscard]] static inline bool RegisterFileTypesImpl
    (const QString &applicationPath, const QString &applicationDisplayName,
     const QString &applicationDescription, const QString &applicationVersion,
     const QString &companyName, const FileAssocIdList &assocIdList,
     const QString &defaultIconPath)
{
    if (applicationPath.isEmpty() || applicationDescription.isEmpty() || companyName.isEmpty()) {
        qWarning() << "Application path, application description and company name can't be empty.";
        return false;
    }
    if (!IsCurrentProcessElevated()) {
        qWarning() << "Can't register file types without administrator privileges.";
        return false;
    }
    if (assocIdList.isEmpty()) {
        qWarning() << "Empty file type list.";
        return false;
    }
    const QFileInfo exeFileInfo(applicationPath);
    if (!exeFileInfo.exists()) {
        qWarning() << "The given application doesn't exist.";
        return false;
    }
    if (!exeFileInfo.isExecutable()) {
        qWarning() << "The given path is not an executable.";
        return false;
    }
    const QString exePath = QuoteFilePath(exeFileInfo.canonicalFilePath());
    const QString displayName = applicationDisplayName.isEmpty() ? exeFileInfo.completeBaseName() : applicationDisplayName;
    const QString version = applicationVersion.isEmpty() ? QStringLiteral("1.0.0.0") : applicationVersion;
    QString progid_com = companyName;
    progid_com.remove(u' ');
    QString progid_app = displayName;
    progid_app.remove(u' ');
    QString progid_ver = version;
    progid_ver.remove(u'.');
    const QString capabilityKeyPath = QStringLiteral(R"(HKEY_LOCAL_MACHINE\SOFTWARE\%1\%2\Capabilities)")
                                          .arg(progid_com, progid_app);
    static const QString hklmClassesKeyPath = QStringLiteral(R"(HKEY_LOCAL_MACHINE\SOFTWARE\Classes)");
    const QString exeIconPath = QStringLiteral("%1,0").arg(exePath);
    const QString exeOpenCmd = exePath + QStringLiteral(R"( "%1")");
    for (auto&& assocId : qAsConst(assocIdList)) {
        if (assocId.extensionName.isEmpty()) {
            qDebug() << "Empty extension name. Skipping.";
            continue;
        }
        QString ext = assocId.extensionName.toLower();
        if (ext.startsWith(u'*')) {
            ext.remove(0, 1);
        }
        QString progid_ext = ext.toUpper();
        if (ext.startsWith(u'.')) {
            progid_ext.remove(0, 1);
        } else {
            ext.prepend(u'.');
        }
        const QString ProgID = QStringLiteral("%1.%2.%3.%4").arg(progid_com, progid_app, progid_ext, progid_ver);
        const QString regKey1 = QStringLiteral(R"(%1\%2)").arg(hklmClassesKeyPath, ProgID);
        QSettings settings1(regKey1, QSettings::NativeFormat);
        if (!assocId.description.isEmpty()) {
            settings1.setValue(kDefault, assocId.description);
        }
        if (!assocId.appUserModelID.isEmpty()) {
            settings1.setValue(QStringLiteral("AppUserModelID"), assocId.appUserModelID);
        }
        if (!assocId.friendlyTypeName.isEmpty()) {
            settings1.setValue(QStringLiteral("FriendlyTypeName"), assocId.friendlyTypeName);
        }
        if (!assocId.clsid.isEmpty()) {
            QSettings settings1_clsid(QStringLiteral(R"(%1\CLSID)").arg(regKey1), QSettings::NativeFormat);
            settings1_clsid.setValue(kDefault, assocId.clsid.toUpper());
        }
        if (!assocId.defaultIcon.isEmpty()) {
            const QFileInfo iconFileInfo(assocId.defaultIcon);
            const QString iconPath = QuoteFilePath(iconFileInfo.canonicalFilePath());
            QSettings settings1_defaultIcon(QStringLiteral(R"(%1\DefaultIcon)").arg(regKey1), QSettings::NativeFormat);
            settings1_defaultIcon.setValue(kDefault, iconPath);
        }
        if (!assocId.operation.isEmpty()) {
            const QString operationKeyPath = QStringLiteral(R"(%1\Shell\%2)").arg(regKey1, assocId.operation);
            QSettings settings1_icon(operationKeyPath, QSettings::NativeFormat);
            settings1_icon.setValue(QStringLiteral("Icon"), exeIconPath);
            QSettings settings1_command(QStringLiteral(R"(%1\Command)").arg(operationKeyPath), QSettings::NativeFormat);
            const QString cmd = assocId.command.isEmpty() ? exeOpenCmd : assocId.command;
            settings1_command.setValue(kDefault, cmd);
        }
        const QString regKey2 = QStringLiteral(R"(%1\%2\OpenWithProgIDs)").arg(hklmClassesKeyPath, ext);
        QSettings settings2(regKey2, QSettings::NativeFormat);
        settings2.setValue(ProgID, QStringLiteral("")); // Empty string.
        QSettings settings3_fileAssociations(QStringLiteral(R"(%1\FileAssociations)").arg(capabilityKeyPath),
                                             QSettings::NativeFormat);
        settings3_fileAssociations.setValue(ext, ProgID);
        if (!assocId.mimeType.isEmpty()) {
            QSettings settings3_mimeAssociations(QStringLiteral(R"(%1\MimeAssociations)").arg(capabilityKeyPath),
                                                 QSettings::NativeFormat);
            settings3_mimeAssociations.setValue(assocId.mimeType.toLower(), ProgID);
        }
    }
    QSettings settings_capability(capabilityKeyPath, QSettings::NativeFormat);
    // ApplicationDescription MUST NOT BE EMPTY!!!
    settings_capability.setValue(QStringLiteral("ApplicationDescription"), applicationDescription);
    settings_capability.setValue(QStringLiteral("ApplicationName"), displayName);
    static const QString registeredAppsKeyPath = QStringLiteral(R"(HKEY_LOCAL_MACHINE\SOFTWARE\RegisteredApplications)");
    QSettings settings_registeredApps(registeredAppsKeyPath, QSettings::NativeFormat);
    settings_registeredApps.setValue(displayName, QStringLiteral(R"(SOFTWARE\%1\%2\Capabilities)")
                                                      .arg(progid_com, progid_app));
    const QString appsKeyPath = QStringLiteral(R"(%1\Applications\%2)").arg(hklmClassesKeyPath, exeFileInfo.fileName());
    QSettings settings_apps(appsKeyPath, QSettings::NativeFormat);
    QSettings settings_apps_defaultIcon(QStringLiteral(R"(%1\DefaultIcon)").arg(appsKeyPath), QSettings::NativeFormat);
    settings_apps_defaultIcon.setValue(kDefault, defaultIconPath.isEmpty() ? exeIconPath : defaultIconPath);
    QSettings settings_apps_open(QStringLiteral(R"(%1\Shell\Open)"), QSettings::NativeFormat);
    settings_apps_open.setValue(QStringLiteral("Icon"), exeIconPath);
    QSettings settings_apps_command(QStringLiteral(R"(%1\Shell\Open\Command)").arg(appsKeyPath), QSettings::NativeFormat);
    settings_apps_command.setValue(kDefault, exeOpenCmd);
    const COMCleaner comCleaner{};
    if (!comCleaner.isInitialized()) {
        return false;
    }
    // Tell the system to flush itself immediately.
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSH, nullptr, nullptr);
    if (IsWin10OrGreater()) {
        // Microsoft Docs: The return value is cast as an HINSTANCE for backward compatibility
        // with 16-bit Windows applications. It is not a true HINSTANCE, however. It can be cast
        // only to an INT_PTR and compared to 32. If the function succeeds, it returns a value
        // greater than 32.
        const HINSTANCE ret = ShellExecuteW(nullptr, nullptr, L"ms-settings:defaultapps", nullptr, nullptr, SW_SHOW);
        if (reinterpret_cast<INT_PTR>(ret) <= 32) {
            return false;
        }
    } else {
        CComPtr<IApplicationAssociationRegistration> pAAR = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration, nullptr, CLSCTX_INPROC, IID_PPV_ARGS(&pAAR));
        if (SUCCEEDED(hr)) {
            hr = pAAR->SetAppAsDefaultAll(qUtf16Printable(displayName));
            if (SUCCEEDED(hr)) {
                return true;
            }
        }
        // Let the user choose the default application.
        CComPtr<IApplicationAssociationRegistrationUI> pAARUI = nullptr;
        hr = CoCreateInstance(CLSID_ApplicationAssociationRegistrationUI, nullptr, CLSCTX_INPROC, IID_PPV_ARGS(&pAARUI));
        if (FAILED(hr)) {
            return false;
        }
        hr = pAARUI->LaunchAdvancedAssociationUI(qUtf16Printable(displayName));
        if (FAILED(hr)) {
            return false;
        }
    }
    return true;
}
#endif

[[nodiscard]] static inline QString applicationIconsDirPath()
{
    static const QString result = QCoreApplication::applicationDirPath() + QStringLiteral("/icons");
    return result;
}

[[nodiscard]] static inline QString applicationDescriptionString()
{
    static const QString result = QStringLiteral("This simple demo application shows how to integrate QtMediaPlayer into Qt Quick applications.");
    return result;
}

QStringList FileAssociation::supportedFileTypes()
{
    static const QStringList result = []() -> QStringList {
        QStringList list = QTMEDIAPLAYER_PREPEND_NAMESPACE(DummyPlayer)::videoFileSuffixes();
        list << QTMEDIAPLAYER_PREPEND_NAMESPACE(DummyPlayer)::audioFileSuffixes();
        return list;
    }();
    return result;
}

#ifdef Q_OS_WINDOWS
bool FileAssociation::registerApplicationAsDefaultHandler()
{
    if (!IsCurrentProcessElevated()) {
        const bool result = RunExecutableAsElevated(QCoreApplication::applicationFilePath(), {QStringLiteral("--register")});
        Q_UNUSED(result);
        return false;
    }
    const QStringList fileTypes = supportedFileTypes();
    FileAssocIdList ids = {};
    for (auto&& extName : qAsConst(fileTypes)) {
        FileAssocId id = {};
        id.extensionName = extName;
        id.operation = QStringLiteral("Open");
        id.defaultIcon = QStringLiteral(R"(%1\%2.ico)").arg(applicationIconsDirPath(), extName);
        //id.description = ...;
        //id.appUserModelID = ...;
        ids.append(id);
    }
    if (ids.isEmpty()) {
        return false;
    }
    return RegisterFileTypesImpl(
        QDir::toNativeSeparators(QCoreApplication::applicationFilePath()),
        QGuiApplication::applicationDisplayName(), applicationDescriptionString(),
        QCoreApplication::applicationVersion(), QCoreApplication::organizationName(), ids,
        QStringLiteral(R"(%1\default.ico)").arg(applicationIconsDirPath()));
}

bool FileAssociation::unregisterApplicationAsDefaultHandler()
{
    return true;
}

bool FileAssociation::isApplicationRegisteredAsDefaultHandler()
{
    const COMCleaner comCleaner{};
    if (!comCleaner.isInitialized()) {
        return false;
    }
    CComPtr<IApplicationAssociationRegistration> pAAR = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration, nullptr, CLSCTX_INPROC, IID_PPV_ARGS(&pAAR));
    if (FAILED(hr)) {
        return false;
    }
    BOOL bDefault = FALSE;
    hr = pAAR->QueryAppIsDefaultAll(AL_EFFECTIVE, qUtf16Printable(QGuiApplication::applicationDisplayName()), &bDefault);
    if (FAILED(hr)) {
        return false;
    }
    return (bDefault != FALSE);
}
#else
bool FileAssociation::registerApplicationAsDefaultHandler()
{
    return false;
}

bool FileAssociation::unregisterApplicationAsDefaultHandler()
{
    return false;
}

bool FileAssociation::isApplicationRegisteredAsDefaultHandler()
{
    return false;
}
#endif
