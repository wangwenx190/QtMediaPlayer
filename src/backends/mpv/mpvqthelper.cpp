/*
 * Copyright (C) 2017 the mpv developers
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

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

#include "mpvqthelper.h"
#include "include/mpv/render_gl.h"
#include <QtCore/qdebug.h>
#include <QtCore/qlibrary.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>

#ifndef WWX190_GENERATE_MPVAPI
#define WWX190_GENERATE_MPVAPI(funcName, resultType, ...) \
    using _WWX190_MPVAPI_lp_##funcName = resultType(*)(__VA_ARGS__); \
    _WWX190_MPVAPI_lp_##funcName m_lp_##funcName = nullptr;
#endif

#ifndef WWX190_RESOLVE_MPVAPI
#define WWX190_RESOLVE_MPVAPI(funcName) \
    if (!m_lp_##funcName) { \
        qCDebug(lcQMPMPV) << "Resolving function:" << #funcName; \
        m_lp_##funcName = reinterpret_cast<_WWX190_MPVAPI_lp_##funcName>(library.resolve(#funcName)); \
        Q_ASSERT(m_lp_##funcName); \
        if (!m_lp_##funcName) { \
            qCWarning(lcQMPMPV) << "Failed to resolve function" << #funcName; \
        } \
    }
#endif

#ifndef WWX190_SETNULL_MPVAPI
#define WWX190_SETNULL_MPVAPI(funcName) \
    if (m_lp_##funcName) { \
        m_lp_##funcName = nullptr; \
    }
#endif

#ifndef WWX190_NOTNULL_MPVAPI
#define WWX190_NOTNULL_MPVAPI(funcName) (m_lp_##funcName != nullptr)
#endif

#ifndef WWX190_CALL_MPVAPI
#define WWX190_CALL_MPVAPI(funcName, ...) \
    if (MPV::Qt::mpvData()->m_lp_##funcName) { \
        MPV::Qt::mpvData()->m_lp_##funcName(__VA_ARGS__); \
    }
#endif

#ifndef WWX190_CALL_MPVAPI_RETURN
#define WWX190_CALL_MPVAPI_RETURN(funcName, defRet, ...) \
    (MPV::Qt::mpvData()->m_lp_##funcName ? MPV::Qt::mpvData()->m_lp_##funcName(__VA_ARGS__) : defRet)
#endif

static const char _mpvHelper_libmpv_fileName_envVar[] = "_WWX190_MPVPLAYER_LIBMPV_FILENAME";

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const MPV::Qt::ErrorReturn &err)
{
    const QDebugStateSaver saver(d);
    d.nospace();
    d.noquote();
    const QString str = QStringLiteral("MPV::Qt::ErrorReturn(errorCode=%1)").arg(QString::number(err.errorCode));
    d << str;
    return d;
}
#endif

namespace MPV::Qt
{

struct MPVData
{
public:
    // client.h
    WWX190_GENERATE_MPVAPI(mpv_client_api_version, unsigned long)
    WWX190_GENERATE_MPVAPI(mpv_error_string, const char *, int)
    WWX190_GENERATE_MPVAPI(mpv_free, void, void *)
    WWX190_GENERATE_MPVAPI(mpv_client_name, const char *, mpv_handle *)
    WWX190_GENERATE_MPVAPI(mpv_client_id, qint64, mpv_handle *)
    WWX190_GENERATE_MPVAPI(mpv_create, mpv_handle *)
    WWX190_GENERATE_MPVAPI(mpv_initialize, int, mpv_handle *)
    WWX190_GENERATE_MPVAPI(mpv_destroy, void, mpv_handle *)
    WWX190_GENERATE_MPVAPI(mpv_terminate_destroy, void, mpv_handle *)
    WWX190_GENERATE_MPVAPI(mpv_create_client, mpv_handle *, mpv_handle *, const char *)
    WWX190_GENERATE_MPVAPI(mpv_create_weak_client, mpv_handle *, mpv_handle *, const char *)
    WWX190_GENERATE_MPVAPI(mpv_load_config_file, int, mpv_handle *, const char *)
    WWX190_GENERATE_MPVAPI(mpv_get_time_us, qint64, mpv_handle *)
    WWX190_GENERATE_MPVAPI(mpv_free_node_contents, void, mpv_node *)
    WWX190_GENERATE_MPVAPI(mpv_set_option, int, mpv_handle *, const char *, mpv_format, void *)
    WWX190_GENERATE_MPVAPI(mpv_set_option_string, int, mpv_handle *, const char *, const char *)
    WWX190_GENERATE_MPVAPI(mpv_command, int, mpv_handle *, const char **)
    WWX190_GENERATE_MPVAPI(mpv_command_node, int, mpv_handle *, mpv_node *, mpv_node *)
    WWX190_GENERATE_MPVAPI(mpv_command_ret, int, mpv_handle *, const char **, mpv_node *)
    WWX190_GENERATE_MPVAPI(mpv_command_string, int, mpv_handle *, const char *)
    WWX190_GENERATE_MPVAPI(mpv_command_async, int, mpv_handle *, quint64, const char **)
    WWX190_GENERATE_MPVAPI(mpv_command_node_async, int, mpv_handle *, quint64, mpv_node *)
    WWX190_GENERATE_MPVAPI(mpv_abort_async_command, void, mpv_handle *, quint64)
    WWX190_GENERATE_MPVAPI(mpv_set_property, int, mpv_handle *, const char *, mpv_format, void *)
    WWX190_GENERATE_MPVAPI(mpv_set_property_string, int, mpv_handle *, const char *, const char *)
    WWX190_GENERATE_MPVAPI(mpv_set_property_async, int, mpv_handle *, quint64, const char *, mpv_format, void *)
    WWX190_GENERATE_MPVAPI(mpv_get_property, int, mpv_handle *, const char *, mpv_format, void *)
    WWX190_GENERATE_MPVAPI(mpv_get_property_string, char *, mpv_handle *, const char *)
    WWX190_GENERATE_MPVAPI(mpv_get_property_osd_string, char *, mpv_handle *, const char *)
    WWX190_GENERATE_MPVAPI(mpv_get_property_async, int, mpv_handle *, quint64, const char *, mpv_format)
    WWX190_GENERATE_MPVAPI(mpv_observe_property, int, mpv_handle *, quint64, const char *, mpv_format)
    WWX190_GENERATE_MPVAPI(mpv_unobserve_property, int, mpv_handle *, quint64)
    WWX190_GENERATE_MPVAPI(mpv_event_name, const char *, mpv_event_id)
    WWX190_GENERATE_MPVAPI(mpv_event_to_node, int, mpv_node *, mpv_event *)
    WWX190_GENERATE_MPVAPI(mpv_request_event, int, mpv_handle *, mpv_event_id, int)
    WWX190_GENERATE_MPVAPI(mpv_request_log_messages, int, mpv_handle *, const char *)
    WWX190_GENERATE_MPVAPI(mpv_wait_event, mpv_event *, mpv_handle *, qreal)
    WWX190_GENERATE_MPVAPI(mpv_wakeup, void, mpv_handle *)
    WWX190_GENERATE_MPVAPI(mpv_set_wakeup_callback, void, mpv_handle *, void (*)(void *), void *)
    WWX190_GENERATE_MPVAPI(mpv_wait_async_requests, void, mpv_handle *)
    WWX190_GENERATE_MPVAPI(mpv_hook_add, int, mpv_handle *, quint64, const char *, int)
    WWX190_GENERATE_MPVAPI(mpv_hook_continue, int, mpv_handle *, quint64)

    // render.h
    WWX190_GENERATE_MPVAPI(mpv_render_context_create, int, mpv_render_context **, mpv_handle *, mpv_render_param *)
    WWX190_GENERATE_MPVAPI(mpv_render_context_set_parameter, int, mpv_render_context *, mpv_render_param)
    WWX190_GENERATE_MPVAPI(mpv_render_context_get_info, int, mpv_render_context *, mpv_render_param)
    WWX190_GENERATE_MPVAPI(mpv_render_context_set_update_callback, void, mpv_render_context *, mpv_render_update_fn, void *)
    WWX190_GENERATE_MPVAPI(mpv_render_context_update, quint64, mpv_render_context *)
    WWX190_GENERATE_MPVAPI(mpv_render_context_render, int, mpv_render_context *, mpv_render_param *)
    WWX190_GENERATE_MPVAPI(mpv_render_context_report_swap, void, mpv_render_context *)
    WWX190_GENERATE_MPVAPI(mpv_render_context_free, void, mpv_render_context *)

    explicit MPVData()
    {
        const bool result = load(qEnvironmentVariable(_mpvHelper_libmpv_fileName_envVar, QStringLiteral("mpv")));
        Q_UNUSED(result);
    }

    ~MPVData()
    {
        const bool result = unload();
        Q_UNUSED(result);
    }

    [[nodiscard]] bool load(const QString &path)
    {
        Q_ASSERT(!path.isEmpty());
        if (path.isEmpty()) {
            qCWarning(lcQMPMPV) << "Failed to load libmpv: empty library path.";
            return false;
        }

        if (isLoaded()) {
            qCDebug(lcQMPMPV) << "libmpv already loaded. Unloading ...";
            if (!unload()) {
                return false;
            }
        }

        library.setFileName(path);
        const bool result = library.load();
        if (result) {
            // We can't get the full file name if QLibrary is not loaded.
            QFileInfo fi(library.fileName());
            fi.makeAbsolute();
            qCDebug(lcQMPMPV) << "Start loading libmpv from:" << QDir::toNativeSeparators(fi.canonicalFilePath());
        } else {
            qCDebug(lcQMPMPV) << "Start loading libmpv from:" << QDir::toNativeSeparators(path);
            qCWarning(lcQMPMPV) << "Failed to load libmpv:" << library.errorString();
            return false;
        }

        // client.h
        WWX190_RESOLVE_MPVAPI(mpv_client_api_version)
        WWX190_RESOLVE_MPVAPI(mpv_error_string)
        WWX190_RESOLVE_MPVAPI(mpv_free)
        WWX190_RESOLVE_MPVAPI(mpv_client_name)
        WWX190_RESOLVE_MPVAPI(mpv_client_id)
        WWX190_RESOLVE_MPVAPI(mpv_create)
        WWX190_RESOLVE_MPVAPI(mpv_initialize)
        WWX190_RESOLVE_MPVAPI(mpv_destroy)
        WWX190_RESOLVE_MPVAPI(mpv_terminate_destroy)
        WWX190_RESOLVE_MPVAPI(mpv_create_client)
        WWX190_RESOLVE_MPVAPI(mpv_create_weak_client)
        WWX190_RESOLVE_MPVAPI(mpv_load_config_file)
        WWX190_RESOLVE_MPVAPI(mpv_get_time_us)
        WWX190_RESOLVE_MPVAPI(mpv_free_node_contents)
        WWX190_RESOLVE_MPVAPI(mpv_set_option)
        WWX190_RESOLVE_MPVAPI(mpv_set_option_string)
        WWX190_RESOLVE_MPVAPI(mpv_command)
        WWX190_RESOLVE_MPVAPI(mpv_command_node)
        WWX190_RESOLVE_MPVAPI(mpv_command_ret)
        WWX190_RESOLVE_MPVAPI(mpv_command_string)
        WWX190_RESOLVE_MPVAPI(mpv_command_async)
        WWX190_RESOLVE_MPVAPI(mpv_command_node_async)
        WWX190_RESOLVE_MPVAPI(mpv_abort_async_command)
        WWX190_RESOLVE_MPVAPI(mpv_set_property)
        WWX190_RESOLVE_MPVAPI(mpv_set_property_string)
        WWX190_RESOLVE_MPVAPI(mpv_set_property_async)
        WWX190_RESOLVE_MPVAPI(mpv_get_property)
        WWX190_RESOLVE_MPVAPI(mpv_get_property_string)
        WWX190_RESOLVE_MPVAPI(mpv_get_property_osd_string)
        WWX190_RESOLVE_MPVAPI(mpv_get_property_async)
        WWX190_RESOLVE_MPVAPI(mpv_observe_property)
        WWX190_RESOLVE_MPVAPI(mpv_unobserve_property)
        WWX190_RESOLVE_MPVAPI(mpv_event_name)
        WWX190_RESOLVE_MPVAPI(mpv_event_to_node)
        WWX190_RESOLVE_MPVAPI(mpv_request_event)
        WWX190_RESOLVE_MPVAPI(mpv_request_log_messages)
        WWX190_RESOLVE_MPVAPI(mpv_wait_event)
        WWX190_RESOLVE_MPVAPI(mpv_wakeup)
        WWX190_RESOLVE_MPVAPI(mpv_set_wakeup_callback)
        WWX190_RESOLVE_MPVAPI(mpv_wait_async_requests)
        WWX190_RESOLVE_MPVAPI(mpv_hook_add)
        WWX190_RESOLVE_MPVAPI(mpv_hook_continue)

        // render.h
        WWX190_RESOLVE_MPVAPI(mpv_render_context_create)
        WWX190_RESOLVE_MPVAPI(mpv_render_context_set_parameter)
        WWX190_RESOLVE_MPVAPI(mpv_render_context_get_info)
        WWX190_RESOLVE_MPVAPI(mpv_render_context_set_update_callback)
        WWX190_RESOLVE_MPVAPI(mpv_render_context_update)
        WWX190_RESOLVE_MPVAPI(mpv_render_context_render)
        WWX190_RESOLVE_MPVAPI(mpv_render_context_report_swap)
        WWX190_RESOLVE_MPVAPI(mpv_render_context_free)

        qCDebug(lcQMPMPV) << "libmpv loaded successfully.";
        return true;
    }

    [[nodiscard]] bool unload()
    {
        // client.h
        WWX190_SETNULL_MPVAPI(mpv_client_api_version)
        WWX190_SETNULL_MPVAPI(mpv_error_string)
        WWX190_SETNULL_MPVAPI(mpv_free)
        WWX190_SETNULL_MPVAPI(mpv_client_name)
        WWX190_SETNULL_MPVAPI(mpv_client_id)
        WWX190_SETNULL_MPVAPI(mpv_create)
        WWX190_SETNULL_MPVAPI(mpv_initialize)
        WWX190_SETNULL_MPVAPI(mpv_destroy)
        WWX190_SETNULL_MPVAPI(mpv_terminate_destroy)
        WWX190_SETNULL_MPVAPI(mpv_create_client)
        WWX190_SETNULL_MPVAPI(mpv_create_weak_client)
        WWX190_SETNULL_MPVAPI(mpv_load_config_file)
        WWX190_SETNULL_MPVAPI(mpv_get_time_us)
        WWX190_SETNULL_MPVAPI(mpv_free_node_contents)
        WWX190_SETNULL_MPVAPI(mpv_set_option)
        WWX190_SETNULL_MPVAPI(mpv_set_option_string)
        WWX190_SETNULL_MPVAPI(mpv_command)
        WWX190_SETNULL_MPVAPI(mpv_command_node)
        WWX190_SETNULL_MPVAPI(mpv_command_ret)
        WWX190_SETNULL_MPVAPI(mpv_command_string)
        WWX190_SETNULL_MPVAPI(mpv_command_async)
        WWX190_SETNULL_MPVAPI(mpv_command_node_async)
        WWX190_SETNULL_MPVAPI(mpv_abort_async_command)
        WWX190_SETNULL_MPVAPI(mpv_set_property)
        WWX190_SETNULL_MPVAPI(mpv_set_property_string)
        WWX190_SETNULL_MPVAPI(mpv_set_property_async)
        WWX190_SETNULL_MPVAPI(mpv_get_property)
        WWX190_SETNULL_MPVAPI(mpv_get_property_string)
        WWX190_SETNULL_MPVAPI(mpv_get_property_osd_string)
        WWX190_SETNULL_MPVAPI(mpv_get_property_async)
        WWX190_SETNULL_MPVAPI(mpv_observe_property)
        WWX190_SETNULL_MPVAPI(mpv_unobserve_property)
        WWX190_SETNULL_MPVAPI(mpv_event_name)
        WWX190_SETNULL_MPVAPI(mpv_event_to_node)
        WWX190_SETNULL_MPVAPI(mpv_request_event)
        WWX190_SETNULL_MPVAPI(mpv_request_log_messages)
        WWX190_SETNULL_MPVAPI(mpv_wait_event)
        WWX190_SETNULL_MPVAPI(mpv_wakeup)
        WWX190_SETNULL_MPVAPI(mpv_set_wakeup_callback)
        WWX190_SETNULL_MPVAPI(mpv_wait_async_requests)
        WWX190_SETNULL_MPVAPI(mpv_hook_add)
        WWX190_SETNULL_MPVAPI(mpv_hook_continue)

        // render.h
        WWX190_SETNULL_MPVAPI(mpv_render_context_create)
        WWX190_SETNULL_MPVAPI(mpv_render_context_set_parameter)
        WWX190_SETNULL_MPVAPI(mpv_render_context_get_info)
        WWX190_SETNULL_MPVAPI(mpv_render_context_set_update_callback)
        WWX190_SETNULL_MPVAPI(mpv_render_context_update)
        WWX190_SETNULL_MPVAPI(mpv_render_context_render)
        WWX190_SETNULL_MPVAPI(mpv_render_context_report_swap)
        WWX190_SETNULL_MPVAPI(mpv_render_context_free)

        if (library.isLoaded()) {
            if (!library.unload()) {
                qCWarning(lcQMPMPV) << "Failed to unload libmpv:" << library.errorString();
                return false;
            }
        }

        qCDebug(lcQMPMPV) << "libmpv unloaded successfully.";
        return true;
    }

    [[nodiscard]] bool isLoaded() const
    {
        const bool result =
                // client.h
                WWX190_NOTNULL_MPVAPI(mpv_client_api_version) &&
                WWX190_NOTNULL_MPVAPI(mpv_error_string) &&
                WWX190_NOTNULL_MPVAPI(mpv_free) &&
                WWX190_NOTNULL_MPVAPI(mpv_client_name) &&
                WWX190_NOTNULL_MPVAPI(mpv_client_id) &&
                WWX190_NOTNULL_MPVAPI(mpv_create) &&
                WWX190_NOTNULL_MPVAPI(mpv_initialize) &&
                WWX190_NOTNULL_MPVAPI(mpv_destroy) &&
                WWX190_NOTNULL_MPVAPI(mpv_terminate_destroy) &&
                WWX190_NOTNULL_MPVAPI(mpv_create_client) &&
                WWX190_NOTNULL_MPVAPI(mpv_create_weak_client) &&
                WWX190_NOTNULL_MPVAPI(mpv_load_config_file) &&
                WWX190_NOTNULL_MPVAPI(mpv_get_time_us) &&
                WWX190_NOTNULL_MPVAPI(mpv_free_node_contents) &&
                WWX190_NOTNULL_MPVAPI(mpv_set_option) &&
                WWX190_NOTNULL_MPVAPI(mpv_set_option_string) &&
                WWX190_NOTNULL_MPVAPI(mpv_command) &&
                WWX190_NOTNULL_MPVAPI(mpv_command_node) &&
                WWX190_NOTNULL_MPVAPI(mpv_command_ret) &&
                WWX190_NOTNULL_MPVAPI(mpv_command_string) &&
                WWX190_NOTNULL_MPVAPI(mpv_command_async) &&
                WWX190_NOTNULL_MPVAPI(mpv_command_node_async) &&
                WWX190_NOTNULL_MPVAPI(mpv_abort_async_command) &&
                WWX190_NOTNULL_MPVAPI(mpv_set_property) &&
                WWX190_NOTNULL_MPVAPI(mpv_set_property_string) &&
                WWX190_NOTNULL_MPVAPI(mpv_set_property_async) &&
                WWX190_NOTNULL_MPVAPI(mpv_get_property) &&
                WWX190_NOTNULL_MPVAPI(mpv_get_property_string) &&
                WWX190_NOTNULL_MPVAPI(mpv_get_property_osd_string) &&
                WWX190_NOTNULL_MPVAPI(mpv_get_property_async) &&
                WWX190_NOTNULL_MPVAPI(mpv_observe_property) &&
                WWX190_NOTNULL_MPVAPI(mpv_unobserve_property) &&
                WWX190_NOTNULL_MPVAPI(mpv_event_name) &&
                WWX190_NOTNULL_MPVAPI(mpv_event_to_node) &&
                WWX190_NOTNULL_MPVAPI(mpv_request_event) &&
                WWX190_NOTNULL_MPVAPI(mpv_request_log_messages) &&
                WWX190_NOTNULL_MPVAPI(mpv_wait_event) &&
                WWX190_NOTNULL_MPVAPI(mpv_wakeup) &&
                WWX190_NOTNULL_MPVAPI(mpv_set_wakeup_callback) &&
                WWX190_NOTNULL_MPVAPI(mpv_wait_async_requests) &&
                WWX190_NOTNULL_MPVAPI(mpv_hook_add) &&
                WWX190_NOTNULL_MPVAPI(mpv_hook_continue) &&
                // render.h
                WWX190_NOTNULL_MPVAPI(mpv_render_context_create) &&
                WWX190_NOTNULL_MPVAPI(mpv_render_context_set_parameter) &&
                WWX190_NOTNULL_MPVAPI(mpv_render_context_get_info) &&
                WWX190_NOTNULL_MPVAPI(mpv_render_context_set_update_callback) &&
                WWX190_NOTNULL_MPVAPI(mpv_render_context_update) &&
                WWX190_NOTNULL_MPVAPI(mpv_render_context_render) &&
                WWX190_NOTNULL_MPVAPI(mpv_render_context_report_swap) &&
                WWX190_NOTNULL_MPVAPI(mpv_render_context_free);
        return result;
    }

private:
    QLibrary library;
};

Q_GLOBAL_STATIC(MPVData, mpvData)

bool isLibmpvAvailable()
{
    return mpvData()->isLoaded();
}

QString getLibmpvVersion()
{
    const auto fullVerNum = mpv_client_api_version();
    const auto majorVerNum = (fullVerNum >> 16) & 0xff;
    const auto minorVerNum = fullVerNum & 0xff;
    return QStringLiteral("%1.%2.0").arg(QString::number(majorVerNum), QString::number(minorVerNum));
}

static inline QVariant node_to_variant(const mpv_node *node)
{
    Q_ASSERT(node);
    if (!node) {
        return {};
    }
    switch (node->format) {
    case MPV_FORMAT_STRING:
        return QString::fromUtf8(node->u.string);
    case MPV_FORMAT_FLAG:
        return static_cast<bool>(node->u.flag);
    case MPV_FORMAT_INT64:
        return static_cast<qint64>(node->u.int64);
    case MPV_FORMAT_DOUBLE:
        return static_cast<qreal>(node->u.double_);
    case MPV_FORMAT_NODE_ARRAY: {
        const mpv_node_list *list = node->u.list;
        QVariantList qlist = {};
        for (int n = 0; n != list->num; ++n) {
            qlist.append(node_to_variant(&list->values[n]));
        }
        return qlist;
    }
    case MPV_FORMAT_NODE_MAP: {
        const mpv_node_list *map = node->u.list;
        QVariantMap qmap = {};
        for (int n = 0; n != map->num; ++n) {
            qmap.insert(QString::fromUtf8(map->keys[n]), node_to_variant(&map->values[n]));
        }
        return qmap;
    }
    default:
        break;
    }
    return {};
}

struct node_builder
{
    explicit node_builder(const QVariant &v)
    {
        set(&node_, v);
    }

    ~node_builder()
    {
        free_node(&node_);
    }

    mpv_node *node()
    {
        return &node_;
    }

private:
    Q_DISABLE_COPY_MOVE(node_builder)

    mpv_node node_;

    mpv_node_list *create_list(mpv_node *dst, bool is_map, int num)
    {
        dst->format = is_map ? MPV_FORMAT_NODE_MAP : MPV_FORMAT_NODE_ARRAY;
        auto *list = new mpv_node_list();
        dst->u.list = list;
        if (list == nullptr) {
            goto err;
        }
        list->values = new mpv_node[num]();
        if (list->values == nullptr) {
            goto err;
        }
        if (is_map) {
            list->keys = new char *[num]();
            if (list->keys == nullptr) {
                goto err;
            }
        }
        return list;
    err:
        free_node(dst);
        return nullptr;
    }
    char *dup_qstring(const QString &s)
    {
        QByteArray b = s.toUtf8();
        char *r = new char[b.size() + 1];
        if (r != nullptr) {
            std::memcpy(r, b.data(), b.size() + 1);
        }
        return r;
    }
    bool test_type(const QVariant &v, QMetaType::Type t)
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        return static_cast<QMetaType::Type>(v.typeId()) == t;
#else
        return static_cast<QMetaType::Type>(v.type()) == t;
#endif
    }
    void set(mpv_node *dst, const QVariant &src)
    {
        if (test_type(src, QMetaType::QString)) {
            dst->format = MPV_FORMAT_STRING;
            dst->u.string = dup_qstring(src.toString());
            if (dst->u.string == nullptr) {
                goto fail;
            }
        } else if (test_type(src, QMetaType::Bool)) {
            dst->format = MPV_FORMAT_FLAG;
            dst->u.flag = src.toBool() ? 1 : 0;
        } else if (test_type(src, QMetaType::Int) || test_type(src, QMetaType::LongLong)
                   || test_type(src, QMetaType::UInt) || test_type(src, QMetaType::ULongLong)) {
            dst->format = MPV_FORMAT_INT64;
            dst->u.int64 = src.toLongLong();
        } else if (test_type(src, QMetaType::Double) || test_type(src, QMetaType::Float)) {
            dst->format = MPV_FORMAT_DOUBLE;
            dst->u.double_ = src.toReal();
        } else if (src.canConvert<QVariantList>()) {
            QVariantList qlist = src.toList();
            mpv_node_list *list = create_list(dst, false, qlist.size());
            if (list == nullptr) {
                goto fail;
            }
            list->num = qlist.size();
            for (int n = 0; n < qlist.size(); n++) {
                set(&list->values[n], qlist[n]);
            }
        } else if (src.canConvert<QVariantMap>()) {
            QVariantMap qmap = src.toMap();
            mpv_node_list *list = create_list(dst, true, qmap.size());
            if (list == nullptr) {
                goto fail;
            }
            list->num = qmap.size();
            for (int n = 0; n < qmap.size(); n++) {
                list->keys[n] = dup_qstring(qmap.keys()[n]);
                if (list->keys[n] == nullptr) {
                    free_node(dst);
                    goto fail;
                }
                set(&list->values[n], qmap.values()[n]);
            }
        } else {
            goto fail;
        }
        return;
    fail:
        dst->format = MPV_FORMAT_NONE;
    }
    void free_node(mpv_node *dst)
    {
        switch (dst->format) {
        case MPV_FORMAT_STRING:
            delete[] dst->u.string;
            break;
        case MPV_FORMAT_NODE_ARRAY:
        case MPV_FORMAT_NODE_MAP: {
            mpv_node_list *list = dst->u.list;
            if (list != nullptr) {
                for (int n = 0; n < list->num; n++) {
                    if (list->keys != nullptr) {
                        delete[] list->keys[n];
                    }
                    if (list->values != nullptr) {
                        free_node(&list->values[n]);
                    }
                }
                delete[] list->keys;
                delete[] list->values;
            }
            delete list;
            break;
        }
        default:
            break;
        }
        dst->format = MPV_FORMAT_NONE;
    }
};

/**
 * RAII wrapper that calls mpv_free_node_contents() on the pointer.
 */
