/* gEDA - GPL Electronic Design Automation
 * libgeda - gEDA's library
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
#include <ctype.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif
#ifndef HAVE_VSNPRINTF
#include <stdarg.h>
#endif

#include <gtk/gtk.h>
#include <libguile.h>

#include "defines.h"
#include "struct.h"
#include "defines.h"
#include "globals.h"
#include "o_types.h"
#include "colors.h"

#include "../include/prototype.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

/*! \brief Selects the given object and adds it to the selection list
 *  \par Selects the given object and does the needed work to make the
 *  object visually selected.
 *  \param [in] head Selection list
 *  \param [in] o_selected Object to select.
 *  \returns a pointer to the selection list, with the object added.
 */
GList *o_selection_add(GList *head, OBJECT *o_selected)
{
  o_selection_select(o_selected, SELECT_COLOR);
  return (g_list_append(head, o_selected));
}

/*! \brief Prints the given selection list.
 *  \par Prints the given selection list.
 *  \param [in] head Selection list to print.
 *
 */
void o_selection_print_all( GList *head )
{
  GList *s_current;

  s_current = head;

  printf("START printing selection ********************\n");
  while(s_current != NULL) {
    if (s_current->data) {
      printf("Selected object: %d\n", 
	     ( (OBJECT *) s_current->data)->sid);
    }
    s_current = s_current->next;
  }
  printf("DONE printing selection ********************\n");
  printf("\n");

}

/*! \brief Selects the given object.
 *  \par Sets the select flag, saves the color, and then selects the 
 *  given object
 *  \param [in] o_selected Object to select.
 *  \param [in] color color of the selected object.
 */
void o_selection_select(OBJECT *object, int color)
{
  if (object->selected == TRUE) {
    printf("object already selected == TRUE\n");
    return;
  }

  if (object->saved_color != -1) {
    printf("object already saved_color != -1\n");
    return;
  }

  object->selected = TRUE;
  object->draw_grips = TRUE;
  object->saved_color = object->color;
  object->color = color;
  if (object->type == OBJ_COMPLEX || object->type == OBJ_PLACEHOLDER) { 
    o_complex_set_color_save(object->complex->prim_objs, color);
  } else if (object->type == OBJ_TEXT) {
    o_complex_set_color_save(object->text->prim_objs, color);
  }
}

/*! \brief Unselects the given object.
 *  \par Unsets the select flag, restores the original color of the
 *  given object.
 *  This function should not be called by anybody outside of this file.
 *  \param [in] object Object to unselect.
 */
void o_selection_unselect(OBJECT *object)
{
  object->selected = FALSE;
  /* object->draw_grips = FALSE; can't do this here... */
  /* draw_grips is cleared in the individual draw functions after the */
  /* grips are erase */
  object->color = object->saved_color;
  if (object->type == OBJ_COMPLEX || object->type == OBJ_PLACEHOLDER) { 
    if (!object->complex) {
      fprintf(stderr, "o_selection_unselect: Called with NULL object.\n");
      return;
    }
    o_complex_unset_color(object->complex->prim_objs);
  } else if (object->type == OBJ_TEXT) {
    if (!object->text) {
      fprintf(stderr, "o_selection_unselect: Called with NULL object.\n");
      return;
    }
    o_complex_unset_color(object->text->prim_objs);
  }

  object->saved_color = -1;
}

/*! \brief Removes the given object from the selection list
 *  \par Removes the given object from the selection list and does the 
 *  needed work to make the object visually unselected.
 *  It's ok to call this function with an object which is not necessarily
 *  selected.
 *  \param [in] head Pointer to the selection list
 *  \param [in] o_selected Object to unselect and remove from the list.
 */
void
o_selection_remove(GList **head, OBJECT *o_selected)
{
  if (o_selected == NULL) {
    fprintf(stderr, "Got NULL for o_selected in o_selection_remove\n");
    return;
  }

  if (g_list_find(*head, o_selected) != NULL) {
    o_selection_unselect(o_selected);
    *head = g_list_remove(*head, o_selected);
  }
}

/*! \brief Unselects all the objects in the given list.
 *  \par Unselects all objects in the given list, does the 
 *  needed work to make the objects visually unselected, and redraw them.
 *  \param [in] w_current TOPLEVEL struct.
 *  \param [in] head Pointer to the selection list
 */
void
o_selection_unselect_list(TOPLEVEL *w_current, GList **head)
{
  GList *list = *head;
  
  while (list != NULL) {
    o_selection_unselect((OBJECT *) list->data);
    o_redraw_single(w_current, (OBJECT *) list->data);
   list = list->next;
  }
  
  g_list_free(*head);
  *head = NULL;  
}
