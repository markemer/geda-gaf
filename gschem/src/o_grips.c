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

/* Kazu on July 16, 1999 - Added these macros to simplify the code */
#define GET_BOX_WIDTH(w)                        \
        abs((w)->last_x - (w)->start_x)
#define GET_BOX_HEIGHT(w)                       \
	        abs((w)->last_y - (w)->start_y)
#define GET_BOX_LEFT(w)                         \
	        min((w)->start_x, (w)->last_x);
#define GET_BOX_TOP(w)                          \
	        min((w)->start_y, (w)->last_y);

#define GET_PICTURE_WIDTH(w)			\
	abs((w)->last_x - (w)->start_x) 
#define GET_PICTURE_HEIGHT(w)			\
	(w)->pixbuf_wh_ratio == 0 ? 0 : abs((w)->last_x - (w)->start_x)/(w)->pixbuf_wh_ratio
#define GET_PICTURE_LEFT(w)				\
	min((w)->start_x, (w)->last_x);
#define GET_PICTURE_TOP(w)				\
	(w)->start_y < (w)->last_y ? (w)->start_y  : \
                                     (w)->start_y-abs((w)->last_x - (w)->start_x)/(w)->pixbuf_wh_ratio;

/*! \brief
 *  This variable holds the identifier of the grip currently under
 *  modification. Its range of values depends on the type of object.
 */
static int whichone_changing = -1;
/*! \brief
 *  This variable holds a pointer on the object under modification.
 */
static OBJECT *object_changing;

/*! \brief Check if point is inside grip.
 *  \par Function Description
 *  This function is used to determine if the (<B>x</B>,<B>y</B>) point is
 *  inside a grip of one of the selected object on the current sheet.
 *  The selected object are in a list starting at
 *  <B>w_current->page_current->selection2_head</B>.
 *  The <B>x</B> and <B>y</B> parameters are in world units.
 *  If the point is inside one grip, a pointer on the object it belongs to is
 *  returned and <B>*whichone</B> is set according to the position of the grip
 *  on the object.
 *  Else, <B>*whichone</B> is unchanged and the function returns <B>NULL</B>.
 *
 *  A specific search function is provided for every kind of graphical object.
 *  The list of selected object is covered : each object is tested with the
 *  appropriate function.
 *
 *  \param [in]  w_current  The TOPLEVEL object.
 *  \param [in]  x          Current x coordinate of pointer in world units.
 *  \param [in]  y          Current y coordinate of pointer in world units.
 *  \param [out] whichone   Which grip point is selected.
 *  \return Pointer to OBJECT the grip is on, NULL otherwise.
 */
OBJECT *o_grips_search_world(TOPLEVEL *w_current, int x, int y, int *whichone)
{
  OBJECT *object=NULL;
  OBJECT *found=NULL;
  GList *s_current;
  int size, x2size;
  int w_size;
	
  if (!whichone) {
    return(NULL);
  }

  /* get the size of the grip according to zoom level */
  size = o_grips_size(w_current);
  /* size is half the width of a grip, x2size is full width */
  x2size = size * 2;

  w_size = WORLDabs( w_current, size );
	
  /* skip over head */
  s_current = w_current->page_current->selection_list;
  while (s_current != NULL) {
    object = (OBJECT *) s_current->data;
    if (object) {
      switch(object->type) {
        case(OBJ_ARC):
          /* check the grips of the arc object */
          found = o_grips_search_arc_world(w_current, object,
                                           x, y, w_size, whichone);
          if(found != NULL) return found;
          break;
				
        case(OBJ_BOX):
          /* check the grips of the box object */
          found = o_grips_search_box_world(w_current, object,
                                           x, y, w_size, whichone);
          if(found != NULL) return found;
          break;
		  
        case(OBJ_PICTURE):
          /* check the grips of the picture object */
          found = o_grips_search_picture_world(w_current, object,
                                               x, y, w_size, whichone);
          if(found != NULL) return found;
          break;
		  
        case(OBJ_CIRCLE):
          /* check the grips of the circle object */
          found = o_grips_search_circle_world(w_current, object,
                                              x, y, w_size, whichone);
          if(found != NULL) return found;
          break;
			
        case(OBJ_LINE):
        case(OBJ_PIN):
        case(OBJ_NET):
        case(OBJ_BUS):
          /* check the grips of the line object */
          /* the function is the same for line, pin, net, bus */
          found = o_grips_search_line_world(w_current, object,
                                            x, y, w_size, whichone);
          if(found != NULL) return found;
          break;

#if 0 
      /* This code is wrong.  Continue searching even if the object */
      /* does not have grips */
        default:
          /* object type is unknown : error condition */
          return NULL;
#endif
      }
    }
    s_current = s_current->next;	
  }
	
  return(NULL);
}

/*! \brief Check if pointer is inside arc grip.
 *  \par Function Description
 *  This function checks if the pointer event occuring at (<B>x</B>,<B>y</B>) is
 *  inside one of the grips of an <B>o_current</B> pointed arc object. If so
 *  the <B>whichone</B> pointed integer is set to the number of this grip and
 *  the return pointer is a pointer on this object. If the point is not
 *  inside a grip the function returns a NULL pointer and the <B>whichone</B>
 *  pointed integer is unset.
 *
 *  An arc object has three grips :
 *  <DL>
 *    <DT>*</DT><DD>one at the center of the arc. This grip is used to modify
 *                  the radius of the arc. If this one is selected, the
 *                  <B>whichone</B> pointed integer is set to <B>ARC_RADIUS</B>.
 *    <DT>*</DT><DD>one at one end of the arc. It corresponds to the starting
 *                  angle of the arc. If this one is selected, the
 *                  <B>whichone</B> pointed integer is set to <B>ARC_START_ANGLE</B>.
 *    <DT>*</DT><DD>one at the other end of the arc. It corresponds to the
 *                  ending angle of the arc. If this one is selected, the
 *                  <B>whichone</B> pointed integer is set to <B>ARC_END_ANGLE</B>.
 *  </DL>
 *
 *  The <B>x</B> and <B>y</B> parameters are in world units.
 *
 *  The <B>size</B> parameter is the width (and height) of the square
 *  representing a grip in world units.
 *
 *  \param [in]  w_current  The TOPLEVEL object.
 *  \param [in]  o_current  Arc OBJECT to check.
 *  \param [in]  x          Current x coordinate of pointer in world units.
 *  \param [in]  y          Current y coordinate of pointer in world units.
 *  \param [in]  size       Half the width of the grip square in world units.
 *  \param [out] whichone   Which grip point is selected.
 *  \return Pointer to OBJECT the grip is on, NULL otherwise.
 */
OBJECT *o_grips_search_arc_world(TOPLEVEL *w_current, OBJECT *o_current,
			   int x, int y, int size, int *whichone)
{
  int centerx, centery, radius, start_angle, end_angle;
  int xmin, ymin, xmax, ymax;
  int x2size;
  double tmp;

  centerx     = o_current->arc->x;
  centery     = o_current->arc->y;
  radius      = o_current->arc->width / 2;
  start_angle = o_current->arc->start_angle;
  end_angle   = o_current->arc->end_angle;

  /* width/height of the grip */
  x2size = 2 * size;

  /* check the grip on the center of the arc */
  xmin = centerx - size;
  ymin = centery - size;
  xmax = xmin + x2size;
  ymax = ymin + x2size;
  if (inside_region(xmin, ymin, xmax, ymax, x, y)) {
    *whichone = ARC_RADIUS;
    return(o_current);
  }

  /* check the grip at the start angle of the arc */
  tmp = ((double) start_angle) * M_PI / 180;
  xmin = centerx + radius * cos(tmp) - size;
  ymin = centery + radius * sin(tmp) - size;
  xmax = xmin + x2size;
  ymax = ymin + x2size;
  if (inside_region(xmin, ymin, xmax, ymax, x, y)) {
    *whichone = ARC_START_ANGLE;
    return(o_current);
  }

  /* check the grip at the end angle of the arc */
  tmp = ((double) start_angle + end_angle) * M_PI / 180;
  xmin = centerx + radius * cos(tmp) - size;
  ymin = centery + radius * sin(tmp) - size;
  xmax = xmin + x2size;
  ymax = ymin + x2size;
  if (inside_region(xmin, ymin, xmax, ymax, x, y)) {
    *whichone = ARC_END_ANGLE;
    return(o_current);
  }

  return NULL;
}

