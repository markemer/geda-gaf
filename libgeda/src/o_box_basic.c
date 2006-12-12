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
#include <math.h>
#include <stdio.h>

#include <gtk/gtk.h>
#include <libguile.h>

#ifdef HAS_LIBGD
#include <gd.h>
#endif

#include "defines.h"
#include "struct.h"
#include "globals.h"
#include "o_types.h"
#include "colors.h"
#include "funcs.h"

#include "../include/prototype.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

/* Kazu on July 16, 1999 - Added these macros to simplify the code */
/*! \brief Returns the box width.
 *  Returns the box width.
 */
#define GET_BOX_WIDTH(w)                        \
        abs((w)->last_x - (w)->start_x)
/*! \brief Returns the box height.
 *  Returns the box height.
 */
#define GET_BOX_HEIGHT(w)                       \
	        abs((w)->last_y - (w)->start_y)
/*! \brief Returns the box left coordinate.
 *  Returns the box left coordinate.
 */
#define GET_BOX_LEFT(w)                         \
	        min((w)->start_x, (w)->last_x);
/*! \brief Returns the box left coordinate.
 *  Returns the box left coordinate.
 */
#define GET_BOX_TOP(w)                          \
		min((w)->start_y, (w)->last_y);

/*! \brief Create a BOX and add it to a list.
 *  \par Function Description
 *  This function creates a new object representing a box.
 *  This object is added to the end of the list <B>object_list</B>
 *  pointed object belongs to.
 *  The box is described by its upper left corner - <B>x1</B>, <B>y1</B> - and
 *  its lower right corner - <B>x2</B>, <B>y2</B>.
 *  The <B>type</B> parameter must be equal to <B>OBJ_BOX</B>. The <B>color</B>
 *  corresponds to the color the box will be drawn with.
 *  The <B>OBJECT</B> structure is allocated with the #s_basic_init_object()
 *  function. The structure describing the box is allocated and initialized
 *  with the parameters given to the function.
 *
 *  Both the line type and the filling type are set to default values : solid
 *  line type with a width of 0, and no filling. It can be changed after
 *  with the #o_set_line_options() and #o_set_fill_options().
 *
 *  The object is added to the end of the list described by the <B>object_list</B>
 *  parameter by the #s_basic_link_object().
 *
 *  \param [in]     w_current    The TOPLEVEL object.
 *  \param [in,out] object_list  OBJECT list to add box to.
 *  \param [in]     type         Box type.
 *  \param [in]     color        Box border color.
 *  \param [in]     x1           Upper x coordinate.
 *  \param [in]     y1           Upper y coordinate.
 *  \param [in]     x2           Lower x coordinate.
 *  \param [in]     y2           Lower y coordinate.
 *  \return A new pointer on the end of the <B>object_list</B>
 */
OBJECT *o_box_add(TOPLEVEL *w_current, OBJECT *object_list,
		  char type, int color,
		  int x1, int y1, int x2, int y2)
{
  OBJECT *new_node;
  BOX *box;

  /* create the object */
  new_node        = s_basic_init_object("box");
  new_node->type  = type;
  new_node->color = color;

  box = (BOX *) g_malloc(sizeof(BOX));
  new_node->box   = box;

  /* describe the box with its upper left and lower right corner */
  box->upper_x = x1;
  box->upper_y = y1;
  box->lower_x = x2;
  box->lower_y = y2;

  /* line type and filling initialized to default */
  o_set_line_options(w_current, new_node,
		     END_NONE, TYPE_SOLID, 0, -1, -1);
  o_set_fill_options(w_current, new_node,
		     FILLING_HOLLOW, -1, -1, -1, -1, -1);

  new_node->draw_func = box_draw_func; 
  new_node->sel_func  = select_func;  

  /* compute the bounding box */
  o_box_recalc(w_current, new_node);

  /* add the object to the list */
  object_list = (OBJECT *) s_basic_link_object(new_node, object_list);

  return(object_list);
}

/*! \brief Copy a box to a list.
 *  \par Function Description
 *  The function #o_box_copy() creates a verbatim copy of the object
 *  pointed by <B>o_current</B> describing a box. The new object is added at
 *  the end of the list, following the <B>list_tail</B> pointed object.
 *
 *  \param [in]      w_current  The TOPLEVEL object.
 *  \param [in,out]  list_tail  OBJECT list to copy to.
 *  \param [in]      o_current  BOX OBJECT to copy.
 *  \return A new pointer on the end of the OBJECT <B>list_tail</B>.
 */
OBJECT *o_box_copy(TOPLEVEL *w_current, OBJECT *list_tail, OBJECT *o_current)
{
  OBJECT *new_obj;
  ATTRIB *a_current;
  int color;
	
  if (o_current->saved_color == -1) {
    color = o_current->color;
  } else {
    color = o_current->saved_color;
  }

  /* 
   * A new box object is added at the end of the object list with
   * #o_box_add(). Values for its fields are default and need to
   * be modified.
   */

  /* create and link a new box object */	
  new_obj = o_box_add(w_current, list_tail,
		      OBJ_BOX, color,
		      0, 0, 0, 0);

  /*
   * The dimensions of the new box are set with the ones of the original box.
   * The two boxes have the same line type and the same filling options.
   *
   * The coordinates and the values in screen unit are computed with
   *  #o_box_recalc().
   */

  /* modifying */
  /* pb20011002 - have to check if o_current is a box object */
  new_obj->box->upper_x = o_current->box->upper_x;
  new_obj->box->upper_y = o_current->box->upper_y;
  new_obj->box->lower_x = o_current->box->lower_x;
  new_obj->box->lower_y = o_current->box->lower_y;
	
  o_set_line_options(w_current, new_obj, o_current->line_end,
		     o_current->line_type, o_current->line_width,
		     o_current->line_length, o_current->line_space);
  o_set_fill_options(w_current, new_obj,
		     o_current->fill_type, o_current->fill_width,
		     o_current->fill_pitch1, o_current->fill_angle1,
		     o_current->fill_pitch2, o_current->fill_angle2);

  o_box_recalc(w_current, new_obj);

  /* new_obj->attribute = 0;*/
  a_current = o_current->attribs;
  if (a_current) {
    while ( a_current ) {
      
      /* head attrib node has prev = NULL */
      if (a_current->prev != NULL) {
	a_current->copied_to = new_obj;
      }
      a_current = a_current->next;
    }
  }

  /* return the new tail of the object list */
  return(new_obj);
} 

/*! \brief Modify a BOX OBJECT's coordinates.
 *  \par Function Description
 *  This function modifies the coordinates of one of the four corner of
 *  the box. The new coordinates of the corner identified by <B>whichone</B>
 *  are given by <B>x</B> and <B>y</B> in world unit.
 *
 *  The coordinates of the corner is modified in the world coordinate system.
 *  Screen coordinates and boundings are then updated.
 *
 *  \param [in]     w_current  The TOPLEVEL object.
 *  \param [in,out] object     BOX OBJECT to be modified.
 *  \param [in]     x          x coordinate.
 *  \param [in]     y          y coordinate.
 *  \param [in]     whichone   coordinate to change.
 *
 *  \note
 *  <B>whichone</B> can take the following values:
 *  <DL>
 *    <DT>*</DT><DD>BOX_UPPER_LEFT
 *    <DT>*</DT><DD>BOX_LOWER_LEFT
 *    <DT>*</DT><DD>BOX_UPPER_RIGHT
 *    <DT>*</DT><DD>BOX_LOWER_RIGHT
 *  </DL>
 *
 *  \par Author's Note:
 *  pb20011002 - rewritten : old one did not used <B>x</B>, <B>y</B> and
 *                           <B>whichone</B>
 */
