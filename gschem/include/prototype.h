/* $Id$ */

/* gschem_toplevel.c */
GSCHEM_TOPLEVEL *gschem_toplevel_new();

/* a_pan.c */
void a_pan_general(GSCHEM_TOPLEVEL *w_current, double world_cx, double world_cy,
		   double relativ_zoom_factor, int flags);
void a_pan(GSCHEM_TOPLEVEL *w_current, int x, int y);
void a_pan_mouse(GSCHEM_TOPLEVEL *w_current, int diff_x, int diff_y);
/* a_zoom.c */
void a_zoom(GSCHEM_TOPLEVEL *w_current, int dir, int selected_from, int pan_flags);
void a_zoom_extents(GSCHEM_TOPLEVEL *w_current, const GList *list, int pan_flags);
void a_zoom_box(GSCHEM_TOPLEVEL *w_current, int pan_flags);
void a_zoom_box_start(GSCHEM_TOPLEVEL *w_current, int x, int y);
void a_zoom_box_end(GSCHEM_TOPLEVEL *w_current, int x, int y);
void a_zoom_box_motion(GSCHEM_TOPLEVEL *w_current, int x, int y);
void a_zoom_box_invalidate_rubber(GSCHEM_TOPLEVEL *w_current);
void a_zoom_box_draw_rubber(GSCHEM_TOPLEVEL *w_current, EdaRenderer *renderer);
void correct_aspect(GSCHEM_TOPLEVEL *w_current);
/* g_attrib.c */
void g_init_attrib ();
/* g_funcs.c */
SCM g_funcs_print(SCM filename);
SCM g_funcs_pdf(SCM filename);
SCM g_funcs_image(SCM filename);
SCM g_funcs_exit(void);
SCM g_funcs_log(SCM msg);
SCM g_funcs_msg(SCM msg);
SCM g_funcs_confirm(SCM msg);
SCM g_funcs_filesel(SCM msg, SCM templ, SCM flags);
SCM g_funcs_use_rc_values(void);
SCM get_selected_component_attributes(GSCHEM_TOPLEVEL *w_current);
/* g_hook.c */
void g_init_hook ();
void g_run_hook_object (GSCHEM_TOPLEVEL *w_current, const char *name, OBJECT *obj);
void g_run_hook_object_list (GSCHEM_TOPLEVEL *w_current, const char *name, GList *obj_lst);
void g_run_hook_page (GSCHEM_TOPLEVEL *w_current, const char *name, PAGE *page);
/* g_keys.c */
void g_keys_reset (GSCHEM_TOPLEVEL *w_current);
int g_keys_execute(GSCHEM_TOPLEVEL *w_current, GdkEventKey *event);
GtkListStore *g_keys_to_list_store (void);
SCM g_keys_file_new(SCM rest);
SCM g_keys_file_new_window(SCM rest);
SCM g_keys_file_open(SCM rest);
SCM g_keys_file_script(SCM rest);
SCM g_keys_file_save(SCM rest);
SCM g_keys_file_save_as(SCM rest);
SCM g_keys_file_save_all(SCM rest);
SCM g_keys_file_print(SCM rest);
SCM g_keys_file_write_png(SCM rest);
SCM g_keys_file_close(SCM rest);
SCM g_keys_file_quit(SCM rest);
SCM g_keys_edit_undo(SCM rest);
SCM g_keys_edit_redo(SCM rest);
SCM g_keys_edit_select(SCM rest);
SCM g_keys_edit_select_all(SCM rest);
SCM g_keys_edit_deselect(SCM rest);
SCM g_keys_edit_copy(SCM rest);
SCM g_keys_edit_copy_hotkey(SCM rest);
SCM g_keys_edit_mcopy(SCM rest);
SCM g_keys_edit_mcopy_hotkey(SCM rest);
SCM g_keys_edit_move(SCM rest);
SCM g_keys_edit_move_hotkey(SCM rest);
SCM g_keys_edit_delete(SCM rest);
SCM g_keys_edit_rotate_90(SCM rest);
SCM g_keys_edit_rotate_90_hotkey(SCM rest);
SCM g_keys_edit_mirror(SCM rest);
SCM g_keys_edit_mirror_hotkey(SCM rest);
SCM g_keys_edit_slot(SCM rest);
SCM g_keys_edit_color(SCM rest);
SCM g_keys_edit_edit(SCM rest);
SCM g_keys_edit_pin_type(SCM rest);
SCM g_keys_edit_text(SCM rest);
SCM g_keys_edit_lock(SCM rest);
SCM g_keys_edit_unlock(SCM rest);
SCM g_keys_edit_linetype(SCM rest);
SCM g_keys_edit_filltype(SCM rest);
SCM g_keys_edit_translate(SCM rest);
SCM g_keys_edit_invoke_macro(SCM rest);
SCM g_keys_edit_embed(SCM rest);
SCM g_keys_edit_unembed(SCM rest);
SCM g_keys_edit_update(SCM rest);
SCM g_keys_edit_show_hidden(SCM rest);
SCM g_keys_edit_find(SCM rest);
SCM g_keys_edit_show_text(SCM rest);
SCM g_keys_edit_hide_text(SCM rest);
SCM g_keys_edit_autonumber_text(SCM rest);
SCM g_keys_clipboard_copy(SCM rest);
SCM g_keys_clipboard_cut(SCM rest);
SCM g_keys_clipboard_paste(SCM rest);
SCM g_keys_clipboard_paste_hotkey(SCM rest);
SCM g_keys_buffer_copy1(SCM rest);
SCM g_keys_buffer_copy2(SCM rest);
SCM g_keys_buffer_copy3(SCM rest);
SCM g_keys_buffer_copy4(SCM rest);
SCM g_keys_buffer_copy5(SCM rest);
SCM g_keys_buffer_cut1(SCM rest);
SCM g_keys_buffer_cut2(SCM rest);
SCM g_keys_buffer_cut3(SCM rest);
SCM g_keys_buffer_cut4(SCM rest);
SCM g_keys_buffer_cut5(SCM rest);
SCM g_keys_buffer_paste1(SCM rest);
SCM g_keys_buffer_paste2(SCM rest);
SCM g_keys_buffer_paste3(SCM rest);
SCM g_keys_buffer_paste4(SCM rest);
SCM g_keys_buffer_paste5(SCM rest);
SCM g_keys_buffer_paste1_hotkey(SCM rest);
SCM g_keys_buffer_paste2_hotkey(SCM rest);
SCM g_keys_buffer_paste3_hotkey(SCM rest);
SCM g_keys_buffer_paste4_hotkey(SCM rest);
SCM g_keys_buffer_paste5_hotkey(SCM rest);
SCM g_keys_view_redraw(SCM rest);
SCM g_keys_view_zoom_full(SCM rest);
SCM g_keys_view_zoom_extents(SCM rest);
SCM g_keys_view_zoom_in(SCM rest);
SCM g_keys_view_zoom_out(SCM rest);
SCM g_keys_view_zoom_in_hotkey(SCM rest);
SCM g_keys_view_zoom_out_hotkey(SCM rest);
SCM g_keys_view_zoom_box(SCM rest);
SCM g_keys_view_zoom_box_hotkey(SCM rest);
SCM g_keys_view_pan(SCM rest);
SCM g_keys_view_pan_left(SCM rest);
SCM g_keys_view_pan_right(SCM rest);
SCM g_keys_view_pan_up(SCM rest);
SCM g_keys_view_pan_down(SCM rest);
SCM g_keys_view_pan_hotkey(SCM rest);
SCM g_keys_view_dark_colors(SCM rest);
SCM g_keys_view_light_colors(SCM rest);
SCM g_keys_view_bw_colors(SCM rest);
SCM g_keys_page_manager(SCM rest);
SCM g_keys_page_next(SCM rest);
SCM g_keys_page_prev(SCM rest);
SCM g_keys_page_new(SCM rest);
SCM g_keys_page_close(SCM rest);
SCM g_keys_page_revert(SCM rest);
SCM g_keys_page_discard(SCM rest);
SCM g_keys_page_print(SCM rest);
SCM g_keys_add_component(SCM rest);
SCM g_keys_add_attribute(SCM rest);
SCM g_keys_add_attribute_hotkey(SCM rest);
SCM g_keys_add_net(SCM rest);
SCM g_keys_add_net_hotkey(SCM rest);
SCM g_keys_add_bus(SCM rest);
SCM g_keys_add_bus_hotkey(SCM rest);
SCM g_keys_add_text(SCM rest);
SCM g_keys_add_line(SCM rest);
SCM g_keys_add_line_hotkey(SCM rest);
SCM g_keys_add_box(SCM rest);
SCM g_keys_add_box_hotkey(SCM rest);
SCM g_keys_add_picture(SCM rest);
SCM g_keys_add_picture_hotkey(SCM rest);
SCM g_keys_add_circle(SCM rest);
SCM g_keys_add_circle_hotkey(SCM rest);
SCM g_keys_add_arc(SCM rest);
SCM g_keys_add_arc_hotkey(SCM rest);
SCM g_keys_add_pin(SCM rest);
SCM g_keys_add_pin_hotkey(SCM rest);
SCM g_keys_hierarchy_down_schematic(SCM rest);
SCM g_keys_hierarchy_down_symbol(SCM rest);
SCM g_keys_hierarchy_up(SCM rest);
SCM g_keys_attributes_attach(SCM rest);
SCM g_keys_attributes_detach(SCM rest);
SCM g_keys_attributes_show_name(SCM rest);
SCM g_keys_attributes_show_value(SCM rest);
SCM g_keys_attributes_show_both(SCM rest);
SCM g_keys_attributes_visibility_toggle(SCM rest);
SCM g_keys_script_console(SCM rest);
SCM g_keys_options_text_size(SCM rest);
SCM g_keys_options_afeedback(SCM rest);
SCM g_keys_options_grid(SCM rest);
SCM g_keys_options_snap(SCM rest);
SCM g_keys_options_snap_size(SCM rest);
SCM g_keys_options_scale_up_snap_size(SCM rest);
SCM g_keys_options_scale_down_snap_size(SCM rest);
SCM g_keys_options_rubberband(SCM rest);
SCM g_keys_options_magneticnet(SCM rest);
SCM g_keys_options_show_log_window(SCM rest);
SCM g_keys_options_show_coord_window(SCM rest);
SCM g_keys_misc(SCM rest);
SCM g_keys_misc2(SCM rest);
SCM g_keys_misc3(SCM rest);
SCM g_keys_help_about(SCM rest);
SCM g_keys_help_hotkeys(SCM rest);
SCM g_keys_cancel(SCM rest);
void g_init_keys ();
/* g_rc.c */
void g_rc_parse_gtkrc();
SCM g_rc_gschem_version(SCM version);
SCM g_rc_net_direction_mode(SCM mode);
SCM g_rc_net_selection_mode(SCM mode);
SCM g_rc_action_feedback_mode(SCM mode);
SCM g_rc_zoom_with_pan(SCM mode);
SCM g_rc_scrollbar_update(SCM mode);
SCM g_rc_logging(SCM mode);
SCM g_rc_embed_components(SCM mode);
SCM g_rc_component_dialog_attributes(SCM stringlist);
SCM g_rc_text_size(SCM size);
SCM g_rc_text_caps_style(SCM mode);
SCM g_rc_snap_size(SCM size);
SCM g_rc_logging_destination(SCM mode);
SCM g_rc_attribute_name(SCM path);
SCM g_rc_scrollbars(SCM mode);
SCM g_rc_image_color(SCM mode);
SCM g_rc_image_size(SCM width, SCM height);
SCM g_rc_log_window(SCM mode);
SCM g_rc_log_window_type(SCM mode);
SCM g_rc_third_button(SCM mode);
SCM g_rc_middle_button(SCM mode);
SCM g_rc_scroll_wheel(SCM mode);
SCM g_rc_net_consolidate(SCM mode);
SCM g_rc_file_preview(SCM mode);
SCM g_rc_enforce_hierarchy(SCM mode);
SCM g_rc_fast_mousepan(SCM mode);
SCM g_rc_raise_dialog_boxes_on_expose(SCM mode);
SCM g_rc_continue_component_place(SCM mode);
SCM g_rc_undo_levels(SCM levels);
SCM g_rc_undo_control(SCM mode);
SCM g_rc_undo_type(SCM mode);
SCM g_rc_undo_panzoom(SCM mode);
SCM g_rc_draw_grips(SCM mode);
SCM g_rc_netconn_rubberband(SCM mode);
SCM g_rc_magnetic_net_mode(SCM mode);
SCM g_rc_sort_component_library(SCM mode);
SCM g_rc_add_menu(SCM menu_name, SCM menu_items);
SCM g_rc_window_size(SCM width, SCM height);
SCM g_rc_warp_cursor(SCM mode);
SCM g_rc_toolbars(SCM mode);
SCM g_rc_handleboxes(SCM mode);
SCM g_rc_bus_ripper_size(SCM size);
SCM g_rc_bus_ripper_type(SCM mode);
SCM g_rc_bus_ripper_rotation(SCM mode);
SCM g_rc_force_boundingbox(SCM mode);
SCM g_rc_dots_grid_dot_size(SCM dotsize);
SCM g_rc_dots_grid_mode(SCM mode);
SCM g_rc_dots_grid_fixed_threshold(SCM spacing);
SCM g_rc_mesh_grid_display_threshold(SCM spacing);
SCM g_rc_add_attribute_offset(SCM offset);
SCM g_rc_auto_save_interval(SCM seconds);
SCM g_rc_mousepan_gain(SCM mode);
SCM g_rc_keyboardpan_gain(SCM mode);
SCM g_rc_select_slack_pixels(SCM pixels);
SCM g_rc_zoom_gain(SCM gain);
SCM g_rc_scrollpan_steps(SCM steps);
SCM g_rc_display_color_map (SCM scm_map);
SCM g_rc_display_outline_color_map (SCM scm_map);
/* g_register.c */
void g_register_funcs(void);
/* g_select.c */
void g_init_select ();
/* g_util.c */
void g_init_util ();
/* g_window.c */
GSCHEM_TOPLEVEL *g_current_window ();
void g_dynwind_window (GSCHEM_TOPLEVEL *w_current);
void g_init_window ();
/* globals.c */
/* gschem.c */
typedef void (*gschem_atexit_func)(gpointer data);
void gschem_atexit(gschem_atexit_func func, gpointer data);
void gschem_quit(void);
void main_prog(void *closure, int argc, char *argv[]);
int main(int argc, char *argv[]);
/* i_basic.c */
void i_show_state(GSCHEM_TOPLEVEL *w_current, const char *message);
void i_set_state(GSCHEM_TOPLEVEL *w_current, enum x_states newstate);
void i_set_state_msg(GSCHEM_TOPLEVEL *w_current, enum x_states newstate, const char *message);
void i_update_middle_button(GSCHEM_TOPLEVEL *w_current, void (*func_ptr)(), const char *string);
void i_update_toolbar(GSCHEM_TOPLEVEL *w_current);
void i_update_menus(GSCHEM_TOPLEVEL *w_current);
void i_set_filename(GSCHEM_TOPLEVEL *w_current, const gchar *string);
void i_update_grid_info(GSCHEM_TOPLEVEL *w_current);
/* i_callbacks.c */
void i_callback_file_new(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_toolbar_file_new(GtkWidget *widget, gpointer data);
void i_callback_file_new_window(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_file_open(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_toolbar_file_open(GtkWidget *widget, gpointer data);
void i_callback_file_script(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_file_save(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_toolbar_file_save(GtkWidget *widget, gpointer data);
void i_callback_file_save_all(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_file_save_as(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_file_print(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_file_write_png(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_file_close(gpointer data, guint callback_action, GtkWidget *widget);
int i_callback_close(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_file_quit(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_undo(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_toolbar_edit_undo(GtkWidget *widget, gpointer data);
void i_callback_edit_redo(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_toolbar_edit_redo(GtkWidget *widget, gpointer data);
void i_callback_edit_select(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_toolbar_edit_select(GtkWidget *widget, gpointer data);
void i_callback_edit_select_all(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_deselect(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_copy(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_copy_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_mcopy(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_mcopy_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_move(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_move_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_delete(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_edit(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_pin_type(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_text(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_slot(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_color(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_rotate_90(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_rotate_90_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_mirror(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_mirror_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_lock(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_unlock(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_translate(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_invoke_macro(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_embed(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_unembed(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_update(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_show_hidden(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_find(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_hide_text(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_show_text(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_autonumber_text(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_linetype(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_edit_filltype(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_view_redraw(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_view_zoom_full(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_view_zoom_extents(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_view_zoom_box(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_view_zoom_box_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_view_zoom_in(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_view_zoom_out(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_view_zoom_in_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_view_zoom_out_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_view_pan(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_view_pan_left(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_view_pan_right(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_view_pan_up(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_view_pan_down(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_view_pan_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_view_dark_colors(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_view_light_colors(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_view_bw_colors(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_page_manager(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_page_next(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_page_prev(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_page_new(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_page_close(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_page_revert(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_page_discard(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_page_print(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_clipboard_copy(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_clipboard_cut(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_clipboard_paste(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_clipboard_paste_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_copy1(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_copy2(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_copy3(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_copy4(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_copy5(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_cut1(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_cut2(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_cut3(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_cut4(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_cut5(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_paste1(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_paste2(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_paste3(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_paste4(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_paste5(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_paste1_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_paste2_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_paste3_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_paste4_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_buffer_paste5_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_add_component(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_toolbar_add_component(GtkWidget *widget, gpointer data);
void i_callback_add_attribute(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_add_attribute_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_add_net(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_add_net_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_toolbar_add_net(GtkWidget *widget, gpointer data);
void i_callback_add_bus(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_add_bus_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_toolbar_add_bus(GtkWidget *widget, gpointer data);
void i_callback_add_text(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_toolbar_add_text(GtkWidget *widget, gpointer data);
void i_callback_add_line(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_add_line_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_add_box(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_add_box_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_add_picture(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_add_picture_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_add_circle(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_add_circle_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_add_arc(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_add_arc_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_add_pin(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_add_pin_hotkey(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_hierarchy_down_schematic(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_hierarchy_down_symbol(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_hierarchy_up(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_attributes_attach(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_attributes_detach(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_attributes_show_name(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_attributes_show_value(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_attributes_show_both(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_attributes_visibility_toggle(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_script_console(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_options_text_size(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_options_snap_size(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_options_scale_up_snap_size(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_options_scale_down_snap_size(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_options_afeedback(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_options_grid(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_options_snap(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_options_rubberband(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_options_magneticnet(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_options_show_log_window(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_misc(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_misc2(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_misc3(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_cancel(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_help_about(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_help_hotkeys(gpointer data, guint callback_action, GtkWidget *widget);
void i_callback_options_show_coord_window(gpointer data, guint callback_action, GtkWidget *widget);
gboolean i_callback_close_wm(GtkWidget *widget, GdkEvent *event, gpointer data);
/* i_vars.c */
void i_vars_set(GSCHEM_TOPLEVEL *w_current);
void i_vars_freenames();
 /* m_basic.c */
int mil_x(GSCHEM_TOPLEVEL *w_current, int val);
int mil_y(GSCHEM_TOPLEVEL *w_current, int val);
int pix_x(GSCHEM_TOPLEVEL *w_current, int val);
int pix_y(GSCHEM_TOPLEVEL *w_current, int val);
void WORLDtoSCREEN(GSCHEM_TOPLEVEL *w_current, int x, int y, int *px, int *py);
void SCREENtoWORLD(GSCHEM_TOPLEVEL *w_current, int mx, int my, int *x, int *y);
int snap_grid(GSCHEM_TOPLEVEL *w_current, int input);
int SCREENabs(GSCHEM_TOPLEVEL *w_current, int val);
int WORLDabs(GSCHEM_TOPLEVEL *w_current, int val);
int WORLDclip_change(GSCHEM_TOPLEVEL *w_current, int *x1, int *y1, int *x2, int *y2);
int clip_nochange(GSCHEM_TOPLEVEL *w_current, int x1, int y1, int x2, int y2);
int visible(GSCHEM_TOPLEVEL *w_current, int wleft, int wtop, int wright, int wbottom);
double round_5_2_1(double unrounded);
/* o_arc.c */
void o_arc_invalidate_rubber(GSCHEM_TOPLEVEL *w_current);
void o_arc_start(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_arc_end1(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_arc_end4(GSCHEM_TOPLEVEL *w_current, int radius, int start_angle, int end_angle);
void o_arc_motion(GSCHEM_TOPLEVEL *w_current, int x, int y, int whichone);
void o_arc_draw_rubber(GSCHEM_TOPLEVEL *w_current, EdaRenderer *renderer);
/* o_attrib.c */
void o_attrib_add_selected(GSCHEM_TOPLEVEL *w_current, SELECTION *selection, OBJECT *selected);
void o_attrib_deselect_invisible(GSCHEM_TOPLEVEL *w_current, SELECTION *selection, OBJECT *selected);
void o_attrib_select_invisible(GSCHEM_TOPLEVEL *w_current, SELECTION *selection, OBJECT *selected);
void o_attrib_toggle_visibility(GSCHEM_TOPLEVEL *w_current, OBJECT *object);
void o_attrib_toggle_show_name_value(GSCHEM_TOPLEVEL *w_current, OBJECT *object, int new_show_name_value);
OBJECT *o_attrib_add_attrib(GSCHEM_TOPLEVEL *w_current, const char *text_string, int visibility, int show_name_value, OBJECT *object);
/* o_basic.c */
void o_redraw_rects(GSCHEM_TOPLEVEL *w_current, GdkRectangle *rectangles, int n_rectangles);
int o_invalidate_rubber(GSCHEM_TOPLEVEL *w_current);
int o_redraw_cleanstates(GSCHEM_TOPLEVEL *w_current);
void o_draw_place(GSCHEM_TOPLEVEL *w_current, int dx, int dy, OBJECT *object);
void o_invalidate_rect(GSCHEM_TOPLEVEL *w_current, int x1, int y1, int x2, int y2);
void o_invalidate_all(GSCHEM_TOPLEVEL *w_current);
void o_invalidate(GSCHEM_TOPLEVEL *w_current, OBJECT *object);
void o_invalidate_glist(GSCHEM_TOPLEVEL *w_current, GList *list);
COLOR *o_drawing_color(GSCHEM_TOPLEVEL *w_current, OBJECT *object);
/* o_box.c */
void o_box_draw(GSCHEM_TOPLEVEL *w_current, OBJECT *o_current);
void o_box_invalidate_rubber(GSCHEM_TOPLEVEL *w_current);
void o_box_start(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_box_end(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_box_motion(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_box_draw_rubber (GSCHEM_TOPLEVEL *w_current, EdaRenderer *renderer);
/* o_buffer.c */
void o_buffer_copy(GSCHEM_TOPLEVEL *w_current, int buf_num);
void o_buffer_cut(GSCHEM_TOPLEVEL *w_current, int buf_num);
void o_buffer_paste_start(GSCHEM_TOPLEVEL *w_current, int x, int y, int buf_num);
void o_buffer_init(void);
void o_buffer_free(GSCHEM_TOPLEVEL *w_current);
/* o_bus.c */
void o_bus_start(GSCHEM_TOPLEVEL *w_current, int x, int y);
int o_bus_end(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_bus_motion(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_bus_draw_rubber (GSCHEM_TOPLEVEL *w_current, EdaRenderer *renderer);
void o_bus_invalidate_rubber(GSCHEM_TOPLEVEL *w_current);
/* o_circle.c */
void o_circle_draw(GSCHEM_TOPLEVEL *w_current, OBJECT *o_current);
void o_circle_invalidate_rubber(GSCHEM_TOPLEVEL *w_current);
void o_circle_start(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_circle_end(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_circle_motion(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_circle_draw_rubber (GSCHEM_TOPLEVEL *w_current, EdaRenderer *renderer);
/* o_complex.c */
void o_complex_prepare_place(GSCHEM_TOPLEVEL *w_current, const CLibSymbol *sym);
void o_complex_place_changed_run_hook(GSCHEM_TOPLEVEL *w_current);
void o_complex_translate_all(GSCHEM_TOPLEVEL *w_current, int offset);
/* o_copy.c */
void o_copy_start(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_copy_end(GSCHEM_TOPLEVEL *w_current);
void o_copy_multiple_end(GSCHEM_TOPLEVEL *w_current);
/* o_cue.c */
/* o_delete.c */
void o_delete(GSCHEM_TOPLEVEL *w_current, OBJECT *object);
void o_delete_selected(GSCHEM_TOPLEVEL *w_current);
/* o_find.c */
gboolean o_find_object(GSCHEM_TOPLEVEL *w_current, int x, int y,
		       gboolean deselect_afterwards);
gboolean o_find_selected_object(GSCHEM_TOPLEVEL *w_current, int x, int y);
/* o_grips.c */
OBJECT *o_grips_search_world(GSCHEM_TOPLEVEL *w_current, int x, int y, int *whichone);
OBJECT *o_grips_search_arc_world(GSCHEM_TOPLEVEL *w_current, OBJECT *o_current, int x, int y, int size, int *whichone);
OBJECT *o_grips_search_box_world(GSCHEM_TOPLEVEL *w_current, OBJECT *o_current, int x, int y, int size, int *whichone);
OBJECT *o_grips_search_path_world(GSCHEM_TOPLEVEL *w_current, OBJECT *o_current, int x, int y, int size, int *whichone);
OBJECT *o_grips_search_picture_world(GSCHEM_TOPLEVEL *w_current, OBJECT *o_current, int x, int y, int size, int *whichone);
OBJECT *o_grips_search_circle_world(GSCHEM_TOPLEVEL *w_current, OBJECT *o_current, int x, int y, int size, int *whichone);
OBJECT *o_grips_search_line_world(GSCHEM_TOPLEVEL *w_current, OBJECT *o_current, int x, int y, int size, int *whichone);
int o_grips_start(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_grips_motion(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_grips_end(GSCHEM_TOPLEVEL *w_current);
void o_grips_cancel(GSCHEM_TOPLEVEL *w_current);
int o_grips_size(GSCHEM_TOPLEVEL *w_current);
void o_grips_draw_rubber(GSCHEM_TOPLEVEL *w_current, EdaRenderer *renderer);
/* o_line.c */
void o_line_invalidate_rubber(GSCHEM_TOPLEVEL *w_current);
void o_line_start(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_line_end(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_line_motion(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_line_draw_rubber(GSCHEM_TOPLEVEL *w_current, EdaRenderer *renderer);
int o_line_visible(GSCHEM_TOPLEVEL *w_current, LINE *line, int *x1, int *y1, int *x2, int *y2);
/* o_misc.c */
void o_edit(GSCHEM_TOPLEVEL *w_current, GList *list);
void o_lock(GSCHEM_TOPLEVEL *w_current);
void o_unlock(GSCHEM_TOPLEVEL *w_current);
void o_rotate_world_update(GSCHEM_TOPLEVEL *w_current, int centerx, int centery, int angle, GList *list);
void o_mirror_world_update(GSCHEM_TOPLEVEL *w_current, int centerx, int centery, GList *list);
void o_edit_show_hidden_lowlevel(GSCHEM_TOPLEVEL *w_current, const GList *o_list);
void o_edit_show_hidden(GSCHEM_TOPLEVEL *w_current, const GList *o_list);
int o_edit_find_text(GSCHEM_TOPLEVEL *w_current, const GList *o_list, char *stext, int descend, int skip);
void o_edit_hide_specific_text(GSCHEM_TOPLEVEL *w_current, const GList *o_list, char *stext);
void o_edit_show_specific_text(GSCHEM_TOPLEVEL *w_current, const GList *o_list, char *stext);
OBJECT *o_update_component(GSCHEM_TOPLEVEL *w_current, OBJECT *o_current);
void o_autosave_backups(GSCHEM_TOPLEVEL *w_current);
/* o_move.c */
void o_move_start(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_move_end_lowlevel(GSCHEM_TOPLEVEL *w_current, OBJECT *object, int diff_x, int diff_y);
void o_move_end(GSCHEM_TOPLEVEL *w_current);
void o_move_cancel(GSCHEM_TOPLEVEL *w_current);
void o_move_motion(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_move_invalidate_rubber(GSCHEM_TOPLEVEL *w_current, int drawing);
void o_move_draw_rubber(GSCHEM_TOPLEVEL *w_current, EdaRenderer *renderer);
int o_move_return_whichone(OBJECT *object, int x, int y);
void o_move_check_endpoint(GSCHEM_TOPLEVEL *w_current, OBJECT *object);
void o_move_prep_rubberband(GSCHEM_TOPLEVEL *w_current);
int o_move_zero_length(OBJECT *object);
void o_move_end_rubberband(GSCHEM_TOPLEVEL *w_current, int world_diff_x, int world_diff_y, GList **objects);
/* o_net.c */
void o_net_reset(GSCHEM_TOPLEVEL *w_current); 
void o_net_guess_direction(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_net_find_magnetic(GSCHEM_TOPLEVEL *w_current, int event_x, int event_y);
void o_net_finishmagnetic(GSCHEM_TOPLEVEL *w_current);
void o_net_start_magnetic(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_net_start(GSCHEM_TOPLEVEL *w_current, int x, int y);
int o_net_end(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_net_motion(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_net_draw_rubber(GSCHEM_TOPLEVEL *w_current, EdaRenderer *renderer);
void o_net_invalidate_rubber(GSCHEM_TOPLEVEL *w_current);
int o_net_add_busrippers(GSCHEM_TOPLEVEL *w_current, OBJECT *net_obj, GList *other_objects);
/* o_picture.c */
void o_picture_start(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_picture_end(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_picture_motion(GSCHEM_TOPLEVEL *w_current, int x, int y);
void picture_selection_dialog (GSCHEM_TOPLEVEL *w_current);
void o_picture_invalidate_rubber(GSCHEM_TOPLEVEL *w_current);
void o_picture_draw_rubber(GSCHEM_TOPLEVEL *w_current, EdaRenderer *renderer);
gboolean o_picture_exchange(GSCHEM_TOPLEVEL *w_current, const gchar *filename, GError **error);
void picture_change_filename_dialog (GSCHEM_TOPLEVEL *w_current);
void o_picture_set_pixbuf(GSCHEM_TOPLEVEL *w_current, GdkPixbuf *pixbuf, char *filename);

/* o_path.c */
void o_path_invalidate_rubber(GSCHEM_TOPLEVEL *w_current);
void o_path_start(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_path_end(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_path_motion(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_path_draw_rubber(GSCHEM_TOPLEVEL *w_current, EdaRenderer *renderer);

/* o_pin.c */
void o_pin_start(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_pin_end(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_pin_motion(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_pin_draw_rubber(GSCHEM_TOPLEVEL *w_current, EdaRenderer *renderer);
void o_pin_invalidate_rubber(GSCHEM_TOPLEVEL *w_current);
/* o_place.c */
void o_place_start(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_place_end(GSCHEM_TOPLEVEL *w_current, int x, int y, int continue_placing, GList **ret_new_objects, const char *hook_name);
void o_place_motion(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_place_invalidate_rubber(GSCHEM_TOPLEVEL *w_current, int drawing);
void o_place_draw_rubber(GSCHEM_TOPLEVEL *w_current, EdaRenderer *renderer);
void o_place_rotate(GSCHEM_TOPLEVEL *w_current);
/* o_select.c */
void o_select_run_hooks(GSCHEM_TOPLEVEL *w_current, OBJECT *o_current, int flag);
void o_select_object(GSCHEM_TOPLEVEL *w_current, OBJECT *o_current, int type, int count);
int o_select_box_start(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_select_box_end(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_select_box_motion(GSCHEM_TOPLEVEL *w_current, int x, int y);
void o_select_box_invalidate_rubber(GSCHEM_TOPLEVEL *w_current);
void o_select_box_draw_rubber(GSCHEM_TOPLEVEL *w_current, EdaRenderer *renderer);
void o_select_box_search(GSCHEM_TOPLEVEL *w_current);
void o_select_connected_nets(GSCHEM_TOPLEVEL *w_current, OBJECT* o_current);
OBJECT *o_select_return_first_object(GSCHEM_TOPLEVEL *w_current);
int o_select_selected(GSCHEM_TOPLEVEL *w_current);
void o_select_unselect_all(GSCHEM_TOPLEVEL *w_current);
void o_select_visible_unlocked(GSCHEM_TOPLEVEL *w_current);
void o_select_move_to_place_list(GSCHEM_TOPLEVEL *w_current);
/* o_slot.c */
void o_slot_start(GSCHEM_TOPLEVEL *w_current, OBJECT *object);
void o_slot_end(GSCHEM_TOPLEVEL *w_current, OBJECT *object, const char *string);
/* o_text.c */
int o_text_get_rendered_bounds(void *user_data, OBJECT *object, int *min_x, int *min_y, int *max_x, int *max_y);
void o_text_prepare_place(GSCHEM_TOPLEVEL *w_current, char *text);
void o_text_edit(GSCHEM_TOPLEVEL *w_current, OBJECT *o_current);
void o_text_edit_end(GSCHEM_TOPLEVEL *w_current, char *string, int len, int text_size, int text_alignment);
void o_text_change(GSCHEM_TOPLEVEL *w_current, OBJECT *object, char *string, int visibility, int show);
/* o_undo.c */
void o_undo_init(void);
void o_undo_savestate(GSCHEM_TOPLEVEL *w_current, int flag);
char *o_undo_find_prev_filename(UNDO *start);
GList *o_undo_find_prev_object_head(UNDO *start);
void o_undo_callback(GSCHEM_TOPLEVEL *w_current, int type);
void o_undo_cleanup(void);
void o_undo_remove_last_undo(GSCHEM_TOPLEVEL *w_current);
/* parsecmd.c */
int parse_commandline(int argc, char *argv[]);
/* s_stretch.c */
GList *s_stretch_add(GList *list, OBJECT *object, int whichone);
GList *s_stretch_remove(GList *list, OBJECT *object);
void s_stretch_print_all(GList *list);
void s_stretch_destroy_all(GList *list);
/* x_attribedit.c */
gint option_menu_get_history(GtkOptionMenu *option_menu);
void attrib_edit_dialog_ok(GtkWidget *w, GSCHEM_TOPLEVEL *w_current);
void attrib_edit_dialog(GSCHEM_TOPLEVEL *w_current, OBJECT *attr_obj, int flag);
/* x_autonumber.c */
void autonumber_text_dialog(GSCHEM_TOPLEVEL *w_current);
/* x_basic.c */
void x_repaint_background_region(GSCHEM_TOPLEVEL *w_current, int x, int y, int width, int height);
void x_hscrollbar_set_ranges(GSCHEM_TOPLEVEL *w_current);
void x_hscrollbar_update(GSCHEM_TOPLEVEL *w_current);
void x_vscrollbar_set_ranges(GSCHEM_TOPLEVEL *w_current);
void x_vscrollbar_update(GSCHEM_TOPLEVEL *w_current);
void x_scrollbars_update(GSCHEM_TOPLEVEL *w_current);
void x_basic_warp_cursor(GtkWidget *widget, gint x, gint y);
/* x_clipboard.c */
void x_clipboard_init (GSCHEM_TOPLEVEL *w_current);
void x_clipboard_finish (GSCHEM_TOPLEVEL *w_current);
void x_clipboard_query_usable (GSCHEM_TOPLEVEL *w_current, void (*callback) (int, void *), void *userdata);
gboolean x_clipboard_set (GSCHEM_TOPLEVEL *w_current, const GList *object_list);
GList *x_clipboard_get (GSCHEM_TOPLEVEL *w_current);
/* x_color.c */
void x_color_init (void);
void x_color_free (void);
void x_color_allocate (void);
GdkColor *x_get_color(int color);
GdkColor *x_get_darkcolor(int color);
COLOR *x_color_lookup(int color);
COLOR *x_color_lookup_dark(int color);
gchar *x_color_get_name(int index);
gboolean x_color_display_enabled (int index);
/* x_dialog.c */
int text_view_calculate_real_tab_width(GtkTextView *textview, int tab_size);
void select_all_text_in_textview(GtkTextView *textview);
void destroy_window(GtkWidget *widget, GtkWidget **window);
void text_input_dialog_apply(GtkWidget *w, GSCHEM_TOPLEVEL *w_current);
void text_input_dialog(GSCHEM_TOPLEVEL *w_current);
gint change_alignment(GtkComboBox *w, GSCHEM_TOPLEVEL *w_current);
void text_edit_dialog_ok(GtkWidget *w, GSCHEM_TOPLEVEL *w_current);
void text_edit_dialog(GSCHEM_TOPLEVEL *w_current, const char *string, int text_size, int text_alignment);
void line_type_dialog(GSCHEM_TOPLEVEL *w_current);
void fill_type_dialog(GSCHEM_TOPLEVEL *w_current);
void arc_angle_dialog(GSCHEM_TOPLEVEL *w_current, OBJECT *arc_object);
void translate_dialog(GSCHEM_TOPLEVEL *w_current);
void text_size_dialog(GSCHEM_TOPLEVEL *w_current);
void snap_size_dialog(GSCHEM_TOPLEVEL *w_current);
void slot_edit_dialog(GSCHEM_TOPLEVEL *w_current, const char *string);
void about_dialog(GSCHEM_TOPLEVEL *w_current);
void coord_display_update(GSCHEM_TOPLEVEL *w_current, int x, int y);
void coord_dialog(GSCHEM_TOPLEVEL *w_current, int x, int y);
gint color_set(GtkWidget *w, gpointer data);
char *index2functionstring(int index);
void color_edit_dialog_apply(GtkWidget *w, GSCHEM_TOPLEVEL *w_current);
void color_edit_dialog(GSCHEM_TOPLEVEL *w_current);
void x_dialog_hotkeys(GSCHEM_TOPLEVEL *w_current);
void x_dialog_raise_all(GSCHEM_TOPLEVEL *w_current);

void generic_msg_dialog(const char *);
int generic_confirm_dialog(const char *);
char * generic_filesel_dialog(const char *, const char *, gint);

void find_text_dialog(GSCHEM_TOPLEVEL *w_current);
void hide_text_dialog(GSCHEM_TOPLEVEL *w_current);
void show_text_dialog(GSCHEM_TOPLEVEL *w_current);
void major_changed_dialog(GSCHEM_TOPLEVEL* w_current);
gboolean x_dialog_close_changed_page (GSCHEM_TOPLEVEL *w_current, PAGE *page);
gboolean x_dialog_close_window (GSCHEM_TOPLEVEL *w_current);
int x_dialog_validate_attribute(GtkWindow* parent, char *attribute);
void x_dialog_edit_pin_type(GSCHEM_TOPLEVEL *w_current, const GList *obj_list);
/* x_event.c */
gint x_event_expose(GtkWidget *widget, GdkEventExpose *event, GSCHEM_TOPLEVEL *w_current);
gint x_event_button_pressed(GtkWidget *widget, GdkEventButton *event, GSCHEM_TOPLEVEL *w_current);
gint x_event_button_released(GtkWidget *widget, GdkEventButton *event, GSCHEM_TOPLEVEL *w_current);
gint x_event_motion(GtkWidget *widget, GdkEventMotion *event, GSCHEM_TOPLEVEL *w_current);
gboolean x_event_configure (GtkWidget *widget, GdkEventConfigure *event, gpointer user_data);
void x_manual_resize(GSCHEM_TOPLEVEL *w_current);
void x_event_hschanged(GtkAdjustment *adj, GSCHEM_TOPLEVEL *w_current);
void x_event_vschanged(GtkAdjustment *adj, GSCHEM_TOPLEVEL *w_current);
gint x_event_enter(GtkWidget *widget, GdkEventCrossing *event, GSCHEM_TOPLEVEL *w_current);
gboolean x_event_key(GtkWidget *widget, GdkEventKey *event, GSCHEM_TOPLEVEL *w_current);
gint x_event_scroll(GtkWidget *widget, GdkEventScroll *event, GSCHEM_TOPLEVEL *w_current);
gboolean x_event_get_pointer_position (GSCHEM_TOPLEVEL *w_current, gboolean snapped, gint *wx, gint *wy);
/* x_compselect.c */
void x_compselect_open (GSCHEM_TOPLEVEL *w_current);
void x_compselect_close (GSCHEM_TOPLEVEL *w_current);
void x_compselect_deselect (GSCHEM_TOPLEVEL *w_current);
/* x_fileselect.c */
void x_fileselect_open(GSCHEM_TOPLEVEL *w_current);
void x_fileselect_save(GSCHEM_TOPLEVEL *w_current);
int x_fileselect_load_backup(void *user_data, GString *message);
/* x_grid.c */
void x_grid_draw_region(GSCHEM_TOPLEVEL *w_current, int x, int y, int width, int height);
int x_grid_query_drawn_spacing(GSCHEM_TOPLEVEL *w_current);
void x_draw_tiles(GSCHEM_TOPLEVEL *w_current);
/* x_image.c */
void x_image_lowlevel(GSCHEM_TOPLEVEL *w_current, const char* filename,
		      int desired_width, int desired_height, char *filetype);
void x_image_setup(GSCHEM_TOPLEVEL *w_current);
GdkPixbuf *x_image_get_pixbuf (GSCHEM_TOPLEVEL *w_current);
/* x_log.c */
void x_log_open ();
void x_log_close ();
void x_log_message (const gchar *log_domain,
                    GLogLevelFlags log_level,
                    const gchar *message);
/* x_misc.c */
gboolean x_show_uri (GSCHEM_TOPLEVEL *w_current, const gchar *buf, GError **err);
/* x_menus.c */
GtkWidget *get_main_menu(GSCHEM_TOPLEVEL *w_current);
GtkWidget *get_main_popup(GSCHEM_TOPLEVEL *w_current);
gint do_popup(GSCHEM_TOPLEVEL *w_current, GdkEventButton *event);
void x_menus_sensitivity(GSCHEM_TOPLEVEL *w_current, const char *buf, int flag);
void x_menus_popup_sensitivity(GSCHEM_TOPLEVEL *w_current, const char *buf, int flag);
void x_menu_attach_recent_files_submenu(GSCHEM_TOPLEVEL *w_current);
/* x_multiattrib.c */
void x_multiattrib_open (GSCHEM_TOPLEVEL *w_current);
void x_multiattrib_close (GSCHEM_TOPLEVEL *w_current);
void x_multiattrib_update (GSCHEM_TOPLEVEL *w_current);
/* x_multimulti.c */
/* x_pagesel.c */
void x_pagesel_open (GSCHEM_TOPLEVEL *w_current);
void x_pagesel_close (GSCHEM_TOPLEVEL *w_current);
void x_pagesel_update (GSCHEM_TOPLEVEL *w_current);
/* x_preview.c */
/* x_print.c */
gboolean x_print_export_pdf_page (GSCHEM_TOPLEVEL *w_current, const gchar *filename);
gboolean x_print_export_pdf (GSCHEM_TOPLEVEL *w_current, const gchar *filename);
void x_print (GSCHEM_TOPLEVEL *w_current);
/* x_rc.c */
void x_rc_parse_gschem (GSCHEM_TOPLEVEL *w_current, const gchar *rcfile);
/* x_script.c */
void setup_script_selector(GSCHEM_TOPLEVEL *w_current);
/* x_stroke.c */
void x_stroke_init (void);
void x_stroke_free (void);
void x_stroke_record (GSCHEM_TOPLEVEL *w_current, gint x, gint y);
gint x_stroke_translate_and_execute (GSCHEM_TOPLEVEL *w_current);
/* x_window.c */
void x_window_setup (GSCHEM_TOPLEVEL *w_current);
void x_window_setup_gc(GSCHEM_TOPLEVEL *w_current);
void x_window_free_gc(GSCHEM_TOPLEVEL *w_current);
void x_window_create_drawing(GtkWidget *drawbox, GSCHEM_TOPLEVEL *w_current);
void x_window_setup_draw_events(GSCHEM_TOPLEVEL *w_current);
void x_window_create_main(GSCHEM_TOPLEVEL *w_current);
void x_window_close(GSCHEM_TOPLEVEL *w_current);
void x_window_close_all(GSCHEM_TOPLEVEL *w_current);
PAGE *x_window_open_page (GSCHEM_TOPLEVEL *w_current, const gchar *filename);
void x_window_set_current_page (GSCHEM_TOPLEVEL *w_current, PAGE *page);
gint x_window_save_page (GSCHEM_TOPLEVEL *w_current, PAGE *page, const gchar *filename);
void x_window_close_page (GSCHEM_TOPLEVEL *w_current, PAGE *page);
void x_window_set_default_icon (void);
