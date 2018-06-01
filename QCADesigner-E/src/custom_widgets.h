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
// This file was contributed by Gabriel Schulhof        //
// (schulhof@atips.ca).                                 //
//////////////////////////////////////////////////////////
// Contents:                                            //
//                                                      //
// Header file for custom_widgets.c .                   //
// This is a place to store widgets that are too        //
// complex to be maintained within the various user     //
// interface elements and have a high chance of being   //
// reused.                                              //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _CUSTOM_WIDGETS_H_
#define _CUSTOM_WIDGETS_H_

#ifndef GTK_GUI
  #include "gdk_structs.h"
#else
#include <gtk/gtk.h>

#define GTK_WIDGET_SET_VISIBLE(widget,visible) \
  ((visible)?(gtk_widget_show((widget))):(gtk_widget_hide((widget))))

GtkWidget *create_pixmap_button (GtkWidget *pixmap, gchar *text, gboolean bToggle) ;
GtkWidget *create_two_pixmap_toggle_button (GtkWidget *pix1, GtkWidget *pix2, gchar *pszLbl) ;
void pixmap_button_toggle_pixmap (GtkWidget *btn, gboolean bShow) ;
GtkWidget *gtk_button_new_with_stock_image (gchar *pszStock, gchar *pszLabel) ;

typedef enum
  {
  ISB_DIR_UP = 1 << 0,
  ISB_DIR_DN = 1 << 1
  } ISBDirection ;

GtkWidget *gtk_spin_button_new_infinite (GtkAdjustment *adj, gdouble climb_rate, guint digits, ISBDirection direction) ;
GtkWidget *clock_select_combo_new () ;
int clock_select_combo_get_clock (GtkWidget *widget) ;
void clock_select_combo_set_clock (GtkWidget *widget, int clock) ;
void swap_container_contents (GtkWidget *src, GtkWidget *dst) ;
void set_widget_background_colour (GtkWidget *widget, int r, int g, int b) ;
void scrolled_window_set_size (GtkScrolledWindow *sw, GtkWidget *rqWidget, double dcxMaxScreenPercent, double dcyMaxScreenPercent) ;
void get_string_dimensions (char *psz, char *pszFont, int *pcx, int *pcy) ;
void draw_string (GdkDrawable *dst, GdkGC *gc, char *pszFont, int x, int y, char *psz) ;
void gtk_widget_get_root_origin (GtkWidget *widget, int *px, int *py) ;
GtkWidget *command_history_create () ;
GtkWidget *create_labelled_progress_bar () ;
void push_cursor (GtkWidget *widget, GdkCursor *cursor) ;
GdkCursor *pop_cursor (GtkWidget *widget) ;
void set_window_icon (GtkWindow *window, char *pszBaseName) ;

#define NUMBER_OF_RULER_SUBDIVISIONS 3
void set_ruler_scale (GtkRuler *ruler, double dLower, double dUpper) ;

void tree_model_row_changed (GtkTreeModel *model, GtkTreeIter *itr) ;
void gtk_widget_button_press (GtkWidget *widget, int button, int x, int y, GdkModifierType mask) ;
#endif /* def GTK_GUI */
void command_history_message (char *pszFmt, ...) ;
void set_progress_bar_visible (gboolean bVisible) ;
void set_progress_bar_label (char *psz) ;
void set_progress_bar_fraction (double dFraction) ;
GdkColor *clr_idx_to_clr_struct (int clr_idx) ;

#endif /* _CUSTOM_WIDGETS_H_ */
