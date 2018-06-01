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
// Bus layout dialog. This allows the user to group     //
// individual inputs into input buses, and individual   //
// outputs into output buses, respectively. The user's  //
// choices are encoded in a BUS_LAYOUT structure, which //
// is part of the DESIGN structure.                     //
//                                                      //
//////////////////////////////////////////////////////////

#include <string.h>
#include <stdarg.h>
#include <gtk/gtk.h>
#include "custom_widgets.h"
#include "support.h"
#include "qcadstock.h"
#include "design.h"
#include "bus_layout_dialog.h"

#define GTK_TREE_SELECTION_SET_PATH_SELECTED(sel,tp,bSel) \
 (bSel ? gtk_tree_selection_select_path ((sel),(tp)) : \
         gtk_tree_selection_unselect_path ((sel),(tp)))

typedef struct
  {
  GtkWidget *dialog ;
  GtkWidget *sw ;
  GtkWidget *tview ;
  GtkWidget *bbox ;
  GtkWidget *btnOK ;
  GtkWidget *btnCreateBus ;
  GtkWidget *btnDeleteBus ;
  GtkWidget *btnMoveCellsUp ;
  GtkWidget *btnMoveCellsDown ;
  GtkWidget *btnMoveBusUp ;
  GtkWidget *btnMoveBusDown ;
  GtkWidget *lblBusName ;
  GtkWidget *txtBusName ;
  GtkWidget *lblWarning ;
  } bus_layout_D ;

typedef struct
  {
  GtkTreeIter itr ;
  int icChildren ;
  GtkTreePath *tp ;
  gboolean bSel ;
  } SWAP_BUSES_STRUCT ;

typedef struct
  {
  GtkTreeSelection *sel ;
  int row_type ;
  } DESELECT_DIFFERENT_CELLS_STRUCT ;

// DFA States
enum
  {
  GSBS_START,
  GSBS_D1NB,
  GSBS_D1B_ACCEPT,
  GSBS_REJECT
  } ;

typedef struct
  {
  GtkTreePath *tpBus ;
  int state ;
  } GET_SELECTION_BUS_STRUCT ;

static bus_layout_D bus_layout_dialog = {NULL} ;

static void create_bus_layout_dialog (bus_layout_D *dialog) ;
static void BusLayoutToDialog (bus_layout_D *dialog, BUS_LAYOUT *bus_layout) ;
static void DialogToBusLayout (bus_layout_D *dialog, BUS_LAYOUT *bus_layout) ;
static gboolean select_cell_row_p (GtkTreeSelection *sel, GtkTreeModel *tm, GtkTreePath *tp, gboolean bSelStatus, gpointer data) ;
static void tree_view_selection_changed (GtkTreeSelection *sel, gpointer data) ;
static void deselect_other_type_of_cells (GtkTreeModel *tm, GtkTreePath *tp, GtkTreeIter *itr, gpointer data) ;
static GtkTreePath *get_selection_bus (GtkTreeSelection *sel, gboolean *pbSomethingSelected) ;
static void get_selection_bus_foreach_func (GtkTreeModel *tm, GtkTreePath *tp, GtkTreeIter *itr, gpointer data) ;
static gboolean whole_bus_selected_p (GtkTreeSelection *sel, GtkTreeModel *tm, GtkTreePath *tpSelBus) ;
static void create_bus_button_clicked (GtkWidget *widget, gpointer data) ;
static void delete_bus_button_clicked (GtkWidget *widget, gpointer data) ;
static GList *get_selected_refs (GtkTreeSelection *sel) ;
static void get_selected_refs_foreach_func (GtkTreeModel *tm, GtkTreePath *tp, GtkTreeIter *itr, gpointer data) ;
static void bus_name_changed (GtkWidget *widget, gpointer data) ;
static GList *get_bus_refs (GtkTreeModel *tm, GtkTreePath *tpBus) ;
static void raise_bus_cell_position (GtkWidget *widget, gpointer data) ;
static void lower_bus_cell_position (GtkWidget *widget, gpointer data) ;
static void raise_bus_position (GtkWidget *widget, gpointer data) ;
static void lower_bus_position (GtkWidget *widget, gpointer data) ;
static inline int gtk_tree_model_path_n_children (GtkTreeModel *tm, GtkTreePath *tp) ;
static void swap_model_paths_contents (GtkTreeModel *tm, GtkTreePath *tpSrc, GtkTreePath *tpDst) ;
static void swap_model_iters_contents (GtkTreeModel *tm, GtkTreeIter *itrSrc, GtkTreeIter *itrDst) ;
static void determine_first_or_last_bus (GtkTreeModel *tm, GtkTreePath *tpSelBus, gboolean *pbFirstBusSelected, gboolean *pbLastBusSelected) ;
static GtkTreePath *get_next_bus (GtkTreeModel *tm, GtkTreePath *tpSelBus) ;
static GtkTreePath *get_prev_bus (GtkTreeModel *tm, GtkTreePath *tpSelBus) ;
static void swap_buses (GtkTreeModel *tm, GtkTreeSelection *sel, bus_layout_D *dialog, GtkTreePath *tpSrc, GtkTreePath *tpDst) ;
static void bus_layout_tree_model_dump_priv (GtkTreeModel *model, FILE *pfile, GtkTreeIter *itr, int icIndent) ;

void get_bus_layout_from_user (GtkWindow *parent, BUS_LAYOUT *bus_layout)
  {
  GtkTreePath *tp = NULL ;

  if (NULL == bus_layout_dialog.dialog)
    create_bus_layout_dialog (&bus_layout_dialog) ;

  gtk_window_set_transient_for (GTK_WINDOW (bus_layout_dialog.dialog), parent) ;

  BusLayoutToDialog (&bus_layout_dialog, bus_layout) ;

  if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (bus_layout_dialog.dialog)))
    DialogToBusLayout (&bus_layout_dialog, bus_layout) ;
  else
    {
    gtk_widget_hide (bus_layout_dialog.lblWarning) ;
    gtk_widget_set_sensitive (bus_layout_dialog.bbox, TRUE) ;
    gtk_widget_set_sensitive (bus_layout_dialog.btnOK, TRUE) ;
    gtk_widget_set_sensitive (bus_layout_dialog.sw, TRUE) ;
    }

  if (NULL != (tp = g_object_get_data (G_OBJECT (bus_layout_dialog.tview), "tpBus")))
    gtk_tree_path_free (tp) ;
  g_object_set_data (G_OBJECT (bus_layout_dialog.tview), "tpBus", NULL) ;
  gtk_widget_hide (bus_layout_dialog.dialog) ;
  }

// start init functions

