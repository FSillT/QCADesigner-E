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
// Applying the data to the vector table dialog and     //
// harvesting the data from the dialog, as well as a    //
// function to set widgets appropriately (in)sensitive. //
//                                                      //
//////////////////////////////////////////////////////////

#include "support.h"
#include "custom_widgets.h"
#include "fileio_helpers.h"
#include "global_consts.h"
#include "bus_layout_dialog.h"
#include "vector_table_options_dialog_data.h"
#include "vector_table_options_dialog_callbacks.h"

void VectorTableToDialog (vector_table_options_D *dialog, BUS_LAYOUT *bus_layout, int *sim_type, VectorTable *pvt)
  {
  GList *llCols = NULL, *llItr = NULL ;
  int Nix ;
  GtkTreeModel *model = NULL ;
  GtkWidget *tbtn = NULL ;
  GtkTreeViewColumn *col = NULL ;

  if (NULL == dialog || NULL == sim_type || NULL == pvt) return ;

  g_object_set_data (G_OBJECT (dialog->dialog), "user_sim_type", sim_type) ;
  g_object_set_data (G_OBJECT (dialog->dialog), "user_pvt", pvt) ;
  g_object_set_data (G_OBJECT (dialog->dialog), "user_bus_layout", bus_layout) ;
  g_object_set_data (G_OBJECT (dialog->dialog), "idxVector", (gpointer)-1) ;

  if (0 == bus_layout->inputs->icUsed)
    (*sim_type) = EXHAUSTIVE_VERIFICATION ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tbtn = (VECTOR_TABLE == (*sim_type) ? dialog->tbtnVT : dialog->tbtnExhaustive)), TRUE) ;

  g_object_set_data (G_OBJECT (dialog->crActive), "pvt", pvt) ;

  if (NULL != (model = GTK_TREE_MODEL (design_bus_layout_tree_store_new (bus_layout, ROW_TYPE_INPUT, 1, G_TYPE_BOOLEAN))))
    {
    gboolean bActive = FALSE ;
    GtkTreeIter itr, itrChild ;
    int row_type = -1, idx = -1 ;
    QCADCell *cell = NULL ;
    gboolean bBusActive = FALSE ;

    // First reflect the active_flag of current cell inputs
    if (gtk_tree_model_get_iter_first (model, &itr))
      while (TRUE)
        {
        gtk_tree_model_get (model, &itr, 
          BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type,
          BUS_LAYOUT_MODEL_COLUMN_CELL, &cell, -1) ;
        if (ROW_TYPE_CELL_INPUT == row_type && NULL != cell)
          if (-1 != (idx = VectorTable_find_input_idx (pvt, cell)))
            gtk_tree_store_set (GTK_TREE_STORE (model), &itr, 
              VECTOR_TABLE_MODEL_COLUMN_ACTIVE, exp_array_index_1d (pvt->inputs, VT_INPUT, idx).active_flag, -1) ;
        if (!gtk_tree_model_iter_next_dfs (model, &itr)) break ;
        }

    // For any given bus, if any of its cells are active, then the bus is active. Reflect this.
    if (gtk_tree_model_get_iter_first (model, &itr))
      while (TRUE)
        {
        bBusActive = FALSE ;
        if (gtk_tree_model_iter_children (model, &itrChild, &itr))
          {
          while (TRUE)
            {
            gtk_tree_model_get (model, &itrChild, VECTOR_TABLE_MODEL_COLUMN_ACTIVE, &bActive, -1) ;
            if ((bBusActive = bActive)) break ;
            if (!gtk_tree_model_iter_next (model, &itrChild)) break ;
            }
          gtk_tree_store_set (GTK_TREE_STORE (model), &itr, VECTOR_TABLE_MODEL_COLUMN_ACTIVE, bBusActive, -1) ;
          }
        if (!gtk_tree_model_iter_next (model, &itr)) break ;
        }

    llCols = gtk_tree_view_get_columns (GTK_TREE_VIEW (dialog->tv)) ;
    for (Nix = 0, llItr = llCols ; Nix < 2 && NULL != llItr ; Nix++, llItr = llItr->next) ;
    for (; llItr != NULL ; llItr = llItr->next)
      gtk_tree_view_remove_column (GTK_TREE_VIEW (dialog->tv), GTK_TREE_VIEW_COLUMN (llItr->data)) ;
    g_list_free (llCols) ;

    gtk_tree_view_set_model (GTK_TREE_VIEW (dialog->tv), model) ;
    }

  gtk_tree_view_expand_all (GTK_TREE_VIEW (dialog->tv)) ;

  for (Nix = 0 ; Nix < pvt->vectors->icUsed ; Nix++)
    {
    if (NULL == col)
      col = add_vector_to_dialog (dialog, pvt, Nix) ;
    else
      add_vector_to_dialog (dialog, pvt, Nix) ;
    // Give the dialog a chance to update itself
    if (0 == Nix % 10)
      while (gtk_events_pending ())
        gtk_main_iteration () ;
    }

  if (NULL != col)
    gtk_tree_view_column_clicked (col) ;

  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (tbtn)))
    vector_table_options_dialog_btnSimType_clicked (tbtn, dialog) ;

  vector_table_options_dialog_reflect_state (dialog) ;
  }

