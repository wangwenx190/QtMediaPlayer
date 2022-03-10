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

#include "qwinthumbnailtoolbar.h"
#include "qwinthumbnailtoolbutton.h"
#include "qwinevent_p.h"
#include <QtCore/qcoreapplication.h>
#include <QtCore/qtimer.h>
#include <QtCore/qabstractnativeeventfilter.h>
#include <QtGui/qwindow.h>
#include <QtCore/private/qsystemlibrary_p.h>
#include <QtCore/qt_windows.h>
#include <shobjidl.h>
#include <dwmapi.h>

#ifndef THBN_CLICKED
#  define THBN_CLICKED (0x1800)
#endif

#ifndef WM_DWMSENDICONICLIVEPREVIEWBITMAP
#  define WM_DWMSENDICONICLIVEPREVIEWBITMAP (0x0326)
#endif

#ifndef WM_DWMSENDICONICTHUMBNAIL
#  define WM_DWMSENDICONICTHUMBNAIL (0x0323)
#endif

QT_BEGIN_NAMESPACE

static constexpr const int windowsLimitedThumbbarSize = 7;

class QWinThumbnailToolBarPrivate : public QObject, QAbstractNativeEventFilter
{
    Q_DISABLE_COPY_MOVE(QWinThumbnailToolBarPrivate)
    Q_DECLARE_PUBLIC(QWinThumbnailToolBar)

public:

    class IconicPixmapCache
    {
        Q_DISABLE_COPY_MOVE(IconicPixmapCache)

    public:
        explicit IconicPixmapCache() = default;
        ~IconicPixmapCache() { deleteBitmap(); }

        [[nodiscard]] operator bool() const { return !m_pixmap.isNull(); }

        [[nodiscard]] QPixmap pixmap() const { return m_pixmap; }
        [[nodiscard]] bool setPixmap(const QPixmap &p);

        [[nodiscard]] HBITMAP bitmap(const QSize &maxSize);

    private:
        void deleteBitmap();

    private:
        QPixmap m_pixmap = {};
        QSize m_size = {};
        HBITMAP m_bitmap = nullptr;
    };

    explicit QWinThumbnailToolBarPrivate(QWinThumbnailToolBar *q);
    ~QWinThumbnailToolBarPrivate();

    void initToolbar();
    void clearToolbar();
    void _q_updateToolbar();
    void _q_scheduleUpdate();
    [[nodiscard]] bool eventFilter(QObject *, QEvent *) override;

    [[nodiscard]] bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

    static void initButtons(THUMBBUTTON *buttons);
    [[nodiscard]] static int makeNativeButtonFlags(const QWinThumbnailToolButton *button);
    [[nodiscard]] static int makeButtonMask(const QWinThumbnailToolButton *button);

    bool updateScheduled = false;
    QList<QWinThumbnailToolButton *> buttonList = {};
    QWindow *window = nullptr;
    ITaskbarList4 *pTbList = nullptr;

    IconicPixmapCache iconicThumbnail = IconicPixmapCache{};
    IconicPixmapCache iconicLivePreview = IconicPixmapCache{};

private:
    [[nodiscard]] bool hasHandle() const;
    [[nodiscard]] HWND handle() const;
    void updateIconicPixmapsEnabled(const bool invalidate);
    void updateIconicThumbnail(const MSG *message);
    void updateIconicLivePreview(const MSG *message);

    QWinThumbnailToolBar *q_ptr = nullptr;
    bool withinIconicThumbnailRequest = false;
    bool withinIconicLivePreviewRequest = false;
    bool comInited = false;
};

/*!
    \class QWinThumbnailToolBar
    \inmodule QtWinExtras
    \since 5.2
    \brief The QWinThumbnailToolBar class allows manipulating the thumbnail toolbar of a window.

    Applications can embed a toolbar in the thumbnail of a window, which is
    shown when hovering over its taskbar icon. A thumbnail toolbar may provide
    quick access to the commands of a window without requiring the user to restore
    or activate the window.

    \image thumbbar.png Media player thumbnail toolbar

    The following example code illustrates how to use the functions in the
    QWinThumbnailToolBar and QWinThumbnailToolButton class to implement a
    thumbnail toolbar:

    \snippet code/thumbbar.cpp thumbbar_cpp

    \sa QWinThumbnailToolButton
 */