struct node_autofree
{
    mpv_node *ptr = nullptr;

    explicit node_autofree(mpv_node *a_ptr)
    {
        ptr = a_ptr;
    }

    ~node_autofree()
    {
        mpv_free_node_contents(ptr);
    }
};

int get_error(const QVariant &v)
{
    if (!v.canConvert<ErrorReturn>()) {
        return 0;
    }
    return qvariant_cast<ErrorReturn>(v).errorCode;
}

/**
 * Return whether the QVariant carries a mpv error code.
 */
bool is_error(const QVariant &v)
{
    return (get_error(v) < 0);
}

/**
 * Return the given property as mpv_node converted to QVariant, or QVariant()
 * on error.
 *
 * @param name the property name
 * @return the property value, or an ErrorReturn with the error code
 */
QVariant get_property(mpv_handle *ctx, const QString &name)
{
    Q_ASSERT(ctx);
    Q_ASSERT(!name.isEmpty());
    if (!ctx || name.isEmpty()) {
        return {};
    }
    mpv_node node;
    const int err = mpv_get_property(ctx, qUtf8Printable(name), MPV_FORMAT_NODE, &node);
    if (err < 0) {
        return QVariant::fromValue(ErrorReturn(err));
    }
    node_autofree f(&node);
    return node_to_variant(&node);
}

