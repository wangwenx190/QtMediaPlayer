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

#include "mdkbackend.h"
#include "mdkplayer.h"
#include "mdkqthelper.h"

Q_LOGGING_CATEGORY(lcQMPMDK, "wangwenx190.mediaplayer.mdk")

bool RegisterBackend(const char *name)
{
    if (qstricmp(name, "mdk") == 0) {
        if (MDK::Qt::isMDKAvailable()) {
            const int typeId = QTMEDIAPLAYER_QML_REGISTER(QTMEDIAPLAYER_PREPEND_NAMESPACE(MDKPlayer));
            Q_UNUSED(typeId);
            return true;
        }
    }
    return false;
}

const char *GetBackendName()
{
    return "MDK";
}

const char *GetBackendVersion()
{
    return qstrdup(qUtf8Printable(MDK::Qt::getMDKVersion()));
}

bool IsRHIBackendSupported(const int enumIntValue)
{
    // MDK supports all QtRHI backends.
    Q_UNUSED(enumIntValue);
    return true;
}
