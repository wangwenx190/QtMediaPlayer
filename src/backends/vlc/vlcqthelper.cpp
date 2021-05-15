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

libvlc_renderer_item_t *libvlc_renderer_item_hold(libvlc_renderer_item_t *p_item)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_renderer_item_hold, nullptr, p_item);
}

void libvlc_renderer_item_release(libvlc_renderer_item_t *p_item)
{
    WWX190_CALL_VLCAPI(libvlc_renderer_item_release, p_item)
}

const char *libvlc_renderer_item_name(const libvlc_renderer_item_t *p_item)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_renderer_item_name, nullptr, p_item);
}

const char *libvlc_renderer_item_type(const libvlc_renderer_item_t *p_item)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_renderer_item_type, nullptr, p_item);
}

const char *libvlc_renderer_item_icon_uri(const libvlc_renderer_item_t *p_item)
{
    WWX190_CALL_VLCAPI_RETURN(libvlc_renderer_item_icon_uri, nullptr, p_item);
}

int libvlc_renderer_item_flags(const libvlc_renderer_item_t *p_item)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_renderer_item_flags, -1, p_item);
}

libvlc_renderer_discoverer_t *libvlc_renderer_discoverer_new(libvlc_instance_t *p_inst, const char *psz_name)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_renderer_discoverer_new, nullptr, p_inst, psz_name);
}

void libvlc_renderer_discoverer_release(libvlc_renderer_discoverer_t *p_rd)
{
    WWX190_CALL_VLCAPI(libvlc_renderer_discoverer_release, p_rd)
}

int libvlc_renderer_discoverer_start(libvlc_renderer_discoverer_t *p_rd)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_renderer_discoverer_start, -1, p_rd);
}

void libvlc_renderer_discoverer_stop(libvlc_renderer_discoverer_t *p_rd)
{
    WWX190_CALL_VLCAPI(libvlc_renderer_discoverer_stop, p_rd)
}

libvlc_event_manager_t *libvlc_renderer_discoverer_event_manager(libvlc_renderer_discoverer_t *p_rd)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_renderer_discoverer_event_manager, nullptr, p_rd);
}

size_t libvlc_renderer_discoverer_list_get(libvlc_instance_t *p_inst, libvlc_rd_description_t ***ppp_services)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_renderer_discoverer_list_get, -1, p_inst, ppp_services);
}

void libvlc_renderer_discoverer_list_release(libvlc_rd_description_t **pp_services, size_t i_count)
{
    WWX190_CALL_VLCAPI(libvlc_renderer_discoverer_list_release, pp_services, i_count)
}

// libvlc_picture.h

void libvlc_picture_retain(libvlc_picture_t *pic)
{
    WWX190_CALL_VLCAPI(libvlc_picture_retain, pic)
}

void libvlc_picture_release(libvlc_picture_t *pic)
{
    WWX190_CALL_VLCAPI(libvlc_picture_release, pic)
}

int libvlc_picture_save(const libvlc_picture_t *pic, const char *path)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_picture_save, -1, pic, path);
}

const unsigned char *libvlc_picture_get_buffer(const libvlc_picture_t *pic, size_t *size)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_picture_get_buffer, nullptr, pic, size);
}

libvlc_picture_type_t libvlc_picture_type(const libvlc_picture_t *pic)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_picture_type, libvlc_picture_Argb, pic);
}

unsigned int libvlc_picture_get_stride(const libvlc_picture_t *pic)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_picture_get_stride, 0, pic);
}

unsigned int libvlc_picture_get_width(const libvlc_picture_t *pic)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_picture_get_width, 0, pic);
}

unsigned int libvlc_picture_get_height(const libvlc_picture_t *pic)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_picture_get_height, 0, pic);
}

libvlc_time_t libvlc_picture_get_time(const libvlc_picture_t *pic)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_picture_get_time, -1, pic);
}

size_t libvlc_picture_list_count(const libvlc_picture_list_t *list)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_picture_list_count, -1, list);
}

libvlc_picture_t *libvlc_picture_list_at(const libvlc_picture_list_t *list, size_t index)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_picture_list_at, nullptr, list, index);
}

void libvlc_picture_list_destroy(libvlc_picture_list_t *list)
{
    WWX190_CALL_VLCAPI(libvlc_picture_list_destroy, list)
}

// libvlc_media.h

libvlc_media_t *libvlc_media_new_location(libvlc_instance_t *p_instance, const char *psz_mrl)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_new_location, nullptr, p_instance, psz_mrl);
}

libvlc_media_t *libvlc_media_new_path(libvlc_instance_t *p_instance, const char *path)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_new_path, nullptr, p_instance, path);
}

libvlc_media_t *libvlc_media_new_fd(libvlc_instance_t *p_instance, int fd)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_new_fd, nullptr, p_instance, fd);
}

libvlc_media_t *libvlc_media_new_callbacks(libvlc_instance_t *instance, libvlc_media_open_cb open_cb, libvlc_media_read_cb read_cb, libvlc_media_seek_cb seek_cb, libvlc_media_close_cb close_cb, void *opaque)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_new_callbacks, nullptr, instance, open_cb, read_cb, seek_cb, close_cb, opaque);
}