/**
 * Set the given property as mpv_node converted from the QVariant argument.
 *
 * @return mpv error code (<0 on error, >= 0 on success)
 */
int set_property(mpv_handle *ctx, const QString &name, const QVariant &v)
{
    Q_ASSERT(ctx);
    Q_ASSERT(!name.isEmpty());
    Q_ASSERT(v.isValid());
    if (!ctx || name.isEmpty() || !v.isValid()) {
        return -1;
    }
    node_builder node(v);
    return mpv_set_property(ctx, qUtf8Printable(name), MPV_FORMAT_NODE, node.node());
}

/**
 * Set the given property asynchronously as mpv_node converted from the QVariant
 * argument.
 *
 * @return mpv error code (<0 on error, >= 0 on success)
 */
int set_property_async(mpv_handle *ctx, const QString &name, const QVariant &v, quint64 reply_userdata)
{
    Q_ASSERT(ctx);
    Q_ASSERT(!name.isEmpty());
    Q_ASSERT(v.isValid());
    if (!ctx || name.isEmpty() || !v.isValid()) {
        return -1;
    }
    node_builder node(v);
    return mpv_set_property_async(ctx, reply_userdata, qUtf8Printable(name), MPV_FORMAT_NODE, node.node());
}

/**
 * mpv_command_node() equivalent.
 *
 * @param args command arguments, with args[0] being the command name as string
 * @return the property value, or an ErrorReturn with the error code
 */
