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
// layer_mapping_dialog.c                               //
// 2004.10.01                                           //
// author: Mike Mazur                                   //
//                                                      //
// description                                          //
//                                                      //
// Layer mapping dialog. When importing a new block,    //
// the user must select, for each block layer, a design //
// layer to merge the block layer's content into.       //
// Alternatively, the user can choose to create a new   //
// layer for a given block layer.                       //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "support.h"
#include "exp_array.h"
#include "custom_widgets.h"
#include "layer_mapping_dialog.h"
#include "design.h"
#include "objects/QCADCellRendererLayerList.h"

typedef struct
  {
  GtkWidget *dlgLayerMapping;
  GtkWidget *tv ;
  GtkCellRenderer *crImport ;
  GtkCellRenderer *crDst ;
  GtkWidget *sw ;
  } layer_mapping_D;

enum
  {
  LAYER_MAPPING_MODEL_COLUMN_IMPORT = LAYER_MODEL_COLUMN_LAST,
  LAYER_MAPPING_MODEL_COLUMN_DEST,
  LAYER_MAPPING_MODEL_COLUMN_LAST
  } ;

static layer_mapping_D layer_mapping_dialog = { NULL };

// function prototypes
static void create_layer_mapping_dialog (layer_mapping_D *dialog);
static GtkTreeModel *layer_mapping_model_new (DESIGN *design, DESIGN *block) ;
void dest_layer_changed (GtkCellRendererText *cr, char *pszPath, QCADLayer *new_layer, gpointer data) ;
void import_layer_toggled (GtkCellRendererToggle *cr, char *pszPath, gpointer user_data) ;
/*
static void add_layer_line (layer_mapping_D *dialog, QCADLayer *block_layer, int idx, DESIGN *design);
static void set_design_combo_layer_data (GtkWidget *widget, gpointer data);
static void import_layer_toggled (GtkWidget *widget, gpointer data);
*/

EXP_ARRAY *get_layer_mapping_from_user (GtkWidget *parent, DESIGN *design, DESIGN *block)
  {
//  int idx = -1;
//  GList *llItr = NULL;
  EXP_ARRAY *layer_mappings = NULL ;
  GtkTreeModel *tm = NULL ;
  gboolean bImport = FALSE ;
//  LAYER_MAPPING *layer_mapping = NULL ;
//  int number_of_block_layers = 0;		// total number of layers in new block

  // create dialog if it doesn't already exist
  if(NULL == layer_mapping_dialog.dlgLayerMapping)
  	create_layer_mapping_dialog(&layer_mapping_dialog);

  // some boilerplate things
  gtk_window_set_transient_for (GTK_WINDOW (layer_mapping_dialog.dlgLayerMapping), GTK_WINDOW (parent));

  tm = layer_mapping_model_new (design, block) ;
  g_object_set (G_OBJECT (layer_mapping_dialog.crDst), "design", design, NULL) ;
  g_object_set_data (G_OBJECT (layer_mapping_dialog.crDst), "model", tm) ;
  g_object_set_data (G_OBJECT (layer_mapping_dialog.crImport), "model", tm) ;
  gtk_tree_view_set_model (GTK_TREE_VIEW (layer_mapping_dialog.tv), tm) ;

  scrolled_window_set_size (GTK_SCROLLED_WINDOW (layer_mapping_dialog.sw), layer_mapping_dialog.tv, 0.8, 0.8) ;

  if (GTK_RESPONSE_OK == gtk_dialog_run(GTK_DIALOG(layer_mapping_dialog.dlgLayerMapping)))
    {
    GtkTreeIter itr ;
    LAYER_MAPPING lm = {NULL, NULL} ;

    if (gtk_tree_model_get_iter_first (tm, &itr))
      {
      layer_mappings = exp_array_new (sizeof (LAYER_MAPPING), 1) ;
      do
        {
        lm.design_layer = lm.block_layer = NULL ;
        gtk_tree_model_get (tm, &itr,
          LAYER_MODEL_COLUMN_LAYER, &(lm.block_layer),
          LAYER_MAPPING_MODEL_COLUMN_DEST, &(lm.design_layer), 
          LAYER_MAPPING_MODEL_COLUMN_IMPORT, &bImport, -1) ;
        if (bImport)
          exp_array_insert_vals (layer_mappings, &lm, 1, 1, -1) ;
        } while (gtk_tree_model_iter_next (tm, &itr)) ;
      }
    }

  gtk_widget_hide (layer_mapping_dialog.dlgLayerMapping);

  if (NULL != parent)
    gtk_window_present (GTK_WINDOW (parent)) ;

  return layer_mappings ;
  }

