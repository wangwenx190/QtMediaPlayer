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

#include "vlcqthelper.h"
#include "include/vlc/vlc.h"
#include <QtCore/qdebug.h>
#include <QtCore/qlibrary.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>

#ifndef WWX190_GENERATE_VLCAPI
#define WWX190_GENERATE_VLCAPI(funcName, resultType, ...) \
    using _WWX190_VLCAPI_lp_##funcName = resultType(*)(__VA_ARGS__); \
    _WWX190_VLCAPI_lp_##funcName m_lp_##funcName = nullptr;
#endif

#ifndef WWX190_RESOLVE_VLCAPI
#define WWX190_RESOLVE_VLCAPI(funcName) \
    if (!m_lp_##funcName) { \
        qCDebug(QTMEDIAPLAYER_PREPEND_NAMESPACE(lcQMPVLC)) << "Resolving function:" << #funcName; \
        m_lp_##funcName = reinterpret_cast<_WWX190_VLCAPI_lp_##funcName>(library.resolve(#funcName)); \
        Q_ASSERT(m_lp_##funcName); \
        if (!m_lp_##funcName) { \
            qCWarning(QTMEDIAPLAYER_PREPEND_NAMESPACE(lcQMPVLC)) << "Failed to resolve function" << #funcName; \
        } \
    }
#endif

#ifndef WWX190_SETNULL_VLCAPI
#define WWX190_SETNULL_VLCAPI(funcName) \
    if (m_lp_##funcName) { \
        m_lp_##funcName = nullptr; \
    }
#endif

#ifndef WWX190_NOTNULL_VLCAPI
#define WWX190_NOTNULL_VLCAPI(funcName) (m_lp_##funcName != nullptr)
#endif

#ifndef WWX190_CALL_VLCAPI
#define WWX190_CALL_VLCAPI(funcName, ...) \
    if (VLC::Qt::vlcData()->m_lp_##funcName) { \
        VLC::Qt::vlcData()->m_lp_##funcName(__VA_ARGS__); \
    }
#endif

#ifndef WWX190_CALL_VLCAPI_RETURN
#define WWX190_CALL_VLCAPI_RETURN(funcName, defRet, ...) \
    (VLC::Qt::vlcData()->m_lp_##funcName ? VLC::Qt::vlcData()->m_lp_##funcName(__VA_ARGS__) : defRet)
#endif

namespace VLC::Qt
{

static const char _vlcHelper_libvlc_fileName_envVar[] = "_WWX190_VLCPLAYER_LIBVLC_FILENAME";

struct VLCData
{
public:
    // libvlc.h
    WWX190_GENERATE_VLCAPI(libvlc_errmsg, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_clearerr, void)
    WWX190_GENERATE_VLCAPI(libvlc_vprinterr, const char *, const char *, va_list)
    WWX190_GENERATE_VLCAPI(libvlc_printerr, const char *, const char *, va_list)
    WWX190_GENERATE_VLCAPI(libvlc_new, libvlc_instance_t *, int, const char *const *)
    WWX190_GENERATE_VLCAPI(libvlc_release, void, libvlc_instance_t *)
    WWX190_GENERATE_VLCAPI(libvlc_retain, void, libvlc_instance_t *)
    WWX190_GENERATE_VLCAPI(libvlc_add_intf, int, libvlc_instance_t *, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_set_exit_handler, void, libvlc_instance_t *, void(*)(void *), void *)
    WWX190_GENERATE_VLCAPI(libvlc_set_user_agent, void, libvlc_instance_t *, const char *, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_set_app_id, void, libvlc_instance_t *, const char *, const char *, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_get_version, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_get_compiler, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_get_changeset, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_free, void, void *)
    WWX190_GENERATE_VLCAPI(libvlc_event_attach, int, libvlc_event_manager_t *, libvlc_event_type_t, libvlc_callback_t, void *)
    WWX190_GENERATE_VLCAPI(libvlc_event_detach, void, libvlc_event_manager_t *, libvlc_event_type_t, libvlc_callback_t, void *)
    WWX190_GENERATE_VLCAPI(libvlc_log_get_context, void, const libvlc_log_t *, const char **, const char **, unsigned *)
    WWX190_GENERATE_VLCAPI(libvlc_log_get_object, void, const libvlc_log_t *, const char **, const char **, uintptr_t *)
    WWX190_GENERATE_VLCAPI(libvlc_log_unset, void, libvlc_instance_t *)
    WWX190_GENERATE_VLCAPI(libvlc_log_set, void, libvlc_instance_t *, libvlc_log_cb, void *)
    WWX190_GENERATE_VLCAPI(libvlc_log_set_file, void, libvlc_instance_t *, FILE *)
    WWX190_GENERATE_VLCAPI(libvlc_module_description_list_release, void, libvlc_module_description_t *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_filter_list_get, libvlc_module_description_t *, libvlc_instance_t *)
    WWX190_GENERATE_VLCAPI(libvlc_video_filter_list_get, libvlc_module_description_t *, libvlc_instance_t *)
    WWX190_GENERATE_VLCAPI(libvlc_clock, int64_t)

    // libvlc_renderer_discoverer.h
    WWX190_GENERATE_VLCAPI(libvlc_renderer_item_hold, libvlc_renderer_item_t *, libvlc_renderer_item_t *)
    WWX190_GENERATE_VLCAPI(libvlc_renderer_item_release, void, libvlc_renderer_item_t *)
    WWX190_GENERATE_VLCAPI(libvlc_renderer_item_name, const char *, const libvlc_renderer_item_t *)
    WWX190_GENERATE_VLCAPI(libvlc_renderer_item_type, const char *, const libvlc_renderer_item_t *)
    WWX190_GENERATE_VLCAPI(libvlc_renderer_item_icon_uri, const char *, const libvlc_renderer_item_t *)
    WWX190_GENERATE_VLCAPI(libvlc_renderer_item_flags, int, const libvlc_renderer_item_t *)
    WWX190_GENERATE_VLCAPI(libvlc_renderer_discoverer_new, libvlc_renderer_discoverer_t *, libvlc_instance_t *, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_renderer_discoverer_release, void, libvlc_renderer_discoverer_t *)
    WWX190_GENERATE_VLCAPI(libvlc_renderer_discoverer_start, int, libvlc_renderer_discoverer_t *)
    WWX190_GENERATE_VLCAPI(libvlc_renderer_discoverer_stop, void, libvlc_renderer_discoverer_t *)
    WWX190_GENERATE_VLCAPI(libvlc_renderer_discoverer_event_manager, libvlc_event_manager_t *, libvlc_renderer_discoverer_t *)
    WWX190_GENERATE_VLCAPI(libvlc_renderer_discoverer_list_get, size_t, libvlc_instance_t *, libvlc_rd_description_t ***)
    WWX190_GENERATE_VLCAPI(libvlc_renderer_discoverer_list_release, void, libvlc_rd_description_t **, size_t)

    // libvlc_picture.h
    WWX190_GENERATE_VLCAPI(libvlc_picture_retain, void, libvlc_picture_t *)
    WWX190_GENERATE_VLCAPI(libvlc_picture_release, void, libvlc_picture_t *)
    WWX190_GENERATE_VLCAPI(libvlc_picture_save, int, const libvlc_picture_t *, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_picture_get_buffer, const unsigned char *, const libvlc_picture_t *, size_t *)
    WWX190_GENERATE_VLCAPI(libvlc_picture_type, libvlc_picture_type_t, const libvlc_picture_t *)
    WWX190_GENERATE_VLCAPI(libvlc_picture_get_stride, unsigned int, const libvlc_picture_t *)
    WWX190_GENERATE_VLCAPI(libvlc_picture_get_width, unsigned int, const libvlc_picture_t *)
    WWX190_GENERATE_VLCAPI(libvlc_picture_get_height, unsigned int, const libvlc_picture_t *)
    WWX190_GENERATE_VLCAPI(libvlc_picture_get_time, libvlc_time_t, const libvlc_picture_t *)
    WWX190_GENERATE_VLCAPI(libvlc_picture_list_count, size_t, const libvlc_picture_list_t *)
    WWX190_GENERATE_VLCAPI(libvlc_picture_list_at, libvlc_picture_t *, const libvlc_picture_list_t *, size_t)
    WWX190_GENERATE_VLCAPI(libvlc_picture_list_destroy, void, libvlc_picture_list_t *)

    // libvlc_media.h
    WWX190_GENERATE_VLCAPI(libvlc_media_new_location, libvlc_media_t *, libvlc_instance_t *, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_media_new_path, libvlc_media_t *, libvlc_instance_t *, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_media_new_fd, libvlc_media_t *, libvlc_instance_t *, int)
    WWX190_GENERATE_VLCAPI(libvlc_media_new_callbacks, libvlc_media_t *, libvlc_instance_t *, libvlc_media_open_cb, libvlc_media_read_cb, libvlc_media_seek_cb, libvlc_media_close_cb, void *)
    WWX190_GENERATE_VLCAPI(libvlc_media_new_as_node, libvlc_media_t *, libvlc_instance_t *, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_media_add_option, void, libvlc_media_t *, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_media_add_option_flag, void, libvlc_media_t *, const char *, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_media_retain, void, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_release, void, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_get_mrl, char *, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_duplicate, libvlc_media_t *, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_get_meta, char *, libvlc_media_t *, libvlc_meta_t)
    WWX190_GENERATE_VLCAPI(libvlc_media_set_meta, void, libvlc_media_t *, libvlc_meta_t, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_media_save_meta, int, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_get_state, libvlc_state_t, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_get_stats, bool, libvlc_media_t *, libvlc_media_stats_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_subitems, libvlc_media_list_t *, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_event_manager, libvlc_event_manager_t *, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_get_duration, libvlc_time_t, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_parse_with_options, int, libvlc_media_t *, libvlc_media_parse_flag_t, int)
    WWX190_GENERATE_VLCAPI(libvlc_media_parse_stop, void, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_get_parsed_status, libvlc_media_parsed_status_t, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_set_user_data, void, libvlc_media_t *, void *)
    WWX190_GENERATE_VLCAPI(libvlc_media_get_user_data, void *, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_get_tracklist, libvlc_media_tracklist_t *, libvlc_media_t *, libvlc_track_type_t)
    WWX190_GENERATE_VLCAPI(libvlc_media_get_codec_description, const char *, libvlc_track_type_t, uint32_t)
    WWX190_GENERATE_VLCAPI(libvlc_media_get_type, libvlc_media_type_t, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_thumbnail_request_by_time, libvlc_media_thumbnail_request_t *, libvlc_media_t *, libvlc_time_t, libvlc_thumbnailer_seek_speed_t, unsigned int, unsigned int, bool, libvlc_picture_type_t, libvlc_time_t)
    WWX190_GENERATE_VLCAPI(libvlc_media_thumbnail_request_by_pos, libvlc_media_thumbnail_request_t *, libvlc_media_t *, float, libvlc_thumbnailer_seek_speed_t, unsigned int, unsigned int, bool, libvlc_picture_type_t, libvlc_time_t)
    WWX190_GENERATE_VLCAPI(libvlc_media_thumbnail_request_cancel, void, libvlc_media_thumbnail_request_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_thumbnail_request_destroy, void, libvlc_media_thumbnail_request_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_slaves_add, int, libvlc_media_t *, libvlc_media_slave_type_t, unsigned int, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_media_slaves_clear, void, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_slaves_get, unsigned int, libvlc_media_t *, libvlc_media_slave_t ***)
    WWX190_GENERATE_VLCAPI(libvlc_media_slaves_release, void, libvlc_media_slave_t **, unsigned int)

    // libvlc_media_player.h
    WWX190_GENERATE_VLCAPI(libvlc_media_player_new, libvlc_media_player_t *, libvlc_instance_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_new_from_media, libvlc_media_player_t *, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_release, void, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_retain, void, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_set_media, void, libvlc_media_player_t *, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_media, libvlc_media_t *, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_event_manager, libvlc_event_manager_t *, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_is_playing, bool, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_play, int, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_set_pause, void, libvlc_media_player_t *, int)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_pause, void, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_stop_async, int, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_set_renderer, int, libvlc_media_player_t *, libvlc_renderer_item_t *)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_callbacks, void, libvlc_media_player_t *, libvlc_video_lock_cb, libvlc_video_unlock_cb, libvlc_video_display_cb, void *)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_format, void, libvlc_media_player_t *, const char *, unsigned, unsigned, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_format_callbacks, void, libvlc_media_player_t *, libvlc_video_format_cb, libvlc_video_cleanup_cb)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_output_callbacks, bool, libvlc_media_player_t *, libvlc_video_engine_t, libvlc_video_output_setup_cb, libvlc_video_output_cleanup_cb, libvlc_video_output_set_resize_cb, libvlc_video_update_output_cb, libvlc_video_swap_cb, libvlc_video_makeCurrent_cb, libvlc_video_getProcAddress_cb, libvlc_video_frameMetadata_cb, libvlc_video_output_select_plane_cb, void *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_set_nsobject, void, libvlc_media_player_t *, void *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_nsobject, void *, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_set_xwindow, void, libvlc_media_player_t *, uint32_t)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_xwindow, uint32_t, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_set_hwnd, void, libvlc_media_player_t *, void *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_hwnd, void *, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_set_android_context, void, libvlc_media_player_t *, void *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_set_callbacks, void, libvlc_media_player_t *, libvlc_audio_play_cb, libvlc_audio_pause_cb, libvlc_audio_resume_cb, libvlc_audio_flush_cb, libvlc_audio_drain_cb, void *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_set_volume_callback, void, libvlc_media_player_t *, libvlc_audio_set_volume_cb)
    WWX190_GENERATE_VLCAPI(libvlc_audio_set_format_callbacks, void, libvlc_media_player_t *, libvlc_audio_setup_cb, libvlc_audio_cleanup_cb)
    WWX190_GENERATE_VLCAPI(libvlc_audio_set_format, void, libvlc_media_player_t *, const char *, unsigned, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_length, libvlc_time_t, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_time, libvlc_time_t, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_set_time, int, libvlc_media_player_t *, libvlc_time_t, bool)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_position, float, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_set_position, int, libvlc_media_player_t *, float, bool)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_set_chapter, void, libvlc_media_player_t *, int)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_chapter, int, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_chapter_count, int, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_chapter_count_for_title, int, libvlc_media_player_t *, int)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_set_title, void, libvlc_media_player_t *, int)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_title, int, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_title_count, int, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_previous_chapter, void, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_next_chapter, void, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_rate, float, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_set_rate, int, libvlc_media_player_t *, float)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_state, libvlc_state_t, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_has_vout, unsigned, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_is_seekable, bool, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_can_pause, bool, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_program_scrambled, bool, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_next_frame, void, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_navigate, void, libvlc_media_player_t *, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_set_video_title_display, void, libvlc_media_player_t *, libvlc_position_t, unsigned int)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_tracklist, libvlc_media_tracklist_t *, libvlc_media_player_t *, libvlc_track_type_t)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_selected_track, libvlc_media_track_t *, libvlc_media_player_t *, libvlc_track_type_t)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_track_from_id, libvlc_media_track_t *, libvlc_media_player_t *, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_select_track, void, libvlc_media_player_t *, const libvlc_media_track_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_unselect_track_type, void, libvlc_media_player_t *, libvlc_track_type_t)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_select_tracks, void, libvlc_media_player_t *, libvlc_track_type_t, const libvlc_media_track_t **, size_t)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_select_tracks_by_ids, void, libvlc_media_player_t *, libvlc_track_type_t, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_add_slave, int, libvlc_media_player_t *, libvlc_media_slave_type_t, const char *, bool)
    WWX190_GENERATE_VLCAPI(libvlc_player_program_delete, void, libvlc_player_program_t *)
    WWX190_GENERATE_VLCAPI(libvlc_player_programlist_count, size_t, const libvlc_player_programlist_t *)
    WWX190_GENERATE_VLCAPI(libvlc_player_programlist_at, libvlc_player_program_t *, libvlc_player_programlist_t *, size_t)
    WWX190_GENERATE_VLCAPI(libvlc_player_programlist_delete, void, libvlc_player_programlist_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_select_program_id, void, libvlc_media_player_t *, int)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_selected_program, libvlc_player_program_t *, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_program_from_id, libvlc_player_program_t *, libvlc_media_player_t *, int)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_programlist, libvlc_player_programlist_t *, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_toggle_fullscreen, void, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_set_fullscreen, void, libvlc_media_player_t *, bool)
    WWX190_GENERATE_VLCAPI(libvlc_get_fullscreen, bool, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_key_input, void, libvlc_media_player_t *, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_mouse_input, void, libvlc_media_player_t *, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_video_get_size, int, libvlc_media_player_t *, unsigned, unsigned *, unsigned *)
    WWX190_GENERATE_VLCAPI(libvlc_video_get_cursor, int, libvlc_media_player_t *, unsigned, int *, int *)
    WWX190_GENERATE_VLCAPI(libvlc_video_get_scale, float, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_scale, void, libvlc_media_player_t *, float)
    WWX190_GENERATE_VLCAPI(libvlc_video_get_aspect_ratio, char *, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_aspect_ratio, void, libvlc_media_player_t *, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_video_new_viewpoint, libvlc_video_viewpoint_t *)
    WWX190_GENERATE_VLCAPI(libvlc_video_update_viewpoint, int, libvlc_media_player_t *, const libvlc_video_viewpoint_t *, bool)
    WWX190_GENERATE_VLCAPI(libvlc_video_get_spu_delay, int64_t, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_video_get_spu_text_scale, float, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_spu_text_scale, void, libvlc_media_player_t *, float)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_spu_delay, int, libvlc_media_player_t *, int64_t)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_full_title_descriptions, int, libvlc_media_player_t *, libvlc_title_description_t ***)
    WWX190_GENERATE_VLCAPI(libvlc_title_descriptions_release, void, libvlc_title_description_t **, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_full_chapter_descriptions, int, libvlc_media_player_t *, int, libvlc_chapter_description_t ***)
    WWX190_GENERATE_VLCAPI(libvlc_chapter_descriptions_release, void, libvlc_chapter_description_t **, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_crop_ratio, void, libvlc_media_player_t *, unsigned, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_crop_window, void, libvlc_media_player_t *, unsigned, unsigned, unsigned, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_crop_border, void, libvlc_media_player_t *, unsigned, unsigned, unsigned, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_video_get_teletext, int, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_teletext, void, libvlc_media_player_t *, int)
    WWX190_GENERATE_VLCAPI(libvlc_video_take_snapshot, int, libvlc_media_player_t *, unsigned, const char *, unsigned int, unsigned int)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_deinterlace, void, libvlc_media_player_t *, int, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_video_get_marquee_int, int, libvlc_media_player_t *, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_marquee_int, void, libvlc_media_player_t *, unsigned, int)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_marquee_string, void, libvlc_media_player_t *, unsigned, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_video_get_logo_int, int, libvlc_media_player_t *, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_logo_int, void, libvlc_media_player_t *, unsigned, int)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_logo_string, void, libvlc_media_player_t *, unsigned, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_video_get_adjust_int, int, libvlc_media_player_t *, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_adjust_int, void, libvlc_media_player_t *, unsigned, int)
    WWX190_GENERATE_VLCAPI(libvlc_video_get_adjust_float, float, libvlc_media_player_t *, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_video_set_adjust_float, void, libvlc_media_player_t *, unsigned, float)
    WWX190_GENERATE_VLCAPI(libvlc_audio_output_list_get, libvlc_audio_output_t *, libvlc_instance_t *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_output_list_release, void, libvlc_audio_output_t *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_output_set, int, libvlc_media_player_t *, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_output_device_enum, libvlc_audio_output_device_t *, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_output_device_list_get, libvlc_audio_output_device_t *, libvlc_instance_t *, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_output_device_list_release, void, libvlc_audio_output_device_t *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_output_device_set, void, libvlc_media_player_t *, const char *, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_output_device_get, char *, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_toggle_mute, void, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_get_mute, int, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_set_mute, void, libvlc_media_player_t *, int)
    WWX190_GENERATE_VLCAPI(libvlc_audio_get_volume, int, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_set_volume, int, libvlc_media_player_t *, int)
    WWX190_GENERATE_VLCAPI(libvlc_audio_get_channel, int, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_set_channel, int, libvlc_media_player_t *, int)
    WWX190_GENERATE_VLCAPI(libvlc_audio_get_delay, int64_t, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_set_delay, int, libvlc_media_player_t *, int64_t)
    WWX190_GENERATE_VLCAPI(libvlc_audio_equalizer_get_preset_count, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_audio_equalizer_get_preset_name, const char *, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_audio_equalizer_get_band_count, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_audio_equalizer_get_band_frequency, float, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_audio_equalizer_new, libvlc_equalizer_t *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_equalizer_new_from_preset, libvlc_equalizer_t *, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_audio_equalizer_release, void, libvlc_equalizer_t *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_equalizer_set_preamp, int, libvlc_equalizer_t *, float)
    WWX190_GENERATE_VLCAPI(libvlc_audio_equalizer_get_preamp, float, libvlc_equalizer_t *)
    WWX190_GENERATE_VLCAPI(libvlc_audio_equalizer_set_amp_at_index, int, libvlc_equalizer_t *, float, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_audio_equalizer_get_amp_at_index, float, libvlc_equalizer_t *, unsigned)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_set_equalizer, int, libvlc_media_player_t *, libvlc_equalizer_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_get_role, int, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_player_set_role, int, libvlc_media_player_t *, unsigned)

    // libvlc_media_list.h
    WWX190_GENERATE_VLCAPI(libvlc_media_list_new, libvlc_media_list_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_release, void, libvlc_media_list_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_retain, void, libvlc_media_list_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_set_media, void, libvlc_media_list_t *, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_media, libvlc_media_t *, libvlc_media_list_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_add_media, int, libvlc_media_list_t *, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_insert_media, int, libvlc_media_list_t *, libvlc_media_t *, int)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_remove_index, int, libvlc_media_list_t *, int)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_count, int, libvlc_media_list_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_item_at_index, libvlc_media_t *, libvlc_media_list_t *, int)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_index_of_item, int, libvlc_media_list_t *, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_is_readonly, bool, libvlc_media_list_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_lock, void, libvlc_media_list_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_unlock, void, libvlc_media_list_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_event_manager, libvlc_event_manager_t *, libvlc_media_list_t *)

    // libvlc_media_list_player.h
    WWX190_GENERATE_VLCAPI(libvlc_media_list_player_new, libvlc_media_list_player_t *, libvlc_instance_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_player_release, void, libvlc_media_list_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_player_retain, void, libvlc_media_list_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_player_event_manager, libvlc_event_manager_t *, libvlc_media_list_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_player_set_media_player, void, libvlc_media_list_player_t *, libvlc_media_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_player_get_media_player, libvlc_media_player_t *, libvlc_media_list_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_player_set_media_list, void, libvlc_media_list_player_t *, libvlc_media_list_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_player_play, void, libvlc_media_list_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_player_pause, void, libvlc_media_list_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_player_set_pause, void, libvlc_media_list_player_t *, int)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_player_is_playing, bool, libvlc_media_list_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_player_get_state, libvlc_state_t, libvlc_media_list_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_player_play_item_at_index, int, libvlc_media_list_player_t *, int)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_player_play_item, int, libvlc_media_list_player_t *, libvlc_media_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_player_stop_async, void, libvlc_media_list_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_player_next, int, libvlc_media_list_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_player_previous, int, libvlc_media_list_player_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_list_player_set_playback_mode, void, libvlc_media_list_player_t *, libvlc_playback_mode_t)

    // libvlc_media_discoverer.h
    WWX190_GENERATE_VLCAPI(libvlc_media_discoverer_new, libvlc_media_discoverer_t *, libvlc_instance_t *, const char *)
    WWX190_GENERATE_VLCAPI(libvlc_media_discoverer_start, int, libvlc_media_discoverer_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_discoverer_stop, void, libvlc_media_discoverer_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_discoverer_release, void, libvlc_media_discoverer_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_discoverer_media_list, libvlc_media_list_t *, libvlc_media_discoverer_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_discoverer_is_running, bool, libvlc_media_discoverer_t *)
    WWX190_GENERATE_VLCAPI(libvlc_media_discoverer_list_get, size_t, libvlc_instance_t *, libvlc_media_discoverer_category_t, libvlc_media_discoverer_description_t ***)
    WWX190_GENERATE_VLCAPI(libvlc_media_discoverer_list_release, void, libvlc_media_discoverer_description_t **, size_t)

    // libvlc_dialog.h
    WWX190_GENERATE_VLCAPI(libvlc_dialog_set_callbacks, void, libvlc_instance_t *, const libvlc_dialog_cbs *, void *)
    WWX190_GENERATE_VLCAPI(libvlc_dialog_set_context, void, libvlc_dialog_id *, void *)
    WWX190_GENERATE_VLCAPI(libvlc_dialog_get_context, void *, libvlc_dialog_id *)
    WWX190_GENERATE_VLCAPI(libvlc_dialog_post_login, int, libvlc_dialog_id *, const char *, const char *, bool)
    WWX190_GENERATE_VLCAPI(libvlc_dialog_post_action, int, libvlc_dialog_id *, int)
    WWX190_GENERATE_VLCAPI(libvlc_dialog_dismiss, int, libvlc_dialog_id *)

    explicit VLCData()
    {
        const bool result = load(qEnvironmentVariable(_vlcHelper_libvlc_fileName_envVar, QStringLiteral("libvlc")));
        Q_UNUSED(result);
    }

    ~VLCData()
    {
        const bool result = unload();
        Q_UNUSED(result);
    }

    [[nodiscard]] bool load(const QString &path)
    {
        Q_ASSERT(!path.isEmpty());
        if (path.isEmpty()) {
            qCWarning(QTMEDIAPLAYER_PREPEND_NAMESPACE(lcQMPVLC)) << "Failed to load libvlc: empty library path.";
            return false;
        }

        if (isLoaded()) {
            qCDebug(QTMEDIAPLAYER_PREPEND_NAMESPACE(lcQMPVLC)) << "libvlc already loaded. Unloading ...";
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
            qCDebug(QTMEDIAPLAYER_PREPEND_NAMESPACE(lcQMPVLC)) << "Start loading libvlc from:" << QDir::toNativeSeparators(fi.canonicalFilePath());
        } else {
            qCDebug(QTMEDIAPLAYER_PREPEND_NAMESPACE(lcQMPVLC)) << "Start loading libvlc from:" << QDir::toNativeSeparators(path);
            qCWarning(QTMEDIAPLAYER_PREPEND_NAMESPACE(lcQMPVLC)) << "Failed to load libvlc:" << library.errorString();
            return false;
        }

        // libvlc.h
        WWX190_RESOLVE_VLCAPI(libvlc_errmsg)
        WWX190_RESOLVE_VLCAPI(libvlc_clearerr)
        WWX190_RESOLVE_VLCAPI(libvlc_vprinterr)
        WWX190_RESOLVE_VLCAPI(libvlc_printerr)
        WWX190_RESOLVE_VLCAPI(libvlc_new)
        WWX190_RESOLVE_VLCAPI(libvlc_release)
        WWX190_RESOLVE_VLCAPI(libvlc_retain)
        WWX190_RESOLVE_VLCAPI(libvlc_add_intf)
        WWX190_RESOLVE_VLCAPI(libvlc_set_exit_handler)
        WWX190_RESOLVE_VLCAPI(libvlc_set_user_agent)
        WWX190_RESOLVE_VLCAPI(libvlc_set_app_id)
        WWX190_RESOLVE_VLCAPI(libvlc_get_version)
        WWX190_RESOLVE_VLCAPI(libvlc_get_compiler)
        WWX190_RESOLVE_VLCAPI(libvlc_get_changeset)
        WWX190_RESOLVE_VLCAPI(libvlc_free)
        WWX190_RESOLVE_VLCAPI(libvlc_event_attach)
        WWX190_RESOLVE_VLCAPI(libvlc_event_detach)
        WWX190_RESOLVE_VLCAPI(libvlc_log_get_context)
        WWX190_RESOLVE_VLCAPI(libvlc_log_get_object)
        WWX190_RESOLVE_VLCAPI(libvlc_log_unset)
        WWX190_RESOLVE_VLCAPI(libvlc_log_set)
        WWX190_RESOLVE_VLCAPI(libvlc_log_set_file)
        WWX190_RESOLVE_VLCAPI(libvlc_module_description_list_release)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_filter_list_get)
        WWX190_RESOLVE_VLCAPI(libvlc_video_filter_list_get)
        WWX190_RESOLVE_VLCAPI(libvlc_clock)

        // libvlc_renderer_discoverer.h
        WWX190_RESOLVE_VLCAPI(libvlc_renderer_item_hold)
        WWX190_RESOLVE_VLCAPI(libvlc_renderer_item_release)
        WWX190_RESOLVE_VLCAPI(libvlc_renderer_item_name)
        WWX190_RESOLVE_VLCAPI(libvlc_renderer_item_type)
        WWX190_RESOLVE_VLCAPI(libvlc_renderer_item_icon_uri)
        WWX190_RESOLVE_VLCAPI(libvlc_renderer_item_flags)
        WWX190_RESOLVE_VLCAPI(libvlc_renderer_discoverer_new)
        WWX190_RESOLVE_VLCAPI(libvlc_renderer_discoverer_release)
        WWX190_RESOLVE_VLCAPI(libvlc_renderer_discoverer_start)
        WWX190_RESOLVE_VLCAPI(libvlc_renderer_discoverer_stop)
        WWX190_RESOLVE_VLCAPI(libvlc_renderer_discoverer_event_manager)
        WWX190_RESOLVE_VLCAPI(libvlc_renderer_discoverer_list_get)
        WWX190_RESOLVE_VLCAPI(libvlc_renderer_discoverer_list_release)

        // libvlc_picture.h
        WWX190_RESOLVE_VLCAPI(libvlc_picture_retain)
        WWX190_RESOLVE_VLCAPI(libvlc_picture_release)
        WWX190_RESOLVE_VLCAPI(libvlc_picture_save)
        WWX190_RESOLVE_VLCAPI(libvlc_picture_get_buffer)
        WWX190_RESOLVE_VLCAPI(libvlc_picture_type)
        WWX190_RESOLVE_VLCAPI(libvlc_picture_get_stride)
        WWX190_RESOLVE_VLCAPI(libvlc_picture_get_width)
        WWX190_RESOLVE_VLCAPI(libvlc_picture_get_height)
        WWX190_RESOLVE_VLCAPI(libvlc_picture_get_time)
        WWX190_RESOLVE_VLCAPI(libvlc_picture_list_count)
        WWX190_RESOLVE_VLCAPI(libvlc_picture_list_at)
        WWX190_RESOLVE_VLCAPI(libvlc_picture_list_destroy)

        // libvlc_media.h
        WWX190_RESOLVE_VLCAPI(libvlc_media_new_location)
        WWX190_RESOLVE_VLCAPI(libvlc_media_new_path)
        WWX190_RESOLVE_VLCAPI(libvlc_media_new_fd)
        WWX190_RESOLVE_VLCAPI(libvlc_media_new_callbacks)
        WWX190_RESOLVE_VLCAPI(libvlc_media_new_as_node)
        WWX190_RESOLVE_VLCAPI(libvlc_media_add_option)
        WWX190_RESOLVE_VLCAPI(libvlc_media_add_option_flag)
        WWX190_RESOLVE_VLCAPI(libvlc_media_retain)
        WWX190_RESOLVE_VLCAPI(libvlc_media_release)
        WWX190_RESOLVE_VLCAPI(libvlc_media_get_mrl)
        WWX190_RESOLVE_VLCAPI(libvlc_media_duplicate)
        WWX190_RESOLVE_VLCAPI(libvlc_media_get_meta)
        WWX190_RESOLVE_VLCAPI(libvlc_media_set_meta)
        WWX190_RESOLVE_VLCAPI(libvlc_media_save_meta)
        WWX190_RESOLVE_VLCAPI(libvlc_media_get_state)
        WWX190_RESOLVE_VLCAPI(libvlc_media_get_stats)
        WWX190_RESOLVE_VLCAPI(libvlc_media_subitems)
        WWX190_RESOLVE_VLCAPI(libvlc_media_event_manager)
        WWX190_RESOLVE_VLCAPI(libvlc_media_get_duration)
        WWX190_RESOLVE_VLCAPI(libvlc_media_parse_with_options)
        WWX190_RESOLVE_VLCAPI(libvlc_media_parse_stop)
        WWX190_RESOLVE_VLCAPI(libvlc_media_get_parsed_status)
        WWX190_RESOLVE_VLCAPI(libvlc_media_set_user_data)
        WWX190_RESOLVE_VLCAPI(libvlc_media_get_user_data)
        WWX190_RESOLVE_VLCAPI(libvlc_media_get_tracklist)
        WWX190_RESOLVE_VLCAPI(libvlc_media_get_codec_description)
        WWX190_RESOLVE_VLCAPI(libvlc_media_get_type)
        WWX190_RESOLVE_VLCAPI(libvlc_media_thumbnail_request_by_time)
        WWX190_RESOLVE_VLCAPI(libvlc_media_thumbnail_request_by_pos)
        WWX190_RESOLVE_VLCAPI(libvlc_media_thumbnail_request_cancel)
        WWX190_RESOLVE_VLCAPI(libvlc_media_thumbnail_request_destroy)
        WWX190_RESOLVE_VLCAPI(libvlc_media_slaves_add)
        WWX190_RESOLVE_VLCAPI(libvlc_media_slaves_clear)
        WWX190_RESOLVE_VLCAPI(libvlc_media_slaves_get)
        WWX190_RESOLVE_VLCAPI(libvlc_media_slaves_release)

        // libvlc_media_player.h
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_new)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_new_from_media)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_release)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_retain)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_set_media)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_media)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_event_manager)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_is_playing)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_play)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_set_pause)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_pause)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_stop_async)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_set_renderer)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_callbacks)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_format)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_format_callbacks)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_output_callbacks)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_set_nsobject)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_nsobject)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_set_xwindow)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_xwindow)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_set_hwnd)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_hwnd)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_set_android_context)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_set_callbacks)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_set_volume_callback)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_set_format_callbacks)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_set_format)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_length)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_time)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_set_time)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_position)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_set_position)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_set_chapter)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_chapter)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_chapter_count)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_chapter_count_for_title)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_set_title)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_title)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_title_count)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_previous_chapter)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_next_chapter)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_rate)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_set_rate)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_state)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_has_vout)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_is_seekable)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_can_pause)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_program_scrambled)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_next_frame)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_navigate)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_set_video_title_display)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_tracklist)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_selected_track)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_track_from_id)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_select_track)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_unselect_track_type)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_select_tracks)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_select_tracks_by_ids)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_add_slave)
        WWX190_RESOLVE_VLCAPI(libvlc_player_program_delete)
        WWX190_RESOLVE_VLCAPI(libvlc_player_programlist_count)
        WWX190_RESOLVE_VLCAPI(libvlc_player_programlist_at)
        WWX190_RESOLVE_VLCAPI(libvlc_player_programlist_delete)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_select_program_id)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_selected_program)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_program_from_id)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_programlist)
        WWX190_RESOLVE_VLCAPI(libvlc_toggle_fullscreen)
        WWX190_RESOLVE_VLCAPI(libvlc_set_fullscreen)
        WWX190_RESOLVE_VLCAPI(libvlc_get_fullscreen)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_key_input)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_mouse_input)
        WWX190_RESOLVE_VLCAPI(libvlc_video_get_size)
        WWX190_RESOLVE_VLCAPI(libvlc_video_get_cursor)
        WWX190_RESOLVE_VLCAPI(libvlc_video_get_scale)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_scale)
        WWX190_RESOLVE_VLCAPI(libvlc_video_get_aspect_ratio)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_aspect_ratio)
        WWX190_RESOLVE_VLCAPI(libvlc_video_new_viewpoint)
        WWX190_RESOLVE_VLCAPI(libvlc_video_update_viewpoint)
        WWX190_RESOLVE_VLCAPI(libvlc_video_get_spu_delay)
        WWX190_RESOLVE_VLCAPI(libvlc_video_get_spu_text_scale)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_spu_text_scale)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_spu_delay)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_full_title_descriptions)
        WWX190_RESOLVE_VLCAPI(libvlc_title_descriptions_release)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_full_chapter_descriptions)
        WWX190_RESOLVE_VLCAPI(libvlc_chapter_descriptions_release)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_crop_ratio)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_crop_window)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_crop_border)
        WWX190_RESOLVE_VLCAPI(libvlc_video_get_teletext)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_teletext)
        WWX190_RESOLVE_VLCAPI(libvlc_video_take_snapshot)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_deinterlace)
        WWX190_RESOLVE_VLCAPI(libvlc_video_get_marquee_int)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_marquee_int)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_marquee_string)
        WWX190_RESOLVE_VLCAPI(libvlc_video_get_logo_int)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_logo_int)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_logo_string)
        WWX190_RESOLVE_VLCAPI(libvlc_video_get_adjust_int)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_adjust_int)
        WWX190_RESOLVE_VLCAPI(libvlc_video_get_adjust_float)
        WWX190_RESOLVE_VLCAPI(libvlc_video_set_adjust_float)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_output_list_get)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_output_list_release)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_output_set)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_output_device_enum)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_output_device_list_get)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_output_device_list_release)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_output_device_set)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_output_device_get)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_toggle_mute)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_get_mute)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_set_mute)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_get_volume)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_set_volume)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_get_channel)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_set_channel)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_get_delay)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_set_delay)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_equalizer_get_preset_count)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_equalizer_get_preset_name)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_equalizer_get_band_count)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_equalizer_get_band_frequency)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_equalizer_new)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_equalizer_new_from_preset)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_equalizer_release)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_equalizer_set_preamp)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_equalizer_get_preamp)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_equalizer_set_amp_at_index)
        WWX190_RESOLVE_VLCAPI(libvlc_audio_equalizer_get_amp_at_index)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_set_equalizer)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_get_role)
        WWX190_RESOLVE_VLCAPI(libvlc_media_player_set_role)

        // libvlc_media_list.h
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_new)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_release)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_retain)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_set_media)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_media)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_add_media)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_insert_media)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_remove_index)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_count)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_item_at_index)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_index_of_item)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_is_readonly)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_lock)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_unlock)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_event_manager)

        // libvlc_media_list_player.h
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_player_new)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_player_release)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_player_retain)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_player_event_manager)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_player_set_media_player)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_player_get_media_player)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_player_set_media_list)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_player_play)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_player_pause)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_player_set_pause)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_player_is_playing)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_player_get_state)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_player_play_item_at_index)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_player_play_item)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_player_stop_async)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_player_next)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_player_previous)
        WWX190_RESOLVE_VLCAPI(libvlc_media_list_player_set_playback_mode)

        // libvlc_media_discoverer.h
        WWX190_RESOLVE_VLCAPI(libvlc_media_discoverer_new)
        WWX190_RESOLVE_VLCAPI(libvlc_media_discoverer_start)
        WWX190_RESOLVE_VLCAPI(libvlc_media_discoverer_stop)
        WWX190_RESOLVE_VLCAPI(libvlc_media_discoverer_release)
        WWX190_RESOLVE_VLCAPI(libvlc_media_discoverer_media_list)
        WWX190_RESOLVE_VLCAPI(libvlc_media_discoverer_is_running)
        WWX190_RESOLVE_VLCAPI(libvlc_media_discoverer_list_get)
        WWX190_RESOLVE_VLCAPI(libvlc_media_discoverer_list_release)

        // libvlc_dialog.h
        WWX190_RESOLVE_VLCAPI(libvlc_dialog_set_callbacks)
        WWX190_RESOLVE_VLCAPI(libvlc_dialog_set_context)
        WWX190_RESOLVE_VLCAPI(libvlc_dialog_get_context)
        WWX190_RESOLVE_VLCAPI(libvlc_dialog_post_login)
        WWX190_RESOLVE_VLCAPI(libvlc_dialog_post_action)
        WWX190_RESOLVE_VLCAPI(libvlc_dialog_dismiss)

        qCDebug(QTMEDIAPLAYER_PREPEND_NAMESPACE(lcQMPVLC)) << "libvlc loaded successfully.";
        return true;
    }

    [[nodiscard]] bool unload()
    {
        // libvlc.h
        WWX190_SETNULL_VLCAPI(libvlc_errmsg)
        WWX190_SETNULL_VLCAPI(libvlc_clearerr)
        WWX190_SETNULL_VLCAPI(libvlc_vprinterr)
        WWX190_SETNULL_VLCAPI(libvlc_printerr)
        WWX190_SETNULL_VLCAPI(libvlc_new)
        WWX190_SETNULL_VLCAPI(libvlc_release)
        WWX190_SETNULL_VLCAPI(libvlc_retain)
        WWX190_SETNULL_VLCAPI(libvlc_add_intf)
        WWX190_SETNULL_VLCAPI(libvlc_set_exit_handler)
        WWX190_SETNULL_VLCAPI(libvlc_set_user_agent)
        WWX190_SETNULL_VLCAPI(libvlc_set_app_id)
        WWX190_SETNULL_VLCAPI(libvlc_get_version)
        WWX190_SETNULL_VLCAPI(libvlc_get_compiler)
        WWX190_SETNULL_VLCAPI(libvlc_get_changeset)
        WWX190_SETNULL_VLCAPI(libvlc_free)
        WWX190_SETNULL_VLCAPI(libvlc_event_attach)
        WWX190_SETNULL_VLCAPI(libvlc_event_detach)
        WWX190_SETNULL_VLCAPI(libvlc_log_get_context)
        WWX190_SETNULL_VLCAPI(libvlc_log_get_object)
        WWX190_SETNULL_VLCAPI(libvlc_log_unset)
        WWX190_SETNULL_VLCAPI(libvlc_log_set)
        WWX190_SETNULL_VLCAPI(libvlc_log_set_file)
        WWX190_SETNULL_VLCAPI(libvlc_module_description_list_release)
        WWX190_SETNULL_VLCAPI(libvlc_audio_filter_list_get)
        WWX190_SETNULL_VLCAPI(libvlc_video_filter_list_get)
        WWX190_SETNULL_VLCAPI(libvlc_clock)

        // libvlc_renderer_discoverer.h
        WWX190_SETNULL_VLCAPI(libvlc_renderer_item_hold)
        WWX190_SETNULL_VLCAPI(libvlc_renderer_item_release)
        WWX190_SETNULL_VLCAPI(libvlc_renderer_item_name)
        WWX190_SETNULL_VLCAPI(libvlc_renderer_item_type)
        WWX190_SETNULL_VLCAPI(libvlc_renderer_item_icon_uri)
        WWX190_SETNULL_VLCAPI(libvlc_renderer_item_flags)
        WWX190_SETNULL_VLCAPI(libvlc_renderer_discoverer_new)
        WWX190_SETNULL_VLCAPI(libvlc_renderer_discoverer_release)
        WWX190_SETNULL_VLCAPI(libvlc_renderer_discoverer_start)
        WWX190_SETNULL_VLCAPI(libvlc_renderer_discoverer_stop)
        WWX190_SETNULL_VLCAPI(libvlc_renderer_discoverer_event_manager)
        WWX190_SETNULL_VLCAPI(libvlc_renderer_discoverer_list_get)
        WWX190_SETNULL_VLCAPI(libvlc_renderer_discoverer_list_release)

        // libvlc_picture.h
        WWX190_SETNULL_VLCAPI(libvlc_picture_retain)
        WWX190_SETNULL_VLCAPI(libvlc_picture_release)
        WWX190_SETNULL_VLCAPI(libvlc_picture_save)
        WWX190_SETNULL_VLCAPI(libvlc_picture_get_buffer)
        WWX190_SETNULL_VLCAPI(libvlc_picture_type)
        WWX190_SETNULL_VLCAPI(libvlc_picture_get_stride)
        WWX190_SETNULL_VLCAPI(libvlc_picture_get_width)
        WWX190_SETNULL_VLCAPI(libvlc_picture_get_height)
        WWX190_SETNULL_VLCAPI(libvlc_picture_get_time)
        WWX190_SETNULL_VLCAPI(libvlc_picture_list_count)
        WWX190_SETNULL_VLCAPI(libvlc_picture_list_at)
        WWX190_SETNULL_VLCAPI(libvlc_picture_list_destroy)

        // libvlc_media.h
        WWX190_SETNULL_VLCAPI(libvlc_media_new_location)
        WWX190_SETNULL_VLCAPI(libvlc_media_new_path)
        WWX190_SETNULL_VLCAPI(libvlc_media_new_fd)
        WWX190_SETNULL_VLCAPI(libvlc_media_new_callbacks)
        WWX190_SETNULL_VLCAPI(libvlc_media_new_as_node)
        WWX190_SETNULL_VLCAPI(libvlc_media_add_option)
        WWX190_SETNULL_VLCAPI(libvlc_media_add_option_flag)
        WWX190_SETNULL_VLCAPI(libvlc_media_retain)
        WWX190_SETNULL_VLCAPI(libvlc_media_release)
        WWX190_SETNULL_VLCAPI(libvlc_media_get_mrl)
        WWX190_SETNULL_VLCAPI(libvlc_media_duplicate)
        WWX190_SETNULL_VLCAPI(libvlc_media_get_meta)
        WWX190_SETNULL_VLCAPI(libvlc_media_set_meta)
        WWX190_SETNULL_VLCAPI(libvlc_media_save_meta)
        WWX190_SETNULL_VLCAPI(libvlc_media_get_state)
        WWX190_SETNULL_VLCAPI(libvlc_media_get_stats)
        WWX190_SETNULL_VLCAPI(libvlc_media_subitems)
        WWX190_SETNULL_VLCAPI(libvlc_media_event_manager)
        WWX190_SETNULL_VLCAPI(libvlc_media_get_duration)
        WWX190_SETNULL_VLCAPI(libvlc_media_parse_with_options)
        WWX190_SETNULL_VLCAPI(libvlc_media_parse_stop)
        WWX190_SETNULL_VLCAPI(libvlc_media_get_parsed_status)
        WWX190_SETNULL_VLCAPI(libvlc_media_set_user_data)
        WWX190_SETNULL_VLCAPI(libvlc_media_get_user_data)
        WWX190_SETNULL_VLCAPI(libvlc_media_get_tracklist)
        WWX190_SETNULL_VLCAPI(libvlc_media_get_codec_description)
        WWX190_SETNULL_VLCAPI(libvlc_media_get_type)
        WWX190_SETNULL_VLCAPI(libvlc_media_thumbnail_request_by_time)
        WWX190_SETNULL_VLCAPI(libvlc_media_thumbnail_request_by_pos)
        WWX190_SETNULL_VLCAPI(libvlc_media_thumbnail_request_cancel)
        WWX190_SETNULL_VLCAPI(libvlc_media_thumbnail_request_destroy)
        WWX190_SETNULL_VLCAPI(libvlc_media_slaves_add)
        WWX190_SETNULL_VLCAPI(libvlc_media_slaves_clear)
        WWX190_SETNULL_VLCAPI(libvlc_media_slaves_get)
        WWX190_SETNULL_VLCAPI(libvlc_media_slaves_release)

        // libvlc_media_player.h
        WWX190_SETNULL_VLCAPI(libvlc_media_player_new)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_new_from_media)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_release)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_retain)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_set_media)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_media)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_event_manager)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_is_playing)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_play)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_set_pause)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_pause)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_stop_async)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_set_renderer)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_callbacks)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_format)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_format_callbacks)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_output_callbacks)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_set_nsobject)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_nsobject)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_set_xwindow)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_xwindow)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_set_hwnd)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_hwnd)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_set_android_context)
        WWX190_SETNULL_VLCAPI(libvlc_audio_set_callbacks)
        WWX190_SETNULL_VLCAPI(libvlc_audio_set_volume_callback)
        WWX190_SETNULL_VLCAPI(libvlc_audio_set_format_callbacks)
        WWX190_SETNULL_VLCAPI(libvlc_audio_set_format)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_length)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_time)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_set_time)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_position)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_set_position)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_set_chapter)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_chapter)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_chapter_count)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_chapter_count_for_title)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_set_title)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_title)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_title_count)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_previous_chapter)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_next_chapter)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_rate)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_set_rate)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_state)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_has_vout)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_is_seekable)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_can_pause)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_program_scrambled)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_next_frame)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_navigate)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_set_video_title_display)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_tracklist)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_selected_track)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_track_from_id)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_select_track)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_unselect_track_type)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_select_tracks)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_select_tracks_by_ids)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_add_slave)
        WWX190_SETNULL_VLCAPI(libvlc_player_program_delete)
        WWX190_SETNULL_VLCAPI(libvlc_player_programlist_count)
        WWX190_SETNULL_VLCAPI(libvlc_player_programlist_at)
        WWX190_SETNULL_VLCAPI(libvlc_player_programlist_delete)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_select_program_id)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_selected_program)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_program_from_id)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_programlist)
        WWX190_SETNULL_VLCAPI(libvlc_toggle_fullscreen)
        WWX190_SETNULL_VLCAPI(libvlc_set_fullscreen)
        WWX190_SETNULL_VLCAPI(libvlc_get_fullscreen)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_key_input)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_mouse_input)
        WWX190_SETNULL_VLCAPI(libvlc_video_get_size)
        WWX190_SETNULL_VLCAPI(libvlc_video_get_cursor)
        WWX190_SETNULL_VLCAPI(libvlc_video_get_scale)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_scale)
        WWX190_SETNULL_VLCAPI(libvlc_video_get_aspect_ratio)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_aspect_ratio)
        WWX190_SETNULL_VLCAPI(libvlc_video_new_viewpoint)
        WWX190_SETNULL_VLCAPI(libvlc_video_update_viewpoint)
        WWX190_SETNULL_VLCAPI(libvlc_video_get_spu_delay)
        WWX190_SETNULL_VLCAPI(libvlc_video_get_spu_text_scale)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_spu_text_scale)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_spu_delay)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_full_title_descriptions)
        WWX190_SETNULL_VLCAPI(libvlc_title_descriptions_release)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_full_chapter_descriptions)
        WWX190_SETNULL_VLCAPI(libvlc_chapter_descriptions_release)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_crop_ratio)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_crop_window)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_crop_border)
        WWX190_SETNULL_VLCAPI(libvlc_video_get_teletext)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_teletext)
        WWX190_SETNULL_VLCAPI(libvlc_video_take_snapshot)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_deinterlace)
        WWX190_SETNULL_VLCAPI(libvlc_video_get_marquee_int)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_marquee_int)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_marquee_string)
        WWX190_SETNULL_VLCAPI(libvlc_video_get_logo_int)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_logo_int)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_logo_string)
        WWX190_SETNULL_VLCAPI(libvlc_video_get_adjust_int)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_adjust_int)
        WWX190_SETNULL_VLCAPI(libvlc_video_get_adjust_float)
        WWX190_SETNULL_VLCAPI(libvlc_video_set_adjust_float)
        WWX190_SETNULL_VLCAPI(libvlc_audio_output_list_get)
        WWX190_SETNULL_VLCAPI(libvlc_audio_output_list_release)
        WWX190_SETNULL_VLCAPI(libvlc_audio_output_set)
        WWX190_SETNULL_VLCAPI(libvlc_audio_output_device_enum)
        WWX190_SETNULL_VLCAPI(libvlc_audio_output_device_list_get)
        WWX190_SETNULL_VLCAPI(libvlc_audio_output_device_list_release)
        WWX190_SETNULL_VLCAPI(libvlc_audio_output_device_set)
        WWX190_SETNULL_VLCAPI(libvlc_audio_output_device_get)
        WWX190_SETNULL_VLCAPI(libvlc_audio_toggle_mute)
        WWX190_SETNULL_VLCAPI(libvlc_audio_get_mute)
        WWX190_SETNULL_VLCAPI(libvlc_audio_set_mute)
        WWX190_SETNULL_VLCAPI(libvlc_audio_get_volume)
        WWX190_SETNULL_VLCAPI(libvlc_audio_set_volume)
        WWX190_SETNULL_VLCAPI(libvlc_audio_get_channel)
        WWX190_SETNULL_VLCAPI(libvlc_audio_set_channel)
        WWX190_SETNULL_VLCAPI(libvlc_audio_get_delay)
        WWX190_SETNULL_VLCAPI(libvlc_audio_set_delay)
        WWX190_SETNULL_VLCAPI(libvlc_audio_equalizer_get_preset_count)
        WWX190_SETNULL_VLCAPI(libvlc_audio_equalizer_get_preset_name)
        WWX190_SETNULL_VLCAPI(libvlc_audio_equalizer_get_band_count)
        WWX190_SETNULL_VLCAPI(libvlc_audio_equalizer_get_band_frequency)
        WWX190_SETNULL_VLCAPI(libvlc_audio_equalizer_new)
        WWX190_SETNULL_VLCAPI(libvlc_audio_equalizer_new_from_preset)
        WWX190_SETNULL_VLCAPI(libvlc_audio_equalizer_release)
        WWX190_SETNULL_VLCAPI(libvlc_audio_equalizer_set_preamp)
        WWX190_SETNULL_VLCAPI(libvlc_audio_equalizer_get_preamp)
        WWX190_SETNULL_VLCAPI(libvlc_audio_equalizer_set_amp_at_index)
        WWX190_SETNULL_VLCAPI(libvlc_audio_equalizer_get_amp_at_index)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_set_equalizer)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_get_role)
        WWX190_SETNULL_VLCAPI(libvlc_media_player_set_role)

        // libvlc_media_list.h
        WWX190_SETNULL_VLCAPI(libvlc_media_list_new)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_release)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_retain)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_set_media)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_media)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_add_media)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_insert_media)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_remove_index)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_count)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_item_at_index)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_index_of_item)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_is_readonly)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_lock)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_unlock)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_event_manager)

        // libvlc_media_list_player.h
        WWX190_SETNULL_VLCAPI(libvlc_media_list_player_new)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_player_release)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_player_retain)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_player_event_manager)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_player_set_media_player)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_player_get_media_player)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_player_set_media_list)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_player_play)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_player_pause)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_player_set_pause)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_player_is_playing)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_player_get_state)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_player_play_item_at_index)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_player_play_item)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_player_stop_async)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_player_next)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_player_previous)
        WWX190_SETNULL_VLCAPI(libvlc_media_list_player_set_playback_mode)

        // libvlc_media_discoverer.h
        WWX190_SETNULL_VLCAPI(libvlc_media_discoverer_new)
        WWX190_SETNULL_VLCAPI(libvlc_media_discoverer_start)
        WWX190_SETNULL_VLCAPI(libvlc_media_discoverer_stop)
        WWX190_SETNULL_VLCAPI(libvlc_media_discoverer_release)
        WWX190_SETNULL_VLCAPI(libvlc_media_discoverer_media_list)
        WWX190_SETNULL_VLCAPI(libvlc_media_discoverer_is_running)
        WWX190_SETNULL_VLCAPI(libvlc_media_discoverer_list_get)
        WWX190_SETNULL_VLCAPI(libvlc_media_discoverer_list_release)

        // libvlc_dialog.h
        WWX190_SETNULL_VLCAPI(libvlc_dialog_set_callbacks)
        WWX190_SETNULL_VLCAPI(libvlc_dialog_set_context)
        WWX190_SETNULL_VLCAPI(libvlc_dialog_get_context)
        WWX190_SETNULL_VLCAPI(libvlc_dialog_post_login)
        WWX190_SETNULL_VLCAPI(libvlc_dialog_post_action)
        WWX190_SETNULL_VLCAPI(libvlc_dialog_dismiss)

        if (library.isLoaded()) {
            if (!library.unload()) {
                qCWarning(QTMEDIAPLAYER_PREPEND_NAMESPACE(lcQMPVLC)) << "Failed to unload libvlc:" << library.errorString();
                return false;
            }
        }

        qCDebug(QTMEDIAPLAYER_PREPEND_NAMESPACE(lcQMPVLC)) << "libvlc unloaded successfully.";
        return true;
    }

    [[nodiscard]] bool isLoaded() const
    {
        const bool result =
                // libvlc.h
                WWX190_NOTNULL_VLCAPI(libvlc_errmsg) &&
                WWX190_NOTNULL_VLCAPI(libvlc_clearerr) &&
                WWX190_NOTNULL_VLCAPI(libvlc_vprinterr) &&
                WWX190_NOTNULL_VLCAPI(libvlc_printerr) &&
                WWX190_NOTNULL_VLCAPI(libvlc_new) &&
                WWX190_NOTNULL_VLCAPI(libvlc_release) &&
                WWX190_NOTNULL_VLCAPI(libvlc_retain) &&
                WWX190_NOTNULL_VLCAPI(libvlc_add_intf) &&
                WWX190_NOTNULL_VLCAPI(libvlc_set_exit_handler) &&
                WWX190_NOTNULL_VLCAPI(libvlc_set_user_agent) &&
                WWX190_NOTNULL_VLCAPI(libvlc_set_app_id) &&
                WWX190_NOTNULL_VLCAPI(libvlc_get_version) &&
                WWX190_NOTNULL_VLCAPI(libvlc_get_compiler) &&
                WWX190_NOTNULL_VLCAPI(libvlc_get_changeset) &&
                WWX190_NOTNULL_VLCAPI(libvlc_free) &&
                WWX190_NOTNULL_VLCAPI(libvlc_event_attach) &&
                WWX190_NOTNULL_VLCAPI(libvlc_event_detach) &&
                WWX190_NOTNULL_VLCAPI(libvlc_log_get_context) &&
                WWX190_NOTNULL_VLCAPI(libvlc_log_get_object) &&
                WWX190_NOTNULL_VLCAPI(libvlc_log_unset) &&
                WWX190_NOTNULL_VLCAPI(libvlc_log_set) &&
                WWX190_NOTNULL_VLCAPI(libvlc_log_set_file) &&
                WWX190_NOTNULL_VLCAPI(libvlc_module_description_list_release) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_filter_list_get) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_filter_list_get) &&
                WWX190_NOTNULL_VLCAPI(libvlc_clock) &&
                // libvlc_renderer_discoverer.h
                WWX190_NOTNULL_VLCAPI(libvlc_renderer_item_hold) &&
                WWX190_NOTNULL_VLCAPI(libvlc_renderer_item_release) &&
                WWX190_NOTNULL_VLCAPI(libvlc_renderer_item_name) &&
                WWX190_NOTNULL_VLCAPI(libvlc_renderer_item_type) &&
                WWX190_NOTNULL_VLCAPI(libvlc_renderer_item_icon_uri) &&
                WWX190_NOTNULL_VLCAPI(libvlc_renderer_item_flags) &&
                WWX190_NOTNULL_VLCAPI(libvlc_renderer_discoverer_new) &&
                WWX190_NOTNULL_VLCAPI(libvlc_renderer_discoverer_release) &&
                WWX190_NOTNULL_VLCAPI(libvlc_renderer_discoverer_start) &&
                WWX190_NOTNULL_VLCAPI(libvlc_renderer_discoverer_stop) &&
                WWX190_NOTNULL_VLCAPI(libvlc_renderer_discoverer_event_manager) &&
                WWX190_NOTNULL_VLCAPI(libvlc_renderer_discoverer_list_get) &&
                WWX190_NOTNULL_VLCAPI(libvlc_renderer_discoverer_list_release) &&
                // libvlc_picture.h
                WWX190_NOTNULL_VLCAPI(libvlc_picture_retain) &&
                WWX190_NOTNULL_VLCAPI(libvlc_picture_release) &&
                WWX190_NOTNULL_VLCAPI(libvlc_picture_save) &&
                WWX190_NOTNULL_VLCAPI(libvlc_picture_get_buffer) &&
                WWX190_NOTNULL_VLCAPI(libvlc_picture_type) &&
                WWX190_NOTNULL_VLCAPI(libvlc_picture_get_stride) &&
                WWX190_NOTNULL_VLCAPI(libvlc_picture_get_width) &&
                WWX190_NOTNULL_VLCAPI(libvlc_picture_get_height) &&
                WWX190_NOTNULL_VLCAPI(libvlc_picture_get_time) &&
                WWX190_NOTNULL_VLCAPI(libvlc_picture_list_count) &&
                WWX190_NOTNULL_VLCAPI(libvlc_picture_list_at) &&
                WWX190_NOTNULL_VLCAPI(libvlc_picture_list_destroy) &&
                // libvlc_media.h
                WWX190_NOTNULL_VLCAPI(libvlc_media_new_location) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_new_path) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_new_fd) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_new_callbacks) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_new_as_node) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_add_option) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_add_option_flag) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_retain) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_release) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_get_mrl) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_duplicate) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_get_meta) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_set_meta) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_save_meta) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_get_state) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_get_stats) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_subitems) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_event_manager) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_get_duration) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_parse_with_options) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_parse_stop) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_get_parsed_status) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_set_user_data) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_get_user_data) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_get_tracklist) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_get_codec_description) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_get_type) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_thumbnail_request_by_time) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_thumbnail_request_by_pos) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_thumbnail_request_cancel) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_thumbnail_request_destroy) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_slaves_add) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_slaves_clear) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_slaves_get) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_slaves_release) &&
                // libvlc_media_player.h
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_new) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_new_from_media) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_release) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_retain) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_set_media) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_media) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_event_manager) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_is_playing) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_play) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_set_pause) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_pause) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_stop_async) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_set_renderer) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_callbacks) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_format) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_format_callbacks) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_output_callbacks) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_set_nsobject) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_nsobject) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_set_xwindow) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_xwindow) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_set_hwnd) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_hwnd) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_set_android_context) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_set_callbacks) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_set_volume_callback) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_set_format_callbacks) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_set_format) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_length) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_time) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_set_time) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_position) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_set_position) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_set_chapter) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_chapter) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_chapter_count) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_chapter_count_for_title) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_set_title) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_title) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_title_count) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_previous_chapter) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_next_chapter) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_rate) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_set_rate) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_state) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_has_vout) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_is_seekable) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_can_pause) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_program_scrambled) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_next_frame) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_navigate) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_set_video_title_display) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_tracklist) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_selected_track) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_track_from_id) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_select_track) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_unselect_track_type) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_select_tracks) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_select_tracks_by_ids) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_add_slave) &&
                WWX190_NOTNULL_VLCAPI(libvlc_player_program_delete) &&
                WWX190_NOTNULL_VLCAPI(libvlc_player_programlist_count) &&
                WWX190_NOTNULL_VLCAPI(libvlc_player_programlist_at) &&
                WWX190_NOTNULL_VLCAPI(libvlc_player_programlist_delete) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_select_program_id) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_selected_program) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_program_from_id) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_programlist) &&
                WWX190_NOTNULL_VLCAPI(libvlc_toggle_fullscreen) &&
                WWX190_NOTNULL_VLCAPI(libvlc_set_fullscreen) &&
                WWX190_NOTNULL_VLCAPI(libvlc_get_fullscreen) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_key_input) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_mouse_input) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_get_size) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_get_cursor) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_get_scale) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_scale) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_get_aspect_ratio) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_aspect_ratio) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_new_viewpoint) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_update_viewpoint) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_get_spu_delay) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_get_spu_text_scale) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_spu_text_scale) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_spu_delay) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_full_title_descriptions) &&
                WWX190_NOTNULL_VLCAPI(libvlc_title_descriptions_release) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_full_chapter_descriptions) &&
                WWX190_NOTNULL_VLCAPI(libvlc_chapter_descriptions_release) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_crop_ratio) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_crop_window) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_crop_border) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_get_teletext) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_teletext) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_take_snapshot) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_deinterlace) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_get_marquee_int) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_marquee_int) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_marquee_string) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_get_logo_int) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_logo_int) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_logo_string) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_get_adjust_int) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_adjust_int) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_get_adjust_float) &&
                WWX190_NOTNULL_VLCAPI(libvlc_video_set_adjust_float) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_output_list_get) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_output_list_release) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_output_set) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_output_device_enum) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_output_device_list_get) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_output_device_list_release) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_output_device_set) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_output_device_get) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_toggle_mute) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_get_mute) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_set_mute) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_get_volume) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_set_volume) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_get_channel) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_set_channel) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_get_delay) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_set_delay) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_equalizer_get_preset_count) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_equalizer_get_preset_name) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_equalizer_get_band_count) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_equalizer_get_band_frequency) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_equalizer_new) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_equalizer_new_from_preset) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_equalizer_release) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_equalizer_set_preamp) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_equalizer_get_preamp) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_equalizer_set_amp_at_index) &&
                WWX190_NOTNULL_VLCAPI(libvlc_audio_equalizer_get_amp_at_index) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_set_equalizer) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_get_role) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_player_set_role) &&
                // libvlc_media_list.h
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_new) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_release) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_retain) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_set_media) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_media) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_add_media) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_insert_media) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_remove_index) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_count) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_item_at_index) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_index_of_item) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_is_readonly) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_lock) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_unlock) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_event_manager) &&
                // libvlc_media_list_player.h
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_player_new) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_player_release) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_player_retain) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_player_event_manager) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_player_set_media_player) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_player_get_media_player) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_player_set_media_list) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_player_play) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_player_pause) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_player_set_pause) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_player_is_playing) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_player_get_state) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_player_play_item_at_index) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_player_play_item) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_player_stop_async) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_player_next) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_player_previous) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_list_player_set_playback_mode) &&
                // libvlc_media_discoverer.h
                WWX190_NOTNULL_VLCAPI(libvlc_media_discoverer_new) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_discoverer_start) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_discoverer_stop) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_discoverer_release) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_discoverer_media_list) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_discoverer_is_running) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_discoverer_list_get) &&
                WWX190_NOTNULL_VLCAPI(libvlc_media_discoverer_list_release) &&
                // libvlc_dialog.h
                WWX190_NOTNULL_VLCAPI(libvlc_dialog_set_callbacks) &&
                WWX190_NOTNULL_VLCAPI(libvlc_dialog_set_context) &&
                WWX190_NOTNULL_VLCAPI(libvlc_dialog_get_context) &&
                WWX190_NOTNULL_VLCAPI(libvlc_dialog_post_login) &&
                WWX190_NOTNULL_VLCAPI(libvlc_dialog_post_action) &&
                WWX190_NOTNULL_VLCAPI(libvlc_dialog_dismiss);
        return result;
    }