void DialogToVectorTable (vector_table_options_D *dialog)
  {
  int *sim_type = NULL ;
  VectorTable *pvt = NULL ;

  if (NULL == dialog) return ;

  sim_type = g_object_get_data (G_OBJECT (dialog->dialog), "user_sim_type") ;
  pvt = g_object_get_data (G_OBJECT (dialog->dialog), "user_pvt") ;

  if (NULL == sim_type || NULL == pvt) return ;

  (*sim_type) = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->tbtnVT)) ? VECTOR_TABLE : EXHAUSTIVE_VERIFICATION ;
  }

void vector_table_options_dialog_reflect_state (vector_table_options_D *dialog)
  {
  char *pszTitle = NULL ;
  int *sim_type = NULL ;
  VectorTable *pvt = NULL ;
  BUS_LAYOUT *bus_layout = NULL ;

  if (NULL == dialog) return ;
  if (NULL == (pvt = g_object_get_data (G_OBJECT (dialog->dialog), "user_pvt"))) return ;
  if (NULL == (sim_type = g_object_get_data (G_OBJECT (dialog->dialog), "user_sim_type"))) return ;
  if (NULL == (bus_layout = g_object_get_data (G_OBJECT (dialog->dialog), "user_bus_layout"))) return ;

  if (VECTOR_TABLE == (*sim_type) && bus_layout->inputs->icUsed > 0)
    {
    gtk_widget_set_sensitive (dialog->btnSave, TRUE) ;
    gtk_widget_set_sensitive (dialog->btnOpen, TRUE) ;
    gtk_widget_show (dialog->tblVT) ;
    scrolled_window_set_size (GTK_SCROLLED_WINDOW (dialog->sw), dialog->tv, 0.8, 0.8) ;
    gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), TRUE) ;

    gtk_window_set_title (GTK_WINDOW (dialog->dialog), 
      pszTitle = g_strdup_printf ("%s - %s", 
        (NULL == pvt->pszFName ? _("Untitled") : base_name (pvt->pszFName)), _("Vector Table Setup"))) ;
    }
  else // EXHAUSTIVE_VERIFICATION
    {
    gtk_window_set_title (GTK_WINDOW (dialog->dialog),
      pszTitle = g_strdup_printf ("%s - %s", _("Exhaustive Verification"), _("Vector Table Setup"))) ;

    gtk_widget_set_sensitive (dialog->btnOpen, FALSE) ;
    gtk_widget_set_sensitive (dialog->btnSave, FALSE) ;
    gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), FALSE) ;
    gtk_widget_hide (dialog->tblVT) ;
    }
  g_free (pszTitle) ;

  gtk_widget_set_sensitive (dialog->tbtnVT, (bus_layout->inputs->icUsed > 0)) ;

  if (pvt->vectors->icUsed <= 0)
    {
    gtk_widget_set_sensitive (dialog->btnInsert, FALSE) ;
    gtk_widget_set_sensitive (dialog->btnDelete, FALSE) ;
    }
  else
    {
    gtk_widget_set_sensitive (dialog->btnInsert, TRUE) ;
    gtk_widget_set_sensitive (dialog->btnDelete, TRUE) ;
    }
  }