libvlc_media_t *libvlc_media_new_as_node(libvlc_instance_t *p_instance, const char *psz_name)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_new_as_node, nullptr, p_instance, psz_name);
}

void libvlc_media_add_option(libvlc_media_t *p_md, const char *psz_options)
{
    WWX190_CALL_VLCAPI(libvlc_media_add_option, p_md, psz_options)
}

void libvlc_media_add_option_flag(libvlc_media_t *p_md, const char *psz_options, unsigned i_flags)
{
    WWX190_CALL_VLCAPI(libvlc_media_add_option_flag, p_md, psz_options, i_flags)
}

void libvlc_media_retain(libvlc_media_t *p_md)
{
    WWX190_CALL_VLCAPI(libvlc_media_retain, p_md)
}

void libvlc_media_release(libvlc_media_t *p_md)
{
    WWX190_CALL_VLCAPI(libvlc_media_release, p_md)
}

char *libvlc_media_get_mrl(libvlc_media_t *p_md)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_get_mrl, nullptr, p_md);
}

libvlc_media_t *libvlc_media_duplicate(libvlc_media_t *p_md)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_duplicate, nullptr, p_md);
}

char *libvlc_media_get_meta(libvlc_media_t *p_md, libvlc_meta_t e_meta)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_get_meta, nullptr, p_md, e_meta);
}

void libvlc_media_set_meta(libvlc_media_t *p_md, libvlc_meta_t e_meta, const char *psz_value)
{
    WWX190_CALL_VLCAPI(libvlc_media_set_meta, p_md, e_meta, psz_value)
}

int libvlc_media_save_meta(libvlc_media_t *p_md)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_save_meta, -1, p_md);
}

libvlc_state_t libvlc_media_get_state(libvlc_media_t *p_md)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_get_state, libvlc_NothingSpecial, p_md);
}

bool libvlc_media_get_stats(libvlc_media_t *p_md, libvlc_media_stats_t *p_stats)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_get_stats, false, p_md, p_stats);
}

libvlc_media_list_t *libvlc_media_subitems(libvlc_media_t *p_md)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_subitems, nullptr, p_md);
}

libvlc_event_manager_t *libvlc_media_event_manager(libvlc_media_t *p_md)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_event_manager, nullptr, p_md);
}

libvlc_time_t libvlc_media_get_duration(libvlc_media_t *p_md)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_get_duration, -1, p_md);
}

int libvlc_media_parse_with_options(libvlc_media_t *p_md, libvlc_media_parse_flag_t parse_flag, int timeout)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_parse_with_options, -1, p_md, parse_flag, timeout);
}

void libvlc_media_parse_stop(libvlc_media_t *p_md)
{
    WWX190_CALL_VLCAPI(libvlc_media_parse_stop, p_md)
}

libvlc_media_parsed_status_t libvlc_media_get_parsed_status(libvlc_media_t *p_md)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_get_parsed_status, libvlc_media_parsed_status_skipped, p_md);
}

void libvlc_media_set_user_data(libvlc_media_t *p_md, void *p_new_user_data)
{
    WWX190_CALL_VLCAPI(libvlc_media_set_user_data, p_md, p_new_user_data)
}

void *libvlc_media_get_user_data(libvlc_media_t *p_md)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_get_user_data, nullptr, p_md);
}

libvlc_media_tracklist_t *libvlc_media_get_tracklist(libvlc_media_t *p_md, libvlc_track_type_t type)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_get_tracklist, nullptr, p_md, type);
}

const char *libvlc_media_get_codec_description(libvlc_track_type_t i_type, uint32_t i_codec)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_get_codec_description, nullptr, i_type, i_codec);
}

libvlc_media_type_t libvlc_media_get_type(libvlc_media_t *p_md)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_get_type, libvlc_media_type_unknown, p_md);
}

libvlc_media_thumbnail_request_t *libvlc_media_thumbnail_request_by_time(libvlc_media_t *md, libvlc_time_t time, libvlc_thumbnailer_seek_speed_t speed, unsigned int width, unsigned int height, bool crop, libvlc_picture_type_t picture_type, libvlc_time_t timeout)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_thumbnail_request_by_time, nullptr, md, time, speed, width, height, crop, picture_type, timeout);
}

libvlc_media_thumbnail_request_t *libvlc_media_thumbnail_request_by_pos(libvlc_media_t *md, float pos, libvlc_thumbnailer_seek_speed_t speed, unsigned int width, unsigned int height, bool crop, libvlc_picture_type_t picture_type, libvlc_time_t timeout)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_thumbnail_request_by_pos, nullptr, md, pos, speed, width, height, crop, picture_type, timeout);
}

void libvlc_media_thumbnail_request_cancel(libvlc_media_thumbnail_request_t *p_req)
{
    WWX190_CALL_VLCAPI(libvlc_media_thumbnail_request_cancel, p_req)
}

void libvlc_media_thumbnail_request_destroy(libvlc_media_thumbnail_request_t *p_req)
{
    WWX190_CALL_VLCAPI(libvlc_media_thumbnail_request_destroy, p_req)
}

int libvlc_media_slaves_add(libvlc_media_t *p_md, libvlc_media_slave_type_t i_type, unsigned int i_priority, const char *psz_uri)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_slaves_add, -1, p_md, i_type, i_priority, psz_uri);
}