void o_box_modify(TOPLEVEL *w_current, OBJECT *object, 
		  int x, int y, int whichone)
{
	int tmp;
	
	/* change the position of the selected corner */
	switch(whichone) {
		case BOX_UPPER_LEFT:
			object->box->upper_x = x;
			object->box->upper_y = y;
			break;
			
		case BOX_LOWER_LEFT:
			object->box->upper_x = x;
			object->box->lower_y = y;
			break;
			
		case BOX_UPPER_RIGHT:
			object->box->lower_x = x;
			object->box->upper_y = y;
			break;
			
		case BOX_LOWER_RIGHT:
			object->box->lower_x = x;
			object->box->lower_y = y;
			break;
			
		default:
			return;
	}
	
	/* need to update the upper left and lower right corners */
	if(object->box->upper_x > object->box->lower_x) {
		tmp                  = object->box->upper_x;
		object->box->upper_x = object->box->lower_x;
		object->box->lower_x = tmp;
	}
	
	if(object->box->upper_y < object->box->lower_y) {
		tmp                  = object->box->upper_y;
		object->box->upper_y = object->box->lower_y;
		object->box->lower_y = tmp;
	}
	
	/* recalculate the screen coords and the boundings */
	o_box_recalc(w_current, object);
  
}

/*! \brief Create a box from a character string.
 *  \par Function Description
 *  This function gets the description of a box from the <B>*buf</B> character
 *  string. The new box is then added to the list of object of which
 *  <B>*object_list</B> is the last element before the call.
 *
 *  Depending on <B>*version</B>, the correct file format is considered.
 *  Currently two file format revisions are supported :
 *  <DL>
 *    <DT>*</DT><DD>the file format used until 20000704 release
 *    <DT>*</DT><DD>the file format used for the releases after 2000704.
 *  </DL>
 *
 *  \param [in]     w_current       The TOPLEVEL object.
 *  \param [in,out] object_list     BOX OBJECT list to add new BOX to.
 *  \param [in]     buf             Character string with box description.
 *  \param [in]     release_ver     libgeda release version number.
 *  \param [in]     fileformat_ver  libgeda file format version number.
 *  \return The BOX OBJECT that was created.
 */
OBJECT *o_box_read(TOPLEVEL *w_current, OBJECT *object_list, char buf[],
		   unsigned int release_ver, unsigned int fileformat_ver)
{
  char type; 
  int x1, y1;
  int width, height; 
  int d_x1, d_y1;
  int d_x2, d_y2;
  int color;
  int box_width, box_space, box_length;
  int fill_width, angle1, pitch1, angle2, pitch2;
  int box_end;
  int box_type;
  int box_filling;
  
  if(release_ver <= VERSION_20000704) {

  /*! \note
   *  The old geda file format, i.e. releases 20000704 and older, does not
   *  handle the line type and the filling of the box object. They are set
   *  to default.
   */

    sscanf(buf, "%c %d %d %d %d %d\n",
	   &type, &x1, &y1, &width, &height, &color);

    box_width   = 0;
    box_end     = END_NONE;
    box_type    = TYPE_SOLID;
    box_length  = -1;
    box_space   = -1;
    
    box_filling = FILLING_HOLLOW;		
    fill_width  = 0;
    angle1      = -1;
    pitch1      = -1;
    angle2      = -1;
    pitch2      = -1;
			  
  } else {

    /*! \note
     *  The current line format to describe a box is a space separated list of
     *  characters and numbers in plain ASCII on a single line. The meaning of
     *  each item is described in the file format documentation.
     */
    sscanf(buf, "%c %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
	   &type, &x1, &y1, &width, &height, &color,
	   &box_width, &box_end, &box_type, &box_length, 
	   &box_space, &box_filling,
	   &fill_width, &angle1, &pitch1, &angle2, &pitch2);
  }

  if (width == 0 || height == 0) {
    fprintf(stderr, "Found a zero width/height box [ %c %d %d %d %d %d ]\n",
            type, x1, y1, width, height, color);
    s_log_message("Found a zero width/height box [ %c %d %d %d %d %d ]\n",
                  type, x1, y1, width, height, color);
  }

  if (color < 0 || color > MAX_COLORS) {
    fprintf(stderr, "Found an invalid color [ %s ]\n", buf);
    s_log_message("Found an invalid color [ %s ]\n", buf);
    s_log_message("Setting color to WHITE\n");
    color = WHITE;
  }
  
  /*! \note
   *  A box is internally described by its lower right and upper left corner
   *  whereas the line describe it with the lower left corner and the width
   *  and height.
   *
   *  A new object is allocated, initialized and added to the object list.
   *  Its filling and line type are set according to the values of the field
   *  on the line.
   */

  /* upper left corner of the box */
  d_x1 = x1;
  d_y1 = y1+height; /* move box origin to top left */
  
  /* lower right corner of the box */
  d_x2 = x1+width; /* end points of the box */
  d_y2 = y1;
  
  /* create and add the box to the list */
  object_list = (OBJECT *) o_box_add(w_current, object_list,
				     type, color,
				     d_x1, d_y1, d_x2, d_y2);
  /* set its line options */
  o_set_line_options(w_current, object_list,
		     box_end, box_type, box_width, 
		     box_length, box_space);
  /* set its fill options */
  o_set_fill_options(w_current, object_list,
		     box_filling, fill_width,
		     pitch1, angle1, pitch2, angle2);
  return(object_list);
}

/*! \brief Create a character string representation of a BOX.
 *  \par Function Description
 *  This function formats a string in the buffer <B>*buff</B> to describe the
 *  box object <B>*object</B>.
 *  It follows the post-20000704 release file format that handle the line type
 *  and fill options. 
 *
 *  \param [in] object  The BOX OBJECT to create string from.
 *  \return A pointer to the BOX character string.
 *
 *  \warning
 *  Caller must free returned character string.
 */
char *o_box_save(OBJECT *object)
{
  int x1, y1; 
  int width, height;
  int color;
  int box_width, box_space, box_length;
  int fill_width, angle1, pitch1, angle2, pitch2;
  OBJECT_END box_end;
  OBJECT_TYPE box_type;
  OBJECT_FILLING box_fill;
  char *buf;

  /*! \note
   *  A box is internally represented by its lower right and upper left corner
   *  whereas it is described in the file format as its lower left corner and
   *  its width and height.
   */

  /* calculate the width and height of the box */
  width  = abs(object->box->lower_x - object->box->upper_x); 
  height = abs(object->box->upper_y - object->box->lower_y);

  /* calculate the lower left corner of the box */
  x1 = object->box->upper_x;
  y1 = object->box->upper_y - height; /* move the origin to 0, 0*/

#if DEBUG
  printf("box: %d %d %d %d\n", x1, y1, width, height);
#endif

  /* description of the line type for the outline */
  box_end    = object->line_end;
  box_width  = object->line_width;
  box_type   = object->line_type;
  box_length = object->line_length;
  box_space  = object->line_space;
  
  /* description of the filling of the box */
  box_fill   = object->fill_type;
  fill_width = object->fill_width;
  angle1     = object->fill_angle1;
  pitch1     = object->fill_pitch1;
  angle2     = object->fill_angle2;
  pitch2     = object->fill_pitch2;

  /* Use the right color */
  if (object->saved_color == -1) {
    color = object->color;
  } else {
    color = object->saved_color;
  }


  buf = g_strdup_printf("%c %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
			object->type,
			x1, y1, width, height, color,
			box_width, box_end, box_type, box_length, box_space, 
			box_fill,
			fill_width, angle1, pitch1, angle2, pitch2);
			
  return(buf);
}

