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
#include <unistd.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <glib.h>

#include <libgeda/libgeda.h>

#include "../include/globals.h"
#include "../include/prototype.h"
#include "../include/x_dialog.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

#define X_IMAGE_DEFAULT_SIZE "800x600"

#ifndef HAS_LIBGD
#define X_IMAGE_SIZE_MENU_NAME "image_size_menu"
#define X_IMAGE_TYPE_MENU_NAME "image_type_menu"

#define X_IMAGE_DEFAULT_TYPE "PNG"
#endif

static char *x_image_sizes[] = {"320x240", "640x480", "800x600", "1200x768",
                                "1280x960", "1600x1200", "3200x2400", NULL};

#if ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION < 6))
/* gtk_combo_box_get_active_text was included in GTK 2.6, so we need to store
   the different image type descriptions in a list. */
GSList *image_type_descriptions = NULL;
#endif


#if ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION < 6))
static void free_image_type_descriptions_list ()
{
  GSList *ptr;
  
  /* Free the data stored in each node */
  ptr = image_type_descriptions;
  while (ptr) {
    g_free(ptr->data);
    ptr->data = NULL;
    ptr = g_slist_next(ptr);
  }

  /* Free the list */
  if (!ptr)
    g_slist_free(image_type_descriptions);
  image_type_descriptions = NULL;
}

#endif

/*! \brief Create the options of the image size combobox
 *  \par This function adds the options of the image size to the given combobox.
 *  \param combo [in] the combobox to add the options to.
 *  \return nothing
 *  \note
 *  This function is only used in this file, there are other create_menus...
 */
static void create_size_menu (GtkComboBox *combo)
{
  char *buf;
  char *default_size;
  int i, default_index = 0;

  default_size = g_strdup_printf(X_IMAGE_DEFAULT_SIZE);
  for (i=0; x_image_sizes[i] != NULL;i++) {
    /* Create a new string and add it as an option*/
    buf = g_strdup_printf(x_image_sizes[i]);
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), buf);

    /* Compare with the default size, to get the default index */
    if (strcasecmp(buf, default_size ) == 0) {
      default_index = i;
    }
    g_free(buf);
  }
  g_free(default_size);

  /* Set the default menu */
  gtk_combo_box_set_active(GTK_COMBO_BOX (combo), default_index);

  return;
}

/*! \brief Create the options of the image type combobox
 *  \par This function adds the options of the image type to the given combobox.
 *  \param combo [in] the combobox to add the options to.
 *  \return nothing
 *  \note
 *  This function is only used in this file, there are other create_menus...
 */
static void create_type_menu(GtkComboBox *combo)
{
#ifndef HAS_LIBGD
  GSList *formats = gdk_pixbuf_get_formats ();
  GSList *ptr;
  char *buf;
  int i=0, default_index=0;

#if ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION < 6))
  /* If GTK < 2.6, free the descriptions list */
  free_image_type_descriptions_list();
#endif

  ptr = formats;
  while (ptr) {
    if (gdk_pixbuf_format_is_writable (ptr->data)) {
      /* Get the format description and add it to the menu */
      buf = g_strdup_printf(gdk_pixbuf_format_get_description(ptr->data));
      gtk_combo_box_append_text (GTK_COMBO_BOX (combo), buf);

      /* If GTK < 2.6, then add it also to the descriptions list. */
#if ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION < 6))
      image_type_descriptions = g_slist_append(image_type_descriptions,
					       buf);
#endif

      /* Compare the name with "png" and store the index */
      buf = g_strdup_printf(gdk_pixbuf_format_get_name(ptr->data));
      if (strcasecmp(buf, X_IMAGE_DEFAULT_TYPE) == 0) {
	default_index = i;
      }
      g_free(buf);
    }
    i++;
    ptr = ptr->next;
  }
  g_slist_free (formats);
  
  /* Set the default menu */
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo), default_index);
#else
  gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "PNG");

  /* Set the default menu */
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
#endif
  return;
}

/*! \brief Given a gdk-pixbuf image type description, it returns the type, 
 *  or extension of the image.
 *  \par Return the gdk-pixbuf image type, or extension, which has the
 *  given gdk-pixbuf description.
 *  \param description The gdk-pixbuf image type description.
 *  \return The gdk-pixbuf type, or extension, of the image.
 *  \note This function is only used in this file.
 */