/*! \brief Check if pointer is inside box grip.
 *  \par Function Description
 *  This function checks if the pointer event occuring at (<B>x</B>,<B>y</B>) is
 *  inside one of the grips of the <B>o_current</B> pointed box object.
 *  If so, the <B>whichone</B> pointed integer is set to the identifier of
 *  this grip and the returned pointer is a pointer on this object. 
 *  If the point is not inside a grip the function returns a NULL pointer
 *  and the <B>whichone</B> pointed integer is unset.
 *
 *  A box object has four grips : one at each corner of the box. The
 *  identifiers of each corner are <B>BOX_UPPER_LEFT</B>,
 *  <B>BOX_UPPER_RIGHT</B>, <B>BOX_LOWER_LEFT</B> and <B>BOX_LOWER_RIGHT</B>.
 *
 *  The <B>x</B> and <B>y</B> parameters are in world units.
 *
 *  The <B>size</B> parameter is half the width (and half the height) of
 *  the square representing a grip in world units.
 *
 *  \param [in]  w_current  The TOPLEVEL object.
 *  \param [in]  o_current  Box OBJECT to check.
 *  \param [in]  x          Current x coordinate of pointer in world units.
 *  \param [in]  y          Current y coordinate of pointer in world units.
 *  \param [in]  size       Half the width of the grip square in world units.
 *  \param [out] whichone   Which grip point is selected.
 *  \return Pointer to OBJECT the grip is on, NULL otherwise.
 */
OBJECT *o_grips_search_box_world(TOPLEVEL *w_current, OBJECT *o_current,
			   int x, int y, int size, int *whichone)
{
  int xmin, ymin, xmax, ymax;
  int x2size;

  /* width/height of the grip */
  x2size = 2 * size;

  /* inside upper left grip ? */
  xmin = o_current->box->upper_x - size;
  ymin = o_current->box->upper_y - size;
  xmax = xmin + x2size;
  ymax = ymin + x2size;
  if (inside_region(xmin, ymin, xmax, ymax, x, y)) {
    *whichone = BOX_UPPER_LEFT;
    return(o_current);
  }

  /* inside lower right grip ? */
  xmin = o_current->box->lower_x - size;
  ymin = o_current->box->lower_y - size;
  xmax = xmin + x2size;
  ymax = ymin + x2size;
  if (inside_region(xmin, ymin, xmax, ymax, x, y)) {
    *whichone = BOX_LOWER_RIGHT;
    return(o_current);
  }

  /* inside upper right grip ? */
  xmin = o_current->box->lower_x - size;
  ymin = o_current->box->upper_y - size;
  xmax = xmin + x2size;
  ymax = ymin + x2size;
  if (inside_region(xmin, ymin, xmax, ymax, x, y)) {
    *whichone = BOX_UPPER_RIGHT;
    return(o_current);
  }

  /* inside lower left grip ? */
  xmin = o_current->box->upper_x - size;
  ymin = o_current->box->lower_y - size;
  xmax = xmin + x2size;
  ymax = ymin + x2size;
  if (inside_region(xmin, ymin, xmax, ymax, x, y)) {
    *whichone = BOX_LOWER_LEFT;
    return(o_current);
  }

  return NULL;
}

/*! \brief Check if pointer is inside picture grip.
 *  \par Function Description
 *  This function checks if the pointer event occuring at (<B>x</B>,<B>y</B>)
 *  is inside one of the grips of the <B>o_current</B> pointed picture object. 
 *  If so, the <B>whichone</B> pointed integer is set to the identifier of
 *  this grip and the returned pointer is a pointer on this object. 
 *  If the point is not inside a grip the function returns a NULL pointer
 *  and the <B>whichone</B> pointed integer is unset.
 *
 *  A picture object has four grips : one at each corner of the picture.
 *  The identifiers of each corner are #PICTURE_UPPER_LEFT,
 *  #PICTURE_UPPER_RIGHT, #PICTURE_LOWER_LEFT and
 *  #PICTURE_LOWER_RIGHT.
 *
 *  The <B>x</B> and <B>y</B> parameters are in world units.
 *
 *  The <B>size</B> parameter is half the width (and half the height) of the
 *  square representing a grip in world units.
 *
 *  \param [in]  w_current  The TOPLEVEL object.
 *  \param [in]  o_current  Picture OBJECT to check.
 *  \param [in]  x          Current x coordinate of pointer in world units.
 *  \param [in]  y          Current y coordinate of pointer in world units.
 *  \param [in]  size       Half the width of the grip square in world units.
 *  \param [out] whichone   Which grip point is selected.
 *  \return Pointer to OBJECT the grip is on, NULL otherwise.
 */
OBJECT *o_grips_search_picture_world(TOPLEVEL *w_current, OBJECT *o_current,
			       int x, int y, int size, int *whichone)
{
  int xmin, xmax, ymin, ymax;
  int x2size;

  /* width/height of the grip */
  x2size = 2 * size;

  /* inside upper left grip ? */
  xmin = o_current->picture->upper_x - size;
  ymin = o_current->picture->upper_y - size;
  xmax = xmin + x2size;
  ymax = ymin + x2size;
  if (inside_region(xmin, ymin, xmax, ymax, x, y)) {
    *whichone = PICTURE_UPPER_LEFT;
    return(o_current);
  }

  /* inside lower right grip ? */
  xmin = o_current->picture->lower_x - size;
  ymin = o_current->picture->lower_y - size;
  xmax = xmin + x2size;
  ymax = ymin + x2size;
  if (inside_region(xmin, ymin, xmax, ymax, x, y)) {
    *whichone = PICTURE_LOWER_RIGHT;
    return(o_current);
  }

  /* inside upper right grip ? */
  xmin = o_current->picture->lower_x - size;
  ymin = o_current->picture->upper_y - size;
  xmax = xmin + x2size;
  ymax = ymin + x2size;
  if (inside_region(xmin, ymin, xmax, ymax, x, y)) {
    *whichone = PICTURE_UPPER_RIGHT;
    return(o_current);
  }

  /* inside lower left grip ? */
  xmin = o_current->picture->upper_x - size;
  ymin = o_current->picture->lower_y - size;
  xmax = xmin + x2size;
  ymax = ymin + x2size;
  if (inside_region(xmin, ymin, xmax, ymax, x, y)) {
    *whichone = PICTURE_LOWER_LEFT;
    return(o_current);
  }

  return NULL;
}

/*! \brief Check if pointer is inside circle grip.
 *  \par Function Description
 *  This function determines if the (<B>x</B>,<B>y</B>) point is inside one of
 *  the grip of the circle object <B>o_current</B>.
 *  It computes the area covered by each grip and check if (<B>x</B>,<B>y</B>)
 *  is in one of these areas.
 *  If the event occured in one of the grip, a pointer on the object is
 *  returned and <B>*whichone</B> is set to the identifier of the grip.
 *  If not, the function returns a <B>NULL</B> pointer and <B>*whichone</B>
 *  is unchanged.
 *
 *  The parameter <B>size</B> is half the size of the grip in world units.
 *
 *  A circle has only one grip on the lower right corner of the box it
 *  is inscribed in. Moving this grip change the radius of the circle.
 *  The identifier of this grip is <B>CIRCLE_RADIUS</B>.
 *
 *  \param [in]  w_current  The TOPLEVEL object.
 *  \param [in]  o_current  Circle OBJECT to check.
 *  \param [in]  x          Current x coordinate of pointer in world units.
 *  \param [in]  y          Current y coordinate of pointer in world units.
 *  \param [in]  size       Half the width of the grip square in world units.
 *  \param [out] whichone   Which grip point is selected.
 *  \return Pointer to OBJECT the grip is on, NULL otherwise.
 */
