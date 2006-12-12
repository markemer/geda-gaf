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
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <math.h>

#include <gtk/gtk.h>
#include <guile/gh.h>

#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixdata.h>


#include "defines.h"
#include "struct.h"
#include "globals.h"
#include "o_types.h"
#include "colors.h"
#include "funcs.h"

#include "../include/prototype.h"

/*! \brief Create picture OBJECT from character string.
 *  \par Function Description
 *  This function will get the description of a picture from the character
 *  string <B>*buf</B>. The new picture is then added to the list of object of
 *  which <B>*object_list</B> is the last element before the call.
 *  The function returns the new last element, that is the added
 *  picture object.
 *
 *  \param [in]  w_current       The TOPLEVEL object.
 *  \param [out] object_list     OBJECT list to create picture in.
 *  \param [in]  buf             Character string with picture description.
 *  \param [in]  fp              Picture file to read.
 *  \param [in]  release_ver     libgeda release version number.
 *  \param [in]  fileformat_ver  libgeda file format version number.
 *  \return A pointer to the new picture object.
 */
OBJECT *o_picture_read(TOPLEVEL *w_current, OBJECT *object_list,
		       char buf[], FILE *fp,
		       unsigned int release_ver,unsigned int fileformat_ver)
{
  int x1, y1;
  int width, height, angle;
  gchar mirrored, embedded;
  int num_conv;
  gchar type;
  gchar buffer[MAX_TEXT_LINE_LENGTH]; 
  gchar *filename;
  GdkPixbuf *pixbuf;
  static char gdk_initialized=0;

  /* Initialize GDK first if it isn't a graphic app */
  /* i.e: it has no main window */
  if ( (w_current->main_window == NULL) &&
       (gdk_initialized == 0)) {
    gdk_init(NULL, NULL);
    gdk_initialized = 1;
  }

  num_conv = sscanf(buf, "%c %d %d %d %d %d %c %c\n",
	 &type, &x1, &y1, &width, &height, &angle, &mirrored, &embedded);
  
  if (num_conv != 8) {
    fprintf(stderr, "Error reading picture definition line: %s.\n", buf);
    s_log_message ("Error reading picture definition line: %s.\n", buf);
  }

  /* Convert from ascii character to number */
  if (g_ascii_isdigit(mirrored)) {
    mirrored -= 0x30;
  }

  if (g_ascii_isdigit(embedded)) {
    embedded -= 0x30;
  }

  if (width == 0 || height == 0) {
    fprintf(stderr, "Found a zero width/height picture [ %c %d %d %d %d ]\n",
            type, x1, y1, width, height);
    s_log_message("Found a zero width/height picture [ %c %d %d %d %d ]\n",
                  type, x1, y1, width, height);
  }

  if ( (mirrored > 1) || (mirrored < 0)) {
    fprintf(stderr, "Found a picture with a wrong 'mirrored' parameter: %c.\n",
	    mirrored);
    s_log_message("Found a picture with a wrong 'mirrored' parameter: %c.\n",
	    mirrored);
    s_log_message("Setting mirrored to 0\n");
    mirrored = 0;
  }

  if ( (embedded > 1) || (embedded < 0)) {
    fprintf(stderr, "Found a picture with a wrong 'embedded' parameter: %c.\n",
	    embedded);
    s_log_message("Found a picture with a wrong 'embedded' parameter: %c.\n",
	    embedded);
    s_log_message("Setting embedded to 0\n");
    embedded = 0;
  }
  switch(angle) {
	
    case(0):
    case(90):
    case(180):
    case(270):
    break;

    default:
    fprintf(stderr, "Found an unsupported picture angle [ %d ]\n", angle);
    s_log_message("Found an unsupported picture angle [ %d ]\n", angle);
    s_log_message("Setting angle to 0\n");
    angle=0;
    break;

  }

  fgets(buffer, 1024, fp);
  
  filename = g_strdup (buffer);
  filename = remove_last_nl(filename);	
    
  pixbuf = NULL;

  if (embedded == 0) {
    pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
    if (pixbuf == NULL) {
      fprintf(stderr, "Error loading picture from file: %s.\n", filename);
      s_log_message( "Error loading picture from file: %s.\n", filename);
    }
  }
  else {
    GString *encoded_picture=g_string_new("");
    GdkPixdata pixdata;
    guint pixdata_serialized_length;
    gchar *pixdata_serialized_data;
    char finished = 0;

    /* Read the encoded picture */
    do {
      finished = 0;
      fgets(buffer, 1024, fp);
      if (g_strcasecmp(buffer, ".\n") != 0) {
	encoded_picture=g_string_append (encoded_picture, buffer);
	encoded_picture=g_string_append (encoded_picture, "\n");
      }
      else {
	finished = 1;
      }
    } while (finished == 0);

    /* Decode the picture */
    pixdata_serialized_data=s_encoding_base64_decode(encoded_picture->str,
						     encoded_picture->len,
						     &pixdata_serialized_length);
    if (pixdata_serialized_data == NULL) {
      fprintf(stderr, "Error decoding picture.\n");      
      s_log_message ("Error decoding picture.\n");
    }
    else {
      /* Deserialize the picture */
      if (!gdk_pixdata_deserialize(&pixdata,pixdata_serialized_length, 
				   (guint8 *) pixdata_serialized_data, NULL)) {
				     fprintf(stderr, "Error deserializing picture.\n");
				   }
      else {
	pixbuf = gdk_pixbuf_from_pixdata (&pixdata,TRUE, NULL);
      }
    }

    if (pixdata_serialized_data != NULL) {
      g_free (pixdata_serialized_data);
    }
    if (encoded_picture != NULL) {
      g_string_free (encoded_picture, TRUE);
    }

  }

  if (pixbuf == NULL) {
    /* If the pixbuf couldn't be loaded, then try to load a warning picture */
    char *temp_filename;

    fprintf(stderr, "Loading warning picture.\n");
    s_log_message ("Loading warning picture.\n");
    
    temp_filename = g_strconcat(w_current->bitmap_directory, 
			       G_DIR_SEPARATOR_S, "gschem-warning.png", NULL);
    pixbuf = gdk_pixbuf_new_from_file (temp_filename, NULL);
    if (pixbuf == NULL) {
      fprintf(stderr, "Error loading picture from file: %s.\n", temp_filename);
      s_log_message( "Error loading picture from file: %s.\n", temp_filename);
    }      
    g_free (temp_filename);
  }
  
  /* create and add the picture to the list */
  /* The picture is described by its upper left and lower right corner */
  object_list = (OBJECT *) o_picture_add(w_current, object_list,
					 pixbuf, filename, 
					 (double) (height/width),
					 type, 
					 x1, y1+height, x1+width, y1, 
					 angle, mirrored, embedded);
  if (object_list->picture->filename) {
    g_free(object_list->picture->filename);
  }
  object_list->picture->filename = g_strdup(filename);
  
  g_free (filename);
  return(object_list);
}