/*!
    Constructs a QWinThumbnailToolBar with the specified \a parent.

    If \a parent is an instance of QWindow, it is automatically
    assigned as the thumbnail toolbar's \l window.
 */
QWinThumbnailToolBar::QWinThumbnailToolBar(QObject *parent) :
    QObject(parent), d_ptr(new QWinThumbnailToolBarPrivate(this))
{
    QWinEventFilter::setup();
    setWindow(qobject_cast<QWindow *>(parent));
}

/*!
    Destroys and clears the QWinThumbnailToolBar.
 */
QWinThumbnailToolBar::~QWinThumbnailToolBar() = default;

/*!
    \property QWinThumbnailToolBar::window
    \brief the window whose thumbnail toolbar is manipulated
 */
void QWinThumbnailToolBar::setWindow(QWindow *window)
{
    Q_D(QWinThumbnailToolBar);
    if (d->window != window) {
        if (d->window) {
            d->window->removeEventFilter(d);
            if (d->window->handle()) {
                d->clearToolbar();
                setIconicPixmapNotificationsEnabled(false);
            }
        }
        d->window = window;
        if (d->window) {
            d->window->installEventFilter(d);
            if (d->window->isVisible()) {
                d->initToolbar();
                d->_q_scheduleUpdate();
            }
        }
        Q_EMIT windowChanged();
    }
}

QWindow *QWinThumbnailToolBar::window() const
{
    Q_D(const QWinThumbnailToolBar);
    return d->window;
}

/*!
    Adds a \a button to the thumbnail toolbar.

    \note The number of buttons is limited to \c 7.
 */
void QWinThumbnailToolBar::addButton(QWinThumbnailToolButton *button)
{
    Q_D(QWinThumbnailToolBar);
    if (d->buttonList.size() >= windowsLimitedThumbbarSize) {
        qWarning() << "Cannot add " << button << " maximum number of buttons ("
                   << windowsLimitedThumbbarSize << ") reached.";
        return;
    }
    if (button && button->toolbar != this) {
        if (button->toolbar) {
            button->toolbar->removeButton(button);
        }
        button->toolbar = this;
        connect(button, &QWinThumbnailToolButton::changed,
                d, &QWinThumbnailToolBarPrivate::_q_scheduleUpdate);
        d->buttonList.append(button);
        d->_q_scheduleUpdate();
        Q_EMIT countChanged();
    }
}

/*!
    Removes the \a button from the thumbnail toolbar.
 */
void QWinThumbnailToolBar::removeButton(QWinThumbnailToolButton *button)
{
    Q_D(QWinThumbnailToolBar);
    if (button && d->buttonList.contains(button)) {
        button->toolbar = nullptr;
        disconnect(button, &QWinThumbnailToolButton::changed,
                   d, &QWinThumbnailToolBarPrivate::_q_scheduleUpdate);

        d->buttonList.removeAll(button);
        d->_q_scheduleUpdate();
        Q_EMIT countChanged();
    }
}

/*!
    Sets the list of \a buttons in the thumbnail toolbar.

    \note Any existing buttons are replaced.
 */
void QWinThumbnailToolBar::setButtons(const QList<QWinThumbnailToolButton *> &buttons)
{
    Q_D(QWinThumbnailToolBar);
    d->buttonList.clear();
    for (QWinThumbnailToolButton *button : buttons)
        addButton(button);
    d->_q_updateToolbar();
    Q_EMIT countChanged();
}

/*!
    Returns the list of buttons in the thumbnail toolbar.
 */
QList<QWinThumbnailToolButton *> QWinThumbnailToolBar::buttons() const
{
    Q_D(const QWinThumbnailToolBar);
    return d->buttonList;
}

