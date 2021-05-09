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

#pragma once

#include <QtCore/qglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>

#ifndef QTMEDIAPLAYER_API
#ifdef QTMEDIAPLAYER_STATIC
#define QTMEDIAPLAYER_API
#else
#ifdef QTMEDIAPLAYER_BUILD_LIBRARY
#define QTMEDIAPLAYER_API Q_DECL_EXPORT
#else
#define QTMEDIAPLAYER_API Q_DECL_IMPORT
#endif
#endif
#endif

#if defined(Q_OS_WIN) && !defined(Q_OS_WINDOWS)
#define Q_OS_WINDOWS
#endif

#ifndef Q_DISABLE_MOVE
#define Q_DISABLE_MOVE(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;
#endif

#ifndef Q_DISABLE_COPY_MOVE
#define Q_DISABLE_COPY_MOVE(Class) \
    Q_DISABLE_COPY(Class) \
    Q_DISABLE_MOVE(Class)
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 0))
#define qAsConst(i) std::as_const(i)
#endif

#ifndef QTMEDIAPLAYER_NAMESPACE
#define QTMEDIAPLAYER_NAMESPACE wangwenx190::_qmp_ns
#endif

#define QTMEDIAPLAYER_BEGIN_NAMESPACE namespace QTMEDIAPLAYER_NAMESPACE {
#define QTMEDIAPLAYER_END_NAMESPACE }
#define QTMEDIAPLAYER_USE_NAMESPACE using namespace QTMEDIAPLAYER_NAMESPACE;
#define QTMEDIAPLAYER_PREPEND_NAMESPACE(x) ::QTMEDIAPLAYER_NAMESPACE::x

#ifndef QTMEDIAPLAYER_QML_URI
#define QTMEDIAPLAYER_QML_URI "org.wangwenx190.QtMediaPlayer"
#endif

#ifndef QTMEDIAPLAYER_QML_TYPENAME
#define QTMEDIAPLAYER_QML_TYPENAME "QtMediaPlayer"
#endif

#ifndef QTMEDIAPLAYER_QML_REGISTER
#define QTMEDIAPLAYER_QML_REGISTER(className) \
    qmlRegisterType<className>(QTMEDIAPLAYER_QML_URI, 1, 0, QTMEDIAPLAYER_QML_TYPENAME)
#endif
