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
#include <string.h>

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
void o_copy_start(TOPLEVEL *w_current, int x, int y)
{
  GList *s_current = NULL;
  GList *new_objects = NULL;
  OBJECT *object = NULL;
  OBJECT *new_object = NULL;
  OBJECT *complex_object = NULL;
  OBJECT *new_objects_head = NULL;
  int color;
  /* int redraw_state;  not needed for now */

  if (w_current->page_current->selection_list == NULL) {
    w_current->inside_action = 0;
    i_set_state(w_current, SELECT);
    return;
  }

  o_undo_savestate(w_current, UNDO_ALL);

  w_current->last_drawb_mode = -1;
  
  object = o_select_return_first_object(w_current);
  
  if (object == NULL) {
    /* TODO: error condition */
    w_current->inside_action = 0;
    i_set_state(w_current, SELECT);
    return;
  }

  w_current->start_x = w_current->last_x = fix_x(w_current, (int) mouse_x);
  w_current->start_y = w_current->last_y = fix_x(w_current, (int) mouse_y);

  s_current = w_current->page_current->selection_list;
  new_objects_head = s_basic_init_object("object_head");

  while(s_current != NULL) {

    object = (OBJECT *) s_current->data;

    if (object == NULL) {
      fprintf(stderr, _("ERROR: NULL object in o_copy_start!\n"));
      exit(-1);
    }

    o_selection_unselect(object);

    switch(object->type) {

      case(OBJ_NET):

	/* ADDING_SEL is a bad name, rename hack */
	/* basically I don't want to add the */
	/* connections till much later */
        w_current->ADDING_SEL=1;
        new_object = (OBJECT *) o_net_copy(w_current,
					   return_tail(new_objects_head),
                                           object);
        w_current->ADDING_SEL=0; 

        new_object->saved_color = object->saved_color;
        new_object->color = object->color;

        s_conn_update_object(w_current, new_object);
        new_objects = o_selection_add(new_objects, new_object);
        break;
        
      case(OBJ_PIN):
	/* ADDING_SEL is a bad name, rename hack */
	/* basically I don't want to add the */
	/* connections till much later */
        w_current->ADDING_SEL=1; 
        new_object = (OBJECT *) o_pin_copy(w_current,
                                           return_tail(new_objects_head), 
                                           object);
        w_current->ADDING_SEL=0; 
        new_object->saved_color = object->saved_color;
        new_object->color = object->color;
        
        s_conn_update_object(w_current, new_object);
        new_objects = o_selection_add(new_objects, new_object);
        break;

      case(OBJ_BUS):
	/* ADDING_SEL is a bad name, rename hack */
	/* basically I don't want to add the */
	/* connections till much later */
        w_current->ADDING_SEL=1; 
        new_object = (OBJECT *) o_bus_copy(w_current,
                                           return_tail(new_objects_head),
                                           object);
        w_current->ADDING_SEL=0; 
        new_object->saved_color = object->saved_color;
        new_object->color = object->color;
        
        s_conn_update_object(w_current, new_object);
        new_objects = o_selection_add(new_objects, new_object);
        break;
        
      case(OBJ_COMPLEX):
      case(OBJ_PLACEHOLDER):
        w_current->ADDING_SEL=1; 
        if (object->complex_clib && 
            strncmp(object->complex_clib, "EMBEDDED", 8) == 0) {

          new_object = (OBJECT *) 
            o_complex_copy_embedded(w_current, 
				    return_tail(new_objects_head), 
                                    object);

        } else {
          new_object = (OBJECT *) o_complex_copy(w_current,
                                                 return_tail(new_objects_head), 
                                                 object);
        }
        w_current->ADDING_SEL=0; 

        new_object->saved_color = object->saved_color;
        new_object->color = object->color;
				
        complex_object = new_object;
	/* NEWSEL: this needs to be fixed too */
	/* this may not be needed anymore? */
        o_attrib_slot_copy(w_current, object, 
                           new_object);

        s_conn_update_complex(w_current, new_object->complex->prim_objs);
        new_objects = o_selection_add(new_objects, new_object);
        break;

      case(OBJ_LINE):
        new_object = (OBJECT *) o_line_copy(w_current,
                                            return_tail(new_objects_head),
                                            object);
        new_object->saved_color = object->saved_color;
        new_object->color = object->color;
        new_objects = o_selection_add(new_objects, new_object);
        break;

      case(OBJ_BOX):
        new_object = (OBJECT *) o_box_copy(w_current,
                                           return_tail(new_objects_head),
                                           object);
        new_object->saved_color = object->saved_color;
        new_object->color = object->color;
        new_objects = o_selection_add(new_objects, new_object);
        
        break;

      case(OBJ_PICTURE):
        new_object = (OBJECT *) o_picture_copy(w_current,
					       return_tail(new_objects_head),
					       object);
        new_object->saved_color = object->saved_color;
        new_object->color = object->color;
        new_objects = o_selection_add(new_objects, new_object);
        
        break;

      case(OBJ_CIRCLE):
        new_object = (OBJECT *) o_circle_copy(w_current,
                                              return_tail(new_objects_head),
                                              object);
        new_object->saved_color = object->saved_color;
        new_object->color = object->color;
        new_objects = o_selection_add(new_objects, new_object);
        break;

      case(OBJ_ARC):
        new_object = (OBJECT *) o_arc_copy(w_current,
					   return_tail(new_objects_head), 
                                           object);
        new_object->saved_color = object->saved_color;
        new_object->color = object->color;
        new_objects = o_selection_add(new_objects, new_object);
        break;

    }

    o_selection_select(object, SELECT_COLOR);

    w_current->page_current->object_tail =
      (OBJECT *) return_tail(w_current->page_current->
                             object_head);
    s_current = s_current->next;
  }

  /* skip over head */
  s_current = w_current->page_current->selection_list;
  while(s_current != NULL) {

    object = (OBJECT *) s_current->data;

    if (object == NULL) {
      fprintf(stderr, _("ERROR: NULL object in o_copy_start!\n"));
      exit(-1);
    }

    switch(object->type) {

      case(OBJ_TEXT):
        w_current->ADDING_SEL=1; 
        new_object = (OBJECT *) o_text_copy(w_current,
                                            return_tail(new_objects_head),
                                            object);
        w_current->ADDING_SEL=0; 
        new_object->color = object->color;

	/* this is also okay NEWSEL new_obj is single */
        if (object->attached_to) {
          if (object->attached_to->copied_to) {
            o_attrib_attach(w_current, new_objects_head,
                            new_object, object->attached_to-> copied_to);
	    	    
            /* TODO: I have no idea if this is
               really needed.... ? */
#if 0
            o_attrib_slot_update(
                                 w_current,
                                 object->attached_to->copied_to);
#endif

            /* satisfied copy request */
            object->attached_to->copied_to = NULL;
          }
        }
				/* old object was attr */
        if (!new_object->attribute && 
            object->attribute) {		 	  
          o_complex_set_color(new_object, new_object->color);
          new_object->visibility = VISIBLE;
          color = new_object->color;
        } else {
	  /*          color = object->saved_color;*/
	  color = object->color;
        }

        new_objects = o_selection_add(new_objects, new_object);
        
	/* signify that object is no longer an attribute */
	/*	o_text_draw(w_current, new_object);*/

        o_complex_set_saved_color_only(new_object->text->prim_objs, 
                                       color);
        break;
    }

    w_current->page_current->object_tail =
      (OBJECT *) return_tail(w_current->page_current->
                             object_head);
    s_current = s_current->next;
  }

  w_current->page_current->complex_place_list = new_objects;
  o_drawbounding(w_current, NULL, w_current->page_current->complex_place_list,
		 x_get_darkcolor(w_current->bb_color), TRUE);
  w_current->inside_action = 1;
}


