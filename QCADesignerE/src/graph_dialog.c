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
// The graph dialog. This displays the waveforms and    //
// the bus values as present in the raw waveform data.  //
// The bus values are interpreted from the waveform     //
// data and an (upper,lower) threshhold pair chosen by  //
// the user. The graph dialog also allows the user to   //
// load and save simulation data.                       //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include "print.h"
#include "graph_dialog.h"
#include "graph_dialog_data.h"
#include "graph_dialog_interface.h"

static graph_D graph = {NULL} ;

void show_graph_dialog (GtkWindow *parent, SIMULATION_OUTPUT *sim_output, gboolean bOKToFree, gboolean bModal)
  {
  int base = 10 ;
  GRAPH_DIALOG_DATA *gdd = NULL ;
  double dThreshLower = -0.5, dThreshUpper = 0.5 ;

  if (NULL == graph.dialog)
    create_graph_dialog (&graph) ;

  gtk_window_set_transient_for (GTK_WINDOW (graph.dialog), parent) ;

  if (NULL != (gdd = g_object_get_data (G_OBJECT (graph.dialog), "graph_dialog_data")))
    {
    dThreshLower = gdd->dHCThreshLower ;
    dThreshUpper = gdd->dHCThreshUpper ;
    base = gdd->base ;
    }

  g_object_set_data_full (G_OBJECT (graph.dialog), "graph_dialog_data",
    gdd = graph_dialog_data_new (sim_output, bOKToFree, dThreshLower, dThreshUpper, base),
    (GDestroyNotify)graph_dialog_data_free) ;

  apply_graph_dialog_data (&graph, gdd) ;

  gtk_widget_show (graph.dialog) ;

  if (bModal)
    while (GTK_WIDGET_VISIBLE (graph.dialog))
      gtk_main_iteration () ;

  if (NULL != parent)
    gtk_window_present (parent) ;
  }

void apply_graph_dialog_data (graph_D *dialog, GRAPH_DIALOG_DATA *dialog_data)
  {
  GtkWidget *trace_ui_widget = NULL, *trace_drawing_widget = NULL, *trace_ruler_widget = NULL ;
  int row_type ;
  GtkTreeIter itr ;
  int idxTbl = 0 ;
  GtkRequisition rq ;

  gtk_tree_view_set_model (GTK_TREE_VIEW (dialog->tview), dialog_data->model) ;

  gtk_widget_size_request (dialog->tview, &rq) ;

  gtk_paned_set_position (GTK_PANED (dialog->hpaned), rq.width) ;

  gtk_tree_view_expand_all (GTK_TREE_VIEW (dialog->tview)) ;

  if (!gtk_tree_model_get_iter_first (dialog_data->model, &itr)) return ;
  while (TRUE)
    {
    gtk_tree_model_get (dialog_data->model, &itr,
      BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type,
      GRAPH_MODEL_COLUMN_TRACE,     &trace_drawing_widget,
      GRAPH_MODEL_COLUMN_RULER,     &trace_ruler_widget,
      GRAPH_MODEL_COLUMN_UI,        &trace_ui_widget, -1) ;
    attach_graph_widgets (dialog, dialog->table_of_traces, trace_drawing_widget, trace_ruler_widget, trace_ui_widget, idxTbl++) ;
    g_object_set_data (G_OBJECT (trace_drawing_widget), "label", dialog->lbl_status) ;
    g_object_set_data (G_OBJECT (trace_drawing_widget), "table", dialog->table_of_traces) ;
    g_object_set_data (G_OBJECT (trace_drawing_widget), "hscroll", dialog->hscroll) ;
    g_object_set_data (G_OBJECT (trace_ruler_widget), "label", dialog->lbl_status) ;
    if (!gtk_tree_model_iter_next_dfs (dialog_data->model, &itr)) break ;
    }
  dialog_data->icGraphLines = idxTbl ;
  }