QVariant command(mpv_handle *ctx, const QVariant &args)
{
    Q_ASSERT(ctx);
    Q_ASSERT(args.isValid());
    if (!ctx || !args.isValid()) {
        return {};
    }
    node_builder node(args);
    mpv_node res;
    const int err = mpv_command_node(ctx, node.node(), &res);
    if (err < 0) {
        return QVariant::fromValue(ErrorReturn(err));
    }
    node_autofree f(&res);
    return node_to_variant(&res);
}

/**
 * Send commands to mpv asynchronously.
 *
 * @param args command arguments, with args[0] being the command name as string
 * @return mpv error code (<0 on error, >= 0 on success)
 */
int command_async(mpv_handle *ctx, const QVariant &args, quint64 reply_userdata)
{
    Q_ASSERT(ctx);
    Q_ASSERT(args.isValid());
    if (!ctx || !args.isValid()) {
        return -1;
    }
    node_builder node(args);
    return mpv_command_node_async(ctx, reply_userdata, node.node());
}

} // namespace MPV::Qt

////////////////////////////////////////////////////
/////         libmpv
///////////////////////////////////////////////////

// client.h

unsigned long mpv_client_api_version()
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_client_api_version, MPV_CLIENT_API_VERSION);
}

const char *mpv_error_string(int error)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_error_string, nullptr, error);
}