void libvlc_media_slaves_clear(libvlc_media_t *p_md)
{
    WWX190_CALL_VLCAPI(libvlc_media_slaves_clear, p_md)
}

unsigned int libvlc_media_slaves_get(libvlc_media_t *p_md, libvlc_media_slave_t ***ppp_slaves)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_slaves_get, 0, p_md, ppp_slaves);
}

void libvlc_media_slaves_release(libvlc_media_slave_t **pp_slaves, unsigned int i_count)
{
    WWX190_CALL_VLCAPI(libvlc_media_slaves_release, pp_slaves, i_count)
}

// libvlc_media_player.h

libvlc_media_player_t *libvlc_media_player_new(libvlc_instance_t *p_libvlc_instance)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_new, nullptr, p_libvlc_instance);
}

libvlc_media_player_t *libvlc_media_player_new_from_media(libvlc_media_t *p_md)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_new_from_media, nullptr, p_md);
}

void libvlc_media_player_release(libvlc_media_player_t *p_mi)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_release, p_mi)
}

void libvlc_media_player_retain(libvlc_media_player_t *p_mi)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_retain, p_mi)
}

void libvlc_media_player_set_media(libvlc_media_player_t *p_mi, libvlc_media_t *p_md)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_set_media, p_mi, p_md)
}

libvlc_media_t *libvlc_media_player_get_media(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_media, nullptr, p_mi);
}

libvlc_event_manager_t *libvlc_media_player_event_manager(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_event_manager, nullptr, p_mi);
}

bool libvlc_media_player_is_playing(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_is_playing, false, p_mi);
}

int libvlc_media_player_play(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_play, -1, p_mi);
}

void libvlc_media_player_set_pause(libvlc_media_player_t *mp, int do_pause)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_set_pause, mp, do_pause)
}

void libvlc_media_player_pause(libvlc_media_player_t *p_mi)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_pause, p_mi)
}

int libvlc_media_player_stop_async(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_stop_async, -1, p_mi);
}

int libvlc_media_player_set_renderer(libvlc_media_player_t *p_mi, libvlc_renderer_item_t *p_item)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_set_renderer, -1, p_mi, p_item);
}

void libvlc_video_set_callbacks(libvlc_media_player_t *mp, libvlc_video_lock_cb lock, libvlc_video_unlock_cb unlock, libvlc_video_display_cb display, void *opaque)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_callbacks, mp, lock, unlock, display, opaque)
}

void libvlc_video_set_format(libvlc_media_player_t *mp, const char *chroma, unsigned width, unsigned height, unsigned pitch)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_format, mp, chroma, width, height, pitch)
}

void libvlc_video_set_format_callbacks(libvlc_media_player_t *mp, libvlc_video_format_cb setup, libvlc_video_cleanup_cb cleanup)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_format_callbacks, mp, setup, cleanup)
}

bool libvlc_video_set_output_callbacks(libvlc_media_player_t *mp, libvlc_video_engine_t engine, libvlc_video_output_setup_cb setup_cb, libvlc_video_output_cleanup_cb cleanup_cb, libvlc_video_output_set_resize_cb resize_cb, libvlc_video_update_output_cb update_output_cb, libvlc_video_swap_cb swap_cb, libvlc_video_makeCurrent_cb makeCurrent_cb, libvlc_video_getProcAddress_cb getProcAddress_cb, libvlc_video_frameMetadata_cb metadata_cb, libvlc_video_output_select_plane_cb select_plane_cb, void *opaque)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_video_set_output_callbacks, false, mp, engine, setup_cb, cleanup_cb, resize_cb, update_output_cb, swap_cb, makeCurrent_cb, getProcAddress_cb, metadata_cb, select_plane_cb, opaque);
}

void libvlc_media_player_set_nsobject(libvlc_media_player_t *p_mi, void *drawable)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_set_nsobject, p_mi, drawable)
}

void *libvlc_media_player_get_nsobject(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_nsobject, nullptr, p_mi);
}

void libvlc_media_player_set_xwindow(libvlc_media_player_t *p_mi, uint32_t drawable)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_set_xwindow, p_mi, drawable)
}

uint32_t libvlc_media_player_get_xwindow(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_xwindow, 0, p_mi);
}

void libvlc_media_player_set_hwnd(libvlc_media_player_t *p_mi, void *drawable)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_set_hwnd, p_mi, drawable)
}

void *libvlc_media_player_get_hwnd(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_hwnd, nullptr, p_mi);
}

void libvlc_media_player_set_android_context(libvlc_media_player_t *p_mi, void *p_awindow_handler)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_set_android_context, p_mi, p_awindow_handler)
}

void libvlc_audio_set_callbacks(libvlc_media_player_t *mp, libvlc_audio_play_cb play, libvlc_audio_pause_cb pause, libvlc_audio_resume_cb resume, libvlc_audio_flush_cb flush, libvlc_audio_drain_cb drain, void *opaque)
{
    WWX190_CALL_VLCAPI(libvlc_audio_set_callbacks, mp, play, pause, resume, flush, drain, opaque)
}

void libvlc_audio_set_volume_callback(libvlc_media_player_t *mp, libvlc_audio_set_volume_cb set_volume)
{
    WWX190_CALL_VLCAPI(libvlc_audio_set_volume_callback, mp, set_volume)
}