/*! \brief Translate a BOX position by a delta.
 *  \par Function Description
 *  This function applies a translation of (<B>dx</B>,<B>dy</B>) to the box
 *  described by <B>*object</B>. <B>dx</B> and <B>dy</B> are in screen unit.
 *
 *  \param [in]     w_current   The TOPLEVEL object.
 *  \param [in]     dx          x distance to move.
 *  \param [in]     dy          y distance to move.
 *  \param [in,out] object      BOX OBJECT to translate.
 */
void o_box_translate(TOPLEVEL *w_current, int dx, int dy, OBJECT *object)
{
  int world_dx, world_dy;

  if (object == NULL) printf("bt NO!\n");

  /* convert the translation vector in world unit */
  world_dx = SCREENabs(w_current, dx);
  world_dy = SCREENabs(w_current, dy);

  /* translate the box */
  o_box_translate_world(w_current, world_dx, world_dy, object);

  /* screen coords and boundings are updated by _translate_world */
  
}

/*! \brief Translate a BOX position in WORLD coordinates by a delta.
 *  \par Function Description
 *  This function applies a translation of (<B>x1</B>,<B>y1</B>) to the box
 *  described by <B>*object</B>. <B>x1</B> and <B>y1</B> are in world unit.
 *
 *  \param [in]     w_current  The TOPLEVEL object.
 *  \param [in]     x1         x distance to move.
 *  \param [in]     y1         y distance to move.
 *  \param [in,out] object     BOX OBJECT to translate.
 */
void o_box_translate_world(TOPLEVEL *w_current, int x1, int y1, OBJECT *object)
{
  if (object == NULL) printf("btw NO!\n");

  /* Do world coords */
  object->box->upper_x = object->box->upper_x + x1;
  object->box->upper_y = object->box->upper_y + y1;
  object->box->lower_x = object->box->lower_x + x1;
  object->box->lower_y = object->box->lower_y + y1;     

  /* recalc the screen coords and the bounding box */
  o_box_recalc(w_current, object);
}

/*! \brief Rotate a BOX OBJECT.
 *  \par Function Description
 *  This function applies a rotation of center (<B>centerx</B>, <B>centery</B>) and
 *  angle <B>angle</B> to the box object <B>*object</B>.
 *  The coordinates of the rotation center are in screen units. 
 *  <B>angle</B> must be a 90 degree multiple. If not, no rotation is applied.
 *
 *  The rotation is made with the #o_box_rotate_world() function that
 *  perform a rotation of angle <B>angle</B> and center (<B>world_centerx</B>,
 *  <B>world_centery</B>) in world unit.
 *
 *  \param [in]     w_current  The TOPLEVEL object.
 *  \param [in]     centerx    Rotation center x coordinate in SCREEN units.
 *  \param [in]     centery    Rotation center y coordinate in SCREEN units.
 *  \param [in]     angle      Rotation angle in degrees (unused).
 *  \param [in,out] object     BOX OBJECT to rotate.
 *
 *  \note
 *  takes in screen coordinates for the centerx,y, and then does the rotate 
 *  in world space
 *  also ignores angle argument... for now, rotate only in 90 degree 
 *  increments
 *  fixed to 90 degrees... it's *not* general now
 */
void o_box_rotate(TOPLEVEL *w_current,
		  int centerx, int centery, int angle,
		  OBJECT *object)
{
  int world_centerx, world_centery;

  /* convert the center of rotation to world unit */
  SCREENtoWORLD(w_current,
				centerx, centery, 
                &world_centerx, &world_centery);  

  /* rotate the box */
  /* the check of the rotation angle is in o_box_rotate_world() */
  o_box_rotate_world(w_current,
					 world_centerx, world_centery, angle,
					 object);

  /* screen coords and boundings are updated by _rotate_world() */
  
}

/*! \brief Rotate BOX OBJECT using WORLD coordinates. 
 *  \par Function Description
 *  The function #o_box_rotate_world() rotate the box described by
 *  <B>*object</B> around the (<B>world_centerx</B>, <B>world_centery</B>) point by
 *  <B>angle</B> degrees.
 *  The center of rotation is in world unit.
 *
 *  \param [in]      w_current      The TOPLEVEL object.
 *  \param [in]      world_centerx  Rotation center x coordinate in WORLD units.
 *  \param [in]      world_centery  Rotation center y coordinate in WORLD units.
 *  \param [in]      angle          Rotation angle in degrees (See note below).
 *  \param [in,out]  object         BOX OBJECT to rotate.
 *
 */
void o_box_rotate_world(TOPLEVEL *w_current, 
			int world_centerx, int world_centery, int angle,
			OBJECT *object)
{
  int newx1, newy1;
  int newx2, newy2;

  /*! \note
   *  Only 90 degree multiple and positive angles are allowed.
   */

  /* angle must be positive */
  if(angle < 0) angle = -angle;
  /* angle must be a 90 multiple or no rotation performed */
  if((angle % 90) != 0) return;

  /*! \note
   *  The center of rotation (<B>world_centerx</B>, <B>world_centery</B>) is
   *  translated to the origin. The rotation of the upper left and lower right
   *  corner are then performed. Finally, the rotated box is translated back
   *  to its previous location.
   */
  /* translate object to origin */
  object->box->upper_x -= world_centerx;
  object->box->upper_y -= world_centery;
  object->box->lower_x -= world_centerx;
  object->box->lower_y -= world_centery;
  
  /* rotate the upper left corner of the box */
  rotate_point_90(object->box->upper_x, object->box->upper_y, angle,
		  &newx1, &newy1);
  
  /* rotate the lower left corner of the box */
  rotate_point_90(object->box->lower_x, object->box->lower_y, angle,
		  &newx2, &newy2);
  
  /* reorder the corners after rotation */
  object->box->upper_x = min(newx1,newx2);
  object->box->upper_y = max(newy1,newy2);
  object->box->lower_x = max(newx1,newx2);
  object->box->lower_y = min(newy1,newy2);
  
  /* translate object back to normal position */
  object->box->upper_x += world_centerx;
  object->box->upper_y += world_centery;
  object->box->lower_x += world_centerx;
  object->box->lower_y += world_centery;
  
  /* recalc boundings and screen coords */
  o_box_recalc(w_current, object);
}

/*! \brief Mirror a BOX.
 *  \par Function Description
 *  This function mirrors the box from the point (<B>centerx</B>,<B>centery</B>) in
 *  screen unit.
 *
 *  The origin of the mirror in screen unit is converted in world unit. The
 *  box is mirrored with the function #o_box_mirror_world() for which the
 *  origin of the mirror must be given in world unit.
 *
 *  \param [in]     w_current  The TOPLEVEL object.
 *  \param [in]     centerx    Origin x coordinate in WORLD units.
 *  \param [in]     centery    Origin y coordinate in WORLD units.
 *  \param [in,out] object     BOX OBJECT to mirror.
 */
