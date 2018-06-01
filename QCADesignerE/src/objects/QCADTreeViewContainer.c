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
// A tree view container that allows "freeze columns".  //
// That is, the horizontal scrolling does not scroll    //
// the entire tree view but, instead, it hides and      //
// shows columns as appropriate, keeping the first n    //
// columns always visible.                              //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include "./generic_utils.h"
#include "QCADTreeViewContainer.h"

static void qcad_tree_view_container_instance_init (QCADTreeViewContainer *tvc) ;
static void qcad_tree_view_container_instance_finalize (GObject *obj) ;
static void qcad_tree_view_container_class_init (QCADTreeViewContainerClass *klass) ;
static void qcad_tree_view_container_add (GtkContainer *container, GtkWidget *child) ;

static void hscroll_adj_value_changed (GtkAdjustment *adj, gpointer data) ;
static void fake_hadj_value_changed (GtkAdjustment *adj, gpointer data) ;
static void tree_view_size_allocate (GtkTreeView *tv, GtkAllocation *alloc, gpointer data) ;
static gboolean tree_view_move_cursor (GtkTreeView *tv, GtkMovementStep movement_step, gint count, gpointer data) ;
static void tree_view_cursor_changed (GtkTreeView *tv, gpointer data) ;

static int tree_view_count_columns (GtkTreeView *tv) ;
static int tree_view_column_get_width (GtkTreeView *tv, GtkTreeViewColumn *col, gboolean bLastCol) ;
static void set_hscroll_upper (QCADTreeViewContainer *tvc) ;
static void tree_view_handle_cursor_movement (GtkTreeView *tv, QCADTreeViewContainer *tvc, int icSteps, gboolean bDownwards) ;

GType qcad_tree_view_container_get_type ()
  {
  static GType tvc_type = 0 ;

  if (0 == tvc_type)
    {
    static GTypeInfo tvc_info = 
      {
      sizeof (QCADTreeViewContainerClass),
      NULL,
      NULL,
      (GClassInitFunc)qcad_tree_view_container_class_init,
      NULL,
      NULL,
      sizeof (QCADTreeViewContainer),
      0,
      (GInstanceInitFunc)qcad_tree_view_container_instance_init
      } ;

    if ((tvc_type = g_type_register_static (GTK_TYPE_SCROLLED_WINDOW, QCAD_TYPE_STRING_TREE_VIEW_CONTAINER, &tvc_info, 0)))
      g_type_class_ref (tvc_type) ;
    }
  return tvc_type ;
  }

static void qcad_tree_view_container_class_init (QCADTreeViewContainerClass *klass)
  {
  GTK_CONTAINER_CLASS (klass)->add = qcad_tree_view_container_add ;
  G_OBJECT_CLASS (g_type_class_peek (QCAD_TYPE_TREE_VIEW_CONTAINER))->finalize = qcad_tree_view_container_instance_finalize ;
  }

static void qcad_tree_view_container_instance_init (QCADTreeViewContainer *tvc)
  {
  tvc->n_frozen_columns = 0 ;
  tvc->fake_hadj = GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1, 1, 1, 1)) ;
  g_signal_connect (G_OBJECT (tvc->fake_hadj), "value-changed", (GCallback)fake_hadj_value_changed, NULL) ;
  }

static void qcad_tree_view_container_instance_finalize (GObject *obj)
  {g_object_unref (QCAD_TREE_VIEW_CONTAINER (obj)->fake_hadj) ;}

static void qcad_tree_view_container_add (GtkContainer *container, GtkWidget *child)
  {
  GtkWidget *old_child = NULL ;
  if (!GTK_IS_TREE_VIEW (child)) return ;

  if (NULL != (old_child = GTK_BIN (container)->child))
    {
    g_signal_handlers_disconnect_matched (G_OBJECT (old_child), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, tree_view_size_allocate, container) ;
    g_signal_handlers_disconnect_matched (G_OBJECT (old_child), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, tree_view_move_cursor, container) ;
    g_signal_handlers_disconnect_matched (G_OBJECT (old_child), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, tree_view_cursor_changed, container) ;
    }

  GTK_BIN (container)->child = child ;
  gtk_widget_set_parent (child, GTK_WIDGET (container)) ;
  gtk_tree_view_set_vadjustment (GTK_TREE_VIEW (child), gtk_range_get_adjustment (GTK_RANGE (GTK_SCROLLED_WINDOW (container)->vscrollbar))) ;
  gtk_tree_view_set_hadjustment (GTK_TREE_VIEW (child), gtk_range_get_adjustment (GTK_RANGE (GTK_SCROLLED_WINDOW (container)->hscrollbar))) ;

  g_signal_connect (G_OBJECT (child), "size-allocate", (GCallback)tree_view_size_allocate, container) ;
  g_signal_connect (G_OBJECT (child), "move-cursor", (GCallback)tree_view_move_cursor, container) ;
  g_signal_connect (G_OBJECT (child), "cursor-changed", (GCallback)tree_view_cursor_changed, container) ;
  }