/*!
    \property QWinThumbnailToolBar::count
    \brief the number of buttons in the thumbnail toolbar

    \note The number of buttons is limited to \c 7.
 */
int QWinThumbnailToolBar::count() const
{
    Q_D(const QWinThumbnailToolBar);
    return d->buttonList.size();
}

void QWinThumbnailToolBarPrivate::updateIconicPixmapsEnabled(const bool invalidate)
{
    Q_Q(QWinThumbnailToolBar);
    const HWND hwnd = handle();
    if (!hwnd) {
         qWarning() << Q_FUNC_INFO << "invoked with hwnd=0";
         return;
    }
    const bool enabled = iconicThumbnail || iconicLivePreview;
    q->setIconicPixmapNotificationsEnabled(enabled);
    if (enabled && invalidate) {
        static const auto pDwmInvalidateIconicBitmaps =
            reinterpret_cast<decltype(&DwmInvalidateIconicBitmaps)>(
                QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmInvalidateIconicBitmaps"));
        if (pDwmInvalidateIconicBitmaps) {
            pDwmInvalidateIconicBitmaps(hwnd);
        }
    }
}

/*
    QWinThumbnailToolBarPrivate::IconicPixmapCache caches a HBITMAP of for one of
    the iconic thumbnail or live preview pixmaps. When the messages
    WM_DWMSENDICONICLIVEPREVIEWBITMAP or WM_DWMSENDICONICTHUMBNAIL are received
    (after setting the DWM window attributes accordingly), the bitmap matching the
    maximum size is constructed on demand.
 */

void QWinThumbnailToolBarPrivate::IconicPixmapCache::deleteBitmap()
{
    if (m_bitmap) {
        DeleteObject(m_bitmap);
        m_size = QSize();
        m_bitmap = nullptr;
    }
}

bool QWinThumbnailToolBarPrivate::IconicPixmapCache::setPixmap(const QPixmap &pixmap)
{
    if (pixmap.cacheKey() == m_pixmap.cacheKey())
        return false;
    deleteBitmap();
    m_pixmap = pixmap;
    return true;
}

HBITMAP QWinThumbnailToolBarPrivate::IconicPixmapCache::bitmap(const QSize &maxSize)
{
    if (m_pixmap.isNull())
        return nullptr;
    if (m_bitmap && m_size.width() <= maxSize.width() && m_size.height() <= maxSize.height())
        return m_bitmap;
    deleteBitmap();
    QPixmap pixmap = m_pixmap;
    if (pixmap.width() >= maxSize.width() || pixmap.height() >= maxSize.width())
        pixmap = pixmap.scaled(maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (const HBITMAP bitmap = pixmap.toImage().toHBITMAP()) {
        m_size = pixmap.size();
        m_bitmap = bitmap;
    }
    return m_bitmap;
}

/*!
    \fn  QWinThumbnailToolBar::iconicThumbnailPixmapRequested()

    This signal is emitted when the operating system requests a new iconic thumbnail pixmap,
    typically when the thumbnail is shown.

    \since 5.4
    \sa iconicThumbnailPixmap
*/

/*!
    \fn QWinThumbnailToolBar::iconicLivePreviewPixmapRequested()

    This signal is emitted when the operating system requests a new iconic live preview pixmap,
    typically when the user ALT-tabs to the application.

    \since 5.4
    \sa iconicLivePreviewPixmap
*/

/*!
    \property QWinThumbnailToolBar::iconicPixmapNotificationsEnabled
    \brief whether signals iconicThumbnailPixmapRequested() and iconicLivePreviewPixmapRequested()
     will be emitted

    \since 5.4
    \sa QWinThumbnailToolBar::iconicThumbnailPixmap, QWinThumbnailToolBar::iconicLivePreviewPixmap
 */

bool QWinThumbnailToolBar::iconicPixmapNotificationsEnabled() const
{
    Q_D(const QWinThumbnailToolBar);
    const HWND hwnd = d->handle();
    if (!hwnd)
        return false;
    static const auto pDwmGetWindowAttribute =
        reinterpret_cast<decltype(&DwmGetWindowAttribute)>(
            QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmGetWindowAttribute"));
    if (pDwmGetWindowAttribute) {
        BOOL enabled = FALSE;
        pDwmGetWindowAttribute(hwnd, DWMWA_FORCE_ICONIC_REPRESENTATION, &enabled, sizeof(enabled));
        return enabled;
    }
    return false;
}

void QWinThumbnailToolBar::setIconicPixmapNotificationsEnabled(const bool enabled)
{
    Q_D(const QWinThumbnailToolBar);
    const HWND hwnd = d->handle();
    if (!hwnd) {
        qWarning() << Q_FUNC_INFO << "invoked with hwnd=0";
        return;
    }
    if (iconicPixmapNotificationsEnabled() == enabled)
        return;
    static const auto pDwmSetWindowAttribute =
        reinterpret_cast<decltype(&DwmSetWindowAttribute)>(
            QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmSetWindowAttribute"));
    if (pDwmSetWindowAttribute) {
        const BOOL value = (enabled ? TRUE : FALSE);
        pDwmSetWindowAttribute(hwnd, DWMWA_FORCE_ICONIC_REPRESENTATION, &value, sizeof(value));
        pDwmSetWindowAttribute(hwnd, DWMWA_HAS_ICONIC_BITMAP, &value, sizeof(value));
    }
    Q_EMIT iconicPixmapNotificationsEnabledChanged();
}

/*!
    \property QWinThumbnailToolBar::iconicThumbnailPixmap
    \brief the pixmap for use as a thumbnail representation

    \since 5.4
    \sa QWinThumbnailToolBar::iconicPixmapNotificationsEnabled
 */

void QWinThumbnailToolBar::setIconicThumbnailPixmap(const QPixmap &pixmap)
{
    Q_D(QWinThumbnailToolBar);
    const bool changed = d->iconicThumbnail.setPixmap(pixmap);
    if (d->hasHandle()) // Potentially 0 when invoked from QML loading, see _q_updateToolbar()
        d->updateIconicPixmapsEnabled(changed && !d->withinIconicThumbnailRequest);
    Q_EMIT iconicThumbnailPixmapChanged();
}

QPixmap QWinThumbnailToolBar::iconicThumbnailPixmap() const
{
    Q_D(const QWinThumbnailToolBar);
    return d->iconicThumbnail.pixmap();
}

/*!
    \property QWinThumbnailToolBar::iconicLivePreviewPixmap
    \brief the pixmap for use as a live (peek) preview when tabbing into the application

    \since 5.4
 */

void QWinThumbnailToolBar::setIconicLivePreviewPixmap(const QPixmap &pixmap)
{
    Q_D(QWinThumbnailToolBar);
    const bool changed = d->iconicLivePreview.setPixmap(pixmap);
    if (d->hasHandle()) // Potentially 0 when invoked from QML loading, see _q_updateToolbar()
        d->updateIconicPixmapsEnabled(changed && !d->withinIconicLivePreviewRequest);
    Q_EMIT iconicLivePreviewPixmapChanged();
}

QPixmap QWinThumbnailToolBar::iconicLivePreviewPixmap() const
{
    Q_D(const QWinThumbnailToolBar);
    return d->iconicLivePreview.pixmap();
}

inline void QWinThumbnailToolBarPrivate::updateIconicThumbnail(const MSG *message)
{
    if (!iconicThumbnail)
        return;
    const QSize maxSize(HIWORD(message->lParam), LOWORD(message->lParam));
    if (const HBITMAP bitmap = iconicThumbnail.bitmap(maxSize)) {
        static const auto pDwmSetIconicThumbnail =
            reinterpret_cast<decltype(&DwmSetIconicThumbnail)>(
                QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmSetIconicThumbnail"));
        if (pDwmSetIconicThumbnail) {
            pDwmSetIconicThumbnail(message->hwnd, bitmap, DWM_SIT_DISPLAYFRAME);
        }
    }
}

inline void QWinThumbnailToolBarPrivate::updateIconicLivePreview(const MSG *message)
{
    if (!iconicLivePreview)
        return;
    RECT rect;
    GetClientRect(message->hwnd, &rect);
    const QSize maxSize(rect.right, rect.bottom);
    POINT offset = {0, 0};
    if (const HBITMAP bitmap = iconicLivePreview.bitmap(maxSize)) {
        static const auto pDwmSetIconicLivePreviewBitmap =
            reinterpret_cast<decltype(&DwmSetIconicLivePreviewBitmap)>(
                QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmSetIconicLivePreviewBitmap"));
        if (pDwmSetIconicLivePreviewBitmap) {
            pDwmSetIconicLivePreviewBitmap(message->hwnd, bitmap, &offset, DWM_SIT_DISPLAYFRAME);
        }
    }
}

/*!
    Removes all buttons from the thumbnail toolbar.
 */
void QWinThumbnailToolBar::clear()
{
    setButtons(QList<QWinThumbnailToolButton *>());
}

QWinThumbnailToolBarPrivate::QWinThumbnailToolBarPrivate(QWinThumbnailToolBar *q) : QObject(nullptr)
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
    buttonList.reserve(windowsLimitedThumbbarSize);
    qApp->installNativeEventFilter(this);
}

QWinThumbnailToolBarPrivate::~QWinThumbnailToolBarPrivate()
{
    if (comInited) {
        if (pTbList) {
            pTbList->Release();
            pTbList = nullptr;
        }
        CoUninitialize();
    }
    qApp->removeNativeEventFilter(this);
}

inline bool QWinThumbnailToolBarPrivate::hasHandle() const
{
    return window && window->handle();
}

inline HWND QWinThumbnailToolBarPrivate::handle() const
{
    return hasHandle() ? reinterpret_cast<HWND>(window->winId()) : nullptr;
}

void QWinThumbnailToolBarPrivate::initToolbar()
{
    if (!pTbList || !window)
        return;
    THUMBBUTTON buttons[windowsLimitedThumbbarSize];
    initButtons(buttons);
    pTbList->ThumbBarAddButtons(handle(), windowsLimitedThumbbarSize, buttons);
}

void QWinThumbnailToolBarPrivate::clearToolbar()
{
    if (!pTbList || !window)
        return;
    THUMBBUTTON buttons[windowsLimitedThumbbarSize];
    initButtons(buttons);
    pTbList->ThumbBarUpdateButtons(handle(), windowsLimitedThumbbarSize, buttons);
}

void QWinThumbnailToolBarPrivate::_q_updateToolbar()
{
    updateScheduled = false;
    if (!pTbList || !window)
        return;
    THUMBBUTTON buttons[windowsLimitedThumbbarSize];
    QList<HICON> createdIcons;
    initButtons(buttons);
    const int thumbbarSize = qMin(buttonList.size(), windowsLimitedThumbbarSize);
    // filling from the right fixes some strange bug which makes last button bg look like first btn bg
    for (int i = (windowsLimitedThumbbarSize - thumbbarSize); i < windowsLimitedThumbbarSize; i++) {
        QWinThumbnailToolButton *button = buttonList.at(i - (windowsLimitedThumbbarSize - thumbbarSize));
        buttons[i].dwFlags = static_cast<THUMBBUTTONFLAGS>(makeNativeButtonFlags(button));
        buttons[i].dwMask  = static_cast<THUMBBUTTONMASK>(makeButtonMask(button));
        if (!button->icon().isNull()) {;
            buttons[i].hIcon = button->icon().pixmap(GetSystemMetrics(SM_CXSMICON)).toImage().toHICON();
            if (!buttons[i].hIcon)
                buttons[i].hIcon = static_cast<HICON>(LoadImageW(nullptr, IDI_APPLICATION, IMAGE_ICON,
                                                                 SM_CXSMICON, SM_CYSMICON, LR_SHARED));
            else
                createdIcons << buttons[i].hIcon;
        }
        if (!button->toolTip().isEmpty()) {
            buttons[i].szTip[button->toolTip().left(sizeof(buttons[i].szTip)/sizeof(buttons[i].szTip[0]) - 1).toWCharArray(buttons[i].szTip)] = 0;
        }
    }
    pTbList->ThumbBarUpdateButtons(handle(), windowsLimitedThumbbarSize, buttons);
    updateIconicPixmapsEnabled(false);
    for (auto & button : buttons) {
        if (button.hIcon) {
            if (createdIcons.contains(button.hIcon))
                DestroyIcon(button.hIcon);
            else
                DeleteObject(button.hIcon);
        }
    }
}

void QWinThumbnailToolBarPrivate::_q_scheduleUpdate()
{
    if (updateScheduled)
        return;
    updateScheduled = true;
    QTimer::singleShot(0, this, &QWinThumbnailToolBarPrivate::_q_updateToolbar);
}

bool QWinThumbnailToolBarPrivate::eventFilter(QObject *object, QEvent *event)
{
    if (object == window && event->type() == QWinEvent::TaskbarButtonCreated) {
        initToolbar();
        _q_scheduleUpdate();
    }
    return QObject::eventFilter(object, event);
}

bool QWinThumbnailToolBarPrivate::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    if ((eventType != QByteArrayLiteral("windows_generic_MSG")) || !message || !result) {
        return false;
    }
    const auto msg = static_cast<LPMSG>(message);
    if (!msg->hwnd) {
        return false;
    }
    if (handle() != msg->hwnd)
        return false;
    Q_Q(QWinThumbnailToolBar);
    switch (msg->message) {
    case WM_COMMAND: {
        if (HIWORD(msg->wParam) == THBN_CLICKED) {
            const int buttonId = LOWORD(msg->wParam) - (windowsLimitedThumbbarSize - qMin(windowsLimitedThumbbarSize, buttonList.size()));
            buttonList.at(buttonId)->click();
            *result = 0;
            return true;
        }
    } break;
    case WM_DWMSENDICONICTHUMBNAIL: {
        withinIconicThumbnailRequest = true;
        Q_EMIT q->iconicThumbnailPixmapRequested();
        withinIconicThumbnailRequest = false;
        updateIconicThumbnail(msg);
        *result = 0;
        return true;
    }
    case WM_DWMSENDICONICLIVEPREVIEWBITMAP: {
        withinIconicLivePreviewRequest = true;
        Q_EMIT q->iconicLivePreviewPixmapRequested();
        withinIconicLivePreviewRequest = false;
        updateIconicLivePreview(msg);
        *result = 0;
        return true;
    }
    default:
        break;
    }
    return false;
}

void QWinThumbnailToolBarPrivate::initButtons(THUMBBUTTON *buttons)
{
    for (int i = 0; i < windowsLimitedThumbbarSize; i++) {
        SecureZeroMemory(&buttons[i], sizeof(buttons[i]));
        buttons[i].iId = UINT(i);
        buttons[i].dwFlags = THBF_HIDDEN;
        buttons[i].dwMask  = THB_FLAGS;
    }
}

int QWinThumbnailToolBarPrivate::makeNativeButtonFlags(const QWinThumbnailToolButton *button)
{
    int nativeFlags = 0;
    if (button->isEnabled())
        nativeFlags |= THBF_ENABLED;
    else
        nativeFlags |= THBF_DISABLED;
    if (button->dismissOnClick())
        nativeFlags |= THBF_DISMISSONCLICK;
    if (button->isFlat())
        nativeFlags |= THBF_NOBACKGROUND;
    if (!button->isVisible())
        nativeFlags |= THBF_HIDDEN;
    if (!button->isInteractive())
        nativeFlags |= THBF_NONINTERACTIVE;
    return nativeFlags;
}

int QWinThumbnailToolBarPrivate::makeButtonMask(const QWinThumbnailToolButton *button)
{
    int mask = 0;
    mask |= THB_FLAGS;
    if (!button->icon().isNull())
        mask |= THB_ICON;
    if (!button->toolTip().isEmpty())
        mask |= THB_TOOLTIP;
    return mask;
}

QT_END_NAMESPACE
