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
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <libgeda/libgeda.h>
#include <gdk/gdkkeysyms.h>

#include "../include/i_vars.h"
#include "../include/globals.h"
#include "../include/prototype.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

/** @brief How many entries to keep in the "Search text" combo box. */
#define HISTORY_LENGTH		15

/* autonumber text structs and enums */
enum {
	AUTONUMBER_SORT_FILE, 
	AUTONUMBER_SORT_YX, 
	AUTONUMBER_SORT_YX_REV, 
      	AUTONUMBER_SORT_XY, 
	AUTONUMBER_SORT_XY_REV, 
	AUTONUMBER_SORT_DIAGONAL
};

enum {
	AUTONUMBER_IGNORE, 
	AUTONUMBER_RENUMBER, 
	AUTONUMBER_RESPECT
};

enum {
	SCOPE_SELECTED, 
	SCOPE_PAGE, 
	SCOPE_HIERARCHY 
};

typedef struct autonumber_text_t AUTONUMBER_TEXT;

/** @brief Stored state of the autonumber text dialog */
struct autonumber_text_t {
	/** @brief Search text history */
  	GList *scope_text;

	/** @brief Scope for searching existing numbers */
  	gint scope_skip;

	/** @brief Scope for autonumbering text */
	gint scope_number;

	/** @brief Overwrite existing numbers in scope */
	gboolean scope_overwrite;

	/** @brief Sort order */
  	gint order;

	/** @brief Starting number for automatic numbering */
	gint startnum;

	/** @brief Remove numbers instead of automatic numbering */
	gboolean removenum;

	/** @brief Automatic assignments of slots */
  	gboolean slotting;

  	/** @brief Pointer to the dialog */ 
  	GtkWidget *dialog;

	/** @brief Pointer to the toplevel struct */
	TOPLEVEL *toplevel;

  	/* variables used while autonumbering */
	gchar * current_searchtext;
	gint root_page;      /* flag whether its the root page or not */
	GList *used_numbers; /* list of used numbers */ 
	GList *free_slots;   /* list of FREE_SLOT objects */
};

typedef struct free_slot_t FREE_SLOT;

struct free_slot_t {
  gchar *symbolname;     /* or should I use the device name? (Werner) */
  gint number;           /* usually this is the refdes number */
  gint slotnr;      /* just the number of the free slot */
};


/********** compare functions for g_list_sort, ... ***********************/
/*! \brief GCompareFunc function to sort a list with g_list_sort().
 *  \par Function Description
 *  Compares the integer values of the gconstpointers a and b.
 *  \return -1 if a<b, 1 if a>b, 0 if a==b
 */
gint autonumber_sort_numbers(gconstpointer a, gconstpointer b) {
  if (GPOINTER_TO_INT(a) < GPOINTER_TO_INT(b))
    return -1;
  if (GPOINTER_TO_INT(a) > GPOINTER_TO_INT(b))
    return 1;
  return 0;
}

/*! \brief GCompareFunc function to sort text objects by there location 
 *  \par Function Description 
 *  This Funcion takes two <B>OBJECT*</B> arguments and compares the  
 *  location of the two text objects. The first sort criteria is the x location,
 *  the second sort criteria is the y location.
 *  The Function is used as GCompareFunc by g_list_sort().
 */