void
o_copy_end(TOPLEVEL *w_current)
{
  OBJECT *object, *new_objects_head;
  GList *list_ptr;
  GList *connected_objects=NULL;
  int diff_x, diff_y;
  int screen_diff_x, screen_diff_y;
  int lx, ly;
  int sx, sy;

  w_current->last_drawb_mode = -1;

  screen_diff_x = w_current->last_x - w_current->start_x;
  screen_diff_y = w_current->last_y - w_current->start_y;

  SCREENtoWORLD(w_current,
                w_current->last_x,
                w_current->last_y,
                &lx,
                &ly);
  SCREENtoWORLD(w_current,
                w_current->start_x,
                w_current->start_y,
                &sx,
                &sy);

  diff_x = lx - sx;
  diff_y = ly - sy;

  list_ptr = w_current->page_current->complex_place_list;
  while (list_ptr) {
    object = (OBJECT *) list_ptr->data;

    switch(object->type) {
      case(OBJ_NET):
        if (w_current->actionfeedback_mode == OUTLINE) {
          o_net_draw_xor(w_current,
                         screen_diff_x, screen_diff_y,
                         object);
        }

        o_net_translate_world(w_current,
                              diff_x, diff_y,
                              object);
        connected_objects = s_conn_return_others(connected_objects,
                                                 object);
	break;
      case(OBJ_PIN):
        if (w_current->actionfeedback_mode == OUTLINE) {
          o_pin_draw_xor(w_current,
                         screen_diff_x,
                         screen_diff_y,
                         object);
        }

        o_pin_translate_world(w_current,
                              diff_x, diff_y,
                              object);
        connected_objects = s_conn_return_others(connected_objects,
                                                 object);
        break;

      case(OBJ_BUS):
        if (w_current->actionfeedback_mode == OUTLINE) {
          o_bus_draw_xor(w_current,
                         screen_diff_x, screen_diff_y,
                         object);
        }

        o_bus_translate_world(w_current,
                              diff_x, diff_y,
                              object);
        connected_objects = s_conn_return_others(connected_objects,
                                                 object);
        break;
      case(OBJ_COMPLEX):
      case(OBJ_PLACEHOLDER):
        if (w_current->actionfeedback_mode == OUTLINE) {
          o_complex_draw_xor(w_current, screen_diff_x, screen_diff_y,
                             object->complex->prim_objs);
        }

        o_complex_world_translate_toplevel(w_current,
                                           diff_x,
                                           diff_y,
                                           object);
        connected_objects = s_conn_return_complex_others(connected_objects,
                                                         object);
	break;

      case(OBJ_LINE):
        if (w_current->actionfeedback_mode == OUTLINE) {
          o_line_draw_xor(w_current,
                          screen_diff_x, screen_diff_y,
                          object);
        }

        w_current->ADDING_SEL=1; 
        o_line_translate_world(w_current,
                               diff_x, diff_y,
                               object);
        w_current->ADDING_SEL=0; 
	break;
      case(OBJ_BOX):
        if (w_current->actionfeedback_mode == OUTLINE) {
          o_box_draw_xor(w_current,
                         screen_diff_x,
                         screen_diff_y,
                         object);
        }

        w_current->ADDING_SEL=1; 
        o_box_translate_world(w_current,
                              diff_x, diff_y,
                              object);
        w_current->ADDING_SEL=0; 
	break;

      case(OBJ_PICTURE):
        if (w_current->actionfeedback_mode == OUTLINE) {
          o_picture_draw_xor(w_current,
			     screen_diff_x,
			     screen_diff_y,
			     object);
        }

        w_current->ADDING_SEL=1; 
        o_picture_translate_world(w_current,
				  diff_x, diff_y,
				  object);
        w_current->ADDING_SEL=0; 
	break;
      case(OBJ_CIRCLE):
        if (w_current->actionfeedback_mode == OUTLINE) {
          o_circle_draw_xor(w_current,
                            screen_diff_x,
                            screen_diff_y,
                            object);
        }

        w_current->ADDING_SEL=1; 
        o_circle_translate_world(w_current,
                                 diff_x, diff_y,
                                 object);
        w_current->ADDING_SEL=0; 
	break;
      case(OBJ_ARC):
        if (w_current->actionfeedback_mode == OUTLINE) {
          o_arc_draw_xor(w_current,
                         screen_diff_x,
                         screen_diff_y,
                         object);
        }

        w_current->ADDING_SEL=1; 
        o_arc_translate_world(w_current,
                              diff_x, diff_y,
                              object);
        w_current->ADDING_SEL=0; 
	break;
      case(OBJ_TEXT):
	
        if (w_current->actionfeedback_mode == OUTLINE) {
          o_text_draw_xor(w_current,
                          screen_diff_x,
                          screen_diff_y,
                          object);
			  }

        w_current->ADDING_SEL=1; 
        o_text_translate_world(w_current, diff_x, diff_y, object);
        w_current->ADDING_SEL=0; 
	break;
    }
    list_ptr = list_ptr->next;
  }


  /* Add the new objects */
  list_ptr = g_list_first(w_current->page_current->complex_place_list);
  if (list_ptr == NULL) {
    fprintf(stderr, "o_copy_end: Something went wrong. Selection list is empty!\n");
    return;
  }
  new_objects_head = (OBJECT *) (list_ptr->data);

  s_basic_link_object(new_objects_head, w_current->page_current->object_tail);

  /* Run the copy component hook */
  object = new_objects_head->next;
  while (object != NULL) {
    if ((object->type == OBJ_COMPLEX) &&
	(scm_hook_empty_p(copy_component_hook) == SCM_BOOL_F)) {
      scm_run_hook(copy_component_hook,
		   scm_cons (g_make_attrib_smob_list(w_current, object),
		   SCM_EOL));
    }
    object = object->next;
  }
  
  /* And redraw them */
  object = new_objects_head;
  while (object) {
    o_redraw_single(w_current, object);
    object=object->next;
  }
  
  w_current->page_current->object_tail = (OBJECT *)
  return_tail(w_current->page_current->object_head);

  /* erase the bounding box */
  if (w_current->actionfeedback_mode == BOUNDINGBOX) {
    o_drawbounding(w_current, NULL,
                   w_current->page_current->complex_place_list,
                   x_get_darkcolor(w_current->bb_color), TRUE);
  }

  o_select_run_hooks(w_current, NULL, 2); 
  o_selection_unselect_list(w_current,
			    &(w_current->page_current->selection_list));
  w_current->page_current->selection_list = w_current->page_current->complex_place_list;

#if DEBUG
  o_selection_print_all(w_current->page_current->selection_list);
#endif

  w_current->page_current->CHANGED = 1;

  /* not needed o_redraw(w_current, w_current->page_current->object_head); */
  o_cue_draw_list(w_current, w_current->page_current->complex_place_list);
  o_cue_undraw_list(w_current, connected_objects);
  o_cue_draw_list(w_current, connected_objects);

  o_undo_savestate(w_current, UNDO_ALL);
  g_list_free(connected_objects);

  w_current->page_current->complex_place_list = NULL;
}