private:
    QLibrary library;
};

Q_GLOBAL_STATIC(VLCData, vlcData)

bool isLibvlcAvailable()
{
    return vlcData()->isLoaded();
}

QString getLibvlcVersion()
{
    return QString::fromUtf8(libvlc_get_version());
}

} // namespace VLC::Qt

//////////////////////////////////////////////////////
/////////           libvlc
//////////////////////////////////////////////////////

// libvlc.h

const char *libvlc_errmsg()
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_errmsg, nullptr);
}

void libvlc_clearerr()
{
    WWX190_CALL_VLCAPI(libvlc_clearerr)
}

const char *libvlc_vprinterr(const char *fmt, va_list ap)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_vprinterr, nullptr, fmt, ap);
}

const char *libvlc_printerr(const char *fmt, va_list ap)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_printerr, nullptr, fmt, ap);
}

libvlc_instance_t *libvlc_new(int argc, const char *const *argv)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_new, nullptr, argc, argv);
}

void libvlc_release(libvlc_instance_t *p_instance)
{
    WWX190_CALL_VLCAPI(libvlc_release, p_instance)
}

void libvlc_retain(libvlc_instance_t *p_instance)
{
    WWX190_CALL_VLCAPI(libvlc_retain, p_instance)
}

int libvlc_add_intf(libvlc_instance_t *p_instance, const char *name)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_add_intf, -1, p_instance, name);
}