gint autonumber_sort_xy(gconstpointer a, gconstpointer b) {
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

/*! \brief GCompareFunc function to sort text objects by there location 
 *  \par Function Description
 *  This funcion takes two <B>OBJECT*</B> arguments and compares the  
 *  location of the two text objects. The first sort criteria is the x location,
 *  the second sort criteria is the y location.
 *  This function sorts the objects in reverse order.
 *  The function is used as GCompareFunc by g_list_sort().
 */
gint autonumber_sort_xy_rev(gconstpointer a, gconstpointer b) {
  OBJECT *aa, *bb;
  aa=(OBJECT *) a;  bb=(OBJECT *) b;
  if (aa->text->x < bb->text->x)
    return 1;
  if (aa->text->x > bb->text->x)
    return -1;
  /* x == x */
  if (aa->text->y > bb->text->y)
    return 1;
  if (aa->text->y < bb->text->y)
    return -1;
  return 0;
}

/*! \brief GCompareFunc function to sort text objects by there location 
 *  \par Function Description
 *  This funcion takes two <B>OBJECT*</B> arguments and compares the 
 *  location of the two text objects. The first sort criteria is the y location,
 *  the second sort criteria is the x location.
 *  The function is used as GCompareFunc by g_list_sort().
 */
int autonumber_sort_yx(gconstpointer a, gconstpointer b) {
  OBJECT *aa, *bb;
  aa=(OBJECT *) a;  bb=(OBJECT *) b;
  if (aa->text->y > bb->text->y)
    return -1;
  if (aa->text->y < bb->text->y)
    return 1;
  /* y == y */
  if (aa->text->x < bb->text->x)
    return -1;
  if (aa->text->x > bb->text->x)
    return 1;
  return 0;
}

/*! \brief GCompareFunc function to sort text objects by there location 
 *  \par Function Description
 *  This Funcion takes two <B>OBJECT*</B> arguments and compares the 
 *  location of the two text objects. The first sort criteria is the y location,
 *  the second sort criteria is the x location.
 *  This function sorts the objects in reverse order.
 *  The function is used as GCompareFunc by g_list_sort().
 */
int autonumber_sort_yx_rev(gconstpointer a, gconstpointer b) {
  OBJECT *aa, *bb;
  aa=(OBJECT *) a;  bb=(OBJECT *) b;
  if (aa->text->y > bb->text->y)
    return 1;
  if (aa->text->y < bb->text->y)
    return -1;
  /* y == y */
  if (aa->text->x < bb->text->x)
    return 1;
  if (aa->text->x > bb->text->x)
    return -1;
  return 0;
}

/*! \brief GCompareFunc function to sort text objects by there location 
 *  \par Function Description
 *  This Funcion takes two <B>OBJECT*</B> arguments and compares the 
 *  location of the two text objects. The sort criteria is the combined x- and the 
 *  y-location. The function sorts from top left to bottom right.
 *  The function is used as GCompareFunc by g_list_sort().
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

/*! \brief GCompareFunc function to acces <B>FREE_SLOT</B> object in a GList
 *  \par Function Description
 *  This Funcion takes two <B>FREE_SLOT*</B> arguments and compares them.
 *  Sorting criteria is are the FREE_SLOT members: first the symbolname, than the 
 *  number and last the slotnr.
 *  If the number or the slotnr is set to zero it acts as a wildcard. 
 *  The function is used as GCompareFunc by GList functions.
 */
gint freeslot_compare(gconstpointer a, gconstpointer b) 
{
  FREE_SLOT *aa, *bb;
  gint res;
  aa = (FREE_SLOT *) a;  bb = (FREE_SLOT *) b;
  
  if ((res = strcmp(aa->symbolname, bb->symbolname)) != 0)
    return res;

  /* aa->symbolname == bb->symbolname */
  if (aa->number == 0 || bb->number == 0)
    return 0;
  if (aa->number > bb->number)
    return 1;
  if (aa->number < bb->number)
    return -1;

  /* aa->number == bb->number */
  if (aa->slotnr == 0 || bb->slotnr == 0)
    return 0;
  if (aa->slotnr > bb->slotnr)
    return 1;
  if (aa->slotnr < bb->slotnr)
    return -1;

  return 0;
}

/*! \brief Prints a <B>GList</B> of <B>FREE_SLOT</B> elements
 *  \par Function Description
 *  This funcion prints the elements of a GList that contains <B>FREE_SLOT</B> elements
 *  It is only used for debugging purposes.
 */
void freeslot_print(GList *list) {
  GList *item;
  FREE_SLOT *fs;
  
  printf("freeslot_print(): symname, number, slot\n");
  for (item = list; item != NULL; item = g_list_next(item)) {
    fs = item ->data;
    printf("  %s, %d, %d\n",fs->symbolname, fs->number, fs->slotnr);
  }
}

/*! \brief Function to test, whether the OBJECT matches the autotext criterias
 *  \par Function Description
 *  The criterias are those of the autonumber text dialog. The function decides
 *  whether the <B>OBJECT</B> has to be renumberd, ignored or taken care of when
 *  renumbering all other objects.  
 *  \return one of these integer values: <B>AUTONUMBER_IGNORE</B>, 
 *  <B>AUTONUMBER_RESPECT</B> or <B>AUTONUMBER_RENUMBER</B> and the current number
 *  of the text object in <B>*number</B>.
 */
gint autonumber_match(AUTONUMBER_TEXT *autotext, OBJECT *o_current, gint *number)
{
  gint i, len, isnumbered=1; 

  
  len = strlen(autotext->current_searchtext);
  /* first find out whether we can ignore that object */
  if (o_current->type != OBJ_TEXT  /* text object */
      || !(strlen(o_current->text->string) - len > 0)
      || !g_str_has_prefix(o_current->text->string, autotext->current_searchtext))
    return AUTONUMBER_IGNORE;

  /* the string object matches with its leading characters to the searchtext */
  /* now look for the extension, either a number or the "?" */
  if (g_str_has_suffix (o_current->text->string,"?")) {
    isnumbered = 0;
  }
  else {
    if (!isdigit(o_current->text->string[len])) /* has at least one digit */
      return AUTONUMBER_IGNORE;
    
    for (i=len+1; o_current->text->string[i]; i++) /* and only digits */
      if (!isdigit(o_current->text->string[i]))
	return AUTONUMBER_IGNORE;
  }
  
  /* we have six cases, 3 from focus multiplied by 2 selection cases */
  if ((autotext->root_page || autotext->scope_number == SCOPE_HIERARCHY)
      && (o_current->selected 
	  || autotext->scope_number == SCOPE_HIERARCHY || autotext->scope_number == SCOPE_PAGE)
      && (!isnumbered || (autotext->scope_overwrite)))
    return AUTONUMBER_RENUMBER;
  
  if (isnumbered) {
    sscanf(&(o_current->text->string[len])," %d", number);
    return AUTONUMBER_RESPECT; /* numbered objects which we don't renumber */
  }
  else
    return AUTONUMBER_IGNORE;  /* unnumbered objects outside the focus */
}


/*! \brief Creates a list of already numbered objects and slots
 *  \par Function Description
 *  This function collects the used numbers of a single schematic page.
 *  The used element numbers are stored in a GList container
 *  inside the <B>AUTONUMBER_TEXT</B> struct.
 *  The slotting container is a little bit different. It stores free slots of
 *  multislotted symbols, that were used only partially.
 *  The criterias are derivated from the autonumber dialog entries.
 */
void autonumber_get_used(TOPLEVEL *w_current, AUTONUMBER_TEXT *autotext)
{
  gint number, numslots, slot, i;
  OBJECT *o_current, *o_parent, *o_numslots;
  ATTRIB *a_current;
  FREE_SLOT *freeslot;
  GList *freeslot_item;
  char *numslot_str, *slot_str;
  
  for (o_current = w_current->page_current->object_head; o_current != NULL;
       o_current = o_current->next) {
    if (autonumber_match(autotext, o_current, &number) == AUTONUMBER_RESPECT) {
      /* check slot and maybe add it to the lists */
      if ((autotext->slotting)
      			&& (a_current = o_current->attached_to) != NULL) {
	o_parent = o_attrib_return_parent(a_current);
	/* check for slotted symbol */
	if ((numslot_str = o_attrib_search_numslots(o_parent, &o_numslots)) != NULL) {
	  sscanf(numslot_str," %d",&numslots);
	  //printf("autonumber_get_used(): numslots %d\n", numslots);
	  free(numslot_str);
	  if (numslots > 0) { 
	    slot_str=o_attrib_search_attrib_name(o_parent->attribs,"slot",0);
	    if (slot_str == NULL) {
	      s_log_message(_("slotted object without slot attribute may cause "
			      "problems when autonumbering slots\n"));
	    }
	    else {
	      sscanf(slot_str, " %d", &slot);
	      freeslot = g_new(FREE_SLOT,1);
	      freeslot->number = number;
	      freeslot->slotnr = slot;
	      freeslot->symbolname = o_parent->complex_basename;
	
	      freeslot_item = g_list_find_custom(autotext->free_slots,
						 freeslot,
						 (GCompareFunc) freeslot_compare);
	      if (freeslot_item == NULL) {
		/* insert all slots to the list, except of the current one */
		for (i=1; i <= numslots; i++) {
		  if (i != slot) {
		    freeslot->slotnr = i;
		    autotext->free_slots = g_list_insert_sorted(autotext->free_slots,
								freeslot,
								(GCompareFunc) freeslot_compare);
		    freeslot = g_memdup(freeslot, sizeof(FREE_SLOT));
		  }
		}
	      }
	      else {
		g_free(freeslot_item->data);
		autotext->free_slots = g_list_delete_link(autotext->free_slots,freeslot_item);
		g_free(freeslot);
	      }
	    }
	  }
	}
	//freeslot_print(autotext->free_slots);
      }
      /* put number into the used list */
      autotext->used_numbers = g_list_insert_sorted(autotext->used_numbers,
						    GINT_TO_POINTER(number),
						    (GCompareFunc) autonumber_sort_numbers);
      //printf("autonumber_get_used(): number=%d\n", number);
    }
  }
}


/*! \brief Gets or generates free numbers for the autonumbering process.
 *  \par Function Description
 *  This function gets or generates new numbers for the <B>OBJECT o_current</B>. 
 *  It uses the element numbers <B>used_numbers</B> and the list of the free slots
 *  <B>free_slots</B> of the <B>AUTONUMBER_TEXT</B> struct.
 *  \return 
 *  The new number is returned into the <B>number</B> parameter.
 *  <B>slot</B> is set if autoslotting is active, else it is set to zero.
 */
void autonumber_get_new_numbers(AUTONUMBER_TEXT *autotext, OBJECT *o_current, 
				gint *number, gint *slot)
{
  GList *item;
  gint new_number, numslots, i;
  FREE_SLOT *freeslot;
  OBJECT *o_parent = NULL, *o_numslots;
  ATTRIB *a_current = NULL;
  GList *freeslot_item;
  gchar *numslot_str;

  new_number = autotext->startnum;
  
  /* Check for slots first */
  /* 1. are there any unused slots in the database? */
  if ((autotext->slotting) && (a_current = o_current->attached_to) != NULL) {
    o_parent = o_attrib_return_parent(a_current);
    freeslot = g_new(FREE_SLOT,1);
    freeslot->symbolname = o_parent->complex_basename;
    freeslot->number = 0;
    freeslot->slotnr = 0;
    freeslot_item = g_list_find_custom(autotext->free_slots,
				       freeslot,
				       (GCompareFunc) freeslot_compare);
    g_free(freeslot);
    /* Yes! -> remove from database, apply it */
    if (freeslot_item != NULL) {
      freeslot = freeslot_item->data;
      *number = freeslot->number;
      *slot = freeslot->slotnr;
      g_free(freeslot);
      autotext->free_slots = g_list_delete_link(autotext->free_slots,freeslot_item);
      
      //freeslot_print(autotext->free_slots);
      return;
    }
  }

  /* get a new number */
  item = autotext->used_numbers; 
  while (1) {
    while (item != NULL && GPOINTER_TO_INT(item->data) < new_number)
      item = g_list_next(item);

    if (item == NULL || GPOINTER_TO_INT(item->data) > new_number)
      break;
    else  /* new_number == item->data */
      new_number++;
  }
  *number = new_number;
  *slot = 0;
  
  /* insert the new number to the used list */
  autotext->used_numbers = g_list_insert_sorted(autotext->used_numbers,
						GINT_TO_POINTER(new_number),
						(GCompareFunc) autonumber_sort_numbers);

  /* 3. is o_current a slotted object ? */
  if ((autotext->slotting) && o_parent != NULL) {
    if ((numslot_str = o_attrib_search_numslots(o_parent, &o_numslots)) != NULL) {
      sscanf(numslot_str," %d",&numslots);
      free(numslot_str);
      if (numslots > 0) { 
	/* Yes! -> new number and slot=1; add the other slots to the database */
	*slot = 1;
	for (i=2; i <=numslots; i++) {
	  freeslot = g_new(FREE_SLOT,1);
	  freeslot->symbolname = o_parent->complex_basename;
	  freeslot->number = new_number;
	  freeslot->slotnr = i;
	  autotext->free_slots = g_list_insert_sorted(autotext->free_slots,
						      freeslot,
						      (GCompareFunc) freeslot_compare);
	}
	//freeslot_print(autotext->free_slots);
      }
    }
  }
}

/** @brief Removes the number from the element.
 *
 *  This function updates the text content of the \a o_current object.
 *
 *  @param autotext Pointer to the state structure
 *  @param o_current Pointer to the object from which to remove the number
 *
 */
void autonumber_remove_number(AUTONUMBER_TEXT * autotext, OBJECT *o_current)
{
  /* replace old text */
  g_free(o_current->text->string);
  o_current->text->string = g_strdup_printf("%s?", 
					autotext->current_searchtext);

  /* redraw the text */
  o_text_erase(autotext->toplevel, o_current);
  o_text_recreate(autotext->toplevel, o_current);
  o_text_draw(autotext->toplevel, o_current);
  autotext->toplevel->page_current->CHANGED = 1;
}

/*! \brief Changes the number <B>OBJECT</B> element. Changes the slot attribute.
 *  \par Function Description
 *  This function updates the text content of the <B>o_current</B> object.
 *  If the <B>slot</B> value is not zero. It updates the slot attribut of the
 *  complex element that is also the parent object of the o_current element.
 */
void autonumber_apply_new_text(AUTONUMBER_TEXT * autotext, OBJECT *o_current,
			       gint number, gint slot)
{
  ATTRIB *a_current;
  OBJECT *o_parent, *o_slot;
  gchar *slot_str;

  /* add the slot as attribute to the object */
  if (slot != 0
      && (a_current = o_current->attached_to) != NULL) {
    o_parent = o_attrib_return_parent(a_current);
    slot_str = o_attrib_search_slot(o_parent, &o_slot);
    if (slot_str != NULL) {
      /* update the slot attribute */
      g_free(slot_str);
      g_free(o_slot->text->string);
      o_slot->text->string = g_strdup_printf("slot=%d",slot);
      o_text_erase(autotext->toplevel, o_slot);
      o_text_recreate(autotext->toplevel, o_slot);
      o_text_draw(autotext->toplevel, o_slot);
    }
    else {
      /* create a new attribute and attach it */
      /*! \bug FIXME :o_slot_end(), called from o_attrib_add_attrib, calls o_undo_savestate().
       *  this will flood the savestates and you have to undo every single slot addition. (Werner)
       */
      o_attrib_add_attrib(autotext->toplevel, 
			  g_strdup_printf("slot=%d",slot),
			  VISIBLE, SHOW_NAME_VALUE,
			  o_parent);
    }
    o_attrib_slot_update(autotext->toplevel, o_parent);
  }

  /* replace old text */
  g_free(o_current->text->string);
  o_current->text->string = g_strdup_printf("%s%d", autotext->current_searchtext,
					    number);
  /* redraw the text */
  o_text_erase(autotext->toplevel, o_current);
  o_text_recreate(autotext->toplevel, o_current);
  o_text_draw(autotext->toplevel, o_current);
  autotext->toplevel->page_current->CHANGED = 1;
}


/*! \brief Handles all the options of the autonumber text dialog
 *  \par Function Description
 *  This function is the master of all autonumber code. It receives the options of
 *  the the autonumber text dialog in an <B>AUTONUMBER_TEXT</B> structure.
 *  First it collects all pages of a hierarchical schematic.
 *  Second it gets all matching text elements for the searchtext.
 *  Then it renumbers all text elements of all schematic pages. The renumbering 
 *  follows the rules of the parameters given in the autonumber text dialog.
 */
void autonumber_text_autonumber(AUTONUMBER_TEXT *autotext)
{
  GList *pages;
  GList *searchtext_list=NULL;
  GList *text_item, *obj_item, *page_item;
  OBJECT *o_current;
  TOPLEVEL *w_current;
  gchar *searchtext;
  gchar *scope_text;
  gchar *new_searchtext;
  gint i, number, slot;
  GList *o_list = NULL;
  
  w_current = autotext->toplevel;
  autotext->current_searchtext = NULL;
  autotext->root_page = 1;
  autotext->used_numbers = NULL;
  autotext->free_slots = NULL;

  scope_text = g_list_first(autotext->scope_text)->data;

  /* Step1: get all pages of the hierarchy */
  pages = s_hierarchy_traversepages(w_current, HIERARCHY_NODUPS);

  //  g_list_foreach(pages, (GFunc) s_hierarchy_print_page, NULL);

  /* Step2: if searchtext has an asterisk at the end we have to find
     all matching searchtextes. 

     Example:  "refdes=*" will match each text that starts with "refdes="
     and has a trailing "?" or a trailing number if the "all"-option is set.
     We get a list of possible prefixes: refdes=R, refdes=C.

     If there is only one search pattern, it becomes a single item
     in the searchtext list */
  
  if (strlen(scope_text) == 0) {
    s_log_message(_("No searchstring given in autonumber text."));
    return; /* error */
  }
  else if (g_str_has_suffix(scope_text,"?") == TRUE) {
    /* single searchtext, strip of the "?" */
    searchtext = g_strndup(scope_text, strlen(scope_text)-1);
    searchtext_list=g_list_append (searchtext_list, searchtext);
  }
  else if (g_str_has_suffix(scope_text,"*") == TRUE) {
    /* strip of the "*" */
    searchtext = g_strndup(scope_text, strlen(scope_text)-1);
    /* collect all the possible searchtexts in all pages of the hierarchy */
    for (page_item = pages; page_item != NULL; page_item = g_list_next(page_item)) {
      s_page_goto(w_current, page_item->data); 
      /* iterate over all objects an look for matching searchtext's */
      for (o_current = w_current->page_current->object_head; o_current != NULL;
	   o_current = o_current->next) {
	if (o_current->type == OBJ_TEXT) {
	  if (autotext->scope_number == SCOPE_HIERARCHY
	      || autotext->scope_number == SCOPE_PAGE
	      || ((autotext->scope_number == SCOPE_SELECTED) && (o_current->selected))) {
	    if (g_str_has_prefix(o_current->text->string, searchtext) == TRUE) {
	      /* the beginnig of the current text matches with the searchtext now */
	      /* strip of the trailing [0-9?] chars and add it too the searchtext */
	      for (i = strlen(o_current->text->string)-1;
		   (i >= strlen(searchtext))
		     && (o_current->text->string[i] == '?'
			 || isdigit(o_current->text->string[i]));
		   i--)
		; /* void */
		
	      new_searchtext=g_strndup(o_current->text->string, i+1);
	      if (g_list_find_custom(searchtext_list, new_searchtext,
				     (GCompareFunc) strcmp) == NULL ) {
		searchtext_list = g_list_append(searchtext_list, new_searchtext);
		//printf("autonumber_text: text \"%s\", \"%s\"\n", 
		//       o_current->text->string, new_searchtext);
	      }
	      else {
		g_free(new_searchtext);
	      }
	    }
	  }
	}
      }
      if (autotext->scope_number == SCOPE_SELECTED || autotext->scope_number == SCOPE_PAGE)
	break; /* search only in the first page */
    }
    g_free(searchtext);
  }
  else {
    s_log_message(_("No '*' or '?' given at the end of the autonumber text.\n"));
    return;
  }

  /* Step3: iterate over the search items in the list */
  for (text_item=searchtext_list; text_item !=NULL; text_item=g_list_next(text_item)) {
    autotext->current_searchtext = text_item->data;
    //    printf("autonumber_text2: searchtext %s\n", autotext->current_searchtext);
    /* decide whether to renumber page by page or get a global used-list */
    if (autotext->scope_skip == SCOPE_HIERARCHY) {  /* whole hierarchy database */
      /* renumbering all means that no db is required */
      if (!(autotext->scope_number == SCOPE_HIERARCHY
	    && autotext->scope_overwrite)) {
	for (page_item = pages; page_item != NULL; page_item = g_list_next(page_item)) {
	  autotext->root_page = (pages->data == page_item->data);
	  s_page_goto(w_current, page_item->data);
	  autonumber_get_used(w_current, autotext);
	}
      }
    }
    
    /* renumber the elements */
    for (page_item = pages; page_item != NULL; page_item = g_list_next(page_item)) {
      s_page_goto(w_current, page_item->data);
      autotext->root_page = (pages->data == page_item->data);
      /* build a page database if we're numbering pagebypage */
      if (autotext->scope_skip == SCOPE_PAGE) {
	autonumber_get_used(w_current, autotext);
      }
      
      /* RENUMBER CODE FOR ONE PAGE AND ONE SEARCHTEXT*/
      /* 1. get objects to renumber */
      for (o_current = w_current->page_current->object_head; o_current != NULL;
	   o_current = o_current->next) {
	if (autonumber_match(autotext, o_current, &number) == AUTONUMBER_RENUMBER) {
	  /* put number into the used list */
	  o_list = g_list_append(o_list, o_current);
	}
      }

      /* 2. sort object list */
      switch (autotext->order) {
      case AUTONUMBER_SORT_YX:
	o_list=g_list_sort(o_list, autonumber_sort_yx);
	break;
      case AUTONUMBER_SORT_YX_REV:
	o_list=g_list_sort(o_list, autonumber_sort_yx_rev);
	break;
      case AUTONUMBER_SORT_XY:
	o_list=g_list_sort(o_list, autonumber_sort_xy);
	break;
      case AUTONUMBER_SORT_XY_REV:
	o_list=g_list_sort(o_list, autonumber_sort_xy_rev);
	break;
      case AUTONUMBER_SORT_DIAGONAL:
	o_list=g_list_sort(o_list, autonumber_sort_diagonal);
	break;
      default:
	; /* unsorted file order */
      }
	 
      /* 3. renumber/reslot the objects */

      for(obj_item=o_list; obj_item != NULL; obj_item=g_list_next(obj_item)) {
	o_current= obj_item->data;
      	if(autotext->removenum) {
	  autonumber_remove_number(autotext, o_current);		
	} else {
	  /* get valid numbers from the database */
	  autonumber_get_new_numbers(autotext, o_current, &number, &slot);
	  /* and apply it. TODO: join these two functions */
	  autonumber_apply_new_text(autotext, o_current, number, slot);
	}
      }
      g_list_free(o_list);
      o_list = NULL;

      /* destroy the page database */
      if (autotext->scope_skip == SCOPE_PAGE) {
	g_list_free(autotext->used_numbers);
	autotext->used_numbers = NULL;
	g_list_foreach(autotext->free_slots, (GFunc) g_free, NULL);
	g_list_free(autotext->free_slots);
	autotext->free_slots = NULL;
      }
      if (autotext->scope_number == SCOPE_SELECTED || autotext->scope_number == SCOPE_PAGE)
	break; /* only renumber the parent page (the first page) */
    }

    /* cleanup everything for the next searchtext */
    if (autotext->used_numbers != NULL) {
      g_list_free(autotext->used_numbers);
      autotext->used_numbers = NULL;
    }
    if (autotext->free_slots != NULL) {
      g_list_foreach(autotext->free_slots, (GFunc) g_free, NULL);
      g_list_free(autotext->free_slots);
      autotext->free_slots = NULL;
    }
  }

  /* cleanup and redraw all*/
  g_list_foreach(searchtext_list, (GFunc) g_free, NULL);
  g_list_free(searchtext_list);
  s_page_goto(w_current, pages->data); /* go back to the root page */
  o_redraw_all(w_current);
  g_list_free(pages);
  o_undo_savestate(w_current, UNDO_ALL);
}

/** @brief Finds a widget by its name given a pointer to its parent.
 *
 * @param widget Pointer to the parent widget.
 * @param widget_name Name of the widget.
 * @return Pointer to the widget or NULL if not found. */
GtkWidget* lookup_widget(GtkWidget *widget, const gchar *widget_name)
{
	GtkWidget *found_widget;

	found_widget = (GtkWidget*) g_object_get_data(G_OBJECT(widget), 
								widget_name);

	return found_widget;
}

/** @brief Get the settings from the autonumber text dialog
 *
 * Get the settings from the autonumber text dialog and store it in the
 * <B>AUTONUMBER_TEXT</B> structure.
 *
 * @param autotext Pointer to the state struct.
 */
void autonumber_get_state(AUTONUMBER_TEXT *autotext)
{
	GtkWidget *widget;
	gchar *text;

	/* Scope */

	/* Search text history */
  	widget = lookup_widget(autotext->dialog, "scope_text");
	text=gtk_combo_box_get_active_text( GTK_COMBO_BOX(widget) );

  	autotext->scope_text=g_list_prepend(autotext->scope_text, text);
	while(g_list_length(autotext->scope_text)>HISTORY_LENGTH) {
		GList *last = g_list_last(autotext->scope_text);

		autotext->scope_text = g_list_remove_link(
						autotext->scope_text,
						last);

		g_free(last->data);
		g_list_free(last);
	}

	widget = lookup_widget(autotext->dialog, "scope_skip");
	autotext->scope_skip = gtk_combo_box_get_active( 
						GTK_COMBO_BOX(widget) );

	widget = lookup_widget(autotext->dialog, "scope_number");
	autotext->scope_number = gtk_combo_box_get_active( 
						GTK_COMBO_BOX(widget) );

	widget = lookup_widget(autotext->dialog, "scope_overwrite");
	autotext->scope_overwrite = gtk_toggle_button_get_active( 
						GTK_TOGGLE_BUTTON(widget) );

	/* Sort order */
	autotext->order=-1;

	if(autotext->order==-1) {
		widget = lookup_widget(autotext->dialog, "order_file");
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
			autotext->order=AUTONUMBER_SORT_FILE;
		}
	}

	if(autotext->order==-1) {
		widget = lookup_widget(autotext->dialog, "order_left2right");
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
			autotext->order=AUTONUMBER_SORT_XY;
		}
	}

	if(autotext->order==-1) {
		widget = lookup_widget(autotext->dialog, "order_right2left");
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
			autotext->order=AUTONUMBER_SORT_XY_REV;
		}
	}

	if(autotext->order==-1) {
		widget = lookup_widget(autotext->dialog, "order_top2bottom");
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
			autotext->order=AUTONUMBER_SORT_YX;
		}
	}

	if(autotext->order==-1) {
		widget = lookup_widget(autotext->dialog, "order_bottom2top");
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
			autotext->order=AUTONUMBER_SORT_YX_REV;
		}
	}

	if(autotext->order==-1) {
		widget = lookup_widget(autotext->dialog, "order_diagonal");
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
			autotext->order=AUTONUMBER_SORT_DIAGONAL;
		}
	}

	/* Options */

	widget = lookup_widget(autotext->dialog, "opt_startnum");
	autotext->startnum=gtk_spin_button_get_value_as_int(
						GTK_SPIN_BUTTON(widget));

	widget = lookup_widget(autotext->dialog, "opt_removenum");
	autotext->removenum = gtk_toggle_button_get_active(
						GTK_TOGGLE_BUTTON(widget) );

	widget = lookup_widget(autotext->dialog, "opt_slotting");
	autotext->slotting = gtk_toggle_button_get_active(
						GTK_TOGGLE_BUTTON(widget) );
}

