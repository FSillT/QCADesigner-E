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
// The callback functions for the vector table options  //
// dialog.                                              //
//                                                      //
//////////////////////////////////////////////////////////

#include <string.h>
#include <stdlib.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "support.h"
#include "generic_utils.h"
#include "global_consts.h"
#include "custom_widgets.h"
#include "vector_table.h"
#include "file_selection_window.h"
#include "bus_layout_dialog.h"
#include "vector_table_options_dialog_data.h"
#include "vector_table_options_dialog_interface.h"
#include "vector_table_options_dialog_callbacks.h"

static void vector_cell_editable_move_cursor (GtkEntry *entry, GtkMovementStep movement_step, gint count, gboolean arg3, gpointer data) ;

static int get_n_active_children (GtkTreeModel *model, GtkTreeIter *itr) ;

void vt_model_active_toggled (GtkCellRenderer *cr, char *pszTreePath, gpointer data)
  {
  VectorTable *pvt = g_object_get_data (G_OBJECT (cr), "pvt") ;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (data)) ;
  GtkTreeIter itr, itrOther ;
  GtkTreePath *tp = NULL ;
  gboolean bActive = TRUE ;
  QCADCell *cell = NULL ;
  int idx ;

  if (NULL == model || NULL == pvt || NULL == (tp = gtk_tree_path_new_from_string (pszTreePath))) return ;
  gtk_tree_model_get_iter (model, &itr, tp) ;

  gtk_tree_model_get (model, &itr, 
    BUS_LAYOUT_MODEL_COLUMN_CELL, &cell,
    VECTOR_TABLE_MODEL_COLUMN_ACTIVE, &bActive, -1) ;

  // Zee beeg fleep
  bActive = !bActive ;

  // Update the vector table input flag
  if (NULL != cell)
    if (-1 != (idx = VectorTable_find_input_idx (pvt, cell)))
      exp_array_index_1d (pvt->inputs, VT_INPUT, idx).active_flag = bActive ;
  gtk_tree_store_set (GTK_TREE_STORE (model), &itr, VECTOR_TABLE_MODEL_COLUMN_ACTIVE, bActive, -1) ;

  // If this node has children (IOW it's a bus node) make all the children be toggled the same way
  // as the bus node, and reflect the state in the vector table
  if (gtk_tree_model_iter_children (model, &itrOther, &itr))
    while (TRUE)
      {
      gtk_tree_model_get (model, &itrOther, BUS_LAYOUT_MODEL_COLUMN_CELL, &cell, -1) ;

      // Update the vector table input flag
      if (NULL != cell)
        if (-1 != (idx = VectorTable_find_input_idx (pvt, cell)))
          exp_array_index_1d (pvt->inputs, VT_INPUT, idx).active_flag = bActive ;
      gtk_tree_store_set (GTK_TREE_STORE (model), &itrOther, VECTOR_TABLE_MODEL_COLUMN_ACTIVE, bActive, -1) ;
      if (!gtk_tree_model_iter_next (model, &itrOther)) break ;
      }
  else
  if (gtk_tree_model_iter_parent (model, &itrOther, &itr))
    {
    gboolean bBusActive = FALSE ;
    gboolean bChildActive = FALSE ;
    GtkTreeIter itrChild ;

    // If all of a bus' children are deselected, so is the bus
    if (gtk_tree_model_iter_children (model, &itrChild, &itrOther))
      {
      while (TRUE)
        {
        gtk_tree_model_get (model, &itrChild, VECTOR_TABLE_MODEL_COLUMN_ACTIVE, &bChildActive, -1) ;
        if ((bBusActive = bChildActive)) break ;
        if (!gtk_tree_model_iter_next (model, &itrChild)) break ;
        }
      gtk_tree_store_set (GTK_TREE_STORE (model), &itrOther, VECTOR_TABLE_MODEL_COLUMN_ACTIVE, bBusActive, -1) ;
      }
    }

  gtk_tree_path_free (tp) ;
  }