static void BusLayoutToDialog (bus_layout_D *dialog, BUS_LAYOUT *bus_layout)
  {
  GtkTreeStore *ts = NULL ;

  ts = design_bus_layout_tree_store_new (bus_layout, ROW_TYPE_ANY, 0) ;

  gtk_widget_set_sensitive (dialog->btnCreateBus, FALSE) ;
  gtk_widget_set_sensitive (dialog->btnDeleteBus, FALSE) ;
  gtk_widget_set_sensitive (dialog->btnMoveBusUp, FALSE) ;
  gtk_widget_set_sensitive (dialog->btnMoveBusDown, FALSE) ;
  gtk_widget_set_sensitive (dialog->btnMoveCellsUp, FALSE) ;
  gtk_widget_set_sensitive (dialog->btnMoveCellsDown, FALSE) ;
  gtk_widget_set_sensitive (dialog->lblBusName, FALSE) ;
  gtk_widget_set_sensitive (dialog->txtBusName, FALSE) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->txtBusName), "") ;

  gtk_tree_view_set_model (GTK_TREE_VIEW (dialog->tview), GTK_TREE_MODEL (ts)) ;

  // Unref the tree store after giving it to the TreeView.  This way, the next time we give
  // a tree store to the tree view, the current tree store will be destroyed
  g_object_unref (ts) ;

  scrolled_window_set_size (GTK_SCROLLED_WINDOW (dialog->sw), dialog->tview, 0.8, 0.4) ;
  }

static void DialogToBusLayout (bus_layout_D *dialog, BUS_LAYOUT *bus_layout)
  {
  GtkTreeModel *tm = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tview)) ;
  int Nix ;
  int icTopLevel = gtk_tree_model_iter_n_children (tm, NULL), icCells = -1 ;
  GtkTreePath *tp = NULL, *tpCells = NULL ;
  GtkTreeIter itr ;
  int Nix1, idxCell = -1 ;
  int row_type ;
  EXP_ARRAY *cell_list = NULL ;
  BUS *bus ;

  // destroy all buses before adding the new ones
  for (Nix = bus_layout->buses->icUsed - 1 ; Nix > -1 ; Nix--)
    {
    exp_array_free (exp_array_index_1d (bus_layout->buses, BUS, Nix).cell_indices) ;
    g_free (exp_array_index_1d (bus_layout->buses, BUS, Nix).pszName) ;
    }
  exp_array_remove_vals (bus_layout->buses, 1, 0, bus_layout->buses->icUsed) ;

  // Since we've destroyed all buses, no cells are members of any bus any longer
  for (Nix = 0 ; Nix < bus_layout->inputs->icUsed ; Nix++)
    exp_array_index_1d (bus_layout->inputs, BUS_LAYOUT_CELL, Nix).bIsInBus = FALSE ;
  for (Nix = 0 ; Nix < bus_layout->outputs->icUsed ; Nix++)
    exp_array_index_1d (bus_layout->outputs, BUS_LAYOUT_CELL, Nix).bIsInBus = FALSE ;

  if (icTopLevel > 0)
    {
    tp = gtk_tree_path_new_first () ;
    for (Nix = 0 ; Nix < icTopLevel ; Nix++, gtk_tree_path_next (tp))
      {
      gtk_tree_model_get_iter (tm, &itr, tp) ;
      gtk_tree_model_get (tm, &itr, BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type, -1) ;
      if (ROW_TYPE_BUS & row_type)
        {
        exp_array_insert_vals (bus_layout->buses, NULL, 1, 1, -1) ;
        bus = &(exp_array_index_1d (bus_layout->buses, BUS, bus_layout->buses->icUsed - 1)) ;
        gtk_tree_model_get (tm, &itr, BUS_LAYOUT_MODEL_COLUMN_NAME, &(bus->pszName), -1) ;
	if (ROW_TYPE_INPUT & row_type)
	  {
	  bus->bus_function = QCAD_CELL_INPUT ;
	  cell_list = bus_layout->inputs ;
	  }
	else
	  {
	  bus->bus_function = QCAD_CELL_OUTPUT ;
	  cell_list = bus_layout->outputs ;
	  }
        bus->cell_indices = exp_array_new (sizeof (int), 1) ;
        if ((icCells = gtk_tree_model_iter_n_children (tm, &itr)) > 0)
          {
          gtk_tree_path_down (tpCells = gtk_tree_path_copy (tp)) ;

          for (Nix1 = 0 ; Nix1 < icCells ; Nix1++, gtk_tree_path_next (tpCells))
            {
            gtk_tree_model_get_iter (tm, &itr, tpCells) ;
            gtk_tree_model_get (tm, &itr, BUS_LAYOUT_MODEL_COLUMN_INDEX, &idxCell, -1) ;
            exp_array_insert_vals (bus->cell_indices, &idxCell, 1, 1, -1) ;
            // Flag this cell in the master cell list as a member of some bus
            exp_array_index_1d (cell_list, BUS_LAYOUT_CELL, idxCell).bIsInBus = TRUE ;
            }
          gtk_tree_path_free (tpCells) ;
          }
        }
      }
    gtk_tree_path_free (tp) ;
    }
  }

// start callbacks

static void lower_bus_cell_position (GtkWidget *widget, gpointer data)
  {
  bus_layout_D *dialog = (bus_layout_D *)data ;
  GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->tview)) ;
  GtkTreeModel *tm = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tview)) ;
  GtkTreePath *tpBus = g_object_get_data (G_OBJECT (dialog->tview), "tpBus") ;
  GtkTreePath *tpSrc = NULL, *tpDst = NULL ;
  int icChildren = -1, Nix ;

  if (NULL == tpBus)
		{
		fprintf (stderr, "lower_bus_cell_position: tpBus == NULL !\n") ;
		return ;
		}
  if (0 == (icChildren = gtk_tree_model_path_n_children (tm, tpBus)))
		{
		fprintf (stderr, "lower_bus_cell_position: tpBus has no children O_o\n") ;
		return ;
		}

  gtk_tree_path_down (tpDst = gtk_tree_path_copy (tpBus)) ;
  for (Nix = 1 ; Nix < icChildren ; Nix++) gtk_tree_path_next (tpDst) ;
  gtk_tree_path_prev (tpSrc = gtk_tree_path_copy (tpDst)) ;

  for (Nix = 1 ; Nix < icChildren ; Nix++)
    {
    if (gtk_tree_selection_path_is_selected (sel, tpSrc))
      {
      // swap the 2 rows
      swap_model_paths_contents (tm, tpSrc, tpDst) ;
      gtk_tree_selection_unselect_path (sel, tpSrc) ;
      gtk_tree_selection_select_path (sel, tpDst) ;
      }
    gtk_tree_path_prev (tpSrc) ;
    gtk_tree_path_prev (tpDst) ;
    }
  }

static void raise_bus_position (GtkWidget *widget, gpointer data)
  {
  // Make sure to maintain the correct items selected
  bus_layout_D *dialog = (bus_layout_D *)data ;
  GtkTreeModel *tm = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tview)) ;
  GtkTreePath *tpBus = g_object_get_data (G_OBJECT (dialog->tview), "tpBus") ;
  GtkTreePath *tpPrev = get_prev_bus (tm, tpBus) ;
  GtkTreeSelection *sel = NULL ;

  if (NULL == tpPrev || NULL == tpBus || NULL == tm) return ;

  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->tview)) ;

  swap_buses (tm, sel, dialog, tpBus, tpPrev) ;
  }

