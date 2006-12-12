/* gEDA - GPL Electronic Design Automation
 * gschem - gEDA Schematic Capture
 * Copyright (C) 1998-2000 Ales V. Hvezda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA
 */
#include <config.h>

#include <stdio.h>
#include <sys/stat.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <libgeda/libgeda.h>

#include "../include/globals.h"
#include "../include/prototype.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

/*! \brief */
struct gsubr_t {
  char* name;
  int req;
  int opt;
  int rst;
  SCM (*fnc)();
};

/*! \brief */
static struct gsubr_t gschem_funcs[] = {
  /* rc file */
  { "gschem-version",           1, 0, 0, g_rc_gschem_version },
  
  { "override-net-color",       7, 0, 0, g_rc_override_net_color },
  { "override-bus-color",       7, 0, 0, g_rc_override_bus_color },
  { "override-pin-color",       7, 0, 0, g_rc_override_pin_color },
  { "attribute-color",          7, 0, 0, g_rc_attribute_color },
  { "detached-attribute-color", 7, 0, 0, g_rc_detachedattr_color },
  { "text-color",               7, 0, 0, g_rc_text_color },
  { "logic-bubble-color",       7, 0, 0, g_rc_logic_bubble_color },
  { "lock-color",               7, 0, 0, g_rc_lock_color },
  { "zoom-box-color",           7, 0, 0, g_rc_zoom_box_color },
  { "net-color",                7, 0, 0, g_rc_net_color },
  { "bus-color",                7, 0, 0, g_rc_bus_color },
  { "pin-color",                7, 0, 0, g_rc_pin_color },
  { "graphic-color",            7, 0, 0, g_rc_graphic_color },
  { "grid-color",               7, 0, 0, g_rc_grid_color },
  { "background-color",         7, 0, 0, g_rc_background_color },
  { "select-color",             7, 0, 0, g_rc_select_color },
  { "boundingbox-color",        7, 0, 0, g_rc_boundingbox_color },
  { "net-endpoint-color",       7, 0, 0, g_rc_net_endpoint_color },
  { "stroke-color",             7, 0, 0, g_rc_stroke_color },
  { "output-color-background",  7, 0, 0, g_rc_output_color_background },
  { "freestyle-color",          7, 0, 0, g_rc_freestyle_color },

  { "net-endpoint-mode",         1, 0, 0, g_rc_net_endpoint_mode },
  { "net-midpoint-mode",         1, 0, 0, g_rc_net_midpoint_mode },
  { "net-style",                 1, 0, 0, g_rc_net_style },
  { "bus-style",                 1, 0, 0, g_rc_bus_style },
  { "pin-style",                 1, 0, 0, g_rc_pin_style },
  { "line-style",                1, 0, 0, g_rc_line_style },
  { "zoom-with-pan",             1, 0, 0, g_rc_zoom_with_pan },
  { "action-feedback-mode",      1, 0, 0, g_rc_action_feedback_mode },
  { "text-feedback",             1, 0, 0, g_rc_text_feedback },
  { "text-display-zoomfactor",   1, 0, 0, g_rc_text_display_zoomfactor },
  { "scrollbars",                1, 0, 0, g_rc_scrollbars },
  { "scrollbar-update",          1, 0, 0, g_rc_scrollbar_update },
  { "object-clipping",           1, 0, 0, g_rc_object_clipping },
  { "embed-components",          1, 0, 0, g_rc_embed_components },
  { "logging",                   1, 0, 0, g_rc_logging },
  { "text-size",                 1, 0, 0, g_rc_text_size },
  { "snap-size",                 1, 0, 0, g_rc_snap_size },

  { "text-caps-style",           1, 0, 0, g_rc_text_caps_style },
  { "logging-destination",       1, 0, 0, g_rc_logging_destination },

