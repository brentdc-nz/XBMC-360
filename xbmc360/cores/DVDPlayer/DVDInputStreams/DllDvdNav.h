#pragma once

/*
 * Static-linking wrapper around libdvdnav/libdvdread for the Xbox 360 port.
 * Interface mirrors the original xbmc4xbox DllDvdNav (DllDynamic) 1:1, so a
 * future DLL-loader migration only requires re-binding this class.
 */

#define DVDNAV_COMPILE
/* use the locally installed (XBMC-adapted) header copies next to this file,
   matching the original xbmc4xbox DllDvdNav include layout */
#include "dvdnav/config.h"
#include "dvdnav/dvdnav.h"

#ifndef WIN32
#define WIN32
#endif
#define HAVE_CONFIG_H
/* CRITICAL_SECTION / struct timeval for the win32 wrappers in dvdnav_internal.h */
#include <xtl.h>
extern "C" {
#include "dvdnav/dvdnav_internal.h"
#include "dvdnav/vm.h"
#include "dvdnav/dvd_types.h"
/* XBMC-added functions exported by dvdnav.c but not declared in dvdnav.h */
vm_t* dvdnav_get_vm(dvdnav_t *self);
int dvdnav_get_nr_of_subtitle_streams(dvdnav_t *self);
int dvdnav_get_nr_of_audio_streams(dvdnav_t *self);
int dvdnav_get_button_info(dvdnav_t* self, int alpha[2][4], int color[2][4]);
dvdnav_status_t dvdnav_jump_to_sector_by_time(dvdnav_t *self, uint64_t timepos, int32_t still_time);
}

