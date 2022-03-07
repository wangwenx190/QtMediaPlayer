/****************************************************************************
 **
 ** Copyright (C) 2022 Ivan Vizir <define-true-false@yandex.com>
 ** Copyright (C) 2022 The Qt Company Ltd.
 ** Contact: https://www.qt.io/licensing/
 **
 ** This file is part of the QtWinExtras module of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL$
 ** Commercial License Usage
 ** Licensees holding valid commercial Qt licenses may use this file in
 ** accordance with the commercial license agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and The Qt Company. For licensing terms
 ** and conditions see https://www.qt.io/terms-conditions. For further
 ** information use the contact form at https://www.qt.io/contact-us.
 **
 ** GNU Lesser General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU Lesser
 ** General Public License version 3 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.LGPL3 included in the
 ** packaging of this file. Please review the following information to
 ** ensure the GNU Lesser General Public License version 3 requirements
 ** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
 **
 ** GNU General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU
 ** General Public License version 2.0 or (at your option) the GNU General
 ** Public license version 3 or any later version approved by the KDE Free
 ** Qt Foundation. The licenses are as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file. Please review the following
 ** information to ensure the GNU General Public License requirements will
 ** be met: https://www.gnu.org/licenses/gpl-2.0.html and
 ** https://www.gnu.org/licenses/gpl-3.0.html.
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#include "qwintaskbarbutton.h"
#include "qwintaskbarprogress.h"
#include <QtCore/qpointer.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qmutex.h>
#include <QtCore/qabstractnativeeventfilter.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qwindow.h>
#include <QtGui/qimage.h>
#include <QtCore/qt_windows.h>
#include <atlbase.h>
#include <shobjidl.h>

QT_BEGIN_NAMESPACE

/*!
    \class QWinTaskbarButton
    \inmodule QtWinExtras
    \brief The QWinTaskbarButton class represents the Windows taskbar button for
    a top-level window (Windows 7 and newer).

    \since 5.2

    The QWinTaskbarButton class enables you to set overlay icons on a taskbar
    button, and provides access to its progress indicator.

    An overlay icon indicates change in the state of an application, whereas
    a progress indicator shows how time-consuming tasks are progressing.

    \image taskbar-button.png Taskbar Button

    The following example code illustrates how to use the QWinTaskbarButton
    and QWinTaskbarProgress classes to adjust the look of the taskbar button:

    \snippet code/taskbar.cpp taskbar_cpp

    \note QWidget::windowHandle() returns a valid instance of a QWindow only
    after the widget has been shown. It is therefore recommended to delay the
    initialization of the QWinTaskbarButton instances until QWidget::showEvent().

    \note The class wraps API only available since Windows 7. Instantiating it
    on Windows XP or Windows Vista causes a runtime warning.

    \sa QWinTaskbarProgress
 */

static inline void qt_qstringToNullTerminated(const QString &src, wchar_t *dst)
{
    Q_ASSERT(!src.isEmpty());
    Q_ASSERT(dst);
    if (src.isEmpty() || !dst) {
        return;
    }
    dst[src.toWCharArray(dst)] = L'\0';
}

[[nodiscard]] static inline wchar_t *qt_qstringToNullTerminated(const QString &src)
{
    Q_ASSERT(!src.isEmpty());
    if (src.isEmpty()) {
        return nullptr;
    }
    const auto buffer = new wchar_t[src.length() + 1];
    qt_qstringToNullTerminated(src, buffer);
    return buffer;
}

[[nodiscard]] static inline TBPFLAG nativeProgressState(const QWinTaskbarProgress * const progress)
{
    if (!progress || !progress->isVisible())
        return TBPF_NOPROGRESS;
    if (progress->isStopped())
        return TBPF_ERROR;
    if (progress->isPaused())
        return TBPF_PAUSED;
    if (progress->minimum() == 0 && progress->maximum() == 0)
        return TBPF_INDETERMINATE;
    return TBPF_NORMAL;
}

[[nodiscard]] static inline QWindow *findWindow(const HWND handle)
{
    Q_ASSERT(handle);
    if (!handle) {
        return nullptr;
    }
    const auto wid = reinterpret_cast<WId>(handle);
    const QWindowList topLevels = QGuiApplication::topLevelWindows();
    for (auto &&topLevel : qAsConst(topLevels)) {
        if (topLevel->handle() && (topLevel->winId() == wid)) {
            return topLevel;
        }
    }
    return nullptr;
}

class QWinEvent : public QEvent
{
    Q_DISABLE_COPY_MOVE(QWinEvent)

public:
    static inline const int TaskbarButtonCreated = QEvent::registerEventType();

    explicit QWinEvent(const int type) : QEvent(static_cast<QEvent::Type>(type)) {}
    ~QWinEvent() override = default;
};