static void lower_bus_position (GtkWidget *widget, gpointer data)
  {
  // Make sure to maintain the correct items selected
  bus_layout_D *dialog = (bus_layout_D *)data ;
  GtkTreeModel *tm = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tview)) ;
  GtkTreePath *tpBus = g_object_get_data (G_OBJECT (dialog->tview), "tpBus") ;
  GtkTreePath *tpNext = get_next_bus (tm, tpBus) ;
  GtkTreeSelection *sel = NULL ;

  if (NULL == tpNext || NULL == tpBus || NULL == tm) return ;

  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->tview)) ;

  swap_buses (tm, sel, dialog, tpBus, tpNext) ;
  }

static void raise_bus_cell_position (GtkWidget *widget, gpointer data)
  {
  bus_layout_D *dialog = (bus_layout_D *)data ;
  GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->tview)) ;
  GtkTreeModel *tm = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tview)) ;
  GtkTreePath *tpBus = g_object_get_data (G_OBJECT (dialog->tview), "tpBus") ;
  GtkTreePath *tpSrc = NULL, *tpDst = NULL ;
  int icChildren = -1, Nix ;

  if (NULL == tpBus) return ;
  if (0 == (icChildren = gtk_tree_model_path_n_children (tm, tpBus))) return ;

  gtk_tree_path_down (tpDst = gtk_tree_path_copy (tpBus)) ;
  gtk_tree_path_next (tpSrc = gtk_tree_path_copy (tpDst)) ;

  for (Nix = 1 ; Nix < icChildren ; Nix++)
    {
    if (gtk_tree_selection_path_is_selected (sel, tpSrc))
      {
      // swap the 2 rows
      swap_model_paths_contents (tm, tpSrc, tpDst) ;
      gtk_tree_selection_unselect_path (sel, tpSrc) ;
      gtk_tree_selection_select_path (sel, tpDst) ;
      }
    gtk_tree_path_next (tpSrc) ;
    gtk_tree_path_next (tpDst) ;
    }
  }

static void bus_name_changed (GtkWidget *widget, gpointer data)
  {
  bus_layout_D *dialog = (bus_layout_D *)data ;
  GtkTreeIter itr ;
  GtkTreePath *tp = g_object_get_data (G_OBJECT (dialog->tview), "tpBus") ;
  GtkTreeModel *tm = NULL ;
  const char *pszText = NULL ;
  gboolean bHaveText = FALSE ;

  if (NULL == tp) return ;

  gtk_tree_model_get_iter (tm = GTK_TREE_MODEL (gtk_tree_view_get_model (GTK_TREE_VIEW (((bus_layout_D *)data)->tview))), &itr, tp) ;

  gtk_tree_store_set (GTK_TREE_STORE (tm), &itr,
    BUS_LAYOUT_MODEL_COLUMN_NAME, pszText = gtk_entry_get_text (GTK_ENTRY (widget)), -1) ;

  if ((bHaveText = ('\0' != pszText[0])))
    gtk_widget_hide (dialog->lblWarning) ;
  else
    gtk_widget_show (dialog->lblWarning) ;

  gtk_widget_set_sensitive (dialog->sw, bHaveText) ;
  gtk_widget_set_sensitive (dialog->bbox, bHaveText) ;
  gtk_widget_set_sensitive (dialog->btnOK, bHaveText) ;
  }

// Create a new bus from the current selection
static void create_bus_button_clicked (GtkWidget *widget, gpointer data)
  {
  int row_type = -1 ;
  bus_layout_D *dialog = (bus_layout_D *)data ;
  GList *llTreeRefs = NULL, *llItr = NULL ;
  GtkTreeStore *ts = GTK_TREE_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tview))) ;
  GtkTreeIter itrBus, itr, itrSrc, itrSrcParent ;
  GtkTreeRowReference *refBus = NULL, *refSrcParent = NULL ;
  GtkTreePath *tp = NULL, *tpSrcParent = NULL ;

  llTreeRefs = get_selected_refs (gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->tview))) ;

  if (NULL == llTreeRefs) return ;

  gtk_tree_model_get_iter (GTK_TREE_MODEL (ts), &itr,
    tp = gtk_tree_row_reference_get_path (llTreeRefs->data)) ;

  gtk_tree_model_get (GTK_TREE_MODEL (ts), &itr, BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type, -1) ;

  gtk_tree_path_free (tp) ;

  gtk_tree_store_prepend (ts, &itrBus, NULL) ;
  gtk_tree_store_set (ts, &itrBus,
    BUS_LAYOUT_MODEL_COLUMN_ICON, (row_type & ROW_TYPE_INPUT) ? QCAD_STOCK_BUS_INPUT : QCAD_STOCK_BUS_OUTPUT,
    BUS_LAYOUT_MODEL_COLUMN_NAME, _("Untitled Bus"),
    BUS_LAYOUT_MODEL_COLUMN_TYPE, (row_type & ROW_TYPE_INPUT) ? ROW_TYPE_BUS_INPUT : ROW_TYPE_BUS_OUTPUT,
    BUS_LAYOUT_MODEL_COLUMN_INDEX, -1, -1) ;

  refBus = gtk_tree_row_reference_new (GTK_TREE_MODEL (ts), tp = gtk_tree_model_get_path (GTK_TREE_MODEL (ts), &itrBus)) ;
  gtk_tree_path_free (tp) ;

  for (llItr = g_list_last (llTreeRefs) ; llItr != NULL ; llItr = llItr->prev)
    {
    gtk_tree_model_get_iter (GTK_TREE_MODEL (ts), &itrBus, tp = gtk_tree_row_reference_get_path (refBus)) ;
    gtk_tree_path_free (tp) ;

    gtk_tree_store_append (ts, &itr, &itrBus) ;

    gtk_tree_model_get_iter (GTK_TREE_MODEL (ts), &itrSrc,
      tp = gtk_tree_row_reference_get_path (llItr->data)) ;
    gtk_tree_path_free (tp) ;

    swap_model_iters_contents (GTK_TREE_MODEL (ts), &itrSrc, &itr) ;

    gtk_tree_model_get_iter (GTK_TREE_MODEL (ts), &itrSrc,
      tp = gtk_tree_row_reference_get_path (llItr->data)) ;

    if (gtk_tree_path_get_depth (tp) > 1)
      {
      gtk_tree_path_up (tpSrcParent = gtk_tree_path_copy (tp)) ;

      refSrcParent = (1 == gtk_tree_model_path_n_children (GTK_TREE_MODEL (ts), tpSrcParent)) ?
        gtk_tree_row_reference_new (GTK_TREE_MODEL (ts), tpSrcParent) : NULL ;

      gtk_tree_path_free (tpSrcParent) ;
      }

    gtk_tree_path_free (tp) ;

    gtk_tree_row_reference_free (llItr->data) ;

    // Remove cell from old location
    gtk_tree_store_remove (ts, &itrSrc) ;

    // The bus that owned the row we just moved has become empty - delete it
    if (NULL != refSrcParent)
      {
      tpSrcParent = gtk_tree_row_reference_get_path (refSrcParent) ;

      gtk_tree_model_get_iter (GTK_TREE_MODEL (ts), &itrSrcParent, tpSrcParent) ;

      gtk_tree_store_remove (ts, &itrSrcParent) ;

      gtk_tree_path_free (tpSrcParent) ;
      gtk_tree_row_reference_free (refSrcParent) ;
      refSrcParent = NULL ;
      }
    }

  gtk_tree_selection_select_path (gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->tview)), tp = gtk_tree_row_reference_get_path (refBus)) ;

  gtk_tree_path_free (tp) ;
  gtk_tree_row_reference_free (refBus) ;
  g_list_free (llTreeRefs) ;
  }

