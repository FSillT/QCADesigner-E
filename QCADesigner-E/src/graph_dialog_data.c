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
// Data for the graph dialog. This structure serves the //
// same purpose as the GtkTreeModel does for a          //
// GtkTreeView. It provides a centralized way for the   //
// graph dialog to store the information it fills       //
// itself out with.                                     //
//                                                      //
//////////////////////////////////////////////////////////

#include "graph_dialog_interface.h"
#include "support.h"
#include "qcadstock.h"
#include "graph_dialog_interface.h"
#include "graph_dialog_data.h"

GRAPH_DIALOG_DATA *graph_dialog_data_new (SIMULATION_OUTPUT *sim_output, gboolean bOKToFree, double dThreshLower, double dThreshUpper, int base)
  {
  GtkTreeStore *ts = NULL ;
  GtkTreeIter itr ;
  GRAPH_DIALOG_DATA *graph_dialog_data = NULL ;

  if (NULL == sim_output) return NULL ;
  if (NULL == sim_output->sim_data || NULL == sim_output->bus_layout) return NULL ;

  graph_dialog_data = g_malloc0 (sizeof (GRAPH_DIALOG_DATA)) ;

  graph_dialog_data->sim_data        = sim_output->sim_data ;
  graph_dialog_data->bus_layout      = sim_output->bus_layout ;
  graph_dialog_data->bFakeCells      = sim_output->bFakeIOLists ;
  graph_dialog_data->bFreeSourceData = bOKToFree ;
  graph_dialog_data->model           =
    GTK_TREE_MODEL (ts = design_bus_layout_tree_store_new (sim_output->bus_layout, ROW_TYPE_ANY,
      5, G_TYPE_BOOLEAN, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_POINTER)) ;
  graph_dialog_data->dHCThreshLower  = dThreshLower ;
  graph_dialog_data->dHCThreshUpper  = dThreshUpper ;
  graph_dialog_data->icDrawingArea   =
  graph_dialog_data->icUIWidget      =
  graph_dialog_data->cyDrawingArea   =
  graph_dialog_data->cxUIWidget      =
  graph_dialog_data->cyUIWidget      =
  graph_dialog_data->icGraphLines    =  0 ;
  graph_dialog_data->bOneTime        = TRUE ;
  graph_dialog_data->base            = base ;
  graph_dialog_data->dScale          = 1.0 ;

  gtk_tree_store_append (GTK_TREE_STORE (graph_dialog_data->model), &itr, NULL) ;
  gtk_tree_store_set (GTK_TREE_STORE (graph_dialog_data->model), &itr,
    BUS_LAYOUT_MODEL_COLUMN_ICON, QCAD_STOCK_CLOCK,
    BUS_LAYOUT_MODEL_COLUMN_NAME, _("Clock 0"),
    BUS_LAYOUT_MODEL_COLUMN_TYPE, ROW_TYPE_CLOCK,
    BUS_LAYOUT_MODEL_COLUMN_INDEX, 0, -1) ;

  gtk_tree_store_append (GTK_TREE_STORE (graph_dialog_data->model), &itr, NULL) ;
  gtk_tree_store_set (GTK_TREE_STORE (graph_dialog_data->model), &itr,
    BUS_LAYOUT_MODEL_COLUMN_ICON, QCAD_STOCK_CLOCK,
    BUS_LAYOUT_MODEL_COLUMN_NAME, _("Clock 1"),
    BUS_LAYOUT_MODEL_COLUMN_TYPE, ROW_TYPE_CLOCK,
    BUS_LAYOUT_MODEL_COLUMN_INDEX, 1, -1) ;

  gtk_tree_store_append (GTK_TREE_STORE (graph_dialog_data->model), &itr, NULL) ;
  gtk_tree_store_set (GTK_TREE_STORE (graph_dialog_data->model), &itr,
    BUS_LAYOUT_MODEL_COLUMN_ICON, QCAD_STOCK_CLOCK,
    BUS_LAYOUT_MODEL_COLUMN_NAME, _("Clock 2"),
    BUS_LAYOUT_MODEL_COLUMN_TYPE, ROW_TYPE_CLOCK,
    BUS_LAYOUT_MODEL_COLUMN_INDEX, 2, -1) ;

  gtk_tree_store_append (GTK_TREE_STORE (graph_dialog_data->model), &itr, NULL) ;
  gtk_tree_store_set (GTK_TREE_STORE (graph_dialog_data->model), &itr,
    BUS_LAYOUT_MODEL_COLUMN_ICON, QCAD_STOCK_CLOCK,
    BUS_LAYOUT_MODEL_COLUMN_NAME, _("Clock 3"),
    BUS_LAYOUT_MODEL_COLUMN_TYPE, ROW_TYPE_CLOCK,
    BUS_LAYOUT_MODEL_COLUMN_INDEX, 3, -1) ;

  if (!gtk_tree_model_get_iter_first (graph_dialog_data->model, &itr))
    {
    g_object_unref (graph_dialog_data->model) ;
    g_free (graph_dialog_data) ;
    return NULL ;
    }

  // Add the widgets corresponding to the model lines
  while (create_graph_widgets (graph_dialog_data, &itr)) ;

  return graph_dialog_data ;
  }

void graph_dialog_data_free (GRAPH_DIALOG_DATA *gdd)
  {
  int Nix ;
  GtkTreeIter itr ;
  GtkWidget *trace = NULL, *ruler = NULL, *ui = NULL ;

  // Destroy the widgets
  if (gtk_tree_model_get_iter_first (gdd->model, &itr))
    {
    while (TRUE)
      {
      gtk_tree_model_get (gdd->model, &itr,
        GRAPH_MODEL_COLUMN_TRACE, &trace,
        GRAPH_MODEL_COLUMN_RULER, &ruler,
        GRAPH_MODEL_COLUMN_UI, &ui, -1) ;

      gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (trace)), trace) ;
      gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (ruler)), ruler) ;
      gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (ui)), ui) ;

      if (!gtk_tree_model_iter_next_dfs (gdd->model, &itr)) break ;
      }
    }

  // if the cells are fake, destroy them
  if (gdd->bFakeCells)
    {
    for (Nix = 0 ; Nix < gdd->bus_layout->inputs->icUsed ; Nix++)
      g_object_unref (exp_array_index_1d (gdd->bus_layout->inputs, BUS_LAYOUT_CELL, Nix).cell) ;
    for (Nix = 0 ; Nix < gdd->bus_layout->outputs->icUsed ; Nix++)
      g_object_unref (exp_array_index_1d (gdd->bus_layout->outputs, BUS_LAYOUT_CELL, Nix).cell) ;
    design_bus_layout_free (gdd->bus_layout) ;
    gdd->bus_layout = NULL ;
    }

  // If it's OK to free the data, free it.
  if (gdd->bFreeSourceData)
    {
    if (NULL != gdd->bus_layout)
      design_bus_layout_free (gdd->bus_layout) ;
    simulation_data_destroy (gdd->sim_data) ;
    }

  // Destroy the model
  g_object_unref (gdd->model) ;

  // Free the structure itself
  g_free (gdd) ;
  }
