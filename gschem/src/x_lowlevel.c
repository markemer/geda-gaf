/* gEDA - GPL Electronic Design Automation
 * gschem - gEDA Schematic Capture
 * Copyright (C) 1998-2010 Ales Hvezda
 * Copyright (C) 1998-2019 gEDA Contributors (see ChangeLog for details)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*! \file x_lowlevel.c
 * \brief Low-level page-related functions.
 *
 * These functions don't usually interact with the user.  In
 * particular, they don't switch pages and don't ask for confirmation
 * before carrying out potentially destructive actions.  However, they
 * *do* warn if an error occurred during an operation.
 */
#include <config.h>

#include "gschem.h"
#include "actions.decl.x"


/*! \brief Open a new page from a file, create a new untitled page
 *         with a titleblock, or find an existing page.
 *
 * Creates a new page and loads the file in it.  If there is already a
 * matching page in \a w_current, returns a pointer to the existing
 * page instead.
 *
 * If \c NULL is passed as a filename, creates an untitled page with a
 * titleblock.  The name of the untitled page is build from
 * configuration data ('untitled-name') and a counter for uniqueness.
 *
 * This function doesn't change the current page of \a w_current.
 *
 * \param [in] w_current  the toplevel environment
 * \param [in] filename   the name of the file to open, or \c NULL to
 *                        create an untitled page with a titleblock
 *
 * \returns a pointer to the page
 *
 * \bug This code should check to make sure any untitled filename does
 *      not conflict with a file on disk.
 */
PAGE*
x_lowlevel_open_page (GschemToplevel *w_current, const gchar *filename)
{
  PAGE *page;
  gchar *fn;

  TOPLEVEL *toplevel = gschem_toplevel_get_toplevel (w_current);
  g_return_val_if_fail (toplevel != NULL, NULL);

  /* Generate untitled filename if none was specified */
  if (filename == NULL) {
    gchar *cwd, *tmp, *untitled_name;
    EdaConfig *cfg;
    cwd = g_get_current_dir ();
    cfg = eda_config_get_context_for_path (cwd);
    untitled_name = eda_config_get_string (cfg, "gschem", "default-filename", NULL);
    tmp = g_strdup_printf ("%s_%d.sch",
                           untitled_name,
                           ++w_current->num_untitled);
    fn = g_build_filename (cwd, tmp, NULL);
    g_free (untitled_name);
    g_free(cwd);
    g_free(tmp);
  } else {
    fn = g_strdup (filename);
  }

  /* Return existing page if it is already loaded */
  page = s_page_search (toplevel, fn);
  if ( page != NULL ) {
    g_free(fn);
    return page;
  }

  PAGE *saved_page = toplevel->page_current;

  page = s_page_new (toplevel, fn);
  s_page_goto (toplevel, page);
  gschem_toplevel_page_changed (w_current);

  /* Load from file if necessary, otherwise just print a message */
  if (filename != NULL) {
    GError *err = NULL;
    if (!quiet_mode)
      s_log_message (_("Loading schematic [%s]\n"), fn);

    if (!f_open (toplevel, page, (gchar *) fn, &err)) {
      GtkWidget *dialog;

      g_warning ("%s\n", err->message);
      dialog = gtk_message_dialog_new_with_markup
        (GTK_WINDOW (w_current->main_window),
         GTK_DIALOG_DESTROY_WITH_PARENT,
         GTK_MESSAGE_ERROR,
         GTK_BUTTONS_CLOSE,
         _("<b>An error occurred while loading the requested file.</b>\n\nLoading from '%s' failed: %s. The gschem log may contain more information."),
         fn, err->message);
      gtk_window_set_title (GTK_WINDOW (dialog), _("Failed to load file"));
      gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
      g_error_free (err);
    } else {
      gtk_recent_manager_add_item (recent_manager, g_filename_to_uri(fn, NULL, NULL));
    }
  } else {
    if (!quiet_mode)
      s_log_message (_("New file [%s]\n"),
                     toplevel->page_current->page_filename);

    g_run_hook_page (w_current, "%new-page-hook", toplevel->page_current);
  }

  o_undo_savestate (w_current, toplevel->page_current, UNDO_ALL, NULL);

  g_free (fn);

  if (saved_page != NULL) {
    s_page_goto (toplevel, saved_page);
    gschem_toplevel_page_changed (w_current);
  }

  return page;
}


