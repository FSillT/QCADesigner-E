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
// Callbacks for the design print properties dialog.    //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _PRINT_PROPERTIES_DIALOG_CALLBACKS_H_
#define _PRINT_PROPERTIES_DIALOG_CALLBACKS_H_

#include <gtk/gtk.h>
#include "design.h"

void on_tbtnPrintOrder_toggled (GtkWidget *widget, gpointer user_data) ;
void on_tbtnCenter_toggled (GtkWidget *widget, gpointer user_data) ;
void toggle_scale_mode (GtkWidget *widget, gpointer user_data) ;
void fill_printed_objects_list (GtkWidget *list, print_properties_D *dialog, DESIGN *design) ;
void validate_value_change (GtkAdjustment *adj_changed, gpointer user_data) ;
void chkPrintedObj_toggled (GtkWidget *widget, gpointer user_data) ;
void user_wants_print_preview (GtkWidget *widget, gpointer user_data) ;
void units_changed (GtkWidget *widget, gpointer user_data) ;

#endif /* _PRINT_PROPERTIES_DIALOG_CALLBACKS_H_*/