/** @brief Allocate and initialize the state structure
 *
 * @return Pointer to the allocated structure or NULL on error. 
 */
AUTONUMBER_TEXT *autonumber_init_state()
{
	AUTONUMBER_TEXT *autotext;

	/* Default contents of the combo box history */
  	gchar *default_text[] = {
				"refdes=*", 
				"refdes=C?", 
				"refdes=D?", 
				"refdes=I?",
				"refdes=L?", 
				"refdes=Q?", 
				"refdes=R?", 
				"refdes=T?",
				"refdes=U?", 
				"refdes=X?", 
				"netname=*", 
				"netname=A?",
				"netname=D?", 
				NULL
				};
	gchar **t;

	autotext = g_new(AUTONUMBER_TEXT, 1);

	if(autotext==NULL) return NULL;

	autotext->scope_text = NULL;
	t=default_text;
	while(*t!=NULL) {
		autotext->scope_text=g_list_append(autotext->scope_text, 
							g_strdup(*t));
		t++;
	}

	autotext->scope_skip = SCOPE_SELECTED;
	autotext->scope_number = SCOPE_SELECTED;

	autotext->scope_overwrite = 0;
	autotext->order = AUTONUMBER_SORT_FILE;

	autotext->startnum=1;

	autotext->removenum=0;
	autotext->slotting=0;

	autotext->dialog = NULL;

	return autotext;
}

