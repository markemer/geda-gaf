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

#include <math.h>
#include <stdio.h>

#include <libgeda/libgeda.h>

#include "../include/x_states.h"
#include "../include/prototype.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
gboolean o_find_object(TOPLEVEL *w_current, int screen_x, int screen_y,
		       gboolean change_selection)
{
  OBJECT *o_current=NULL;
  gboolean object_found = FALSE;

  if (w_current->page_current->object_lastplace == NULL) {
    o_current = w_current->page_current->object_head;
  } else {
    o_current = w_current->page_current->object_lastplace;
  }

  /* do first search */
  while (o_current != NULL) {
    if (inside_region(o_current->left, o_current->top,
                      o_current->right, o_current->bottom, 
                      screen_x, screen_y)) {
      if (o_current->sel_func != NULL &&
	  o_current->type != OBJ_HEAD &&
	  (o_current->visibility == VISIBLE ||
	   (o_current->visibility == INVISIBLE &&
	    w_current->show_hidden_text))) {
	if (change_selection) {
	  (*o_current->sel_func)(
				 w_current, o_current, 
				 SINGLE, 0); /* 0 is count */
	}
	object_found = TRUE;
	w_current->page_current-> object_lastplace =
	  o_current->next;
	i_update_menus(w_current);
	return object_found;
      }
    }
    o_current = o_current->next;
  } 

#if DEBUG
  printf("SEARCHING AGAIN\n");
#endif

  /* now search again since we didn't find anything starting at start
     just in case we started last time at object_lastplace */
  o_current = w_current->page_current->object_head;
  while (o_current != NULL && 
         o_current != w_current->page_current->object_lastplace) {
    if (inside_region(o_current->left, o_current->top,
                      o_current->right, o_current->bottom, 
                      screen_x, screen_y)) {
      
      if (o_current->sel_func != NULL &&
          o_current->type != OBJ_HEAD &&
          (o_current->visibility == VISIBLE ||
           (o_current->visibility == INVISIBLE &&
            w_current->show_hidden_text))) {
	if (change_selection) {
	  /* 0 is count */
	  (*o_current->sel_func)(w_current, o_current, SINGLE, 0);
	}
	w_current->page_current->object_lastplace = o_current;
 	object_found = TRUE;
        
        i_update_menus(w_current);
	return object_found;
      }
    }
    o_current = o_current->next;
  }

  /* didn't find anything.... reset lastplace */
  w_current->page_current->object_lastplace = NULL;

  /* deselect everything only if shift key isn't pressed and 
     the caller allows it */	
  if (change_selection && (!w_current->SHIFTKEY)) {

#if DEBUG
    o_selection_print_all( w_current->page_current->selection_list);
#endif
    o_select_run_hooks(w_current, NULL, 2); 
    o_selection_unselect_list (w_current,
			       &(w_current->page_current->selection_list));
  }

  i_update_menus(w_current);
  
  return (object_found);
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
gboolean o_find_selected_object(TOPLEVEL *w_current,
				int screen_x, int screen_y)
{
  OBJECT *o_current=NULL;
  GList *s_current;

  s_current = w_current->page_current->selection_list;
  /* do first search */
  while (s_current != NULL) {
    o_current = (OBJECT *) s_current->data;
    if (inside_region(o_current->left, o_current->top,
                      o_current->right, o_current->bottom, 
                      screen_x, screen_y)) {

#if DEBUG
      printf("o_find_selected_object:\n");
      printf("Object bounds:\n\tL: %i\tR: %i\n\tT: %i\tB: %i.\n",
	     o_current->left, o_current->right, o_current->top, o_current->bottom);
      printf("Screen pointer at: (%i,%i)\n", screen_x, screen_y);
#endif
      if (o_current->sel_func != NULL &&
	  o_current->type != OBJ_HEAD &&
	  (o_current->visibility == VISIBLE ||
	   (o_current->visibility == INVISIBLE &&
	    w_current->show_hidden_text))) {
	return TRUE;
      }
    }
    
    s_current = s_current->next;
  } 

  return (FALSE);
}