void o_box_mirror(TOPLEVEL *w_current,
		  int centerx, int centery,
		  OBJECT *object)
{
  int world_centerx, world_centery;

  /* convert the origin of mirror */
  SCREENtoWORLD(w_current, centerx, centery, 
                &world_centerx,
                &world_centery);  

  /* apply the mirror in world coords */
  o_box_mirror_world(w_current,
		     world_centerx, world_centery,
		     object);
  
  /* screen coords and boundings are updated by _mirror_world() */
  
}

/*! \brief Mirror BOX using WORLD coordinates.
 *  \par Function Description
 *  This function mirrors the box from the point
 *  (<B>world_centerx</B>,<B>world_centery</B>) in world unit.
 *
 *  The box is first translated to the origin, then mirrored and finally
 *  translated back at its previous position.
 *
 *  \param [in]     w_current      The TOPLEVEL object.
 *  \param [in]     world_centerx  Origin x coordinate in WORLD units.
 *  \param [in]     world_centery  Origin y coordinate in WORLD units.
 *  \param [in,out] object         BOX OBJECT to mirror.
 */
void o_box_mirror_world(TOPLEVEL *w_current,
			int world_centerx, int world_centery,
			OBJECT *object)
{
  int newx1, newy1;
  int newx2, newy2;

  /* translate object to origin */
  object->box->upper_x -= world_centerx;
  object->box->upper_y -= world_centery;
  object->box->lower_x -= world_centerx;
  object->box->lower_y -= world_centery;

  /* mirror the corners */
  newx1 = -object->box->upper_x;
  newy1 = object->box->upper_y;
  newx2 = -object->box->lower_x;
  newy2 = object->box->lower_y;

  /* reorder the corners */
  object->box->upper_x = min(newx1,newx2);
  object->box->upper_y = max(newy1,newy2);
  object->box->lower_x = max(newx1,newx2);
  object->box->lower_y = min(newy1,newy2);

  /* translate back in position */
  object->box->upper_x += world_centerx;
  object->box->upper_y += world_centery;
  object->box->lower_x += world_centerx;
  object->box->lower_y += world_centery;

  /* recalc boundings and screen coords */
  o_box_recalc(w_current, object);
  
}

/*! \brief Recalculate BOX coordinates in SCREEN units.
 *  \par Function Description
 *  This function recalculates the screen coords of the <B>o_current</B> pointed
 *  box object from its world coords.
 *
 *  The box coordinates and its bounding are recalculated as well as the
 *  OBJECT specific fields (line width, filling ...).
 *
 *  \param [in] w_current      The TOPLEVEL object.
 *  \param [in,out] o_current  BOX OBJECT to be recalculated.
 */
void o_box_recalc(TOPLEVEL *w_current, OBJECT *o_current)
{
  int left, top, right, bottom;
  int screen_x1, screen_y1;
  int screen_x2, screen_y2;

  if (o_current->box == NULL) {
    return;
  }

  /* update the screen coords of the upper left corner of the box */
  WORLDtoSCREEN(w_current,
		o_current->box->upper_x, o_current->box->upper_y, 
		&screen_x1, &screen_y1);  
  o_current->box->screen_upper_x = screen_x1;
  o_current->box->screen_upper_y = screen_y1;

  /* update the screen coords of the lower right corner of the box */
  WORLDtoSCREEN(w_current,
		o_current->box->lower_x, o_current->box->lower_y, 
		&screen_x2, &screen_y2);  
  o_current->box->screen_lower_x = screen_x2;
  o_current->box->screen_lower_y = screen_y2;

  /* update the bounding box - screen unit */
  get_box_bounds(w_current, o_current->box, &left, &top, &right, &bottom);
  o_current->left   = left;
  o_current->top    = top;
  o_current->right  = right;
  o_current->bottom = bottom;
  
  /* recalc OBJECT specific parameters */
  o_object_recalc(w_current, o_current);
}

/*! \brief Get BOX bounding rectangle.
 *  \par Function Description
 *  This function sets the <B>left</B>, <B>top</B>, <B>right</B> and <B>bottom</B>
 *  parameters to the bounding rectangle of the box object described in
 *  <B>*box</B> in SCREEN units.
 *
 *  \param [in]  w_current  The TOPLEVEL object.
 *  \param [in]  box        BOX OBJECT to read coordinates from.
 *  \param [out] left       Left box coordinate in SCREEN units.
 *  \param [out] top        Top box coordinate in SCREEN units.
 *  \param [out] right      Right box coordinate in SCREEN units.
 *  \param [out] bottom     Bottom box coordinate in SCREEN units.
 */
void get_box_bounds(TOPLEVEL *w_current, BOX *box,
		    int *left, int *top, int *right, int *bottom)
{
  *left   = box->screen_upper_x;
  *top    = box->screen_upper_y;
  *right  = box->screen_lower_x;
  *bottom = box->screen_lower_y;

  /* PB : bounding box has to take into account the width of the line */
  /* PB : but line width is unknown here */
	
  *left   = *left   - 4;
  *top    = *top    - 4;
  *right  = *right  + 4;
  *bottom = *bottom + 4;
}

/*! \brief Get BOX bounding rectangle in WORLD coordinates.
 *  \par Function Description
 *  This function sets the <B>left</B>, <B>top</B>, <B>right</B> and <B>bottom</B>
 *  parameters to the boundings of the box object described in <B>*box</B>
 *  in world units.
 *
 *  \param [in]  w_current  The TOPLEVEL object.
 *  \param [in]  box        BOX OBJECT to read coordinates from.
 *  \param [out] left       Left box coordinate in WORLD units.
 *  \param [out] top        Top box coordinate in WORLD units.
 *  \param [out] right      Right box coordinate in WORLD units.
 *  \param [out] bottom     Bottom box coordinate in WORLD units.
 */
void world_get_box_bounds(TOPLEVEL *w_current, BOX *box,
			  int *left, int *top, int *right, int *bottom)
{
  /* pb20011002 - why using min and max here and not above ? */
  *left   = min(box->upper_x, box->lower_x);
  *top    = min(box->upper_y, box->lower_y);
  *right  = max(box->upper_x, box->lower_x);
  *bottom = max(box->upper_y, box->lower_y);
  
  /* PB : same as above here for width of edges */	

#if DEBUG 
  printf("box: %d %d %d %d\n", *left, *top, *right, *bottom);
#endif
	
}
                 
/*! \brief Print BOX to Postscript document.
 *  \par Function Description
 *  This function prints the box described by the <B>o_current</B>
 *  parameter to a Postscript document. It takes into account its line
 *  type and fill type.
 *  The Postscript document is descibed by the file pointer <B>fp</B>.
 *
 *  The validity of the <B>o_current</B> parameter is verified : a null pointer
 *  causes an error message and a return.
 *
 *  The description of the box is extracted from
 *  the <B>o_current</B> parameter :
 *  the coordinates of the box - upper left corner and width and
 *  height of the box -, its line type, its fill type.
 *
 *  The outline and the inside of the box are successively handled by two
 *  differend sets of functions.
 *  
 *  \param [in] w_current  The TOPLEVEL object.
 *  \param [in] fp         FILE pointer to Postscript document.
 *  \param [in] o_current  BOX OBJECT to write to document.
 *  \param [in] origin_x   Page x coordinate to place BOX OBJECT.
 *  \param [in] origin_y   Page y coordinate to place BOX OBJECT.
 */