/*! \brief Create a character string representation of a picture OBJECT.
 *  \par Function Description
 *  This function formats a string in the buffer <B>*buff</B> to describe
 *  the picture object <B>*object</B>.
 *
 *  \param [in] object  Picture OBJECT to create string from.
 *  \return A pointer to the picture OBJECT character string.
 *
 *  \note
 *  Caller must free returned character string.
 *
 */
char *o_picture_save(OBJECT *object)
{
  int width, height, x1, y1;
  gchar *encoded_picture=NULL;
  gchar *out=NULL;
  gchar *pixdata_serialized_data=NULL;
  guint pixdata_serialized_stream_length;
  guint encoded_picture_length;

  /* calculate the width and height of the box */
  width  = abs(object->picture->lower_x - object->picture->upper_x); 
  height = abs(object->picture->upper_y - object->picture->lower_y);

  /* calculate the lower left corner of the box */
  x1 = object->picture->upper_x;
  y1 = object->picture->upper_y - height; /* move the origin to 0, 0*/
  
#if DEBUG
  printf("picture: %d %d %d %d\n", x1, y1, width, height);
#endif

  /* Encode the picture if it's embedded */
  if (object->picture->embedded == 1) {
    GdkPixdata pixdata;
    
    /* Serialize the picture data */
    gdk_pixdata_from_pixbuf (&pixdata, object->picture->original_picture, FALSE);
    pixdata_serialized_data = (gchar *) gdk_pixdata_serialize(&pixdata, 
						    &pixdata_serialized_stream_length);
    if (pixdata_serialized_data == NULL) {
      fprintf (stderr, "ERROR: o_picture_save: failed to create serialized data from picture\n");
      s_log_message ("ERROR: o_picture_save: failed to create serialized data from picture\n");
    }
    else {
      /* Encode the picture */
      encoded_picture = s_encoding_base64_encode(pixdata_serialized_data, 
						 pixdata_serialized_stream_length,
						 &encoded_picture_length, TRUE); 
      if (encoded_picture == NULL) {
	fprintf(stderr, "ERROR: o_picture_save: unable to encode the picture.\n");
	s_log_message("ERROR: o_picture_save: unable to encode the picture.\n");
      }
    }
  }

  if (object->picture->embedded==1) {
    out = g_strdup_printf("%c %d %d %d %d %d %c %c\n%s\n%s\n%s", 
			  object->type,
			  x1, y1, width, height,
			  object->picture->angle,
			  /* Convert the (0,1) chars to ASCII */
			  (object->picture->mirrored)+0x30, 
			  object->picture->embedded+0x30, 
			  object->picture->filename,
			  encoded_picture,
			  ".");
  }
  else {
    out = g_strdup_printf("%c %d %d %d %d %d %c %c\n%s", 
			  object->type,
			  x1, y1, width, height,
			  object->picture->angle,
			  /* Convert the (0,1) chars to ASCII */
			  (object->picture->mirrored)+0x30, 
			  object->picture->embedded+0x30, 
			  object->picture->filename);
  }
  if (encoded_picture != NULL) {
    g_free(encoded_picture);
  }
  if (pixdata_serialized_data != NULL) {
    g_free(pixdata_serialized_data);
  }

  return(out);

}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 *  \param [in] w_current  The TOPLEVEL object.
 *  \param [in] pixbuf
 *  \param [in] filename
 */