/*! \brief Save a page to a given filename.
 *
 * \a page doesn't have to be the current page of \a w_current.
 * This function doesn't change the current page of \a w_current.
 *
 * If \a filename is different from the current filename of \a page,
 * the page's filename is updated.
 *
 * \param [in] w_current  the toplevel environment
 * \param [in] page       the page to save
 * \param [in] filename   the name of the file to which to save the page
 *
 * \returns \c 1 if the page could be saved, \c 0 otherwise
 */
gint
x_lowlevel_save_page (GschemToplevel *w_current, PAGE *page, const gchar *filename)
{
  TOPLEVEL *toplevel = gschem_toplevel_get_toplevel (w_current);
  const gchar *log_msg, *state_msg;
  gint ret;
  GError *err = NULL;

  g_return_val_if_fail (toplevel != NULL, 0);
  g_return_val_if_fail (page     != NULL, 0);
  g_return_val_if_fail (filename != NULL, 0);

  /* try saving page to filename */
  ret = (gint)f_save (toplevel, page, filename, &err);

  if (ret != 1) {
    log_msg   = _("Could NOT save page [%s]\n");
    state_msg = _("Error while trying to save");

    GtkWidget *dialog;

    dialog = gtk_message_dialog_new (GTK_WINDOW (w_current->main_window),
                                     GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_MESSAGE_ERROR,
                                     GTK_BUTTONS_CLOSE,
                                     "%s",
                                     err->message);
    gtk_window_set_title (GTK_WINDOW (dialog), _("Failed to save file"));
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
    g_clear_error (&err);
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

    /* add to recent file list */
    gtk_recent_manager_add_item (recent_manager, g_filename_to_uri(filename, NULL, NULL));

    /* i_set_filename (w_current, page->page_filename); */
    x_pagesel_update (w_current);

    if (page == gschem_toplevel_get_toplevel (w_current)->page_current)
      gschem_action_set_sensitive (action_page_revert, TRUE, w_current);
  }

  /* log status of operation */
  s_log_message (log_msg, filename);

  i_set_state_msg  (w_current, SELECT, state_msg);

  return ret;
}


/*! \brief Close a page.
 *
 * Switches to the next valid page if necessary.  If this was the last
 * page of the toplevel, a new untitled page is created.
 *
 * \param [in] w_current  the toplevel environment
 * \param [in] page       the page to close
 */
void
x_lowlevel_close_page (GschemToplevel *w_current, PAGE *page)
{
  TOPLEVEL *toplevel = gschem_toplevel_get_toplevel (w_current);
  PAGE *new_current = NULL;
  GList *iter;

  g_return_if_fail (toplevel != NULL);
  g_return_if_fail (page     != NULL);

  g_assert (page->pid != -1);

  /* If we're closing whilst inside an action, re-wind the
   * page contents back to their state before we started */
  if (w_current->inside_action) {
    i_cancel (w_current);
  }

  if (page == toplevel->page_current) {
    /* as it will delete current page, select new current page */
    /* first look up in page hierarchy */
    new_current = s_page_search_by_page_id (toplevel->pages, page->up);

    if (new_current == NULL) {
      /* no up in hierarchy, choice is prev, next, new page */
      iter = g_list_find( geda_list_get_glist( toplevel->pages ), page );

      if ( g_list_previous( iter ) ) {
        new_current = (PAGE *)g_list_previous( iter )->data;
      } else if ( g_list_next( iter ) ) {
        new_current = (PAGE *)g_list_next( iter )->data;
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
  gschem_toplevel_page_changed (w_current);

  /* Switch to a different page if we just removed the current */
  if (toplevel->page_current == NULL) {

    /* Create a new page if there wasn't another to switch to */
    if (new_current == NULL) {
      new_current = x_lowlevel_open_page (w_current, NULL);
    }

    /* change to new_current and update display */
    x_window_set_current_page (w_current, new_current);
  }
}