void vector_table_options_dialog_btnDelete_clicked (GtkWidget *widget, gpointer data)
  {
  char *psz = NULL ;
  GtkCellRenderer *cr = NULL ;
  int Nix; ;
  GList *llItr = NULL, *llCols = NULL, *llRemove = NULL, *llFirst = NULL, *llClick = NULL ;
  vector_table_options_D *dialog = g_object_get_data (G_OBJECT (data), "dialog") ;
  int idxVector = (int)g_object_get_data (G_OBJECT (data), "idxVector") ;
  VectorTable *pvt = g_object_get_data (G_OBJECT (data), "user_pvt") ;

  if (NULL == dialog || NULL == pvt) return ;

  if (NULL != (llCols = gtk_tree_view_get_columns (GTK_TREE_VIEW (dialog->tv))))
    // The first vector
    if (NULL != (llFirst = g_list_nth (llCols, 2)))
      if (NULL != (llRemove = g_list_nth (llFirst, idxVector)))
        llClick = ((NULL == llRemove->next) ? ((NULL == llRemove->prev) ? NULL : llRemove->prev) : llRemove->next) ;

  if (NULL != llRemove)
    gtk_tree_view_remove_column (GTK_TREE_VIEW (dialog->tv), GTK_TREE_VIEW_COLUMN (llRemove->data)) ;

  for (Nix = 0, llItr = llFirst ; llItr != NULL ; llItr = llItr->next, Nix++)
    if (llItr != llRemove)
      {
      if (NULL != (cr = g_object_get_data (G_OBJECT (llItr->data), "cr")))
        {
        g_object_set_data (G_OBJECT (cr), "idxVector", (gpointer)Nix) ;
        gtk_tree_view_column_set_title (GTK_TREE_VIEW_COLUMN (llItr->data), psz = g_strdup_printf ("%d", Nix)) ;
        g_free (psz) ;
        }
      }
    else
      Nix-- ;

  if (NULL != llClick)
    gtk_tree_view_column_clicked (GTK_TREE_VIEW_COLUMN (llClick->data)) ;

  VectorTable_del_vector (pvt, idxVector) ;

  vector_table_options_dialog_reflect_state (dialog) ;
  }

void vector_table_options_dialog_btnClose_clicked (GtkWidget *widget, gpointer data)
  {
  vector_table_options_D *dialog = g_object_get_data (G_OBJECT (widget), "dialog") ;

  if (NULL == dialog) return ;

  if (NULL != dialog)
    gtk_widget_hide (dialog->dialog) ;
  }

void vector_table_options_dialog_btnSimType_clicked (GtkWidget *widget, gpointer data) 
  {
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  int sim_type = (int)g_object_get_data (G_OBJECT (widget), "sim_type") ;
  int *user_sim_type = NULL ;

  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) return ;
  if (NULL == dialog) return ;

  if (NULL != (user_sim_type = g_object_get_data (G_OBJECT (dialog->dialog), "user_sim_type")))
    (*user_sim_type) = sim_type ;

  vector_table_options_dialog_reflect_state (dialog) ;
  }

void vector_table_options_dialog_btnOpen_clicked (GtkWidget *widget, gpointer data)
  {
  VTL_RESULT vtl_result = VTL_FILE_FAILED ;
  char *pszOldFName = NULL, *psz = NULL ;
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  BUS_LAYOUT *bus_layout = NULL ;
  int *sim_type = NULL ;
  VectorTable *pvt = NULL ;

  if (NULL == dialog) return ;
  if (NULL == (pvt = g_object_get_data (G_OBJECT (dialog->dialog), "user_pvt"))) return ;
  if (NULL == (psz = get_file_name_from_user (GTK_WINDOW (dialog->dialog), _("Open Vector Table"), pvt->pszFName, FALSE))) return ;

  pszOldFName = pvt->pszFName ;
  pvt->pszFName = psz ;

  vtl_result = VectorTable_load (pvt) ;

  if (VTL_FILE_FAILED == vtl_result || VTL_MAGIC_FAILED == vtl_result)
    {
    GtkWidget *msg = NULL ;

    gtk_dialog_run (GTK_DIALOG (msg = 
      gtk_message_dialog_new (GTK_WINDOW (dialog->dialog), 
        GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, 
          (VTL_FILE_FAILED == vtl_result 
            ? _("Failed to open vector table file \"%s\"!")
            : _("File \"%s\" does not appear to be a vector table file !")), psz))) ;

    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;

    g_free (psz) ;
    pvt->pszFName = pszOldFName ;

    return ;
    }
  else
    {
    if (VTL_SHORT == vtl_result || VTL_TRUNC == vtl_result)
      {
      GtkWidget *msg = NULL ;

      gtk_dialog_run (GTK_DIALOG (msg = 
        gtk_message_dialog_new (GTK_WINDOW (dialog->dialog), 
          GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, 
            (VTL_SHORT == vtl_result 
              ? _("File \"%s\" contains fewer inputs than the current vector table. Padding with zeroes.")
              : _("File \"%s\" contains more inputs than the current design. Truncating.")), psz))) ;

      gtk_widget_hide (msg) ;
      gtk_widget_destroy (msg) ;
      }

    if (NULL != (bus_layout = g_object_get_data (G_OBJECT (dialog->dialog), "user_bus_layout")) &&
        NULL != (sim_type = g_object_get_data (G_OBJECT (dialog->dialog), "user_sim_type")))
      VectorTableToDialog (dialog, bus_layout, sim_type, pvt) ;

    g_free (pszOldFName) ;

    vector_table_options_dialog_reflect_state (dialog) ;
    }
  }