class DllDvdNavInterface
{
public:
  virtual ~DllDvdNavInterface() {}
  virtual dvdnav_status_t dvdnav_open(dvdnav_t **dest, const char *path)=0;
  virtual dvdnav_status_t dvdnav_close(dvdnav_t *self)=0;
  virtual dvdnav_status_t dvdnav_reset(dvdnav_t *self)=0;
  virtual const char* dvdnav_err_to_string(dvdnav_t *self)=0;
  virtual dvdnav_status_t dvdnav_set_readahead_flag(dvdnav_t *self, int32_t read_ahead_flag)=0;
  virtual dvdnav_status_t dvdnav_set_PGC_positioning_flag(dvdnav_t *self, int32_t pgc_based_flag)=0;
  virtual dvdnav_status_t dvdnav_get_next_cache_block(dvdnav_t *self, uint8_t **buf, int32_t *event, int32_t *len)=0;
  virtual dvdnav_status_t dvdnav_free_cache_block(dvdnav_t *self, unsigned char *buf)=0;
  virtual dvdnav_status_t dvdnav_still_skip(dvdnav_t *self)=0;
  virtual dvdnav_status_t dvdnav_wait_skip(dvdnav_t *self)=0;
  virtual dvdnav_status_t dvdnav_stop(dvdnav_t *self)=0;
  virtual dvdnav_status_t dvdnav_button_select(dvdnav_t *self, pci_t *pci, int32_t button)=0;
  virtual dvdnav_status_t dvdnav_button_activate(dvdnav_t *self, pci_t *pci)=0;
  virtual dvdnav_status_t dvdnav_upper_button_select(dvdnav_t *self, pci_t *pci)=0;
  virtual dvdnav_status_t dvdnav_lower_button_select(dvdnav_t *self, pci_t *pci)=0;
  virtual dvdnav_status_t dvdnav_right_button_select(dvdnav_t *self, pci_t *pci)=0;
  virtual dvdnav_status_t dvdnav_left_button_select(dvdnav_t *self, pci_t *pci)=0;
  virtual dvdnav_status_t dvdnav_sector_search(dvdnav_t *self, uint64_t offset, int32_t origin)=0;
  virtual pci_t* dvdnav_get_current_nav_pci(dvdnav_t *self)=0;
  virtual dsi_t* dvdnav_get_current_nav_dsi(dvdnav_t *self)=0;
  virtual dvdnav_status_t dvdnav_get_position(dvdnav_t *self, uint32_t *pos, uint32_t *len)=0;
  virtual dvdnav_status_t dvdnav_current_title_info(dvdnav_t *self, int32_t *title, int32_t *part)=0;
  virtual dvdnav_status_t dvdnav_spu_language_select(dvdnav_t *self, char *code)=0;
  virtual dvdnav_status_t dvdnav_audio_language_select(dvdnav_t *self, char *code)=0;
  virtual dvdnav_status_t dvdnav_menu_language_select(dvdnav_t *self, char *code)=0;
  virtual int8_t dvdnav_is_domain_vts(dvdnav_t *self)=0;
  virtual int8_t dvdnav_get_active_spu_stream(dvdnav_t *self)=0;
  virtual int8_t dvdnav_get_spu_logical_stream(dvdnav_t *self, uint8_t subp_num)=0;
  virtual uint16_t dvdnav_spu_stream_to_lang(dvdnav_t *self, uint8_t stream)=0;
  virtual dvdnav_status_t dvdnav_get_current_highlight(dvdnav_t *self, int32_t *button)=0;
  virtual dvdnav_status_t dvdnav_menu_call(dvdnav_t *self, DVDMenuID_t menu)=0;
  virtual dvdnav_status_t dvdnav_prev_pg_search(dvdnav_t *self)=0;
  virtual dvdnav_status_t dvdnav_next_pg_search(dvdnav_t *self)=0;
  virtual dvdnav_status_t dvdnav_get_highlight_area(pci_t *nav_pci , int32_t button, int32_t mode, dvdnav_highlight_area_t *highlight)=0;
  virtual dvdnav_status_t dvdnav_go_up(dvdnav_t *self)=0;
  virtual int8_t dvdnav_get_active_audio_stream(dvdnav_t *self)=0;
  virtual uint16_t dvdnav_audio_stream_to_lang(dvdnav_t *self, uint8_t stream)=0;
  virtual vm_t* dvdnav_get_vm(dvdnav_t *self)=0;
  virtual int dvdnav_get_nr_of_subtitle_streams(dvdnav_t *self)=0;
  virtual int dvdnav_get_nr_of_audio_streams(dvdnav_t *self)=0;
  virtual int dvdnav_get_button_info(dvdnav_t* self, int alpha[2][4], int color[2][4])=0;
  virtual int8_t dvdnav_get_audio_logical_stream(dvdnav_t *self, uint8_t audio_num)=0;
  virtual dvdnav_status_t dvdnav_set_region_mask(dvdnav_t *self, int32_t region_mask)=0;
  virtual uint8_t dvdnav_get_video_aspect(dvdnav_t *self)=0;
  virtual uint8_t dvdnav_get_video_scale_permission(dvdnav_t *self)=0;
  virtual dvdnav_status_t dvdnav_get_number_of_titles(dvdnav_t *self, int32_t *titles)=0;
  virtual dvdnav_status_t dvdnav_get_number_of_parts(dvdnav_t *self, int32_t title, int32_t *parts)=0;
  virtual dvdnav_status_t dvdnav_title_play(dvdnav_t *self, int32_t title)=0;
  virtual dvdnav_status_t dvdnav_part_play(dvdnav_t *self, int32_t title, int32_t part)=0;
  virtual dvdnav_status_t dvdnav_subpicture_change(dvdnav_t *self, int32_t subpicture)=0;
  virtual dvdnav_status_t dvdnav_audio_change(dvdnav_t *self, int32_t audio)=0;
  virtual dvdnav_status_t dvdnav_get_audio_info(dvdnav_t * self, int32_t streamid, audio_attr_t* audio_attributes)=0;
  virtual dvdnav_status_t dvdnav_get_stitle_info(dvdnav_t * self, int32_t streamid, subp_attr_t* stitle_attributes)=0;
  virtual dvdnav_status_t dvdnav_time_search(dvdnav_t * self, uint64_t timepos)=0;
  virtual dvdnav_status_t dvdnav_jump_to_sector_by_time(dvdnav_t * self, uint64_t timepos, int32_t still_time)=0;
  virtual int64_t dvdnav_convert_time(dvd_time_t *time)=0;
  virtual dvdnav_status_t dvdnav_get_state(dvdnav_t *self, dvd_state_t *save_state)=0;
  virtual dvdnav_status_t dvdnav_set_state(dvdnav_t *self, dvd_state_t *save_state)=0;
  virtual dvdnav_status_t dvdnav_get_angle_info(dvdnav_t *self, int32_t *current_angle,int32_t *number_of_angles)=0;
  virtual dvdnav_status_t dvdnav_mouse_activate(dvdnav_t *self, pci_t *pci, int32_t x, int32_t y)=0;
  virtual dvdnav_status_t dvdnav_mouse_select(dvdnav_t *self, pci_t *pci, int32_t x, int32_t y)=0;
};