static char *x_image_get_type_from_description(char *description) {
#ifndef HAS_LIBGD
  gchar *descr = g_strdup_printf(description);
  GSList *formats = gdk_pixbuf_get_formats ();
  GSList *ptr;
  gchar *ptr_descr;

  ptr = formats;
  while (ptr) {
    ptr_descr = gdk_pixbuf_format_get_description (ptr->data);
    if (strcasecmp(ptr_descr, descr) == 0) {
      g_free(descr);
      return(gdk_pixbuf_format_get_name(ptr->data));
    }

    ptr = ptr->next;
  }
  g_free (descr);
  return NULL;  
#else
  /* When using libgd, there is only one supported type: PNG */
  return(g_strdup("png"));
#endif

}

/*! \brief Update the filename of a file dialog, when the image type has changed.
 *  \par Given a combobox inside a file chooser dialog, this function updates
 *  the filename displayed by the dialog, removing the current extension, and
 *  adding the extension of the image type selected.
 *  \param combo [in] A combobox inside a file chooser dialog, with gdk-pixbuf image type descriptions.
 *  \param w_current [in] the TOPLEVEL structure.
 *  \return nothing.
 * 
 */
static void x_image_update_dialog_filename(GtkComboBox *combo, 
					  TOPLEVEL *w_current) {

  char* image_type_descr = NULL;
  char *image_type = NULL;
  char *old_image_filename = NULL;
  char *file_basename = NULL;
  char *file_name = NULL ;
  char *new_image_filename = NULL;
  GtkWidget *file_chooser;
  
  /* Get the current image type */
#if ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION < 6))
  GSList *ptr;
  /* If GTK < 2.6, get the description from the descriptions list */
  ptr = g_slist_nth(image_type_descriptions,
		   gtk_combo_box_get_active(GTK_COMBO_BOX(combo)));
  image_type_descr = (char *) (ptr->data);
#else
  image_type_descr = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo));
#endif
  image_type = x_image_get_type_from_description(image_type_descr);

  /* Get the parent dialog */
  file_chooser = gtk_widget_get_ancestor(GTK_WIDGET(combo),
					 GTK_TYPE_FILE_CHOOSER);

  /* Get the previous file name. If none, revert to the page filename */
  old_image_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));
  if (!old_image_filename) {
    old_image_filename = w_current->page_current->page_filename;
  }

  /* Get the file name, without extension */
  if (old_image_filename) {
    file_basename = g_path_get_basename(old_image_filename);

    if (g_strrstr(file_basename, ".") != NULL) {
      file_name = g_strndup(file_basename, 
			    g_strrstr(file_basename, ".") - file_basename);
    }
  }

  /* Add the extension */
  if (file_name) {
    new_image_filename = g_strdup_printf("%s.%s", file_name, 
					 image_type);
  } else {
    new_image_filename = g_strdup_printf("%s.%s", file_basename, 
					 image_type);    
  }

  /* Set the new filename */
  if (file_chooser) {
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(file_chooser),
				      new_image_filename);
  } else {
    s_log_message("x_image_update_dialog_filename: No parent file chooser found!.\n");
    fprintf(stderr, "x_image_update_dialog_filename: No parent file chooser found!.\n");
  }

  g_free(file_name);
  g_free(file_basename);
  g_free(new_image_filename);
#ifdef HAS_LIBGD
  g_free(image_type);
#endif
}

/*! \brief Write the image file, with the desired options.
 *  \par This function writes the image file, with the options set in the
 *  dialog by the user.
 *  \param w_current [in] the TOPLEVEL structure.
 *  \param filename [in] the image filename.
 *  \param desired_width [in] the image width chosen by the user.
 *  \param desired_height [in] the image height chosen by the user.
 *  \param filetype [in] image filetype.
 *  \return nothing
 *
 */
