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

#include "i18n.h"
#include <QtCore/qmutex.h>
#include <QtCore/qhash.h>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qtranslator.h>
#include <QtCore/qlocale.h>
#include <QtQml/qqmlengine.h>

struct I18NHelper
{
    QMutex mutex = {};

    QStringList availableTranslations = {};
    QHash<QString, QString> qmFilePaths = {};
    QTranslator *translator = nullptr;
    QQmlEngine *engine = nullptr;

    explicit I18NHelper() = default;
    ~I18NHelper() = default;

private:
    Q_DISABLE_COPY_MOVE(I18NHelper)
};

Q_GLOBAL_STATIC(I18NHelper, g_i18nHelper)

I18N::I18N(QObject *parent) : QObject(parent)
{
    reload();
}

I18N::~I18N()
{
    removeCurrentTranslationIfAny();
}

void I18N::setEngine(QQmlEngine *value)
{
    Q_ASSERT(value);
    if (!value) {
        return;
    }
    QMutexLocker locker(&g_i18nHelper()->mutex);
    if (g_i18nHelper()->engine) {
        return;
    }
    g_i18nHelper()->engine = value;
    connect(g_i18nHelper()->engine, &QQmlEngine::destroyed, this, [this](){
        // This is a singleton QML type, it will live longer than the QML engine,
        // make sure we won't interact with the engine after it has been destroyed.
        disconnect(this, &I18N::translationChanged, g_i18nHelper()->engine, &QQmlEngine::retranslate);
        g_i18nHelper()->engine = nullptr;
    });
    // Notify the QML engine to refresh the translation strings, this additional step
    // is necessary for Qt Quick applications.
    connect(this, &I18N::translationChanged, g_i18nHelper()->engine, &QQmlEngine::retranslate);
}

QStringList I18N::translations() const
{
    QMutexLocker locker(&g_i18nHelper()->mutex);
    return g_i18nHelper()->availableTranslations;
}

QString I18N::translation() const
{
    return m_currentTranslation;
}

void I18N::setTranslation(const QString &value)
{
    Q_ASSERT(!value.isEmpty());
    if (value.isEmpty()) {
        return;
    }
    if (m_currentTranslation == value) {
        return;
    }
    g_i18nHelper()->mutex.lock();
    Q_ASSERT(g_i18nHelper()->engine);
    if (!g_i18nHelper()->engine) {
        g_i18nHelper()->mutex.unlock();
        return;
    }
    if (!g_i18nHelper()->availableTranslations.contains(value)) {
        g_i18nHelper()->mutex.unlock();
        return;
    }
    if (!g_i18nHelper()->qmFilePaths.contains(value)) {
        g_i18nHelper()->mutex.unlock();
        return;
    }
    g_i18nHelper()->mutex.unlock();
    removeCurrentTranslationIfAny();
    QMutexLocker locker(&g_i18nHelper()->mutex);
    auto newTranslator = new QTranslator;
    if (newTranslator->load(g_i18nHelper()->qmFilePaths.value(value))) {
        if (QCoreApplication::installTranslator(newTranslator)) {
            g_i18nHelper()->translator = newTranslator;
            m_currentTranslation = value;
            Q_EMIT translationChanged();
        }
    }
    if (!g_i18nHelper()->translator) {
        delete newTranslator;
        newTranslator = nullptr;
    }
}

void I18N::reload()
{
    static const QString dirPath = QCoreApplication::applicationDirPath() + QStringLiteral("/translations");
    const QDir dir(dirPath);
    if (!dir.exists()) {
        return;
    }
    const QFileInfoList fileInfoList = dir.entryInfoList({QStringLiteral("*.qm")},
                              QDir::Files | QDir::NoDotAndDotDot | QDir::Readable, QDir::Name);
    if (fileInfoList.isEmpty()) {
        return;
    }
    QMutexLocker locker(&g_i18nHelper()->mutex);
    if (!g_i18nHelper()->qmFilePaths.isEmpty()) {
        g_i18nHelper()->qmFilePaths.clear();
    }
    if (!g_i18nHelper()->availableTranslations.isEmpty()) {
        g_i18nHelper()->availableTranslations.clear();
    }
    for (auto &&fileInfo : qAsConst(fileInfoList)) {
        const QString languageCode = [&fileInfo]() -> QString {
            QString str = fileInfo.fileName();
            static const QString prefix = QStringLiteral("qtmediaplayerdemo_");
            if (str.startsWith(prefix, Qt::CaseInsensitive)) {
                str.remove(0, prefix.length());
            }
            str.replace(u'-', u'_');
            return str;
        }();
        const QString displayName = QLocale(languageCode).nativeLanguageName();
        g_i18nHelper()->qmFilePaths.insert(displayName, fileInfo.canonicalFilePath());
        g_i18nHelper()->availableTranslations.append(displayName);
    }
}

void I18N::removeCurrentTranslationIfAny()
{
    QMutexLocker locker(&g_i18nHelper()->mutex);
    if (g_i18nHelper()->translator) {
        QCoreApplication::removeTranslator(g_i18nHelper()->translator);
        delete g_i18nHelper()->translator;
        g_i18nHelper()->translator = nullptr;
    }
}