void o_picture_set_pixbuf(TOPLEVEL *w_current,
			  GdkPixbuf *pixbuf, char *filename)
{

  /* need to put an error messages here */
  if (pixbuf == NULL)  {
    fprintf(stderr, "error! picture in set pixbuf was NULL\n");
    return;
  }

  if (w_current->current_pixbuf != NULL) {
    g_object_unref(w_current->current_pixbuf);
    w_current->current_pixbuf=NULL;
  }

  if (w_current->pixbuf_filename != NULL) {
    g_free(w_current->pixbuf_filename);
    w_current->pixbuf_filename=NULL;
  }

  w_current->current_pixbuf = pixbuf;
  w_current->pixbuf_filename = (char *) g_strdup(filename);
  
  w_current->pixbuf_wh_ratio = (double) gdk_pixbuf_get_width(pixbuf) / 
                                        gdk_pixbuf_get_height(pixbuf);

  /* be sure to free this pixbuf somewhere */
}

/*! \brief Create and add picture OBJECT to list.
 *  \par Function Description
 *  This function creates a new object representing a picture.
 *  This object is added to the end of the list <B>object_list</B> pointed
 *  object belongs to.
 *  The picture is described by its upper left corner - <B>x1</B>, <B>y1</B> -
 *  and its lower right corner - <B>x2</B>, <B>y2</B>.
 *  The <B>type</B> parameter must be equal to #OBJ_PICTURE. 
 *
 *  The #OBJECT structure is allocated with the
 *  #s_basic_init_object() function. The structure describing the
 *  picture is allocated and initialized with the parameters given to the
 *  function.
 *
 *  The object is added to the end of the list described by the
 *  <B>object_list</B> parameter by the #s_basic_link_object().
 *
 *  \param [in]     w_current    The TOPLEVEL object.
 *  \param [in,out] object_list  OBJECT list to add line to.
 *  \param [in]     pixbuf       The GdkPixbuf picture to add.
 *  \param [in]     filename     File name to read picture from.
 *  \param [in]     ratio        Picture height to width ratio.
 *  \param [in]     type         Must be OBJ_PICTURE.
 *  \param [in]     x1           Upper x coordinate.
 *  \param [in]     y1           Upper y coordinate.
 *  \param [in]     x2           Lower x coordinate.
 *  \param [in]     y2           Lower y coordinate.
 *  \param [in]     angle        Picture rotation angle.
 *  \param [in]     mirrored     Whether the image should be mirrored or not.
 *  \param [in]     embedded     ???
 *  \return A pointer to the new end of the object list.
 */
OBJECT *o_picture_add(TOPLEVEL *w_current, OBJECT *object_list,
		      GdkPixbuf * pixbuf, char *filename, double ratio,
		      char type, 
		      int x1, int y1, int x2, int y2, int angle, char mirrored,
		      char embedded)
{
  OBJECT *new_node;
  PICTURE *picture;

  /* create the object */
  new_node        = s_basic_init_object("picture");
  new_node->type  = type;
  
  picture = (PICTURE *) g_malloc(sizeof(PICTURE));
  new_node->picture   = picture;
  
  /* describe the picture with its upper left and lower right corner */
  picture->upper_x = x1;
  picture->upper_y = y1;
  picture->lower_x = x2;
  picture->lower_y = y2;
  
  /* Fill the picture data */
  
  /* Create a copy of the pixbuf */
  picture->original_picture = gdk_pixbuf_rotate(pixbuf, 0);
  picture->filename = (char *) g_strdup(filename);
  picture->ratio = ratio;
  picture->displayed_picture = NULL;
  picture->angle = angle;
  picture->mirrored = mirrored;
  picture->embedded = embedded;
  
  if (picture->original_picture == NULL) {
    fprintf(stderr, "new picture: Couldn't get enough memory for the new picture\n");
  }
  
  /* Clean w_current variables */
  /* \bug This should be done when canceling the action. */
  /* Pixbuf filename is not freed because we want to
     remember the last directory
  */
  /*
    if (w_current->current_pixbuf != NULL) {
      g_object_unref(w_current->current_pixbuf);
    }
    w_current->current_pixbuf = NULL;
    w_current->pixbuf_wh_ratio = 0;
  */

  new_node->draw_func = picture_draw_func; 
  new_node->sel_func  = select_func;  

  /* compute the bounding picture */
  o_picture_recalc(w_current, new_node);
  
  /* add the object to the list */
  object_list = (OBJECT *) s_basic_link_object(new_node, object_list);
  
  return(object_list);
}

/*! \brief Recalculate picture coordinates in SCREEN units.
 *  \par Function Description
 *  This function recalculates the screen coords of the <B>o_current</B>
 *  parameter picture object from its world coords.
 *
 *  The picture coordinates and its bounding are recalculated as well
 *  as the #OBJECT specific fields (line width, filling ...).
 *
 *  \param [in] w_current      The TOPLEVEL object.
 *  \param [in,out] o_current  Picture OBJECT to be recalculated.
 */
