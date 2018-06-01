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
// Header for the functions responsible for creating    //
// the graph dialog display elements, including the     //
// dialog itself, as well as the trace widgets.         //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _GRAPH_DIALOG_INTERFACE_H_
#define _GRAPH_DIALOG_INTERFACE_H_

#include <gtk/gtk.h>
#include "graph_dialog_data.h"

// Table index offsets
#define TRACE_TABLE_MIN_X 0
#define TRACE_TABLE_MIN_Y 0
// Initial trace size request
#define TRACE_MIN_CX 300
#define TRACE_MIN_CY 30

typedef struct
  {
  GtkWidget *dialog ;
  GtkWidget *sw ;
  GtkWidget *vp ;
  GtkWidget *hscroll ;
  GtkWidget *table_of_traces ;
  GtkWidget *tview ;
  GtkWidget *hpaned ;
  GtkWidget *lbl_status ;
  } graph_D ;

void create_graph_dialog (graph_D *dialog) ;

gboolean create_graph_widgets (GRAPH_DIALOG_DATA *graph_dialog_data, GtkTreeIter *itr) ;

void attach_graph_widgets (graph_D *dialog, GtkWidget *table, GtkWidget *trace, GtkWidget *ruler, GtkWidget *ui, int idx) ;

#endif /* def _GRAPH_DIALOG_INTERFACE_H_ */
