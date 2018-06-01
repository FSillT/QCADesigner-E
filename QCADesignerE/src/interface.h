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
// Header file for the main UI. This creates the main   //
// QCADesigner window. main() calls                     //
// create_main_winow().                                 //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <gtk/gtk.h>

#define MAIN_WND_BASE_TITLE "QCADesigner"

//!Structure holding main window widgets
typedef struct
  {
  GtkWidget *bus_layout_button;
  GtkWidget *bus_layout_menu_item;
  GtkWidget *clocks_combo_table;
  GtkWidget *default_action_button;
  GtkWidget *drawing_area;
  GtkWidget *drawing_area_frame;
  GtkWidget *horizontal_ruler;
  GtkWidget *insert_cell_array_button;
  GtkWidget *insert_type_1_cell_button;
  GtkWidget *label_button;
  GtkWidget *layers_combo;
  GtkWidget *main_window;
#ifdef STDIO_FILEIO
  GtkWidget *recent_files_menu;
  GtkWidget *save_output_to_file_menu_item;
#endif /* def STDIO_FILEIO */
  GtkWidget *remove_layer_button;
  GtkWidget *rotate_cell_button;
  GtkWidget *show_simulation_results_menu_item;
  GtkWidget *snap_to_grid_menu_item;
  GtkWidget *substrate_button;
  GtkWidget *toggle_alt_display_button;
  GtkWidget *toolbar;
#ifdef UNDO_REDO
  GtkWidget *redo_menu_item;
  GtkWidget *undo_menu_item;
#endif /* def UNDO_REDO */
  GtkWidget *vbox1;
  GtkWidget *vertical_ruler;
  GtkWidget *vpaned1;
  } main_W ;

void create_main_window (main_W *main_window);

#endif /* _INTERFACE_H_ */