GtkWidget *qcad_tree_view_container_new ()
  {
  GtkWidget *ret = g_object_new (QCAD_TYPE_TREE_VIEW_CONTAINER, "hadjustment", NULL, "vadjustment", NULL, NULL) ;
  g_signal_connect (G_OBJECT (gtk_range_get_adjustment (GTK_RANGE (GTK_SCROLLED_WINDOW (ret)->hscrollbar))),
    "value-changed", (GCallback)hscroll_adj_value_changed, ret) ;
  return ret ;
  }

void qcad_tree_view_container_freeze_columns (QCADTreeViewContainer *tvc, int n_columns)
  {
  int n_tv_cols = 0 ;
  GtkTreeView *tv = NULL ;
  GtkAdjustment *adjHScroll = NULL ;

  if (!QCAD_IS_TREE_VIEW_CONTAINER (tvc)) return ;
  if (NULL == (tv = GTK_TREE_VIEW (GTK_BIN (tvc)->child))) return ;

  n_columns = 
  QCAD_TREE_VIEW_CONTAINER (tvc)->n_frozen_columns = 
    CLAMP (n_columns, 0, n_tv_cols = tree_view_count_columns (tv)) ;

  // Set the tree view hadjustment if we're going back to normal scrolling
  adjHScroll = gtk_range_get_adjustment (GTK_RANGE (GTK_SCROLLED_WINDOW (tvc)->hscrollbar)) ;
  if (0 == n_columns && (gtk_tree_view_get_hadjustment (tv) != adjHScroll))
    gtk_tree_view_set_hadjustment (tv, adjHScroll) ;
  else
  // Remove the hadjustment if we're freezing columns
  if (n_columns > 0 && gtk_tree_view_get_hadjustment (tv) == adjHScroll)
    gtk_tree_view_set_hadjustment (tv, tvc->fake_hadj) ;

  if (n_columns > 0 && NULL != (adjHScroll = gtk_range_get_adjustment (GTK_RANGE (GTK_SCROLLED_WINDOW (tvc)->hscrollbar))))
    {
    adjHScroll->value = 0 ;
    adjHScroll->lower = 0 ;
    adjHScroll->upper = 1 ;
    adjHScroll->step_increment = 1 ;
    adjHScroll->page_increment = 10 ;
    adjHScroll->page_size = 1 ;
    set_hscroll_upper (tvc) ;
    gtk_adjustment_value_changed (adjHScroll) ;
    }
  }

static void fake_hadj_value_changed (GtkAdjustment *adj, gpointer data)
  {if (adj->value != adj->lower) gtk_adjustment_set_value (adj, adj->lower) ;}

static void tree_view_size_allocate (GtkTreeView *tv, GtkAllocation *alloc, gpointer data)
  {set_hscroll_upper (QCAD_TREE_VIEW_CONTAINER (data)) ;}

static void tree_view_cursor_changed (GtkTreeView *tv, gpointer data)
  {
  if (QCAD_TREE_VIEW_CONTAINER (data) > 0)
    tree_view_handle_cursor_movement (tv, QCAD_TREE_VIEW_CONTAINER (data), 0, FALSE) ;
  }

static gboolean tree_view_move_cursor (GtkTreeView *tv, GtkMovementStep movement_step, gint count, gpointer data)
  {
  if (GTK_MOVEMENT_VISUAL_POSITIONS == movement_step && QCAD_TREE_VIEW_CONTAINER (data)->n_frozen_columns > 0)
    {
    tree_view_handle_cursor_movement (tv, QCAD_TREE_VIEW_CONTAINER (data), count, TRUE) ;
    return TRUE ;
    }
  return FALSE ;
  }

static void hscroll_adj_value_changed (GtkAdjustment *adj, gpointer data)
  {
  int Nix ;
  GList *llCols = NULL, *llItr = NULL ;
  QCADTreeViewContainer *tvc = QCAD_TREE_VIEW_CONTAINER (data) ;

  llCols = gtk_tree_view_get_columns (GTK_TREE_VIEW (GTK_BIN (tvc)->child)) ;

  for (Nix = adj->value, llItr = g_list_nth (llCols, tvc->n_frozen_columns) ; 
       llItr != NULL ; 
       Nix--, llItr = llItr->next)
    {
    if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN (llItr->data)) && (Nix <= 0)) break ;
    gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN (llItr->data), (Nix <= 0)) ;
    }

  g_list_free (llCols) ;
  }

static int tree_view_count_columns (GtkTreeView *tv)
  {
  int n_columns = 0 ;
  GList *llCols = NULL ;

  if (NULL == tv) return 0 ;

  llCols = gtk_tree_view_get_columns (tv) ;

  n_columns = g_list_length (llCols) ;

  g_list_free (llCols) ;

  return n_columns ;
  }

