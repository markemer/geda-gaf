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

#include "gschem.h"

GSCHEM_TOPLEVEL *gschem_toplevel_new ()
{
  GSCHEM_TOPLEVEL *w_current;

  w_current = g_new0 (GSCHEM_TOPLEVEL, 1);

  w_current->toplevel = NULL;

  /* ------------------- */
  /* main window widgets */
  /* ------------------- */
  w_current->main_window  = NULL;
  w_current->drawing_area = NULL;
  w_current->menubar      = NULL;
  w_current->popup_menu   = NULL;
  w_current->h_scrollbar  = NULL;
  w_current->v_scrollbar  = NULL;
  w_current->h_adjustment = NULL;
  w_current->v_adjustment = NULL;
  w_current->left_label   = NULL;
  w_current->middle_label = NULL;
  w_current->right_label  = NULL;
  w_current->grid_label = NULL;
  w_current->status_label = NULL;

  w_current->toolbar_select = NULL;
  w_current->toolbar_net    = NULL;
  w_current->toolbar_bus    = NULL;

  w_current->keyaccel_string = NULL;
  w_current->keyaccel_string_source_id = FALSE;

  /* ------------ */
  /* Dialog boxes */
  /* ------------ */
  w_current->sowindow     = NULL;
  w_current->pfswindow    = NULL;
  w_current->cswindow     = NULL;
  w_current->iwindow      = NULL;
  w_current->pswindow     = NULL;
  w_current->tiwindow     = NULL;
  w_current->tewindow     = NULL;
  w_current->sewindow     = NULL;
  w_current->aawindow     = NULL;
  w_current->mawindow     = NULL;
  w_current->aewindow     = NULL;
  w_current->trwindow     = NULL;
  w_current->tswindow     = NULL;
  w_current->tshowwindow  = NULL;
  w_current->thidewindow  = NULL;
  w_current->tfindwindow  = NULL;
  w_current->hkwindow     = NULL;
  w_current->clwindow     = NULL;
  w_current->edit_color   = 0;
  w_current->cowindow     = NULL;
  w_current->coord_world  = NULL;
  w_current->coord_screen = NULL;

  /* ----------------- */
  /* Picture placement */
  /* ----------------- */
  w_current->current_pixbuf = NULL;
  w_current->pixbuf_filename = NULL;
  w_current->pixbuf_wh_ratio = 0;

  /* ---------------------- */
  /* graphics context stuff */
  /* ---------------------- */
  w_current->gc              = NULL;

  /* ---------------- */
  /* Drawing surfaces */
  /* ---------------- */
  w_current->window = NULL;
  w_current->drawable = NULL;
  w_current->cr = NULL;
  w_current->pl = NULL;
  w_current->win_width = 0;
  w_current->win_height = 0;

  /* ------------- */
  /* Drawing state */
  /* ------------- */
  w_current->renderer = EDA_RENDERER (g_object_new (EDA_TYPE_RENDERER, NULL));
  w_current->first_wx = -1;
  w_current->first_wy = -1;
  w_current->second_wx = -1;
  w_current->second_wy = -1;
  w_current->third_wx = -1;
  w_current->third_wy = -1;
  w_current->distance = 0;
  w_current->magnetic_wx = -1;
  w_current->magnetic_wy = -1;
  w_current->inside_action = 0;
  w_current->rubber_visible = 0;
  w_current->net_direction = 0;
  w_current->which_grip = -1;
  w_current->which_object = NULL;

  /* ------------------ */
  /* Rubberbanding nets */
  /* ------------------ */
  w_current->stretch_list = NULL;

  /* --------------------- */
  /* Gschem internal state */
  /* --------------------- */
  w_current->num_untitled = 0;
  w_current->event_state = SELECT;
  w_current->image_width  = 0;
  w_current->image_height = 0;
  w_current->grid = GRID_MESH;
  w_current->min_zoom = 0;
  w_current->max_zoom = 8;
  w_current->text_alignment = 0;
  w_current->drawbounding_action_mode = FREE;
  w_current->last_drawb_mode = LAST_DRAWB_MODE_NONE;
  w_current->CONTROLKEY = 0;
  w_current->SHIFTKEY   = 0;
  w_current->ALTKEY     = 0;
  w_current->doing_pan = 0;
  w_current->buffer_number = 0;
  w_current->last_callback = NULL;
  w_current->clipboard_buffer = NULL;

  /* ------------------ */
  /* rc/user parameters */
  /* ------------------ */
  w_current->text_caps = 0;
  w_current->text_size = 0;

  w_current->zoom_with_pan = 0;
  w_current->actionfeedback_mode = OUTLINE;
  w_current->net_direction_mode = TRUE;
  w_current->net_selection_mode = 0;
  w_current->net_selection_state = 0;
  w_current->embed_complex = 0;
  w_current->include_complex = 0;
  w_current->scrollbars_flag = 0;
  w_current->log_window = 0;
  w_current->log_window_type = 0;
  w_current->third_button = 0;
  w_current->middle_button = 0;
  w_current->file_preview = 0;
  w_current->enforce_hierarchy = 0;
  w_current->fast_mousepan = 0;
  w_current->raise_dialog_boxes = 0;
  w_current->continue_component_place = 0;
  w_current->component_select_attrlist = NULL;
  w_current->undo_levels = 0;
  w_current->undo_control = 0;
  w_current->undo_type = 0;
  w_current->undo_panzoom = 0;
  w_current->draw_grips = 0;
  w_current->netconn_rubberband = 0;
  w_current->sort_component_library = 0;
  w_current->warp_cursor = 0;
  w_current->toolbars = 0;
  w_current->handleboxes = 0;
  w_current->bus_ripper_size = 0;
  w_current->bus_ripper_type = 0;
  w_current->bus_ripper_rotation = 0;
  w_current->magneticnet_mode = 0;
  w_current->dots_grid_fixed_threshold = 10;
  w_current->dots_grid_dot_size = 1;
  w_current->dots_grid_mode = DOTS_GRID_VARIABLE_MODE;
  w_current->mesh_grid_display_threshold = 3;
  w_current->add_attribute_offset = 50;
  w_current->mousepan_gain = 5;
  w_current->keyboardpan_gain = 10;
  w_current->select_slack_pixels = 4;
  w_current->zoom_gain = 20;
  w_current->scrollpan_steps = 8;
  w_current->snap = SNAP_GRID;
  w_current->snap_size = 100;

  w_current->smob = SCM_UNDEFINED;

  return w_current;
}