OBJECT *o_grips_search_circle_world(TOPLEVEL *w_current, OBJECT *o_current,
			      int x, int y, int size, int *whichone)
{
	int xmin, ymin, xmax, ymax;
	int x1, y1;
	int x2size;
	
	/* width/height of the grip */
	x2size = 2 * size;

        /* check the grip for radius */	
	x1 = o_current->circle->center_x + o_current->circle->radius;
	y1 = o_current->circle->center_y - o_current->circle->radius;
	xmin = x1 - size;
	ymin = y1 - size;
	xmax = xmin + x2size;
	ymax = ymin + x2size;
	if (inside_region(xmin, ymin, xmax, ymax, x, y)) {
		/* printf("found something 0!\n"); */
		*whichone = CIRCLE_RADIUS;
		return(o_current);
	}

	return NULL;
}

/*! \brief Check if pointer is inside line grip.
 *  \par Function Description
 *  This function determines if the (<B>x</B>,<B>y</B>) point is inside one of
 *  the grip of the line object <B>o_current</B>.
 *  It computes the area covered by each grip and check if (<B>x</B>,<B>y</B>)
 *  is in one of these areas. 
 *  If the event occured in one of its grip, a pointer on the object is
 *  returned and <B>*whichone</B> is set to the identifier of the grip. If not,
 *  the function returns <B>NULL</B> pointer and <B>*whichone</B> is unchanged.
 *
 *  The parameter <B>size</B> is half the size of the grip in world units.
 *
 *  \param [in]  w_current  The TOPLEVEL object.
 *  \param [in]  o_current  Line OBJECT to check.
 *  \param [in]  x          Current x coordinate of pointer in world units.
 *  \param [in]  y          Current y coordinate of pointer in world units.
 *  \param [in]  size       Half the width of the grip square in world units.
 *  \param [out] whichone   Which grip point is selected.
 *  \return Pointer to OBJECT the grip is on, NULL otherwise.
 */
OBJECT *o_grips_search_line_world(TOPLEVEL *w_current, OBJECT *o_current,
			    int x, int y, int size, int *whichone)
{
	int xmin, ymin, xmax, ymax;
	int x2size;

	/* width/height of the grip */
	x2size = 2 * size;

	/* check the grip on the end of line 1 */
	xmin = o_current->line->x[LINE_END1] - size;
	ymin = o_current->line->y[LINE_END1] - size;
	xmax = xmin + x2size;
	ymax = ymin + x2size;
	if (inside_region(xmin, ymin, xmax, ymax, x, y)) {
		*whichone = LINE_END1;
		return(o_current);
	}

	/* check the grip on the end of line 2 */
	xmin = o_current->line->x[LINE_END2] - size;
	ymin = o_current->line->y[LINE_END2] - size;
	xmax = xmin + x2size;
	ymax = ymin + x2size;
	if (inside_region(xmin, ymin, xmax, ymax, x, y)) {
		*whichone = LINE_END2;
		return(o_current);
	}

	return NULL;
}

/*! \brief Start process of modifiying one grip.
 *  \par Function Description
 *  This function starts the process of modifying one grip of an object
 *  on the current sheet. The event occured in (<B>x</B>,<B>y</B>) in screen unit.
 *  If this position is related to a grip of an object, the function
 *  prepares the modification of this grip thanks to the user input.
 *
 *  The function returns <B>FALSE</B> if an error occured of if no grip
 *  have been found under (<B>x</B>,<B>y</B>). It returns <B>TRUE</B> if a grip
 *  has been found and modification of the object has been started.
 *
 *  If a grip has been found, this function modifies the global variables
 *  <B>whichone_changing</B> and <B>object_changing</B> with respectively the
 *  identifier of the grip and the object it belongs to.
 *
 *  \param [in]  w_current  The TOPLEVEL object.
 *  \param [in]  x          Current x coordinate of pointer in screen units.
 *  \param [in]  y          Current y coordinate of pointer in screen units.
 *  \return FALSE if an error occurred or no grip was found, TRUE otherwise.
 */
int o_grips_start(TOPLEVEL *w_current, int x, int y)
{
  int w_x, w_y;
  OBJECT *object;
  int whichone;
	
  if (w_current->draw_grips == FALSE) {
    return(FALSE);
  }

  SCREENtoWORLD( w_current, x, y, &w_x, &w_y );

  /* search if there is a grip on a selected object at (x,y) */
  object = o_grips_search_world(w_current, w_x, w_y, &whichone);
  if (object) {
    /* there is one */
    /* depending on its type, start the modification process */
    switch(object->type) {
      case(OBJ_ARC):
	/* start the modification of a grip on an arc */
        o_grips_start_arc(w_current, object, x, y, whichone);
				
        whichone_changing = whichone;
        object_changing = object;
        return(TRUE);
        break;

      case(OBJ_BOX):
	/* start the modification of a grip on a box */
        o_grips_start_box(w_current, object, x, y, whichone);
				
        whichone_changing = whichone;
        object_changing = object;
        return(TRUE);
        break;
				
      case(OBJ_PICTURE):
	/* start the modification of a grip on a picture */
        o_grips_start_picture(w_current, object, x, y, whichone);
				
        whichone_changing = whichone;
        object_changing = object;
        return(TRUE);
        break;
				
      case(OBJ_CIRCLE):
	/* start the modification of a grip on a circle */
        o_grips_start_circle(w_current, object, x, y, whichone);
				
        whichone_changing = whichone;
        object_changing = object;
        return(TRUE);
        break;
			
      case(OBJ_LINE):
	/* start the modification of a grip on a line */
        o_grips_start_line(w_current, object, x, y, whichone);
				
        whichone_changing = whichone;
        object_changing = object;
        return(TRUE);
        break;

      case(OBJ_NET): 
        w_current->last_drawb_mode = -1;
        WORLDtoSCREEN( w_current, object->line->x[whichone], object->line->y[whichone],
                       &w_current->last_x, &w_current->last_y );
        WORLDtoSCREEN( w_current, object->line->x[!whichone], object->line->y[!whichone],
                       &w_current->start_x, &w_current->start_y );
				
        o_net_erase(w_current, object);
        gdk_gc_set_foreground(w_current->xor_gc, 
                              x_get_darkcolor(w_current->select_color) );
        gdk_draw_line(w_current->window, w_current->xor_gc, 
                      w_current->start_x, w_current->start_y, 
                      w_current->last_x, w_current->last_y);
        o_line_erase_grips(w_current, object);
				
        whichone_changing = whichone;
        object_changing = object;
        gdk_gc_set_foreground(w_current->gc, 
                              x_get_color(w_current->background_color)); 
        return(TRUE);
				
        break; 
				
      case(OBJ_PIN): 
				
        w_current->last_drawb_mode = -1;
        WORLDtoSCREEN( w_current, object->line->x[whichone], object->line->y[whichone],
                       &w_current->last_x, &w_current->last_y );
        WORLDtoSCREEN( w_current, object->line->x[!whichone], object->line->y[!whichone],
                       &w_current->start_x, &w_current->start_y );
				
        o_pin_erase(w_current, object);
        gdk_gc_set_foreground(w_current->xor_gc, 
                              x_get_darkcolor(w_current->select_color) );
        gdk_draw_line(w_current->window, w_current->xor_gc, 
                      w_current->start_x, w_current->start_y, 
                      w_current->last_x, w_current->last_y);
        o_line_erase_grips(w_current, object);
				
        whichone_changing = whichone;
        object_changing = object;
        return(TRUE);
				
        break; 
				
      case(OBJ_BUS): 
        w_current->last_drawb_mode = -1;
        WORLDtoSCREEN( w_current, object->line->x[whichone], object->line->y[whichone],
                       &w_current->last_x, &w_current->last_y );
        WORLDtoSCREEN( w_current, object->line->x[!whichone], object->line->y[!whichone],
                       &w_current->start_x, &w_current->start_y );
				
        o_bus_erase(w_current, object);
        gdk_gc_set_foreground(w_current->xor_gc, 
                              x_get_darkcolor(w_current->select_color) );
        gdk_draw_line(w_current->window, w_current->xor_gc, 
                      w_current->start_x, w_current->start_y, 
                      w_current->last_x, w_current->last_y);
        o_line_erase_grips(w_current, object);
				
        whichone_changing = whichone;
        object_changing = object;
        gdk_gc_set_foreground(w_current->gc, 
                              x_get_color(w_current->background_color)); 
        return(TRUE);
				
        break;

      default:
				/* object type unknown : error condition */
        return(FALSE);
				
    }
  }
	
  return(FALSE);
}