void o_picture_recalc(TOPLEVEL *w_current, OBJECT *o_current)
{
  int left, top, right, bottom;
  int screen_x1, screen_y1;
  int screen_x2, screen_y2;

  if (o_current->picture == NULL) {
    return;
  }

  /* update the screen coords of the upper left corner of the picture */
  WORLDtoSCREEN(w_current,
		o_current->picture->upper_x, o_current->picture->upper_y, 
		&screen_x1, &screen_y1);  
  o_current->picture->screen_upper_x = screen_x1;
  o_current->picture->screen_upper_y = screen_y1;
  
  /* update the screen coords of the lower right corner of the picture */
  WORLDtoSCREEN(w_current,
		o_current->picture->lower_x, o_current->picture->lower_y, 
		&screen_x2, &screen_y2);  
  o_current->picture->screen_lower_x = screen_x2;
  o_current->picture->screen_lower_y = screen_y2;
  
  /* update the bounding picture - screen unit */
  get_picture_bounds(w_current, o_current->picture,
		     &left, &top, &right, &bottom);
  o_current->left   = left;
  o_current->top    = top;
  o_current->right  = right;
  o_current->bottom = bottom;
  
  /* recalc OBJECT specific parameters */
  o_object_recalc(w_current, o_current);

}

/*! \brief Get picture bounding rectangle.
 *  \par Function Description 
 *  This function sets the <B>left</B>, <B>top</B>, <B>right</B> and
 *  <B>bottom</B> parameters to the boundings of the picture object described
 *  in <B>*picture</B> in SCREEN units.
 *
 *  \param [in]  w_current  The TOPLEVEL object.
 *  \param [in]  picture    Picture OBJECT to read coordinates from.
 *  \param [out] left       Left picture coordinate in SCREEN units.
 *  \param [out] top        Top picture coordinate in SCREEN units.
 *  \param [out] right      Right pircture coordinate in SCREEN units.
 *  \param [out] bottom     Bottom picture coordinate in SCREEN units.
 */
void get_picture_bounds(TOPLEVEL *w_current, PICTURE *picture,
			int *left, int *top, int *right, int *bottom)
{
  *left   = picture->screen_upper_x;
  *top    = picture->screen_upper_y;
  *right  = picture->screen_lower_x;
  *bottom = picture->screen_lower_y;

}

/*! \brief Get picture bounding rectangle in WORLD coordinates.
 *  \par Function Description
 *  This function sets the <B>left</B>, <B>top</B>, <B>right</B> and
 *  <B>bottom</B> parameters to the boundings of the picture object
 *  described in <B>*picture</B> in WORLD units.
 *
 *  \param [in]  w_current  The TOPLEVEL object.
 *  \param [in]  picture    Picture OBJECT to read coordinates from.
 *  \param [out] left       Left picture coordinate in WORLD units.
 *  \param [out] top        Top picture coordinate in WORLD units.
 *  \param [out] right      Right picture coordinate in WORLD units.
 *  \param [out] bottom     Bottom picture coordinate in WORLD units.
 */
void world_get_picture_bounds(TOPLEVEL *w_current, PICTURE *picture,
			      int *left, int *top, int *right, int *bottom)
{
	/* pb20011002 - why using min and max here and not above ? */
	*left   = min(picture->upper_x, picture->lower_x);
	*top    = min(picture->upper_y, picture->lower_y);
	*right  = max(picture->upper_x, picture->lower_x);
	*bottom = max(picture->upper_y, picture->lower_y);
	
	/* PB : same as above here for width of edges */	

#if DEBUG 
	printf("picture: %d %d %d %d\n", *left, *top, *right, *bottom);
#endif
	
}
                 
/*! \brief Modify the description of a picture OBJECT.
 *  \par Function Description
 *  This function modifies the coordinates of one of the four corner of
 *  the picture. The new coordinates of the corner identified by
 *  <B>whichone</B> are given by <B>x</B> and <B>y</B> in world unit.
 *
 *  The coordinates of the corner is modified in the world coordinate system.
 *  Screen coordinates and boundings are then updated.
 *
 *  \param [in]     w_current  The TOPLEVEL object.
 *  \param [in,out] object     Picture OBJECT to modify.
 *  \param [in]     x          New x coordinate.
 *  \param [in]     y          New y coordinate.
 *  \param [in]     whichone   Which picture parameter to modify.
 *
 *  <B>whichone</B> can have the following values:
 *  <DL>
 *    <DT>*</DT><DD>PICTURE_UPPER_LEFT
 *    <DT>*</DT><DD>PICTURE_LOWER_LEFT
 *    <DT>*</DT><DD>PICTURE_UPPER_RIGHT
 *    <DT>*</DT><DD>PICTURE_LOWER_RIGHT
 *  </DL>
 *
 *  \par Author's note
 *  pb20011002 - rewritten : old one did not used x, y and whichone
 */
