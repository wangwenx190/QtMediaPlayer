/*
 * MIT License
 *
 * Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)
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

#include "mpvbackend.h"
#include "mpvplayer.h"
#include "mpvqthelper.h"
#include <QtQuick/qsgrendererinterface.h>

QTMEDIAPLAYER_BEGIN_NAMESPACE
Q_LOGGING_CATEGORY(lcQMPMPV, "wangwenx190.mediaplayer.mpv")
QTMEDIAPLAYER_END_NAMESPACE

bool RegisterBackend(const char *name)
{
    if (!name) {
        return false;
    }
    if (qstricmp(name, "mpv") == 0) {
        if (MPV::Qt::isLibmpvAvailable()) {
            const int typeId = QTMEDIAPLAYER_QML_REGISTER(QTMEDIAPLAYER_PREPEND_NAMESPACE(MPVPlayer));
            Q_UNUSED(typeId);
            return true;
        }
    }
    return false;
}

const char *GetBackendName()
{
    return "MPV";
}

const char *GetBackendVersion()
{
    return qstrdup(qUtf8Printable(MPV::Qt::getLibmpvVersion()));
}

bool IsRHIBackendSupported(const int enumIntValue)
{
    const auto api = static_cast<QSGRendererInterface::GraphicsApi>(enumIntValue);
    switch (api) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    case QSGRendererInterface::OpenGL: // Equal to OpenGLRhi in Qt6.
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    case QSGRendererInterface::OpenGLRhi:
#endif
    case QSGRendererInterface::Software:
        return true;
    default:
        return false;
    }
    return false;
}