static void delete_bus_button_clicked (GtkWidget *widget, gpointer data)
  {
  int icChildren = -1 ;
  bus_layout_D *dialog = (bus_layout_D *)data ;
  GtkTreeModel *tm = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tview)) ;
  GtkTreePath *tpBus = g_object_get_data (G_OBJECT (dialog->tview), "tpBus") ;
  GtkTreeRowReference *trrBus = NULL ;
  GtkTreeIter itr, itrSrc ;
  GList *llBusRefs = NULL, *llItr = NULL ;
  GtkTreePath *tp = NULL ;

  if (NULL == tpBus) return ;

  icChildren = gtk_tree_model_path_n_children (tm, tpBus) ;

  // There should always be some children
  if (0 == icChildren) return ;

  trrBus = gtk_tree_row_reference_new (tm, tpBus) ;

  llBusRefs = get_bus_refs (tm, tpBus) ;

  for (llItr = llBusRefs ; llItr != NULL ; llItr = llItr->next)
    {
    gtk_tree_store_append (GTK_TREE_STORE (tm), &itr, NULL) ;

    gtk_tree_model_get_iter (tm, &itrSrc, tp = gtk_tree_row_reference_get_path (llItr->data)) ;
    gtk_tree_path_free (tp) ;

    swap_model_iters_contents (tm, &itrSrc, &itr) ;

    gtk_tree_row_reference_free (llItr->data) ;
    }

  g_list_free (llBusRefs) ;

  gtk_tree_model_get_iter (tm, &itrSrc, tp = gtk_tree_row_reference_get_path (trrBus)) ;
  gtk_tree_path_free (tp) ;

  gtk_tree_store_remove (GTK_TREE_STORE (tm), &itrSrc) ;

  gtk_tree_row_reference_free (trrBus) ;
  }

// Implements the selection constraints for the available cells tree view
static gboolean select_cell_row_p (GtkTreeSelection *sel, GtkTreeModel *tm, GtkTreePath *tp, gboolean bSelStatus, gpointer data)
  {
  int Nix ;
  static gboolean bIgnore = FALSE ;
  int this_row_type = -1 ;
  GtkTreeIter itr ;

  if (bIgnore) return TRUE ;

  gtk_tree_model_get_iter (tm, &itr, tp) ;

  gtk_tree_model_get (tm, &itr, BUS_LAYOUT_MODEL_COLUMN_TYPE, &this_row_type, -1) ;

  if (this_row_type & ROW_TYPE_BUS)
    {
    GtkTreePath *tpNew = NULL ;
    int icChildren = gtk_tree_model_iter_n_children (tm, &itr) ;
    int x, y ;
    GdkModifierType mask ;

    if (bSelStatus)
      {
      bIgnore = TRUE ;
      gtk_tree_selection_unselect_path (sel, tp) ;
      bIgnore = FALSE ;
      }

    gtk_tree_view_expand_row (gtk_tree_selection_get_tree_view (sel), tp, TRUE) ;

    if (icChildren > 0)
      {
      // I should be able to programmatically simulate a "click" event, because its meaning depends
      // on whether Ctrl or Shift are held down.  I should not have to reproduce that behaviour here,
      // because it may change in the future. I need to simulate a "click" event because I would like to
      // implement the following: Clicking on a bus is equivalent to clicking on all its inputs
      gdk_window_get_pointer (GTK_WIDGET (gtk_tree_selection_get_tree_view (sel))->window, &x, &y, &mask) ;

      gtk_tree_path_down (tpNew = gtk_tree_path_copy (tp)) ;

      bSelStatus = gtk_tree_selection_path_is_selected (sel, tpNew) ;

      for (Nix = 0 ; Nix < icChildren ; Nix++, gtk_tree_path_next (tpNew))
        if ((mask & GDK_CONTROL_MASK))
          {
          if (bSelStatus)
            gtk_tree_selection_unselect_path (sel, tpNew) ;
          else
            gtk_tree_selection_select_path (sel, tpNew) ;
          }
        else
          gtk_tree_selection_select_path (sel, tpNew) ;

      gtk_tree_path_free (tpNew) ;
      }
    return FALSE ;
    }
  else
  if (this_row_type & ROW_TYPE_CELL)
    {
    if (!bSelStatus)
      {
      DESELECT_DIFFERENT_CELLS_STRUCT ddcs = {NULL, 0} ;

      ddcs.sel = sel ;
      ddcs.row_type = this_row_type ;

      gtk_tree_selection_selected_foreach (sel, deselect_other_type_of_cells, &ddcs) ;
      }
    }

  return TRUE ;
  }

static void deselect_other_type_of_cells (GtkTreeModel *tm, GtkTreePath *tp, GtkTreeIter *itr, gpointer data)
  {
  DESELECT_DIFFERENT_CELLS_STRUCT *ddcs = (DESELECT_DIFFERENT_CELLS_STRUCT *)data ;
  int row_type = -1 ;

  gtk_tree_model_get (tm, itr, BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type, -1) ;

  if (ddcs->row_type != row_type)
    gtk_tree_selection_unselect_path (ddcs->sel, tp) ;
  }

