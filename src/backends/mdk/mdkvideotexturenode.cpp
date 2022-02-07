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

#include "mdkvideotexturenode.h"
#include "mdkplayer.h"
#include "include/mdk/Player.h"
#include <QtQuick/qquickwindow.h>
#include <QtGui/qscreen.h>

QTMEDIAPLAYER_BEGIN_NAMESPACE

MDKVideoTextureNode::MDKVideoTextureNode(QQuickItem *item) : VideoTextureNode(item)
{
    Q_ASSERT(item);
    if (!item) {
        qFatal("null mdk player item.");
    }
    m_item = static_cast<MDKPlayer *>(item);
    m_window = m_item->window();
    m_player = m_item->m_player;
    connect(m_window, &QQuickWindow::beforeRendering, this, &MDKVideoTextureNode::render);
    connect(m_window, &QQuickWindow::screenChanged, this, [this](QScreen *screen){
        Q_UNUSED(screen);
        m_item->update();
    });
}

MDKVideoTextureNode::~MDKVideoTextureNode()
{
    const auto tex = texture();
    if (tex) {
        delete tex;
    }
    // When device lost occurs
    const auto player = m_player.lock();
    if (!player) {
        return;
    }
    player->setVideoSurfaceSize(-1, -1, m_window);
    qCDebug(lcQMPMDK) << "Renderer destroyed.";
}

void MDKVideoTextureNode::sync()
{
    Q_ASSERT(m_item);
    Q_ASSERT(m_window);
    if (!m_item || !m_window) {
        return;
    }

    // effectiveDevicePixelRatio() will always give the correct result even if QQuickWindow is not available.
    const auto dpr = m_window->effectiveDevicePixelRatio();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    const QSize newSize = QSizeF(m_item->size() * dpr).toSize();
#else
    const QSize newSize = {qRound(m_item->width() * dpr), qRound(m_item->height() * dpr)};
#endif
    if (texture() && (newSize == m_size)) {
        return;
    }
    const auto player = m_player.lock();
    if (!player) {
        return;
    }
    m_size = newSize;
    const auto tex = ensureTexture(player.data(), m_size);
    if (!tex) {
        return;
    }
    delete texture();
    setTexture(tex);
    // MUST set when texture() is available
    setTextureCoordinatesTransform(m_transformMode);
    setFiltering(QSGTexture::Linear);
    // Qt's own API will apply correct DPR automatically. Don't double scale.
    setRect(0, 0, m_item->width(), m_item->height());
    // if qsg render loop is threaded, a new render thread will be created when item's window changes, so mdk vo_opaque parameter must be bound to item window
    player->setVideoSurfaceSize(m_size.width(), m_size.height(), m_window);
}

// This is hooked up to beforeRendering() so we can start our own render
// command encoder. If we instead wanted to use the scenegraph's render command
// encoder (targeting the window), it should be connected to
// beforeRenderPassRecording() instead.
void MDKVideoTextureNode::render()
{
    const auto player = m_player.lock();
    if (!player) {
        return;
    }
    player->renderVideo(m_window);
}

QTMEDIAPLAYER_END_NAMESPACE