void libvlc_audio_set_format_callbacks(libvlc_media_player_t *mp, libvlc_audio_setup_cb setup, libvlc_audio_cleanup_cb cleanup)
{
    WWX190_CALL_VLCAPI(libvlc_audio_set_format_callbacks, mp, setup, cleanup)
}

void libvlc_audio_set_format(libvlc_media_player_t *mp, const char *format, unsigned rate, unsigned channels)
{
    WWX190_CALL_VLCAPI(libvlc_audio_set_format, mp, format, rate, channels)
}

libvlc_time_t libvlc_media_player_get_length(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_length, -1, p_mi);
}

libvlc_time_t libvlc_media_player_get_time(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_time, -1, p_mi);
}

int libvlc_media_player_set_time(libvlc_media_player_t *p_mi, libvlc_time_t i_time, bool b_fast)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_set_time, -1, p_mi, i_time, b_fast);
}

float libvlc_media_player_get_position(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_position, 0.0, p_mi);
}

int libvlc_media_player_set_position(libvlc_media_player_t *p_mi, float f_pos, bool b_fast)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_set_position, -1, p_mi, f_pos, b_fast);
}

void libvlc_media_player_set_chapter(libvlc_media_player_t *p_mi, int i_chapter)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_set_chapter, p_mi, i_chapter)
}

int libvlc_media_player_get_chapter(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_chapter, -1, p_mi);
}

int libvlc_media_player_get_chapter_count(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_chapter_count, -1, p_mi);
}

int libvlc_media_player_get_chapter_count_for_title(libvlc_media_player_t *p_mi, int i_title)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_chapter_count_for_title, -1, p_mi, i_title);
}

void libvlc_media_player_set_title(libvlc_media_player_t *p_mi, int i_title)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_set_title, p_mi, i_title)
}

int libvlc_media_player_get_title(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_title, -1, p_mi);
}

int libvlc_media_player_get_title_count(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_title_count, -1, p_mi);
}

void libvlc_media_player_previous_chapter(libvlc_media_player_t *p_mi)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_previous_chapter, p_mi)
}

void libvlc_media_player_next_chapter(libvlc_media_player_t *p_mi)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_next_chapter, p_mi)
}

float libvlc_media_player_get_rate(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_rate, 0.0, p_mi);
}

int libvlc_media_player_set_rate(libvlc_media_player_t *p_mi, float rate)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_set_rate, -1, p_mi, rate);
}

libvlc_state_t libvlc_media_player_get_state(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_state, libvlc_NothingSpecial, p_mi);
}

unsigned libvlc_media_player_has_vout(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_has_vout, 0, p_mi);
}

bool libvlc_media_player_is_seekable(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_is_seekable, false, p_mi);
}

bool libvlc_media_player_can_pause(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_can_pause, false, p_mi);
}

bool libvlc_media_player_program_scrambled(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_program_scrambled, false, p_mi);
}

void libvlc_media_player_next_frame(libvlc_media_player_t *p_mi)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_next_frame, p_mi)
}

void libvlc_media_player_navigate(libvlc_media_player_t *p_mi, unsigned navigate)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_navigate, p_mi, navigate)
}

void libvlc_media_player_set_video_title_display(libvlc_media_player_t *p_mi, libvlc_position_t position, unsigned int timeout)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_set_video_title_display, p_mi, position, timeout)
}

libvlc_media_tracklist_t *libvlc_media_player_get_tracklist(libvlc_media_player_t *p_mi, libvlc_track_type_t type)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_tracklist, nullptr, p_mi, type);
}

libvlc_media_track_t *libvlc_media_player_get_selected_track(libvlc_media_player_t *p_mi, libvlc_track_type_t type)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_selected_track, nullptr, p_mi, type);
}

libvlc_media_track_t *libvlc_media_player_get_track_from_id(libvlc_media_player_t *p_mi, const char *psz_id)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_track_from_id, nullptr, p_mi, psz_id);
}

void libvlc_media_player_select_track(libvlc_media_player_t *p_mi, const libvlc_media_track_t *track)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_select_track, p_mi, track)
}

void libvlc_media_player_unselect_track_type(libvlc_media_player_t *p_mi, libvlc_track_type_t type)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_unselect_track_type, p_mi, type)
}

void libvlc_media_player_select_tracks(libvlc_media_player_t *p_mi, libvlc_track_type_t type, const libvlc_media_track_t **tracks, size_t track_count)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_select_tracks, p_mi, type, tracks, track_count)
}

void libvlc_media_player_select_tracks_by_ids(libvlc_media_player_t *p_mi, libvlc_track_type_t type, const char *psz_ids)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_select_tracks_by_ids, p_mi, type, psz_ids)
}

int libvlc_media_player_add_slave(libvlc_media_player_t *p_mi, libvlc_media_slave_type_t i_type, const char *psz_uri, bool b_select)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_add_slave, -1, p_mi, i_type, psz_uri, b_select);
}

void libvlc_player_program_delete(libvlc_player_program_t *program)
{
    WWX190_CALL_VLCAPI(libvlc_player_program_delete, program)
}