void vector_table_options_dialog_btnSave_clicked (GtkWidget *widget, gpointer data)
  {
  char *psz = NULL, *psz1 = NULL ;
  VectorTable *pvt = g_object_get_data (G_OBJECT (data), "user_pvt") ;
  int sim_type = (int)g_object_get_data (G_OBJECT (data), "user_sim_type") ;

  if (EXHAUSTIVE_VERIFICATION == sim_type) return ;

  if (NULL == (psz = get_file_name_from_user (GTK_WINDOW (data), _("Save Vector Table"), pvt->pszFName, TRUE)))
    return ;

  psz1 = pvt->pszFName ;
  pvt->pszFName = psz ;

  if (VectorTable_save (pvt))
    vector_table_options_dialog_reflect_state (g_object_get_data (G_OBJECT (data), "dialog")) ;
  else
    {
    GtkWidget *msg = NULL ;

    gtk_dialog_run (GTK_DIALOG (msg = 
      gtk_message_dialog_new (GTK_WINDOW (data), 
        GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, 
           _("Failed to save vector table file \"%s\"!"), psz))) ;

    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;

    pvt->pszFName = psz1 ;
    g_free (psz) ;
    }
  }

void vector_table_options_dialog_btnAdd_clicked (GtkWidget *widget, gpointer data)
  {
  vector_table_options_D *dialog = g_object_get_data (G_OBJECT (data), "dialog") ;
  VectorTable *pvt = NULL ;
  int idxVector = -1 ;

  if (NULL == dialog) return ;
  if (NULL == (pvt = g_object_get_data (G_OBJECT (dialog->dialog), "user_pvt"))) return ;
  if (dialog->btnInsert == widget)
    idxVector = (int)g_object_get_data (G_OBJECT (data), "idxVector") ;

  VectorTable_add_vector (pvt, idxVector) ;

  add_vector_to_dialog (dialog, pvt, idxVector) ;

  vector_table_options_dialog_reflect_state (dialog) ;
  }

void vector_column_clicked (GtkObject *obj, gpointer data)
  {
  GList *llCols = NULL ;
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  GtkTreeViewColumn *col = NULL ;
  GtkCellRenderer *cr = NULL ;

  if (NULL == dialog) return ;

  col = GTK_TREE_VIEW_COLUMN (GTK_IS_CELL_RENDERER (obj) ? g_object_get_data (G_OBJECT (obj), "col") : obj) ;
  cr = g_object_get_data (G_OBJECT (col), "cr") ;
  g_object_set_data (G_OBJECT (dialog->dialog), "idxVector", g_object_get_data (G_OBJECT (cr), "idxVector")) ;

  if (NULL != (llCols = gtk_tree_view_get_columns (GTK_TREE_VIEW (dialog->tv))))
    {
    GList *llItr = NULL ;
    GtkCellRenderer *cr = NULL ;

    for (llItr = llCols ; llItr != NULL ; llItr = llItr->next)
      if (NULL != (cr = g_object_get_data (G_OBJECT (llItr->data), "cr")))
        g_object_set (G_OBJECT (cr), "cell-background-set", (llItr->data == col), NULL) ;

    g_list_free (llCols) ;

    gtk_widget_queue_draw (dialog->tv) ;
    }
  }