void mpv_free(void *data)
{
    WWX190_CALL_MPVAPI(mpv_free, data)
}

const char *mpv_client_name(mpv_handle *ctx)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_client_name, nullptr, ctx);
}

qint64 mpv_client_id(mpv_handle *ctx)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_client_id, -1, ctx);
}

mpv_handle *mpv_create()
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_create, nullptr);
}

int mpv_initialize(mpv_handle *ctx)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_initialize, -1, ctx);
}

void mpv_destroy(mpv_handle *ctx)
{
    WWX190_CALL_MPVAPI(mpv_destroy, ctx)
}

void mpv_terminate_destroy(mpv_handle *ctx)
{
    WWX190_CALL_MPVAPI(mpv_terminate_destroy, ctx)
}

mpv_handle *mpv_create_client(mpv_handle *ctx, const char *name)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_create_client, nullptr, ctx, name);
}

mpv_handle *mpv_create_weak_client(mpv_handle *ctx, const char *name)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_create_weak_client, nullptr, ctx, name);
}

int mpv_load_config_file(mpv_handle *ctx, const char *filename)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_load_config_file, -1, ctx, filename);
}

qint64 mpv_get_time_us(mpv_handle *ctx)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_get_time_us, -1, ctx);
}

void mpv_free_node_contents(mpv_node *node)
{
    WWX190_CALL_MPVAPI(mpv_free_node_contents, node)
}