void o_picture_modify(TOPLEVEL *w_current, OBJECT *object, 
		      int x, int y, int whichone)
{
  int tmp;
  
  /* change the position of the selected corner */
  switch(whichone) {
    case PICTURE_UPPER_LEFT:
      object->picture->upper_x = x;
      tmp = abs(object->picture->upper_x - object->picture->lower_x) / 
	object->picture->ratio;
      if (y < object->picture->lower_y) {
	tmp = -tmp;
      }
      object->picture->upper_y = object->picture->lower_y + tmp;
      break;
			
    case PICTURE_LOWER_LEFT:
      object->picture->upper_x = x;
      tmp = abs(object->picture->upper_x - object->picture->lower_x) / 
	object->picture->ratio;
      if (y > object->picture->upper_y) {
	tmp = -tmp;
      }
      object->picture->lower_y = object->picture->upper_y - tmp;
      break;
      
    case PICTURE_UPPER_RIGHT:
      object->picture->lower_x = x;
      tmp = abs(object->picture->upper_x - object->picture->lower_x) / 
	object->picture->ratio;
      if (y < object->picture->lower_y) {
	tmp = -tmp;
      }
      object->picture->upper_y = object->picture->lower_y + tmp;
      break;
      
    case PICTURE_LOWER_RIGHT:
      object->picture->lower_x = x;
      tmp = abs(object->picture->upper_x - object->picture->lower_x) / 
	object->picture->ratio;
      if (y > object->picture->upper_y) {
	tmp = -tmp;
      }
      object->picture->lower_y = object->picture->upper_y - tmp;
      break;
      
    default:
      return;
  }
  
  /* need to update the upper left and lower right corners */
  if(object->picture->upper_x > object->picture->lower_x) {
    tmp                      = object->picture->upper_x;
    object->picture->upper_x = object->picture->lower_x;
    object->picture->lower_x = tmp;
  }
  
  if(object->picture->upper_y < object->picture->lower_y) {
    tmp                      = object->picture->upper_y;
    object->picture->upper_y = object->picture->lower_y;
    object->picture->lower_y = tmp;
  }
	
  /* recalculate the screen coords and the boundings */
  o_picture_recalc(w_current, object);
}

/*! \brief Rotate a picture OBJECT.
 *  \par Function Description
 *  This function applies a rotation of center (<B>centerx</B>, <B>centery</B>)
 *  and angle <B>angle</B> to the picture object <B>*object</B>.
 *  The coordinates of the rotation center are in screen units. 
 *  <B>angle</B> must be a 90 degree multiple. If not, no rotation is applied.
 *
 *  The rotation is made with the #o_picture_rotate_world() function
 *  that perform a rotation of angle <B>angle</B> and center
 *  (<B>world_centerx</B>, <B>world_centery</B>) in world unit.
 *
 *  \param [in]     w_current  The TOPLEVEL object.
 *  \param [in]     centerx    Rotation center x coordinate in SCREEN units.
 *  \param [in]     centery    Rotation center y coordinate in SCREEN units.
 *  \param [in]     angle      Rotation angle in degrees (unused).
 *  \param [in,out] object     Picture OBJECT to rotate.
 *
 *  \note
 *  takes in screen coordinates for the centerx,y, and then does the rotate 
 *  in world space
 *  also ignores angle argument... for now, rotate only in 90 degree 
 *  increments
 *  fixed to 90 degrees... it's *not* general now
 */
void o_picture_rotate(TOPLEVEL *w_current,
		      int centerx, int centery, int angle,
		      OBJECT *object)
{
  int world_centerx, world_centery;

  /* convert the center of rotation to world unit */
  SCREENtoWORLD(w_current,
				centerx, centery, 
                &world_centerx, &world_centery);  

  /* rotate the picture */
  /* the check of the rotation angle is in o_picture_rotate_world() */
  o_picture_rotate_world(w_current,
					 world_centerx, world_centery, angle,
					 object);

  /* screen coords and boundings are updated by _rotate_world() */
  
}

/*! \brief Rotate picture OBJECT using WORLD coordinates.
 *  \par Function Description 
 *  This function rotates the picture described by <B>*object</B> around
 *  the (<B>world_centerx</B>, <B>world_centery</B>) point by <B>angle</B>
 *  degrees.
 *  The center of rotation is in world units.
 *
 *  \param [in]      w_current      The TOPLEVEL object.
 *  \param [in]      world_centerx  Rotation center x coordinate in
 *                                  WORLD units.
 *  \param [in]      world_centery  Rotation center y coordinate in
 *                                  WORLD units.
 *  \param [in]      angle          Rotation angle in degrees (See note below).
 *  \param [in,out]  object         Picture OBJECT to rotate.
 */
