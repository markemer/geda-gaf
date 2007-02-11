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
#include <math.h>

#include <libgeda/libgeda.h>

#include "../include/globals.h"
#include "../include/prototype.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void o_net_draw(TOPLEVEL *w_current, OBJECT *o_current)
{
  int size;
  int x1, y1, x2, y2; /* screen coords */

#if NET_DEBUG /* debug */
  char *tempstring;
  GdkFont *font;
#endif

  if (o_current == NULL) {
    return;
  }

  if (o_current->line == NULL) {
    return;
  }

  o_net_recalc(w_current, o_current);

  /* reuse line's routine */
  if ( (w_current->DONT_REDRAW == 1) || 
       (!o_line_visible(w_current, o_current->line, &x1, &y1, &x2, &y2)) ) {
    return;
  }

#if DEBUG
  printf("drawing net\n\n");
#endif

  if (w_current->net_style == THICK ) {
    size = SCREENabs(w_current, NET_WIDTH);

    if (size < 0)
      size=0;

    gdk_gc_set_line_attributes(w_current->gc, size, 
                               GDK_LINE_SOLID,
                               GDK_CAP_BUTT,
                               GDK_JOIN_MITER);

    gdk_gc_set_line_attributes(w_current->bus_gc, size, 
                               GDK_LINE_SOLID,
                               GDK_CAP_BUTT,
                               GDK_JOIN_MITER);
  }

  if (w_current->override_color != -1 ) {

    gdk_gc_set_foreground(w_current->gc,
                          x_get_color(w_current->override_color));

    gdk_draw_line(w_current->window, w_current->gc,
                  x1, y1, x2, y2);
    gdk_draw_line(w_current->backingstore, w_current->gc,
                  x1, y1, x2, y2);
  } else {

    gdk_gc_set_foreground(w_current->gc,
                          x_get_color(o_current->color));
    gdk_draw_line(w_current->window, w_current->gc,
                  x1, y1, x2, y2);
    gdk_draw_line(w_current->backingstore, w_current->gc,
                  x1, y1, x2, y2);


#if NET_DEBUG
    /* temp debug only */
    font = gdk_fontset_load ("10x20");
    tempstring = g_strdup_printf("%s", o_current->name);
    gdk_draw_text (w_current->window,
                   font,
                   w_current->gc,
                   x1+20, y1+20,
                   tempstring,
                   strlen(tempstring));
    gdk_draw_text (w_current->backingstore,
                   font,
                   w_current->gc,
                   x1+20, y1+20,
                   tempstring,
                   strlen(tempstring));
    gdk_font_unref(font);
    g_free(tempstring);
#endif
  }

#if DEBUG 
  printf("drew net\n\n");
#endif

  /* yes zero is right for the width -> use hardware lines */
  if (w_current->net_style == THICK ) {
    gdk_gc_set_line_attributes(w_current->gc, 0, 
                               GDK_LINE_SOLID,
                               GDK_CAP_NOT_LAST,
                               GDK_JOIN_MITER);

    gdk_gc_set_line_attributes(w_current->bus_gc, 0, 
                               GDK_LINE_SOLID,
                               GDK_CAP_NOT_LAST,
                               GDK_JOIN_MITER);
  }

  if (o_current->draw_grips && w_current->draw_grips == TRUE) {	

	  /* pb20011109 - modified to use the new o_line_[draw|erase]_grips() */
	  /*              reuse the line functions */
	  if (!o_current->selected) {
		  /* object is no more selected, erase the grips */
		  o_current->draw_grips = FALSE;
		  o_line_erase_grips(w_current, o_current);
	  } else {
		  /* object is selected, draw the grips */
		  o_line_draw_grips(w_current, o_current);
	  }

  }
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void o_net_erase(TOPLEVEL *w_current, OBJECT *o_current)
{
  w_current->override_color = w_current->background_color;
  o_net_draw(w_current, o_current);
  w_current->override_color = -1;
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void o_net_draw_xor(TOPLEVEL *w_current, int dx, int dy, OBJECT *o_current)
{
  int size;
  int color;

  if (o_current->line == NULL) {
    return;
  }

  if (o_current->saved_color != -1) {
    color = o_current->saved_color;
  } else {
    color = o_current->color;
  }

  gdk_gc_set_foreground(w_current->outline_xor_gc,
			x_get_darkcolor(color));

  if (w_current->net_style == THICK ) {
    size = SCREENabs(w_current, NET_WIDTH);
    gdk_gc_set_line_attributes(w_current->outline_xor_gc, size+1,
                               GDK_LINE_SOLID,
                               GDK_CAP_NOT_LAST,
                               GDK_JOIN_MITER);
  }

  gdk_draw_line(w_current->window, w_current->outline_xor_gc,
                o_current->line->screen_x[0]+dx,
                o_current->line->screen_y[0]+dy,
                o_current->line->screen_x[1]+dx,
                o_current->line->screen_y[1]+dy);

  /* backing store ? not approriate here */

  if (w_current->net_style == THICK ) {
    gdk_gc_set_line_attributes(w_current->outline_xor_gc, 0,
                               GDK_LINE_SOLID,
                               GDK_CAP_NOT_LAST,
                               GDK_JOIN_MITER);
  }
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void o_net_draw_xor_single(TOPLEVEL *w_current, int dx, int dy, int whichone,
			   OBJECT *o_current)
{
  int color;
  int dx1 = -1, dx2 = -1, dy1 = -1,dy2 = -1;

  if (o_current->line == NULL) {
    return;
  }

  if (o_current->saved_color != -1) {
    color = o_current->saved_color;
  } else {
    color = o_current->color;
  }

  gdk_gc_set_foreground(w_current->outline_xor_gc,
			x_get_darkcolor(color));

#if 0 /* if I had this enabled, than xoring would leave a lot of mouse drops */
  if (w_current->net_style == THICK ) {
    size = SCREENabs(w_current, NET_WIDTH);
    gdk_gc_set_line_attributes(w_current->outline_xor_gc, size+1,
                               GDK_LINE_SOLID,
                               GDK_CAP_NOT_LAST,
                               GDK_JOIN_MITER);
  }
#endif

  if (whichone == 0) {
    dx1 = dx;
    dy1 = dy;
    dx2 = 0;
    dy2 = 0;
  } else if (whichone == 1) {
    dx2 = dx;
    dy2 = dy;
    dx1 = 0;
    dy1 = 0;
  } else {
    fprintf(stderr, _("Got an invalid which one in o_net_draw_xor_single\n"));
  }

  gdk_draw_line(w_current->window, w_current->outline_xor_gc,
                o_current->line->screen_x[0]+dx1,
                o_current->line->screen_y[0]+dy1,
                o_current->line->screen_x[1]+dx2,
                o_current->line->screen_y[1]+dy2);

  /* backing store ? not approriate here */

#if 0 /* if I had this enabled, than xoring would leave a lot of mouse drops */
  if (w_current->net_style == THICK ) {
    gdk_gc_set_line_attributes(w_current->outline_xor_gc, 0,
                               GDK_LINE_SOLID,
                               GDK_CAP_NOT_LAST,
                               GDK_JOIN_MITER);
  }
#endif
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void o_net_start(TOPLEVEL *w_current, int x, int y)
{
  int size;

  /* initalize all parameters used when drawing the new net */
  w_current->last_x = w_current->start_x = w_current->second_x = 
    fix_x(w_current, x);
  w_current->last_y = w_current->start_y = w_current->second_y = 
    fix_y(w_current, y);

#if 0 /* not ready for prime time use, this is the snap any point #if 0 */
  int distance1;
  int distance2;
  OBJECT *real;
  OBJECT *o_current;
  int temp_x, temp_y;
  o_current = o_CONN_search_closest_range(w_current,
                                          w_current->page_current->object_head,
                                          w_current->start_x, w_current->start_y,
                                          &temp_x, &temp_y, 200, NULL, NULL);

  if (o_current) {
    w_current->last_x = w_current->start_x = temp_x;
    w_current->last_y = w_current->start_y = temp_y;
  } else {
    w_current->last_x = w_current->start_x = fix_x(w_current, x);
    w_current->last_y = w_current->start_y = fix_y(w_current, y);
  }
#endif

#if 0 /* not ready for prime time use */
  /* new net extenstion stuff */
  o_current = (OBJECT *) w_current->page_current->selection_list->data;
  if (o_current != NULL && w_current->event_state == STARTDRAWNET) {
    if (o_current->type == OBJ_NET) {
      if (o_current->line) {

        real = o_list_sear( /* ch */ 
                           w_current->page_current->object_head,
                           o_current);

        if (!real) {
          fprintf(stderr, _("selected a nonexistant object!\n"));
          exit(-1);
        }
        distance1 = dist(
                         real->line->screen_x[0],
                         real->line->screen_y[0],
                         w_current->start_x, w_current->start_y);

        distance2 = dist(
                         real->line->screen_x[1],
                         real->line->screen_y[1],
                         w_current->start_x, w_current->start_y);

        printf("%d %d\n", distance1, distance2);

        if (distance1 < distance2) {
          w_current->last_x = w_current->start_x =
            real->line->screen_x[0];
          w_current->last_y = w_current->start_y =
            real->line->screen_y[0];
        } else {
          w_current->last_x = w_current->start_x =
            real->line->screen_x[1];
          w_current->last_y = w_current->start_y =
            real->line->screen_y[1];
        }
      }
    } else if (o_current->type == OBJ_COMPLEX || 
               o_current->type == OBJ_PLACEHOLDER) {
      real = o_list_sear( /* ch */
                         w_current->page_current->object_head,
                         o_current);

      if (!real) {
        fprintf(stderr, _("selected a nonexistant object!\n"));
        exit(-1);
      }

      o_CONN_search_closest(w_current, o_current->complex,
                            w_current->start_x, w_current->start_y,
                            &temp_x, &temp_y, NULL);
      w_current->last_x = w_current->start_x = temp_x;
      w_current->last_y = w_current->start_y = temp_y;
    }

  }
#endif

  if (w_current->net_style == THICK ) {
    size = SCREENabs(w_current, NET_WIDTH);
    gdk_gc_set_line_attributes(w_current->xor_gc, size,
                               GDK_LINE_SOLID,
                               GDK_CAP_NOT_LAST,
                               GDK_JOIN_MITER);
  }

  gdk_gc_set_foreground(w_current->xor_gc,
			x_get_darkcolor(w_current->select_color) );
  gdk_draw_line(w_current->window, w_current->xor_gc, 
		w_current->start_x, w_current->start_y, 
		w_current->last_x, w_current->last_y);

  if (w_current->net_style == THICK ) {
    gdk_gc_set_line_attributes(w_current->xor_gc, 0,
                               GDK_LINE_SOLID,
                               GDK_CAP_NOT_LAST,
                               GDK_JOIN_MITER);
  }
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
int o_net_end(TOPLEVEL *w_current, int x, int y)
{
  int x1, y1;
  int x2, y2;
  int x3, y3;
  int color;
  int size;
  int primary_zero_length, secondary_zero_length;
  int found_primary_connection = FALSE;

  /*int temp_x, temp_y;*/
  /* OBJECT *o_current;*/
  GList *other_objects = NULL;
  OBJECT *new_net = NULL;
  
  if (w_current->inside_action == 0) {
    o_redraw(w_current, w_current->page_current->object_head, TRUE);
    return(FALSE);
  }

  if (w_current->override_net_color == -1) {
    color = w_current->net_color;
  } else {
    color = w_current->override_net_color;
  }

  size = SCREENabs(w_current, NET_WIDTH);

  if (w_current->net_style == THICK ) {
    gdk_gc_set_line_attributes(w_current->xor_gc, size,
                               GDK_LINE_SOLID,
                               GDK_CAP_NOT_LAST,
                               GDK_JOIN_MITER);
  }

  gdk_gc_set_foreground(w_current->xor_gc,
			x_get_darkcolor(w_current->select_color) );

  /* Erase primary rubber net line */
  gdk_draw_line(w_current->window, w_current->xor_gc,
		w_current->start_x, w_current->start_y,
		w_current->last_x, w_current->last_y);

  /* Erase secondary rubber net line */
  gdk_draw_line(w_current->window, w_current->xor_gc,
		 w_current->last_x, w_current->last_y,
		 w_current->second_x, w_current->second_y);

  if (w_current->net_style == THICK) {
    gdk_gc_set_line_attributes(w_current->xor_gc, 0,
			       GDK_LINE_SOLID,
			       GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
    gdk_gc_set_line_attributes(w_current->gc, size,
			       GDK_LINE_SOLID,
			       GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
  }


  /* See if either of the nets are zero length.  We'll only add */
  /* the non-zero ones */
  primary_zero_length = (w_current->start_x == w_current->last_x) &&
      (w_current->start_y == w_current->last_y);
 
  secondary_zero_length = (w_current->last_x == w_current->second_x) &&
      (w_current->last_y == w_current->second_y);

  /* If both nets are zero length... */
  /* this ends the net drawing behavior we want this? hack */
  if ( primary_zero_length && secondary_zero_length ) {
    w_current->start_x = (-1);
    w_current->start_y = (-1);
    w_current->last_x = (-1);
    w_current->last_y = (-1);
    w_current->second_x = (-1);
    w_current->second_y = (-1);
    w_current->inside_action = 0;
    i_set_state(w_current, STARTDRAWNET);
    o_net_eraserubber(w_current);
    
    return (FALSE);
  }
#if 0				/* not ready for prime time use */
  /* second attempt at all snapping */
  o_current = o_CONN_search_closest_range(w_current,
					  w_current->page_current->
					  object_head, w_current->last_x,
					  w_current->last_y, &temp_x,
					  &temp_y, 200, NULL, NULL);

  if (o_current) {
    w_current->last_x = temp_x;
    w_current->last_y = temp_y;
  } else {
    w_current->last_x = fix_x(w_current, x);
    w_current->last_y = fix_y(w_current, y);
  }
#endif


  /* Primary net runs from (x1,y1)-(x2,y2) */
  /* Secondary net from (x2,y2)-(x3,y3) */
  SCREENtoWORLD(w_current, w_current->start_x, w_current->start_y, &x1,	&y1);
  SCREENtoWORLD(w_current, w_current->last_x, w_current->last_y, &x2, &y2);
  SCREENtoWORLD(w_current, w_current->second_x, w_current->second_y, &x3, &y3);

  /* Snap points to closest grid location */
  x1 = snap_grid(w_current, x1);
  y1 = snap_grid(w_current, y1);
  x2 = snap_grid(w_current, x2);
  y2 = snap_grid(w_current, y2);
  x3 = snap_grid(w_current, x3);
  y3 = snap_grid(w_current, y3);

  w_current->save_x = w_current->second_x;
  w_current->save_y = w_current->second_y;

  if (!primary_zero_length ) {
  /* create primary net */
      w_current->page_current->object_tail =
	  new_net = o_net_add(w_current,
			      w_current->page_current->object_tail,
			      OBJ_NET, color, x1, y1, x2, y2);
  
      /* conn stuff */
      /* LEAK CHECK 1 */
      other_objects = s_conn_return_others(other_objects,
					   w_current->page_current->
					   object_tail);

      if (o_net_add_busrippers(w_current, new_net, other_objects)) {
	  g_list_free(other_objects);
	  other_objects = NULL;
	  other_objects = s_conn_return_others(other_objects, new_net);
      }

#if DEBUG 
      printf("primary:\n"); 
      s_conn_print(new_net->conn_list);
#endif

      gdk_gc_set_foreground(w_current->gc, x_get_color(color));
      gdk_draw_line(w_current->window, w_current->gc,
		    new_net->line->screen_x[0], new_net->line->screen_y[0],
		    new_net->line->screen_x[1], new_net->line->screen_y[1]);
      gdk_draw_line(w_current->backingstore, w_current->gc,
		    new_net->line->screen_x[0], new_net->line->screen_y[0],
		    new_net->line->screen_x[1], new_net->line->screen_y[1]);

      if (w_current->net_style == THICK) {
	  gdk_gc_set_line_attributes(w_current->gc, 0,
				     GDK_LINE_SOLID,
				     GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
      }

      o_cue_undraw_list(w_current, other_objects);
      o_cue_draw_list(w_current, other_objects);
      o_cue_draw_single(w_current, new_net);

      /* Go off and search for valid connection on this newly created net */
      found_primary_connection = s_conn_net_search(new_net, 1, 
                                                   new_net->conn_list);
      if (found_primary_connection)
      {
      	/* if a net connection is found, reset start point of next net */
	w_current->save_x = w_current->last_x;
	w_current->save_y = w_current->last_y;
      }

      /* you don't want to consolidate nets which are drawn non-ortho */
      if (w_current->net_consolidate == TRUE && !w_current->CONTROLKEY) {
	  o_net_consolidate_segments(w_current, new_net);
      }
  }


  /* If the second net is not zero length, add it as well */
  /* Also, a valid net connection from the primary net was not found */
  if (!secondary_zero_length && !found_primary_connection) {
      
      /* Add secondary net */
      w_current->page_current->object_tail =
	  new_net = o_net_add(w_current,
			      w_current->page_current->object_tail,
			      OBJ_NET, color, x2, y2, x3, y3);
  
      /* conn stuff */
      /* LEAK CHECK 2 */
      other_objects = s_conn_return_others(other_objects,
					   w_current->page_current->
					   object_tail);

      if (o_net_add_busrippers(w_current, new_net, other_objects)) {
	  g_list_free(other_objects);
	  other_objects = NULL;
	  other_objects = s_conn_return_others(other_objects, new_net);
      }
#if DEBUG
      s_conn_print(new_net->conn_list);
#endif

      gdk_gc_set_foreground(w_current->gc, x_get_color(color));
      gdk_draw_line(w_current->window, w_current->gc,
		    new_net->line->screen_x[0], new_net->line->screen_y[0],
		    new_net->line->screen_x[1], new_net->line->screen_y[1]);
      gdk_draw_line(w_current->backingstore, w_current->gc,
		    new_net->line->screen_x[0], new_net->line->screen_y[0],
		    new_net->line->screen_x[1], new_net->line->screen_y[1]);

      if (w_current->net_style == THICK) {
	  gdk_gc_set_line_attributes(w_current->gc, 0,
				     GDK_LINE_SOLID,
				     GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
      }

      o_cue_undraw_list(w_current, other_objects);
      o_cue_draw_list(w_current, other_objects);
      o_cue_draw_single(w_current, new_net);

      /* you don't want to consolidate nets which are drawn non-ortho */
      if (w_current->net_consolidate == TRUE && !w_current->CONTROLKEY) {
	  o_net_consolidate_segments(w_current, new_net);
      }
  }
  
  /* LEAK CHECK 3 */
  g_list_free(other_objects);

  w_current->page_current->CHANGED = 1;
  w_current->start_x = w_current->save_x;
  w_current->start_y = w_current->save_y;
  o_undo_savestate(w_current, UNDO_ALL);

#if 0				/* a false attempt at ending the rubberbanding.. */
  if (conn_count) {
    w_current->inside_action = 0;
    i_set_state(w_current, STARTDRAWNET);
    o_net_eraserubber(w_current);
  }
#endif

  return (TRUE);
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void o_net_rubbernet(TOPLEVEL *w_current, int x, int y)
{
  int diff_x, diff_y;
  int size;
  int ortho;

  if (w_current->inside_action == 0) {
    o_redraw(w_current, w_current->page_current->object_head, TRUE);
    return;
  }

  if (w_current->net_style == THICK) {
    size = SCREENabs(w_current, NET_WIDTH);
    gdk_gc_set_line_attributes(w_current->xor_gc, size,
			       GDK_LINE_SOLID,
			       GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
  }
  gdk_gc_set_foreground(w_current->xor_gc,
			x_get_darkcolor(w_current->select_color));

  /* Orthognal mode enabled when Control Key is NOT pressed */
  ortho = !w_current->CONTROLKEY;

  /* Erase primary line */
  gdk_draw_line(w_current->window, w_current->xor_gc,
		w_current->start_x, w_current->start_y,
		w_current->last_x, w_current->last_y);
  /* Erase secondary line*/
  if ( w_current->second_x != -1 && w_current->second_y != -1 ) {
      gdk_draw_line(w_current->window, w_current->xor_gc,
		    w_current->last_x, w_current->last_y,
		    w_current->second_x, w_current->second_y);
  }
 
  /* In orthogonal mode secondary line is the same as the first */
  if (!ortho)
  {
      w_current->second_x = w_current->last_x;
      w_current->second_y = w_current->last_y;
  }

  w_current->last_x = fix_x(w_current, x);
  w_current->last_y = fix_y(w_current, y);

  /* If you press the control key then you can draw non-ortho nets */
  if (ortho) {
    diff_x = abs(w_current->last_x - w_current->start_x);
    diff_y = abs(w_current->last_y - w_current->start_y);

    /* calculate the co-ordinates necessary to draw the lines*/
    /* Pressing the shift key will cause the vertical and horizontal lines to switch places */
    if ( !w_current->SHIFTKEY ) {
      w_current->last_y = w_current->start_y;
      w_current->second_x = w_current->last_x;
      w_current->second_y = fix_y(w_current,y);
    } else {
      w_current->last_x = w_current->start_x;
      w_current->second_x = fix_x(w_current,x);
      w_current->second_y = w_current->last_y;
    }
  }

  gdk_gc_set_foreground(w_current->xor_gc,
			x_get_darkcolor(w_current->select_color));
  
  /* draw primary line */
  gdk_draw_line(w_current->window, w_current->xor_gc,
		w_current->start_x, w_current->start_y,
		w_current->last_x, w_current->last_y);

  /* Draw secondary line */
  gdk_draw_line(w_current->window, w_current->xor_gc,
		w_current->last_x, w_current->last_y,
		w_current->second_x, w_current->second_y);

  if (w_current->net_style == THICK) {
    gdk_gc_set_line_attributes(w_current->xor_gc, 0,
			       GDK_LINE_SOLID,
			       GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
  }
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 *  \note
 *  used in button cancel code in x_events.c
 */
void o_net_eraserubber(TOPLEVEL *w_current)
{
  int size;

  if (w_current->net_style == THICK) {
    size = SCREENabs(w_current, NET_WIDTH);

    if (size < 0)
      size = 0;

    gdk_gc_set_line_attributes(w_current->xor_gc, size,
			       GDK_LINE_SOLID,
			       GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
  }

#if 0
  gdk_gc_set_foreground(w_current->gc,
			x_get_color(w_current->background_color));
#endif

  /* Erase primary primary rubber net line */
  gdk_draw_line(w_current->window, w_current->xor_gc, w_current->start_x,
		w_current->start_y, w_current->last_x, w_current->last_y);

  /* Erase secondary rubber net line */
  gdk_draw_line(w_current->window, w_current->xor_gc,
		w_current->last_x, w_current->last_y,
		w_current->second_x, w_current->second_y);

  if (w_current->net_style == THICK) {
    gdk_gc_set_line_attributes(w_current->xor_gc, 0,
			       GDK_LINE_SOLID,
			       GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
  }
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 *  \note
 *  used in x_event_expose() in x_events.c
 */
void o_net_xorrubber(TOPLEVEL *w_current)
{
  int size;

  if (w_current->net_style == THICK ) {

    size = SCREENabs(w_current, NET_WIDTH);

    if (size < 0)
      size=0;

    gdk_gc_set_line_attributes(w_current->gc, size,
                               GDK_LINE_SOLID,
                               GDK_CAP_NOT_LAST,
                               GDK_JOIN_MITER);
  }

  gdk_gc_set_foreground(w_current->gc,
			x_get_darkcolor(w_current->select_color) );
  gdk_draw_line(w_current->window, w_current->gc, 
		w_current->start_x, w_current->start_y, 
		w_current->last_x, w_current->last_y);
  gdk_draw_line(w_current->window, w_current->gc, 
		w_current->second_x, w_current->second_y, 
		w_current->last_x, w_current->last_y);

  if (w_current->net_style == THICK ) {
    gdk_gc_set_line_attributes(w_current->gc, 0,
                               GDK_LINE_SOLID,
                               GDK_CAP_NOT_LAST,
                               GDK_JOIN_MITER);
  }
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
int o_net_add_busrippers(TOPLEVEL *w_current, OBJECT *net_obj,
			 GList *other_objects)

{
  int color;
  GList *cl_current = NULL;
  OBJECT *bus_object = NULL;
  CONN *found_conn = NULL;
  int done;
  int otherone;
  BUS_RIPPER rippers[2];
  int ripper_count = 0;
  int i;
  double length;
  int sign;
  double distance1, distance2;
  int first, second;
  int made_changes = FALSE;
  const int ripper_size = w_current->bus_ripper_size;
  int complex_angle = 0;
  char *clib = NULL;
  
  length = o_line_length(net_obj);

  if (!other_objects) {
    return(FALSE);
  }
  
  if (length <= ripper_size) {
    return(FALSE);
  }

  if (w_current->override_net_color == -1) {
    color = w_current->net_color;
  } else {
    color = w_current->override_net_color;
  }

  
  /* check for a bus connection and draw rippers if so */
  cl_current = other_objects;
  while (cl_current != NULL) {
    
    bus_object = (OBJECT *) cl_current->data;
    if (bus_object && bus_object->type == OBJ_BUS) {
      /* yes, using the net routine is okay */
      int bus_orientation = o_net_orientation(bus_object);
      int net_orientation = o_net_orientation(net_obj);

      /* find the CONN structure which is associated with this object */
      GList *cl_current2 = net_obj->conn_list;
      done = FALSE;
      while (cl_current2 != NULL && !done) {
	CONN *tmp_conn = (CONN *) cl_current2->data;

	if (tmp_conn && tmp_conn->other_object &&
	    tmp_conn->other_object == bus_object) {

	  found_conn = tmp_conn;
	  done = TRUE;
	}

	cl_current2 = cl_current2->next;
      }

      if (!found_conn) {
        return(FALSE);
      }
      
      otherone = !found_conn->whichone;
      
      /* now deal with the found connection */
      if (bus_orientation == HORIZONTAL && net_orientation == VERTICAL) {
	/* printf("found horiz bus %s %d!\n", bus_object->name, 
           found_conn->whichone);*/

        sign = bus_object->bus_ripper_direction;
        if (!sign) {
          if (bus_object->line->x[0] < bus_object->line->x[1]) {
            first = 0;
            second = 1;
          } else {
            first = 1;
            second = 0;
          }
              
          distance1 = abs(bus_object->line->x[first] -
                          net_obj->line->x[found_conn->whichone]);
          distance2 = abs(bus_object->line->x[second] -
                          net_obj->line->x[found_conn->whichone]);
          
          if (distance1 <= distance2) {
            sign = 1;
          } else {
            sign = -1;
          }
          bus_object->bus_ripper_direction = sign;
        }
        /* printf("hor sign: %d\n", sign); */

        if (net_obj->line->y[otherone] < bus_object->line->y[0]) {
          /* new net is below bus */
          /*printf("below\n");*/

          if (ripper_count >= 2) {
            /* try to exit gracefully */
            fprintf(stderr, _("Tried to add more than two bus rippers. Internal gschem error.\n"));
            made_changes = FALSE;
            break;
          }

          if (w_current->bus_ripper_rotation == NON_SYMMETRIC) {
            /* non-symmetric */
            if (sign == 1) {
              complex_angle = 0;
            } else {
              complex_angle = 90;
            }
          } else {
            /* symmetric */
            complex_angle = 0;
          }

          net_obj->line->y[found_conn->whichone] -= ripper_size;
          o_net_recalc(w_current, net_obj);
          rippers[ripper_count].x[0] = 
            net_obj->line->x[found_conn->whichone];
          rippers[ripper_count].y[0] =
            net_obj->line->y[found_conn->whichone];
          rippers[ripper_count].x[1] =
            net_obj->line->x[found_conn->whichone] + sign*ripper_size;       
          rippers[ripper_count].y[1] =
            net_obj->line->y[found_conn->whichone] + ripper_size;
          ripper_count++;
          /* printf("done\n"); */
          made_changes++;
          
        } else {
          /* new net is above bus */
          /* printf("above\n"); */

          if (ripper_count >= 2) {
            /* try to exit gracefully */
            fprintf(stderr, _("Tried to add more than two bus rippers. Internal gschem error.\n"));
            made_changes = FALSE;
            break;
          }

          if (w_current->bus_ripper_rotation == NON_SYMMETRIC) {
            /* non-symmetric */
            if (sign == 1) {
              complex_angle = 270;
            } else {
              complex_angle = 180;
            }
          } else {
            /* symmetric */
            complex_angle = 180;
          }
          
          net_obj->line->y[found_conn->whichone] += ripper_size;
          o_net_recalc(w_current, net_obj);
          rippers[ripper_count].x[0] = 
            net_obj->line->x[found_conn->whichone];
          rippers[ripper_count].y[0] =
            net_obj->line->y[found_conn->whichone];
          rippers[ripper_count].x[1] =
            net_obj->line->x[found_conn->whichone] + sign*ripper_size;      
          rippers[ripper_count].y[1] =
            net_obj->line->y[found_conn->whichone] - ripper_size;
            ripper_count++;
            
            /* printf("done\n"); */
          made_changes++;
        }
        
        
      } else if (bus_orientation == VERTICAL &&
		 net_orientation == HORIZONTAL) {

	/* printf("found vert bus %s %d!\n", bus_object->name,
           found_conn->whichone); */

        sign = bus_object->bus_ripper_direction;
        if (!sign) {
          if (bus_object->line->y[0] < bus_object->line->y[1]) {
            first = 0;
            second = 1;
          } else {
            first = 1;
            second = 0;
          }

          distance1 = abs(bus_object->line->y[first] -
                          net_obj->line->y[found_conn->whichone]);
          distance2 = abs(bus_object->line->y[second] -
                          net_obj->line->y[found_conn->whichone]);
          
          if (distance1 <= distance2) {
            sign = 1;
          } else {
            sign = -1;
          }
          bus_object->bus_ripper_direction = sign;
        } 
        /* printf("ver sign: %d\n", sign); */

        
        if (net_obj->line->x[otherone] < bus_object->line->x[0]) {
          /* new net is to the left of the bus */
          /* printf("left\n"); */
          
          if (ripper_count >= 2) {
            /* try to exit gracefully */
            fprintf(stderr, _("Tried to add more than two bus rippers. Internal gschem error.\n"));
            made_changes = FALSE;
            break;
          }

          if (w_current->bus_ripper_rotation == NON_SYMMETRIC) {
            /* non-symmetric */
            if (sign == 1) {
              complex_angle = 0;
            } else {
              complex_angle = 270;
            }
          } else {
            /* symmetric */
            complex_angle = 270;
          }

          net_obj->line->x[found_conn->whichone] -= ripper_size;
          o_net_recalc(w_current, net_obj);
          rippers[ripper_count].x[0] = 
            net_obj->line->x[found_conn->whichone];
          rippers[ripper_count].y[0] =
            net_obj->line->y[found_conn->whichone];
          rippers[ripper_count].x[1] =
            net_obj->line->x[found_conn->whichone] + ripper_size;
          rippers[ripper_count].y[1] =
            net_obj->line->y[found_conn->whichone] + sign*ripper_size;
          ripper_count++;
                    
          made_changes++;
        } else {
          /* new net is to the right of the bus */
          /* printf("right\n"); */

          if (ripper_count >= 2) {
            /* try to exit gracefully */
            fprintf(stderr, _("Tried to add more than two bus rippers. Internal gschem error.\n"));
            made_changes = FALSE;
            break;
          }

          if (w_current->bus_ripper_rotation == NON_SYMMETRIC) {
            /* non-symmetric */
            if (sign == 1) {
              complex_angle = 90;
            } else {
              complex_angle = 180;
            }
          } else {
            /* symmetric */
            complex_angle = 90;
          }

          net_obj->line->x[found_conn->whichone] += ripper_size;
          o_net_recalc(w_current, net_obj);
          rippers[ripper_count].x[0] = 
            net_obj->line->x[found_conn->whichone];
          rippers[ripper_count].y[0] =
            net_obj->line->y[found_conn->whichone];
          rippers[ripper_count].x[1] =
            net_obj->line->x[found_conn->whichone] - ripper_size;
          rippers[ripper_count].y[1] =
            net_obj->line->y[found_conn->whichone] + sign*ripper_size;
          ripper_count++;

          made_changes++;
        }
      }
    }


    cl_current = cl_current->next;
  }
 
  if (made_changes) {
    s_conn_remove(w_current, net_obj);

    if (w_current->bus_ripper_type == COMP_BUS_RIPPER) {
      const GSList *clibs = s_clib_search_basename (w_current->bus_ripper_symname);
      if (clibs != NULL) {
        clib = (gchar*)clibs->data;
      }
    }
    
    for (i = 0; i < ripper_count; i++) {
      if (w_current->bus_ripper_type == NET_BUS_RIPPER) {
        w_current->page_current->object_tail =
          o_net_add(w_current, w_current->page_current->object_tail,
                    OBJ_NET, color,
                    rippers[i].x[0], rippers[i].y[0],
                    rippers[i].x[1], rippers[i].y[1]);
      } else {

        if (clib) {
          w_current->page_current->object_tail = 
          (OBJECT *) o_complex_add(
                                   w_current,
                                   w_current->page_current->object_tail,
				   NULL,
                                   OBJ_COMPLEX, WHITE,
                                   rippers[i].x[0], rippers[i].y[0],
                                   complex_angle, 0,
                                   clib,
                                   w_current->bus_ripper_symname, 1, TRUE);
          
          o_complex_draw(w_current,w_current->page_current->object_tail);
        } else {
          s_log_message(_("Could not find %s in any component-library\n"),
                        w_current->bus_ripper_symname);
        }
      }
    }
    
    s_conn_update_object(w_current, net_obj);
    return(TRUE);
  }

  return(FALSE);
}
