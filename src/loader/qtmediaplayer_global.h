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

#pragma once

#include <QtCore/qglobal.h>
#include <QtCore/qloggingcategory.h>

#ifndef QTMEDIAPLAYER_LOADER_API
#  ifdef QTMEDIAPLAYER_LOADER_STATIC
#    define QTMEDIAPLAYER_LOADER_API
#  else
#    ifdef QTMEDIAPLAYER_LOADER_BUILD_LIBRARY
#      define QTMEDIAPLAYER_LOADER_API Q_DECL_EXPORT
#    else
#      define QTMEDIAPLAYER_LOADER_API Q_DECL_IMPORT
#    endif
#  endif
#endif

#if defined(Q_OS_WIN) && !defined(Q_OS_WINDOWS)
#  define Q_OS_WINDOWS
#endif

#ifndef Q_DISABLE_COPY_MOVE
#  define Q_DISABLE_COPY_MOVE(Class) \
      Q_DISABLE_COPY(Class) \
      Class(Class &&) = delete; \
      Class &operator=(Class &&) = delete;
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 0))
#  define qAsConst(i) std::as_const(i)
#endif

#ifndef Q_NAMESPACE_EXPORT
#  define Q_NAMESPACE_EXPORT(ns) Q_NAMESPACE(ns)
#endif

#ifndef Q_NODISCARD
#  if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#    define Q_NODISCARD [[nodiscard]]
#  else
#    define Q_NODISCARD
#  endif
#endif

#ifndef QTMEDIAPLAYER_MAKE_VERSION
#  define QTMEDIAPLAYER_MAKE_VERSION(Major, Minor, Patch, Tweak) \
      (((Major & 0xff) << 24) | ((Minor & 0xff) << 16) | ((Patch & 0xff) << 8) | (Tweak & 0xff))
#endif

#ifndef QTMEDIAPLAYER_NAMESPACE
#  define QTMEDIAPLAYER_NAMESPACE wangwenx190::QtMediaPlayer
#endif

#ifndef QTMEDIAPLAYER_BEGIN_NAMESPACE
#  define QTMEDIAPLAYER_BEGIN_NAMESPACE namespace QTMEDIAPLAYER_NAMESPACE {
#endif

#ifndef QTMEDIAPLAYER_END_NAMESPACE
#  define QTMEDIAPLAYER_END_NAMESPACE }
#endif

#ifndef QTMEDIAPLAYER_USE_NAMESPACE
#  define QTMEDIAPLAYER_USE_NAMESPACE using namespace QTMEDIAPLAYER_NAMESPACE;
#endif

#ifndef QTMEDIAPLAYER_PREPEND_NAMESPACE
#  define QTMEDIAPLAYER_PREPEND_NAMESPACE(X) ::QTMEDIAPLAYER_NAMESPACE::X
#endif

#ifndef QTMEDIAPLAYER_QML_URI
#  define QTMEDIAPLAYER_QML_URI "org.wangwenx190.QtMediaPlayer"
#endif

#ifndef QTMEDIAPLAYER_QML_REGISTER
#  define QTMEDIAPLAYER_QML_REGISTER(className) \
      qmlRegisterType<className>(QTMEDIAPLAYER_QML_URI, 1, 0, #className)
#endif

#ifndef QTMEDIAPLAYER_QML_NAMED_REGISTER
#  define QTMEDIAPLAYER_QML_NAMED_REGISTER(className, typeName) \
      qmlRegisterType<className>(QTMEDIAPLAYER_QML_URI, 1, 0, #typeName)
#endif

QTMEDIAPLAYER_BEGIN_NAMESPACE
Q_DECLARE_LOGGING_CATEGORY(lcQMPLoader)
[[maybe_unused]] static constexpr const int QTMEDIAPLAYER_VERSION_MAJOR = 1;
[[maybe_unused]] static constexpr const int QTMEDIAPLAYER_VERSION_MINOR = 1;
[[maybe_unused]] static constexpr const int QTMEDIAPLAYER_VERSION_PATCH = 0;
[[maybe_unused]] static constexpr const int QTMEDIAPLAYER_VERSION_TWEAK = 0;
[[maybe_unused]] static constexpr const int QTMEDIAPLAYER_VERSION
                               = QTMEDIAPLAYER_MAKE_VERSION(QTMEDIAPLAYER_VERSION_MAJOR,
                                                            QTMEDIAPLAYER_VERSION_MINOR,
                                                            QTMEDIAPLAYER_VERSION_PATCH,
                                                            QTMEDIAPLAYER_VERSION_TWEAK);
[[maybe_unused]] static constexpr const char QTMEDIAPLAYER_VERSION_STR[] = "1.1.0.0";
QTMEDIAPLAYER_END_NAMESPACE
