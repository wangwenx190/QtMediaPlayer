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

#include "qtmediaplayer_global.h"
#include <QtQuick/qsgtextureprovider.h>
#include <QtQuick/qsgsimpletexturenode.h>

QT_BEGIN_NAMESPACE
QT_FORWARD_DECLARE_CLASS(QQuickWindow)
QT_FORWARD_DECLARE_CLASS(QOpenGLFramebufferObject)
QT_END_NAMESPACE

QTMEDIAPLAYER_BEGIN_NAMESPACE

class QtMPVPlayer;

class MPVVideoTextureNode : public QSGTextureProvider, public QSGSimpleTextureNode
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MPVVideoTextureNode)

public:
    explicit MPVVideoTextureNode(QtMPVPlayer *item);
    ~MPVVideoTextureNode() override;

    QSGTexture *texture() const override;

    void sync();

private Q_SLOTS:
    void render();

private:
    QSGTexture *ensureTexture(const QSize &size);

private:
#if QT_CONFIG(opengl)
    QScopedPointer<QOpenGLFramebufferObject> fbo_gl;
#endif
    QQuickWindow *m_window = nullptr;
    QtMPVPlayer *m_item = nullptr;
    QSize m_size = {};
};

QTMEDIAPLAYER_END_NAMESPACE
