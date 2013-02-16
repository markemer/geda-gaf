/* gEDA - GPL Electronic Design Automation
 * gschem - gEDA Schematic Capture
 * Copyright (C) 1998-2010 Ales Hvezda
 * Copyright (C) 1998-2010 gEDA Contributors (see ChangeLog for details)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include <config.h>
#include <stdio.h>

#include "gschem.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

/*! \def INIT_STR(w, name, str) */
#define INIT_STR(w, name, str) {                                        \
        g_free((w)->name);                                              \
        (w)->name = g_strdup(((default_ ## name) != NULL) ?             \
                             (default_ ## name) : (str));               \
}

/* Absolute default used when default_... strings are NULL */

int   default_text_size = 10;
int   default_text_caps = LOWER;
int   default_attribute_color = ATTRIBUTE_COLOR;
int   default_detachattr_color = DETACHED_ATTRIBUTE_COLOR;
int   default_net_endpoint_color = NET_ENDPOINT_COLOR;
int   default_junction_color = JUNCTION_COLOR;
int   default_override_net_color = -1;
int   default_override_bus_color = -1;
int   default_override_pin_color = -1;
int   default_net_direction_mode = TRUE;
int   default_net_selection_mode = 0;
int   default_background_color = BACKGROUND_COLOR;
int   default_actionfeedback_mode = OUTLINE;
int   default_zoom_with_pan = TRUE;
int   default_do_logging = TRUE;
int   default_logging_dest = LOG_WINDOW;
int   default_embed_complex = FALSE;
int   default_include_complex = FALSE;
int   default_snap_size = 100;

int   default_scrollbars_flag = TRUE;
int   default_image_color = FALSE;
int   default_image_width = 800;
int   default_image_height = 600;
int   default_log_window = MAP_ON_STARTUP;
int   default_log_window_type = DECORATED;
int   default_third_button = POPUP_ENABLED;
#ifdef HAVE_LIBSTROKE
int   default_middle_button = STROKE;
#else
int   default_middle_button = REPEAT;
#endif
int   default_scroll_wheel = SCROLL_WHEEL_CLASSIC;
int   default_net_consolidate = TRUE;
int   default_file_preview = FALSE;
int   default_enforce_hierarchy = TRUE;
int   default_fast_mousepan = TRUE;
int   default_raise_dialog_boxes = FALSE;
int   default_continue_component_place = TRUE;
GList *default_component_select_attrlist = NULL;
int   default_undo_levels = 20;
int   default_undo_control = TRUE;
int   default_undo_type = UNDO_DISK;
int   default_undo_panzoom = FALSE;
int   default_draw_grips = TRUE;
int   default_netconn_rubberband = FALSE;
int   default_magnetic_net_mode = TRUE;
int   default_sort_component_library = FALSE;
int   default_warp_cursor = TRUE;
int   default_toolbars = TRUE;
int   default_handleboxes = TRUE;
int   default_setpagedevice_orientation = FALSE;
int   default_setpagedevice_pagesize = FALSE;
int   default_bus_ripper_size = 200;
int   default_bus_ripper_type = COMP_BUS_RIPPER;
int   default_bus_ripper_rotation = NON_SYMMETRIC;
int   default_force_boundingbox = FALSE;
int   default_dots_grid_dot_size = 1;
int   default_dots_grid_mode = DOTS_GRID_VARIABLE_MODE;
int   default_dots_grid_fixed_threshold = 10;
int   default_mesh_grid_display_threshold = 3;
int   default_add_attribute_offset = 50;

int   default_auto_save_interval = 120;

int   default_width = 800;  /* these variables are used in x_window.c */
int   default_height = 600;

int default_mousepan_gain = 5;
int default_keyboardpan_gain = 20;
int default_select_slack_pixels = 4;
int default_zoom_gain = 20;
int default_scrollpan_steps = 8;

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void i_vars_set(GSCHEM_TOPLEVEL *w_current)
{
  TOPLEVEL *toplevel = w_current->toplevel;
  i_vars_libgeda_set(toplevel);

  /* this will be false if logging cannot be enabled */
  if (do_logging != FALSE) {
    do_logging = default_do_logging;
  }

  logging_dest = default_logging_dest;

  w_current->text_size     = default_text_size;
  w_current->text_caps     = default_text_caps;

  toplevel->background_color = default_background_color;

  w_current->net_direction_mode = default_net_direction_mode;
  w_current->net_selection_mode = default_net_selection_mode;

  toplevel->override_net_color = default_override_net_color;

  toplevel->override_bus_color = default_override_bus_color;

  toplevel->override_pin_color = default_override_pin_color;

  w_current->zoom_with_pan           = default_zoom_with_pan;
  w_current->actionfeedback_mode     = default_actionfeedback_mode;
  w_current->scrollbars_flag         = default_scrollbars_flag;

  w_current->embed_complex   = default_embed_complex;
  w_current->include_complex = default_include_complex;
  w_current->snap_size       = default_snap_size;
  w_current->log_window      = default_log_window;
  w_current->log_window_type = default_log_window_type;

  toplevel->image_color        = default_image_color;
  w_current->image_width        = default_image_width;
  w_current->image_height       = default_image_height;
  w_current->third_button       = default_third_button;
  w_current->middle_button      = default_middle_button;
  w_current->scroll_wheel       = default_scroll_wheel;
  toplevel->net_consolidate    = default_net_consolidate;
  w_current->file_preview       = default_file_preview;
  w_current->enforce_hierarchy  = default_enforce_hierarchy;
  w_current->fast_mousepan      = default_fast_mousepan;
  w_current->raise_dialog_boxes = default_raise_dialog_boxes;
  w_current->continue_component_place = default_continue_component_place;
  w_current->component_select_attrlist = default_component_select_attrlist;
  w_current->undo_levels = default_undo_levels;
  w_current->undo_control = default_undo_control;
  w_current->undo_type = default_undo_type;
  w_current->undo_panzoom = default_undo_panzoom;

  w_current->draw_grips = default_draw_grips;
  w_current->netconn_rubberband = default_netconn_rubberband;
  w_current->magneticnet_mode = default_magnetic_net_mode;
  w_current->sort_component_library = default_sort_component_library;
  w_current->warp_cursor = default_warp_cursor;
  w_current->toolbars = default_toolbars;
  w_current->handleboxes = default_handleboxes;

  w_current->bus_ripper_size  = default_bus_ripper_size;
  w_current->bus_ripper_type  = default_bus_ripper_type;
  w_current->bus_ripper_rotation  = default_bus_ripper_rotation;

  toplevel->force_boundingbox  = default_force_boundingbox;

  w_current->dots_grid_dot_size          = default_dots_grid_dot_size;
  w_current->dots_grid_mode              = default_dots_grid_mode;
  w_current->dots_grid_fixed_threshold   = default_dots_grid_fixed_threshold;
  w_current->mesh_grid_display_threshold = default_mesh_grid_display_threshold;

  w_current->add_attribute_offset  = default_add_attribute_offset;

  w_current->mousepan_gain = default_mousepan_gain;
  w_current->keyboardpan_gain = default_keyboardpan_gain;

  w_current->select_slack_pixels = default_select_slack_pixels;
  w_current->zoom_gain = default_zoom_gain;
  w_current->scrollpan_steps = default_scrollpan_steps;

  toplevel->auto_save_interval = default_auto_save_interval;
}


/*! \brief Free default names
 *  \par Function Description
 *  This function will free all of the default variables for gschem.
 *
 */
void i_vars_freenames()
{
}