/*! \brief Initialize grip motion process for an arc.
 *  \par Function Description
 *  This function initializes the grip motion process for an arc.
 *  From the <B>o_current</B> pointed object, it stores into the TOPLEVEL
 *  structure the coordinates of the center, the radius and the two angle
 *  that describes an arc. These variables are used in the grip process.
 *
 *  The coordinates of the center of the arc on x- and y-axis are stored
 *  into the <B>loc_x</B> and <B>loc_y</B> fields of the TOPLEVEL structure
 *  in screen unit.
 *
 *  The radius of the center is stored into the <B>distance</B> field of
 *  the TOPLEVEL structure in screen unit.
 *
 *  The two angles describing the arc on a circle are stored into the
 *  <B>start_x</B> for the starting angle and <B>start_y</B> for the ending angle.
 *  These angles are expressed in degrees.
 *
 *  Depending on which grips has been selected on the arc, the
 *  corresponding variables in its original state is duplicated in
 *  <B>last_x</B> and/or <B>last_y</B> of the TOPLEVEL structure.
 *
 *  \param [in]  w_current  The TOPLEVEL object.
 *  \param [in]  o_current  Arc OBJECT to check.
 *  \param [in]  x          (unused)
 *  \param [in]  y          (unused)
 *  \param [out] whichone   (unused)
 */
void o_grips_start_arc(TOPLEVEL *w_current, OBJECT *o_current,
		       int x, int y, int whichone)
{
  w_current->last_drawb_mode = -1;

  /* erase the arc before */
  o_arc_erase(w_current, o_current);

  /* describe the arc with TOPLEVEL variables */
  /* center */
  WORLDtoSCREEN( w_current, o_current->arc->x, o_current->arc->y, &w_current->start_x, &w_current->start_y );
  /* radius */
  w_current->distance = SCREENabs( w_current, o_current->arc->width / 2 );
  /* angles */
  w_current->loc_x = o_current->arc->start_angle;
  w_current->loc_y = o_current->arc->end_angle;

  /* draw the first temporary arc */
  o_arc_rubberarc_xor(w_current);
	
}

/*! \brief Initialize grip motion process for a box.
 *  \par Function Description
 *  This function initializes the grip motion process for a box. From the
 *  <B>o_current</B> pointed object, it stores into the TOPLEVEL structure
 *  the .... These variables are used in the grip process.
 *
 *  The function first erases the grips.
 *
 *  The coordinates of the selected corner are put in
 *  (<B>w_current->last_x</B>,<B>w_current->last_y</B>).
 *
 *  The coordinates of the opposite corner go in
 *  (<B>w_current->start_x</B>,<B>w_current->start_y</B>). They are not suppose
 *  to change during the action.
 *
 *  \param [in]  w_current  The TOPLEVEL object.
 *  \param [in]  o_current  Box OBJECT to check.
 *  \param [in]  x          (unused)
 *  \param [in]  y          (unused)
 *  \param [out] whichone   Which coordinate to check.
 */
void o_grips_start_box(TOPLEVEL *w_current, OBJECT *o_current,
		       int x, int y, int whichone)
{
  w_current->last_drawb_mode = -1;
	
  /* erase the box before */
  o_box_erase(w_current, o_current);

  /* (last_x,last_y)    is the selected corner */
  /* (start_x, start_y) is the opposite corner */
  switch(whichone) {
    case BOX_UPPER_LEFT: 
      WORLDtoSCREEN( w_current, o_current->box->upper_x, o_current->box->upper_y,
                     &w_current->last_x, &w_current->last_y );
      WORLDtoSCREEN( w_current, o_current->box->lower_x, o_current->box->lower_y,
                     &w_current->start_x, &w_current->start_y );
      break;
    case BOX_LOWER_RIGHT: 
      WORLDtoSCREEN( w_current, o_current->box->lower_x, o_current->box->lower_y,
                     &w_current->last_x, &w_current->last_y );
      WORLDtoSCREEN( w_current, o_current->box->upper_x, o_current->box->upper_y,
                     &w_current->start_x, &w_current->start_y );
      break;
    case BOX_UPPER_RIGHT: 
      WORLDtoSCREEN( w_current, o_current->box->lower_x, o_current->box->upper_y,
                     &w_current->last_x, &w_current->last_y );
      WORLDtoSCREEN( w_current, o_current->box->upper_x, o_current->box->lower_y,
                     &w_current->start_x, &w_current->start_y );
      break;
    case BOX_LOWER_LEFT: 
      WORLDtoSCREEN( w_current, o_current->box->upper_x, o_current->box->lower_y,
                     &w_current->last_x, &w_current->last_y );
      WORLDtoSCREEN( w_current, o_current->box->lower_x, o_current->box->upper_y,
                     &w_current->start_x, &w_current->start_y );
      break;
    default:
      return; /* error */
  }

  /* draw the first temporary box */
  o_box_rubberbox_xor(w_current);

}

/*! \brief Initialize grip motion process for a picture.
 *  \par Function Description
 *  This function initializes the grip motion process for a picture.
 *  From the <B>o_current</B> pointed object, it stores into the TOPLEVEL
 *  structure the .... These variables are used in the grip process.
 *
 *  The function first erases the grips.
 *
 *  The coordinates of the selected corner are put in
 *  (<B>w_current->last_x</B>,<B>w_current->last_y</B>).
 *
 *  The coordinates of the opposite corner go in
 *  (<B>w_current->start_x</B>,<B>w_current->start_y</B>). They are not
 *  suppose to change during the action.
 *
 *  \param [in]  w_current  The TOPLEVEL object.
 *  \param [in]  o_current  Picture OBJECT to check.
 *  \param [in]  x          (unused)
 *  \param [in]  y          (unused)
 *  \param [out] whichone   Which coordinate to check.
 */
void o_grips_start_picture(TOPLEVEL *w_current, OBJECT *o_current,
			   int x, int y, int whichone)
{
  w_current->last_drawb_mode = -1;
	
  /* erase the picture before */
  o_picture_erase(w_current, o_current);
  w_current->current_pixbuf = o_current->picture->original_picture;
  w_current->pixbuf_filename = o_current->picture->filename;
  w_current->pixbuf_wh_ratio = o_current->picture->ratio;

  /* (last_x,last_y)    is the selected corner */
  /* (start_x, start_y) is the opposite corner */
  switch(whichone) {
    case PICTURE_UPPER_LEFT: 
      WORLDtoSCREEN( w_current, o_current->picture->upper_x, o_current->picture->upper_y,
                     &w_current->last_x, &w_current->last_y );
      WORLDtoSCREEN( w_current, o_current->picture->lower_x, o_current->picture->lower_y,
                     &w_current->start_x, &w_current->start_y );
      break;
    case PICTURE_LOWER_RIGHT: 
      WORLDtoSCREEN( w_current, o_current->picture->lower_x, o_current->picture->lower_y,
                     &w_current->last_x, &w_current->last_y );
      WORLDtoSCREEN( w_current, o_current->picture->upper_x, o_current->picture->upper_y,
                     &w_current->start_x, &w_current->start_y );
      break;
    case PICTURE_UPPER_RIGHT: 
      WORLDtoSCREEN( w_current, o_current->picture->lower_x, o_current->picture->upper_y,
                     &w_current->last_x, &w_current->last_y );
      WORLDtoSCREEN( w_current, o_current->picture->upper_x, o_current->picture->lower_y,
                     &w_current->start_x, &w_current->start_y );
      break;
    case PICTURE_LOWER_LEFT: 
      WORLDtoSCREEN( w_current, o_current->picture->upper_x, o_current->picture->lower_y,
                     &w_current->last_x, &w_current->last_y );
      WORLDtoSCREEN( w_current, o_current->picture->lower_x, o_current->picture->upper_y,
                     &w_current->start_x, &w_current->start_y );
      break;
    default:
      return; /* error */
  }

  /* draw the first temporary picture */
  o_picture_rubberbox_xor(w_current);

}