// This callback is responsible for enabling/disabling the appropriate buttons depending on
// the contents of the selection
static void tree_view_selection_changed (GtkTreeSelection *sel, gpointer data)
  {
  bus_layout_D *dialog = (bus_layout_D *)data ;
  gboolean bSomethingSelected = FALSE ;
  gboolean bBusSelected = FALSE ;
  gboolean bSelectionIsFromBus = FALSE ;
  gboolean bFirstSelected = FALSE ;
  gboolean bLastSelected = FALSE ;
  gboolean bFirstBusSelected = TRUE ;
  gboolean bLastBusSelected = TRUE ;
  GtkTreePath *tpSelBus = NULL, *tp = NULL ;
  GtkTreeModel *tm = NULL ;
  int icChildren = -1, Nix ;

  tm = gtk_tree_view_get_model (gtk_tree_selection_get_tree_view (sel)) ;

  if ((bSelectionIsFromBus = (NULL != (tpSelBus = get_selection_bus (sel, &bSomethingSelected)))))
    if (!(bFirstSelected = bLastSelected = bBusSelected = whole_bus_selected_p (sel, tm, tpSelBus)))
      if ((icChildren = gtk_tree_model_path_n_children (tm, tpSelBus)) > 0)
        {
        gtk_tree_path_down (tp = gtk_tree_path_copy (tpSelBus)) ;
        bFirstSelected = gtk_tree_selection_path_is_selected (sel, tp) ;
        for (Nix = 1 ; Nix < icChildren ; Nix++) gtk_tree_path_next (tp) ;
        bLastSelected = gtk_tree_selection_path_is_selected (sel, tp) ;
        gtk_tree_path_free (tp) ;
        }

  if (bSelectionIsFromBus)
    determine_first_or_last_bus (tm, tpSelBus, &bFirstBusSelected, &bLastBusSelected) ;

  gtk_widget_set_sensitive (dialog->btnCreateBus, bSomethingSelected && !bBusSelected) ;
  gtk_widget_set_sensitive (dialog->btnDeleteBus, bSelectionIsFromBus) ;
  gtk_widget_set_sensitive (dialog->btnMoveBusUp, bSelectionIsFromBus && !bFirstBusSelected) ;
  gtk_widget_set_sensitive (dialog->btnMoveBusDown, bSelectionIsFromBus && !bLastBusSelected) ;
  gtk_widget_set_sensitive (dialog->btnMoveCellsUp, bSelectionIsFromBus && !bFirstSelected) ;
  gtk_widget_set_sensitive (dialog->btnMoveCellsDown, bSelectionIsFromBus && !bLastSelected) ;
  gtk_widget_set_sensitive (dialog->lblBusName, bSelectionIsFromBus) ;
  gtk_widget_set_sensitive (dialog->txtBusName, bSelectionIsFromBus) ;

  // Fill in the text box with the name of the bus
  if (bSelectionIsFromBus)
    {
    GtkTreeIter itr ;
    char *psz = NULL ;

    gtk_tree_model_get_iter (tm, &itr, tpSelBus) ;

    gtk_tree_model_get (tm, &itr, BUS_LAYOUT_MODEL_COLUMN_NAME, &psz, -1) ;

    g_signal_handlers_block_matched (G_OBJECT (dialog->txtBusName), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)bus_name_changed, NULL) ;
    gtk_entry_set_text (GTK_ENTRY (dialog->txtBusName), psz) ;
    g_signal_handlers_unblock_matched (G_OBJECT (dialog->txtBusName), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)bus_name_changed, NULL) ;

    g_free (psz) ;
    }

  if (NULL != (tp = g_object_get_data (G_OBJECT (gtk_tree_selection_get_tree_view (sel)), "tpBus")))
    gtk_tree_path_free (tp) ;
  g_object_set_data (G_OBJECT (gtk_tree_selection_get_tree_view (sel)), "tpBus", tpSelBus) ;
  }

// start helpers
static void swap_buses (GtkTreeModel *tm, GtkTreeSelection *sel, bus_layout_D *dialog, GtkTreePath *tpSrc, GtkTreePath *tpDst)
  {
  int Nix ;
  GtkTreeIter itr ;
  SWAP_BUSES_STRUCT src, dst ;
  SWAP_BUSES_STRUCT *min = NULL, *max = NULL ;
  EXP_ARRAY *ar_bSelSrc = NULL, *ar_bSelDst = NULL ;
  gboolean bDstExpanded = TRUE ;

  src.tp = gtk_tree_path_copy (tpSrc) ;
  gtk_tree_model_get_iter (tm, &(src.itr), tpSrc) ;
  src.icChildren = gtk_tree_model_iter_n_children (tm, &(src.itr)) ;

  dst.tp = gtk_tree_path_copy (tpDst) ;
  gtk_tree_model_get_iter (tm, &(dst.itr), tpDst) ;
  dst.icChildren = gtk_tree_model_iter_n_children (tm, &(dst.itr)) ;

  if (dst.icChildren < src.icChildren)
    {
    min = &dst ;
    max = &src ;
    }
  else
    {
    min = &src ;
    max = &dst ;
    }

  bDstExpanded = gtk_tree_view_row_expanded (gtk_tree_selection_get_tree_view (sel), dst.tp) ;

  g_signal_handlers_block_matched (G_OBJECT (sel), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)tree_view_selection_changed, NULL) ;

  for (Nix = 0 ; Nix < max->icChildren - min->icChildren ; Nix++)
    gtk_tree_store_append (GTK_TREE_STORE (tm), &itr, &(min->itr)) ;

  gtk_tree_path_down (src.tp) ;
  gtk_tree_path_down (dst.tp) ;

  ar_bSelSrc = exp_array_new (sizeof (gboolean), 1) ;
  ar_bSelDst = exp_array_new (sizeof (gboolean), 1) ;
  exp_array_insert_vals (ar_bSelSrc, NULL, max->icChildren, 1, 0) ;
  exp_array_insert_vals (ar_bSelDst, NULL, max->icChildren, 1, 0) ;

  for (Nix = 0 ; Nix < max->icChildren ; Nix++)
    {
    exp_array_index_1d (ar_bSelSrc, gboolean, Nix) =
      gtk_tree_selection_path_is_selected (sel, src.tp) ;
    exp_array_index_1d (ar_bSelDst, gboolean, Nix) =
      gtk_tree_selection_path_is_selected (sel, dst.tp) ;

    gtk_tree_path_next (src.tp) ;
    gtk_tree_path_next (dst.tp) ;
    }

  gtk_tree_path_up (src.tp) ;
  gtk_tree_path_up (dst.tp) ;
  gtk_tree_path_down (src.tp) ;
  gtk_tree_path_down (dst.tp) ;

  for (Nix = 0 ; Nix < max->icChildren ; Nix++)
    {
    if (exp_array_index_1d (ar_bSelDst, gboolean, Nix))
      gtk_tree_view_expand_to_path (gtk_tree_selection_get_tree_view (sel), src.tp) ;

    if (exp_array_index_1d (ar_bSelSrc, gboolean, Nix))
      gtk_tree_view_expand_to_path (gtk_tree_selection_get_tree_view (sel), dst.tp) ;

    gtk_tree_path_next (src.tp) ;
    gtk_tree_path_next (dst.tp) ;
    }

  gtk_tree_path_up (src.tp) ;
  gtk_tree_path_up (dst.tp) ;
  gtk_tree_path_down (src.tp) ;
  gtk_tree_path_down (dst.tp) ;

  for (Nix = 0 ; Nix < max->icChildren ; Nix++)
    {
    swap_model_paths_contents (tm, src.tp, dst.tp) ;
    gtk_tree_path_next (src.tp) ;
    gtk_tree_path_next (dst.tp) ;
    }

  gtk_tree_path_up (src.tp) ;
  gtk_tree_path_up (dst.tp) ;
  gtk_tree_path_down (src.tp) ;
  gtk_tree_path_down (dst.tp) ;

  for (Nix = 0 ; Nix < max->icChildren ; Nix++)
    {
    GTK_TREE_SELECTION_SET_PATH_SELECTED (sel, src.tp, exp_array_index_1d (ar_bSelDst, gboolean, Nix)) ;
    GTK_TREE_SELECTION_SET_PATH_SELECTED (sel, dst.tp, exp_array_index_1d (ar_bSelSrc, gboolean, Nix)) ;
    gtk_tree_path_next (src.tp) ;
    gtk_tree_path_next (dst.tp) ;
    }

  gtk_tree_path_up (src.tp) ;
  gtk_tree_path_up (dst.tp) ;
  swap_model_paths_contents (tm, src.tp, dst.tp) ;

  gtk_tree_path_down (src.tp) ;
  gtk_tree_path_down (dst.tp) ;

  for (Nix = 0 ; Nix < min->icChildren ; Nix++)
    gtk_tree_path_next (max->tp) ;
  if (gtk_tree_model_get_iter (tm, &itr, max->tp))
    while (gtk_tree_store_remove (GTK_TREE_STORE (tm), &itr)) ;

  if (!bDstExpanded)
    {
    for (Nix = 0 ; Nix < ar_bSelDst->icUsed ; Nix++)
      if (exp_array_index_1d (ar_bSelDst, gboolean, Nix))
        break ;

    if (Nix == ar_bSelDst->icUsed)
      {
      gtk_tree_path_up (src.tp) ;
      gtk_tree_view_collapse_row (gtk_tree_selection_get_tree_view (sel), src.tp) ;
      }
    }

  exp_array_free (ar_bSelSrc) ;
  exp_array_free (ar_bSelDst) ;
  gtk_tree_path_free (src.tp) ;
  gtk_tree_path_free (dst.tp) ;
  g_signal_handlers_unblock_matched (G_OBJECT (sel), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)tree_view_selection_changed, NULL) ;
  tree_view_selection_changed (sel, dialog) ;
  }