void vector_data_func (GtkTreeViewColumn *col, GtkCellRenderer *cr, GtkTreeModel *model, GtkTreeIter *itr, gpointer data)
  {
  GtkTreeIter itrChild ;
  long long bus_value = 0 ;
  int idxInput = -1 ;
  int idxVector = (int)g_object_get_data (G_OBJECT (cr), "idxVector") ;
  QCADCell *cell = NULL ;
  VectorTable *pvt = (VectorTable *)data ;
  gboolean bActive = FALSE ;

  gtk_tree_model_get (model, itr, 
    BUS_LAYOUT_MODEL_COLUMN_CELL, &cell, -1) ;

  if (NULL != cell)
    if (-1 != (idxInput = VectorTable_find_input_idx (pvt, cell)))
      {
      g_object_set (cr, "active", exp_array_index_2d (pvt->vectors, gboolean, idxVector, idxInput), NULL) ;
      return ;
      }

  if (gtk_tree_model_iter_children (model, &itrChild, itr))
    {
    char *psz = NULL ;
    while (TRUE)
      {
      gtk_tree_model_get (model, &itrChild, 
        BUS_LAYOUT_MODEL_COLUMN_CELL, &cell, 
        BUS_LAYOUT_MODEL_COLUMN_NAME, &psz,
        VECTOR_TABLE_MODEL_COLUMN_ACTIVE, &bActive,
        -1) ;
      if (bActive)
        {
        bus_value = bus_value << 1 ;
        if (-1 != (idxInput = VectorTable_find_input_idx (pvt, cell)))
          bus_value += exp_array_index_2d (pvt->vectors, gboolean, idxVector, idxInput) ? 1 : 0 ;
        }
      if (!gtk_tree_model_iter_next (model, &itrChild)) break ;
      }
    g_object_set (cr, "text", psz = g_strdup_printf ("%llu", bus_value), NULL) ;
    g_free (psz) ;
    return ;
    }
  g_object_set (cr, "text", "", NULL) ;
  }

void tree_view_style_set (GtkWidget *widget, GtkStyle *old_style, gpointer data)
  {
  gboolean bUseBackground = FALSE ;
  GdkColor *clrNew = &((gtk_widget_get_style (widget))->base[3]) ;
  GList *llCols = NULL, *llItr = NULL ;
  GtkCellRenderer *cr = NULL ;

  if (NULL == (llCols = gtk_tree_view_get_columns (GTK_TREE_VIEW (widget)))) return ;

  for (llItr = llCols ; llItr != NULL ; llItr = llItr->next)
    if (NULL != (cr = g_object_get_data (G_OBJECT (llItr->data), "cr")))
      {
      // Why does cell-background-gdk modify cell-background-set ?
      g_object_get (G_OBJECT (cr), "cell-background-set", &bUseBackground, NULL) ;
      g_object_set (G_OBJECT (cr), 
        "cell-background-gdk", clrNew, 
        "cell-background-set", bUseBackground, NULL) ;
      }
  }

void vector_value_editing_started (GtkCellRendererText *cr, GtkCellEditable *editable, char *pszPath, gpointer data)
  {
  if (GTK_IS_ENTRY (editable))
    {
    g_object_set_data_full (G_OBJECT (editable), "pszPath", g_strdup (pszPath), (GDestroyNotify)g_free) ;
    if (!g_signal_handler_find (G_OBJECT (editable), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, vector_cell_editable_move_cursor, data))
      g_signal_connect (G_OBJECT (editable), "move_cursor", (GCallback)vector_cell_editable_move_cursor, data) ;
    }
  }

static void vector_cell_editable_move_cursor (GtkEntry *entry, GtkMovementStep movement_step, gint count, gboolean arg3, gpointer data)
  {
  int position = -1 ;
  const char *pszText = NULL ;
  gboolean bWhichColumn = FALSE ; // FALSE <=> Next column

  if (NULL == (pszText = gtk_entry_get_text (entry))) return ;
  g_object_get (G_OBJECT (entry), "cursor-position", &position, NULL) ;

  if ((bWhichColumn = (0 == position && count < 0)) ||
      (strlen (pszText) == position && count > 0))
    {
    vector_table_options_D *dialog = (vector_table_options_D *)data ;
    GList *llCols = NULL, *llIdx = NULL ;
    char *pszPath = NULL ;
    int idxVector = -1 ;

    if (NULL == dialog) return ;
    if (NULL == (llCols = gtk_tree_view_get_columns (GTK_TREE_VIEW (dialog->tv)))) return ;
    if (NULL != (llIdx = g_list_nth (llCols, (idxVector = (int)g_object_get_data (G_OBJECT (dialog->dialog), "idxVector")) + 2)))
      {
      if (NULL != (pszPath = g_object_get_data (G_OBJECT (entry), "pszPath")))
        {
        GList *llDst = NULL ;

        // if (0 == idxVector) do not grab a frozen column
        if (NULL != (llDst = (bWhichColumn ? (0 == idxVector ? NULL : llIdx->prev) : llIdx->next)))
          {
          GtkTreePath *tp = NULL ;

          if (NULL != (tp = gtk_tree_path_new_from_string (pszPath)))
            {
            GtkCellRenderer *cr = NULL ;

            if (NULL != (cr = g_object_get_data (G_OBJECT (llIdx->data), "cr")))
              {
              gtk_cell_editable_editing_done (GTK_CELL_EDITABLE (entry)) ;
              gtk_cell_editable_remove_widget (GTK_CELL_EDITABLE (entry)) ;
              gtk_bindings_activate (GTK_OBJECT (dialog->tv), bWhichColumn ? GDK_Left : GDK_Right, 0) ;
//              gtk_tree_view_set_cursor (GTK_TREE_VIEW (dialog->tv), tp, GTK_TREE_VIEW_COLUMN (llIdx->data), FALSE) ;
//              GTK_TREE_VIEW_CLASS (g_type_class_peek (GTK_TYPE_TREE_VIEW))->move_cursor (GTK_TREE_VIEW (dialog->tv), GTK_MOVEMENT_VISUAL_POSITIONS, bWhichColumn ? -1 : 1) ;
              while (gtk_events_pending ())
                gtk_main_iteration () ;
              gtk_tree_view_set_cursor (GTK_TREE_VIEW (dialog->tv), tp, GTK_TREE_VIEW_COLUMN (llDst->data), TRUE) ;
              }

            gtk_tree_path_free (tp) ;
            }
          }
        }
      }
    g_list_free (llCols) ;
    }
  }