  { "attribute-name",            1, 0, 0, g_rc_attribute_name },
  { "paper-size",                2, 0, 0, g_rc_paper_size },
  { "paper-sizes",               3, 0, 0, g_rc_paper_sizes },
  /* text-output is old... will eventually be removed! */
  { "text-output",               1, 0, 0, g_rc_output_text }, 
  { "output-text",               1, 0, 0, g_rc_output_text },
  { "output-type",               1, 0, 0, g_rc_output_type },
  { "output-orientation",        1, 0, 0, g_rc_output_orientation },
  { "output-color",              1, 0, 0, g_rc_output_color },
  { "output-capstyle",           1, 0, 0, g_rc_output_capstyle },
  { "image-color",               1, 0, 0, g_rc_image_color },
  { "image-size",                2, 0, 0, g_rc_image_size },
  { "log-window",                1, 0, 0, g_rc_log_window },
  { "log-window-type",           1, 0, 0, g_rc_log_window_type },
  { "third-button",              1, 0, 0, g_rc_third_button },
  { "middle-button",             1, 0, 0, g_rc_middle_button },
  { "net-consolidate",           1, 0, 0, g_rc_net_consolidate },
  { "file-preview",              1, 0, 0, g_rc_file_preview },
  { "enforce-hierarchy",         1, 0, 0, g_rc_enforce_hierarchy },
  { "text-origin-marker",        1, 0, 0, g_rc_text_origin_marker },
  { "fast-mousepan",             1, 0, 0, g_rc_fast_mousepan },
  { "raise-dialog-boxes-on-expose", 1, 0, 0, g_rc_raise_dialog_boxes_on_expose },
  { "attribute-promotion",       1, 0, 0, g_rc_attribute_promotion },
  { "promote-invisible",         1, 0, 0, g_rc_promote_invisible },
  { "keep-invisible",            1, 0, 0, g_rc_keep_invisible },
  { "continue-component-place",  1, 0, 0, g_rc_continue_component_place },
  { "undo-levels",               1, 0, 0, g_rc_undo_levels },
  { "undo-control",              1, 0, 0, g_rc_undo_control },
  { "undo-type",                 1, 0, 0, g_rc_undo_type },

  { "drag-can-move",             1, 0, 0, g_rc_drag_can_move },

  { "draw-grips",                1, 0, 0, g_rc_draw_grips },
  { "netconn-rubberband",        1, 0, 0, g_rc_netconn_rubberband },
  { "sort-component-library",    1, 0, 0, g_rc_sort_component_library },
  { "add-menu",                  2, 0, 0, g_rc_add_menu },
  { "window-size",               2, 0, 0, g_rc_window_size },
  { "warp-cursor",               1, 0, 0, g_rc_warp_cursor },
  { "toolbars",                  1, 0, 0, g_rc_toolbars },
  { "handleboxes",               1, 0, 0, g_rc_handleboxes },
  { "setpagedevice-orientation", 1, 0, 0, g_rc_setpagedevice_orientation },
  { "setpagedevice-pagesize",    1, 0, 0, g_rc_setpagedevice_pagesize },
  { "bus-ripper-size",           1, 0, 0, g_rc_bus_ripper_size },
  { "bus-ripper-type",           1, 0, 0, g_rc_bus_ripper_type },
  { "bus-ripper-rotation",       1, 0, 0, g_rc_bus_ripper_rotation },
  { "force-boundingbox",         1, 0, 0, g_rc_force_boundingbox },
  { "grid-dot-size",             1, 0, 0, g_rc_grid_dot_size },
  { "grid-mode",		 1, 0, 0, g_rc_grid_mode },
  { "grid-fixed-threshold",   	 1, 0, 0, g_rc_grid_fixed_threshold },
  { "output-vector-threshold",   1, 0, 0, g_rc_output_vector_threshold },
  { "add-attribute-offset",      1, 0, 0, g_rc_add_attribute_offset },
  { "mousepan-gain",             1, 0, 0, g_rc_mousepan_gain },
  { "keyboardpan-gain",          1, 0, 0, g_rc_keyboardpan_gain },
  { "print-command",             1, 0, 0, g_rc_print_command },

