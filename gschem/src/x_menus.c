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

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <libgeda/libgeda.h>

#include "../include/globals.h"
#include "../include/prototype.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

static GtkItemFactoryEntry popup_items[] = {
  { N_("/Add Net"), 	      NULL, i_callback_add_net,           0, NULL},
  { N_("/Add Attribute..."),  NULL, i_callback_add_attribute,     0, NULL},
  { N_("/Add Component..."),  NULL, i_callback_add_component,     0, NULL},
  { N_("/Add Bus"), 	      NULL, i_callback_add_bus,           0, NULL},
  { N_("/Add Text"), 	      NULL, i_callback_add_text,          0, NULL},
  { "/sep1", NULL, NULL, 0, "<Separator>"},
  { N_("/Zoom In"),       NULL, i_callback_view_zoom_in,      0, NULL},
  { N_("/Zoom Out"),      NULL, i_callback_view_zoom_out,     0, NULL},
  { N_("/Zoom Box"),      NULL, i_callback_view_zoom_box,     0, NULL},
  { N_("/Zoom Extents"),  NULL, i_callback_view_zoom_extents, 0, NULL},
  { "/sep1", NULL, NULL, 0, "<Separator>"},
  { N_("/Select"), 	  NULL, i_callback_edit_select,       0, NULL},
  { N_("/Edit..."), 	  NULL, i_callback_edit_edit,         0, NULL},
  { N_("/Copy"),          NULL, i_callback_edit_copy,         0, NULL},
  { N_("/Move"),          NULL, i_callback_edit_move,         0, NULL},
  { N_("/Delete"),        NULL, i_callback_edit_delete,       0, NULL},
  /* Menu items for hierarchy added by SDB 1.9.2005.  */
  {"/sep1", NULL, NULL, 0, "<Separator>"},
  {N_("/Down Schematic"), NULL, i_callback_hierarchy_down_schematic, 0, NULL},
  {N_("/Down Symbol"),    NULL, i_callback_hierarchy_down_symbol,    0, NULL},
  {N_("/Up"),             NULL, i_callback_hierarchy_up,             0, NULL},
};  