static void set_hscroll_upper (QCADTreeViewContainer *tvc)
  {
  GtkAdjustment *adjHScroll = NULL ;
  gboolean bUpperChanged = FALSE, bValueChanged = FALSE ;
  GdkRectangle rcTV = {0} ;
  GtkTreeView *tv = NULL ;
  GList *llCols = NULL, *llItr = NULL ;
  int Nix, icToHide = 0, icCols = 0, new_value = -1, new_upper = -1 ;

  if (0 == tvc->n_frozen_columns) return ;
  if (NULL == GTK_SCROLLED_WINDOW (tvc)->hscrollbar) return ;
  if ((icCols = tree_view_count_columns (tv = GTK_TREE_VIEW (GTK_BIN (tvc)->child))) <= tvc->n_frozen_columns) return ;
  if (NULL == (llCols = gtk_tree_view_get_columns (tv))) return ;

  gtk_tree_view_get_visible_rect (tv, &rcTV) ;

  for (Nix = 0, llItr = llCols; Nix < tvc->n_frozen_columns && llItr != NULL ; Nix++, llItr = llItr->next)
    rcTV.width -= tree_view_column_get_width (tv, GTK_TREE_VIEW_COLUMN (llItr->data), (NULL == llItr->next)) ;

  for (Nix = 0, llItr = g_list_last (llCols) ; llItr != NULL && Nix < icCols - tvc->n_frozen_columns ; llItr = llItr->prev, Nix++)
    if ((rcTV.width -= tree_view_column_get_width (tv, GTK_TREE_VIEW_COLUMN (llItr->data), (NULL == llItr->next))) < 0)
      icToHide++ ;

  g_list_free (llCols) ;

  adjHScroll = gtk_range_get_adjustment (GTK_RANGE (GTK_SCROLLED_WINDOW (tvc)->hscrollbar)) ;

  new_upper = icToHide + 1 ;
  new_value = new_upper - (adjHScroll->upper - adjHScroll->value) ;
  new_value = CLAMP (new_value, 0, new_upper - adjHScroll->page_size) ;

  if ((bUpperChanged = (new_upper != adjHScroll->upper)))
    adjHScroll->upper = new_upper ;
  if ((bValueChanged = (adjHScroll->value != new_value)))
    adjHScroll->value = new_value ;
  if (bUpperChanged)
    gtk_adjustment_changed (adjHScroll) ;
  if (bValueChanged)
    gtk_adjustment_value_changed (adjHScroll) ;
  }

static int tree_view_column_get_width (GtkTreeView *tv, GtkTreeViewColumn *col, gboolean bLastCol)
  {
  int col_width = 0 ;

  if (NULL == col) return 0 ;

  col_width = MAX (col->requested_width, col->button_request) ;
  if (-1 != col->min_width)
    col_width = MAX (col_width, col->min_width) ;
  if (-1 != col->max_width)
    col_width = MIN (col_width, col->max_width) ;

  return col_width ;
  }

static void tree_view_handle_cursor_movement (GtkTreeView *tv, QCADTreeViewContainer *tvc, int icSteps, gboolean bDownwards)
  {
  GtkTreePath *tp = NULL ;
  GtkTreeViewColumn *col = NULL ;

  gtk_tree_view_get_cursor (tv, &tp, &col) ;
  if (NULL != col)
    {
    GList *llCols = NULL ;

    if (NULL != (llCols = gtk_tree_view_get_columns (tv)))
      {
      GList *llFirst = NULL ;

      if (NULL != (llFirst = g_list_nth (llCols, tvc->n_frozen_columns)))
        {
        GtkAdjustment *adjHScroll = NULL ;

        if (NULL != (adjHScroll = gtk_range_get_adjustment (GTK_RANGE (GTK_SCROLLED_WINDOW (tvc)->hscrollbar))))
          {
          GList *llItr = NULL ;
          int idx, new_value = 0 ;
          for (idx = 0, llItr = llCols ; llItr != NULL ; llItr = llItr->next, idx++)
            if (llItr->data == col)
              break ;

          if (idx >= tvc->n_frozen_columns)
            {
            GList *llDst = NULL ;

            idx = idx - tvc->n_frozen_columns + icSteps ;

            if (bDownwards)
              {
              if (idx < adjHScroll->value)
                gtk_adjustment_set_value (adjHScroll, MAX (adjHScroll->lower, idx)) ;
              }
            else
              if (idx >= adjHScroll->value)
                if (NULL != (llDst = g_list_nth (llFirst, idx)))
                  {
                  int icToHide = 0 ;
                  GdkRectangle rcTV = {0} ;

                  gtk_tree_view_get_visible_rect (tv, &rcTV) ;

                  for (llItr = llCols ; llItr != NULL ; llItr = llItr->next)
                    {
                    if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN (llItr->data)))
                      if ((rcTV.width -= tree_view_column_get_width (tv, GTK_TREE_VIEW_COLUMN (llItr->data), (NULL == llItr->next))) < 0)
                        icToHide++ ;
                    if (llItr == llDst) break ;
                    }

                  if (icToHide > 0)
                    {
                    new_value = adjHScroll->value + icToHide ;
                    new_value = CLAMP (new_value, adjHScroll->lower, adjHScroll->upper - adjHScroll->page_size) ;
                    if (new_value != adjHScroll->value)
                      gtk_adjustment_set_value (adjHScroll, new_value) ;
                    }
                  }
            }
          }
        }
      g_list_free (llCols) ;
      }
    }
  if (NULL != tp) gtk_tree_path_free (tp) ;
  }