size_t libvlc_player_programlist_count(const libvlc_player_programlist_t *list)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_player_programlist_count, -1, list);
}

libvlc_player_program_t *libvlc_player_programlist_at(libvlc_player_programlist_t *list, size_t index)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_player_programlist_at, nullptr, list, index);
}

void libvlc_player_programlist_delete(libvlc_player_programlist_t *list)
{
    WWX190_CALL_VLCAPI(libvlc_player_programlist_delete, list)
}

void libvlc_media_player_select_program_id(libvlc_media_player_t *p_mi, int i_group_id)
{
    WWX190_CALL_VLCAPI(libvlc_media_player_select_program_id, p_mi, i_group_id)
}

libvlc_player_program_t *libvlc_media_player_get_selected_program(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_selected_program, nullptr, p_mi);
}

libvlc_player_program_t *libvlc_media_player_get_program_from_id(libvlc_media_player_t *p_mi, int i_group_id)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_program_from_id, nullptr, p_mi, i_group_id);
}

libvlc_player_programlist_t *libvlc_media_player_get_programlist(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_programlist, nullptr, p_mi);
}

void libvlc_toggle_fullscreen(libvlc_media_player_t *p_mi)
{
    WWX190_CALL_VLCAPI(libvlc_toggle_fullscreen, p_mi)
}

void libvlc_set_fullscreen(libvlc_media_player_t *p_mi, bool b_fullscreen)
{
    WWX190_CALL_VLCAPI(libvlc_set_fullscreen, p_mi, b_fullscreen)
}

bool libvlc_get_fullscreen(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_get_fullscreen, false, p_mi);
}

void libvlc_video_set_key_input(libvlc_media_player_t *p_mi, unsigned on)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_key_input, p_mi, on)
}

void libvlc_video_set_mouse_input(libvlc_media_player_t *p_mi, unsigned on)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_mouse_input, p_mi, on)
}

int libvlc_video_get_size(libvlc_media_player_t *p_mi, unsigned num, unsigned *px, unsigned *py)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_video_get_size, -1, p_mi, num, px, py);
}

int libvlc_video_get_cursor(libvlc_media_player_t *p_mi, unsigned num, int *px, int *py)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_video_get_cursor, -1, p_mi, num, px, py);
}

float libvlc_video_get_scale(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_video_get_scale, 0.0, p_mi);
}

void libvlc_video_set_scale(libvlc_media_player_t *p_mi, float f_factor)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_scale, p_mi, f_factor)
}

char *libvlc_video_get_aspect_ratio(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_video_get_aspect_ratio, nullptr, p_mi);
}

void libvlc_video_set_aspect_ratio(libvlc_media_player_t *p_mi, const char *psz_aspect)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_aspect_ratio, p_mi, psz_aspect)
}

libvlc_video_viewpoint_t *libvlc_video_new_viewpoint()
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_video_new_viewpoint, nullptr);
}

int libvlc_video_update_viewpoint(libvlc_media_player_t *p_mi, const libvlc_video_viewpoint_t *p_viewpoint, bool b_absolute)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_video_update_viewpoint, -1, p_mi, p_viewpoint, b_absolute);
}

int64_t libvlc_video_get_spu_delay(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_video_get_spu_delay, -1, p_mi);
}

float libvlc_video_get_spu_text_scale(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_video_get_spu_text_scale, 0.0, p_mi);
}

void libvlc_video_set_spu_text_scale(libvlc_media_player_t *p_mi, float f_scale)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_spu_text_scale, p_mi, f_scale)
}

int libvlc_video_set_spu_delay(libvlc_media_player_t *p_mi, int64_t i_delay)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_spu_delay, p_mi, i_delay)
}

int libvlc_media_player_get_full_title_descriptions(libvlc_media_player_t *p_mi, libvlc_title_description_t ***titles)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_full_title_descriptions, -1, p_mi, titles);
}

void libvlc_title_descriptions_release(libvlc_title_description_t **p_titles, unsigned i_count)
{
    WWX190_CALL_VLCAPI(libvlc_title_descriptions_release, p_titles, i_count)
}

int libvlc_media_player_get_full_chapter_descriptions(libvlc_media_player_t *p_mi, int i_chapters_of_title, libvlc_chapter_description_t ***pp_chapters)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_full_chapter_descriptions, -1, p_mi, i_chapters_of_title, pp_chapters);
}

void libvlc_chapter_descriptions_release(libvlc_chapter_description_t **p_chapters, unsigned i_count)
{
    WWX190_CALL_VLCAPI(libvlc_chapter_descriptions_release, p_chapters, i_count)
}

void libvlc_video_set_crop_ratio(libvlc_media_player_t *mp, unsigned num, unsigned den)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_crop_ratio, mp, num, den)
}

void libvlc_video_set_crop_window(libvlc_media_player_t *mp, unsigned x, unsigned y, unsigned width, unsigned height)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_crop_window, mp, x, y, width, height)
}

void libvlc_video_set_crop_border(libvlc_media_player_t *mp, unsigned left, unsigned right, unsigned top, unsigned bottom)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_crop_border, mp, left, right, top, bottom)
}

int libvlc_video_get_teletext(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_video_get_teletext, -1, p_mi);
}