void o_box_print(TOPLEVEL *w_current, FILE *fp, OBJECT *o_current, 
		 int origin_x, int origin_y)
{
  int x, y, width, height;
  int color;
  int line_width, length, space;
  int fill_width, angle1, pitch1, angle2, pitch2;
  void (*outl_func)() = NULL;
  void (*fill_func)() = NULL;

  if (o_current == NULL) {
    printf("got null in o_box_print\n");
    return;
  }

  x = o_current->box->upper_x;
  y = o_current->box->upper_y;
  width  = abs(o_current->box->lower_x - o_current->box->upper_x);
  height = abs(o_current->box->lower_y - o_current->box->upper_y);
  color  = o_current->color;

  /*! \note
   *  Depending on the type of the line for this particular box, the
   *  appropriate function is chosen among #o_box_print_solid(),
   *  #o_box_print_dotted(), #o_box_print_dashed(),
   *  #o_box_print_center() and #o_box_print_phantom().
   *
   *  The needed parameters for each of these type is extracted from the
   *  <B>o_current</B> object. Depending on the type, unused parameters are
   *  set to -1.
   *
   *  In the eventuality of a length and/or space null, the line is printed
   *  solid to avoid and endless loop produced by other functions in such a
   *  case.
   */
  line_width = o_current->line_width;
  
  if(line_width <=2) {
    if(w_current->line_style == THICK) {
      line_width=LINE_WIDTH;
    } else {
      line_width=2;
    }
  }
  length = o_current->line_length;
  space  = o_current->line_space;

  switch(o_current->line_type) {
    case(TYPE_SOLID):
      length = -1; space  = -1;
      outl_func = o_box_print_solid;
      break;
      
    case(TYPE_DOTTED):
      length = -1;
      outl_func = o_box_print_dotted;
      break;
		
    case(TYPE_DASHED):
      outl_func = o_box_print_dashed;
      break;
      
    case(TYPE_CENTER):
      outl_func = o_box_print_center;
      break;
		
    case(TYPE_PHANTOM):
      outl_func = o_box_print_phantom;
      break;
		
    case(TYPE_ERASE):
      /* Unused for now, print it solid */
      length = -1; space  = -1;
      outl_func = o_box_print_solid;
      break;
  }

  if((length == 0) || (space == 0)) {
    length = -1; space  = -1;
    outl_func = o_box_print_solid;
  }

  (*outl_func)(w_current, fp,
	       x, y, width, height,
	       color,
	       line_width,
	       length, space,
	       origin_x, origin_y);

  /*! \note
   *  If the filling type of the box is not <B>HOLLOW</B>, the appropriate
   *  function is chosen among #o_box_print_filled(), #o_box_print_mesh()
   *  and #o_box_print_hatch(). The corresponding parameters are extracted
   *  from the <B>o_current</B> object and corrected afterward.
   *
   *  The case where <B>pitch1</B> and <B>pitch2</B> are null or negative is
   *  avoided as it leads to an endless loop in most of the called functions.
   *  In such a case, the box is printed filled. Unused parameters for each of
   *  these functions are set to -1 or any passive value.
   */
  if(o_current->fill_type != FILLING_HOLLOW) {
    fill_width = o_current->fill_width;
    angle1     = o_current->fill_angle1;
    pitch1     = o_current->fill_pitch1;
    angle2     = o_current->fill_angle2;
    pitch2     = o_current->fill_pitch2;
	
    switch(o_current->fill_type) {
      case(FILLING_FILL):
	angle1 = -1; pitch1 = 1;
	angle2 = -1; pitch2 = 1;
	fill_width = -1;
	fill_func = o_box_print_filled;
	break;
			
      case(FILLING_MESH):
	fill_func = o_box_print_mesh;
	break;
			
      case(FILLING_HATCH):
	angle2 = -1; pitch2 = 1;
	fill_func = o_box_print_hatch;
	break;
			
      case(FILLING_VOID):
	/* Unused for now, print it filled */
	angle1 = -1; pitch1 = 1;
	angle2 = -1; pitch2 = 1;
	fill_width = -1;
	fill_func = o_box_print_filled;
	break;
      case(FILLING_HOLLOW):
	/* nop */
	break;
	
    }
    
    if((pitch1 <= 0) || (pitch2 <= 0)) {
      angle1 = -1; pitch1 = 1;
      angle2 = -1; pitch2 = 1;
      fill_func = o_box_print_filled;
    }
    
    (*fill_func)(w_current, fp,
                 x, y, width, height,
                 color,
                 fill_width,
                 angle1, pitch1, angle2, pitch2,
                 origin_x, origin_y);
  }
}

/*! \brief Print a solid BOX to Postscript document.
 *  \par Function Description
 *  This function prints the outline of a box when a solid line type is
 *  required. The box is defined by the coordinates of its upper left corner
 *  in (<B>x</B>,<B>y</B>) and its width and height given by the <B>width</B> and
 *  <B>height</B> parameters. 
 *  The postscript file is defined by the file pointer <B>fp</B>.
 *  The parameters <B>length</B> and <B>space</B> are ignored.
 *
 *  It uses the function #o_line_print_solid() to print the outline.
 *  It performs four calls to this function, one for each of its side.
 *
 *  All dimensions are in mils.
 *
 *  \param [in] w_current   The TOPLEVEL object.
 *  \param [in] fp          FILE pointer to Postscript document.
 *  \param [in] x           Upper x coordinate of BOX.
 *  \param [in] y           Upper y coordinate of BOX.
 *  \param [in] width       Width of BOX.
 *  \param [in] height      Height of BOX.
 *  \param [in] color       BOX color.
 *  \param [in] line_width  BOX Line width.
 *  \param [in] length      Dashed line length.
 *  \param [in] space       Amount of space between dashes.
 *  \param [in] origin_x    Page x coordinate to place BOX OBJECT.
 *  \param [in] origin_y    Page y coordinate to place BOX OBJECT.
 *
 *  \par Author's Note
 *   PB : parameter filled removed
 */
void
o_box_print_solid(TOPLEVEL *w_current, FILE *fp,
                  int x, int y,
                  int width, int height,
                  int color,
                  int line_width, int length, int space, 
                  int origin_x, int origin_y)
{
  int x1, y1;

  if (w_current->print_color) {
    f_print_set_color(fp, color);
  }

  x1 = x;
  y1 = y - height; /* move the origin to 0, 0*/

  o_line_print_solid(w_current, fp,
                     x1, y1, x1 + width, y1,
                     color,
                     line_width, length, space,
                     origin_x, origin_y);
  o_line_print_solid(w_current, fp,
                     x1 + width, y1, x1 + width, y1 + height,
                     color,
                     line_width, length, space,
                     origin_x, origin_y);
  o_line_print_solid(w_current, fp,
                     x1 + width, y1 + height, x1, y1 + height,
                     color,
                     line_width, length, space,
                     origin_x, origin_y);
  o_line_print_solid(w_current, fp,
                     x1, y1 + height, x1, y1,
                     color,
                     line_width, length, space,
                     origin_x, origin_y);
}