static void swap_model_paths_contents (GtkTreeModel *tm, GtkTreePath *tpSrc, GtkTreePath *tpDst)
  {
  GtkTreeIter itrSrc, itrDst ;

  gtk_tree_model_get_iter (tm, &itrSrc, tpSrc) ;
  gtk_tree_model_get_iter (tm, &itrDst, tpDst) ;

  swap_model_iters_contents (tm, &itrSrc, &itrDst) ;
  }

static void swap_model_iters_contents (GtkTreeModel *tm, GtkTreeIter *itrSrc, GtkTreeIter *itrDst)
  {
  GValue valSrc, valDst ;
  int Nix = 0, icCols = -1 ;
  GtkTreeStore *ts = GTK_TREE_STORE (tm) ;

  if (NULL == tm || NULL == itrSrc || NULL == itrDst) return ;

  memset (&valSrc, 0, sizeof (GValue)) ;
  memset (&valDst, 0, sizeof (GValue)) ;

  icCols = gtk_tree_model_get_n_columns (tm) ;

  for (Nix = 0 ; Nix < icCols ; Nix++)
    {
    gtk_tree_model_get_value (tm, itrSrc, Nix, &valSrc) ;
    gtk_tree_model_get_value (tm, itrDst, Nix, &valDst) ;
    gtk_tree_store_set_value (ts, itrSrc, Nix, &valDst) ;
    gtk_tree_store_set_value (ts, itrDst, Nix, &valSrc) ;

    g_value_unset (&valSrc) ;
    g_value_unset (&valDst) ;
    }
  }

static inline int gtk_tree_model_path_n_children (GtkTreeModel *tm, GtkTreePath *tp)
  {
  GtkTreeIter itr ;

  gtk_tree_model_get_iter (tm, &itr, tp) ;
  return gtk_tree_model_iter_n_children (tm, &itr) ;
  }

static GList *get_bus_refs (GtkTreeModel *tm, GtkTreePath *tpBus)
  {
  int Nix, icChildren = -1 ;
  GList *llRefs = NULL ;
  GtkTreePath *tp = gtk_tree_path_copy (tpBus) ;

  icChildren = gtk_tree_model_path_n_children (tm, tp) ;
  gtk_tree_path_down (tp) ;

  for (Nix = 0 ; Nix < icChildren ; Nix++, gtk_tree_path_next (tp))
    llRefs = g_list_prepend (llRefs, gtk_tree_row_reference_new (tm, tp)) ;

  gtk_tree_path_free (tp) ;

  return llRefs ;
  }

static GList *get_selected_refs (GtkTreeSelection *sel)
  {
  GList *llRet = NULL ;

  gtk_tree_selection_selected_foreach (sel, (GtkTreeSelectionForeachFunc)get_selected_refs_foreach_func, &llRet) ;

  return llRet ;
  }

static void get_selected_refs_foreach_func (GtkTreeModel *tm, GtkTreePath *tp, GtkTreeIter *itr, gpointer data)
  {(*((GList **)(data))) = g_list_prepend ((*((GList **)(data))), gtk_tree_row_reference_new (tm, tp)) ;}

// Grab the bus this selection belongs to (if any). This is done via the following DFA:
// If the selection is not a member of a single bus, return NULL
//
//                              d==2
//          +---------->--------------------->----------------+   +---<---+
//          |                                                 v   v       | d==2 &&
//    +-----+----+ d==1  +---------+ row_type==BUS +---------------+      | parent==tpBus
//    |GSBS_START+------>|GSBS_D1NB+-------------->|GSBS_D1B_ACCEPT+--->--+
//    +----------+       +-----+---+               +--------+------+
//                             |                           |
//                             |row_type!=BUS              | d==2&&parent!=tpBus
//                             |       +------------+      |
//                             +------>|GSBS_REJECT|<------+
//                                     +--+---------+
//                                        |     ^
//                                        +-----+ no matter what
//
static GtkTreePath *get_selection_bus (GtkTreeSelection *sel, gboolean *pbSomethingSelected)
  {
  GET_SELECTION_BUS_STRUCT gsbs = {NULL, GSBS_START} ;

  gtk_tree_selection_selected_foreach (sel, (GtkTreeSelectionForeachFunc)get_selection_bus_foreach_func, &gsbs) ;

  (*pbSomethingSelected) = (GSBS_START != gsbs.state) ;

  if (gsbs.state != GSBS_D1B_ACCEPT)
    {
    if (NULL != gsbs.tpBus)
      gtk_tree_path_free (gsbs.tpBus) ;
    gsbs.tpBus = NULL ;
    }

  return gsbs.tpBus ;
  }

static void get_selection_bus_foreach_func (GtkTreeModel *tm, GtkTreePath *tp, GtkTreeIter *itr, gpointer data)
  {
  GET_SELECTION_BUS_STRUCT *gsbs = (GET_SELECTION_BUS_STRUCT *)data ;
  int this_depth = gtk_tree_path_get_depth (tp), row_type = -1 ;

  if (gsbs->state == GSBS_REJECT) return ;

  gtk_tree_model_get (tm, itr, BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type, -1) ;

  if (GSBS_START == gsbs->state && 2 == this_depth)
    {
    gtk_tree_path_up (gsbs->tpBus = gtk_tree_path_copy (tp)) ;
    gsbs->state = GSBS_D1B_ACCEPT ;
    }
  else
  if (GSBS_START == gsbs->state && 1 == this_depth)
    {
    if (row_type & ROW_TYPE_BUS)
      {
      gsbs->tpBus = gtk_tree_path_copy (tp) ;
      gsbs->state = GSBS_D1B_ACCEPT ;
      }
    else
      gsbs->state = GSBS_REJECT ;
    }
  else // At this point, gsbs->state can only be GSBS_ACCEPT
    {
    if (1 == this_depth) gsbs->state = GSBS_REJECT ;
    else
      {
      GtkTreePath *tpThisParent = NULL ;

      gtk_tree_path_up (tpThisParent = gtk_tree_path_copy (tp)) ;

      gsbs->state = (0 == gtk_tree_path_compare (tpThisParent, gsbs->tpBus)) ? GSBS_D1B_ACCEPT : GSBS_REJECT ;

      gtk_tree_path_free (tpThisParent) ;
      }
    }
  }