int mpv_set_option(mpv_handle *ctx, const char *name, mpv_format format, void *data)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_set_option, -1, ctx, name, format, data);
}

int mpv_set_option_string(mpv_handle *ctx, const char *name, const char *data)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_set_option_string, -1, ctx, name, data);
}

int mpv_command(mpv_handle *ctx, const char **args)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_command, -1, ctx, args);
}

int mpv_command_node(mpv_handle *ctx, mpv_node *args, mpv_node *result)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_command_node, -1, ctx, args, result);
}

int mpv_command_ret(mpv_handle *ctx, const char **args, mpv_node *result)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_command_ret, -1, ctx, args, result);
}

int mpv_command_string(mpv_handle *ctx, const char *args)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_command_string, -1, ctx, args);
}

int mpv_command_async(mpv_handle *ctx, quint64 reply_userdata, const char **args)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_command_async, -1, ctx, reply_userdata, args);
}

int mpv_command_node_async(mpv_handle *ctx, quint64 reply_userdata, mpv_node *args)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_command_node_async, -1, ctx, reply_userdata, args);
}

void mpv_abort_async_command(mpv_handle *ctx, quint64 reply_userdata)
{
    WWX190_CALL_MPVAPI(mpv_abort_async_command, ctx, reply_userdata)
}