/*! \brief Print a dotted BOX to Postscript document.
 *  \par Function Description
 *  This function prints the outline of a box when a dotted line type is
 *  required. The box is defined by the coordinates of its upper left corner
 *  in (<B>x</B>,<B>y</B>) and its width and height given by the <B>width</B> and
 *  <B>height</B> parameters. 
 *  The postscript file is defined by the file pointer <B>fp</B>.
 *  The parameters <B>length</B> is ignored.
 *
 *  It uses the function #o_line_print_dotted() to print the outline.
 *  It performs four calls to this function, one for each of its side.
 *
 *  All dimensions are in mils.
 *
 *  \param [in] w_current   The TOPLEVEL object.
 *  \param [in] fp          FILE pointer to Postscript document.
 *  \param [in] x           Upper x coordinate of BOX.
 *  \param [in] y           Upper y coordinate of BOX.
 *  \param [in] width       Width of BOX.
 *  \param [in] height      Height of BOX.
 *  \param [in] color       BOX color.
 *  \param [in] line_width  BOX Line width.
 *  \param [in] length      Dashed line length.
 *  \param [in] space       Amount of space between dashes.
 *  \param [in] origin_x    Page x coordinate to place BOX OBJECT.
 *  \param [in] origin_y    Page y coordinate to place BOX OBJECT.
 *
 *  \par Author's Note
 *  PB : parameter filled removed
 *  PB : parameter o_current removed
 */
void o_box_print_dotted(TOPLEVEL *w_current, FILE *fp,
			int x, int y,
			int width, int height,
			int color,
			int line_width, int length, int space, 
			int origin_x, int origin_y)
{
  int x1, y1;

  if (w_current->print_color) {
    f_print_set_color(fp, color);
  }

  x1 = x;
  y1 = y - height; /* move the origin to 0, 0*/

  o_line_print_dotted(w_current, fp,
                      x1, y1, x1 + width, y1,
                      color,
                      line_width, length, space,
                      origin_x, origin_y);
  o_line_print_dotted(w_current, fp,
                      x1 + width, y1, x1 + width, y1 + height,
                      color,
                      line_width, length, space,
                      origin_x, origin_y);
  o_line_print_dotted(w_current, fp,
                      x1 + width, y1 + height, x1, y1 + height,
                      color,
                      line_width, length, space,
                      origin_x, origin_y);
  o_line_print_dotted(w_current, fp,
                      x1, y1 + height, x1, y1,
                      color,
                      line_width, length, space,
                      origin_x, origin_y);
}

/*! \brief Print a dashed BOX to Postscript document.
 *  \par Function Description
 *  This function prints the outline of a box when a dashed line type is
 *  required. The box is defined by the coordinates of its upper left corner
 *  in (<B>x</B>,<B>y</B>) and its width and height given by the <B>width</B> and
 *  <B>height</B> parameters. 
 *  The postscript file is defined by the file pointer <B>fp</B>.
 *
 *  It uses the function #o_line_print_dashed() to print the outline.
 *  It performs four calls to this function, one for each of its side.
 *
 *  All dimensions are in mils.
 *
 *  \param [in] w_current   The TOPLEVEL object.
 *  \param [in] fp          FILE pointer to Postscript document.
 *  \param [in] x           Upper x coordinate of BOX.
 *  \param [in] y           Upper y coordinate of BOX.
 *  \param [in] width       Width of BOX.
 *  \param [in] height      Height of BOX.
 *  \param [in] color       BOX color.
 *  \param [in] line_width  BOX Line width.
 *  \param [in] length      Dashed line length.
 *  \param [in] space       Amount of space between dashes.
 *  \param [in] origin_x    Page x coordinate to place BOX OBJECT.
 *  \param [in] origin_y    Page y coordinate to place BOX OBJECT.
 * 
 *  \par Author's Note
 *  PB : parameter filled removed
 *  PB : parameter o_current removed
 */
void o_box_print_dashed(TOPLEVEL *w_current, FILE *fp,
			int x, int y,
			int width, int height,
			int color,
			int line_width, int length, int space, 
			int origin_x, int origin_y)
{
  int x1, y1;

  if (w_current->print_color) {
    f_print_set_color(fp, color);
  }

  
  x1 = x;
  y1 = y - height; /* move the origin to 0, 0*/

  o_line_print_dashed(w_current, fp,
                      x1, y1, x1 + width, y1,
                      color,
                      line_width, length, space,
                      origin_x, origin_y);
  o_line_print_dashed(w_current, fp,
                      x1 + width, y1, x1 + width, y1 + height,
                      color,
                      line_width, length, space,
                      origin_x, origin_y);
  o_line_print_dashed(w_current, fp,
                      x1 + width, y1 + height, x1, y1 + height,
                      color,
                      line_width, length, space,
                      origin_x, origin_y);
  o_line_print_dashed(w_current, fp,
                      x1, y1 + height, x1, y1,
                      color,
                      line_width, length, space,
                      origin_x, origin_y);
}

/*! \brief Print centered line type BOX to Postscript document.
 *  \par Function Description
 *  This function prints the outline of a box when a centered line type is
 *  required. The box is defined by the coordinates of its upper left corner
 *  in (<B>x</B>,<B>y</B>) and its width and height given by the <B>width</B> and
 *  <B>height</B> parameters. 
 *  The postscript file is defined by the file pointer <B>fp</B>.
 *
 *  It uses the function #o_line_print_center() to print the outline.
 *  It performs four calls to this function, one for each of its side.
 *
 *  All dimensions are in mils.
 *
 *  \param [in] w_current   The TOPLEVEL object.
 *  \param [in] fp          FILE pointer to Postscript document.
 *  \param [in] x           Upper x coordinate of BOX.
 *  \param [in] y           Upper y coordinate of BOX.
 *  \param [in] width       Width of BOX.
 *  \param [in] height      Height of BOX.
 *  \param [in] color       BOX color.
 *  \param [in] line_width  BOX Line width.
 *  \param [in] length      Dashed line length.
 *  \param [in] space       Amount of space between dashes.
 *  \param [in] origin_x    Page x coordinate to place BOX OBJECT.
 *  \param [in] origin_y    Page y coordinate to place BOX OBJECT.
 *
 *  \par Author's note
 * PB : parameter filled removed
 * PB : parameter o_current removed
 */
void o_box_print_center(TOPLEVEL *w_current, FILE *fp,
			int x, int y,
			int width, int height,
			int color,
			int line_width, int length, int space, 
			int origin_x, int origin_y)
{
  int x1, y1;

  if (w_current->print_color) {
    f_print_set_color(fp, color);
  }

  x1 = x;
  y1 = y - height; /* move the origin to 0, 0*/

  o_line_print_center(w_current, fp,
                      x1, y1, x1 + width, y1,
                      color,
                      line_width, length, space,
                      origin_x, origin_y);
  o_line_print_center(w_current, fp,
                      x1 + width, y1, x1 + width, y1 + height,
                      color,
                      line_width, length, space,
                      origin_x, origin_y);
  o_line_print_center(w_current, fp,
                      x1 + width, y1 + height, x1, y1 + height,
                      color,
                      line_width, length, space,
                      origin_x, origin_y);
  o_line_print_center(w_current, fp,
                      x1, y1 + height, x1, y1,
                      color,
                      line_width, length, space,
                      origin_x, origin_y);
}

/*! \brief Print phantom line type BOX to Postscript document.
 *  \par Function Description
 *  This function prints the outline of a box when a phantom line type is
 *  required. The box is defined by the coordinates of its upper left corner
 *  in (<B>x</B>,<B>y</B>) and its width and height given by the <B>width</B> and
 *  <B>height</B> parameters. 
 *  The postscript file is defined by the file pointer <B>fp</B>.
 *
 *  It uses the function #o_line_print_phantom() to print the outline.
 *  It performs four calls to this function, one for each of its side.
 *
 *  All dimensions are in mils.
 *  
 *  \param [in] w_current   The TOPLEVEL object.
 *  \param [in] fp          FILE pointer to Postscript document.
 *  \param [in] x           Upper x coordinate of BOX.
 *  \param [in] y           Upper y coordinate of BOX.
 *  \param [in] width       Width of BOX.
 *  \param [in] height      Height of BOX.
 *  \param [in] color       BOX color.
 *  \param [in] line_width  BOX Line width.
 *  \param [in] length      Dashed line length.
 *  \param [in] space       Amount of space between dashes.
 *  \param [in] origin_x    Page x coordinate to place BOX OBJECT.
 *  \param [in] origin_y    Page y coordinate to place BOX OBJECT.
 *
 *  \par Author's note
 *  PB : parameter filled removed
 *  PB : parameter o_current removed
 */