void libvlc_video_set_teletext(libvlc_media_player_t *p_mi, int i_page)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_teletext, p_mi, i_page)
}

int libvlc_video_take_snapshot(libvlc_media_player_t *p_mi, unsigned num, const char *psz_filepath, unsigned int i_width, unsigned int i_height)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_video_take_snapshot, -1, p_mi, num, psz_filepath, i_width, i_height);
}

void libvlc_video_set_deinterlace(libvlc_media_player_t *p_mi, int deinterlace, const char *psz_mode)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_deinterlace, p_mi, deinterlace, psz_mode)
}

int libvlc_video_get_marquee_int(libvlc_media_player_t *p_mi, unsigned option)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_video_get_marquee_int, -1, p_mi, option);
}

void libvlc_video_set_marquee_int(libvlc_media_player_t *p_mi, unsigned option, int i_val)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_marquee_int, p_mi, option, i_val)
}

void libvlc_video_set_marquee_string(libvlc_media_player_t *p_mi, unsigned option, const char *psz_text)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_marquee_string, p_mi, option, psz_text)
}

int libvlc_video_get_logo_int(libvlc_media_player_t *p_mi, unsigned option)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_video_get_logo_int, -1, p_mi, option);
}

void libvlc_video_set_logo_int(libvlc_media_player_t *p_mi, unsigned option, int value)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_logo_int, p_mi, option, value)
}

void libvlc_video_set_logo_string(libvlc_media_player_t *p_mi, unsigned option, const char *psz_value)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_logo_string, p_mi, option, psz_value)
}

int libvlc_video_get_adjust_int(libvlc_media_player_t *p_mi, unsigned option)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_video_get_adjust_int, -1, p_mi, option);
}

void libvlc_video_set_adjust_int(libvlc_media_player_t *p_mi, unsigned option, int value)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_adjust_int, p_mi, option, value)
}

float libvlc_video_get_adjust_float(libvlc_media_player_t *p_mi, unsigned option)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_video_get_adjust_float, 0.0, p_mi, option);
}

void libvlc_video_set_adjust_float(libvlc_media_player_t *p_mi, unsigned option, float value)
{
    WWX190_CALL_VLCAPI(libvlc_video_set_adjust_float, p_mi, option, value)
}

libvlc_audio_output_t *libvlc_audio_output_list_get(libvlc_instance_t *p_instance)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_output_list_get, nullptr, p_instance);
}

void libvlc_audio_output_list_release(libvlc_audio_output_t *p_list)
{
    WWX190_CALL_VLCAPI(libvlc_audio_output_list_release, p_list)
}

int libvlc_audio_output_set(libvlc_media_player_t *p_mi, const char *psz_name)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_output_set, -1, p_mi, psz_name);
}

libvlc_audio_output_device_t *libvlc_audio_output_device_enum(libvlc_media_player_t *mp)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_output_device_enum, nullptr, mp);
}

libvlc_audio_output_device_t *libvlc_audio_output_device_list_get(libvlc_instance_t *p_instance, const char *aout)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_output_device_list_get, nullptr, p_instance, aout);
}

void libvlc_audio_output_device_list_release(libvlc_audio_output_device_t *p_list)
{
    WWX190_CALL_VLCAPI(libvlc_audio_output_device_list_release, p_list)
}

void libvlc_audio_output_device_set(libvlc_media_player_t *mp, const char *module, const char *device_id)
{
    WWX190_CALL_VLCAPI(libvlc_audio_output_device_set, mp, module, device_id)
}

char *libvlc_audio_output_device_get(libvlc_media_player_t *mp)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_output_device_get, nullptr, mp);
}

void libvlc_audio_toggle_mute(libvlc_media_player_t *p_mi)
{
    WWX190_CALL_VLCAPI(libvlc_audio_toggle_mute, p_mi)
}

int libvlc_audio_get_mute(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_get_mute, -1, p_mi);
}

void libvlc_audio_set_mute(libvlc_media_player_t *p_mi, int status)
{
    WWX190_CALL_VLCAPI(libvlc_audio_set_mute, p_mi, status)
}

int libvlc_audio_get_volume(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_get_volume, -1, p_mi);
}

int libvlc_audio_set_volume(libvlc_media_player_t *p_mi, int i_volume)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_set_volume, -1, p_mi, i_volume);
}

int libvlc_audio_get_channel(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_get_channel, -1, p_mi);
}

int libvlc_audio_set_channel(libvlc_media_player_t *p_mi, int channel)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_set_channel, -1, p_mi, channel);
}

int64_t libvlc_audio_get_delay(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_get_delay, -1, p_mi);
}

int libvlc_audio_set_delay(libvlc_media_player_t *p_mi, int64_t i_delay)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_set_delay, -1, p_mi, i_delay);
}

unsigned libvlc_audio_equalizer_get_preset_count()
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_equalizer_get_preset_count, 0);
}

const char *libvlc_audio_equalizer_get_preset_name(unsigned u_index)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_equalizer_get_preset_name, nullptr, u_index);
}

unsigned libvlc_audio_equalizer_get_band_count()
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_equalizer_get_band_count, 0);
}

float libvlc_audio_equalizer_get_band_frequency(unsigned u_index)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_equalizer_get_band_frequency, 0.0, u_index);
}

