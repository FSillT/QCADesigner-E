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
// The layer order dialog. It allows the user to change //
// the vertical order of layers.                        //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include "support.h"
#include "custom_widgets.h"
#include "design.h"

typedef struct
  {
  GtkWidget *dialog ;
  GtkWidget *tview ;
  GtkWidget *sw ;
  } layer_order_D ;

extern char *layer_pixmap_stock_id[LAYER_TYPE_LAST_TYPE] ;

static layer_order_D layer_order_dialog = {NULL} ;

static void create_layer_order_dialog (layer_order_D *dialog) ;
static void LayerOrderToDialog (layer_order_D *dialog, DESIGN *design) ;
static GList *DialogToLayerOrder (layer_order_D *dialog) ;

GList *get_layer_order_from_user (GtkWindow *parent, DESIGN *design)
  {
  GList *llRet = NULL ;

  if (NULL == layer_order_dialog.dialog)
    create_layer_order_dialog (&layer_order_dialog) ;

  gtk_window_set_transient_for (GTK_WINDOW (layer_order_dialog.dialog), parent) ;

  LayerOrderToDialog (&layer_order_dialog, design) ;

  if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (layer_order_dialog.dialog)))
    llRet = DialogToLayerOrder (&layer_order_dialog) ;

  gtk_widget_hide (layer_order_dialog.dialog) ;

  return llRet ;
  }

static GList *DialogToLayerOrder (layer_order_D *dialog)
  {
  GList *llRet = NULL ;
  GtkTreeModel *tm = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tview)) ;
  GtkTreePath *tp = NULL ;
  GtkTreeIter itr ;
  gpointer p = NULL ;
  int icChildren = 0, Nix ;

  icChildren = gtk_tree_model_iter_n_children (tm, NULL) ;

  tp = gtk_tree_path_new_first () ;

  for (Nix = 0 ; Nix < icChildren ; Nix++)
    {
    gtk_tree_model_get_iter (tm, &itr, tp) ;
    gtk_tree_model_get (tm, &itr, LAYER_MODEL_COLUMN_LAYER, &p, -1) ;
    llRet = g_list_prepend (llRet, p) ;
    gtk_tree_path_next (tp) ;
    }

  gtk_tree_path_free (tp) ;

  return llRet ;
  }

static void LayerOrderToDialog (layer_order_D *dialog, DESIGN *design)
  {
  GtkListStore *ls = NULL ;

  ls = design_layer_list_store_new (design, 0) ;

  gtk_tree_view_set_model (GTK_TREE_VIEW (dialog->tview), GTK_TREE_MODEL (ls)) ;

  scrolled_window_set_size (GTK_SCROLLED_WINDOW (dialog->sw), dialog->tview, 0.8, 0.8) ;

  g_object_unref (ls) ;
  }

static void create_layer_order_dialog (layer_order_D *dialog)
  {
  GtkWidget *tbl = NULL, *lbl = NULL ;
  GtkTreeViewColumn *col = NULL ;
  GtkCellRenderer *cr = NULL ;

  dialog->dialog = gtk_dialog_new () ;
  gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), TRUE) ;
  gtk_window_set_title (GTK_WINDOW (dialog->dialog), _("Layer Order")) ;

  tbl = gtk_table_new (2, 1, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog->dialog)->vbox), tbl, TRUE, TRUE, 0) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;

  lbl = gtk_label_new (_("Drag and drop layers to their new location:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5) ;

  dialog->sw = gtk_scrolled_window_new (NULL, NULL) ;
  gtk_widget_show (dialog->sw) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->sw, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (dialog->sw), 2) ;
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (dialog->sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC) ;
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (dialog->sw), GTK_SHADOW_IN) ;

  dialog->tview = gtk_tree_view_new () ;
  gtk_widget_show (dialog->tview) ;
  gtk_container_add (GTK_CONTAINER (dialog->sw), dialog->tview) ;
  gtk_tree_view_append_column (GTK_TREE_VIEW (dialog->tview), col = gtk_tree_view_column_new ()) ;
  gtk_tree_view_column_pack_start (col, cr = gtk_cell_renderer_pixbuf_new (), FALSE) ;
  gtk_tree_view_column_add_attribute (col, cr, "stock-id", LAYER_MODEL_COLUMN_ICON) ;
  gtk_tree_view_column_pack_start (col, cr = gtk_cell_renderer_text_new (), FALSE) ;
  gtk_tree_view_column_add_attribute (col, cr, "text", LAYER_MODEL_COLUMN_NAME) ;
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (dialog->tview), FALSE) ;
  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (dialog->tview), TRUE) ;

  gtk_dialog_add_button (GTK_DIALOG (dialog->dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->dialog), GTK_STOCK_OK,     GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->dialog), GTK_RESPONSE_OK) ;
  }
