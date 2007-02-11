extern void (*arc_draw_func)();
extern void (*box_draw_func)();
extern void (*picture_draw_func)();
extern void (*circle_draw_func)();
extern void (*complex_draw_func)();
extern void (*line_draw_func)();
extern void (*net_draw_func)();
extern void (*bus_draw_func)();
extern void (*text_draw_func)();
extern void (*pin_draw_func)();
extern void (*select_func)();
extern void (*pin_conn_recalc_func)(); 
extern void (*net_conn_recalc_func)();
extern void (*x_log_update_func)();
extern void (*variable_set_func)();
extern char * (*ps_color_string)();
extern int (*image_color_int)();
extern int (*image_gdcolor_init)();
/* load_newer_backup_func is called if an autosave backup file is found
   when loading a new schematic. It should ask the user what to do, and
   return TRUE if the backup file should be loaded, or FALSE otherwise */
extern int (*load_newer_backup_func)();
