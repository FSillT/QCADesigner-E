//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: qcadesigner@gmail.com                         //
// WEB: http://qcadesigner.ca/                          //
//////////////////////////////////////////////////////////
//******************************************************//
//*********** PLEASE DO NOT REFORMAT THIS CODE *********//
//******************************************************//
// If your editor wraps long lines disable it or don't  //
// save the core files that way. Any independent files  //
// you generate format as you wish.                     //
//////////////////////////////////////////////////////////
// Please use complete names in variables and fucntions //
// This will reduce ramp up time for new people trying  //
// to contribute to the project.                        //
//////////////////////////////////////////////////////////
// Contents:                                            //
//                                                      //
// Header file for callbacks.c, the file containing the //
// callback functions for main window UI elements.      //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_

#include <gtk/gtk.h>

void add_layer_button_clicked (GtkWidget *widget, gpointer data) ;
void remove_layer_button_clicked (GtkWidget *widget, gpointer data) ;
void layer_properties_button_clicked (GtkWidget *widget, gpointer data) ;
void main_window_show (GtkWidget *widget, gpointer data) ;
void on_mirror_button_clicked (GtkButton *button, gpointer user_data) ;
void on_cell_properties_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_show_tb_icons_menu_item_activate (GtkMenuItem * menuitem, gpointer user_data) ;
void on_snap_to_grid_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_show_grid_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_clock_select_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_clock_increment_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_simulation_type_setup_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_simulation_engine_setup_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_random_fault_setup_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_reset_zoom_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_zoom_in_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_zoom_out_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_zoom_die_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_zoom_extents_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_zoom_layer_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data) ;
void on_print_menu_item_activate(GtkMenuItem *menuitem, gpointer user_data);
#ifdef UNDO_REDO
void on_undo_menu_item_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_redo_menu_item_activate(GtkMenuItem *menuitem, gpointer user_data);
#endif/* def UNDO_REDO */
void on_copy_cell_button_clicked (GtkButton *button, gpointer user_data);
void on_delete_menu_item_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_start_simulation_menu_item_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_stop_simulation_menu_item_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_show_simulation_results_menu_item_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_contents_menu_item_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_about_menu_item_activate(GtkMenuItem *menuitem, gpointer user_data);
void file_operations (GtkWidget *widget, gpointer user_data);
#ifdef STDIO_FILEIO
void on_preview_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_load_output_from_file_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_save_output_to_file_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data);
gboolean autosave_timer_event (gpointer data) ;
#endif /* def STDIO_FILEIO */
void rotate_selection_menu_item_activate (GtkWidget *widget, gpointer user_data);
void on_scale_menu_item_activate(GtkMenuItem *menuitem, gpointer user_data);
void layer_selected (GtkWidget *widget, gpointer data) ;
void on_translate_selection_button_clicked (GtkWidget *widget, gpointer user_data) ;
void reorder_layers_button_clicked (GtkWidget *widget, gpointer user_data) ;
void action_button_clicked (GtkWidget *widget, gpointer data) ;
void toggle_alt_display_button_clicked (GtkWidget *widget, gpointer data) ;
void on_clocks_combo_changed (GtkWidget *widget, gpointer data) ;
void bus_layout_button_clicked (GtkWidget *widget, gpointer data) ;
void on_hide_layers_menu_item_activate (GtkWidget *widget, gpointer data) ;
gboolean drawing_area_motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
gboolean drawing_area_button_pressed (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean drawing_area_button_released (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data);
gboolean scroll_event(GtkWidget *widget, GdkEventScroll *event, gpointer user_data) ;
gboolean expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean on_quit_menu_item_activate(GtkWidget *main_window, gpointer user_data);

#endif /* _CALLBACKS_H_*/
