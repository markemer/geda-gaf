/* gEDA - GPL Electronic Design Automation
 * gschem - gEDA Schematic Capture
 * Copyright (C) 1998-2010 Ales Hvezda
 * Copyright (C) 1998-2011 gEDA Contributors (see ChangeLog for details)
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
#include <math.h>

#include "gschem.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif


/*! \brief set the start point of a new bus
 *  \par Function Description
 *  This function sets the start point (<B>w_x</B>,<B>w_y</B>) of a new bus
 *  in the <B>GSCHEM_TOPLEVEL</B> structure.
 *  
 *  The start point is stored in <B>first_wx</B>, <B>first_wy</B>.
 *  
 *  \param [in] w_current  The GSCHEM_TOPLEVEL object.
 *  \param [in] w_x        the x position in world coords
 *  \param [in] w_y        the y position in world coords
 */
void o_bus_start(GSCHEM_TOPLEVEL *w_current, int w_x, int w_y)
{
  w_current->first_wx = w_current->second_wx = w_x;
  w_current->first_wy = w_current->second_wy = w_y;
}

/*! \brief finish a bus drawing action
 *  \par Function Description
 *  This function finishes a net drawing action. The function draws
 *  a bus from the point (<B>first_wx</B>,<B>first_wy</B>) to 
 *  (<B>second_wx</B>,<B>second_wy</B>). Both points are taken from
 *  the <B>GSCHEM_TOPLEVEL</B> structure.
 *
 *  The function returns TRUE if a bus object has been created and 
 *  FALSE if no bus object has been created.
 *
 *  \param [in] w_current  The GSCHEM_TOPLEVEL object.
 *  \param [in] w_x        (unused)
 *  \param [in] w_y        (unused)
 */
int o_bus_end(GSCHEM_TOPLEVEL *w_current, int w_x, int w_y)
{
  TOPLEVEL *toplevel = w_current->toplevel;
  OBJECT *new_obj;
  int color;
  GList *prev_conn_objects = NULL;

  g_assert( w_current->inside_action != 0 );

  if (toplevel->override_bus_color == -1) {
    color = BUS_COLOR;
  } else {
    color = toplevel->override_bus_color;
  }

  /* erase the rubberbus */
  /* o_bus_invalidate_rubber (w_current); */
  w_current->rubber_visible = 0;

  /* don't allow zero length bus */
  /* this ends the bus drawing behavior we want this? hack */
  if ( (w_current->first_wx == w_current->second_wx) &&
       (w_current->first_wy == w_current->second_wy) ) {
    return FALSE;
  }

  new_obj = o_bus_new(toplevel, OBJ_BUS, color,
                      w_current->first_wx, w_current->first_wy,
                      w_current->second_wx, w_current->second_wy, 0);
  s_page_append (toplevel, toplevel->page_current, new_obj);

  /* connect the new bus to the other busses */
  prev_conn_objects = s_conn_return_others (prev_conn_objects, new_obj);
  o_invalidate_glist (w_current, prev_conn_objects);
  g_list_free (prev_conn_objects);

  /* Call add-objects-hook */
  g_run_hook_object (w_current, "%add-objects-hook", new_obj);

  toplevel->page_current->CHANGED=1;
  w_current->first_wx = w_current->second_wx;
  w_current->first_wy = w_current->second_wy;
  o_undo_savestate(w_current, UNDO_ALL);
  return TRUE;
}

/*! \brief draw the bus rubber when creating a bus
 *  \par Function Description
 *  This function draws
 *  a bus rubber from the point (<B>first_wx</B>,<B>first_wy</B>) from  
 *  the <B>GSCHEM_TOPLEVEL</B> structure to the input parameter
 *  (<B>w_x</B>, <B>w_y</B>).
 *
 *  The function stores creates an non-orthogonal bus segment if the 
 *  CONTROLKEY is pressed. The coordinates of the second rubberbus point
 *  is stored as (<B>second_wx</B>,<B>second_wy</B>) in the 
 *  <B>GSCHEM_TOPLEVEL</B> structure.
 *
 *  \param [in] w_current  The GSCHEM_TOPLEVEL object.
 *  \param [in] w_x        current x position in world units
 *  \param [in] w_y        current y position in world units
 */
void o_bus_motion (GSCHEM_TOPLEVEL *w_current, int w_x, int w_y)
{
  int diff_x, diff_y;

  g_assert( w_current->inside_action != 0 );

  if (w_current->rubber_visible)
    o_bus_invalidate_rubber (w_current);

  w_current->second_wx = w_x;
  w_current->second_wy = w_y;

  /* If you press the control key then you can draw non-ortho bus */
  if (!w_current->CONTROLKEY) {
    diff_x = abs(w_current->second_wx - w_current->first_wx);
    diff_y = abs(w_current->second_wy - w_current->first_wy);

    if (diff_x >= diff_y) {
      w_current->second_wy = w_current->first_wy;
    } else {
      w_current->second_wx = w_current->first_wx;
    }
  }

  o_bus_invalidate_rubber (w_current);
  w_current->rubber_visible = 1;
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void o_bus_invalidate_rubber (GSCHEM_TOPLEVEL *w_current)
{
  TOPLEVEL *toplevel = w_current->toplevel;
  int x1, y1, x2, y2;
  int min_x, min_y, max_x, max_y;
  int bloat = 0;

  WORLDtoSCREEN (w_current, w_current->first_wx, w_current->first_wy, &x1, &y1);
  WORLDtoSCREEN (w_current, w_current->second_wx, w_current->second_wy, &x2, &y2);

  bloat = SCREENabs (w_current, BUS_WIDTH) / 2;

  min_x = min (x1, x2) - bloat;
  max_x = max (x1, x2) + bloat;
  min_y = min (y1, y2) - bloat;
  max_y = max (y1, y2) + bloat;

  o_invalidate_rect (w_current, min_x, min_y, max_x, max_y);
}

/*! \brief draw a rubberbus segment
 *  \par Function Description
 *  This function draws a bus segment from the point
 *  (<B>first_wx</B>,<B>first_wy</B>) to the point 
 *  (<B>second_wx</B>,<B>second_wy</B>) from the <B>GSCHEM_TOPLEVEL</B>
 *   structure.
 *
 *  The function can be used to draw or erase the rubberbus on the screen.
 *
 *  \param [in] w_current  The GSCHEM_TOPLEVEL object
 */
void
o_bus_draw_rubber (GSCHEM_TOPLEVEL *w_current, EdaRenderer *renderer)
{
  int size = BUS_WIDTH;
  cairo_t *cr = eda_renderer_get_cairo_context (renderer);
  GArray *color_map = eda_renderer_get_color_map (renderer);
  int flags = eda_renderer_get_cairo_flags (renderer);

  eda_cairo_line (cr, flags, END_NONE, size,
                  w_current->first_wx,  w_current->first_wy,
                  w_current->second_wx, w_current->second_wy);
  eda_cairo_set_source_color (cr, SELECT_COLOR, color_map);
  eda_cairo_stroke (cr, flags, TYPE_SOLID, END_NONE, size, -1, -1);
}