void x_image_lowlevel(TOPLEVEL *w_current, const char* filename,
		      int desired_width, int desired_height, char *filetype)
{
  int width, height;
  int save_height, save_width;
  int save_page_left, save_page_right, save_page_top, save_page_bottom;
  int page_width, page_height, page_center_left, page_center_top;
#ifndef HAS_LIBGD
  GdkPixbuf *pixbuf;
  GError *gerror = NULL;
  GtkWidget *dialog;
#endif
  float prop;

  w_current->image_width = width = desired_width;
  w_current->image_height = height = desired_height;

  save_width = w_current->width;
  save_height = w_current->height;

  w_current->width = width;
  w_current->height = height;

  save_page_left = w_current->page_current->left;
  save_page_right = w_current->page_current->right;
  save_page_top = w_current->page_current->top;
  save_page_bottom = w_current->page_current->bottom;
  
  page_width = save_page_right - save_page_left;
  page_height = save_page_bottom - save_page_top;

  page_center_left = save_page_left + (page_width / 2);
  page_center_top = save_page_top + (page_height / 2);

  /* Preserve proportions */
  prop = (float)width / height;
  if(page_width > page_height) {
    page_height = (page_width / prop);
  }else{
    page_width = (page_height * prop);
  }

  /* need to do this every time you change width / height */
  set_window(w_current, w_current->page_current,
             page_center_left - (page_width / 2),
             page_center_left + (page_width / 2),
             page_center_top - (page_height / 2),
             page_center_top + (page_height / 2));

  /* de select everything first */
  o_select_run_hooks(w_current, NULL, 2); 
  o_selection_unselect_list(w_current,
			    &(w_current->page_current->selection_list));
		

#ifdef HAS_LIBGD
  /* try to use recalc here */
  o_redraw_all(w_current);

  f_image_write(w_current, filename, width, height, 
                w_current->image_color);
#else
  pixbuf = x_image_get_pixbuf(w_current);
  if (pixbuf != NULL) {
    if (!gdk_pixbuf_save(pixbuf, filename, filetype, &gerror, NULL)) {
      fprintf(stderr, "x_image_lowlevel: Unable to save %s file  %s.\n", 
	      filetype, filename);
      fprintf(stderr, "%s\n", gerror->message);
      s_log_message(_("x_image_lowlevel: Unable to write %s file %s.\n"),
		    filetype, filename);
      s_log_message(gerror->message);

      /* Warn the user */
      dialog = gtk_message_dialog_new (GTK_WINDOW(w_current->main_window),
				       GTK_DIALOG_MODAL
				       | GTK_DIALOG_DESTROY_WITH_PARENT,
				       GTK_MESSAGE_ERROR,
				       GTK_BUTTONS_OK,
				       _("There was the following error when saving image with type %s to filename:\n%s\n\n%s.\n"),
				       filetype, filename, gerror->message
					 );
      
      gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);

      /* Free the gerror */
      g_error_free(gerror); 
      gerror = NULL;

      /* Unlink the output file */
      /* It's not safe to unlink the file if there was an error.
	 For example: if the operation was not allowed due to permissions, 
	 the _previous existing_ file will be removed */
      /* unlink(filename); */
    }
    else {
      if (w_current->image_color == TRUE) {
	s_log_message(_("Wrote color image to [%s] [%d x %d]\n"), filename, width, height);
      } else {
	s_log_message(_("Wrote black and white image to [%s] [%d x %d]\n"), filename, width, height);
      }
    }
    if (filetype != NULL)
      g_free(filetype);
    if (pixbuf != NULL)
      g_object_unref(pixbuf); 
  }
  else {
    fprintf(stderr, "x_image_lowlevel: Unable to get pixbuf from gschem's window.\n");
    s_log_message(_("x_image_lowlevel: Unable to get pixbuf from gschem's window.\n"));
  }
#endif

  w_current->width = save_width;
  w_current->height = save_height;

  /* need to do this every time you change width / height */
  set_window(w_current, w_current->page_current,
             save_page_left,
             save_page_right,
             save_page_top,
             save_page_bottom);

  /* try to use recalc here... */
  o_redraw_all(w_current);

#ifdef HAS_LIBGD
  if (w_current->image_color == TRUE) {
    s_log_message(_("Wrote color image to [%s] [%d x %d]\n"), filename, width, height);
  } else {
    s_log_message(_("Wrote black and white image to [%s] [%d x %d]\n"), filename, width, height);
  }
#endif
}

/*! \brief Display the image file selection dialog.
 *  \par Display the image file selection dialog, allowing the user to
 *  set several options, like image size and image type.
 *  When the user hits "ok", then it writes the image file.
 *  \param w_current [in] the TOPLEVEL structure.
 *  \return nothing
 */