int npopup_items = sizeof(popup_items) / sizeof(popup_items[0]);

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
static void g_menu_execute(char *func)
{
  gchar *guile_string;
  
  guile_string = g_strdup_printf("(%s)", func);
#if DEBUG
  printf("%s\n", guile_string);
#endif
  scm_c_eval_string (guile_string);
  g_free(guile_string);
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void get_main_menu(TOPLEVEL * w_current, GtkWidget ** menubar)
{
  char *buf;
  GtkWidget *menu_item;
  GtkWidget *root_menu;
  GtkWidget *menu_bar;
  GtkWidget *menu;
  int scm_items_len;
  SCM scm_items;
  SCM scm_item;
  SCM scm_item_name;
  SCM scm_item_func;
  SCM scm_item_hotkey_func;
  SCM scm_index;
  SCM scm_keys;
  char *menu_name;
  char **raw_menu_name = g_malloc (sizeof(char *));
  char *menu_item_name;
  char *raw_menu_item_name;
  char *menu_item_func;
  char *menu_item_hotkey_func;
  char *menu_item_keys;
  char *spaces;
  const char *help_string;
  int i, j;
  int name_len, key_len, pad;
  int sum, diff, max_size, space_size;
  PangoLayout *layout;
  int name_width, keys_width;

  menu_bar = gtk_menu_bar_new ();
  for (i = 0 ; i < s_menu_return_num(); i++) {
    
    scm_items = s_menu_return_entry(i, raw_menu_name);   
    if (*raw_menu_name == NULL) {
      fprintf(stderr, "Oops.. got a NULL menu name in get_main_menu()\n");
      exit(-1);
    }

    menu = gtk_menu_new();

    menu_item = gtk_tearoff_menu_item_new ();
    gtk_menu_append(GTK_MENU(menu), menu_item);
    gtk_widget_show(menu_item);

    layout = gtk_widget_create_pango_layout(menu, " ");
    pango_layout_get_pixel_size(layout, &space_size, NULL);
    g_object_unref(layout);

    layout = gtk_widget_create_pango_layout(menu, 
	                                    "123456789012345678901234567890");
    pango_layout_get_pixel_size(layout, &max_size, NULL);
    g_object_unref(layout);

    scm_items_len = (int) scm_ilength (scm_items);
    for (j = 0 ; j < scm_items_len; j++) {

      scm_index = SCM_MAKINUM (j);
      scm_item = scm_list_ref (scm_items, scm_index);
      scm_item_name = SCM_CAR (scm_item);
      scm_item_func = SCM_CADR (scm_item);
      scm_item_hotkey_func = SCM_CADDR (scm_item);
      SCM_ASSERT(SCM_STRINGP(scm_item_name), scm_item_name, SCM_ARGn, "get_main_menu item_name");
      SCM_ASSERT(SCM_SYMBOLP(scm_item_func), scm_item_func, SCM_ARGn, "get_main_menu item_func");
      SCM_ASSERT(SCM_SYMBOLP(scm_item_hotkey_func), scm_item_hotkey_func, SCM_ARGn, "get_main_menu hotkey_func");

      raw_menu_item_name = SCM_STRING_CHARS (scm_item_name);
      menu_item_func = SCM_SYMBOL_CHARS (scm_item_func);
      menu_item_hotkey_func = SCM_SYMBOL_CHARS (scm_item_hotkey_func);

      menu_item_name = (char *) gettext(raw_menu_item_name);

      if (strcmp(menu_item_name, "SEPARATOR") == 0) {
        menu_item = gtk_menu_item_new();
        gtk_menu_append(GTK_MENU(menu), menu_item);
      } else { 
        buf = g_strdup_printf("(find-key '%s)", menu_item_hotkey_func);
        scm_keys = scm_c_eval_string (buf);
	g_free(buf);
        if (scm_keys == SCM_BOOL_F) {
          menu_item_keys = g_malloc(sizeof(char)*2);
          menu_item_keys[0] = ' ';
          menu_item_keys[1] = '\0';
        } else {
          menu_item_keys = g_strdup (SCM_STRING_CHARS (scm_keys));
        }      

        name_len = strlen(menu_item_name);
        key_len = strlen(menu_item_keys);

        layout = gtk_widget_create_pango_layout(menu, menu_item_name);
        pango_layout_get_pixel_size(layout, &name_width, NULL);
        g_object_unref(layout);

        layout = gtk_widget_create_pango_layout(menu, menu_item_keys);
        pango_layout_get_pixel_size(layout, &keys_width, NULL);
        g_object_unref(layout);

        sum = name_width + keys_width;

	diff = max_size - sum;
	pad = diff/space_size;
	if (pad < 0) { 
           pad = 1;
	} 

        spaces = g_malloc(sizeof(char)*(pad+1));
        memset(spaces, ' ', pad);
        spaces[pad] = '\0';
        buf = g_strdup_printf("%s%s%s", menu_item_name, spaces, menu_item_keys);

#if DEBUG
        printf("%s :\n %d %d %d = %d\n", buf, name_len, pad, key_len,
               name_len + pad + key_len);
#endif
      
        menu_item = gtk_menu_item_new_with_label(buf);
	g_free(buf);
        gtk_menu_append(GTK_MENU(menu), menu_item);
        gtk_signal_connect_object(GTK_OBJECT(menu_item), "activate",
                                  GTK_SIGNAL_FUNC(g_menu_execute),
                                  (gpointer) g_strdup (menu_item_func));
        /* The g_strdup is a memory leak, but this is okay. I think. */
        g_free(spaces);
        g_free(menu_item_keys);
      }
      
      gtk_widget_show(menu_item);

      /* add a handle to the menu_bar object to get access to widget objects */
      /* This string should NOT be internationalized */
      buf = g_strdup_printf("%s/%s", *raw_menu_name, raw_menu_item_name);
      gtk_object_set_data(GTK_OBJECT(menu_bar), buf, menu_item);
      g_free(buf);
      
    }
    
    menu_name = (char *) gettext(*raw_menu_name);
    root_menu = gtk_menu_item_new_with_label (menu_name);
    /* do not free *raw_menu_name */

    /* no longer right justify the help menu since that has gone out of style */

    gtk_widget_show (root_menu);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (root_menu), menu);
    gtk_menu_bar_append (GTK_MENU_BAR (menu_bar), root_menu);
  }
  
  g_free(raw_menu_name);
  *menubar = menu_bar;
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
static gchar* gettext_fn(const gchar *path,
			 gpointer func_data ATTRIBUTE_UNUSED)
{
	return gettext(path);
}

GtkWidget *get_main_popup(TOPLEVEL *w_current)
{
  static GtkItemFactory *item_factory;
  GtkAccelGroup *accel_group;
  GtkWidget *menu;

  accel_group = gtk_accel_group_new();

  /* This function initializes the item factory.
     Param 1: The type of menu - can be GTK_TYPE_MENU_BAR, GTK_TYPE_MENU, or GTK_TYPE_OPTION_MENU.
     Param 2: The path of the menu.
     Param 3: A pointer to a gtk_accel_group.  The item factory sets up
     the accelerator table while generating menus.
  */
  item_factory = gtk_item_factory_new(GTK_TYPE_MENU, "<popup>",
                                      accel_group);
  gtk_item_factory_set_translate_func (item_factory,
                                       gettext_fn,
                                       NULL, NULL);
  /* This function creates the pop-up menu itself & attaches it to the
     GtkItemFactory. Pass the item factory,
     the number of items in the array, the array itself, and any
     callback data for the the menu items. Note that npopup_items is 
     a static var declared in this file above; popup_items is also a
     static var declared above.
  */
  gtk_item_factory_create_items(item_factory, npopup_items, popup_items, w_current);

  /* Finally, return the actual menu created by the item factory. */
  menu = (GtkWidget *) gtk_item_factory_get_widget(item_factory, "<popup>");
  return (menu);

}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 *  \note
 *  need to look at this... here and the setup
 */
gint do_popup (TOPLEVEL *w_current, GdkEventButton *event)
{
  GtkWidget *menu;   /* =NULL; */ /* was static */

  menu = NULL;  /* Why do I need to do this? */
  if (!menu)
    menu = (GtkWidget *) w_current->popup_menu;

  if (menu == NULL) {
    printf("null menu\n");
  }

  gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
                  event->button, event->time);

  return FALSE;
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void x_menus_sensitivity (TOPLEVEL* w_current, const char *buf, int flag)
{
  GtkWidget* item=NULL;
  
  if (!buf) {
    return;
  }

  if (!w_current->menubar) {
    return;
  }
  
  item = (GtkWidget *) gtk_object_get_data(GTK_OBJECT(w_current->menubar), buf);

  if (item) {
    gtk_widget_set_sensitive(GTK_WIDGET(item), flag);
    /* free(item); */ /* Why doesn't this need to be freed?  */
  } else {
    s_log_message(_("Tried to set the sensitivity on a non-existent menu item\n")); 
  }
 
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *  This function sets the sensitivity of the items in the right button
 *  popup.
 *
 *  \note
 *  1.9.2005 -- SDB.
 */
void x_menus_popup_sensitivity (TOPLEVEL* w_current, const char *buf, int flag)
{
  GtkWidget *menu_item;
  GtkItemFactory *menu_item_factory;
  
  if (!buf) {
    return;
  }

  if (!w_current->popup_menu) {
    s_log_message(_("Popup_menu_item_factory doesn't exist!\n")); 
    return;
  }

  /* 
   * first get entire item factory from popup, then get the individual 
   * menu item indexed by buf.
   */
  menu_item_factory = (GtkItemFactory *)gtk_item_factory_from_widget(w_current->popup_menu);  
  menu_item = (GtkWidget *) gtk_item_factory_get_widget(menu_item_factory, buf);
  if (menu_item) {
    gtk_widget_set_sensitive(GTK_WIDGET(menu_item), flag);
  } else {
    s_log_message(_("Tried to set the sensitivity on a non-existent popup menu_item\n")); 
  }
}