int mpv_set_property(mpv_handle *ctx, const char *name, mpv_format format, void *data)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_set_property, -1, ctx, name, format, data);
}

int mpv_set_property_string(mpv_handle *ctx, const char *name, const char *data)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_set_property_string, -1, ctx, name, data);
}

int mpv_set_property_async(mpv_handle *ctx, quint64 reply_userdata, const char *name, mpv_format format, void *data)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_set_property_async, -1, ctx, reply_userdata, name, format, data);
}

int mpv_get_property(mpv_handle *ctx, const char *name, mpv_format format, void *data)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_get_property, -1, ctx, name, format, data);
}

char *mpv_get_property_string(mpv_handle *ctx, const char *name)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_get_property_string, nullptr, ctx, name);
}

char *mpv_get_property_osd_string(mpv_handle *ctx, const char *name)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_get_property_osd_string, nullptr, ctx, name);
}

int mpv_get_property_async(mpv_handle *ctx, quint64 reply_userdata, const char *name, mpv_format format)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_get_property_async, -1, ctx, reply_userdata, name, format);
}

int mpv_observe_property(mpv_handle *ctx, quint64 reply_userdata, const char *name, mpv_format format)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_observe_property, -1, ctx, reply_userdata, name, format);
}

int mpv_unobserve_property(mpv_handle *ctx, quint64 registered_reply_userdata)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_unobserve_property, -1, ctx, registered_reply_userdata);
}