void x_image_setup (TOPLEVEL *w_current)
{
  GtkWidget *dialog;
  GtkWidget *vbox1;
  GtkWidget *hbox;
  GtkWidget *label1;
  GtkWidget *size_combo;
  GtkWidget *vbox2;
  GtkWidget *label2;
  GtkWidget *type_combo;
#ifndef HAS_LIBGD
  char *image_type_descr;
#endif
  char *filename;
  char *image_size;
  char *image_type;
  int width, height;

  hbox = gtk_hbox_new(FALSE, 0);
  
  /* Image size selection */
  vbox1 = gtk_vbox_new(TRUE, 0);
  label1 = gtk_label_new (_("Width x Height"));
  gtk_widget_show (label1);
  gtk_misc_set_alignment( GTK_MISC (label1), 0, 0);
  gtk_misc_set_padding (GTK_MISC (label1), 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox1),
		      label1, FALSE, FALSE, 0);
  
  size_combo =  gtk_combo_box_new_text ();
  create_size_menu (GTK_COMBO_BOX(size_combo));
  
  gtk_widget_show (size_combo);
  gtk_box_pack_start (GTK_BOX (vbox1), size_combo, TRUE, TRUE, 0);
  gtk_widget_show(vbox1);
  
  /* Image type selection */
  vbox2 = gtk_vbox_new(TRUE, 0);
  label2 = gtk_label_new (_("Image type"));
  gtk_widget_show (label2);
  gtk_misc_set_alignment( GTK_MISC (label2), 0, 0);
  gtk_misc_set_padding (GTK_MISC (label2), 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox2),
		      label2, FALSE, FALSE, 0);
  
  type_combo = gtk_combo_box_new_text ();
  gtk_box_pack_start (GTK_BOX (vbox2), type_combo, TRUE, TRUE, 0);
  create_type_menu (GTK_COMBO_BOX(type_combo));
  
  /* Connect the changed signal to the callback, so the filename
     gets updated every time the image type is changed */
  g_signal_connect (type_combo, "changed", 
		    G_CALLBACK(x_image_update_dialog_filename),
		    w_current);

  gtk_widget_show (type_combo);
  gtk_widget_show(vbox2);
  
  /* Create the dialog */
  dialog = gtk_file_chooser_dialog_new (_("Write image..."),
					GTK_WINDOW(w_current->main_window),
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE,   GTK_RESPONSE_ACCEPT,
					NULL);
  
  /* Add the extra widgets to the dialog*/
  gtk_box_pack_start(GTK_BOX(hbox), vbox1, FALSE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(hbox), vbox2, FALSE, FALSE, 10);

  gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER(dialog), hbox);
  
  g_object_set (dialog,
		/* GtkFileChooser */
		"select-multiple", FALSE,
#if ((GTK_MAJOR_VERSION > 2) || ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION >=8)))
		/* only in GTK 2.8 */
                "do-overwrite-confirmation", TRUE,
#endif
		  NULL);
  
  /* Update the filename */
  x_image_update_dialog_filename(GTK_COMBO_BOX(type_combo), w_current);  

  gtk_dialog_set_default_response(GTK_DIALOG(dialog),
				  GTK_RESPONSE_ACCEPT);

  gtk_window_position (GTK_WINDOW (dialog),
		       GTK_WIN_POS_MOUSE);

  gtk_container_set_border_width(GTK_CONTAINER(dialog),
				 DIALOG_BORDER_SPACING);
  gtk_box_set_spacing(GTK_BOX(GTK_DIALOG(dialog)->vbox), 
		      DIALOG_V_SPACING);

  gtk_widget_show (dialog);
  
  if (gtk_dialog_run((GTK_DIALOG(dialog))) == GTK_RESPONSE_ACCEPT) {    
#if ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION < 6))
    image_size = 
      x_image_sizes[gtk_combo_box_get_active(GTK_COMBO_BOX(size_combo))];
#else
    image_size = gtk_combo_box_get_active_text(GTK_COMBO_BOX(size_combo));
#endif

#ifndef HAS_LIBGD
#if ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION < 6))
  GSList *ptr;
  /* If GTK < 2.6, get the description from the descriptions list */
  ptr = g_slist_nth(image_type_descriptions,
		   gtk_combo_box_get_active(GTK_COMBO_BOX(type_combo)));
  image_type_descr = (char *) (ptr->data);
#else
    image_type_descr = gtk_combo_box_get_active_text(GTK_COMBO_BOX(type_combo));
#endif

    image_type = x_image_get_type_from_description(image_type_descr);
#else
    /* Image type not used when using libgd */
    image_type = NULL;
#endif
    sscanf(image_size, "%ix%i", &width, &height);
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

    x_image_lowlevel(w_current, filename, width, height, image_type);
  }
  
#if ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION < 6))
  /* If GTK < 2.6, free the descriptions list */
  free_image_type_descriptions_list();