void libvlc_set_exit_handler(libvlc_instance_t *p_instance, void(*cb)(void *), void *opaque)
{
    WWX190_CALL_VLCAPI(libvlc_set_exit_handler, p_instance, cb, opaque)
}

void libvlc_set_user_agent(libvlc_instance_t *p_instance, const char *name, const char *http)
{
    WWX190_CALL_VLCAPI(libvlc_set_user_agent, p_instance, name, http)
}

void libvlc_set_app_id(libvlc_instance_t *p_instance, const char *id, const char *version, const char *icon)
{
    WWX190_CALL_VLCAPI(libvlc_set_app_id, p_instance, id, version, icon)
}

const char *libvlc_get_version()
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_get_version, nullptr);
}

const char *libvlc_get_compiler()
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_get_compiler, nullptr);
}

const char *libvlc_get_changeset()
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_get_changeset, nullptr);
}

void libvlc_free(void *ptr)
{
    WWX190_CALL_VLCAPI(libvlc_free, ptr)
}

int libvlc_event_attach(libvlc_event_manager_t *p_event_manager, libvlc_event_type_t i_event_type, libvlc_callback_t f_callback, void *user_data)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_event_attach, -1, p_event_manager, i_event_type, f_callback, user_data);
}

void libvlc_event_detach(libvlc_event_manager_t *p_event_manager, libvlc_event_type_t i_event_type, libvlc_callback_t f_callback, void *p_user_data)
{
    WWX190_CALL_VLCAPI(libvlc_event_detach, p_event_manager, i_event_type, f_callback, p_user_data)
}