static gboolean whole_bus_selected_p (GtkTreeSelection *sel, GtkTreeModel *tm, GtkTreePath *tpSelBus)
  {
  int Nix ;
  int icChildren = -1 ;
  GtkTreePath *tp = NULL ;

  icChildren = gtk_tree_model_path_n_children (tm, tpSelBus) ;

  if (icChildren > 0)
    {
    gtk_tree_path_down (tp = gtk_tree_path_copy (tpSelBus)) ;

    for (Nix = 0 ; Nix < icChildren ; Nix++, gtk_tree_path_next (tp))
      if (!gtk_tree_selection_path_is_selected (sel, tp))
        break ;

    gtk_tree_path_free (tp) ;

    return (Nix == icChildren) ;
    }
  fprintf (stderr, "What ?! The bus had 0 children ?!\n") ;
  return TRUE ;
  }

static GtkTreePath *get_prev_bus (GtkTreeModel *tm, GtkTreePath *tpSelBus)
  {
  GtkTreePath *tp = NULL ;
  GtkTreeIter itr ;
  int row_type ;

  if (1 != gtk_tree_path_get_depth (tpSelBus)) return NULL ;

  tp = gtk_tree_path_copy (tpSelBus) ;
  while (gtk_tree_path_prev (tp))
    {
    gtk_tree_model_get_iter (tm, &itr, tp) ;
    gtk_tree_model_get (tm, &itr, BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type, -1) ;
    if (ROW_TYPE_BUS & row_type)
      return tp ;
    }
  gtk_tree_path_free (tp) ;
  return NULL ;
  }

static GtkTreePath *get_next_bus (GtkTreeModel *tm, GtkTreePath *tpSelBus)
  {
  int row_type ;
  int icRootChildren = -1 ;
  GtkTreePath *tp = NULL ;
  int idxChild = 0 ;
  GtkTreeIter itr ;

  if (1 != gtk_tree_path_get_depth (tpSelBus))
    return NULL ;

  icRootChildren = gtk_tree_model_iter_n_children (tm, NULL) ;

  tp = gtk_tree_path_new_first () ;
  while (gtk_tree_path_compare (tp, tpSelBus) <= 0 && idxChild < icRootChildren)
    {
    gtk_tree_model_get_iter (tm, &itr, tp) ;
    gtk_tree_path_next (tp) ;
    idxChild++ ;
    }

  while (idxChild < icRootChildren)
    {
    gtk_tree_model_get_iter (tm, &itr, tp) ;
    gtk_tree_model_get (tm, &itr, BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type, -1) ;
    if (ROW_TYPE_BUS & row_type)
      return tp ;
    if (++idxChild < icRootChildren)
      gtk_tree_path_next (tp) ;
    }
  gtk_tree_path_free (tp) ;
  return NULL ;
  }

static void determine_first_or_last_bus (GtkTreeModel *tm, GtkTreePath *tpSelBus, gboolean *pbFirstBusSelected, gboolean *pbLastBusSelected)
  {
  GtkTreePath *tp = NULL ;

  if (1 != gtk_tree_path_get_depth (tpSelBus))
    return ;

  (*pbFirstBusSelected) =
  (*pbLastBusSelected) = TRUE ;

  if (NULL != (tp = get_prev_bus (tm, tpSelBus)))
    {
    (*pbFirstBusSelected) = FALSE ;
    gtk_tree_path_free (tp) ;
    }

  if (NULL != (tp = get_next_bus (tm, tpSelBus)))
    {
    (*pbLastBusSelected) = FALSE ;
    gtk_tree_path_free (tp) ;
    }
  }

// start create functions

static void create_bus_layout_dialog (bus_layout_D *dialog)
  {
  GtkWidget *tblMain = NULL, *frm = NULL, *img = NULL, *tbl = NULL, *align = NULL ;

  dialog->dialog = gtk_dialog_new () ;
  gtk_window_set_title (GTK_WINDOW (dialog->dialog), _("Bus Layout")) ;
  gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), TRUE) ;
  gtk_window_set_default_size (GTK_WINDOW (dialog->dialog), 300, 200) ;

  tblMain = gtk_table_new (1, 2, FALSE) ;
  gtk_widget_show (tblMain) ;
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog->dialog)->vbox), tblMain, TRUE, TRUE, 0) ;
  gtk_container_set_border_width (GTK_CONTAINER (tblMain), 2) ;

  frm = gtk_frame_new (_("Cells And Buses")) ;
  gtk_widget_show (frm) ;
  gtk_table_attach (GTK_TABLE (tblMain), frm, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (frm), 2) ;

  dialog->sw = gtk_scrolled_window_new (NULL, NULL) ;
  gtk_widget_show (dialog->sw) ;
  gtk_container_add (GTK_CONTAINER (frm), dialog->sw) ;
  gtk_container_set_border_width (GTK_CONTAINER (dialog->sw), 2) ;
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (dialog->sw), GTK_SHADOW_IN) ;
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (dialog->sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC) ;

  dialog->tview = create_bus_layout_tree_view (FALSE, NULL, GTK_SELECTION_MULTIPLE) ;
  gtk_widget_show (dialog->tview) ;
  gtk_container_add (GTK_CONTAINER (dialog->sw), dialog->tview) ;
  gtk_tree_selection_set_select_function (gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->tview)),
    (GtkTreeSelectionFunc)select_cell_row_p, NULL, NULL) ;

  align = gtk_alignment_new (0.0, 0.0, 1.0, 1.0) ;
  gtk_widget_show (align) ;
  gtk_table_attach (GTK_TABLE (tblMain), align, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (align), 2) ;

  tbl = gtk_table_new (3, 1, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_container_add (GTK_CONTAINER (align), tbl) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;

  dialog->bbox = gtk_vbutton_box_new () ;
  gtk_widget_show (dialog->bbox) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->bbox, 0, 1, 0, 1,
    (GtkAttachOptions)(0),
    (GtkAttachOptions)(0), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (dialog->bbox), 2) ;
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog->bbox), gtk_vbutton_box_get_spacing_default ()) ;

  dialog->btnCreateBus = create_pixmap_button (img = gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_BUTTON), _("Create Bus"), FALSE) ;
  gtk_widget_show (dialog->btnCreateBus) ;
  gtk_widget_show (img) ;
  gtk_box_pack_start (GTK_BOX (dialog->bbox), dialog->btnCreateBus, FALSE, TRUE, 0) ;

  dialog->btnDeleteBus = create_pixmap_button (img = gtk_image_new_from_stock (GTK_STOCK_REMOVE, GTK_ICON_SIZE_BUTTON), _("Delete Bus"), FALSE) ;
  gtk_widget_show (dialog->btnDeleteBus) ;
  gtk_widget_show (img) ;
  gtk_box_pack_start (GTK_BOX (dialog->bbox), dialog->btnDeleteBus, FALSE, TRUE, 0) ;

  dialog->btnMoveBusUp = create_pixmap_button (img = gtk_image_new_from_stock (GTK_STOCK_GO_UP, GTK_ICON_SIZE_BUTTON), _("Move Bus Up"), FALSE) ;
  gtk_widget_show (dialog->btnMoveBusUp) ;
  gtk_widget_show (img) ;
  gtk_box_pack_start (GTK_BOX (dialog->bbox), dialog->btnMoveBusUp, FALSE, TRUE, 0) ;

  dialog->btnMoveBusDown = create_pixmap_button (img = gtk_image_new_from_stock (GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_BUTTON), _("Move Bus Down"), FALSE) ;
  gtk_widget_show (dialog->btnMoveBusDown) ;
  gtk_widget_show (img) ;
  gtk_box_pack_start (GTK_BOX (dialog->bbox), dialog->btnMoveBusDown, FALSE, TRUE, 0) ;

  dialog->btnMoveCellsUp = create_pixmap_button (img = gtk_image_new_from_stock (GTK_STOCK_GO_UP, GTK_ICON_SIZE_BUTTON), _("Make Cell(s) More Significant"), FALSE) ;
  gtk_widget_show (dialog->btnMoveCellsUp) ;
  gtk_widget_show (img) ;
  gtk_box_pack_start (GTK_BOX (dialog->bbox), dialog->btnMoveCellsUp, FALSE, TRUE, 0) ;

  dialog->btnMoveCellsDown = create_pixmap_button (img = gtk_image_new_from_stock (GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_BUTTON), _("Make Cell(s) Less Significant"), FALSE) ;
  gtk_widget_show (dialog->btnMoveCellsDown) ;
  gtk_widget_show (img) ;
  gtk_box_pack_start (GTK_BOX (dialog->bbox), dialog->btnMoveCellsDown, FALSE, TRUE, 0) ;

  dialog->lblBusName = gtk_label_new (_("Bus Name:")) ;
  gtk_widget_show (dialog->lblBusName) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->lblBusName, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(0), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (dialog->lblBusName), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (dialog->lblBusName), 0.0, 1.0) ;

  dialog->txtBusName = gtk_entry_new () ;
  gtk_widget_show (dialog->txtBusName) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->txtBusName, 0, 1, 2, 3,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(0), 2, 2) ;

  dialog->lblWarning = gtk_label_new (NULL) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->lblWarning, 0, 1, 3, 4,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(0), 2, 2) ;
  gtk_label_set_markup (GTK_LABEL (dialog->lblWarning), _("<span foreground=\"red\">Warning:</span> The bus must have a name.")) ;
  gtk_label_set_justify (GTK_LABEL (dialog->lblWarning), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (dialog->lblWarning), 0.0, 0.0) ;
  gtk_label_set_line_wrap (GTK_LABEL (dialog->lblWarning), TRUE) ;

  gtk_dialog_add_button (GTK_DIALOG (dialog->dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  dialog->btnOK = gtk_dialog_add_button (GTK_DIALOG (dialog->dialog), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->dialog), GTK_RESPONSE_OK) ;

  g_signal_connect (G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->tview))), "changed", (GCallback)tree_view_selection_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->btnCreateBus),     "clicked", (GCallback)create_bus_button_clicked, dialog) ;
  g_signal_connect (G_OBJECT (dialog->btnDeleteBus),     "clicked", (GCallback)delete_bus_button_clicked, dialog) ;
  g_signal_connect (G_OBJECT (dialog->btnMoveCellsUp),   "clicked", (GCallback)raise_bus_cell_position,   dialog) ;
  g_signal_connect (G_OBJECT (dialog->btnMoveCellsDown), "clicked", (GCallback)lower_bus_cell_position,   dialog) ;
  g_signal_connect (G_OBJECT (dialog->btnMoveBusUp),     "clicked", (GCallback)raise_bus_position,        dialog) ;
  g_signal_connect (G_OBJECT (dialog->btnMoveBusDown),   "clicked", (GCallback)lower_bus_position,        dialog) ;
  g_signal_connect (G_OBJECT (dialog->txtBusName),       "changed", (GCallback)bus_name_changed,          dialog) ;
  }