  /* backup functions */
  { "auto-save-interval",        1, 0, 0, g_rc_auto_save_interval },

  /* general guile functions */
  { "gschem-print",              1, 0, 0, g_funcs_print },
  { "gschem-postscript",         1, 0, 0, g_funcs_postscript },
  { "gschem-image",              1, 0, 0, g_funcs_image },
  { "gschem-key-name",           1, 0, 0, g_funcs_key_name },
  { "gschem-key-value",          1, 0, 0, g_funcs_key_value },
  { "gschem-key-done",           0, 0, 0, g_funcs_key_done },
  { "gschem-use-rc-values",      0, 0, 0, g_funcs_use_rc_values },
  { "gschem-exit",               0, 0, 0, g_funcs_exit },
  { "gschem-log",                1, 0, 0, g_funcs_log },
  { "gschem-msg",                1, 0, 0, g_funcs_msg },
  { "gschem-confirm",            1, 0, 0, g_funcs_confirm },
  { "gschem-filesel",            2, 0, 1, g_funcs_filesel },

  /* keymapping callbacks */
  { "file-new-window",           0, 0, 0, g_keys_file_new_window },
  { "file-new",                  0, 0, 0, g_keys_file_new },
  { "file-open",                 0, 0, 0, g_keys_file_open },
  { "file-script",               0, 0, 0, g_keys_file_script },
  { "file-save",                 0, 0, 0, g_keys_file_save },
  { "file-save-as",              0, 0, 0, g_keys_file_save_as },
  { "file-save-all",             0, 0, 0, g_keys_file_save_all },
  { "file-print",                0, 0, 0, g_keys_file_print },
  { "file-image",                0, 0, 0, g_keys_file_write_png },
  { "file-close-window",         0, 0, 0, g_keys_file_close },
  { "file-quit",                 0, 0, 0, g_keys_file_quit },
  { "edit-undo",                 0, 0, 0, g_keys_edit_undo },
  { "edit-redo",                 0, 0, 0, g_keys_edit_redo },
  { "edit-select",               0, 0, 0, g_keys_edit_select },
  { "edit-copy",                 0, 0, 0, g_keys_edit_copy },
  { "edit-copy-hotkey",          0, 0, 0, g_keys_edit_copy_hotkey },
  { "edit-mcopy",                0, 0, 0, g_keys_edit_mcopy },
  { "edit-mcopy-hotkey",         0, 0, 0, g_keys_edit_mcopy_hotkey },
  { "edit-move",                 0, 0, 0, g_keys_edit_move },
  { "edit-move-hotkey",          0, 0, 0, g_keys_edit_move_hotkey },
  { "edit-delete",               0, 0, 0, g_keys_edit_delete },
  { "edit-rotate-90",            0, 0, 0, g_keys_edit_rotate_90 },
  { "edit-rotate-90-hotkey",     0, 0, 0, g_keys_edit_rotate_90_hotkey },
  { "edit-mirror",               0, 0, 0, g_keys_edit_mirror },
  { "edit-mirror-hotkey",        0, 0, 0, g_keys_edit_mirror_hotkey },
#if 0 /* Obsolete */
  { "edit-stretch",              0, 0, 0, g_keys_edit_stretch },
  { "edit-stretch-hotkey",       0, 0, 0, g_keys_edit_stretch_hotkey },
#endif
  { "edit-slot",                 0, 0, 0, g_keys_edit_slot },
  { "edit-color",                0, 0, 0, g_keys_edit_color },
  { "edit-edit",                 0, 0, 0, g_keys_edit_edit },
  { "edit-text",                 0, 0, 0, g_keys_edit_text },
  { "edit-lock",                 0, 0, 0, g_keys_edit_lock },
  { "edit-unlock",               0, 0, 0, g_keys_edit_unlock },
  { "edit-linetype",             0, 0, 0, g_keys_edit_linetype },
  { "edit-filltype",             0, 0, 0, g_keys_edit_filltype },
  { "edit-translate",            0, 0, 0, g_keys_edit_translate },
  { "edit-embed",                0, 0, 0, g_keys_edit_embed },
  { "edit-unembed",              0, 0, 0, g_keys_edit_unembed },
  { "edit-update",               0, 0, 0, g_keys_edit_update },
  { "edit-show-hidden",          0, 0, 0, g_keys_edit_show_hidden },
  { "edit-make-text-visible",    0, 0, 0, g_keys_edit_make_visible },
  { "edit-find-text",            0, 0, 0, g_keys_edit_find },
  { "edit-show-text",            0, 0, 0, g_keys_edit_show_text },
  { "edit-hide-text",            0, 0, 0, g_keys_edit_hide_text },
  { "edit-autonumber",           0, 0, 0, g_keys_edit_autonumber_text },