void vector_value_edited (GtkCellRendererText *cr, char *pszPath, char *pszNewText, gpointer data)
  {
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (data)) ;
  VectorTable *pvt = g_object_get_data (G_OBJECT (cr), "pvt") ;
  int idxVector = (int)g_object_get_data (G_OBJECT (cr), "idxVector") ;
  GtkTreePath *tp = NULL ;
  GtkTreeIter itr, itrChild ;
  int idx = -1, the_shift = 0 ;
  long long new_value = -1 ;
  QCADCell *cell = NULL ;
  GtkTreeViewColumn *col = g_object_get_data (G_OBJECT (cr), "col") ;
  gboolean bActive = FALSE ;

  if (NULL != col)
    gtk_tree_view_column_clicked (col) ;

  if (NULL == (tp = gtk_tree_path_new_from_string (pszPath))) return ;
  gtk_tree_model_get_iter (model, &itr, tp) ;

  new_value = (long long)atoi (pszNewText) ;

  // If this is a bus node, update the children
  if (gtk_tree_model_iter_children (model, &itrChild, &itr))
    {
    long long max_val = (1 << (the_shift = get_n_active_children (model, &itr))) - 1 ;

    new_value = CLAMP (new_value, 0, max_val) ;
    while (TRUE)
      {
      gtk_tree_model_get (model, &itrChild, 
        BUS_LAYOUT_MODEL_COLUMN_CELL, &cell, 
        VECTOR_TABLE_MODEL_COLUMN_ACTIVE, &bActive, -1) ;
      if (NULL != cell && bActive)
        if (-1 != (idx = VectorTable_find_input_idx (pvt, cell)))
          exp_array_index_2d (pvt->vectors, gboolean, idxVector, idx) = (new_value >> (--the_shift)) & 0x1 ;
      if (!gtk_tree_model_iter_next (model, &itrChild)) break ;
      }
    }
  // Otherwise, update the checkbox and the corresponding bus
  else
    {
    if (new_value == 0 || new_value == 1)
      if (!(NULL == model || NULL == pvt))
        {
        gtk_tree_model_get (model, &itr, BUS_LAYOUT_MODEL_COLUMN_CELL, &cell, -1) ;
        if (NULL != cell)
          if (-1 != (idx = VectorTable_find_input_idx (pvt, cell)))
            {
            tree_model_row_changed (model, &itr) ;
            // Zee ozer beeg fleep (1 == value ) would be noop
            exp_array_index_2d (pvt->vectors, gboolean, idxVector, idx) = (0 == new_value) ;
            }
        }
    }

  gtk_tree_path_free (tp) ;  
  }

static int get_n_active_children (GtkTreeModel *model, GtkTreeIter *itr)
  {
  int ret = 0 ;
  GtkTreeIter itrChild ;
  gboolean bActive = FALSE ;

  if (NULL == model || NULL == itr) return 0 ;
  if (!gtk_tree_model_iter_children (model, &itrChild, itr)) return 0 ;

  while (TRUE)
    {
    gtk_tree_model_get (model, &itrChild, VECTOR_TABLE_MODEL_COLUMN_ACTIVE, &bActive, -1) ;
    if (bActive) ret++ ;
    if (!gtk_tree_model_iter_next (model, &itrChild)) break ;
    }
  return ret ;
  }
