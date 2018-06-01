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
// An alternative interface for specifying vector       //
// tables, which works with the new vector table        //
// library.                                             //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "support.h"
#include "custom_widgets.h"
#include "global_consts.h"
#include "bus_layout_dialog.h"
#include "vector_table_options_dialog.h"
#include "vector_table_options_dialog_data.h"
#include "vector_table_options_dialog_callbacks.h"
#include "vector_table_options_dialog_interface.h"

VectorTable *pvt ;

static vector_table_options_D vto = {NULL} ;

void get_vector_table_options_from_user (GtkWindow *parent, BUS_LAYOUT *bus_layout, int *sim_type, VectorTable *pvt)
  {
  if (NULL == vto.dialog)
    create_vector_table_options_dialog (&vto) ;

  gtk_window_set_transient_for (GTK_WINDOW (vto.dialog), parent) ;

  VectorTableToDialog (&vto, bus_layout, sim_type, pvt) ;

  gtk_widget_show (vto.dialog) ;

  vector_table_options_dialog_btnSimType_clicked (VECTOR_TABLE == (*sim_type) ? vto.tbtnVT : vto.tbtnExhaustive, &vto) ;

  while (GTK_WIDGET_VISIBLE (vto.dialog))
    gtk_main_iteration () ;

  DialogToVectorTable (&vto) ;

  if (NULL != parent)
    gtk_window_present (parent) ;
  }