  { "buffer-copy1",              0, 0, 0, g_keys_buffer_copy1 },
  { "buffer-copy2",              0, 0, 0, g_keys_buffer_copy2 },
  { "buffer-copy3",              0, 0, 0, g_keys_buffer_copy3 },
  { "buffer-copy4",              0, 0, 0, g_keys_buffer_copy4 },
  { "buffer-copy5",              0, 0, 0, g_keys_buffer_copy5 },
  { "buffer-cut1",               0, 0, 0, g_keys_buffer_cut1 },
  { "buffer-cut2",               0, 0, 0, g_keys_buffer_cut2 },
  { "buffer-cut3",               0, 0, 0, g_keys_buffer_cut3 },
  { "buffer-cut4",               0, 0, 0, g_keys_buffer_cut4 },
  { "buffer-cut5",               0, 0, 0, g_keys_buffer_cut5 },
  { "buffer-paste1",             0, 0, 0, g_keys_buffer_paste1 },
  { "buffer-paste2",             0, 0, 0, g_keys_buffer_paste2 },
  { "buffer-paste3",             0, 0, 0, g_keys_buffer_paste3 },
  { "buffer-paste4",             0, 0, 0, g_keys_buffer_paste4 },
  { "buffer-paste5",             0, 0, 0, g_keys_buffer_paste5 },
  { "buffer-paste1-hotkey",      0, 0, 0, g_keys_buffer_paste1_hotkey },
  { "buffer-paste2-hotkey",      0, 0, 0, g_keys_buffer_paste2_hotkey },
  { "buffer-paste3-hotkey",      0, 0, 0, g_keys_buffer_paste3_hotkey },
  { "buffer-paste4-hotkey",      0, 0, 0, g_keys_buffer_paste4_hotkey },
  { "buffer-paste5-hotkey",      0, 0, 0, g_keys_buffer_paste5_hotkey },