#endif

  gtk_widget_destroy (dialog);
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
static void x_image_convert_to_greyscale(GdkPixbuf *pixbuf)
{
  int width, height, rowstride, n_channels;
  guchar *pixels, *p, new_value;
  int i, j;

  n_channels = gdk_pixbuf_get_n_channels (pixbuf);
  
  if (n_channels != 3)
  {
    return;
  }
 
  if (gdk_pixbuf_get_colorspace (pixbuf) != GDK_COLORSPACE_RGB)
  {
    return;
  }

  if (gdk_pixbuf_get_bits_per_sample (pixbuf) != 8)
  {
    return;
  }

  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);

  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  pixels = gdk_pixbuf_get_pixels (pixbuf);

  for (j = 0; j < height; j++)
  {
    for (i = 0; i < width; i++)
    {
      p = pixels + j * rowstride + i * n_channels;

      new_value = 0.3 * p[0] + 0.59 * p[1] + 0.11 * p[2];
      p[0] = new_value;
      p[1] = new_value;
      p[2] = new_value;
    }
  }
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
GdkPixbuf *x_image_get_pixbuf (TOPLEVEL *w_current)
{
  GdkPixbuf *pixbuf;
  int origin_x, origin_y, bottom, right;
  int size_x, size_y, s_right, s_left, s_top,s_bottom;
  TOPLEVEL toplevel;

  /* Do a copy of the toplevel struct and work with it */
  memcpy(&toplevel, w_current, sizeof(TOPLEVEL));

  WORLDtoSCREEN(&toplevel, 
		w_current->page_current->right,
		w_current->page_current->left,
		&s_right,
		&s_left);
  WORLDtoSCREEN(&toplevel, 
		w_current->page_current->bottom,
		w_current->page_current->top,
		&s_bottom,
		&s_top);

  size_x = s_left - s_right;
  size_y = s_bottom - s_top;

  size_x = toplevel.image_width;
  size_y = toplevel.image_height;

  toplevel.window = gdk_pixmap_new(w_current->window, size_x, size_y, -1);
  toplevel.backingstore = gdk_pixmap_new(w_current->window, size_x, size_y, -1);
  toplevel.grid = 0;
  toplevel.text_origin_marker = FALSE;

  toplevel.display_width = toplevel.image_width;
  toplevel.display_height = toplevel.image_height;

  toplevel.win_width = toplevel.image_width;
  toplevel.win_height = toplevel.image_height;

  if (toplevel.image_color == FALSE)
  {
    /* We are going to be doing black&white (grayscale) output, so change the */
    /* color of all objects to a nice and dark color, say black */
    toplevel.override_color = BLACK;  

    /* also reset the background to white */
    toplevel.background_color = WHITE;
  }

  origin_x = origin_y = 0;
  right = size_x;
  bottom = size_y;

  /* ------------------  Begin optional code ------------------------ */
  /* If the the code in this region is commented, the PNG returned will
     be the same as the one returned using libgd.
     I mean: there will be some border all around the schematic.
     This code is used to adjust the schematic to the border of the image */
#if 0

  /* Do a zoom extents to get fit all the schematic in the window */
  /* Commented so the image returned will be the same as with libgd */  
  a_zoom_extents (&toplevel,
		  toplevel.page_current->object_head,
		  A_PAN_DONT_REDRAW);

  
  /* See if there are objects */
  
  aux = w_current->page_current->object_head;
  while (aux != NULL) {
    if (aux->type != -1) {
      object_found = 1;
      break;
    }
    aux = aux->next;
  }

  
  /* If there are no objects, can't use zoom_extents */
  if (object_found) {
    o_redraw_all (&toplevel); 
    get_object_list_bounds(&toplevel, 
			   toplevel.page_current->object_head, 
			   &origin_x, &origin_y, 
			   &right, &bottom);
  }
#endif
  /* ------------------  End optional code ------------------------ */
  
  o_redraw_all (&toplevel); 

  /* Get the pixbuf */
  pixbuf=gdk_pixbuf_get_from_drawable(NULL,toplevel.backingstore, NULL, 
				      origin_x, origin_y, 0, 0,           
				      right-origin_x, 
				      bottom-origin_y);

  if (toplevel.image_color == FALSE)
  {
    x_image_convert_to_greyscale(pixbuf); 
  }

  if (toplevel.window != NULL) {
    g_object_unref(toplevel.window);
  }
  if (toplevel.backingstore != NULL) {
    g_object_unref(toplevel.backingstore);
  }

  return(pixbuf);
}