class QWinEventFilter : public QAbstractNativeEventFilter
{
    Q_DISABLE_COPY_MOVE(QWinEventFilter)

public:
    explicit QWinEventFilter() = default;
    ~QWinEventFilter() override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    [[nodiscard]] bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    [[nodiscard]] bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
#endif

private:
    const UINT tbButtonCreatedMsgId = RegisterWindowMessageW(L"TaskbarButtonCreated");
};

QWinEventFilter::~QWinEventFilter()
{
    qApp->removeNativeEventFilter(this);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool QWinEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
#else
bool QWinEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
#endif
{
    if ((eventType != QByteArrayLiteral("windows_generic_MSG")) || !message || !result) {
        return false;
    }
    const auto msg = static_cast<LPMSG>(message);
    if (!msg->hwnd) {
        return false;
    }
    if (msg->message == tbButtonCreatedMsgId) {
        if (QWindow * const window = findWindow(msg->hwnd)) {
            QWinEvent event(QWinEvent::TaskbarButtonCreated);
            QCoreApplication::sendEvent(window, &event);
            *result = 0;
            return true;
        }
    }
    return false;
}

struct Win32EventFilterHelper
{
    QMutex mutex = {};
    QScopedPointer<QWinEventFilter> instance;

    explicit Win32EventFilterHelper() = default;
    ~Win32EventFilterHelper() = default;

private:
    Q_DISABLE_COPY_MOVE(Win32EventFilterHelper)
};

Q_GLOBAL_STATIC(Win32EventFilterHelper, g_globalFilter)

class QWinTaskbarButtonPrivate
{
    Q_DECLARE_PUBLIC(QWinTaskbarButton)
    Q_DISABLE_COPY_MOVE(QWinTaskbarButtonPrivate)

public:
    explicit QWinTaskbarButtonPrivate(QWinTaskbarButton *q);
    ~QWinTaskbarButtonPrivate();

private:
    [[nodiscard]] HWND handle() const;
    [[nodiscard]] int iconSize() const;
    void updateOverlayIcon();
    void _q_updateProgress();

private:
    QWinTaskbarButton *q_ptr = nullptr;

    QPointer<QWinTaskbarProgress> progressBar = nullptr;
    QIcon overlayIcon = {};
    QString overlayAccessibleDescription = {};

    bool comInited = false;
    CComPtr<ITaskbarList4> pTbList = nullptr;
    QWindow *window = nullptr;
};

QWinTaskbarButtonPrivate::QWinTaskbarButtonPrivate(QWinTaskbarButton *q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    if (SUCCEEDED(CoInitializeEx(nullptr,
                  COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))) {
        comInited = true;
        if (SUCCEEDED(CoCreateInstance(CLSID_TaskbarList, nullptr,
                            CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pTbList)))) {
            pTbList->HrInit();
        }
    }
}

QWinTaskbarButtonPrivate::~QWinTaskbarButtonPrivate()
{
    if (comInited) {
        CoUninitialize();
    }
}

HWND QWinTaskbarButtonPrivate::handle() const
{
    return reinterpret_cast<HWND>(window->winId());
}

int QWinTaskbarButtonPrivate::iconSize() const
{
    return GetSystemMetrics(SM_CXSMICON);
}

void QWinTaskbarButtonPrivate::updateOverlayIcon()
{
    if (!pTbList || !window)
        return;

    wchar_t *descrPtr = nullptr;
    HICON hicon = nullptr;
    if (!overlayAccessibleDescription.isEmpty())
        descrPtr = qt_qstringToNullTerminated(overlayAccessibleDescription);
    if (!overlayIcon.isNull())
        hicon = overlayIcon.pixmap(iconSize()).toImage().toHICON();

    if (hicon)
        pTbList->SetOverlayIcon(handle(), hicon, descrPtr);
    else if (!hicon && !overlayIcon.isNull())
        pTbList->SetOverlayIcon(handle(),
                                static_cast<HICON>(LoadImageW(nullptr, IDI_APPLICATION,
                                 IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, LR_SHARED)), descrPtr);
    else
        pTbList->SetOverlayIcon(handle(), nullptr, descrPtr);

    if (hicon)
        DestroyIcon(hicon);
    delete [] descrPtr;
}

void QWinTaskbarButtonPrivate::_q_updateProgress()
{
    if (!pTbList || !window)
        return;

    if (progressBar) {
        const int min = progressBar->minimum();
        const int max = progressBar->maximum();
        const int range = max - min;
        if (range > 0) {
            const int value = qRound(qreal(100) * qreal(progressBar->value() - min) / qreal(range));
            pTbList->SetProgressValue(handle(), ULONGLONG(value), 100);
        }
    }
    pTbList->SetProgressState(handle(), nativeProgressState(progressBar));
}