void o_box_print_phantom(TOPLEVEL *w_current, FILE *fp,
			 int x, int y,
			 int width, int height,
			 int color,
			 int line_width, int length, int space, 
			 int origin_x, int origin_y)
{
  int x1, y1;

  if (w_current->print_color) {
    f_print_set_color(fp, color);
  }

  x1 = x;
  y1 = y - height; /* move the origin to 0, 0*/

  o_line_print_phantom(w_current, fp,
                       x1, y1, x1 + width, y1,
                       color,
                       line_width, length, space,
                       origin_x, origin_y);
  o_line_print_phantom(w_current, fp,
                       x1 + width, y1, x1 + width, y1 + height,
                       color,
                       line_width, length, space,
                       origin_x, origin_y);
  o_line_print_phantom(w_current, fp,
                       x1 + width, y1 + height, x1, y1 + height,
                       color,
                       line_width, length, space,
                       origin_x, origin_y);
  o_line_print_phantom(w_current, fp,
                       x1, y1 + height, x1, y1,
                       color,
                       line_width, length, space,
                       origin_x, origin_y);
}

/*! \brief Print a solid pattern BOX to Postscript document.
 *  \par Function Description
 *  The function prints a filled box with a solid pattern. No outline is
 *  printed. 
 *  The box is defined by the coordinates of its upper left corner in
 *  (<B>x</B>,<B>y</B>) and its width and height given by the <B>width</B> and
 *  <B>height</B> parameters. The postscript file is defined by the file
 *  pointer <B>fp</B>.
 *  <B>fill_width</B>, <B>angle1</B> and <B>pitch1</B>, <B>angle2</B> and <B>pitch2</B>
 *  parameters are ignored in this functions but kept for compatibility
 *  with other fill functions.
 *
 *  It uses the fbox postscript function defined in the prolog to
 *  specify a filled box.
 * 
 *  All dimensions are in mils.
 *
 *  \param [in] w_current   The TOPLEVEL object.
 *  \param [in] fp          FILE pointer to Postscript document.
 *  \param [in] x           Upper x coordinate of BOX.
 *  \param [in] y           Upper y coordinate of BOX.
 *  \param [in] width       Width of BOX.
 *  \param [in] height      Height of BOX.
 *  \param [in] color       BOX color.
 *  \param [in] fill_width  BOX fill width. (unused).
 *  \param [in] angle1      (unused).
 *  \param [in] pitch1      (unused).
 *  \param [in] angle2      (unused).
 *  \param [in] pitch2      (unused).
 *  \param [in] origin_x    Page x coordinate to place BOX OBJECT.
 *  \param [in] origin_y    Page y coordinate to place BOX OBJECT.
 */
void o_box_print_filled(TOPLEVEL *w_current, FILE *fp,
			int x, int y,
			int width, int height,
			int color,
			int fill_width,
			int angle1, int pitch1,
			int angle2, int pitch2,
			int origin_x, int origin_y)
{
  int x1, y1;

  if (w_current->print_color) {
    f_print_set_color(fp, color);
  }

  x1 = x;
  y1 = y-height; /* move the origin to 0, 0*/
  fprintf(fp, "%d %d %d %d fbox\n",
	  width, height,
	  x1-origin_x, y1-origin_y);
	  
}

/*! \brief Print a mesh pattern BOX to Postscript document.
 *  \par Function Description
 *  This function prints a meshed box. No outline is printed. The box is
 *  defined by the coordinates of its upper left corner in (<B>x</B>,<B>y</B>) and
 *  its width and height given by the <B>width</B> and <B>height</B> parameters.
 *  The postscript file is defined by the file pointer <B>fp</B>.
 *
 *  The inside mesh is achieved by two successive call to the
 *  #o_box_print_hatch() function, given <B>angle1</B> and <B>pitch1</B> the first
 *  time and <B>angle2</B> and <B>pitch2</B> the second time.
 *
 *  Negative or null values for <B>pitch1</B> and/or <B>pitch2</B> are not allowed
 *  as it leads to an endless loop in #o_box_print_hatch().
 * 
 *  All dimensions are in mils.
 *
 *  \param [in] w_current   The TOPLEVEL object.
 *  \param [in] fp          FILE pointer to Postscript document.
 *  \param [in] x           Upper x coordinate of BOX.
 *  \param [in] y           Upper y coordinate of BOX.
 *  \param [in] width       Width of BOX.
 *  \param [in] height      Height of BOX.
 *  \param [in] color       BOX color.
 *  \param [in] fill_width  BOX fill width.
 *  \param [in] angle1      1st angle for mesh pattern.
 *  \param [in] pitch1      1st pitch for mesh pattern.
 *  \param [in] angle2      2nd angle for mesh pattern.
 *  \param [in] pitch2      2nd pitch for mesh pattern.
 *  \param [in] origin_x    Page x coordinate to place BOX OBJECT.
 *  \param [in] origin_y    Page y coordinate to place BOX OBJECT.
 */
void o_box_print_mesh(TOPLEVEL *w_current, FILE *fp,
		      int x, int y,
		      int width, int height,
		      int color,
		      int fill_width,
		      int angle1, int pitch1,
		      int angle2, int pitch2,
		      int origin_x, int origin_y)
{
  o_box_print_hatch(w_current, fp,
                    x, y, width, height,
                    color,
                    fill_width,
                    angle1, pitch1, -1, -1,
                    origin_x, origin_y);
  o_box_print_hatch(w_current, fp,
                    x, y, width, height,
                    color,
                    fill_width,
                    angle2, pitch2, -1, -1,
                    origin_x, origin_y);

}

/*! \brief Print a hatch pattern BOX to Postscript document.
 *  \par Function Description
 *  The function prints a hatched box. No outline is printed. The box is
 *  defined by the coordinates of its upper left corner in (<B>x</B>,<B>y</B>) and
 *  its width and height given by the <B>width</B> and <B>height</B> parameters.
 *  The postscript file is defined by the file pointer <B>fp</B>. 
 *  <B>fill_width</B>, <B>angle1</B>, <B>pitch1</B> parameters define the way the box
 *  has to be hatched.
 *  <B>angle2</B> and <B>pitch2</B> parameters are unused but kept for compatibility
 *  with other fill functions.
 *
 *  Negative or null values for <B>pitch1</B> are not allowed as it leads to an
 *  endless loop.
 *
 *  All dimensions are in mils.
 *
 *  \param [in] w_current   The TOPLEVEL object.
 *  \param [in] fp          FILE pointer to Postscript document.
 *  \param [in] x           Upper x coordinate of BOX.
 *  \param [in] y           Upper y coordinate of BOX.
 *  \param [in] width       Width of BOX.
 *  \param [in] height      Height of BOX.
 *  \param [in] color       BOX color.
 *  \param [in] fill_width  BOX fill width.
 *  \param [in] angle1      Angle of hatch pattern.
 *  \param [in] pitch1      Pitch of hatch pattern.
 *  \param [in] angle2      (unused).
 *  \param [in] pitch2      (unused).
 *  \param [in] origin_x    Page x coordinate to place BOX OBJECT.
 *  \param [in] origin_y    Page y coordinate to place BOX OBJECT.
 */
