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
#include <ctype.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <libgen.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <libgeda/libgeda.h>

#include "../include/globals.h"
#include "../include/prototype.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

#define FOCUS_SELECTION 0
#define FOCUS_SHEET 1
#define FOCUS_ALL 2

#define SELECTION_ALL 0
#define SELECTION_UNNUMBERED 1

#define AUTONUMBER_SKIP_NOT 0
#define AUTONUMBER_SKIP_PAGE 1
#define AUTONUMBER_SKIP_SELECTION 2

#define AUTONUMBER_SORT_YX 1
#define AUTONUMBER_SORT_XY 2
#define AUTONUMBER_SORT_DIAGONAL 3

/* break with the tradition here and input a list */
/*! \todo probably should go back and do the same for o_copy o_move
 *  o_delete...
 */
/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void o_edit(TOPLEVEL *w_current, GList *list)
{
  char *equal_ptr;
  OBJECT *o_current;
  int num_lines = 0;

  if (list == NULL) {
    w_current->inside_action = 0;
    i_set_state(w_current, SELECT);
    return;
  }

  o_current = (OBJECT *) list->data;	
  if (o_current == NULL) {
    fprintf(stderr, _("Got an unexpected NULL in o_edit\n"));
    exit(-1);
  }

  /* for now deal with only the first item */
  switch(o_current->type) {

    /* also add the ability to multi attrib edit: nets, busses, pins */
    case(OBJ_COMPLEX):
    case(OBJ_PLACEHOLDER):
    case(OBJ_NET):
    case(OBJ_PIN):
    case(OBJ_BUS):
    x_multiattrib_open (w_current, o_current);
    break;

    case(OBJ_PICTURE):
    picture_change_filename_dialog(w_current);
    break;
    case(OBJ_TEXT):
    if(strchr(o_current->text->string,'=')) {

      /* now really make sure it's an attribute by 
       * checking that there are NO spaces around the ='s 
       */
      equal_ptr = strchr(o_current->text->string, '=');

      /* and also make sure it is only a single line */
      num_lines = o_text_num_lines(o_current->text->string);

      /* there is a possiblity for core dump yes? */
      /* by exceeding the size of the text_string? */
      /* or maybe not, since if the ='s is at the end of */
      /* the string, there better be a null after it! */
      if ( (*(equal_ptr + 1) != ' ') &&
           (*(equal_ptr - 1) != ' ') &&
           (num_lines == 1) ) {
	        attrib_edit_dialog(w_current,o_current, FROM_MENU); 
	/*	multi_attrib_edit(w_current, o_current); */

      } else {
        o_text_edit(w_current, o_current);
      } 
    } else {
      o_text_edit(w_current, o_current);
    }
    break;
  }

  /* has to be more extensive in the future */
  /* some sort of redrawing? */
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
/* This locks the entire selected list.  It does lock components, but does NOT
 * change the color (of primatives of the components) though 
 * this cannot be called recursively */
void o_lock(TOPLEVEL *w_current)
{
  OBJECT *object = NULL;
  GList *s_current = NULL;

  /* skip over head */
  s_current = w_current->page_current->selection_list;

  while(s_current != NULL) {
    object = (OBJECT *) s_current->data;
    if (object) {
      /* check to see if locked_color is already being used */
      if (object->locked_color == -1) {
        object->sel_func = NULL;
        object->locked_color = object->color;
        object->color = w_current->lock_color;
        w_current->page_current->CHANGED=1;
      } else {
        s_log_message(_("Object alreadly locked\n"));
      }
    }

    s_current=s_current->next;
  }

  o_unselect_all(w_current);
  o_undo_savestate(w_current, UNDO_ALL);
  i_update_menus(w_current);
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
/* You can unlock something by selecting it with a bounding box... */
/* this will probably change in the future, but for now it's a
   something.. :-) */
/* this cannot be called recursively */
void o_unlock(TOPLEVEL *w_current)
{
  OBJECT *object = NULL;
  GList *s_current = NULL;

  s_current = w_current->page_current->selection_list;

  while(s_current != NULL) {
    object = (OBJECT *) s_current->data;
    if (object) {
      /* only unlock if sel_func is not set to something */
      if (object->sel_func == NULL) {
        object->sel_func = select_func;
        object->color = object->locked_color;
        object->locked_color = -1;
        w_current->page_current->CHANGED = 1;
      } else {
        s_log_message(_("Object alreadly unlocked\n"));
      }
    }

    s_current=s_current->next;
  }
  o_undo_savestate(w_current, UNDO_ALL);
}

/*! \brief Rotate all objects in list.
 *  \par Function Description
 *  Given the selection <B>list</B>, and the center of rotation
 *  (<B>centerx</B>,<B>centery</B>, this function traverses all the selection
 *  list, rotating each object.
 *  The selection list contains a given object and all its attributes
 *  (refdes, pinname, pinlabel, ...).
 *  There is a second pass to run the rotate hooks of non-simple objects,
 *  like pin or complex objects, for example.
 * 
 *  \param [in] w_current  The TOPLEVEL object.
 *  \param [in] list       The list of objects to rotate.
 *  \param [in] centerx    Center x coordinate of rotation.
 *  \param [in] centery    Center y coordinate of rotation.
 */
void o_rotate_90(TOPLEVEL *w_current, GList *list,
		 int centerx, int centery)
{
  OBJECT *object;
  GList *s_current;
  int new_angle;
  GList *other_objects=NULL;
  GList *connected_objects=NULL;
  OBJECT *o_current=NULL;
        
  /* this is okay if you just hit rotate and have nothing selected */
  if (list == NULL) {
    w_current->inside_action = 0;
    i_set_state(w_current, SELECT);
    return;
  }

  s_current = list;

  while (s_current != NULL) {
    object = (OBJECT *) s_current->data;

    if (!object) {
      fprintf(stderr, _("ERROR: NULL object in o_rotate_90!\n"));
      return;
    }

    g_list_free(other_objects);
    other_objects = NULL;
    g_list_free(connected_objects);
    connected_objects = NULL;

    switch(object->type) {


      case(OBJ_NET):
	if (!w_current->DONT_REDRAW) {
	  o_cue_undraw(w_current, object);
	  o_net_erase(w_current, object);
	  o_line_erase_grips(w_current, object);
	}
                                
        /* save the other objects */
        other_objects = s_conn_return_others(other_objects, object);
        s_conn_remove(w_current, object);
                                
        o_net_rotate(w_current, centerx, centery, 90, object);
        s_conn_update_object(w_current, object);
	if (!w_current->DONT_REDRAW) {
	  o_net_draw(w_current, object);
                                
	  /* draw the other objects */
	  o_cue_undraw_list(w_current, other_objects);
	  o_cue_draw_list(w_current, other_objects);
	}

        /* get other connected objects and redraw */
        connected_objects = s_conn_return_others(connected_objects, object);
	if (!w_current->DONT_REDRAW) {
	  o_cue_undraw_list(w_current, connected_objects);
	  o_cue_draw_list(w_current, connected_objects);
	  
	  /* finally redraw the cues on the current object */
	  o_cue_draw_single(w_current, object); 
	}
        break;

      case(OBJ_BUS):
	if (!w_current->DONT_REDRAW) {
	  o_cue_undraw(w_current, object);
	  o_bus_erase(w_current, object);
	  o_line_erase_grips(w_current, object);
        }

        other_objects = s_conn_return_others(other_objects, object);
        s_conn_remove(w_current, object);
        
        o_bus_rotate(w_current, centerx, centery, 90, object);
        s_conn_update_object(w_current, object);
	if (!w_current->DONT_REDRAW) {
	  o_bus_draw(w_current, object);
        
	  /* draw the other objects */
	  o_cue_undraw_list(w_current, other_objects);
	  o_cue_draw_list(w_current, other_objects);
	}

        /* get other connected objects and redraw */
        connected_objects = s_conn_return_others(connected_objects, object);
	if (!w_current->DONT_REDRAW) {
	  o_cue_undraw_list(w_current, connected_objects);
	  o_cue_draw_list(w_current, connected_objects);

	  /* finally redraw the cues on the current object */
	  o_cue_draw_single(w_current, object); 
	}
        break;

      case(OBJ_PIN):
	if (!w_current->DONT_REDRAW) {
	  o_cue_undraw(w_current, object);
	  o_pin_erase(w_current, object);
	  o_line_erase_grips(w_current, object);
	}
        
        other_objects = s_conn_return_others(other_objects, object);
        s_conn_remove(w_current, object);
        
        o_pin_rotate(w_current, centerx, centery, 
                     90, object);
        s_conn_update_object(w_current, object);
	if (!w_current->DONT_REDRAW) {
	  o_pin_draw(w_current, object);
        
	  /* draw the other objects */
	  o_cue_undraw_list(w_current, other_objects);
	  o_cue_draw_list(w_current, other_objects);
	}
  
	/* get other connected objects and redraw */
        connected_objects = s_conn_return_others(connected_objects, object);
	if (!w_current->DONT_REDRAW) {
	  o_cue_undraw_list(w_current, connected_objects);
	  o_cue_draw_list(w_current, connected_objects);
	  
	  /* finally redraw the cues on the current object */
	  o_cue_draw_single(w_current, object);
	}
        break;

      case(OBJ_COMPLEX):
	if (!w_current->DONT_REDRAW) {
	  o_cue_undraw_objects(w_current, object->complex->prim_objs);
	  /* erase the current selection */
	  o_complex_erase(w_current, object);
	}

        other_objects = s_conn_return_complex_others(other_objects, object);
        
        /* remove all conn references */
        o_current = object->complex->prim_objs;
        while(o_current != NULL) {
          s_conn_remove(w_current, o_current);
          o_current = o_current->next;
        }
      
        /* do the rotate */
        /*w_current->ADDING_SEL=1; NEWSEL: needed? */
        new_angle = (object->complex->angle + 90) % 360;
        o_complex_rotate(w_current, centerx, centery,
                         new_angle, 90, object);
        /*w_current->ADDING_SEL = 0; NEWSEL: needed? */
        s_conn_update_complex(w_current, object->complex->prim_objs);
	if (!w_current->DONT_REDRAW) {
	  o_complex_draw(w_current, object);
	  
	  o_cue_undraw_list(w_current, other_objects);
	  o_cue_draw_list(w_current, other_objects);
	}

        /* now draw the newly connected objects */
        connected_objects = s_conn_return_complex_others(connected_objects,
                                                         object);
	if (!w_current->DONT_REDRAW) {
	  o_cue_undraw_list(w_current, connected_objects);
	  o_cue_draw_list(w_current, connected_objects);
	}
        break;
        
      case(OBJ_LINE):
	if (!w_current->DONT_REDRAW) {
	  o_line_erase_grips(w_current, object);
	  o_line_erase(w_current, object);
	}

        o_line_rotate(w_current, centerx, centery, 
                      90, object);

	if (!w_current->DONT_REDRAW) {
	  o_line_draw(w_current, object);
	}
        break;

      case(OBJ_BOX):
	/* erase the current selection */
	if (!w_current->DONT_REDRAW) {
	  o_box_erase_grips(w_current, object);
	  o_box_erase(w_current, object);
	}

        o_box_rotate(w_current, centerx, centery, 
                     90, object);

	if (!w_current->DONT_REDRAW) {
	  o_box_draw(w_current, object);
	}
        break;

      case(OBJ_PICTURE):
				/* erase the current selection */
	
	if (!w_current->DONT_REDRAW) {
	  o_picture_erase_grips(w_current, object);
	  o_picture_erase(w_current, object);
	}
	
        o_picture_rotate(w_current, centerx, centery, 
                     90, object);

	if (!w_current->DONT_REDRAW) {
	  o_picture_draw(w_current, object);
	}
        break;

      case(OBJ_CIRCLE):
	if (!w_current->DONT_REDRAW) {
	  o_circle_erase_grips(w_current, object);
	  o_circle_erase(w_current, object);
	}

        o_circle_rotate(w_current, centerx, centery, 
                        90, object);

	if (!w_current->DONT_REDRAW) {
	  o_circle_draw(w_current, object);
	}
        break;

      case(OBJ_ARC):
	if (!w_current->DONT_REDRAW) {
	  o_arc_erase(w_current, object);
	}

#if 0 /* not needed anymore */
	SCREENtoWORLD(w_current, centerx, centery, 
		      &world_centerx, &world_centery);
        o_arc_rotate_world(w_current, world_centerx, world_centery, 90, object);
#endif

        o_arc_rotate(w_current, centerx, centery, 90, object);
	if (!w_current->DONT_REDRAW) {
	  o_arc_draw(w_current, object);
	}
        break;

      case(OBJ_TEXT):
	/* erase the current selection */
	if (!w_current->DONT_REDRAW) {
	  o_text_erase(w_current, object);
	}

        new_angle = (object->text->angle + 90) % 360;
        o_text_rotate(w_current, centerx, centery,
                      new_angle, 90, object);

	if (!w_current->DONT_REDRAW) {
	  o_text_draw(w_current, object);
	}
        break;
    }
    s_current = s_current->next;
  }

  /* All objects were rotated. Do a 2nd pass to run the rotate hooks */
  /* Do not run any hooks for simple objects here, like text, since they
     were rotated in the previous pass, and the selection list can contain
     an object and all its attributes (text) */
  s_current = list;
  while (s_current != NULL) {
    object = (OBJECT *) s_current->data;

    if (!object) {
      fprintf(stderr, _("ERROR: NULL object in o_rotate_90!\n"));
      return;
    }

    switch(object->type) {
      case(OBJ_PIN):
	/* Run the rotate pin hook */
	if (scm_hook_empty_p(rotate_pin_hook) == SCM_BOOL_F &&
	    object != NULL) {
	  scm_run_hook(rotate_pin_hook,
		       scm_cons(g_make_object_smob(w_current, object),
				SCM_EOL));
	}
	break;

      case (OBJ_COMPLEX):
	/* Run the rotate hook */
	if (scm_hook_empty_p(rotate_component_object_hook) == SCM_BOOL_F &&
	    object != NULL) {
	  scm_run_hook(rotate_component_object_hook,
		       scm_cons(g_make_object_smob(w_current, object),
				SCM_EOL));
	}
	break;
    default:
	break;
    }

    s_current = s_current->next;
  }

  /* Don't save the undo state if we are inside an action */
  /* This is useful when rotating the selection while moving, for example */
  w_current->page_current->CHANGED = 1;
  if (!w_current->inside_action) {
    o_undo_savestate(w_current, UNDO_ALL);
  }
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 * 
 */
void o_embed(TOPLEVEL *w_current, OBJECT *o_current)
{
  gchar *new_basename;

  /* check o_current is a complex and is not already embedded */
  if (o_current->type == OBJ_COMPLEX &&
      !o_complex_is_embedded (o_current))
  {
    /* change the clib of complex to "EMBEDDED" */
    if (o_current->complex_clib) {
      g_free (o_current->complex_clib);
    }
    o_current->complex_clib = g_strdup ("EMBEDDED");

    /* change the basename to "EMBEDDED"+basename */
    new_basename = g_strconcat ("EMBEDDED",
                                o_current->complex_basename,
                                NULL);
    g_free (o_current->complex_basename);
    o_current->complex_basename = new_basename;

    s_log_message (_("Component [%s] has been embedded\n"),
                   o_current->complex_basename + 8);
    
    /* page content has been modified */
    w_current->page_current->CHANGED = 1;
  }

  /* If it's a picture and it's not embedded */
  if ( (o_current->type == OBJ_PICTURE) &&
       (o_current->picture->embedded == 0) ) {

    o_current->picture->embedded = 1;
    
    s_log_message (_("Picture [%s] has been embedded\n"),
		   basename(o_current->picture->filename));
    
    
    /* page content has been modified */
    w_current->page_current->CHANGED = 1;
  }
  
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 * 
 */
void o_unembed(TOPLEVEL *w_current, OBJECT *o_current)
{
  gchar *new_basename, *new_clib;
  const GSList *clibs;
  
  /* check o_current is an embedded complex */
  if (o_current->type == OBJ_COMPLEX &&
      o_complex_is_embedded (o_current))
  {
    /* get ride of the EMBEDDED word in basename */
    new_basename = g_strdup (o_current->complex_basename + 8);
    
    /* search for the symbol in the library */
    clibs = s_clib_search_basename (new_basename);

    if (!clibs) {
      /* symbol not found in the symbol library: signal an error */
      s_log_message (_("Could not find component [%s], while trying to unembed. Component is still embedded\n"),
                     o_current->complex_basename + 8);
      
    } else {
      /* set the object new basename */
      g_free (o_current->complex_basename);
      o_current->complex_basename = new_basename;

      /* set the object new clib */
      g_free (o_current->complex_clib);
      if (g_slist_next (clibs)) {
        s_log_message (_("More than one component found with name [%s]\n"),
                       new_basename);
        /* PB: for now, use the first directory in clibs */
        /* PB: maybe open a dialog to select the right one? */
      }
      new_clib = g_strdup ((gchar*)clibs->data);
      o_current->complex_clib = new_clib;

      s_log_message (_("Component [%s] has been successfully unembedded\n"),
                     o_current->complex_basename);
      
      /* page content has been modified */
      w_current->page_current->CHANGED = 1;
      
    }
  }

  /* If it's a picture and it's embedded */
  if ( (o_current->type == OBJ_PICTURE) &&
       (o_current->picture->embedded == 1) ) {

    o_current->picture->embedded = 0;
    
    s_log_message (_("Picture [%s] has been unembedded\n"),
		   basename(o_current->picture->filename));
    
    
    /* page content has been modified */
    w_current->page_current->CHANGED = 1;
  }
  
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 * 
 */
void o_mirror(TOPLEVEL *w_current, GList *list, int centerx, int centery)
{
  OBJECT *object;
  GList *s_current;
  OBJECT *o_current = NULL;
  GList *other_objects=NULL;
  GList *connected_objects=NULL;

  if (list == NULL) {
    w_current->inside_action = 0;
    i_set_state(w_current, SELECT);
    return;
  }

  s_current = list;

  while (s_current != NULL) {

    object = (OBJECT *) s_current->data;

    if (!object) {
      fprintf(stderr, _("ERROR: NULL object in o_mirror!\n"));
      return;
    }
    
    g_list_free(other_objects);
    other_objects = NULL;
    g_list_free(connected_objects);
    connected_objects = NULL;

    switch(object->type) {


      case(OBJ_NET):
        o_cue_undraw(w_current, object);
        o_net_erase(w_current, object);
        o_line_erase_grips(w_current, object);
        
        other_objects = s_conn_return_others(other_objects, object);
        s_conn_remove(w_current, object);

        o_net_mirror(w_current, centerx, centery, object);
        s_conn_update_object(w_current, object);
        o_net_draw(w_current, object);
        
        /* draw the other objects */
        o_cue_undraw_list(w_current, other_objects);
        o_cue_draw_list(w_current, other_objects);

        /* get other connected objects and redraw */
        connected_objects = s_conn_return_others(connected_objects, object);
        o_cue_undraw_list(w_current, connected_objects);
        o_cue_draw_list(w_current, connected_objects);

        /* finally redraw the cues on the current object */
        o_cue_draw_single(w_current, object); 
        break;

      case(OBJ_PIN):
        o_cue_undraw(w_current, object);
        o_pin_erase(w_current, object);
        o_line_erase_grips(w_current, object);
        
        other_objects = s_conn_return_others(other_objects, object);
        s_conn_remove(w_current, object);

        o_pin_mirror(w_current, centerx, centery, object);
        s_conn_update_object(w_current, object);
        o_pin_draw(w_current, object);

        /* draw the other objects */
        o_cue_undraw_list(w_current, other_objects);
        o_cue_draw_list(w_current, other_objects);

        /* get other connected objects and redraw */
        connected_objects = s_conn_return_others(connected_objects, object);
        o_cue_undraw_list(w_current, connected_objects);
        o_cue_draw_list(w_current, connected_objects);

        /* finally redraw the cues on the current object */
        o_cue_draw_single(w_current, object); 
        break;

      case(OBJ_BUS):
        o_bus_erase(w_current, object);
        o_line_erase_grips(w_current, object);

        other_objects = s_conn_return_others(other_objects, object);
        s_conn_remove(w_current, object);
        
        o_bus_mirror(w_current, centerx, centery, object);
        s_conn_update_object(w_current, object);
        o_bus_draw(w_current, object);
        
        /* draw the other objects */
        o_cue_undraw_list(w_current, other_objects);
        o_cue_draw_list(w_current, other_objects);

        /* get other connected objects and redraw */
        connected_objects = s_conn_return_others(connected_objects, object);
        o_cue_undraw_list(w_current, connected_objects);
        o_cue_draw_list(w_current, connected_objects);

        /* finally redraw the cues on the current object */
        o_cue_draw_single(w_current, object); 
        break;
        
      case(OBJ_COMPLEX):
        o_cue_undraw_objects(w_current, object->complex->prim_objs);
        /* erase the current selection */
        o_complex_erase(w_current, object);

        other_objects = s_conn_return_complex_others(other_objects, object);
        
        /* remove all conn references */
        o_current = object->complex->prim_objs;
        while(o_current != NULL) {
          s_conn_remove(w_current, o_current);
          o_current = o_current->next;
        }
      
        o_complex_mirror(w_current, centerx, centery, object);
        s_conn_update_complex(w_current, object->complex->prim_objs);
        o_complex_draw(w_current, object);

        o_cue_undraw_list(w_current, other_objects);
        o_cue_draw_list(w_current, other_objects);

        /* now draw the newly connected objects */
        connected_objects = s_conn_return_complex_others(connected_objects,
                                                         object);
        o_cue_undraw_list(w_current, connected_objects);
        o_cue_draw_list(w_current, connected_objects);
        break;

      case(OBJ_LINE):
        o_line_erase_grips(w_current, object);
        o_line_erase(w_current, object);
        o_line_mirror(w_current,
                      centerx, centery, object);
        o_line_draw(w_current, object);
        break;

      case(OBJ_BOX):
        o_box_erase_grips(w_current, object);
        o_box_erase(w_current, object);
        o_box_mirror(w_current,
                     centerx, centery, object);
        o_box_draw(w_current, object);
        break;

      case(OBJ_PICTURE):
        o_picture_erase_grips(w_current, object);
        o_picture_erase(w_current, object);
        o_picture_mirror(w_current,
			 centerx, centery, object);
        o_picture_draw(w_current, object);
        break;

      case(OBJ_CIRCLE):
        o_circle_erase_grips(w_current, object);
        o_circle_erase(w_current, object);
        o_circle_mirror(w_current,
                        centerx, centery, object);
        o_circle_draw(w_current, object);
        break;

      case(OBJ_ARC):
        o_arc_erase(w_current, object);
#if 0 /* not needed anymore */
	SCREENtoWORLD(w_current, centerx, centery, 
		      &world_centerx, &world_centery);
#endif
        o_arc_mirror(w_current, centerx, centery, object);
        o_arc_draw(w_current, object);
        break;

      case(OBJ_TEXT):
        o_text_erase(w_current, object);
        o_text_mirror(w_current,
                      centerx, centery, object);
        o_text_draw(w_current, object);
        break;

    }

    s_current = s_current->next;

  }

  /* All objects were rotated. Do a 2nd pass to run the rotate hooks */
  /* Do not run any hooks for simple objects here, like text, since they
     were rotated in the previous pass, and the selection list can contain
     an object and all its attributes (text) */
  s_current = list;
  while (s_current != NULL) {
    object = (OBJECT *) s_current->data;

    if (!object) {
      fprintf(stderr, _("ERROR: NULL object in o_rotate_90!\n"));
      return;
    }

    switch(object->type) {
      case(OBJ_PIN):
	/* Run the rotate pin hook */
	if (scm_hook_empty_p(mirror_pin_hook) == SCM_BOOL_F &&
	    object != NULL) {
	  scm_run_hook(rotate_pin_hook,
		       scm_cons(g_make_object_smob(w_current, object),
				SCM_EOL));
	}
	break;

      case (OBJ_COMPLEX):
	/* Run the rotate pin hook */
	if (scm_hook_empty_p(rotate_component_object_hook) == SCM_BOOL_F &&
	    object != NULL) {
	  scm_run_hook(mirror_component_object_hook,
		       scm_cons(g_make_object_smob(w_current, object),
				SCM_EOL));
	}
	break;
    default:
	break;
    }

    s_current = s_current->next;
  }


  w_current->page_current->CHANGED=1;
  o_undo_savestate(w_current, UNDO_ALL);
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 * 
 */
void o_edit_show_hidden_lowlevel(TOPLEVEL *w_current, OBJECT *o_list)
{
  OBJECT *o_current = o_list;

  if (o_current == NULL) {
    return;
  }

  while(o_current != NULL) {
    if (o_current->type == OBJ_TEXT && o_current->visibility == INVISIBLE) {

      /* don't toggle the visibility flag */

      if (w_current->show_hidden_text) {
        /* draw the text object if it hidden  */
        if (o_current->text->prim_objs == NULL) {
          o_text_recreate(w_current, o_current);
        }
        o_text_recalc(w_current, o_current);
        o_text_draw(w_current, o_current);
      } else {
        /* object is hidden and we are now NOT drawing it, so */
        /* get rid of the extra primitive data */
        o_text_recreate(w_current, o_current);
        o_text_recalc(w_current, o_current);
        /* unfortunately, you cannot erase the old visible text here */
        /* because o_text_draw will just return */
      }    
    }

    if (o_current->type == OBJ_COMPLEX || o_current->type == OBJ_PLACEHOLDER) {
      o_edit_show_hidden_lowlevel(w_current, o_current->complex->prim_objs);
      o_complex_recalc(w_current, o_current);
    }
   
    o_current = o_current->next;
  }
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 * 
 */
void o_edit_show_hidden(TOPLEVEL *w_current, OBJECT *o_list)
{
  /* this function just shows the hidden text, but doesn't toggle it */
  /* this function does not change the CHANGED bit, no real changes are */
  /* made to the schematic */
  
  /* toggle show_hidden_text variable, which when it is true */
  /* means that hidden text IS drawn */
  w_current->show_hidden_text = !w_current->show_hidden_text;
  i_show_state(w_current, NULL); /* update screen status */

  o_edit_show_hidden_lowlevel(w_current, o_list);
  o_redraw_all_fast(w_current);

  if (w_current->show_hidden_text) {
    s_log_message(_("Hidden text is now visible\n"));
  } else {
    s_log_message(_("Hidden text is now invisible\n"));
  }
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 * 
 */
void o_edit_make_visible(TOPLEVEL *w_current, OBJECT *o_list)
{
  /* this function actually changes the visibility flag */
  OBJECT *o_current = NULL;

  if (o_list == NULL)
  return;
  o_current = o_list;

  while(o_current != NULL) {

    if (o_current->type == OBJ_TEXT) {
      if (o_current->visibility == INVISIBLE) {
        o_current->visibility = VISIBLE;

        if (o_current->text->prim_objs == NULL) {
          o_text_recreate(w_current, o_current);
        }

        o_text_draw(w_current, o_current);

        w_current->page_current->CHANGED = 1; 
      }
    }
    o_current = o_current->next;
  }
  o_undo_savestate(w_current, UNDO_ALL);

}

#define FIND_WINDOW_HALF_SIZE (5000)

OBJECT *last_o = NULL;
int skiplast;

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 * 
 */
int o_edit_find_text(TOPLEVEL * w_current, OBJECT * o_list, char *stext,
		     int descend, int skip)
{

  char *attrib = NULL;
  int count = 0;
  PAGE *parent = NULL;
  char *current_filename = NULL;
  int page_control = 0;
  int pcount = 0;
  int rv;
  int text_screen_height;

  OBJECT *o_current = NULL;

  skiplast = skip;
  o_current = o_list;

  if (o_current == NULL) {
    return 1;
  }

  while (o_current != NULL) {

    if (descend) {
      if (o_current->type == OBJ_COMPLEX) {
	parent = w_current->page_current;
	attrib = o_attrib_search_name_single_count(o_current,
						   "source", count);

	/* if above is null, then look inside symbol */
	if (attrib == NULL) {
	  attrib = o_attrib_search_name(o_current->
					complex->
					prim_objs, "source", count);
	  /*          looking_inside = TRUE; */
	}

	if (attrib) {
	  pcount = 0;
	  current_filename = u_basic_breakup_string(attrib, ',', pcount);
	  if (current_filename != NULL) {
	    page_control =
		s_hierarchy_down_schematic_single(w_current,
						  current_filename,
						  parent,
						  page_control,
						  HIERARCHY_NORMAL_LOAD);
	    /* o_redraw_all(w_current); */

	    rv = o_edit_find_text(w_current,
				  w_current->page_current->object_head,
				  stext, descend, skiplast);
	    if (!rv) {
	      return 0;
	    }
	    s_hierarchy_up(w_current, w_current->page_current->up);
	  }
	}
      }
    }

    if (o_current->type == OBJ_TEXT) {
     /* replaced strcmp with strstr to simplify the search */
      if (strstr(o_current->text->string,stext)) {
	/*            printf(_("Found %s\n"), stext);
	   if (!o_current->selected&&(!descend)) {
	   w_current->page_current->selection_list = 
	     o_selection_add(w_current->page_current->selection_list,
	                     o_current);
	   } */
	if (!skiplast) {

#if 0 /* replaced by code below by avh, might not quite be right though */
	  set_window(w_current, o_current->text->x - FIND_WINDOW_HALF_SIZE,
		     o_current->text->x + FIND_WINDOW_HALF_SIZE,
		     o_current->text->y - FIND_WINDOW_HALF_SIZE,
		     o_current->text->y + FIND_WINDOW_HALF_SIZE);
          correct_aspect(w_current);

#endif
          a_zoom(w_current, ZOOM_FULL, DONTCARE, A_PAN_DONT_REDRAW);
          text_screen_height =  SCREENabs(w_current,
                                          o_text_height(o_current->
							text->string,
                                                        o_current->
                                                        text->size));
          /* this code will zoom/pan till the text screen height is about */
          /* 50 pixels high, perhaps a future enhancement will be to make */
          /* this number configurable */
          while (text_screen_height < 50) {
            a_zoom(w_current, ZOOM_IN, DONTCARE, A_PAN_DONT_REDRAW);
            text_screen_height =  SCREENabs(w_current,
                                            o_text_height(o_current->
							  text->string,
                                                          o_current->
                                                          text->size));
          }
	  a_pan_general(w_current, o_current->text->x, o_current->text->y, 
			1, 0);

	  last_o = o_current;
	  break;
	}
	if (last_o == o_current) {
	  skiplast = 0;
	}

      }
    }
    o_current = o_current->next;

    if (o_current == NULL) {
      return 1;
    }
  }
  return (o_current == NULL);
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 * 
 */
int autonumber_sort_numbers(gconstpointer a, gconstpointer b) {
  if (GPOINTER_TO_INT(a) < GPOINTER_TO_INT(b))
    return -1;
  if (GPOINTER_TO_INT(a) > GPOINTER_TO_INT(b))
    return 1;
  return 0;
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 * 
 */
int autonumber_sort_xy(gconstpointer a, gconstpointer b) {
  OBJECT *aa, *bb;
  aa=(OBJECT *) a;  bb=(OBJECT *) b;
  if (aa->text->x < bb->text->x)
    return -1;
  if (aa->text->x > bb->text->x)
    return 1;
  /* x == x */
  if (aa->text->y > bb->text->y)
    return -1;
  if (aa->text->y < bb->text->y)
    return 1;
  return 0;
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 * 
 */
int autonumber_sort_yx(gconstpointer a, gconstpointer b) {
  OBJECT *aa, *bb;
  aa=(OBJECT *) a;  bb=(OBJECT *) b;
  if (aa->text->y > bb->text->y)
    return -1;
  if (aa->text->y < bb->text->y)
    return 1;
  /* x == x */
  if (aa->text->x < bb->text->x)
    return -1;
  if (aa->text->x > bb->text->x)
    return 1;
  return 0;
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 * 
 */
int autonumber_sort_diagonal(gconstpointer a, gconstpointer b) {
  OBJECT *aa, *bb;
  aa=(OBJECT *) a;  bb=(OBJECT *) b;
  if (aa->text->x - aa->text->y < bb->text->x - bb->text->y)
    return -1;
  if (aa->text->x - aa->text->y > bb->text->x - bb->text->y)
    return 1;
  return 0;
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 * 
 */
void autonumber_find(TOPLEVEL * w_current, OBJECT * o_list, char *stext, 
		     GSList **used_numbers, int skip)
{
  OBJECT *o_current = NULL;
  char *attrib = NULL;
  int count = 0;
  PAGE *parent = NULL;
  char *current_filename = NULL;
  int page_control = 0;
  int pcount = 0;
  int l, m;

  for (o_current = o_list; o_current != NULL; o_current=o_current->next) {
    if (o_current->type == OBJ_TEXT && skip != AUTONUMBER_SKIP_PAGE) {
      if (!(o_current->selected) || skip != AUTONUMBER_SKIP_SELECTION) {
	if ((l = strlen(o_current->text->string) - strlen(stext)) > 0) {
	  if (!strncmp(stext, o_current->text->string, strlen(stext))) {
	    if (isdigit((int) o_current->text->string[strlen(stext)])) {
	      sscanf(o_current->text->string + strlen(stext), "%d", &m);
	      *used_numbers = g_slist_append(*used_numbers,GINT_TO_POINTER(m));
	      /* printf(" %d\n", m);*/
	    }
	  }
	}
      }
    }
    if (o_current->type == OBJ_COMPLEX) {
      parent = w_current->page_current;
      attrib = o_attrib_search_name_single_count(o_current,
						 "source", count);

      /* if above is null, then look inside symbol */
      if (attrib == NULL) {
	attrib = o_attrib_search_name(o_current->
				      complex->prim_objs, "source", count);
	/* looking_inside = TRUE; */
      }

      if (attrib) {
	pcount = 0;
	current_filename = u_basic_breakup_string(attrib, ',', pcount);
	if (current_filename != NULL) {
	  /* printf("autonumber_find: recursion filename \"%s\"\n",
	     current_filename); */
	  page_control =
	    s_hierarchy_down_schematic_single(w_current,
					      current_filename,
					      parent,
					      page_control,
					      HIERARCHY_NORMAL_LOAD);
	  if (page_control != -1) {
	    autonumber_find(w_current, w_current->page_current->object_head,
			    stext, used_numbers, AUTONUMBER_SKIP_NOT);
	    s_hierarchy_up(w_current, w_current->page_current->up);
	  }
	  else {
	    s_log_message(_("Cannot find source [%s]\n"),
			  current_filename);
	    fprintf(stderr,
		    _("Cannot find source [%s]\n"), current_filename);
	  }
	}
      } 
    }
  }
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 * 
 */
void autonumber_text(TOPLEVEL * w_current, char *stext, GSList **used, 
		     int *currentnumber, int unnumbered, int searchfocus, 
		     int sortorder)
{
  GSList *o_list = NULL;
  OBJECT *o_current = NULL;
  GSList *o_list_complex = NULL;
  OBJECT *o_complex_current = NULL;
  GSList *item;

  char *attrib = NULL;
  int count = 0;
  PAGE *parent = NULL;
  char *current_filename = NULL;
  int page_control = 0;
  int save_page_control = 0;
  int pcount = 0;

  char *ss;
  int l;

  o_current = w_current->page_current->object_head;

  for (o_current = w_current->page_current->object_head; o_current != NULL;
       o_current = o_current->next) {
    if (o_current->type == OBJ_COMPLEX) {
      o_list_complex = g_slist_append(o_list_complex, o_current);
    }
    if (o_current->type == OBJ_TEXT) {
      if (searchfocus != FOCUS_SELECTION || o_current->selected) {
	if ((l = strlen(o_current->text->string) - strlen(stext)) > 0) {
	  if (!strncmp(stext, o_current->text->string, strlen(stext))) {
	    if (o_current->text->string[strlen(stext)] == '?'
		|| (isdigit((int) o_current->text->string[strlen(stext)])
		    && unnumbered == SELECTION_ALL)){
	      o_list=g_slist_append(o_list,o_current);
	      /* printf("autonumber_text: text \"%s\" \n", 
		 o_current->text->string);*/
	    }
	  }
	}
      }
    }
  }

  /* sort o_list */
  switch (sortorder) {
  case AUTONUMBER_SORT_YX:
    o_list=g_slist_sort(o_list,autonumber_sort_yx);
    break;
  case AUTONUMBER_SORT_XY:
    o_list=g_slist_sort(o_list,autonumber_sort_xy);
    break;
  case AUTONUMBER_SORT_DIAGONAL:
    o_list=g_slist_sort(o_list,autonumber_sort_diagonal);
    break;
  default:
    ; /* unsorted file order */
  }

  for (item=o_list; item != NULL; item=g_slist_next(item)) {
    o_current= item->data;
    g_free(o_current->text->string);

    while (*used != NULL && *currentnumber > GPOINTER_TO_INT((*used)->data)) {
      *used=(*used)->next;
    }

    while (*used != NULL && *currentnumber == GPOINTER_TO_INT((*used)->data)) {
      *used=(*used)->next;
      (*currentnumber)++;
      while (*used != NULL && *currentnumber > GPOINTER_TO_INT((*used)->data))
	*used=(*used)->next;
    }

    /* printf("autonumber_text: currentnumber %d\n",*currentnumber);*/

    ss = g_strdup_printf("%s%d", stext, (*currentnumber)++);
    /*            printf("%s\n", ss); */
    o_current->text->string =
      (char *) g_malloc(sizeof(char) * (strlen(ss) + 1));
    strcpy(o_current->text->string, ss);
    g_free(ss);
    o_text_erase(w_current, o_current);
    o_text_recreate(w_current, o_current);
    o_text_draw(w_current, o_current);
    w_current->page_current->CHANGED = 1;
  }

  /* only walk down the hierarchy if requested */
  if (searchfocus == FOCUS_ALL) {
    for (item=o_list_complex; item != NULL; item=g_slist_next(item)) {
      o_complex_current= item->data;
      parent = w_current->page_current;
      attrib = o_attrib_search_name_single_count(o_complex_current,
						 "source", count);

      /* if above is null, then look inside symbol */
      if (attrib == NULL) {
	attrib = o_attrib_search_name(o_complex_current->
				      complex->prim_objs, "source", count);
	/*            looking_inside = TRUE; */
      }

      if (attrib) {
	pcount = 0;
	current_filename = u_basic_breakup_string(attrib, ',', pcount);
	if (current_filename != NULL) {
	  save_page_control = page_control;
	  page_control =
	    s_hierarchy_down_schematic_single(w_current,
					      current_filename,
					      parent,
					      page_control,
					      HIERARCHY_NORMAL_LOAD);
	  if (page_control != -1) {
	    autonumber_text(w_current, stext, used, currentnumber, 
			    unnumbered, searchfocus, sortorder);
	    s_hierarchy_up(w_current, w_current->page_current->up);
	  }
	  else {
	    /* restore if we failed to load the page */
	    page_control = save_page_control;
	  }
	}
      }
    }
  }
  g_slist_free(o_list);
  g_slist_free(o_list_complex);
  o_undo_savestate(w_current, UNDO_ALL);
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 * 
 */
void o_edit_autonumber_text(TOPLEVEL * w_current, char * searchtext,
			    int startnumber,int searchfocus,
			    int unnumbered, int sortorder)
{
  GSList *used_numbers=NULL;
  GSList *searchtext_list=NULL;
  GSList *item;
  char* stext=NULL;
  int skip_numbering=AUTONUMBER_SKIP_NOT;
  GSList *used_iterator=NULL;  /* used in recursive autonumber_text */
  int currentnumber;           /* used in recursive autonumber_text */

  /* Step2: if searchtext has an asterisk at the end we have to find
     all matching searchtextes. 
     The function can take care of the selection options.
     The search function should return all possibles searchtexts that have
     its own number space.
     This requires something like:
       grep searchtext | sort | uniq
     in the object list down the hierarchy.

     Example:  "refdes=*" will match each text that starts with "refdes="
     and has a trailing "?" (and a trailing number if the "all"-option is set.
     The function should return a list of all possible prefixes:
     refdes=R, refdes=C in the same way the searchstring is entered.

     If there is only one search pattern, it becomes a single item
     in the searchtext list */

  if (searchtext[strlen(searchtext)-1] == '*') {
    fprintf(stderr,"o_edit_autonumber_text: multiple searchtext option\n"
	    "  not implemented yet\n");
    /* collect the possible searchtextes, not implemented yet */
  }
  else {
    /* single searchtext */
    searchtext_list=g_slist_append (searchtext_list, searchtext);
  }

  /* Step3: iterate over the search item in the list */
  for (item=searchtext_list; item !=NULL; item=g_slist_next(item)) {
    stext = item->data;
    /* printf("o_edit_autonumber_text: iterate text: \"%s\"\n",stext);*/
    /* Step3.1: get the used numbers, ignore numbers that will be overwritten*/
    if (unnumbered==SELECTION_ALL && searchfocus==FOCUS_ALL) {
      ; /* all numbers will be overwritten, don't search */
    }
    else {
      if (unnumbered == SELECTION_ALL && searchfocus == FOCUS_SHEET)
	skip_numbering = AUTONUMBER_SKIP_PAGE;
      if (unnumbered == SELECTION_ALL && searchfocus == FOCUS_SELECTION)
	skip_numbering = AUTONUMBER_SKIP_SELECTION;

      autonumber_find(w_current, w_current->page_current->object_head, 
		   stext, &used_numbers, skip_numbering);
    }

    /* printf("o_edit_autonumber_text: listlength: %d\n",
       g_slist_length(used_numbers)); */
    /* sort used numbers */
    used_numbers=g_slist_sort(used_numbers,autonumber_sort_numbers);

  /* Step3.2: get a list of all matching objects 
              sort the list (sort option)
	      renumber all objects (start number option)
     this whole step is done in the autonumber_text function */
    used_iterator=used_numbers;
    currentnumber=startnumber;
    autonumber_text(w_current, stext, &used_iterator, &currentnumber, 
		    unnumbered, searchfocus, sortorder);
    g_slist_free(used_numbers);
    used_numbers=NULL;
  }
  g_slist_free(searchtext_list);
  o_redraw_all(w_current);
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 * 
 */
void o_edit_hide_specific_text(TOPLEVEL * w_current, OBJECT * o_list,
			       char *stext)
{
  OBJECT *o_current = NULL;

  if (o_list == NULL)
    return;

  o_current = o_list;

  while (o_current != NULL) {

    if (o_current->type == OBJ_TEXT) {
      if (!strncmp(stext, o_current->text->string, strlen(stext))) {
	if (o_current->visibility == VISIBLE) {
	  o_current->visibility = INVISIBLE;

	  if (o_current->text->prim_objs == NULL) {
	    o_text_recreate(w_current, o_current);
	  }
	  w_current->page_current->CHANGED = 1;
	}
      }
    }
    o_current = o_current->next;
  }
  o_undo_savestate(w_current, UNDO_ALL);
  o_redraw_all(w_current);
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 * 
 */
void o_edit_show_specific_text(TOPLEVEL * w_current, OBJECT * o_list,
			       char *stext)
{
  OBJECT *o_current = NULL;

  if (o_list == NULL)
    return;

  o_current = o_list;

  while (o_current != NULL) {

    if (o_current->type == OBJ_TEXT) {
      if (!strncmp(stext, o_current->text->string, strlen(stext))) {
	if (o_current->visibility == INVISIBLE) {
	  o_current->visibility = VISIBLE;

	  if (o_current->text->prim_objs == NULL) {
	    o_text_recreate(w_current, o_current);
	  }
	  o_text_draw(w_current, o_current);
	  w_current->page_current->CHANGED = 1;
	}
      }
    }
    o_current = o_current->next;
  }
  o_undo_savestate(w_current, UNDO_ALL);
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 * 
 */
void o_update_component(TOPLEVEL *w_current, OBJECT *o_current)
{
  OBJECT *tmp_list, *new_complex;
  ATTRIB *new_attribs, *a_current;
  gboolean is_embedded;
  gchar *basename, *clib;

  is_embedded = o_complex_is_embedded (o_current);

  /* identify symbol name */
  if (is_embedded) {
    const GSList *clibs;
    
    /* skip over EMBEDDED word */
    basename = g_strdup (o_current->complex_basename + 8);

    /* search for basename in component library */
    clibs    = s_clib_search_basename (basename);
    if (!clibs) {
      s_log_message (_("Could not unembedded component, could not find appropriate .sym file\n"));
      s_log_message (_("Component still embedded and not updated\n"));
      return;
    }
    if (g_slist_next (clibs)) {
      s_log_message (_("More than one component found with name [%s]\n"),
                     basename);
      /* PB: for now, use the first directory in clibs */
      /* PB: maybe open a dialog to select the right one? */
    }
    clib = (gchar*)clibs->data;
  } else {
    basename = o_current->complex_basename;
    clib     = o_current->complex_clib;
  }

  /* erase the complex object */
  o_erase_single (w_current, o_current);
  /* delete its connections */
  s_conn_remove_complex (w_current, o_current);
  /* and unselect but keep it in the selection list */
  o_selection_unselect (o_current);

  /* build a temporary list and add a complex to this list */
  tmp_list = s_basic_init_object ("update component");
  new_complex = o_complex_add (w_current,
                               tmp_list, NULL,
                               OBJ_COMPLEX,
                               WHITE,
                               o_current->complex->x,
                               o_current->complex->y,
                               o_current->complex->angle,
                               o_current->complex->mirror,
                               clib, basename, 1, TRUE);

  /* updating the old complex with data from the new one */
  /* first process the prim_objs: */
  /*   - delete the prim_objs of the old component */
  s_delete_list_fromstart (w_current, 
                           o_current->complex->prim_objs);
  /*   - put the prim_objs of the new component in the old one */
  o_current->complex->prim_objs = new_complex->complex->prim_objs;
  /*   - reset the new complex prim_objs */
  new_complex->complex->prim_objs = NULL;

  /* then process the attributes: */
  new_attribs = new_complex->attribs;
  /*   - check each attrib of the new complex */
  a_current = new_attribs ? new_attribs->next : NULL;
  while (a_current != NULL) {
    OBJECT *o_attrib;
    gchar *name, *value;
    char *attrfound;
    g_assert (a_current->object->type == OBJ_TEXT);
    o_attrib_get_name_value (a_current->object->text->string,
                             &name, &value);

    attrfound = o_attrib_search_name_single(o_current, name, NULL); 
    if (attrfound == NULL) {
      /* attribute with same name not found in old component: */
      /* add new attribute to old component */

      /* make a copy of the attribute object */
      o_list_copy_to (w_current, o_current,
                      a_current->object, NORMAL_FLAG, &o_attrib);
      if (o_current->attribs == NULL) {
        /* object has no attribute list: create it */
        o_current->attribs = add_attrib_head(o_current);
      }
      /* add the attribute to old */
      o_attrib_add (w_current, o_current->attribs, o_attrib);
      /* redraw the attribute object */
      o_redraw_single (w_current, o_attrib);
      /* note: this object is unselected (not added to selection). */
    }
    else
    {
      g_free(attrfound);
    }


    a_current = a_current->next;
  }
    
  /* finally delete the temp list with the updated complex */
  s_delete_list_fromstart (w_current, tmp_list);
  /* reconnect, re-select and redraw */
  s_conn_update_complex (w_current, o_current->complex->prim_objs);
  o_selection_select (o_current, SELECT_COLOR);
  o_redraw_single (w_current, o_current);

  if (is_embedded) {
    /* we previously allocated memory for basename and clib */
    g_free (basename);
    g_free (clib);
  }
    
  /* mark the page as modified */
  w_current->page_current->CHANGED = 1;
  o_undo_savestate (w_current, UNDO_ALL);
    
}

/*! \brief Do autosave on all pages that are marked.
 *  \par Function Description
 *  Looks for pages with the do_autosave_backup flag activated and
 *  autosaves them.
 *
 *  \param [in] toplevel  The TOPLEVEL object to search for autosave's.
 */
void o_autosave_backups(TOPLEVEL *toplevel)
{
  PAGE *p_save, *p_current;
  gchar *backup_filename;
  gchar *real_filename;
  gchar *only_filename;
  gchar *dirname;
  mode_t saved_umask;
  mode_t mask;
  struct stat st;

  g_assert (toplevel->page_head != NULL &&
            toplevel->page_head->pid == -1);

  /* save current page */
  p_save = toplevel->page_current;
  
  for (p_current = toplevel->page_head->next;
       p_current != NULL;
       p_current = p_current->next) {

    if (p_current->do_autosave_backup == 0) {
      continue;
    }
    if (p_current->ops_since_last_backup != 0) {
      /* make p_current the current page of toplevel */
      s_page_goto (toplevel, p_current);
      
      /* Get the real filename and file permissions */
      real_filename = follow_symlinks (p_current->page_filename, NULL);
      
      if (real_filename == NULL) {
	s_log_message (_("o_autosave_backups: Can't get the real filename of %s."), p_current->page_filename);
	fprintf (stderr, "o_autosave_backups: Can't get the real filename of %s.\n", p_current->page_filename);
      }
      else {
	/* Get the directory in which the real filename lives */
	dirname = g_path_get_dirname (real_filename);
	only_filename = g_path_get_basename(real_filename);  
      

	backup_filename = g_strdup_printf("%s%c"AUTOSAVE_BACKUP_FILENAME_STRING,
					  dirname, G_DIR_SEPARATOR, only_filename);
	
	/* If there is not an existing file with that name, compute the
	 * permissions and uid/gid that we will use for the newly-created file.
	 */
	
	if (stat (real_filename, &st) != 0)
	  {
	    struct stat dir_st;
	    int result;
	    
	    /* Use default permissions */
	    saved_umask = umask(0);
	    st.st_mode = 0666 & ~saved_umask;
	    umask(saved_umask);
	    st.st_uid = getuid ();
	    
	    result = stat (dirname, &dir_st);
	    
	    if (result == 0 && (dir_st.st_mode & S_ISGID))
	      st.st_gid = dir_st.st_gid;
	    else
	      st.st_gid = getgid ();
	  }
	g_free (dirname);
	g_free (only_filename);
	g_free (real_filename);

	/* Make the backup file writable before saving a new one */
	if ( g_file_test (backup_filename, G_FILE_TEST_EXISTS) && 
	     (! g_file_test (backup_filename, G_FILE_TEST_IS_DIR))) {
	  saved_umask = umask(0);
	  if (chmod(backup_filename, (S_IWRITE|S_IWGRP|S_IWOTH) & 
		    ((~saved_umask) & 0777)) != 0) {
	    s_log_message (_("Could NOT set previous backup file [%s] read-write\n"), 
			   backup_filename);	    
	  }
	  umask(saved_umask);
	}
	
	if (o_save (toplevel, backup_filename)) {

	  p_current->ops_since_last_backup = 0;
          p_current->do_autosave_backup = 0;

	  /* Make the backup file readonly so a 'rm *' command will ask 
	     the user before deleting it */
	  saved_umask = umask(0);
	  mask = (S_IWRITE|S_IWGRP|S_IEXEC|S_IXGRP|S_IXOTH);
	  mask = (~mask)&0777;
	  mask &= ((~saved_umask) & 0777);
	  if (chmod(backup_filename,mask) != 0) {
	    s_log_message (_("Could NOT set backup file [%s] readonly\n"), 
			   backup_filename);	    
	  }
	  umask(saved_umask);
	} else {
	  s_log_message (_("Could NOT save backup file [%s]\n"), 
			 backup_filename);
	}
	g_free (backup_filename);
      }
    }
  }
  /* restore current page */
  s_page_goto (toplevel, p_save);
}