void libvlc_log_get_context(const libvlc_log_t *ctx, const char **module, const char **file, unsigned *line)
{
    WWX190_CALL_VLCAPI(libvlc_log_get_context, ctx, module, file, line)
}

void libvlc_log_get_object(const libvlc_log_t *ctx, const char **name, const char **header, uintptr_t *id)
{
    WWX190_CALL_VLCAPI(libvlc_log_get_object, ctx, name, header, id)
}

void libvlc_log_unset(libvlc_instance_t *p_instance)
{
    WWX190_CALL_VLCAPI(libvlc_log_unset, p_instance)
}

void libvlc_log_set(libvlc_instance_t *p_instance, libvlc_log_cb cb, void *data)
{
    WWX190_CALL_VLCAPI(libvlc_log_set, p_instance, cb, data)
}

void libvlc_log_set_file(libvlc_instance_t *p_instance, FILE *stream)
{
    WWX190_CALL_VLCAPI(libvlc_log_set_file, p_instance, stream)
}

void libvlc_module_description_list_release(libvlc_module_description_t *p_list)
{
    WWX190_CALL_VLCAPI(libvlc_module_description_list_release, p_list)
}

libvlc_module_description_t *libvlc_audio_filter_list_get(libvlc_instance_t *p_instance)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_filter_list_get, nullptr, p_instance);
}

libvlc_module_description_t *libvlc_video_filter_list_get(libvlc_instance_t *p_instance)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_video_filter_list_get, nullptr, p_instance);
}

int64_t libvlc_clock()
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_clock, -1);
}

// libvlc_renderer_discoverer.h

// TODO