/** @brief Restore the settings for the autonumber text dialog
 *
 * @param autotext Pointer to the state struct.
 */
void autonumber_set_state(AUTONUMBER_TEXT *autotext)
{
	GtkWidget *widget;

	/* Scope */

	/* Search text history */
  	widget = lookup_widget(autotext->dialog, "scope_text");

	GList *el = autotext->scope_text;

	while(el!=NULL) {
		gtk_combo_box_append_text(GTK_COMBO_BOX(widget), el->data);
		el=g_list_next(el);
	}

	widget = lookup_widget(autotext->dialog, "scope_skip");
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 
						autotext->scope_skip);

	widget = lookup_widget(autotext->dialog, "scope_number");
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 
						autotext->scope_number);

	widget = lookup_widget(autotext->dialog, "scope_overwrite");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), 
						autotext->scope_overwrite);

	/* Sort order */
	switch(autotext->order) {
		case AUTONUMBER_SORT_FILE:

		widget = lookup_widget(autotext->dialog, "order_file");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), 1);
		break;

		case AUTONUMBER_SORT_XY:

		widget = lookup_widget(autotext->dialog, "order_left2right");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), 1);
		break;

		case AUTONUMBER_SORT_XY_REV:

		widget = lookup_widget(autotext->dialog, "order_right2left");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), 1);
		break;

		case AUTONUMBER_SORT_YX:

		widget = lookup_widget(autotext->dialog, "order_top2bottom");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), 1);
		break;

		case AUTONUMBER_SORT_YX_REV:

		widget = lookup_widget(autotext->dialog, "order_bottom2top");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), 1);
		break;

		case AUTONUMBER_SORT_DIAGONAL:

		widget = lookup_widget(autotext->dialog, "order_diagonal");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), 1);
		break;
	}

	/* Options */

	widget = lookup_widget(autotext->dialog, "opt_startnum");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), 
							autotext->startnum);

	widget = lookup_widget(autotext->dialog, "opt_removenum");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), 
							autotext->removenum);

	widget = lookup_widget(autotext->dialog, "opt_slotting");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
							autotext->slotting);
}