  { "view-redraw",               0, 0, 0, g_keys_view_redraw },
  { "view-zoom-full",            0, 0, 0, g_keys_view_zoom_full },
  { "view-zoom-extents",         0, 0, 0, g_keys_view_zoom_extents },
  { "view-zoom-in",              0, 0, 0, g_keys_view_zoom_in },
  { "view-zoom-out",             0, 0, 0, g_keys_view_zoom_out },
  { "view-zoom-in-hotkey",       0, 0, 0, g_keys_view_zoom_in_hotkey },
  { "view-zoom-out-hotkey",      0, 0, 0, g_keys_view_zoom_out_hotkey },
  { "view-zoom-box",             0, 0, 0, g_keys_view_zoom_box },
  { "view-zoom-box-hotkey",      0, 0, 0, g_keys_view_zoom_box_hotkey },
  { "view-pan",                  0, 0, 0, g_keys_view_pan },
  { "view-pan-hotkey",           0, 0, 0, g_keys_view_pan_hotkey },
  { "view-pan-left",		 0, 0, 0, g_keys_view_pan_left },
  { "view-pan-right",		 0, 0, 0, g_keys_view_pan_right },
  { "view-pan-up",		 0, 0, 0, g_keys_view_pan_up },
  { "view-pan-down",		 0, 0, 0, g_keys_view_pan_down },
  { "view-update-cues",          0, 0, 0, g_keys_view_update_cues },
  { "page-manager",              0, 0, 0, g_keys_page_manager },
  { "page-next",                 0, 0, 0, g_keys_page_next },
  { "page-prev",                 0, 0, 0, g_keys_page_prev },
  { "page-new",                  0, 0, 0, g_keys_page_new },
  { "page-close",                0, 0, 0, g_keys_page_close },
  { "page-revert",               0, 0, 0, g_keys_page_revert },
  { "page-discard",              0, 0, 0, g_keys_page_discard },
  { "page-print",                0, 0, 0, g_keys_page_print },
  { "add-component",             0, 0, 0, g_keys_add_component },
  { "add-attribute",             0, 0, 0, g_keys_add_attribute },
  { "add-attribute-hotkey",      0, 0, 0, g_keys_add_attribute_hotkey },
  { "add-net",                   0, 0, 0, g_keys_add_net },
  { "add-net-hotkey",            0, 0, 0, g_keys_add_net_hotkey },
  { "add-bus",                   0, 0, 0, g_keys_add_bus },
  { "add-bus-hotkey",            0, 0, 0, g_keys_add_bus_hotkey },
  { "add-text",                  0, 0, 0, g_keys_add_text },
  { "add-line",                  0, 0, 0, g_keys_add_line },
  { "add-line-hotkey",           0, 0, 0, g_keys_add_line_hotkey },
  { "add-box",                   0, 0, 0, g_keys_add_box },
  { "add-box-hotkey",            0, 0, 0, g_keys_add_box_hotkey },
  { "add-picture",               0, 0, 0, g_keys_add_picture},
  { "add-picture-hotkey",        0, 0, 0, g_keys_add_picture_hotkey},
  { "add-circle",                0, 0, 0, g_keys_add_circle },
  { "add-circle-hotkey",         0, 0, 0, g_keys_add_circle_hotkey },
  { "add-arc",                   0, 0, 0, g_keys_add_arc },
  { "add-arc-hotkey",            0, 0, 0, g_keys_add_arc_hotkey },
  { "add-pin",                   0, 0, 0, g_keys_add_pin },
  { "add-pin-hotkey",            0, 0, 0, g_keys_add_pin_hotkey },
  { "hierarchy-down-schematic",  0, 0, 0, g_keys_hierarchy_down_schematic },
  { "hierarchy-down-symbol",     0, 0, 0, g_keys_hierarchy_down_symbol },
  { "hierarchy-up",              0, 0, 0, g_keys_hierarchy_up },
  { "hierarchy-documentation",   0, 0, 0, g_keys_hierarchy_documentation },
  { "attributes-attach",         0, 0, 0, g_keys_attributes_attach },
  { "attributes-detach",         0, 0, 0, g_keys_attributes_detach },
  { "attributes-show-name",      0, 0, 0, g_keys_attributes_show_name },
  { "attributes-show-value",     0, 0, 0, g_keys_attributes_show_value },
  { "attributes-show-both",      0, 0, 0, g_keys_attributes_show_both },
  { "attributes-visibility-toggle", 0, 0, 0, g_keys_attributes_visibility_toggle },
  { "options-text-size",         0, 0, 0, g_keys_options_text_size },
  { "options-snap-size",         0, 0, 0, g_keys_options_snap_size },
  { "options-action-feedback",   0, 0, 0, g_keys_options_afeedback },
  { "options-grid",              0, 0, 0, g_keys_options_grid },
  { "options-snap",              0, 0, 0, g_keys_options_snap },
  { "options-rubberband",        0, 0, 0, g_keys_options_rubberband },
  { "options-show-log-window",   0, 0, 0, g_keys_options_show_log_window },
  { "options-show-coord-window", 0, 0, 0, g_keys_options_show_coord_window },
  { "help-about",                0, 0, 0, g_keys_help_about },
  { "help-manual",               0, 0, 0, g_keys_help_manual },
  { "help-hotkeys",              0, 0, 0, g_keys_help_hotkeys },
  { "help-component",            0, 0, 0, g_keys_hierarchy_documentation },
  { "misc-misc",                 0, 0, 0, g_keys_misc },
  { "misc-misc2",                0, 0, 0, g_keys_misc2 },
  { "misc-misc3",                0, 0, 0, g_keys_misc3 },
  { "cancel",                    0, 0, 0, g_keys_cancel },