void o_picture_rotate_world(TOPLEVEL *w_current, 
			    int world_centerx, int world_centery, int angle,
			    OBJECT *object)
{
  int newx1, newy1;
  int newx2, newy2;
  
  /* Only 90 degree multiple and positive angles are allowed. */
  /* angle must be positive */
  if(angle < 0) angle = -angle;
  /* angle must be a 90 multiple or no rotation performed */
  if((angle % 90) != 0) return;
  
  object->picture->angle = ( object->picture->angle + angle ) % 360;
	
  /* The center of rotation (<B>world_centerx</B>, <B>world_centery</B>) is
   * translated to the origin. The rotation of the upper left and lower
   * right corner are then performed. Finally, the rotated picture is
   * translated back to its previous location.
   */
  /* translate object to origin */
  object->picture->upper_x -= world_centerx;
  object->picture->upper_y -= world_centery;
  object->picture->lower_x -= world_centerx;
  object->picture->lower_y -= world_centery;
  
  /* rotate the upper left corner of the picture */
  rotate_point_90(object->picture->upper_x, object->picture->upper_y, angle,
		  &newx1, &newy1);
  
  /* rotate the lower left corner of the picture */
  rotate_point_90(object->picture->lower_x, object->picture->lower_y, angle,
		  &newx2, &newy2);
  
  /* reorder the corners after rotation */
  object->picture->upper_x = min(newx1,newx2);
  object->picture->upper_y = max(newy1,newy2);
  object->picture->lower_x = max(newx1,newx2);
  object->picture->lower_y = min(newy1,newy2);
  
  /* translate object back to normal position */
  object->picture->upper_x += world_centerx;
  object->picture->upper_y += world_centery;
  object->picture->lower_x += world_centerx;
  object->picture->lower_y += world_centery;
  
  /* recalc boundings and screen coords */
  o_picture_recalc(w_current, object);
	
}

/*! \brief Mirror a picture.
 *  \par Function Description
 *  This function mirrors the picture from the point
 *  (<B>centerx</B>,<B>centery</B>) in screen unit.
 *
 *  The origin of the mirror in screen unit is converted in world unit.
 *  The picture is mirrored with the function #o_picture_mirror_world()
 *  for which the origin of the mirror must be given in world unit.
 *
 *  \param [in]     w_current  The TOPLEVEL object.
 *  \param [in]     centerx    Origin x coordinate in WORLD units.
 *  \param [in]     centery    Origin y coordinate in WORLD units.
 *  \param [in,out] object     Picture OBJECT to mirror.
 */
void o_picture_mirror(TOPLEVEL *w_current,
		      int centerx, int centery,
		      OBJECT *object)
{
  int world_centerx, world_centery;

  /* convert the origin of mirror */
  SCREENtoWORLD(w_current, centerx, centery, 
                &world_centerx,
                &world_centery);  

  /* apply the mirror in world coords */
  o_picture_mirror_world(w_current,
			 world_centerx, world_centery,
			 object);

  /* screen coords and boundings are updated by _mirror_world() */
  
}

/*! \brief Mirror a picture using WORLD coordinates.
 *  \par Function Description
 *  This function mirrors the picture from the point
 *  (<B>world_centerx</B>,<B>world_centery</B>) in world unit.
 *
 *  The picture is first translated to the origin, then mirrored and
 *  finally translated back at its previous position.
 *
 *  \param [in]     w_current      The TOPLEVEL object.
 *  \param [in]     world_centerx  Origin x coordinate in WORLD units.
 *  \param [in]     world_centery  Origin y coordinate in WORLD units.
 *  \param [in,out] object         Picture OBJECT to mirror.
 */
void o_picture_mirror_world(TOPLEVEL *w_current,
			    int world_centerx, int world_centery,
			    OBJECT *object)
{
  int newx1, newy1;
  int newx2, newy2;

  
  /* Set info in object */
  object->picture->mirrored = (object->picture->mirrored ^ 1) & 1;

  /* translate object to origin */
  object->picture->upper_x -= world_centerx;
  object->picture->upper_y -= world_centery;
  object->picture->lower_x -= world_centerx;
  object->picture->lower_y -= world_centery;

  /* mirror the corners */
  newx1 = -object->picture->upper_x;
  newy1 = object->picture->upper_y;
  newx2 = -object->picture->lower_x;
  newy2 = object->picture->lower_y;

  /* reorder the corners */
  object->picture->upper_x = min(newx1,newx2);
  object->picture->upper_y = max(newy1,newy2);
  object->picture->lower_x = max(newx1,newx2);
  object->picture->lower_y = min(newy1,newy2);

  /* translate back in position */
  object->picture->upper_x += world_centerx;
  object->picture->upper_y += world_centery;
  object->picture->lower_x += world_centerx;
  object->picture->lower_y += world_centery;

  /* recalc boundings and screen coords */
  o_picture_recalc(w_current, object);
  
}

/*! \brief Translate a picture position by a delta.
 *  \par Function Description
 *  This function applies a translation of (<B>dx</B>,<B>dy</B>) to the picture
 *  described by <B>*object</B>. <B>dx</B> and <B>dy</B> are in screen units.
 *
 *  The translation vector is converted in world unit. The translation is 
 *  made with #o_picture_translate_world().
 *
 *  \param [in]     w_current   The TOPLEVEL object.
 *  \param [in]     dx          x distance to move.
 *  \param [in]     dy          y distance to move.
 *  \param [in,out] object      Picture OBJECT to translate.
 */
void o_picture_translate(TOPLEVEL *w_current, int dx, int dy, OBJECT *object)
{
  int world_dx, world_dy;

  if (object == NULL) printf("bt NO!\n");

  /* convert the translation vector in world unit */
  world_dx = SCREENabs(w_current, dx);
  world_dy = SCREENabs(w_current, dy);

  /* translate the picture */
  o_picture_translate_world(w_current, world_dx, world_dy, object);

  /* screen coords and boundings are updated by _translate_world */
  
}