/* Static binding: all calls resolve directly to the linked libdvdnav.lib */
class DllDvdNav : public DllDvdNavInterface
{
public:
  virtual ~DllDvdNav() {}
  /* DLL-loader compatibility no-ops (static build) */
  bool Load() { return true; }
  void Unload() {}
  virtual dvdnav_status_t dvdnav_open(dvdnav_t **dest, const char *path) { return ::dvdnav_open(dest, path); }
  virtual dvdnav_status_t dvdnav_close(dvdnav_t *self) { return ::dvdnav_close(self); }
  virtual dvdnav_status_t dvdnav_reset(dvdnav_t *self) { return ::dvdnav_reset(self); }
  virtual const char* dvdnav_err_to_string(dvdnav_t *self) { return ::dvdnav_err_to_string(self); }
  virtual dvdnav_status_t dvdnav_set_readahead_flag(dvdnav_t *self, int32_t read_ahead_flag) { return ::dvdnav_set_readahead_flag(self, read_ahead_flag); }
  virtual dvdnav_status_t dvdnav_set_PGC_positioning_flag(dvdnav_t *self, int32_t pgc_based_flag) { return ::dvdnav_set_PGC_positioning_flag(self, pgc_based_flag); }
  virtual dvdnav_status_t dvdnav_get_next_cache_block(dvdnav_t *self, uint8_t **buf, int32_t *event, int32_t *len) { return ::dvdnav_get_next_cache_block(self, buf, event, len); }
  virtual dvdnav_status_t dvdnav_free_cache_block(dvdnav_t *self, unsigned char *buf) { return ::dvdnav_free_cache_block(self, buf); }
  virtual dvdnav_status_t dvdnav_still_skip(dvdnav_t *self) { return ::dvdnav_still_skip(self); }
  virtual dvdnav_status_t dvdnav_wait_skip(dvdnav_t *self) { return ::dvdnav_wait_skip(self); }
  virtual dvdnav_status_t dvdnav_stop(dvdnav_t *self) { return ::dvdnav_stop(self); }
  virtual dvdnav_status_t dvdnav_button_select(dvdnav_t *self, pci_t *pci, int32_t button) { return ::dvdnav_button_select(self, pci, button); }
  virtual dvdnav_status_t dvdnav_button_activate(dvdnav_t *self, pci_t *pci) { return ::dvdnav_button_activate(self, pci); }
  virtual dvdnav_status_t dvdnav_upper_button_select(dvdnav_t *self, pci_t *pci) { return ::dvdnav_upper_button_select(self, pci); }
  virtual dvdnav_status_t dvdnav_lower_button_select(dvdnav_t *self, pci_t *pci) { return ::dvdnav_lower_button_select(self, pci); }
  virtual dvdnav_status_t dvdnav_right_button_select(dvdnav_t *self, pci_t *pci) { return ::dvdnav_right_button_select(self, pci); }
  virtual dvdnav_status_t dvdnav_left_button_select(dvdnav_t *self, pci_t *pci) { return ::dvdnav_left_button_select(self, pci); }
  virtual dvdnav_status_t dvdnav_sector_search(dvdnav_t *self, uint64_t offset, int32_t origin) { return ::dvdnav_sector_search(self, offset, origin); }
  virtual pci_t* dvdnav_get_current_nav_pci(dvdnav_t *self) { return ::dvdnav_get_current_nav_pci(self); }
  virtual dsi_t* dvdnav_get_current_nav_dsi(dvdnav_t *self) { return ::dvdnav_get_current_nav_dsi(self); }
  virtual dvdnav_status_t dvdnav_get_position(dvdnav_t *self, uint32_t *pos, uint32_t *len) { return ::dvdnav_get_position(self, pos, len); }
  virtual dvdnav_status_t dvdnav_current_title_info(dvdnav_t *self, int32_t *title, int32_t *part) { return ::dvdnav_current_title_info(self, title, part); }
  virtual dvdnav_status_t dvdnav_spu_language_select(dvdnav_t *self, char *code) { return ::dvdnav_spu_language_select(self, code); }
  virtual dvdnav_status_t dvdnav_audio_language_select(dvdnav_t *self, char *code) { return ::dvdnav_audio_language_select(self, code); }
  virtual dvdnav_status_t dvdnav_menu_language_select(dvdnav_t *self, char *code) { return ::dvdnav_menu_language_select(self, code); }
  virtual int8_t dvdnav_is_domain_vts(dvdnav_t *self) { return ::dvdnav_is_domain_vts(self); }
  virtual int8_t dvdnav_get_active_spu_stream(dvdnav_t *self) { return ::dvdnav_get_active_spu_stream(self); }
  virtual int8_t dvdnav_get_spu_logical_stream(dvdnav_t *self, uint8_t subp_num) { return ::dvdnav_get_spu_logical_stream(self, subp_num); }
  virtual uint16_t dvdnav_spu_stream_to_lang(dvdnav_t *self, uint8_t stream) { return ::dvdnav_spu_stream_to_lang(self, stream); }
  virtual dvdnav_status_t dvdnav_get_current_highlight(dvdnav_t *self, int32_t *button) { return ::dvdnav_get_current_highlight(self, button); }
  virtual dvdnav_status_t dvdnav_menu_call(dvdnav_t *self, DVDMenuID_t menu) { return ::dvdnav_menu_call(self, menu); }
  virtual dvdnav_status_t dvdnav_prev_pg_search(dvdnav_t *self) { return ::dvdnav_prev_pg_search(self); }
  virtual dvdnav_status_t dvdnav_next_pg_search(dvdnav_t *self) { return ::dvdnav_next_pg_search(self); }
  virtual dvdnav_status_t dvdnav_get_highlight_area(pci_t *nav_pci , int32_t button, int32_t mode, dvdnav_highlight_area_t *highlight) { return ::dvdnav_get_highlight_area(nav_pci, button, mode, highlight); }
  virtual dvdnav_status_t dvdnav_go_up(dvdnav_t *self) { return ::dvdnav_go_up(self); }
  virtual int8_t dvdnav_get_active_audio_stream(dvdnav_t *self) { return ::dvdnav_get_active_audio_stream(self); }
  virtual uint16_t dvdnav_audio_stream_to_lang(dvdnav_t *self, uint8_t stream) { return ::dvdnav_audio_stream_to_lang(self, stream); }
  virtual vm_t* dvdnav_get_vm(dvdnav_t *self) { return ::dvdnav_get_vm(self); }
  virtual int dvdnav_get_nr_of_subtitle_streams(dvdnav_t *self) { return ::dvdnav_get_nr_of_subtitle_streams(self); }
  virtual int dvdnav_get_nr_of_audio_streams(dvdnav_t *self) { return ::dvdnav_get_nr_of_audio_streams(self); }
  virtual int dvdnav_get_button_info(dvdnav_t* self, int alpha[2][4], int color[2][4]) { return ::dvdnav_get_button_info(self, alpha, color); }
  virtual int8_t dvdnav_get_audio_logical_stream(dvdnav_t *self, uint8_t audio_num) { return ::dvdnav_get_audio_logical_stream(self, audio_num); }
  virtual dvdnav_status_t dvdnav_set_region_mask(dvdnav_t *self, int32_t region_mask) { return ::dvdnav_set_region_mask(self, region_mask); }
  virtual uint8_t dvdnav_get_video_aspect(dvdnav_t *self) { return ::dvdnav_get_video_aspect(self); }
  virtual uint8_t dvdnav_get_video_scale_permission(dvdnav_t *self) { return ::dvdnav_get_video_scale_permission(self); }
  virtual dvdnav_status_t dvdnav_get_number_of_titles(dvdnav_t *self, int32_t *titles) { return ::dvdnav_get_number_of_titles(self, titles); }
  virtual dvdnav_status_t dvdnav_get_number_of_parts(dvdnav_t *self, int32_t title, int32_t *parts) { return ::dvdnav_get_number_of_parts(self, title, parts); }
  virtual dvdnav_status_t dvdnav_title_play(dvdnav_t *self, int32_t title) { return ::dvdnav_title_play(self, title); }
  virtual dvdnav_status_t dvdnav_part_play(dvdnav_t *self, int32_t title, int32_t part) { return ::dvdnav_part_play(self, title, part); }
  virtual dvdnav_status_t dvdnav_subpicture_change(dvdnav_t *self, int32_t subpicture) { return ::dvdnav_subpicture_change(self, subpicture); }
  virtual dvdnav_status_t dvdnav_audio_change(dvdnav_t *self, int32_t audio) { return ::dvdnav_audio_change(self, audio); }
  virtual dvdnav_status_t dvdnav_get_audio_info(dvdnav_t * self, int32_t streamid, audio_attr_t* audio_attributes) { return ::dvdnav_get_audio_info(self, streamid, audio_attributes); }
  virtual dvdnav_status_t dvdnav_get_stitle_info(dvdnav_t * self, int32_t streamid, subp_attr_t* stitle_attributes) { return ::dvdnav_get_stitle_info(self, streamid, stitle_attributes); }
  virtual dvdnav_status_t dvdnav_time_search(dvdnav_t * self, uint64_t timepos) { return ::dvdnav_time_search(self, timepos); }
  virtual dvdnav_status_t dvdnav_jump_to_sector_by_time(dvdnav_t * self, uint64_t timepos, int32_t still_time) { return ::dvdnav_jump_to_sector_by_time(self, timepos, still_time); }
  virtual int64_t dvdnav_convert_time(dvd_time_t *time) { return ::dvdnav_convert_time(time); }
  virtual dvdnav_status_t dvdnav_get_state(dvdnav_t *self, dvd_state_t *save_state) { return ::dvdnav_get_state(self, save_state); }
  virtual dvdnav_status_t dvdnav_set_state(dvdnav_t *self, dvd_state_t *save_state) { return ::dvdnav_set_state(self, save_state); }
  virtual dvdnav_status_t dvdnav_get_angle_info(dvdnav_t *self, int32_t *current_angle,int32_t *number_of_angles) { return ::dvdnav_get_angle_info(self, current_angle, number_of_angles); }
  virtual dvdnav_status_t dvdnav_mouse_activate(dvdnav_t *self, pci_t *pci, int32_t x, int32_t y) { return ::dvdnav_mouse_activate(self, pci, x, y); }
  virtual dvdnav_status_t dvdnav_mouse_select(dvdnav_t *self, pci_t *pci, int32_t x, int32_t y) { return ::dvdnav_mouse_select(self, pci, x, y); }
};