GtkWidget *create_bus_layout_tree_view (gboolean bColsVisible, char *pszColumnTitle, GtkSelectionMode sel_mode)
  {
  GtkTreeViewColumn *col = NULL ;
  GtkCellRenderer *cr = NULL ;
  GtkWidget *tview = gtk_tree_view_new () ;

  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (tview)), sel_mode);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tview), bColsVisible) ;
  gtk_tree_view_append_column (GTK_TREE_VIEW (tview), col = gtk_tree_view_column_new ()) ;
  if (NULL != pszColumnTitle)
    gtk_tree_view_column_set_title (col, pszColumnTitle) ;
  gtk_tree_view_column_pack_start (col, cr = gtk_cell_renderer_pixbuf_new (), FALSE) ;
  gtk_tree_view_column_add_attribute (col, cr, "stock-id", BUS_LAYOUT_MODEL_COLUMN_ICON) ;
  gtk_tree_view_column_pack_start (col, cr = gtk_cell_renderer_text_new (), TRUE) ;
  gtk_tree_view_column_add_attribute (col, cr, "text", BUS_LAYOUT_MODEL_COLUMN_NAME) ;
  return tview ;
  }

gboolean gtk_tree_model_iter_next_dfs (GtkTreeModel *model, GtkTreeIter *itr)
  {
  GtkTreeIter itrTree ;

  if (gtk_tree_model_iter_n_children (model, itr) > 0)
    {
    if (!gtk_tree_model_iter_children (model, &itrTree, itr))
      return gtk_tree_model_iter_next (model, itr) ;
    else
      {
      memcpy (itr, &itrTree, sizeof (GtkTreeIter)) ;
      return TRUE ;
      }
    }
  else
  if (gtk_tree_model_iter_next (model, itr))
    return TRUE ;
  else
    {
    if (!gtk_tree_model_iter_parent (model, &itrTree, itr))
      return FALSE ;
    else
    if (!gtk_tree_model_iter_next (model, &itrTree))
      return FALSE ;
    else
      memcpy (itr, &itrTree, sizeof (GtkTreeIter)) ;
    }
  return TRUE ;
  }

void bus_layout_tree_model_dump (GtkTreeModel *model, FILE *pfile)
  {
  GtkTreeIter itr ;

  if (!gtk_tree_model_get_iter_first (model, &itr)) return ;

  bus_layout_tree_model_dump_priv (model, pfile, &itr, 0) ;
  }

static void bus_layout_tree_model_dump_priv (GtkTreeModel *model, FILE *pfile, GtkTreeIter *itr, int icIndent)
  {
  char *pszIcon = NULL, *pszName = NULL ;
  int row_type = -1, idx = -1 ;
  GtkTreeIter itrChild ;

  do
    {
    gtk_tree_model_get (model, itr, 
      BUS_LAYOUT_MODEL_COLUMN_ICON, &pszIcon,
      BUS_LAYOUT_MODEL_COLUMN_NAME, &pszName,
      BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type,
      BUS_LAYOUT_MODEL_COLUMN_INDEX, &idx, -1) ;

    fprintf (pfile, "%*sicon:%s, name:%s, row_type:%d, idx = %d\n", icIndent, "", pszIcon, pszName, row_type, idx) ;
    g_free (pszIcon) ;
    g_free (pszName) ;

    if (gtk_tree_model_iter_children (model, &itrChild, itr))
      bus_layout_tree_model_dump_priv (model, pfile, &itrChild, icIndent + 2) ;
    }
  while (gtk_tree_model_iter_next (model, itr)) ;
  }