static GtkTreeModel *layer_mapping_model_new (DESIGN *design, DESIGN *block)
  {
  GList *llItr = NULL ;
  GtkListStore *ls = NULL ;
  GtkTreeIter itr ;
  QCADLayer *src_layer = NULL, *dst_layer = NULL ;

  ls = design_layer_list_store_new (block, 2, G_TYPE_BOOLEAN, G_TYPE_POINTER) ;

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (ls), &itr))
    do
      {
      dst_layer = NULL ;
      gtk_tree_model_get (GTK_TREE_MODEL (ls), &itr, LAYER_MODEL_COLUMN_LAYER, &src_layer, -1) ;
      for (llItr = design->lstLayers ; llItr != NULL && NULL == dst_layer ; llItr = llItr->next)
        if (QCAD_LAYER (llItr->data)->type == src_layer->type && 
            !(1 == (int)g_object_get_data (G_OBJECT (llItr->data), "dst_layer")))
          {
          if (NULL == QCAD_LAYER (llItr->data)->pszDescription && NULL == src_layer->pszDescription)
            dst_layer = llItr->data ;
          else
          if (!(NULL == QCAD_LAYER (llItr->data)->pszDescription || NULL == src_layer->pszDescription))
            if (!strcmp (QCAD_LAYER (llItr->data)->pszDescription, src_layer->pszDescription))
              dst_layer = llItr->data ;
          }
      if (NULL != dst_layer)
        g_object_set_data (G_OBJECT (dst_layer), "dst_layer", (gpointer)1) ;
      gtk_list_store_set (ls, &itr, 
        LAYER_MAPPING_MODEL_COLUMN_IMPORT, TRUE,
        LAYER_MAPPING_MODEL_COLUMN_DEST, dst_layer, -1) ;
      }
    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (ls), &itr)) ;

  for (llItr = design->lstLayers ; llItr != NULL ; llItr = llItr->next)
    g_object_set_data (G_OBJECT (llItr->data), "dst_layer", (gpointer)0) ;

  return GTK_TREE_MODEL (ls) ;
  }

void import_layer_toggled (GtkCellRendererToggle *cr, char *pszPath, gpointer user_data)
  {
  gboolean bImport = TRUE ;
  GtkTreePath *tp = NULL ;
  GtkTreeIter itr ;
  GtkTreeModel *tm = g_object_get_data (G_OBJECT (cr), "model") ;

  if (NULL == tm) return ;
  if (NULL == (tp = gtk_tree_path_new_from_string (pszPath))) return ;

  gtk_tree_model_get_iter (tm, &itr, tp) ;
  gtk_tree_model_get (tm, &itr, LAYER_MAPPING_MODEL_COLUMN_IMPORT, &bImport, -1) ;
  bImport = !bImport ;
  gtk_list_store_set (GTK_LIST_STORE (tm), &itr, LAYER_MAPPING_MODEL_COLUMN_IMPORT, bImport, -1) ;

  gtk_tree_path_free (tp) ;
  }

void dest_layer_changed (GtkCellRendererText *cr, char *pszPath, QCADLayer *new_layer, gpointer data)
  {
  QCADLayer *layer = NULL ;
  GtkTreePath *tp = NULL ;
  GtkTreeIter itr ;
  GtkTreeModel *tm = g_object_get_data (G_OBJECT (cr), "model") ;

  if (NULL == tm) return ;
  if (NULL == (tp = gtk_tree_path_new_from_string (pszPath))) return ;

  if (gtk_tree_model_get_iter_first (tm, &itr))
    do
      {
      gtk_tree_model_get (tm, &itr, LAYER_MAPPING_MODEL_COLUMN_DEST, &layer, -1) ;
      if (layer == new_layer)
        {
        gtk_list_store_set (GTK_LIST_STORE (tm), &itr, LAYER_MAPPING_MODEL_COLUMN_DEST, NULL, -1) ;
        break ;
        }
      }
    while (gtk_tree_model_iter_next (tm, &itr)) ;

  gtk_tree_model_get_iter (tm, &itr, tp) ;
  gtk_list_store_set (GTK_LIST_STORE (tm), &itr, LAYER_MAPPING_MODEL_COLUMN_DEST, new_layer, -1) ;

  gtk_tree_path_free (tp) ;
  }