const char *mpv_event_name(mpv_event_id event)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_event_name, nullptr, event);
}

int mpv_event_to_node(mpv_node *dst, mpv_event *src)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_event_to_node, -1, dst, src);
}

int mpv_request_event(mpv_handle *ctx, mpv_event_id event, int enable)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_request_event, -1, ctx, event, enable);
}

int mpv_request_log_messages(mpv_handle *ctx, const char *min_level)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_request_log_messages, -1, ctx, min_level);
}

mpv_event *mpv_wait_event(mpv_handle *ctx, qreal timeout)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_wait_event, nullptr, ctx, timeout);
}

void mpv_wakeup(mpv_handle *ctx)
{
    WWX190_CALL_MPVAPI(mpv_wakeup, ctx)
}

void mpv_set_wakeup_callback(mpv_handle *ctx, void (*cb)(void *d), void *d)
{
    WWX190_CALL_MPVAPI(mpv_set_wakeup_callback, ctx, cb, d)
}

void mpv_wait_async_requests(mpv_handle *ctx)
{
    WWX190_CALL_MPVAPI(mpv_wait_async_requests, ctx)
}

int mpv_hook_add(mpv_handle *ctx, quint64 reply_userdata, const char *name, int priority)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_hook_add, -1, ctx, reply_userdata, name, priority);
}

int mpv_hook_continue(mpv_handle *ctx, quint64 id)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_hook_continue, -1, ctx, id);
}

// render.h

int mpv_render_context_create(mpv_render_context **res, mpv_handle *ctx, mpv_render_param *params)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_render_context_create, -1, res, ctx, params);
}

int mpv_render_context_set_parameter(mpv_render_context *ctx, mpv_render_param param)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_render_context_set_parameter, -1, ctx, param);
}

int mpv_render_context_get_info(mpv_render_context *ctx, mpv_render_param param)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_render_context_get_info, -1, ctx, param);
}

void mpv_render_context_set_update_callback(mpv_render_context *ctx, mpv_render_update_fn callback, void *callback_ctx)
{
    WWX190_CALL_MPVAPI(mpv_render_context_set_update_callback, ctx, callback, callback_ctx)
}

quint64 mpv_render_context_update(mpv_render_context *ctx)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_render_context_update, 0, ctx);
}

int mpv_render_context_render(mpv_render_context *ctx, mpv_render_param *params)
{
    return WWX190_CALL_MPVAPI_RETURN(mpv_render_context_render, -1, ctx, params);
}

void mpv_render_context_report_swap(mpv_render_context *ctx)
{
    WWX190_CALL_MPVAPI(mpv_render_context_report_swap, ctx)
}

void mpv_render_context_free(mpv_render_context *ctx)
{
    WWX190_CALL_MPVAPI(mpv_render_context_free, ctx)
}