/*! \brief Translate a picture position in WORLD coordinates by a delta.
 *  \par Function Description
 *  This function applies a translation of (<B>x1</B>,<B>y1</B>) to the picture
 *  described by <B>*object</B>. <B>x1</B> and <B>y1</B> are in world units.
 *
 *  \param [in]     w_current  The TOPLEVEL object.
 *  \param [in]     x1         x distance to move.
 *  \param [in]     y1         y distance to move.
 *  \param [in,out] object     Picture OBJECT to translate.
 */
void o_picture_translate_world(TOPLEVEL *w_current,
			       int x1, int y1, OBJECT *object)
{
  if (object == NULL) printf("btw NO!\n");

  /* Do world coords */
  object->picture->upper_x = object->picture->upper_x + x1;
  object->picture->upper_y = object->picture->upper_y + y1;
  object->picture->lower_x = object->picture->lower_x + x1;
  object->picture->lower_y = object->picture->lower_y + y1;     
  
  /* recalc the screen coords and the bounding picture */
  o_picture_recalc(w_current, object);
}

/*! \brief Create a copy of a picture.
 *  \par Function Description
 *  This function creates a verbatim copy of the object pointed by
 *  <B>o_current</B> describing a picture. The new object is added at the
 *  end of the list, following the <B>list_tail</B> pointed object.
 *
 *  \param [in]  w_current  The TOPLEVEL object.
 *  \param [out] list_tail  OBJECT list to copy to.
 *  \param [in]  o_current  Picture OBJECT to copy.
 *  \return A new pointer to the end of the object list.
 */