libvlc_equalizer_t *libvlc_audio_equalizer_new()
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_equalizer_new, nullptr);
}

libvlc_equalizer_t *libvlc_audio_equalizer_new_from_preset(unsigned u_index)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_equalizer_new_from_preset, nullptr, u_index);
}

void libvlc_audio_equalizer_release(libvlc_equalizer_t *p_equalizer)
{
    WWX190_CALL_VLCAPI(libvlc_audio_equalizer_release, p_equalizer)
}

int libvlc_audio_equalizer_set_preamp(libvlc_equalizer_t *p_equalizer, float f_preamp)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_equalizer_set_preamp, -1, p_equalizer, f_preamp);
}

float libvlc_audio_equalizer_get_preamp(libvlc_equalizer_t *p_equalizer)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_equalizer_get_preamp, 0.0, p_equalizer);
}

int libvlc_audio_equalizer_set_amp_at_index(libvlc_equalizer_t *p_equalizer, float f_amp, unsigned u_band)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_equalizer_set_amp_at_index, -1, p_equalizer, f_amp, u_band);
}

float libvlc_audio_equalizer_get_amp_at_index(libvlc_equalizer_t *p_equalizer, unsigned u_band)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_audio_equalizer_get_amp_at_index, 0.0, p_equalizer, u_band);
}

int libvlc_media_player_set_equalizer(libvlc_media_player_t *p_mi, libvlc_equalizer_t *p_equalizer)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_set_equalizer, -1, p_mi, p_equalizer);
}

int libvlc_media_player_get_role(libvlc_media_player_t *p_mi)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_get_role, -1, p_mi);
}

int libvlc_media_player_set_role(libvlc_media_player_t *p_mi, unsigned role)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_player_set_role, -1, p_mi, role);
}

// libvlc_media_list.h

libvlc_media_list_t *libvlc_media_list_new()
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_new, nullptr);
}

void libvlc_media_list_release(libvlc_media_list_t *p_ml)
{
    WWX190_CALL_VLCAPI(libvlc_media_list_release, p_ml)
}

void libvlc_media_list_retain(libvlc_media_list_t *p_ml)
{
    WWX190_CALL_VLCAPI(libvlc_media_list_retain, p_ml)
}

void libvlc_media_list_set_media(libvlc_media_list_t *p_ml, libvlc_media_t *p_md)
{
    WWX190_CALL_VLCAPI(libvlc_media_list_set_media, p_ml, p_md)
}

libvlc_media_t *libvlc_media_list_media(libvlc_media_list_t *p_ml)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_media, nullptr, p_ml);
}

int libvlc_media_list_add_media(libvlc_media_list_t *p_ml, libvlc_media_t *p_md)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_add_media, -1, p_ml, p_md);
}

int libvlc_media_list_insert_media(libvlc_media_list_t *p_ml, libvlc_media_t *p_md, int i_pos)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_insert_media, -1, p_ml, p_md, i_pos);
}

int libvlc_media_list_remove_index(libvlc_media_list_t *p_ml, int i_pos)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_remove_index, -1, p_ml, i_pos);
}

int libvlc_media_list_count(libvlc_media_list_t *p_ml)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_count, -1, p_ml);
}

libvlc_media_t *libvlc_media_list_item_at_index(libvlc_media_list_t *p_ml, int i_pos)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_item_at_index, nullptr, p_ml, i_pos);
}

int libvlc_media_list_index_of_item(libvlc_media_list_t *p_ml, libvlc_media_t *p_md)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_index_of_item, -1, p_ml, p_md);
}

bool libvlc_media_list_is_readonly(libvlc_media_list_t *p_ml)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_is_readonly, false, p_ml);
}

void libvlc_media_list_lock(libvlc_media_list_t *p_ml)
{
    WWX190_CALL_VLCAPI(libvlc_media_list_lock, p_ml)
}

void libvlc_media_list_unlock(libvlc_media_list_t *p_ml)
{
    WWX190_CALL_VLCAPI(libvlc_media_list_unlock, p_ml)
}

libvlc_event_manager_t *libvlc_media_list_event_manager(libvlc_media_list_t *p_ml)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_event_manager, nullptr, p_ml);
}

// libvlc_media_list_player.h

libvlc_media_list_player_t *libvlc_media_list_player_new(libvlc_instance_t *p_instance)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_player_new, nullptr, p_instance);
}

void libvlc_media_list_player_release(libvlc_media_list_player_t *p_mlp)
{
    WWX190_CALL_VLCAPI(libvlc_media_list_player_release, p_mlp)
}

void libvlc_media_list_player_retain(libvlc_media_list_player_t *p_mlp)
{
    WWX190_CALL_VLCAPI(libvlc_media_list_player_retain, p_mlp)
}

libvlc_event_manager_t *libvlc_media_list_player_event_manager(libvlc_media_list_player_t *p_mlp)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_player_event_manager, nullptr, p_mlp);
}

void libvlc_media_list_player_set_media_player(libvlc_media_list_player_t *p_mlp, libvlc_media_player_t *p_mi)
{
    WWX190_CALL_VLCAPI(libvlc_media_list_player_set_media_player, p_mlp, p_mi)
}