/*! \brief Initialize grip motion process for a circle.
 *  \par Function Description
 *  This function initializes the grip motion process for a circle.
 *  From the <B>o_current</B> pointed object, it stores into the TOPLEVEL
 *  structure the coordinate of the center and the radius. These variables
 *  are used in the grip process.
 *
 *  The function first erases the grips.
 *
 *  The coordinates of the center are put in
 *  (<B>w_current->start_x</B>,<B>w_current->start_y</B>). They are not suppose
 *  to change during the action.
 *
 *  The coordinates of the point on the circle to the right of the center
 *  go in (<B>w_current->last_x</B>,<B>w_current->last_y</B>).
 *
 *  \param [in]  w_current  The TOPLEVEL object.
 *  \param [in]  o_current  Circle OBJECT to check.
 *  \param [in]  x          (unused)
 *  \param [in]  y          (unused)
 *  \param [out] whichone   Which coordinate to check.
 */
void o_grips_start_circle(TOPLEVEL *w_current, OBJECT *o_current,
			  int x, int y, int whichone)
{
  w_current->last_drawb_mode = -1;
  
  /* erase the circle before */
  o_circle_erase(w_current, o_current);
  
  /* describe the circle with TOPLEVEL variables */
  /* (start_x, start_y) is the center of the circle */
  WORLDtoSCREEN( w_current, o_current->circle->center_x, o_current->circle->center_y,
                 &w_current->start_x, &w_current->start_y );
  /* (last_x,last_y)    is the point on circle on the right of center */
  WORLDtoSCREEN( w_current, o_current->circle->center_x + o_current->circle->radius, o_current->circle->center_y,
                 &w_current->last_x, &w_current->last_y );
  /* distance           is the radius of the circle */
  w_current->distance = SCREENabs( w_current, o_current->circle->radius );
  
  /* draw the first temporary circle */
  o_circle_rubbercircle_xor(w_current);

}
	
/*! \brief Initialize grip motion process for a line.
 *  This function starts the move of one of the two grips of the line
 *  object <B>o_current</B>.
 *  The line and its grips are first erased. The move of the grips is
 *  materializd with a temporary line in selection color drawn over the
 *  sheet with an xor-function.
 *
 *  During the move of the grip, the line is described by
 *  (<B>w_current->start_x</B>,<B>w_current->start_y</B>) and
 *  (<B>w_current->last_x</B>,<B>w_current->last_y</B>).
 *
 *  The line end that corresponds to the moving grip is in
 *  (<B>w_current->last_x</B>,<B>w_current->last_y</B>).
 *
 *  \param [in]  w_current  The TOPLEVEL object.
 *  \param [in]  o_current  Line OBJECT to check.
 *  \param [in]  x          (unused)
 *  \param [in]  y          (unused)
 *  \param [out] whichone   Which coordinate to check.
 */
void o_grips_start_line(TOPLEVEL *w_current, OBJECT *o_current,
			int x, int y, int whichone)
{
  w_current->last_drawb_mode = -1;
  
  /* erase the line before */
  o_line_erase(w_current, o_current);
  
  /* describe the line with TOPLEVEL variables */
  WORLDtoSCREEN( w_current, o_current->line->x[whichone], o_current->line->y[whichone],
                 &w_current->last_x, &w_current->last_y );
  WORLDtoSCREEN( w_current, o_current->line->x[!whichone], o_current->line->y[!whichone],
                 &w_current->start_x, &w_current->start_y );
  
  /* draw the first temporary line */
  o_line_rubberline_xor(w_current);
}

/*! \brief Modify previously selected object according to mouse position.
 *  \par Function Description
 *  This function modify the previously selected
 *  object according to the mouse position in <B>x</B> and <B>y</B>.
 *  The grip under modification is updated and the temporary object displayed.
 *
 *  The object under modification is <B>object_changing</B> and the grip
 *  concerned is <B>whichone_changing</B>.
 *
 *  Depending on the object type, a specific function is used.
 *  It erases the temporary object, updates its internal representation,
 *  and draws it again.
 *
 *  \param [in] w_current  The TOPLEVEL object.
 *  \param [in] x          Current x coordinate of pointer in screen units.
 *  \param [in] y          Current y coordinate of pointer in screen units.
 */
void o_grips_motion(TOPLEVEL *w_current, int x, int y)
{
	
  if (w_current->inside_action == 0) {
    o_redraw(w_current, w_current->page_current->object_head, TRUE);
    return;
  }

  /* no object changing */
  if (object_changing == NULL) {
    /* stop grip process */
    o_redraw(w_current, w_current->page_current->object_head, TRUE);
    return;
  }
	
  switch(object_changing->type) {
    case(OBJ_ARC):
    /* erase, update and draw an arc */
    o_grips_motion_arc(w_current, x, y, whichone_changing);
    break;

    case(OBJ_BOX):
    /* erase, update and draw a box */
    o_grips_motion_box(w_current, x, y, whichone_changing);
    break;
		
    case(OBJ_PICTURE):
    /* erase, update and draw a box */
    o_grips_motion_picture(w_current, x, y, whichone_changing);
    break;
		
    case(OBJ_CIRCLE):
    /* erase, update and draw a circle */
    o_grips_motion_circle(w_current, x, y, whichone_changing);
    break;
		
    case(OBJ_LINE):
    case(OBJ_NET):
    case(OBJ_PIN):
    case(OBJ_BUS):
    /* erase, update and draw a line */
    /* same for net, pin and bus as they share the same internal rep. */
    o_grips_motion_line(w_current, x, y, whichone_changing);
    break;

    default:
    return; /* error condition */
  }
	
}

/*! \brief Modify previously selected arc according to mouse position.
 *  \par Function Description
 *  This function is the refreshing part of the grip motion process.
 *  It is called whenever the position of the pointer is changed,
 *  therefore requiring the TOPLEVEL variables to be updated.
 *  Depending on the grip selected and moved, the temporary TOPLEVEL
 *  variables are changed according to the current position of the pointer.
 *
 *  If the grip at the center of the arc has been moved - modifying the
 *  radius of the arc -, the <B>w_current->distance</B> field is updated.
 *  To increase the radius of the arc, the user must drag the grip to the
 *  right of the center. To decrease the radius of the arc, the user must
 *  drag the grip to the left of the center. Negative radius can not be
 *  obtained.
 *
 *  If one of the end of arc grip has been moved - modifying the arc
 *  describing the arc -, the <B>w_current->start_x</B> or
 *  <B>w_current->start_y</B> are updated according to which of the grip
 *  has been selected.
 *
 *  \param [in] w_current  The TOPLEVEL object.
 *  \param [in] x          Current x coordinate of pointer in screen units.
 *  \param [in] y          Current y coordinate of pointer in screen units.
 *  \param [in] whichone   Which grip to start motion with.
 */
void o_grips_motion_arc(TOPLEVEL *w_current, int x, int y, int whichone)
{
  o_arc_rubberarc(w_current, x, y, whichone);
}

/*! \brief Modify previously selected box according to mouse position.
 *  \par Function Description
 *  This function is the refreshing part of the grip motion process. It is
 *  called whenever the position of the pointer is changed, therefore
 *  requiring the TOPLEVEL variables to be updated.
 *  Depending on the grip selected and moved, the temporary TOPLEVEL
 *  variables are changed according to the current position of the pointer
 *  and the modifications temporary drawn.
 *
 *  This function only makes a call to #o_box_rubberbox() that updates
 *  the TOPLEVEL variables, erase the previous temporary box and draw the
 *  new temporary box.
 *
 *  \param [in] w_current  The TOPLEVEL object.
 *  \param [in] x          Current x coordinate of pointer in screen units.
 *  \param [in] y          Current y coordinate of pointer in screen units.
 *  \param [in] whichone   Which grip to start motion with.
 */
void o_grips_motion_box(TOPLEVEL *w_current, int x, int y, int whichone)
{
  /* erase, update and draw the temporary box */
  o_box_rubberbox(w_current, x, y);
}

/*! \brief Modify previously selected picture according to mouse position.
 *  \par Function Description
 *  This function is the refreshing part of the grip motion process. It is
 *  called whenever the position of the pointer is changed, therefore
 *  requiring the TOPLEVEL variables to be updated.
 *  Depending on the grip selected and moved, the temporary TOPLEVEL
 *  variables are changed according to the current position of the pointer
 *  and the modifications temporary drawn.
 *
 *  This function only makes a call to #o_picture_rubberbox() that
 *  updates the TOPLEVEL variables, erase the previous temporary picture
 *  and draw the new temporary picture.
 *
 *  \param [in] w_current  The TOPLEVEL object.
 *  \param [in] x          Current x coordinate of pointer in screen units.
 *  \param [in] y          Current y coordinate of pointer in screen units.
 *  \param [in] whichone   Which grip to start motion with.
 */
