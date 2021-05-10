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

#include "include/mpv/client.h"
#include "include/mpv/render_gl.h"
#include <QtCore/qvariant.h>

namespace MPV::Qt
{

bool libmpvAvailable();

QVariant node_to_variant(const mpv_node *node);

/**
 * This is used to return error codes wrapped in QVariant for functions which
 * return QVariant.
 *
 * You can use get_error() or is_error() to extract the error status from a
 * QVariant value.
 */
struct ErrorReturn
{
    /**
     * enum mpv_error value (or a value outside of it if ABI was extended)
     */
    int errorCode = -1;

    explicit ErrorReturn() = default;
    explicit ErrorReturn(int code)
    {
        errorCode = code;
    }
};

/**
 * Return the mpv error code packed into a QVariant, or 0 (success) if it's not
 * an error value.
 *
 * @return error code (<0) or success (>=0)
 */
int get_error(const QVariant &v);

/**
 * Return whether the QVariant carries a mpv error code.
 */
bool is_error(const QVariant &v);

/**
 * Return the given property as mpv_node converted to QVariant, or QVariant()
 * on error.
 *
 * @param name the property name
 * @return the property value, or an ErrorReturn with the error code
 */
QVariant get_property(mpv_handle *ctx, const QString &name);

/**
 * Set the given property as mpv_node converted from the QVariant argument.
 *
 * @return mpv error code (<0 on error, >= 0 on success)
 */
int set_property(mpv_handle *ctx, const QString &name, const QVariant &v);

/**
 * Set the given property asynchronously as mpv_node converted from the QVariant
 * argument.
 *
 * @return mpv error code (<0 on error, >= 0 on success)
 */
int set_property_async(mpv_handle *ctx, const QString &name, const QVariant &v, quint64 reply_userdata);

/**
 * mpv_command_node() equivalent.
 *
 * @param args command arguments, with args[0] being the command name as string
 * @return the property value, or an ErrorReturn with the error code
 */
QVariant command(mpv_handle *ctx, const QVariant &args);

/**
 * Send commands to mpv asynchronously.
 *
 * @param args command arguments, with args[0] being the command name as string
 * @return mpv error code (<0 on error, >= 0 on success)
 */
int command_async(mpv_handle *ctx, const QVariant &args, quint64 reply_userdata);

int load_config_file(mpv_handle *ctx, const QString &fileName);

int observe_property(mpv_handle *ctx, const QString &name, quint64 reply_userdata);

QString error_string(int errCode);

int render_context_create(mpv_render_context **res, mpv_handle *ctx, mpv_render_param *params);

void render_context_set_update_callback(mpv_render_context *ctx, mpv_render_update_fn callback, void *callback_ctx);

int render_context_render(mpv_render_context *ctx, mpv_render_param *params);

void set_wakeup_callback(mpv_handle *ctx, void (*cb)(void *d), void *d);

int initialize(mpv_handle *ctx);

void render_context_free(mpv_render_context *ctx);

void terminate_destroy(mpv_handle *ctx);

int request_log_messages(mpv_handle *ctx, const QString &min_level);

mpv_event *wait_event(mpv_handle *ctx, qreal timeout);

mpv_handle *create();

QString event_name(mpv_event_id event);

} // namespace MPV::Qt

Q_DECLARE_METATYPE(MPV::Qt::ErrorReturn)