OBJECT *o_picture_copy(TOPLEVEL *w_current, OBJECT *list_tail,
		       OBJECT *o_current)
{
  OBJECT *new_obj;
  ATTRIB *a_current;
  int color;
	
  if (o_current->saved_color == -1) {
    color = o_current->color;
  } else {
    color = o_current->saved_color;
  }

  /* A new picture object is added at the end of the object list with
   * #o_picture_add(). Values for its fields are default and need to
   * be modified.
   */

  /* create and link a new picture object */	
  new_obj = o_picture_add(w_current, list_tail,
			  w_current->current_pixbuf, 
			  w_current->pixbuf_filename, 
			  w_current->pixbuf_wh_ratio,
			  OBJ_PICTURE,
			  0, 0, 0, 0, 0, FALSE, FALSE);

  /* The dimensions of the new picture are set with the ones of the
   * original picture. The two picturees have the same line type and
   * the same filling options.
   *
   * The coordinates and the values in screen unit are computed with
   * #o_picture_recalc().
   */
  /* modifying */
  /* pb20011002 - have to check if o_current is a picture object */
  new_obj->picture->upper_x = o_current->picture->upper_x;
  new_obj->picture->upper_y = o_current->picture->upper_y;
  new_obj->picture->lower_x = o_current->picture->lower_x;
  new_obj->picture->lower_y = o_current->picture->lower_y;
  
  /* Copy the picture data */
  new_obj->picture->original_picture = 
    gdk_pixbuf_scale_simple(o_current->picture->original_picture,
			    gdk_pixbuf_get_width (o_current->picture->original_picture),
			    gdk_pixbuf_get_height (o_current->picture->original_picture),
			    GDK_INTERP_BILINEAR);
  new_obj->picture->displayed_picture = 
    gdk_pixbuf_scale_simple(o_current->picture->displayed_picture,
			    gdk_pixbuf_get_width (o_current->picture->displayed_picture),
			    gdk_pixbuf_get_height (o_current->picture->displayed_picture),
			    GDK_INTERP_BILINEAR);

  new_obj->picture->ratio = o_current->picture->ratio;
  new_obj->picture->filename = g_strdup(o_current->picture->filename);
  
  new_obj->picture->angle = o_current->picture->angle;
  new_obj->picture->mirrored = o_current->picture->mirrored;
  
  o_picture_recalc(w_current, new_obj);
  
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

/*! \brief Get RGB data from image.
 *  \par Function Description
 *  This function returns the RGB data of the given image.
 *  Function taken from the DIA source code (http://www.gnome.org/projects/dia)
 *  and licensed under the GNU GPL version 2.
 *
 *  \param [in] image  GdkPixbuf image to read RGB data from.
 *  \return Array of rgb data from image.
 *
 *  \note
 *  Caller must free returned guint8 array.
 */
guint8 *o_picture_rgb_data(GdkPixbuf *image)
{
  int width = gdk_pixbuf_get_width(image);
  int height = gdk_pixbuf_get_height(image);
  int rowstride = gdk_pixbuf_get_rowstride(image);
  int size = height*rowstride;
  guint8 *rgb_pixels = g_malloc(size);

  if (gdk_pixbuf_get_has_alpha(image)) {
    guint8 *pixels = gdk_pixbuf_get_pixels(image);
    int i, j;
    for (i = 0; i < height; i++) {
      for (j = 0; j < width; j++) {
	rgb_pixels[i*rowstride+j*3] = pixels[i*rowstride+j*4];
	rgb_pixels[i*rowstride+j*3+1] = pixels[i*rowstride+j*4+1];
	rgb_pixels[i*rowstride+j*3+2] = pixels[i*rowstride+j*4+2];
      }
    }
    return rgb_pixels;
  } else {
    guint8 *pixels = gdk_pixbuf_get_pixels(image);

    g_memmove(rgb_pixels, pixels, height*rowstride);
    return rgb_pixels;
  }
}

/*! \brief Get mask data from image.
 *  \par Function Description
 *  This function returns the mask data of the given image.
 *  Function taken from the DIA source code (http://www.gnome.org/projects/dia)
 *  and licensed under the GNU GPL version 2.
 *
 *  \param [in] image  GdkPixbuf image to get mask data from.
 *  \return Array of mask data from image.
 *
 *  \note
 *  Caller must free returned guint8 array.
 */
guint8 *o_picture_mask_data(GdkPixbuf *image)
{
  guint8 *pixels;
  guint8 *mask;
  int i, size;

  if (!gdk_pixbuf_get_has_alpha(image)) {
    return NULL;
  }
  
  pixels = gdk_pixbuf_get_pixels(image);

  size = gdk_pixbuf_get_width(image)*
    gdk_pixbuf_get_height(image);

  mask = g_malloc(size);

  /* Pick every fourth byte (the alpha channel) into mask */
  for (i = 0; i < size; i++)
    mask[i] = pixels[i*4+3];

  return mask;
}

/*! \brief Print picture to Postscript document.
 *  \par Function Description
 *  This function prints a picture object. The picture is defined by the
 *  coordinates of its upper left corner in (<B>x</B>,<B>y</B>) and its width
 *  and height given by the <B>width</B> and <B>height</B> parameters. 
 *  The Postscript document is defined by the file pointer <B>fp</B>.
 *  Function based on the DIA source code (http://www.gnome.org/projects/dia)
 *  and licensed under the GNU GPL version 2.
 *
 *  All dimensions are in mils.
 *  
 *  \param [in] w_current  The TOPLEVEL object.
 *  \param [in] fp         FILE pointer to Postscript document.
 *  \param [in] o_current  Picture OBJECT to write to document.
 *  \param [in] origin_x   Page x coordinate to place picture OBJECT.
 *  \param [in] origin_y   Page y coordinate to place picture OBJECT.
 */
void o_picture_print(TOPLEVEL *w_current, FILE *fp, OBJECT *o_current, 
		     int origin_x, int origin_y)
{
  int x1, y1, x, y;
  int height, width;
  GdkPixbuf* image = o_current->picture->displayed_picture;
  int img_width, img_height, img_rowstride;
  double ratio;
  guint8 *rgb_data;
  guint8 *mask_data;

  /* calculate the width and height of the box */
  width  = abs(o_current->picture->lower_x - o_current->picture->upper_x); 
  height = abs(o_current->picture->upper_y - o_current->picture->lower_y);

  /* calculate the origin of the box */
  x1 = o_current->picture->upper_x;
  y1 = o_current->picture->upper_y;

  img_width = gdk_pixbuf_get_width(image);
  img_rowstride = gdk_pixbuf_get_rowstride(image);
  img_height = gdk_pixbuf_get_height(image);

  rgb_data = o_picture_rgb_data(image);
  mask_data = o_picture_mask_data(image);

  ratio = height/width;

  fprintf(fp, "gsave\n");

  /* color output only */
  fprintf(fp, "/pix %i string def\n", img_width * 3);
  fprintf(fp, "%i %i 8\n", img_width, img_height);
  fprintf(fp, "%i %i translate\n", x1, y1);
  fprintf(fp, "%i %i scale\n", width, height);
  fprintf(fp, "[%i 0 0 -%i 0 0]\n", img_width, img_height);

  fprintf(fp, "{currentfile pix readhexstring pop}\n");
  fprintf(fp, "false 3 colorimage\n");
  fprintf(fp, "\n");

  if (mask_data) {
    for (y = 0; y < img_height; y++) {
      for (x = 0; x < img_width; x++) {
	int i = y*img_rowstride+x*3;
	int m = y*img_width+x;
        fprintf(fp, "%02x", 255-(mask_data[m]*(255-rgb_data[i])/255));
        fprintf(fp, "%02x", 255-(mask_data[m]*(255-rgb_data[i+1])/255));
        fprintf(fp, "%02x", 255-(mask_data[m]*(255-rgb_data[i+2])/255));
      }
      fprintf(fp, "\n");
    }
  } else {
    for (y = 0; y < img_height; y++) {
      for (x = 0; x < img_width; x++) {
	int i = y*img_rowstride+x*3;
        fprintf(fp, "%02x", (int)(rgb_data[i]));
        fprintf(fp, "%02x", (int)(rgb_data[i+1]));
        fprintf(fp, "%02x", (int)(rgb_data[i+2]));
      }
      fprintf(fp, "\n");
    }
  }
  fprintf(fp, "grestore\n");
  fprintf(fp, "\n");
   
  g_free(rgb_data);
  g_free(mask_data);

	
}