/*! \brief OK Button callback of the autonumber text dialog
 *  \par Function Description
 *  This function applies the dialog settings to the schematics.
 */
void autonumber_text_ok(GtkWidget * w, AUTONUMBER_TEXT *autotext)
{
	GtkWidget *combobox;

  	autonumber_get_state(autotext);

  	autonumber_text_autonumber(autotext);

	combobox=lookup_widget(autotext->dialog, "scope_text");
	gtk_combo_box_prepend_text(GTK_COMBO_BOX(combobox), 
					autotext->scope_text->data);
}

/*! \brief Destroy callback function of the autonumber text dialog
 */
void autonumber_text_done(GtkWidget * w, AUTONUMBER_TEXT *autotext)
{
	/* The usual behaviour is that dialog contents are not stored if
	 * the user pressed "cancel" */

	/* autonumber_text_getdata(autotext); */

	autotext->dialog = NULL;
}

/** @brief Close button callback function of the autonumber text dialog
 *
 *  Just destroys the dialog. The triggered destroy event will save the 
 *  dialog contents.
 */
void autonumber_text_close(GtkWidget * w, AUTONUMBER_TEXT *autotext)
{
  gtk_widget_destroy(autotext->dialog);

  /* the settings are stored by autonumber_text_done, 
     called by the destroy event */
}