void o_grips_motion_picture(TOPLEVEL *w_current, int x, int y, int whichone)
{
  /* erase, update and draw the temporary picture */
  o_picture_rubberbox(w_current, x, y);
}

/*! \brief Modify previously selected circle according to mouse position.
 *  \par Function Description
 *  This function is the refreshing part of the grip motion process. It is
 *  called whenever the position of the pointer is changed, therefore
 *  requiring the TOPLEVEL variables to be updated.
 *  Depending on the grip selected and moved, the temporary TOPLEVEL
 *  variables are changed according to the current position of the pointer
 *  and the modifications temporary drawn.
 *
 *  This function only makes a call to #o_circle_rubbercircle() that updates
 *  the TOPLEVEL variables, erase the previous temporary circle and draw
 *  the new temporary circle.
 *
 *  \param [in] w_current  The TOPLEVEL object.
 *  \param [in] x          Current x coordinate of pointer in screen units.
 *  \param [in] y          Current y coordinate of pointer in screen units.
 *  \param [in] whichone   Which grip to start motion with.
 */
void o_grips_motion_circle(TOPLEVEL *w_current, int x, int y, int whichone)
{
	/* erase, update and draw the temporary circle */
	o_circle_rubbercircle(w_current, x, y);

}

/*! \brief Modify previously selected line according to mouse position.
 *  \par Function Description
 *  This function is called during the move of the grip to update the
 *  temporary line drawn under the mouse pointer.
 *  The current position of the mouse is in <B>x</B> and <B>y</B> in screen coords.
 *
 *  \param [in] w_current  The TOPLEVEL object.
 *  \param [in] x          Current x coordinate of pointer in screen units.
 *  \param [in] y          Current y coordinate of pointer in screen units.
 *  \param [in] whichone   Which grip to start motion with.
 */
void o_grips_motion_line(TOPLEVEL *w_current, int x, int y, int whichone)
{
  /* erase, update and draw the temporary line */
  o_line_rubberline(w_current, x, y);
	
}

/*! \brief End process of modifying object with grip.
 *  \par Function Description
 *  This function ends the process of modifying a parameter of an object
 *  with a grip.
 *  The temporary representation of the object is erased, the object is
 *  modified and finally drawn.
 *
 *  The object under modification is <B>object_changing</B> and the grip
 *  concerned is <B>whichone_changing</B>.
 *
 *  Depending on the object type, a specific function is used. It erases
 *  the temporary object, updates the object and draws the modified object
 *  normally.
 *
 *  \param [in,out] w_current  The TOPLEVEL object.
 */
void o_grips_end(TOPLEVEL *w_current)
{
  OBJECT *object=NULL;
  int x, y;
  GList *other_objects = NULL;
  GList *connected_objects = NULL;
  int size;
	
  object = object_changing;

  if (!object) {
    /* actually this is an error condition hack */
    w_current->inside_action = 0;
    i_set_state(w_current, SELECT);
    return;
  }

  switch(object->type) {
    
    case(OBJ_ARC):
    /* modify an arc object */
    o_grips_end_arc(w_current, object, whichone_changing);
    break;
	  
    case(OBJ_BOX):
    /* modify a box object */
    o_grips_end_box(w_current, object, whichone_changing);
    break;
	  
    case(OBJ_PICTURE):
    /* modify a picture object */
    o_grips_end_picture(w_current, object, whichone_changing);
    break;
	  
    case(OBJ_CIRCLE):
    /* modify a circle object */
    o_grips_end_circle(w_current, object, whichone_changing);
    break;
	  
    case(OBJ_LINE):
    /* modify a line object */
    o_grips_end_line(w_current, object, whichone_changing);
    break;

    case(OBJ_NET):
    /* don't allow zero length nets / lines / pins
     * this ends the net drawing behavior 
     * we want this? hack */
    if ((w_current->start_x == w_current->last_x) &&
        (w_current->start_y == w_current->last_y)) {
      w_current->start_x = (-1);
      w_current->start_y = (-1);
      w_current->last_x = (-1);
      w_current->last_y = (-1);
      w_current->inside_action=0;
      i_set_state(w_current, SELECT);
      o_net_eraserubber(w_current);
      o_redraw_single(w_current, object);
      i_update_toolbar(w_current);
      return;
    }
	  
	  
    SCREENtoWORLD(w_current, 
                  w_current->last_x, 
                  w_current->last_y, &x, &y);
	  
    x = snap_grid(w_current, x);
    y = snap_grid(w_current, y);
	  
    o_cue_undraw(w_current, object);
    o_net_erase(w_current, object);
    /* erase xor line */
    gdk_gc_set_foreground(w_current->xor_gc,
                          x_get_darkcolor(w_current->select_color));
    gdk_draw_line(w_current->window, w_current->xor_gc,
                  w_current->start_x, w_current->start_y,
                  w_current->last_x, w_current->last_y);
    o_line_erase_grips(w_current, object);
	  
    other_objects = s_conn_return_others(other_objects, object);
    s_conn_remove(w_current, object);
	  
    o_net_modify(w_current, object, x, y, whichone_changing);

    s_conn_update_object(w_current, object);
	  
    /* get the other connected objects and redraw them */
    connected_objects = s_conn_return_others(connected_objects,
                                             object);
    
    /* add bus rippers if necessary */
    if (o_net_add_busrippers(w_current, object, connected_objects)) {
      
      o_net_erase(w_current, object);
      /*o_line_erase_grips(w_current, object); */
      
      if (w_current->net_style == THICK ) {
        size = SCREENabs(w_current, 10);
        
        if (size < 0)
          size=0;
          
          gdk_gc_set_line_attributes(w_current->gc, size, 
                                     GDK_LINE_SOLID,
                                     GDK_CAP_BUTT,
                                     GDK_JOIN_MITER);
        }
      
      gdk_gc_set_foreground(w_current->gc,
                          x_get_color(w_current->background_color));
      gdk_draw_line(w_current->window, w_current->gc,
                    w_current->start_x, w_current->start_y,
                    w_current->last_x, w_current->last_y);

      o_cue_undraw(w_current, object);
      o_net_draw(w_current, object);
      o_cue_draw_single(w_current, object);
      
      if (w_current->net_style == THICK ) {
        gdk_gc_set_line_attributes(w_current->gc, 0, 
                                   GDK_LINE_SOLID,
                                   GDK_CAP_NOT_LAST,
                                   GDK_JOIN_MITER);
      }
    }

    /* draw the object objects */
    o_cue_undraw_list(w_current, other_objects);
    o_cue_draw_list(w_current, other_objects);
    
    o_redraw_single(w_current, object);

    if (connected_objects) {
      g_list_free(connected_objects);
      connected_objects = NULL;
    }

    /* get the other connected objects and redraw them */
    connected_objects = s_conn_return_others(connected_objects,
                                             object);
    
    o_cue_undraw_list(w_current, connected_objects);
    o_cue_draw_list(w_current, connected_objects);
    /* finally draw this objects cues */
    o_cue_draw_single(w_current, object);
    break;

    case(OBJ_PIN):
    /* don't allow zero length nets / lines / pins
     * this ends the net drawing behavior 
     * we want this? hack */
    if ((w_current->start_x == w_current->last_x) &&
        (w_current->start_y == w_current->last_y)) {
      w_current->start_x = (-1);
      w_current->start_y = (-1);
      w_current->last_x = (-1);
      w_current->last_y = (-1);
      o_redraw_single(w_current, object);
      w_current->inside_action=0;
      i_set_state(w_current, SELECT);
      i_update_toolbar(w_current);
      return;
    }
	  
	  
    SCREENtoWORLD(w_current, 
                  w_current->last_x, 
                  w_current->last_y, &x, &y);
	  
    x = snap_grid(w_current, x);
    y = snap_grid(w_current, y);
	  
    o_cue_undraw(w_current, object);
    o_pin_erase(w_current, object);
    /* erase xor line */
    gdk_gc_set_foreground(w_current->xor_gc,
                          x_get_darkcolor(w_current->select_color));
    gdk_draw_line(w_current->window, w_current->xor_gc,
                  w_current->start_x, w_current->start_y,
                  w_current->last_x, w_current->last_y);
    o_line_erase_grips(w_current, object);
	  
    other_objects = s_conn_return_others(other_objects, object);
    s_conn_remove(w_current, object);
	  
    o_pin_modify(w_current, object, x, y, 
                 whichone_changing);
    s_conn_update_object(w_current, object);
    o_redraw_single(w_current, object);
	  
    /* draw the object objects */
    o_cue_undraw_list(w_current, other_objects);
    o_cue_draw_list(w_current, other_objects);
	  
    /* get the other connected objects and redraw them */
    connected_objects = s_conn_return_others(connected_objects,
                                             object);
    o_cue_undraw_list(w_current, connected_objects);
    o_cue_draw_list(w_current, connected_objects);
	  
    /* finally draw this objects cues */
    o_cue_draw_single(w_current, object);
    break;
	  
    case(OBJ_BUS):
    /* don't allow zero length nets / lines / pins
     * this ends the net drawing behavior 
     * we want this? hack */
    if ((w_current->start_x == w_current->last_x) &&
        (w_current->start_y == w_current->last_y)) {
      w_current->start_x = (-1);
      w_current->start_y = (-1);
      w_current->last_x = (-1);
      w_current->last_y = (-1);
      o_net_eraserubber(w_current);
      o_redraw_single(w_current, object);
      w_current->inside_action=0;
      i_set_state(w_current, SELECT);
      i_update_toolbar(w_current);
      return;
    }
	  
    SCREENtoWORLD(w_current, 
                  w_current->last_x, 
                  w_current->last_y, &x, &y);
	  
    x = snap_grid(w_current, x);
    y = snap_grid(w_current, y);
	  
    o_cue_undraw(w_current, object);
    o_bus_erase(w_current, object);
    /* erase xor line */
    gdk_gc_set_foreground(w_current->xor_gc,
                          x_get_darkcolor(w_current->select_color));
    gdk_draw_line(w_current->window, w_current->xor_gc,
                  w_current->start_x, w_current->start_y,
                  w_current->last_x, w_current->last_y);
    o_line_erase_grips(w_current, object);
	  
    other_objects = s_conn_return_others(other_objects, object);
    s_conn_remove(w_current, object);
	  
    o_bus_modify(w_current, object, x, y, 
                 whichone_changing);
    s_conn_update_object(w_current, object);
    o_redraw_single(w_current, object);
	  
    /* draw the object objects */
    o_cue_undraw_list(w_current, other_objects);
    o_cue_draw_list(w_current, other_objects);
	  
    /* get the other connected objects and redraw them */
    connected_objects = s_conn_return_others(connected_objects,
                                             object);
    o_cue_undraw_list(w_current, connected_objects);
    o_cue_draw_list(w_current, connected_objects);
	  
    /* finally draw this objects cues */
    o_cue_draw_single(w_current, object);
    break;
	  
    default:
    return;
  }

			
  w_current->page_current->CHANGED=1;
  
  g_list_free(other_objects);
  other_objects = NULL;
  g_list_free(connected_objects);
  connected_objects = NULL;

  /* reset global variables */
  whichone_changing = -1;
  object_changing = NULL;
  
  o_undo_savestate(w_current, UNDO_ALL);
}

