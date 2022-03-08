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

#include "qwinevent_p.h"
#include <QtGui/qguiapplication.h>
#include <QtGui/qwindow.h>
#include <QtCore/qt_windows.h>
#include <QtCore/qmutex.h>

QT_BEGIN_NAMESPACE

int QWinEvent::TaskbarButtonCreated = QEvent::registerEventType();

QWinEvent::QWinEvent(const int type) : QEvent(static_cast<QEvent::Type>(type)) {}

QWinEvent::~QWinEvent() = default;

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

QWinEventFilter::QWinEventFilter() : QAbstractNativeEventFilter(),
    tbButtonCreatedMsgId(RegisterWindowMessageW(L"TaskbarButtonCreated"))
{
}

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

void QWinEventFilter::setup()
{
    QMutexLocker locker(&g_globalFilter()->mutex);
    if (g_globalFilter()->instance.isNull()) {
        g_globalFilter()->instance.reset(new QWinEventFilter);
        qApp->installNativeEventFilter(g_globalFilter()->instance.data());
    }
}

QT_END_NAMESPACE