/** @brief Creates a new GtkImage displaying from an icon file.
 *
 * @param stock Name of the icon.
 * @return Pointer to the new GtkImage object.
 */
static GtkWidget *autonumber_create_pixmap(const char *file, 
							TOPLEVEL *w_current)
{
	GtkWidget *wpixmap;
	gchar *path;

	path=g_strconcat(w_current->bitmap_directory, 
			G_DIR_SEPARATOR_S, file, NULL);

	wpixmap = gtk_image_new_from_file (path);

	g_free(path);

	return wpixmap;
}

/** @brief Creates the autonumber text dialog.
 *
 * Dialog is not shown. No callbacks are registered. This is basically
 * unmodified code returned by Glade.
 *
 * Only modification was the following substitution:
 *
 * %s/create_pixmap (autonumber_text, "\(.*\)")/autonumber_create_pixmap("gschem-\1", w_current)/
 * 
 * and addition of the "w_current" parameter.
 *
 * @param w_current Pointer to the top level struct.
 * @return Pointer to the dialog window.
 */
GtkWidget* autonumber_create_dialog(TOPLEVEL *w_current)
{
  GtkWidget *autonumber_text;
  GtkWidget *vbox1;
  GtkWidget *frame1;
  GtkWidget *alignment1;
  GtkWidget *vbox3;
  GtkWidget *table1;
  GtkWidget *label4;
  GtkWidget *label6;
  GtkWidget *scope_text;
  GtkWidget *scope_skip;
  GtkWidget *label8;
  GtkWidget *scope_number;
  GtkWidget *scope_overwrite;
  GtkWidget *label1;
  GtkWidget *frame2;
  GtkWidget *alignment2;
  GtkWidget *table2;
  GtkWidget *order_file;
  GSList *order_file_group = NULL;
  GtkWidget *order_top2bottom;
  GtkWidget *order_left2right;
  GtkWidget *order_diagonal;
  GtkWidget *image2;
  GtkWidget *image3;
  GtkWidget *image1;
  GtkWidget *image4;
  GtkWidget *image5;
  GtkWidget *order_bottom2top;
  GtkWidget *order_right2left;
  GtkWidget *label2;
  GtkWidget *frame3;
  GtkWidget *alignment3;
  GtkWidget *vbox4;
  GtkWidget *hbox1;
  GtkWidget *label12;
  GtkObject *opt_startnum_adj;
  GtkWidget *opt_startnum;
  GtkWidget *opt_removenum;
  GtkWidget *opt_slotting;
  GtkWidget *label3;
  GtkWidget *hbuttonbox1;
  GtkWidget *button_cancel;
  GtkWidget *button_ok;
  GtkAccelGroup *accel_group;

  accel_group = gtk_accel_group_new ();

  autonumber_text = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (autonumber_text), "Autonumber text");
  gtk_window_set_resizable (GTK_WINDOW (autonumber_text), FALSE);
  // gtk_window_set_type_hint (GTK_WINDOW (autonumber_text), GDK_WINDOW_TYPE_HINT_DIALOG);

  vbox1 = gtk_vbox_new (FALSE, 24);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (autonumber_text), vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 12);

  frame1 = gtk_frame_new (NULL);
  gtk_widget_show (frame1);
  gtk_box_pack_start (GTK_BOX (vbox1), frame1, TRUE, TRUE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_NONE);

  alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment1);
  gtk_container_add (GTK_CONTAINER (frame1), alignment1);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment1), 0, 0, 24, 0);

  vbox3 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox3);
  gtk_container_add (GTK_CONTAINER (alignment1), vbox3);

  table1 = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (vbox3), table1, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 12);

  label4 = gtk_label_new_with_mnemonic ("Search _for:");
  gtk_widget_show (label4);
  gtk_table_attach (GTK_TABLE (table1), label4, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label4), 0, 0.5);

  label6 = gtk_label_new_with_mnemonic ("_Skip numbers found in:");
  gtk_widget_show (label6);
  gtk_table_attach (GTK_TABLE (table1), label6, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);

  scope_text = gtk_combo_box_entry_new_text ();
  gtk_widget_show (scope_text);
  gtk_table_attach (GTK_TABLE (table1), scope_text, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  scope_skip = gtk_combo_box_new_text ();
  gtk_widget_show (scope_skip);
  gtk_table_attach (GTK_TABLE (table1), scope_skip, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_widget_add_accelerator (scope_skip, "grab_focus", accel_group,
                              GDK_s, (GdkModifierType) 0,
                              GTK_ACCEL_VISIBLE);
  gtk_combo_box_append_text (GTK_COMBO_BOX (scope_skip), "Selected objects");
  gtk_combo_box_append_text (GTK_COMBO_BOX (scope_skip), "Current page");
  gtk_combo_box_append_text (GTK_COMBO_BOX (scope_skip), "Whole hierarchy");

  label8 = gtk_label_new_with_mnemonic ("_Autonumber text in:");
  gtk_widget_show (label8);
  gtk_table_attach (GTK_TABLE (table1), label8, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label8), 0, 0.5);

  scope_number = gtk_combo_box_new_text ();
  gtk_widget_show (scope_number);
  gtk_table_attach (GTK_TABLE (table1), scope_number, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_widget_add_accelerator (scope_number, "grab_focus", accel_group,
                              GDK_s, (GdkModifierType) 0,
                              GTK_ACCEL_VISIBLE);
  gtk_combo_box_append_text (GTK_COMBO_BOX (scope_number), "Selected objects");
  gtk_combo_box_append_text (GTK_COMBO_BOX (scope_number), "Current page");
  gtk_combo_box_append_text (GTK_COMBO_BOX (scope_number), "Whole hierarchy");

  scope_overwrite = gtk_check_button_new_with_mnemonic ("_Overwrite existing numbers");
  gtk_widget_show (scope_overwrite);
  gtk_box_pack_start (GTK_BOX (vbox3), scope_overwrite, FALSE, FALSE, 6);
  gtk_widget_add_accelerator (scope_overwrite, "clicked", accel_group,
                              GDK_o, (GdkModifierType) 0,
                              GTK_ACCEL_VISIBLE);

  label1 = gtk_label_new ("<b>Scope</b>");
  gtk_widget_show (label1);
  gtk_frame_set_label_widget (GTK_FRAME (frame1), label1);
  gtk_label_set_use_markup (GTK_LABEL (label1), TRUE);

  frame2 = gtk_frame_new (NULL);
  gtk_widget_show (frame2);
  gtk_box_pack_start (GTK_BOX (vbox1), frame2, TRUE, TRUE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_NONE);

  alignment2 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment2);
  gtk_container_add (GTK_CONTAINER (frame2), alignment2);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment2), 0, 0, 24, 0);

  table2 = gtk_table_new (4, 4, FALSE);
  gtk_widget_show (table2);
  gtk_container_add (GTK_CONTAINER (alignment2), table2);
  gtk_table_set_row_spacings (GTK_TABLE (table2), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table2), 12);

  order_file = gtk_radio_button_new_with_mnemonic (NULL, "File order");
  gtk_widget_show (order_file);
  gtk_table_attach (GTK_TABLE (table2), order_file, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (order_file), order_file_group);
  order_file_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (order_file));

  order_top2bottom = gtk_radio_button_new_with_mnemonic (NULL, "Top to bottom");
  gtk_widget_show (order_top2bottom);
  gtk_table_attach (GTK_TABLE (table2), order_top2bottom, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (order_top2bottom), order_file_group);
  order_file_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (order_top2bottom));

  order_left2right = gtk_radio_button_new_with_mnemonic (NULL, "Left to right");
  gtk_widget_show (order_left2right);
  gtk_table_attach (GTK_TABLE (table2), order_left2right, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (order_left2right), order_file_group);
  order_file_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (order_left2right));

  order_diagonal = gtk_radio_button_new_with_mnemonic (NULL, "Diagonal");
  gtk_widget_show (order_diagonal);
  gtk_table_attach (GTK_TABLE (table2), order_diagonal, 3, 4, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (order_diagonal), order_file_group);
  order_file_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (order_diagonal));

  image2 = autonumber_create_pixmap("gschem-left2right.png", w_current);
  gtk_widget_show (image2);
  gtk_table_attach (GTK_TABLE (table2), image2, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  image3 = autonumber_create_pixmap("gschem-diagonal.png", w_current);
  gtk_widget_show (image3);
  gtk_table_attach (GTK_TABLE (table2), image3, 3, 4, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  image1 = autonumber_create_pixmap("gschem-top2bottom.png", w_current);
  gtk_widget_show (image1);
  gtk_table_attach (GTK_TABLE (table2), image1, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  image4 = autonumber_create_pixmap("gschem-bottom2top.png", w_current);
  gtk_widget_show (image4);
  gtk_table_attach (GTK_TABLE (table2), image4, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  image5 = autonumber_create_pixmap("gschem-right2left.png", w_current);
  gtk_widget_show (image5);
  gtk_table_attach (GTK_TABLE (table2), image5, 2, 3, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  order_bottom2top = gtk_radio_button_new_with_mnemonic (NULL, "Bottom to top");
  gtk_widget_show (order_bottom2top);
  gtk_table_attach (GTK_TABLE (table2), order_bottom2top, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (order_bottom2top), order_file_group);
  order_file_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (order_bottom2top));

  order_right2left = gtk_radio_button_new_with_mnemonic (NULL, "Right to left");
  gtk_widget_show (order_right2left);
  gtk_table_attach (GTK_TABLE (table2), order_right2left, 2, 3, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (order_right2left), order_file_group);
  order_file_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (order_right2left));

  label2 = gtk_label_new ("<b>Sort order</b>");
  gtk_widget_show (label2);
  gtk_frame_set_label_widget (GTK_FRAME (frame2), label2);
  gtk_label_set_use_markup (GTK_LABEL (label2), TRUE);

  frame3 = gtk_frame_new (NULL);
  gtk_widget_show (frame3);
  gtk_box_pack_start (GTK_BOX (vbox1), frame3, TRUE, TRUE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame3), GTK_SHADOW_NONE);

  alignment3 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment3);
  gtk_container_add (GTK_CONTAINER (frame3), alignment3);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment3), 0, 0, 24, 0);

  vbox4 = gtk_vbox_new (FALSE, 3);
  gtk_widget_show (vbox4);
  gtk_container_add (GTK_CONTAINER (alignment3), vbox4);

  hbox1 = gtk_hbox_new (FALSE, 12);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox4), hbox1, TRUE, TRUE, 0);

  label12 = gtk_label_new_with_mnemonic ("Starting _number:");
  gtk_widget_show (label12);
  gtk_box_pack_start (GTK_BOX (hbox1), label12, FALSE, FALSE, 0);

  opt_startnum_adj = gtk_adjustment_new (1, 0, 10000, 1, 10, 10);
  opt_startnum = gtk_spin_button_new (GTK_ADJUSTMENT (opt_startnum_adj), 1, 0);
  gtk_widget_show (opt_startnum);
  gtk_box_pack_start (GTK_BOX (hbox1), opt_startnum, FALSE, FALSE, 0);

  opt_removenum = gtk_check_button_new_with_mnemonic ("_Remove numbers");
  gtk_widget_show (opt_removenum);
  gtk_box_pack_start (GTK_BOX (vbox4), opt_removenum, FALSE, FALSE, 0);
  gtk_widget_add_accelerator (opt_removenum, "clicked", accel_group,
                              GDK_r, (GdkModifierType) 0,
                              GTK_ACCEL_VISIBLE);

  opt_slotting = gtk_check_button_new_with_mnemonic ("Automatic slo_tting");
  gtk_widget_show (opt_slotting);
  gtk_box_pack_start (GTK_BOX (vbox4), opt_slotting, FALSE, FALSE, 0);
  gtk_widget_add_accelerator (opt_slotting, "clicked", accel_group,
                              GDK_t, (GdkModifierType) 0,
                              GTK_ACCEL_VISIBLE);

  label3 = gtk_label_new ("<b>Options</b>");
  gtk_widget_show (label3);
  gtk_frame_set_label_widget (GTK_FRAME (frame3), label3);
  gtk_label_set_use_markup (GTK_LABEL (label3), TRUE);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_show (hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox1, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1), GTK_BUTTONBOX_END);
  gtk_box_set_spacing (GTK_BOX (hbuttonbox1), 12);

  button_cancel = gtk_button_new_from_stock ("gtk-close");
  gtk_widget_show (button_cancel);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button_cancel);
  GTK_WIDGET_SET_FLAGS (button_cancel, GTK_CAN_DEFAULT);

  button_ok = gtk_button_new_from_stock ("gtk-apply");
  gtk_widget_show (button_ok);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button_ok);
  GTK_WIDGET_SET_FLAGS (button_ok, GTK_CAN_DEFAULT);
  gtk_widget_add_accelerator (button_ok, "clicked", accel_group,
                              GDK_Return, (GdkModifierType) 0,
                              GTK_ACCEL_VISIBLE);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label12), opt_startnum);

  GLADE_HOOKUP_OBJECT (autonumber_text, scope_text, "scope_text");
  GLADE_HOOKUP_OBJECT (autonumber_text, scope_skip, "scope_skip");
  GLADE_HOOKUP_OBJECT (autonumber_text, scope_number, "scope_number");
  GLADE_HOOKUP_OBJECT (autonumber_text, scope_overwrite, "scope_overwrite");
  GLADE_HOOKUP_OBJECT (autonumber_text, order_file, "order_file");
  GLADE_HOOKUP_OBJECT (autonumber_text, order_top2bottom, "order_top2bottom");
  GLADE_HOOKUP_OBJECT (autonumber_text, order_left2right, "order_left2right");
  GLADE_HOOKUP_OBJECT (autonumber_text, order_diagonal, "order_diagonal");
  GLADE_HOOKUP_OBJECT (autonumber_text, order_bottom2top, "order_bottom2top");
  GLADE_HOOKUP_OBJECT (autonumber_text, order_right2left, "order_right2left");
  GLADE_HOOKUP_OBJECT (autonumber_text, opt_startnum, "opt_startnum");
  GLADE_HOOKUP_OBJECT (autonumber_text, opt_removenum, "opt_removenum");
  GLADE_HOOKUP_OBJECT (autonumber_text, opt_slotting, "opt_slotting");
  GLADE_HOOKUP_OBJECT (autonumber_text, button_cancel, "button_close");
  GLADE_HOOKUP_OBJECT (autonumber_text, button_ok, "button_ok");

  gtk_window_add_accel_group (GTK_WINDOW (autonumber_text), accel_group);

  return autonumber_text;
}