/*! \brief End process of modifying arc object with grip.
 *  \par Function Description
 *  This function ends the grips process specific to an arc object. It erases
 *  the old arc and write back to the object the new parameters of the arc.
 *  Depending on the grip selected and moved, the right fields are updated.
 *  The function handles the conversion from screen unit to world unit before
 *  updating and redrawing.
 *
 *  If the grip at the center of the arc has been moved - modifying the radius
 *  of the arc -, the new radius is calculated expressed in world unit
 *  (the center is unchanged). It is updated with the function #o_arc_modify().
 *
 *  If one of the end of arc grip has been moved - modifying one of the
 *  angles describing the arc -, this angle is updated with the
 *  #o_arc_modify() function.
 *
 *  \param [in] w_current  The TOPLEVEL object.
 *  \param [in] o_current  Arc OBJECT to end modification on.
 *  \param [in] whichone   Which grip is pointed to.
 */
void o_grips_end_arc(TOPLEVEL *w_current, OBJECT *o_current, int whichone)
{
  int arg1, arg2;

  /* erase the temporary arc */
  o_arc_rubberarc_xor(w_current);

  /* determination of the parameters to give to o_arc_modify() */
  switch(whichone) {
    case ARC_RADIUS:
      /* convert the radius in world coords */
      arg1 = WORLDabs(w_current, w_current->distance);
      /* second parameter is not used */
      arg2 = -1;
      break;
		  
    case ARC_START_ANGLE:
      /* get the start angle from w_current */
      arg1 = w_current->loc_x;
      /* second parameter is not used */
      arg2 = -1;
      break;

    case ARC_END_ANGLE:
      /* get the end angle from w_current */
      arg1 = w_current->loc_y;
      /* second parameter is not used */
      arg2 = -1;
      break;

    default:
      return;
  }

  /* modify the arc with the parameters determined above */
  o_arc_modify(w_current, o_current, arg1, arg2, whichone);

  /* display the new arc */
  o_redraw_single(w_current, o_current);

}

/*! \todo Finish function documentation!!!
 *  \brief End process of modifying box object with grip.
 *  \par Function Description
 * 
 *  \param [in] w_current  The TOPLEVEL object.
 *  \param [in] o_current  Box OBJECT to end modification on.
 *  \param [in] whichone   Which grip is pointed to.
 */
void o_grips_end_box(TOPLEVEL *w_current, OBJECT *o_current, int whichone)
{
  int box_width, box_height;
  int x, y;
  
  box_width  = GET_BOX_WIDTH (w_current);
  box_height = GET_BOX_HEIGHT(w_current);
  
  /* don't allow zero width/height boxes
   * this ends the box drawing behavior 
   * we want this? hack */
  if ((box_width == 0) && (box_height == 0)) {
    w_current->start_x = (-1);
    w_current->start_y = (-1);
    w_current->last_x  = (-1);
    w_current->last_y  = (-1);
    
    w_current->inside_action=0;
    i_set_state(w_current, SELECT);
    
    o_redraw_single(w_current, o_current);
    i_update_toolbar(w_current);
    
    return;
  }

  SCREENtoWORLD(w_current, 
		w_current->last_x, w_current->last_y,
		&x, &y);
  x = snap_grid(w_current, x);
  y = snap_grid(w_current, y);
  
  o_box_modify(w_current, o_current, x, y, whichone);
  
  /* erase the temporary box */
  o_box_rubberbox_xor(w_current);
  
  /* draw the modified box */
  o_redraw_single(w_current, o_current);
}

/*! \todo Finish function documentation!!!
 *  \brief End process of modifying picture object with grip.
 *  \par Function Description
 * 
 *  \param [in] w_current  The TOPLEVEL object.
 *  \param [in] o_current  Picture OBJECT to end modification on.
 *  \param [in] whichone   Which grip is pointed to.
 */
void o_grips_end_picture(TOPLEVEL *w_current, OBJECT *o_current, int whichone)
{
  int picture_width, picture_height;
  int x, y;
  
  picture_width  = GET_PICTURE_WIDTH (w_current);
  picture_height = GET_PICTURE_HEIGHT(w_current);

  /* don't allow zero width/height picturees
   * this ends the picture drawing behavior 
   * we want this? hack */
  if ((picture_width == 0) && (picture_height == 0)) {
    w_current->start_x = (-1);
    w_current->start_y = (-1);
    w_current->last_x  = (-1);
    w_current->last_y  = (-1);
    
    w_current->inside_action=0;
    i_set_state(w_current, SELECT);
    
    o_redraw_single(w_current, o_current);
    i_update_toolbar(w_current);
    
    return;
  }
  
  SCREENtoWORLD(w_current, 
		w_current->last_x, w_current->last_y,
		&x, &y);
  x = snap_grid(w_current, x);
  y = snap_grid(w_current, y);
  
  o_picture_modify(w_current, o_current, x, y, whichone);
  
  /* erase the temporary picture */
  o_picture_rubberbox_xor(w_current);
  
  /* draw the modified picture */
  o_redraw_single(w_current, o_current);
  
  w_current->current_pixbuf = NULL;
  w_current->pixbuf_filename = NULL;
  w_current->pixbuf_wh_ratio = 0;
}

