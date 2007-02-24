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

#include <libgeda/libgeda.h>

#include "../include/x_event.h"
#include "../include/i_vars.h"
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
void x_window_setup (TOPLEVEL *toplevel)
{
  PAGE *page;
  
  /* x_window_setup_rest() - BEGIN */
  toplevel->num_untitled=0;

  toplevel->start_x = -1;
  toplevel->start_y = -1;
  toplevel->save_x = -1;
  toplevel->save_y = -1;
  toplevel->last_x = -1;
  toplevel->last_y = -1;
  toplevel->loc_x = -1;
  toplevel->loc_y = -1;
  toplevel->distance = -1;
  toplevel->event_state = SELECT;
  toplevel->inside_action = 0;
  toplevel->snap = 1;
  toplevel->grid = 1;

  toplevel->show_hidden_text = 0;
  
  toplevel->complex_rotate = 0;

  toplevel->current_attribute = NULL;
  toplevel->current_visible = -1; /* not sure on these */
  toplevel->current_show = -1;

  toplevel->internal_basename=NULL;
  toplevel->internal_clib=NULL;

  toplevel->series_name = NULL;
  toplevel->untitled_name = NULL;
  toplevel->font_directory = NULL;
  toplevel->scheme_directory = NULL;
  toplevel->bitmap_directory = NULL;
  toplevel->bus_ripper_symname = NULL;

  toplevel->override_color = -1;
  toplevel->inside_redraw=0;

  toplevel->FORCE_CONN_UPDATE=0;
  toplevel->ADDING_SEL=0;
  toplevel->REMOVING_SEL=0;

  toplevel->drawbounding_action_mode=FREE;
  toplevel->last_drawb_mode = -1;
  toplevel->CONTROLKEY=0;
  toplevel->SHIFTKEY=0;
  toplevel->last_callback=NULL;

  toplevel->major_changed_refdes = NULL;

  toplevel->status_label = NULL;
  toplevel->middle_label = NULL;
  toplevel->grid_label = NULL;
  
  toplevel->cswindow = NULL;
  toplevel->aswindow = NULL;
  toplevel->fowindow = NULL;
  toplevel->sowindow = NULL;
  toplevel->fswindow = NULL;

  toplevel->tiwindow = NULL;
  toplevel->tewindow = NULL;
  toplevel->exwindow = NULL;
  toplevel->aawindow = NULL;
  toplevel->mawindow = NULL;
  toplevel->aewindow = NULL;
  toplevel->trwindow = NULL;
  toplevel->tswindow = NULL;
  toplevel->pswindow = NULL;
  toplevel->iwindow = NULL;
  toplevel->abwindow = NULL;
  toplevel->hkwindow = NULL;
  toplevel->cowindow = NULL;
  toplevel->clwindow = NULL;
  toplevel->ltwindow = NULL;
  toplevel->ftwindow = NULL;
  toplevel->sewindow = NULL;

  toplevel->coord_world = NULL;
  toplevel->coord_screen = NULL;
  toplevel->doing_pan=FALSE;
  /* toplevel->preview = NULL;experimental widget */

  toplevel->buffer_number=0;
  /* x_window_setup_rest() - END */

  /* immediately setup user params */
  i_vars_set(toplevel);

  /* Initialize the autosave callback */
  s_page_autosave_init(toplevel);

  /* make sure none of these events happen till we are done */
  toplevel->DONT_DRAW_CONN = 1;
  toplevel->DONT_RESIZE    = 1;
  toplevel->DONT_EXPOSE    = 1;
  toplevel->DONT_RECALC    = 1;

  /* X related stuff */
  toplevel->display_height = gdk_screen_height ();
  toplevel->display_width  = gdk_screen_width ();

  /* x_window_setup_world() - BEGIN */
  toplevel->init_left = -45;
  toplevel->init_top  = -45;
  /* init_right and _bottom are set before this function is called */
  toplevel->min_zoom  = 0;
  toplevel->max_zoom  = 256;  /* was 128 */

  toplevel->width  = default_width;
  toplevel->height = default_height;

  toplevel->starting_width = toplevel->width;

  toplevel->win_width  = toplevel->width;
  toplevel->win_height = toplevel->height;
  /* x_window_setup_world() - END */

  /* X related stuff */
  /* do X fill in first */
  x_window_create_main (toplevel);

  /* Now create a blank page */
  page = s_page_new (toplevel, "unknown");
  s_page_goto (toplevel, page);

  /* Do a zoom extents */
  a_zoom_extents(toplevel, toplevel->page_current->object_head, 0);
  
  o_undo_savestate(toplevel, UNDO_ALL);
  i_update_menus(toplevel);

  /* now update the scrollbars */
  toplevel->DONT_REDRAW = 1;
  x_hscrollbar_update(toplevel);
  x_vscrollbar_update(toplevel);
  toplevel->DONT_REDRAW = 0;

  /* renable the events */
  toplevel->DONT_DRAW_CONN=0;
  toplevel->DONT_RESIZE=0;
  toplevel->DONT_EXPOSE=0;
  toplevel->DONT_RECALC=0;

}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void x_window_setup_colors(void)
{
  gdk_color_parse ("black", &black);
  if (!gdk_colormap_alloc_color (colormap,
                                 &black,
                                 FALSE,
                                 TRUE)) {
    fprintf (stderr, _("Could not allocate the color %s!\n"), _("black"));
    exit (-1);
  }

  gdk_color_parse ("white", &white);
  if (!gdk_colormap_alloc_color (colormap,
                                 &white,
                                 FALSE,
                                 TRUE)) {
    fprintf (stderr, _("Could not allocate the color %s!\n"), _("white"));
    exit (-1);
  }

  x_color_allocate_all ();

}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void x_window_free_colors(TOPLEVEL *w_current)
{
  GdkColor *colors[] = { &black, &white };
  
  gdk_colormap_free_colors (colormap, *colors, 2);
  
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void x_window_setup_gc(TOPLEVEL *w_current)
{
  GdkGCValues     values;
  GdkGCValuesMask  values_mask;

  w_current->gc = gdk_gc_new(w_current->window);

  if (w_current->gc == NULL) {
    fprintf(stderr, _("Couldn't allocate gc\n"));
    exit(-1);
  }

  values.foreground = white;
  values.background = black;

  values.function = GDK_XOR;
  values_mask = GDK_GC_FOREGROUND | GDK_GC_BACKGROUND | GDK_GC_FUNCTION;
  w_current->xor_gc = gdk_gc_new_with_values(w_current->window,
                                             &values, values_mask);

  if (w_current->xor_gc == NULL) {
    fprintf(stderr, _("Couldn't allocate xor_gc\n"));
    exit(-1);
  }

  values.foreground = white;
  values.background = black;

  values.function = GDK_XOR;
  values_mask = GDK_GC_FOREGROUND | GDK_GC_BACKGROUND | GDK_GC_FUNCTION;
  w_current->outline_xor_gc = gdk_gc_new_with_values(w_current->window,
                                                     &values, values_mask);

  if (w_current->outline_xor_gc == NULL) {
    fprintf(stderr, _("Couldn't allocate outline_xor_gc\n"));
    exit(-1);
  }

  values.foreground = white;
  values.background = black;

  values.function = GDK_XOR;
  values.line_style = GDK_LINE_ON_OFF_DASH;
  values_mask = GDK_GC_FOREGROUND | GDK_GC_BACKGROUND |
  GDK_GC_LINE_STYLE | GDK_GC_FUNCTION;

  w_current->bounding_xor_gc = gdk_gc_new_with_values(w_current->window,
                                                      &values, values_mask);

  if (w_current->bounding_xor_gc == NULL) {
    fprintf(stderr, _("Couldn't allocate bounding_xor_gc\n"));
    exit(-1);
  }

  w_current->bus_gc = gdk_gc_new(w_current->window);

  if (w_current->bus_gc == NULL) {
    fprintf(stderr, _("Couldn't allocate bus_gc\n"));
    exit(-1);
  }
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void x_window_free_gc(TOPLEVEL *w_current)
{
  gdk_gc_unref(w_current->gc);
  gdk_gc_unref(w_current->xor_gc);
  gdk_gc_unref(w_current->bounding_xor_gc);
  gdk_gc_unref(w_current->outline_xor_gc);
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void x_window_create_drawing(GtkWidget *drawbox, TOPLEVEL *w_current)
{
  /* drawing next */
  w_current->drawing_area = gtk_drawing_area_new ();
  /* Set the size here.  Be sure that it has an aspect ratio of 1.333
   * We could calculate this based on root window size, but for now
   * lets just set it to:
   * Width = root_width*3/4   Height = Width/1.3333333333
   * 1.3333333 is the desired aspect ratio!
   */

  gtk_drawing_area_size (GTK_DRAWING_AREA (w_current->drawing_area),
                         w_current->win_width,
                         w_current->win_height);

  gtk_box_pack_start (GTK_BOX (drawbox), w_current->drawing_area,
                      TRUE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS (w_current->drawing_area, GTK_CAN_FOCUS );
  gtk_widget_grab_focus (w_current->drawing_area);
  gtk_widget_show (w_current->drawing_area);

}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void x_window_setup_draw_events(TOPLEVEL *w_current)
{
  struct event_reg_t {
    gchar *detailed_signal;
    void (*c_handler)(void);
  };

  struct event_reg_t drawing_area_events[] = {
    { "expose_event",         G_CALLBACK(x_event_expose)          },
    { "button_press_event",   G_CALLBACK(x_event_button_pressed)  },
    { "button_release_event", G_CALLBACK(x_event_button_released) },
    { "motion_notify_event",  G_CALLBACK(x_event_motion)          },
    { "configure_event",      G_CALLBACK(x_event_configure)       },
    { "key_press_event",      G_CALLBACK(x_event_key_press)       },
    { NULL,                   NULL                                } };
  struct event_reg_t main_window_events[] = {
    { "enter_notify_event",   G_CALLBACK(x_event_enter)           },
    { "scroll_event",         G_CALLBACK(x_event_scroll)          },
    { NULL,                   NULL                                } };
  struct event_reg_t *tmp;

  /* is the configure event type missing here? hack */
  gtk_widget_set_events (w_current->drawing_area,
                         GDK_EXPOSURE_MASK |
                         GDK_POINTER_MOTION_MASK |
                         GDK_BUTTON_PRESS_MASK   |
                         GDK_ENTER_NOTIFY_MASK |
                         GDK_KEY_PRESS_MASK |
                         GDK_BUTTON_RELEASE_MASK);

  for (tmp = drawing_area_events; tmp->detailed_signal != NULL; tmp++) {
    g_signal_connect (w_current->drawing_area,
                      tmp->detailed_signal,
                      tmp->c_handler,
                      w_current);
  }

  for (tmp = main_window_events; tmp->detailed_signal != NULL; tmp++) {
    g_signal_connect (w_current->main_window,
                      tmp->detailed_signal,
                      tmp->c_handler,
                      w_current);
  }			  
}


/*! \brief Creates a new GtkImage displaying a GTK stock icon if available.
 *
 * If a stock GTK icon with the requested name was not found, this function
 * falls back to the bitmap icons provided in the distribution.
 *
 * \param stock Name of the stock icon ("new", "open", etc.)
 * \return Pointer to the new GtkImage object.
 */
static GtkWidget *x_window_stock_pixmap(const char *stock, TOPLEVEL *w_current)
{
  GtkWidget *wpixmap;
  GdkPixmap *pixmap;
  GdkBitmap *mask;
  GtkStockItem item;

  GdkWindow *window=w_current->main_window->window;
  GdkColor *background=&w_current->main_window->style->bg[GTK_STATE_NORMAL];

  gchar *filename=g_strconcat(w_current->bitmap_directory, 
                              G_DIR_SEPARATOR_S, 
                              "gschem-", stock, ".xpm", NULL);

  gchar *stockid=g_strconcat("gtk-", stock, NULL);

  /* First check if GTK knows this stock icon */
  if(gtk_stock_lookup(stockid, &item)) {
    wpixmap = gtk_image_new_from_stock(stockid, 
                                       GTK_ICON_SIZE_SMALL_TOOLBAR);
  } else {
    /* Fallback to the original custom icon */
    pixmap = gdk_pixmap_create_from_xpm (window, &mask, 
                                         background, filename);
    wpixmap = gtk_image_new_from_pixmap (pixmap, mask);
  }

  g_free(filename);
  g_free(stockid);

  return wpixmap;
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void x_window_create_main(TOPLEVEL *w_current)
{
  GtkWidget *label=NULL;
  GtkWidget *main_box=NULL;
  GtkWidget *menubar=NULL;
  GtkWidget *drawbox=NULL;
  GtkWidget *bottom_box=NULL;
  GtkWidget *toolbar=NULL;
  GtkWidget *handlebox=NULL;

  /* used to signify that the window isn't mapped yet */
  w_current->window = NULL; 

  w_current->main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_widget_set_name (w_current->main_window, "gschem");
  gtk_window_set_policy (GTK_WINDOW (w_current->main_window), TRUE, TRUE, TRUE);

  /* We want the widgets to flow around the drawing area, so we don't
   * set a size of the main window.  The drawing area's size is fixed,
   * see below
   */

   /* 
    * normally we let the window manager handle locating and sizing
    * the window.  However, for some batch processing of schematics
    * (generating a pdf of all schematics for example) we want to
    * override this.  Hence "auto_place_mode".
    */
   if( auto_place_mode )
   	gtk_widget_set_uposition (w_current->main_window, 10, 10);

  /* I could not get the destroy signal to work. always got a: */
  /* Gdk-ERROR **: an x io error occurred */
  /* aborting... */
  /* message */
#if 0
  gtk_signal_connect (GTK_OBJECT (w_current->main_window), "destroy",
                      GTK_SIGNAL_FUNC(i_callback_destroy_wm),
                      w_current);
#endif

  /* this should work fine */
  gtk_signal_connect (GTK_OBJECT (w_current->main_window), "delete_event",
                      GTK_SIGNAL_FUNC (i_callback_close_wm),
                      w_current);

  /* Containers first */
  main_box = gtk_vbox_new(FALSE, 1);
  gtk_container_border_width(GTK_CONTAINER(main_box), 0);
  gtk_container_add(GTK_CONTAINER(w_current->main_window), main_box);

  get_main_menu(w_current, &menubar);
  if (w_current->handleboxes) {
  	handlebox = gtk_handle_box_new ();
  	gtk_box_pack_start(GTK_BOX(main_box), handlebox, FALSE, FALSE, 0);
  	gtk_container_add (GTK_CONTAINER (handlebox), menubar);
  } else {
  	gtk_box_pack_start(GTK_BOX(main_box), menubar, FALSE, FALSE, 0);
  }

  w_current->menubar = menubar;
  gtk_widget_realize (w_current->main_window);

  if (w_current->handleboxes && w_current->toolbars) {
  	handlebox = gtk_handle_box_new ();
  	gtk_box_pack_start (GTK_BOX (main_box), handlebox, FALSE, FALSE, 0);
  }
 
  if (w_current->toolbars) {
    toolbar = gtk_toolbar_new();
    gtk_toolbar_set_orientation (GTK_TOOLBAR(toolbar), 
                                 GTK_ORIENTATION_HORIZONTAL);
    gtk_toolbar_set_style (GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);

    if (w_current->handleboxes) {
      gtk_container_add (GTK_CONTAINER (handlebox), toolbar);
    } else {
      gtk_box_pack_start(GTK_BOX(main_box), toolbar, FALSE, FALSE, 0);
    }

    gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), 
                             _("New"), 
                             _("New file"), 
                             "toolbar/new", 
                             x_window_stock_pixmap("new", w_current),
                             (GtkSignalFunc) i_callback_toolbar_file_new, 
                             w_current);
    gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), 
                             _("Open"), 
                             _("Open file..."), 
                             "toolbar/open",
                             x_window_stock_pixmap("open", w_current),
                             (GtkSignalFunc) i_callback_toolbar_file_open, 
                             w_current);
    gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), 
                             _("Save"), 
                             _("Save file"), 
                             "toolbar/save", 
                             x_window_stock_pixmap("save", w_current),
                             (GtkSignalFunc) i_callback_toolbar_file_save, 
                             w_current);
    gtk_toolbar_append_space (GTK_TOOLBAR(toolbar)); 
    gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), 
                             _("Undo"), 
                             _("Undo last operation"), 
                             "toolbar/undo", 
                             x_window_stock_pixmap("undo", w_current),
                             (GtkSignalFunc) i_callback_toolbar_edit_undo, 
                             w_current);
    gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), 
                             _("Redo"), 
                             _("Redo last undo"), 
                             "toolbar/redo", 
                             x_window_stock_pixmap("redo", w_current),
                             (GtkSignalFunc) i_callback_toolbar_edit_redo, 
                             w_current);
    gtk_toolbar_append_space (GTK_TOOLBAR(toolbar)); 
    /* not part of any radio button group */
    gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), 
                             _("Component"), 
                             _("Add component...\nSelect library and component from list, move the mouse into main window, click to place\nRight mouse button to cancel"), 
                             "toolbar/component", 
                             x_window_stock_pixmap("comp", w_current),
                             (GtkSignalFunc) i_callback_toolbar_add_component, 
                             w_current);
    w_current->toolbar_net = 
      gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
                                 GTK_TOOLBAR_CHILD_RADIOBUTTON,
                                 NULL,
                                 _("Nets"),
                                 _("Add nets mode\nRight mouse button to cancel"),
                                 "toolbar/nets",
                                 x_window_stock_pixmap("net", w_current),
                                 (GtkSignalFunc) i_callback_toolbar_add_net,
                                 w_current);
    w_current->toolbar_bus = 
      gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
                                 GTK_TOOLBAR_CHILD_RADIOBUTTON,
                                 w_current->toolbar_net,
                                 _("Bus"),
                                 _("Add buses mode\nRight mouse button to cancel"),
                                 "toolbar/bus",
                                 x_window_stock_pixmap("bus", w_current),
                                 (GtkSignalFunc) i_callback_toolbar_add_bus,
                                 w_current);
    /* not part of any radio button group */
    gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), 
                             _("Text"), 
                             _("Add Text..."), 
                             "toolbar/text", 
                             x_window_stock_pixmap("text", w_current),
                             (GtkSignalFunc) i_callback_toolbar_add_text, 
                             w_current);
    gtk_toolbar_append_space (GTK_TOOLBAR(toolbar)); 
    w_current->toolbar_select = 
      gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
                                 GTK_TOOLBAR_CHILD_RADIOBUTTON,
                                 w_current->toolbar_bus,
                                 _("Select"),
                                 _("Select mode"),
                                 "toolbar/select",
                                 x_window_stock_pixmap("select", w_current),
                                 (GtkSignalFunc) i_callback_toolbar_edit_select, 
                                 w_current);


    gtk_toolbar_append_space (GTK_TOOLBAR(toolbar)); 
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_current->toolbar_select),
                                 TRUE);
  } 


  /*  Try to create popup menu (appears in right mouse button  */
  w_current->popup_menu = (GtkWidget *) get_main_popup(w_current);

  drawbox = gtk_hbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(drawbox), 0);
  gtk_container_add(GTK_CONTAINER(main_box), drawbox);

  x_window_create_drawing(drawbox, w_current);
  x_window_setup_draw_events(w_current);

  if (w_current->scrollbars_flag == TRUE) {
    /* setup scroll bars */
    w_current->v_adjustment =
      gtk_adjustment_new (w_current->init_bottom,
                          0.0, w_current->init_bottom,
                          100.0, 100.0, 10.0);

    w_current->v_scrollbar = gtk_vscrollbar_new (GTK_ADJUSTMENT (
                                                                 w_current->v_adjustment));

    gtk_range_set_update_policy (GTK_RANGE (w_current->v_scrollbar),
                                 GTK_UPDATE_CONTINUOUS);

    gtk_box_pack_start (GTK_BOX (drawbox), w_current->v_scrollbar,
                        FALSE, FALSE, 0);

    gtk_signal_connect (GTK_OBJECT (w_current->v_adjustment),
                        "value_changed",
                        GTK_SIGNAL_FUNC (x_event_vschanged),
                        w_current);

    w_current->h_adjustment = gtk_adjustment_new (0.0, 0.0,
                                                  w_current->init_right,
                                                  100.0, 100.0, 10.0);

    w_current->h_scrollbar = gtk_hscrollbar_new (GTK_ADJUSTMENT (
                                                                 w_current->h_adjustment));

    gtk_range_set_update_policy (GTK_RANGE (w_current->h_scrollbar),
                                 GTK_UPDATE_CONTINUOUS);

    gtk_box_pack_start (GTK_BOX (main_box), w_current->h_scrollbar,
                        FALSE, FALSE, 0);

    gtk_signal_connect (GTK_OBJECT (w_current->h_adjustment),
                        "value_changed",
                        GTK_SIGNAL_FUNC (x_event_hschanged),
                        w_current);
  }

  /* bottom box */
  bottom_box = gtk_hbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(bottom_box), 1);
  gtk_box_pack_start (GTK_BOX (main_box), bottom_box, FALSE, FALSE, 0);

  /*	label = gtk_label_new ("Mouse buttons:");
        gtk_box_pack_start (GTK_BOX (bottom_box), label, FALSE, FALSE, 10);
  */

  label = gtk_label_new (" ");
  gtk_box_pack_start (GTK_BOX (bottom_box), label, FALSE, FALSE, 2);

  w_current->left_label = gtk_label_new (_("Pick"));
  gtk_box_pack_start (GTK_BOX (bottom_box), w_current->left_label,
                      FALSE, FALSE, 0);

  label = gtk_label_new ("|");
  gtk_box_pack_start (GTK_BOX (bottom_box), label, FALSE, FALSE, 5);

  if (w_current->middle_button == STROKE) {
#ifdef HAS_LIBSTROKE
    w_current->middle_label = gtk_label_new (_("Stroke"));
#else
    w_current->middle_label = gtk_label_new (_("none"));
#endif
  } else if (w_current->middle_button == ACTION) {
    w_current->middle_label = gtk_label_new (_("Action"));
  } else {
    w_current->middle_label = gtk_label_new (_("Repeat/none"));
  }

  gtk_box_pack_start (GTK_BOX (bottom_box), w_current->middle_label,
                      FALSE, FALSE, 0);

  label = gtk_label_new ("|");
  gtk_box_pack_start (GTK_BOX (bottom_box), label, FALSE, FALSE, 5);

  if (default_third_button == POPUP_ENABLED) {
    w_current->right_label = gtk_label_new (_("Menu/Cancel"));
  } else {
    w_current->right_label = gtk_label_new (_("Pan/Cancel"));
  }
  gtk_box_pack_start (GTK_BOX (bottom_box), w_current->right_label,
                      FALSE, FALSE, 0);

  label = gtk_label_new (" ");
  gtk_box_pack_start (GTK_BOX (bottom_box), label, FALSE, FALSE, 5);

  w_current->grid_label = gtk_label_new (" ");
  gtk_box_pack_start (GTK_BOX (bottom_box), w_current->grid_label,
                      FALSE, FALSE, 10);

  w_current->status_label = gtk_label_new (_("Select Mode"));
  gtk_box_pack_end (GTK_BOX (bottom_box), w_current->status_label, FALSE,
                    FALSE, 10);

  gtk_widget_show_all (w_current->main_window);

  w_current->window = w_current->drawing_area->window;

  /* draw a black rectangle in drawing area just to make it look nice */
  /* don't do this now */
  /* gdk_draw_rectangle(window, main_window->style->black_gc, TRUE, 0, 0,
   *				win_width, win_height);
   *
   */

  w_current->backingstore = gdk_pixmap_new(w_current->window,
                                           w_current->drawing_area->allocation.width,
                                           w_current->drawing_area->allocation.height,
                                           -1);
  x_window_setup_gc(w_current);
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void x_window_close(TOPLEVEL *w_current)
{
  gboolean last_window = FALSE;

  /* last chance to save possible unsaved pages */
  if (!x_dialog_close_window (w_current)) {
    /* user somehow cancelled the close */
    return;
  }

#if DEBUG
  o_conn_print_hash(w_current->page_current->conn_table);
#endif

  /* close all the dialog boxes */
  if (w_current->fowindow)
  gtk_widget_destroy(w_current->fowindow);

  if (w_current->sowindow)
  gtk_widget_destroy(w_current->sowindow);

  if (w_current->fswindow)
  gtk_widget_destroy(w_current->fswindow);

  if (w_current->aswindow)
  gtk_widget_destroy(w_current->aswindow);

  if (w_current->cswindow)
  gtk_widget_destroy(w_current->cswindow);

  if (w_current->tiwindow)
  gtk_widget_destroy(w_current->tiwindow);

  if (w_current->tewindow)
  gtk_widget_destroy(w_current->tewindow);

  if (w_current->aawindow)
  gtk_widget_destroy(w_current->aawindow);

  if (w_current->mawindow)
  gtk_widget_destroy(w_current->mawindow);

  if (w_current->aewindow)
  gtk_widget_destroy(w_current->aewindow);

  if (w_current->trwindow)
  gtk_widget_destroy(w_current->trwindow);

  x_pagesel_close (w_current);

  if (w_current->exwindow)
  gtk_widget_destroy(w_current->exwindow);

  if (w_current->tswindow)
  gtk_widget_destroy(w_current->tswindow);

  if (w_current->abwindow)
  gtk_widget_destroy(w_current->abwindow);

  if (w_current->iwindow)
  gtk_widget_destroy(w_current->iwindow);

  if (w_current->hkwindow)
  gtk_widget_destroy(w_current->hkwindow);

  if (w_current->cowindow)
  gtk_widget_destroy(w_current->cowindow);

  if (w_current->clwindow)
  gtk_widget_destroy(w_current->clwindow);

  if (w_current->ltwindow)
  gtk_widget_destroy(w_current->ltwindow);

  if (w_current->sewindow)
  gtk_widget_destroy(w_current->sewindow);

  g_assert(w_current->prev != NULL);
  if (w_current->next == NULL && w_current->prev->prev == NULL) {
    /* no more window after this one, remember to quit */
    last_window = TRUE;
  }
  
  o_attrib_free_current(w_current);
  o_complex_free_filename(w_current);

  if (w_current->major_changed_refdes) {
    GList* current = w_current->major_changed_refdes;
    while (current)
    {
      /* printf("yeah freeing: %s\n", (char*) current->data); */
      g_free(current->data);
      current = current->next;
    }
    g_list_free(w_current->major_changed_refdes);
  }

  /* stuff that has to be done before we free w_current */
  if (last_window) {
    /* free all fonts */
    o_text_freeallfonts (w_current);
    /* close the log file */
    s_log_close ();
    /* free the buffers */
    o_buffer_free (w_current);
  }

  if (w_current->backingstore) {
    gdk_pixmap_unref(w_current->backingstore);
  }

  x_window_free_gc(w_current);

  /* finally close the main window */
  gtk_widget_destroy(w_current->main_window);
  
  s_toplevel_delete (w_current);

  /* just closed last window, so quit */
  if (last_window) {
    gschem_quit();
  }
  
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void x_window_close_all(TOPLEVEL *toplevel)
{
  TOPLEVEL *last, *current;

  /* find last toplevel in toplevel list */
  for (last = toplevel; last->next != NULL; last = last->next);

  /* now close each toplevel */
  for (current = last; current->prev != NULL; current = last) {
    /* save a ref to the previous toplevel in the list */
    last = current->prev;
    /* close current if it is not a preview toplevel */
    if (current->main_window != NULL) {
      x_window_close (current);
    }
  }

}

/*! \brief Opens a new untitled page.
 *  \par Function Description
 *  This function creates an empty, untitled page in <B>toplevel</B>.
 *
 *  It returns a pointer on the new page.
 *
 *  The new page becomes the current page of <B>toplevel</B>.
 *
 *  The name of the untitled page is build from configuration data
 *  ('untitled-name') and a counter for uniqueness.
 *
 *  \param [in] toplevel The toplevel environment.
 *  \returns A pointer on the new page.
 */
PAGE*
x_window_open_untitled_page (TOPLEVEL *toplevel)
{
  PAGE *page;
  gchar *cwd, *tmp, *filename;
 
  cwd = g_get_current_dir ();
  tmp = g_strdup_printf ("%s_%d.sch",
                         toplevel->untitled_name,
                         toplevel->num_untitled++);
  filename = g_build_filename (cwd, tmp, NULL);
  g_free (cwd);
  g_free (tmp);
   
  page = x_window_open_page (toplevel, filename);
  
  g_free (filename);

  if (scm_hook_empty_p (new_page_hook) == SCM_BOOL_F) {
    scm_run_hook (new_page_hook,
                  scm_cons (g_make_page_smob (toplevel, page), SCM_EOL));
  }
  
  a_zoom_extents(toplevel, page->object_head, 0);
  
  return page;
}

/*! \brief Opens a new page from a file.
 *  \par Function Description
 *  This function opens the file whose name is <B>filename</B> in a
 *  new PAGE of <B>toplevel</B>.
 *
 *  If there is no page for <B>filename</B> in <B>toplevel</B>'s list
 *  of pages, it creates a new PAGE, loads the file in it and returns
 *  a pointer on the new page. Otherwise it returns a pointer on the
 *  existing page.
 *
 *  The opened page becomes the current page of <B>toplevel</B>.
 *
 *  \param [in] toplevel The toplevel environment.
 *  \param [in] filename The name of the file to open.
 *  \returns A pointer on the new page.
 */
PAGE*
x_window_open_page (TOPLEVEL *toplevel, const gchar *filename)
{
  PAGE *old_current, *page;

  g_return_val_if_fail (toplevel != NULL, NULL);
  g_return_val_if_fail (filename != NULL, NULL);
  
  /* save current page for restore after opening */
  old_current = toplevel->page_current;

  /* is file already loaded? */
  page = s_page_search (toplevel, filename);
  if (page == NULL) {
    /* no, create a page and load file in it */
    page = s_page_new (toplevel, filename);
    s_page_goto (toplevel, page);
    
    if (!quiet_mode) {
      printf(_("Loading schematic [%s]\n"), filename);
    }

    if (g_file_test (filename, G_FILE_TEST_EXISTS)) {
      f_open (toplevel, (gchar *) filename);
    }
    
    i_set_filename (toplevel, toplevel->page_current->page_filename);
    a_zoom_extents (toplevel,
                    toplevel->page_current->object_head,
                    A_PAN_DONT_REDRAW);
    o_undo_savestate (toplevel, UNDO_ALL);

    x_pagesel_update (toplevel);
    
  }

  /* display the page in window */
  x_window_set_current_page (toplevel, page);
  
  return page;
}

/*! \brief Changes the current page.
 *  \par Function Description
 *  This function displays the specified page <B>page</B> in the
 *  window attached to <B>toplevel</B>.
 *
 *  It changes the <B>toplevel</B>'s current page to <B>page</B>,
 *  draws it and updates the user interface.
 *
 *  <B>page</B> has to be in the list of PAGEs attached to <B>toplevel</B>.
 *
 *  \param [in] toplevel The toplevel environment.
 *  \param [in] page     The page to become current page.
 */
void
x_window_set_current_page (TOPLEVEL *toplevel, PAGE *page)
{
  g_return_if_fail (toplevel != NULL);
  g_return_if_fail (page != NULL);

  s_page_goto (toplevel, page);

  i_update_menus (toplevel);
  i_set_filename (toplevel, page->page_filename);

  x_pagesel_update (toplevel);
  
  toplevel->DONT_REDRAW = 1;
  x_repaint_background (toplevel);
  x_manual_resize (toplevel);
  x_hscrollbar_update (toplevel);
  x_vscrollbar_update (toplevel);
  toplevel->DONT_REDRAW = 0;
  
  o_redraw_all (toplevel);
  
}

/*! \brief Saves a page to a file.
 *  \par Function Description
 *  This function saves the page <B>page</B> to a file named
 *  <B>filename</B>.
 *
 *  It returns the value returned by function <B>f_save()</B> trying
 *  to save page <B>page</B> to file <B>filename</B> (1 on success, 0
 *  on failure).
 *
 *  <B>page</B> may not be the current page of <B>toplevel</B>. The
 *  current page of <B>toplevel</B> is not affected by this function.
 *
 *  \param [in] toplevel The toplevel environment.
 *  \param [in] page     The page to save.
 *  \param [in] filename The name of the file in which to save page.
 *  \returns 1 on success, 0 otherwise.
 */
gint
x_window_save_page (TOPLEVEL *toplevel, PAGE *page, const gchar *filename)
{
  PAGE *old_current;
  const gchar *log_msg, *state_msg;
  gint ret;

  g_return_val_if_fail (toplevel != NULL, 0);
  g_return_val_if_fail (page     != NULL, 0);
  g_return_val_if_fail (filename != NULL, 0);
  
  /* save current page for restore after opening */
  old_current = toplevel->page_current;

  /* change to page */
  s_page_goto (toplevel, page);
  /* and try saving current page to filename */
  ret = (gint)f_save (toplevel, filename);
  if (ret != 1) {
    /* an error occured when saving page to file */
    log_msg   = _("Could NOT save page [%s]\n");
    state_msg = _("Error while trying to save");
    
  } else {
    /* successful save of page to file, update page... */
    /* change page name if necessary and prepare log message */
    if (g_ascii_strcasecmp (page->page_filename, filename) != 0) {
      g_free (page->page_filename);
      page->page_filename = g_strdup (filename);

      log_msg = _("Saved as [%s]\n");
    } else {
      log_msg = _("Saved [%s]\n");
    }
    state_msg = _("Saved");
      
    /* reset page CHANGED flag */
    page->CHANGED = 0;
    
  }

  /* log status of operation */
  s_log_message (log_msg, filename);
  
  /* update display and page manager */
  i_set_state_msg  (toplevel, SELECT, state_msg);
  i_set_filename   (toplevel, page->page_filename);
  i_update_toolbar (toplevel);
  i_update_menus   (toplevel);
  x_pagesel_update (toplevel);

  /* restore current page in toplevel */
  s_page_goto (toplevel, old_current);

  return ret;
}

/*! \brief Closes a page.
 *  \par Function Description
 *  This function closes the page <B>page</B> of toplevel
 *  <B>toplevel</B>.
 *
 *  If necessary, the current page of <B>toplevel</B> is changed to
 *  the next valid page or to a new untitled page.
 *
 *  \param [in] toplevel The toplevel environment.
 *  \param [in] page     The page to close.
 */
void
x_window_close_page (TOPLEVEL *toplevel, PAGE *page)
{
  PAGE *new_current = NULL;

  g_return_if_fail (toplevel != NULL);
  g_return_if_fail (page     != NULL);

  g_assert (page->pid != -1);

  if (page == toplevel->page_current) {
    /* as it will delete current page, select new current page */
    /* first look up in page hierarchy */
    new_current = s_hierarchy_find_page (toplevel->page_head, page->up);
    if (new_current == NULL) {
      /* no up in hierarchy, choice is next, prev, new page */
      if (page->prev && page->prev->pid != -1) {
        new_current = page->prev;
      } else if (page->next != NULL) {
        new_current = page->next;
      } else {
        /* need to add a new untitled page */
        new_current = NULL;
      }
    }
    /* new_current will be the new current page at the end of the function */
  }

  s_log_message (page->CHANGED ?
                 _("Discarding page [%s]\n") : _("Closing [%s]\n"),
                 page->page_filename);
  /* remove page from toplevel list of page and free */
  s_page_delete (toplevel, page);

  if (toplevel->page_current == NULL) {
    /* page was the current page of toplevel, set new current */
    if (new_current == NULL) {
      /* empty page list in toplevel, create new page */
      new_current = x_window_open_untitled_page (toplevel);
    } else {
      /* change to new_current and update display */
      x_window_set_current_page (toplevel, new_current);
    }    
    g_assert (new_current != NULL);
    
  }

}