static void create_layer_mapping_dialog (layer_mapping_D *dialog)
  {
//  GtkWidget *lbl = NULL;
  GtkTreeViewColumn *col = NULL ;
  GtkCellRenderer *cr = NULL ;
  GtkWidget *lbl = NULL ;

  // create the dialog itself
  dialog->dlgLayerMapping = gtk_dialog_new ();
  gtk_window_set_title(GTK_WINDOW (dialog->dlgLayerMapping), _("Map Layers"));
  gtk_window_set_resizable (GTK_WINDOW (dialog->dlgLayerMapping), TRUE);

  lbl = gtk_label_new (_("Click on any selected destination layer to change it:")) ;
  gtk_widget_show (lbl) ;
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog->dlgLayerMapping)->vbox), lbl, TRUE, TRUE, 2);
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5) ;
  gtk_label_set_line_wrap (GTK_LABEL (lbl), TRUE) ;

  // create the scroll window and add to dialog
  dialog->sw = gtk_scrolled_window_new (
  	GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 2, 1, 1, 1)),
  	GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 2, 1, 1, 1)));
  gtk_widget_show (dialog->sw);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog->dlgLayerMapping)->vbox), dialog->sw, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->sw), 2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (dialog->sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (dialog->sw), GTK_SHADOW_IN) ;

  dialog->tv = gtk_tree_view_new () ;
  gtk_widget_show (dialog->tv) ;
  gtk_container_add (GTK_CONTAINER (dialog->sw), dialog->tv) ;
  gtk_tree_view_append_column (GTK_TREE_VIEW (dialog->tv), col = gtk_tree_view_column_new ()) ;
  gtk_tree_view_column_set_title (GTK_TREE_VIEW_COLUMN (col), _("Import")) ;
  gtk_tree_view_column_pack_start (col, cr = dialog->crImport = gtk_cell_renderer_toggle_new (), FALSE) ;
  gtk_tree_view_column_add_attribute (col, cr, "active", LAYER_MAPPING_MODEL_COLUMN_IMPORT) ;

  g_signal_connect (G_OBJECT (cr), "toggled", (GCallback)import_layer_toggled, NULL) ;

  gtk_tree_view_append_column (GTK_TREE_VIEW (dialog->tv), col = gtk_tree_view_column_new ()) ;
  gtk_tree_view_column_set_title (GTK_TREE_VIEW_COLUMN (col), _("Source Layer")) ;
  gtk_tree_view_column_pack_start (col, cr = gtk_cell_renderer_pixbuf_new (), FALSE) ;
#if (GTK_MINOR_VERSION >= 4)
  gtk_tree_view_column_add_attribute (col, cr, "sensitive", LAYER_MAPPING_MODEL_COLUMN_IMPORT) ;
#endif
  gtk_tree_view_column_add_attribute (col, cr, "stock-id", LAYER_MODEL_COLUMN_ICON) ;
  gtk_tree_view_column_pack_start (col, cr = qcad_cell_renderer_layer_list_new (), FALSE) ;
#if (GTK_MINOR_VERSION >= 4)
  gtk_tree_view_column_add_attribute (col, cr, "sensitive", LAYER_MAPPING_MODEL_COLUMN_IMPORT) ;
#endif
  gtk_tree_view_column_add_attribute (col, cr, "layer", LAYER_MODEL_COLUMN_LAYER) ;
  gtk_tree_view_append_column (GTK_TREE_VIEW (dialog->tv), col = gtk_tree_view_column_new ()) ;
  gtk_tree_view_column_set_title (GTK_TREE_VIEW_COLUMN (col), _("Destination Layer")) ;
  gtk_tree_view_column_pack_start (col, cr = dialog->crDst = qcad_cell_renderer_layer_list_new (), FALSE) ;
#if (GTK_MINOR_VERSION >= 4)
  gtk_tree_view_column_add_attribute (col, cr, "sensitive", LAYER_MAPPING_MODEL_COLUMN_IMPORT) ;
#endif
  gtk_tree_view_column_add_attribute (col, cr, "layer", LAYER_MAPPING_MODEL_COLUMN_DEST) ;
  gtk_tree_view_column_add_attribute (col, cr, "template", LAYER_MODEL_COLUMN_LAYER) ;
  g_object_set (G_OBJECT (cr), "editable", TRUE, NULL) ;
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (dialog->tv), TRUE) ;

  g_signal_connect (G_OBJECT (dialog->crDst), "layer-changed", (GCallback)dest_layer_changed, NULL) ;

  // add OK and Cancel buttons
  gtk_dialog_add_button (GTK_DIALOG (dialog->dlgLayerMapping), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
  gtk_dialog_add_button (GTK_DIALOG (dialog->dlgLayerMapping), GTK_STOCK_OK,     GTK_RESPONSE_OK);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->dlgLayerMapping), GTK_RESPONSE_OK);
  }