/*! \brief End process of modifying circle object with grip.
 *  \par Function Description
 *  This function ends the process of modifying the radius of the circle
 *  object <B>*o_current</B>.
 *  The modified circle is finally normally drawn.
 *
 *  A circle with a null radius is not allowed. In this case, the process
 *  is stopped and the circle is left unchanged.
 *
 *  The last value of the radius is in <B>w_current->distance</B> in screen units.
 *
 *  \param [in] w_current  The TOPLEVEL object.
 *  \param [in] o_current  Circle OBJECT to end modification on.
 *  \param [in] whichone   Which grip is pointed to.
 */
void o_grips_end_circle(TOPLEVEL *w_current, OBJECT *o_current, int whichone)
{
  int radius;
  
  /* erase the temporary circle */
  o_circle_rubbercircle_xor(w_current);
  
  /* don't allow zero radius circles
   * this ends the circle drawing behavior 
   * we want this? hack */
  if ((w_current->start_x == w_current->last_x) &&
      (w_current->start_y == w_current->last_y)) {
    w_current->start_x = (-1);
    w_current->start_y = (-1);
    w_current->last_x  = (-1);
    w_current->last_y  = (-1);
    
    /* return to select mode */
    w_current->inside_action = 0;
    i_set_state(w_current, SELECT);
    i_update_toolbar(w_current);
    
    o_redraw_single(w_current, o_current);
    return;
  }
  
  /* convert the radius in world unit */
  radius = WORLDabs(w_current, w_current->distance);
  
  /* modify the radius of the circle */
  o_circle_modify(w_current, o_current, radius, -1, CIRCLE_RADIUS);
  
  /* display the new circle */
  o_redraw_single(w_current, o_current); 
}

/*! \brief End process of modifying line object with grip.
 *  \par Function Description
 *  This function ends the process of modifying one end of the line
 *  object <B>*o_current</B>.
 *  This end is identified by <B>whichone</B>. The line object is modified
 *  according to the <B>whichone</B> parameter and the last position of the
 *  line end.
 *  The modified line is finally normally drawn.
 *
 *  A line with a null width, i.e. when both ends are identical, is not
 *  allowed. In this case, the process is stopped and the line unchanged.
 *
 *  \param [in] w_current  The TOPLEVEL object.
 *  \param [in] o_current  Circle OBJECT to end modification on.
 *  \param [in] whichone   Which grip is pointed to.
 */
void o_grips_end_line(TOPLEVEL *w_current, OBJECT *o_current, int whichone)
{
  int x, y;
  
  /* erase the temporary line */
  o_line_rubberline_xor(w_current);
  
  /* don't allow zero length nets / lines / pins
   * this ends the net drawing behavior 
   * we want this? hack */
  if ((w_current->start_x == w_current->last_x) &&
      (w_current->start_y == w_current->last_y)) {
    w_current->start_x = (-1);
    w_current->start_y = (-1);
    w_current->last_x  = (-1);
    w_current->last_y  = (-1);
    
    /* return to select mode */
    w_current->inside_action=0;
    i_set_state(w_current, SELECT);
    i_update_toolbar(w_current);
    
    o_redraw_single(w_current, o_current);
    return;
  }
  
  /* convert the line end coords in world unit */
  SCREENtoWORLD(w_current, 
		w_current->last_x, w_current->last_y,
		&x, &y);
  x = snap_grid(w_current, x);
  y = snap_grid(w_current, y);
  
  /* modify the right line end according to whichone */
  o_line_modify(w_current, o_current, x, y, whichone);
  
  /* display the new line */
  o_redraw_single(w_current, o_current);
}
	
/*! \brief Get half the width and height of grip in screen units.
 *  \par Function Description
 *  According to the current zoom level, the function returns half the width
 *  and height of a grip in screen units.
 *
 *  <B>GRIP_SIZE1</B> and <B>GRIP_SIZE2</B> and <B>GRIP_SIZE3</B> are macros defined
 *  in libgeda #defines.h. They are the half width/height of a grip in
 *  world unit for a determined range of zoom factors.
 *
 *  \param [in] w_current  The TOPLEVEL object.
 *  \return Half grip size in screen units.
 */
int o_grips_size(TOPLEVEL *w_current)
{
  int factor, size;
  
  factor = (int) w_current->page_current->to_world_x_constant;
  if (factor > SMALL_ZOOMFACTOR1) {
    /* big zoom factor : small size converted to screen unit */
    size = SCREENabs(w_current, GRIP_SIZE1);
  } else if (factor > SMALL_ZOOMFACTOR2) {
    /* medium zoom factor : medium size converted to screen unit */
    size = SCREENabs(w_current, GRIP_SIZE2);
  } else {
    /* small zoom factor : big size converted to screen unit */
    size = SCREENabs(w_current, GRIP_SIZE3);
  }
  
  return size;
}

/*! \brief Draw grip centered at <B>x</B>, <B>y</B>
 *  \par Function Description
 *  This function draws a grip centered at (<B>x</B>,<B>y</B>). Its color is
 *  either the selection color or the overriding color from
 *  <B>w_current->override_color</B>.
 *
 *  The size of the grip depends on the current zoom factor.
 *
 *  <B>x</B> and <B>y</B> are in screen unit.
 *
 *  \param [in] w_current  The TOPLEVEL object.
 *  \param [in] x          Center x screen coordinate for drawing grip.
 *  \param [in] y          Center y screen coordinate for drawing grip.
 */
void o_grips_draw(TOPLEVEL *w_current, int x, int y)
{
  GdkColor *color;
  int size, x2size;
  
  /*
   * Depending on the current zoom level, the size of the grip is
   * determined. <B>size</B> is half the width and height of the grip
   * and <B>x2size</B> is the full width and height of the grip.
   */
  /* size is half the width of grip */
  size = o_grips_size(w_current);
  /* x2size is full width */
  x2size = 2 * size;

  /*
   * The grip can be displayed or erased : if <B>w_current->override_color</B>
   * is not set the grip is drawn with the selection color ; if
   * <B>w_current->override_color</B> is set then the color it refers it
   * is used. This way the grip can be erased if this color is the
   * background color.
   */
  if (w_current->override_color != -1 ) {
    /* override : use the override_color instead */
    color = x_get_color(w_current->override_color);
  } else {
    /* use the normal selection color */
    color = x_get_color(w_current->select_color);
  }
  /* set the color for the grip */
  gdk_gc_set_foreground(w_current->gc, color);
 
  /* set the line options for grip : solid, 1 pix wide */
  gdk_gc_set_line_attributes(w_current->gc, 0, GDK_LINE_SOLID,
			     GDK_CAP_BUTT, GDK_JOIN_MITER);
	
  /*
   * A grip is a hollow square centered at (<B>x</B>,<B>y</B>) with a
   * width/height of <B>x2size</B>.
   */
  if (w_current->DONT_REDRAW == 0) {
    /* draw the grip in window */
    gdk_draw_rectangle(w_current->window, w_current->gc, FALSE,
		       x - size, y - size, x2size, x2size);
    /* draw the grip in backingstore */
    gdk_draw_rectangle(w_current->backingstore, w_current->gc, FALSE,
		       x - size, y - size, x2size, x2size);
  }
}

/*! \brief Erase grip centered at <B>x</B>,<B>y</B>
 *  \par Function Description
 *  This function erases a grip centered at (<B>x</B>,<B>y</B>). 
 *  The size of the grip depends on the current zoom factor.
 *
 *  The grip is erased by drawing with the background color over the
 *  visible grip.
 *
 *  \param [in] w_current  The TOPLEVEL object.
 *  \param [in] x          Center x screen coordinate for drawing grip.
 *  \param [in] y          Center y screen coordinate for drawing grip.
 */
void o_grips_erase(TOPLEVEL *w_current, int x, int y)
{
  /* set overriding color */
  w_current->override_color = w_current->background_color;
  /* draw a grip with backgound color : erase grip */
  o_grips_draw(w_current, x, y);
  /* return to default */
  w_current->override_color = -1;
}