/*! \brief Create or restore the autonumber text dialog
 *
 *  If the function is called the first time the dialog is created.
 *  If the dialog is only in background it is moved to the foreground.
 *
 *  @param w_current Pointer to the top level struct
 */
void autonumber_text_dialog(TOPLEVEL *w_current)
{
	static AUTONUMBER_TEXT *autotext = NULL;

	GtkWidget *button_ok = NULL;
	GtkWidget *button_close = NULL;

	if(autotext == NULL) {
		/* first call of this function, init dialog structure */
		autotext=autonumber_init_state();
	}

	/* set the toplevel always. Can it be changed between the calls??? */
	autotext->toplevel = w_current;

	if(autotext->dialog == NULL) {
		/* Dialog is not currently displayed - create it */

		autotext->dialog = autonumber_create_dialog(w_current);

		button_ok = lookup_widget(autotext->dialog, "button_ok");
		button_close = lookup_widget(autotext->dialog, "button_close");

		gtk_signal_connect(GTK_OBJECT(autotext->dialog),
				"destroy",
				(GtkSignalFunc) autonumber_text_done,
				autotext);

		gtk_signal_connect(GTK_OBJECT(button_ok), 
				"clicked",
				GTK_SIGNAL_FUNC(autonumber_text_ok), 
				autotext);

		gtk_signal_connect(GTK_OBJECT(button_close), 
				"clicked",
				GTK_SIGNAL_FUNC(autonumber_text_close), 
				autotext);

		autonumber_set_state(autotext);

		gtk_widget_show_all(autotext->dialog);
	}

	/* if the dialog is in the background or minimized: show it */
	gtk_window_present(GTK_WINDOW(autotext->dialog));
}