  /*help functions for generating netlists*/
  { "get-selected-filename",     0, 0, 0, g_get_selected_filename },
  { "get-selected-component-attributes", 0, 0, 0, g_get_selected_component_attributes },
  
  { NULL,                        0, 0, 0, NULL } };
  
/*! \brief Register function with Scheme.
 *  \par Function Description
 *  Creates <B>subr</B> objects to make <B>g_rc_*</B> functions that are defined *  #g_rc.c, #g_keys.c and #g_funcs.c visible to Scheme.
 */
void g_register_funcs (void)
{
  struct gsubr_t *tmp = gschem_funcs;
 
  while (tmp->name != NULL) {
    scm_c_define_gsubr (tmp->name, tmp->req, tmp->opt, tmp->rst, tmp->fnc);
    tmp++;
  }

  g_init_attrib_smob ();
  g_init_object_smob ();
  g_init_page_smob ();

  /* Hook stuff */
  scm_c_define_gsubr ("add-attribute-to-object", 5, 0, 0, g_add_attrib);
  scm_c_define_gsubr ("get-object-attributes", 1, 0, 0, g_get_object_attributes);
  scm_c_define_gsubr ("get-object-bounds", 3, 0, 0, g_get_object_bounds);
  scm_c_define_gsubr ("get-object-pins", 1, 0, 0, g_get_object_pins);
  scm_c_define_gsubr ("get-pin-ends", 1, 0, 0, g_get_pin_ends);
  scm_c_define_gsubr ("set-attribute-text-properties!", 7, 0, 0, g_set_attrib_text_properties);
  scm_c_define_gsubr ("set-attribute-value!", 2, 0, 0, g_set_attrib_value_x);
  scm_c_define_gsubr ("add-component-at-xy", 7, 0, 0, g_add_component);
  scm_c_define_gsubr ("get-objects-in-page", 1, 0, 0, g_get_objects_in_page);

  add_component_hook  = scm_create_hook ("add-component-hook", 1);
  add_component_object_hook  = scm_create_hook ("add-component-object-hook", 
						1);
  rotate_component_object_hook  = scm_create_hook ("rotate-component-object-hook",
						   1);
  mirror_component_object_hook  = scm_create_hook ("mirror-component-object-hook", 
						1);
  copy_component_hook = scm_create_hook ("copy-component-hook", 1);
  move_component_hook = scm_create_hook ("move-component-hook", 1);
  deselect_component_hook = scm_create_hook ("deselect-component-hook", 1);
  deselect_net_hook = scm_create_hook ("deselect-net-hook", 1);
  deselect_all_hook = scm_create_hook ("deselect-all-hook", 1);
  select_component_hook = scm_create_hook ("select-component-hook", 1);
  select_net_hook = scm_create_hook ("select-net-hook", 1);

  add_pin_hook = scm_create_hook ("add-pin-hook", 1);
  mirror_pin_hook = scm_create_hook ("mirror-pin-hook", 1);
  rotate_pin_hook = scm_create_hook ("rotate-pin-hook", 1);
  add_attribute_hook = scm_create_hook ("add-attribute-hook", 1);
  new_page_hook = scm_create_hook ("new-page-hook", 1);

}