libvlc_media_player_t *libvlc_media_list_player_get_media_player(libvlc_media_list_player_t *p_mlp)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_player_get_media_player, nullptr, p_mlp);
}

void libvlc_media_list_player_set_media_list(libvlc_media_list_player_t *p_mlp, libvlc_media_list_t *p_mlist)
{
    WWX190_CALL_VLCAPI(libvlc_media_list_player_set_media_list, p_mlp, p_mlist)
}

void libvlc_media_list_player_play(libvlc_media_list_player_t *p_mlp)
{
    WWX190_CALL_VLCAPI(libvlc_media_list_player_play, p_mlp)
}

void libvlc_media_list_player_pause(libvlc_media_list_player_t *p_mlp)
{
    WWX190_CALL_VLCAPI(libvlc_media_list_player_pause, p_mlp)
}

void libvlc_media_list_player_set_pause(libvlc_media_list_player_t *p_mlp, int do_pause)
{
    WWX190_CALL_VLCAPI(libvlc_media_list_player_set_pause, p_mlp, do_pause)
}

bool libvlc_media_list_player_is_playing(libvlc_media_list_player_t *p_mlp)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_player_is_playing, false, p_mlp);
}

libvlc_state_t libvlc_media_list_player_get_state(libvlc_media_list_player_t *p_mlp)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_player_get_state, libvlc_NothingSpecial, p_mlp);
}

int libvlc_media_list_player_play_item_at_index(libvlc_media_list_player_t *p_mlp, int i_index)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_player_play_item_at_index, -1, p_mlp, i_index);
}

int libvlc_media_list_player_play_item(libvlc_media_list_player_t *p_mlp, libvlc_media_t *p_md)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_player_play_item, -1, p_mlp, p_md);
}

void libvlc_media_list_player_stop_async(libvlc_media_list_player_t *p_mlp)
{
    WWX190_CALL_VLCAPI(libvlc_media_list_player_stop_async, p_mlp)
}

int libvlc_media_list_player_next(libvlc_media_list_player_t *p_mlp)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_player_next, -1, p_mlp);
}

int libvlc_media_list_player_previous(libvlc_media_list_player_t *p_mlp)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_list_player_previous, -1, p_mlp);
}

void libvlc_media_list_player_set_playback_mode(libvlc_media_list_player_t *p_mlp, libvlc_playback_mode_t e_mode)
{
    WWX190_CALL_VLCAPI(libvlc_media_list_player_set_playback_mode, p_mlp, e_mode)
}

// libvlc_media_discoverer.h

libvlc_media_discoverer_t *libvlc_media_discoverer_new(libvlc_instance_t *p_inst, const char *psz_name)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_discoverer_new, nullptr, p_inst, psz_name);
}

int libvlc_media_discoverer_start(libvlc_media_discoverer_t *p_mdis)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_discoverer_start, -1, p_mdis);
}

void libvlc_media_discoverer_stop(libvlc_media_discoverer_t *p_mdis)
{
    WWX190_CALL_VLCAPI(libvlc_media_discoverer_stop, p_mdis)
}

void libvlc_media_discoverer_release(libvlc_media_discoverer_t *p_mdis)
{
    WWX190_CALL_VLCAPI(libvlc_media_discoverer_release, p_mdis)
}

libvlc_media_list_t *libvlc_media_discoverer_media_list(libvlc_media_discoverer_t *p_mdis)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_discoverer_media_list, nullptr, p_mdis);
}

bool libvlc_media_discoverer_is_running(libvlc_media_discoverer_t *p_mdis)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_discoverer_is_running, false, p_mdis);
}

size_t libvlc_media_discoverer_list_get(libvlc_instance_t *p_inst, libvlc_media_discoverer_category_t i_cat, libvlc_media_discoverer_description_t ***ppp_services)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_media_discoverer_list_get, 0, p_inst, i_cat, ppp_services);
}

void libvlc_media_discoverer_list_release(libvlc_media_discoverer_description_t **pp_services, size_t i_count)
{
    WWX190_CALL_VLCAPI(libvlc_media_discoverer_list_release, pp_services, i_count)
}

// libvlc_dialog.h

void libvlc_dialog_set_callbacks(libvlc_instance_t *p_instance, const libvlc_dialog_cbs *p_cbs, void *p_data)
{
    WWX190_CALL_VLCAPI(libvlc_dialog_set_callbacks, p_instance, p_cbs, p_data)
}

void libvlc_dialog_set_context(libvlc_dialog_id *p_id, void *p_context)
{
    WWX190_CALL_VLCAPI(libvlc_dialog_set_context, p_id, p_context)
}

void *libvlc_dialog_get_context(libvlc_dialog_id *p_id)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_dialog_get_context, nullptr, p_id);
}

int libvlc_dialog_post_login(libvlc_dialog_id *p_id, const char *psz_username, const char *psz_password, bool b_store)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_dialog_post_login, -1, p_id, psz_username, psz_password, b_store);
}

int libvlc_dialog_post_action(libvlc_dialog_id *p_id, int i_action)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_dialog_post_action, -1, p_id, i_action);
}

int libvlc_dialog_dismiss(libvlc_dialog_id *p_id)
{
    return WWX190_CALL_VLCAPI_RETURN(libvlc_dialog_dismiss, -1, p_id);
}