void o_box_print_hatch(TOPLEVEL *w_current, FILE *fp,
		       int x, int y,
		       int width, int height,
		       int color,
		       int fill_width,
		       int angle1, int pitch1,
		       int angle2, int pitch2,
		       int origin_x, int origin_y)
{
  int x3, y3, x4, y4;
  double cos_a_, sin_a_;
  double x0, y0, r;
  double x1, y1, x2, y2;
  double amin, amax, a[4], min1, min2, max1, max2;

  if (w_current->print_color) {
    f_print_set_color(fp, color);
  }

  /* Avoid printing line widths too small */
  if (fill_width <= 1) fill_width = 2;

  /* The cosinus and sinus of <B>angle1</B> are computed once and reused later. */
  cos_a_ = cos(((double) angle1) * M_PI/180);
  sin_a_ = sin(((double) angle1) * M_PI/180);

  /*! \note
   *  The function considers the smallest circle around the box. Its radius
   *  is given by the following relation. Its center is given by the point
   *  at the middle of the box horizontally and vertically (intersection of
   *  its two diagonals).
   */
  r = sqrt((double) (pow(width, 2) + pow(height, 2))) / 2;

  /*
   *  When drawing a line in a circle there is two intersections. With the
   *  previously described circle, these intersections are out of the box.
   *  They can be easily calculated, the first by resolution of an equation
   *  and the second one by symetry in relation to the vertical axis going
   *  through the center of the circle.
   *
   *  These two points are then rotated of angle <B>angle1</B> using the matrix
   *  previously mentioned.
   */
  y0 = 0;
  while(y0 < r) {
    x0 = pow(r, 2) - pow(y0, 2);
    x0 = sqrt(x0);

    x1 = (x0*cos_a_ - y0*sin_a_);
    y1 = (x0*sin_a_ + y0*cos_a_);
    x2 = ((-x0)*cos_a_ - y0*sin_a_);
    y2 = ((-x0)*sin_a_ + y0*cos_a_);
  
    /*
     * It now parametrizes the segment : first intersection is given the
     * value of 0 and the second is given the value of 1. The four values for
     * each intersection of the segment and the four sides (vertical or
     * horizontal) of the box are given by the following relations :
     */                                                             
    if((int) (x2 - x1) != 0) {
      a[0] = ((-width/2) - x1) / (x2 - x1);
      a[1] = ((width/2)  - x1) / (x2 - x1);
    } else {
      a[0] = 0; a[1] = 1;
    }
    
    if((int) (y2 - y1) != 0) {
      a[2] = ((-height/2) - y1) / (y2 - y1);
      a[3] = ((height/2)  - y1) / (y2 - y1);
    } else {
      a[2] = 0; a[3] = 1;
    }

    /*
     * It now has to check which of these four values are for intersections
     * with the sides of the box (some values may be for intersections out of
     * the box). This is made by a min/max function.
     */
    if(a[0] < a[1]) {
      min1 = a[0]; max1 = a[1];
    } else {
      min1 = a[1]; max1 = a[0];
    }
    
    if(a[2] < a[3]) {
      min2 = a[2]; max2 = a[3];
    } else {
      min2 = a[3]; max2 = a[2];
    }
    
    amin = (min1 < min2) ? min2 : min1;
    amin = (amin < 0) ? 0 : amin;
    
    amax = (max1 < max2) ? max1 : max2;
    amax = (amax < 1) ? amax : 1;

    /*
     * If the segment really go through the box it prints the line. It also
     * takes the opportunity of the symetry in the box in relation to its
     * center to print the second line at the same time.
     *
     * If there is no intersection of the segment with any of the sides, then
     * there is no need to continue : there would be no more segment in the
     * box to print.
     */
    if((amax > amin) && (amax != 1) && (amin != 0)) {
      /* There is intersection between the line and the box edges */
      x3 = (int) (x1 + amin*(x2 - x1));
      y3 = (int) (y1 + amin*(y2 - y1));
      
      x4 = (int) (x1 + amax*(x2 - x1));
      y4 = (int) (y1 + amax*(y2 - y1));
      
      fprintf(fp,"%d %d %d %d %d line\n",
	      x3 + (x + width/2), y3 + (y - height/2),
	      x4 + (x + width/2), y4 + (y - height/2),
	      fill_width);
      
      fprintf(fp,"%d %d %d %d %d line\n",
	      -x3 + (x + width/2), -y3 + (y - height/2),
	      -x4 + (x + width/2), -y4 + (y - height/2),
	      fill_width);
      
    } else {
      break;
    }
    
    y0 = y0 + pitch1;
  }
}

#if 0 /* original way of printing box, no longer used */
/*! \brief Print BOX to Postscript document using old method.
 *  \par Function Description
 *  This function is the old function to print a box. It does not handle line
 *  type and filling of a box.
 *
 *  \param [in] w_current  The TOPLEVEL object.
 *  \param [in] fp         FILE pointer to Postscript document.
 *  \param [in] origin_x   Page x coordinate to place BOX OBJECT.
 *  \param [in] origin_y   Page x coordinate to place BOX OBJECT.
 */
void o_box_print_old(TOPLEVEL *w_current, FILE *fp,
		     int origin_x, int origin_y)
{
  int width, height;
  int x1, y1;
  if (o_current == NULL) {
    printf("got null in o_box_print\n");
    return;
  }

  if (w_current->print_color) {
    f_print_set_color(fp, o_current->color);
  }


  width = abs(o_current->line_points->x2 - o_current->line_points->x1); 
  height = abs(o_current->line_points->y1 - o_current->line_points->y2);

  x1 = o_current->line_points->x1;
  y1 = o_current->line_points->y1-height; /* move the origin to 0, 0*/

  fprintf(fp, "newpath\n");
  fprintf(fp, "%d mils %d mils moveto\n", x1-origin_x, y1-origin_y);
  fprintf(fp, "%d mils %d mils box\n", width, height);

}
#endif

/*! \brief Draw a box in an image.
 *  \par Function Description
 *  This function draws a box in an image with the libgd function
 *  #gdImageRectangle().
 *
 *  \param [in] w_current   The TOPLEVEL object.
 *  \param [in] o_current   BOX OBJECT to draw.
 *  \param [in] origin_x    (unused).
 *  \param [in] origin_y    (unused).
 *  \param [in] color_mode  Draw box in color if TRUE, B/W otherwise.
 */
void o_box_image_write(TOPLEVEL *w_current, OBJECT *o_current, 
		       int origin_x, int origin_y, int color_mode)
{
  int color;


  if (o_current == NULL) {
    printf("got null in o_box_image_write\n");
    return;
  }


  if (color_mode == TRUE) {
    color = o_image_geda2gd_color(o_current->color);
  } else {
    color = image_black;
  }

  /* assumes screen coords are already calculated correctly */
#ifdef HAS_LIBGD

  gdImageSetThickness(current_im_ptr, SCREENabs(w_current,
                                                o_current->line_width));

  gdImageRectangle(current_im_ptr, 
                   o_current->box->screen_upper_x,
                   o_current->box->screen_upper_y,
                   o_current->box->screen_lower_x,
                   o_current->box->screen_lower_y, 
                   color);
#endif

}