/*!
    Constructs a QWinTaskbarButton with the specified \a parent.

    If \a parent is an instance of QWindow, it is automatically
    assigned as the taskbar button's \l window.
 */
QWinTaskbarButton::QWinTaskbarButton(QObject *parent) :
    QObject(parent), d_ptr(new QWinTaskbarButtonPrivate(this))
{
    g_globalFilter()->mutex.lock();
    if (g_globalFilter()->instance.isNull()) {
        g_globalFilter()->instance.reset(new QWinEventFilter);
        qApp->installNativeEventFilter(g_globalFilter()->instance.data());
    }
    g_globalFilter()->mutex.unlock();
    setWindow(qobject_cast<QWindow *>(parent));
}

/*!
    Destroys the QWinTaskbarButton.
 */
QWinTaskbarButton::~QWinTaskbarButton() = default;

/*!
    \property QWinTaskbarButton::window
    \brief the window whose taskbar button is manipulated
 */
void QWinTaskbarButton::setWindow(QWindow *window)
{
    Q_D(QWinTaskbarButton);
    if (d->window)
        d->window->removeEventFilter(this);
    d->window = window;
    if (d->window) {
        d->window->installEventFilter(this);
        if (d->window->isVisible()) {
            d->_q_updateProgress();
            d->updateOverlayIcon();
        }
    }
    Q_EMIT windowChanged();
}

QWindow *QWinTaskbarButton::window() const
{
    Q_D(const QWinTaskbarButton);
    return d->window;
}

/*!
    \property QWinTaskbarButton::overlayIcon
    \brief the overlay icon of the taskbar button
 */
QIcon QWinTaskbarButton::overlayIcon() const
{
    Q_D(const QWinTaskbarButton);
    return d->overlayIcon;
}

void QWinTaskbarButton::setOverlayIcon(const QIcon &icon)
{
    Q_D(QWinTaskbarButton);

    d->overlayIcon = icon;
    d->updateOverlayIcon();

    Q_EMIT overlayIconChanged();
}

void QWinTaskbarButton::clearOverlayIcon()
{
    setOverlayAccessibleDescription({});
    setOverlayIcon({});
}

/*!
    \property QWinTaskbarButton::overlayAccessibleDescription
    \brief the description of the overlay for accessibility purposes

    \sa overlayIcon
 */
QString QWinTaskbarButton::overlayAccessibleDescription() const
{
    Q_D(const QWinTaskbarButton);
    return d->overlayAccessibleDescription;
}

void QWinTaskbarButton::setOverlayAccessibleDescription(const QString &description)
{
    Q_D(QWinTaskbarButton);

    d->overlayAccessibleDescription = description;
    d->updateOverlayIcon();

    Q_EMIT overlayAccessibleDescriptionChanged();
}

/*!
    \property QWinTaskbarButton::progress
    \brief the progress indicator of the taskbar button

    \note The progress indicator is not \l{QWinTaskbarProgress::visible}{visible} by default.
 */
QWinTaskbarProgress *QWinTaskbarButton::progress() const
{
    Q_D(const QWinTaskbarButton);
    if (!d->progressBar) {
        const auto that = const_cast<QWinTaskbarButton *>(this);
        const auto pbar = new QWinTaskbarProgress(that);
        connect(pbar, &QWinTaskbarProgress::destroyed, this, [that](){
            that->d_func()->_q_updateProgress();
        });
        connect(pbar, &QWinTaskbarProgress::valueChanged, this, [that](){
            that->d_func()->_q_updateProgress();
        });
        connect(pbar, &QWinTaskbarProgress::minimumChanged, this, [that](){
            that->d_func()->_q_updateProgress();
        });
        connect(pbar, &QWinTaskbarProgress::maximumChanged, this, [that](){
            that->d_func()->_q_updateProgress();
        });
        connect(pbar, &QWinTaskbarProgress::visibilityChanged, this, [that](){
            that->d_func()->_q_updateProgress();
        });
        connect(pbar, &QWinTaskbarProgress::pausedChanged, this, [that](){
            that->d_func()->_q_updateProgress();
        });
        connect(pbar, &QWinTaskbarProgress::stoppedChanged, this, [that](){
            that->d_func()->_q_updateProgress();
        });
        that->d_func()->progressBar = pbar;
        that->d_func()->_q_updateProgress();
    }
    return d->progressBar;
}

/*!
    \internal
    Intercepts TaskbarButtonCreated messages.
 */
bool QWinTaskbarButton::eventFilter(QObject *object, QEvent *event)
{
    Q_D(QWinTaskbarButton);
    if ((object == d->window) && (event->type() == QWinEvent::TaskbarButtonCreated)) {
        d->_q_updateProgress();
        d->updateOverlayIcon();
    }
    return QObject::eventFilter(object, event);
}

QT_END_NAMESPACE
