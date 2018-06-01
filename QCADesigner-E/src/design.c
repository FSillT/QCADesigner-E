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
// The design. This file defines functions related to   //
// manipulating designs, selections and, for each       //
// design, its corresponding bus layout.                //
//                                                      //
//////////////////////////////////////////////////////////

#include <inttypes.h>
#include <stdint.h>
#if UINTPTR_MAX!=4294967295U
#define PTRDISPLAYWIDTH "16"
#else
#define PTRDISPLAYWIDTH "8"
#endif

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "objects/object_helpers.h"
#include "objects/QCADDOContainer.h"
#include "objects/QCADCompoundDO.h"
#include "fileio_helpers.h"
#include "custom_widgets.h"
#include "support.h"
#include "design.h"
#ifdef GTK_GUI
  #include "qcadstock.h"
#endif /* def GTK_GUI */

//#define DBG_OVERLAP
#define DBG_D(s)

extern GdkColor clrBlack ;
extern GdkColor clrOrange ;

//#define DBG_WEAK_REFS
#ifdef DBG_WEAK_REFS
typedef struct
  {
  QCADDesignObject **place_to_nullify ;
  int idx ;
  EXP_ARRAY *object_array ;
  } DESIGN_OBJECT_ARRAY_WEAK_REF_STRUCT ;
#endif /* def DBG_WEAK_REFS */

static void design_bus_layout_remove_cell (DESIGN *design, QCADCell *cell, QCADCellFunction cell_function) ;
static void design_rebuild_io_lists (DESIGN *design) ;
#ifdef STDIO_FILEIO
static gboolean design_bus_layout_bus_unserialize (EXP_ARRAY *buses, FILE *pfile) ;
static gboolean design_bus_layout_bus_data_unserialize (EXP_ARRAY *cell_indices, FILE *pfile) ;
#endif /* def STDIO_FILEIO */
static EXP_ARRAY *design_select_single_object (QCADDesignObject *obj, EXP_ARRAY *ar) ;
#ifdef DBG_WEAK_REFS
static void design_selection_object_array_member_destroyed (gpointer data, gpointer ex_obj) ;
#endif /* def DBG_WEAK_REFS */
static void qcad_layer_design_object_added (QCADLayer *layer, QCADDesignObject *obj, gpointer data) ;
static void qcad_layer_design_object_removed (QCADLayer *layer, QCADDesignObject *obj, gpointer data) ;
static void cell_function_changed (QCADCell *cell, DESIGN *design) ;
static void design_bus_layout_next_unassigned_cell (BUS_LAYOUT_ITER *bus_layout_iter) ;

#ifdef GTK_GUI
char *layer_pixmap_stock_id[LAYER_TYPE_LAST_TYPE] =
  {
  QCAD_STOCK_SUBSTRATE_LAYER,
  QCAD_STOCK_CELL_LAYER,
  QCAD_STOCK_CLOCKING_LAYER,
  QCAD_STOCK_DRAWING_LAYER
  } ;
#endif /* def GTK_GUI */

// ASSUMPTION: only active layers can contain selections.

DESIGN *design_new (QCADSubstrate **psubs)
  {
  QCADLayer *layer = NULL ;
  DESIGN *design = NULL ;

  design = g_malloc0 (sizeof (DESIGN)) ;

  // Initialize the design

  layer = qcad_layer_new (LAYER_TYPE_DRAWING, LAYER_STATUS_VISIBLE, _("Drawing Layer")) ;
  design_layer_add (design, layer) ;
  design->lstCurrentLayer =
  design->lstLastLayer = design->lstLayers ;

  layer = qcad_layer_new (LAYER_TYPE_SUBSTRATE, LAYER_STATUS_VISIBLE, _("Substrate")) ;
  design_layer_add (design, layer) ;
  qcad_do_container_add (QCAD_DO_CONTAINER (layer), QCAD_DESIGN_OBJECT ((*psubs) = QCAD_SUBSTRATE (qcad_substrate_new (0.0, 0.0, 6000.0, 3000.0, 20.0)))) ;
  g_object_unref (G_OBJECT (*psubs)) ;

  layer = qcad_layer_new (LAYER_TYPE_CELLS, LAYER_STATUS_ACTIVE, _("Main Cell Layer")) ;
  design_layer_add (design, layer) ;

  design->bus_layout = design_bus_layout_new () ;

  design_rebuild_io_lists (design) ;

  return design ;
  }

DESIGN *design_copy (DESIGN *design)
  {
  DESIGN *new_design = NULL ;
  GList *llItr = NULL ;
  GList *llItrObj = NULL ;
  QCADLayer *new_layer = NULL ;
  BUS bus = {NULL, 0, NULL}, *src_bus = NULL ;
  EXP_ARRAY *cell_list = NULL ;
  int Nix, Nix1 ;

  if (NULL == design) return NULL ;

  new_design = g_malloc0 (sizeof (DESIGN)) ;
  new_design->lstLayers =
  new_design->lstLastLayer =
  new_design->lstCurrentLayer = NULL ;

  new_design->bus_layout = design_bus_layout_new () ;

  for (llItr = design->lstLastLayer ; llItr != NULL ; llItr = llItr->prev)
    {
    new_design->lstLayers = g_list_prepend (new_design->lstLayers,
      new_layer = QCAD_LAYER (qcad_design_object_new_from_object (QCAD_DESIGN_OBJECT (llItr->data)))) ;

    g_signal_connect (G_OBJECT (new_layer), "added",   (GCallback)qcad_layer_design_object_added,   new_design) ;
    g_signal_connect (G_OBJECT (new_layer), "removed", (GCallback)qcad_layer_design_object_removed, new_design) ;

    // Artificially call the "added" callback for the new layer
    for (llItrObj = new_layer->lstObjs ; llItrObj != NULL ; llItrObj = llItrObj->next)
      {
      if (QCAD_IS_CELL (llItrObj->data))
        g_signal_connect (G_OBJECT (llItrObj->data), "cell-function-changed", (GCallback)cell_function_changed, new_design) ;
      qcad_layer_design_object_added (new_layer, QCAD_DESIGN_OBJECT (llItrObj->data), new_design) ;
      }

    if (NULL == new_design->lstLastLayer)
      new_design->lstLastLayer = new_design->lstLayers ;
    if (design->lstCurrentLayer == llItr)
      new_design->lstCurrentLayer = new_design->lstLayers ;
    }

  for (Nix = 0 ; Nix < design->bus_layout->buses->icUsed ; Nix++)
    {
    src_bus = &(exp_array_index_1d (design->bus_layout->buses, BUS, Nix)) ;
    bus.pszName = g_strdup (src_bus->pszName) ;
    bus.bus_function = src_bus->bus_function ;
    bus.cell_indices = exp_array_copy (src_bus->cell_indices) ;
    exp_array_insert_vals (new_design->bus_layout->buses, &bus, 1, 1, -1) ;

    // Flag all the cells in the appropriate I/O list as "used-in-a-bus"
    cell_list = (QCAD_CELL_INPUT == bus.bus_function ? new_design->bus_layout->inputs : new_design->bus_layout->outputs) ;
    for (Nix1 = 0 ; Nix1 < bus.cell_indices->icUsed ; Nix1++)
      exp_array_index_1d (cell_list, BUS_LAYOUT_CELL, exp_array_index_1d (bus.cell_indices, int, Nix1)).bIsInBus = TRUE ;
    }

  return new_design ;
  }

// This function always returns NULL
DESIGN *design_destroy (DESIGN *design)
  {
  GList *lstLayer = NULL ;

  if (NULL == design) return NULL ;

  for (lstLayer = design->lstLayers ; lstLayer != NULL ; lstLayer = lstLayer->next)
    g_object_unref ((QCAD_LAYER (lstLayer->data))) ;

  design->lstLayers =
  design->lstCurrentLayer =
  design->lstLastLayer = NULL ;

  design->bus_layout = design_bus_layout_free (design->bus_layout) ;

  g_free (design) ;

  return NULL ;
  }

void design_layer_add (DESIGN *design, QCADLayer *layer)
  {
  GList *llItr = NULL ;

  design->lstLayers = g_list_prepend (design->lstLayers, layer) ;
  g_signal_connect (G_OBJECT (layer), "added", (GCallback)qcad_layer_design_object_added, design) ;
  g_signal_connect (G_OBJECT (layer), "removed", (GCallback)qcad_layer_design_object_removed, design) ;

  // If, upon adding a new layer, said layer already contains objects, then run the "added" callback for
  // each object
  for (llItr = layer->lstObjs ; llItr != NULL ; llItr = llItr->next)
    if (NULL != llItr->data)
      qcad_layer_design_object_added (layer, llItr->data, design) ;
  }

// Removes the layer and returns the new current layer
QCADLayer *design_layer_remove (DESIGN *design, QCADLayer *layer)
  {
  GList *llLayer = NULL, *llRet = NULL, *llCellItr = NULL ;

  if (NULL == design || NULL == layer) return NULL ;

  llLayer = g_list_find (design->lstLayers, layer) ;
  llRet = design->lstCurrentLayer ;

  // Let's hope lstLastLayer and lstLayers are properly maintained
  if (llLayer == design->lstLastLayer && llLayer == design->lstLayers)
    {
    fprintf (stderr, "ERROR: Cannot remove the last layer of a design !\n") ;
    return NULL ;
    }

  if (LAYER_TYPE_CELLS == layer->type)
    for (llCellItr = layer->lstObjs ; llCellItr != NULL ; llCellItr = llCellItr->next)
      if (QCAD_CELL_INPUT  == QCAD_CELL (llCellItr->data)->cell_function ||
          QCAD_CELL_OUTPUT == QCAD_CELL (llCellItr->data)->cell_function)
        design_bus_layout_remove_cell (design, QCAD_CELL (llCellItr->data), QCAD_CELL (llCellItr->data)->cell_function) ;

  if (llLayer == design->lstLastLayer)
    design->lstLastLayer = design->lstLastLayer->prev ;

  if (llLayer == design->lstCurrentLayer)
    design->lstCurrentLayer =
    llRet =
      NULL == design->lstCurrentLayer->prev ?
        design->lstCurrentLayer->next :
        design->lstCurrentLayer->prev ;

  design->lstLayers = g_list_delete_link (design->lstLayers, llLayer) ;

  g_object_unref (layer) ;

  return (QCAD_LAYER (llRet->data)) ;
  }

QCADDesignObject *design_hit_test (DESIGN *design, int x, int y)
  {
  GList *lstLayer = NULL ;
  QCADDesignObject *hit_object = NULL ;

  for (lstLayer = design->lstLayers ; NULL != lstLayer ; lstLayer = lstLayer->next)
    if (NULL != (hit_object = qcad_design_object_hit_test (QCAD_DESIGN_OBJECT (lstLayer->data), x, y)))
      return QCAD_DESIGN_OBJECT (hit_object) ;
  return NULL ;
  }

#ifdef GTK_GUI
void design_draw (DESIGN *design, GdkDrawable *dst, GdkFunction rop, WorldRectangle *rc, int flags)
  {
  GList *llLayer = NULL ;

  for (llLayer = design->lstLayers ; NULL != llLayer ; llLayer = llLayer->next)
    if (LAYER_STATUS_VISIBLE == QCAD_LAYER (llLayer->data)->status ||
        LAYER_STATUS_ACTIVE  == QCAD_LAYER (llLayer->data)->status)
    qcad_layer_draw (QCAD_LAYER (llLayer->data), dst, rop, rc, flags) ;
  }

GtkListStore *design_layer_list_store_new (DESIGN *design, int icExtraColumns, ...)
  {
  GList *llItr = NULL ;
  int Nix ;
  GType type = 0 ;
  GtkListStore *ls = NULL ;
  EXP_ARRAY *ar = NULL ;
  GtkTreeIter itr ;

  if (NULL == design) return NULL ;

  ar = exp_array_new (sizeof (GType), 1) ;

  type = G_TYPE_STRING  ; exp_array_insert_vals (ar, &type, 1, 1, -1) ;
  type = G_TYPE_STRING  ; exp_array_insert_vals (ar, &type, 1, 1, -1) ;
  type = G_TYPE_POINTER ; exp_array_insert_vals (ar, &type, 1, 1, -1) ;

  if (icExtraColumns > 0)
    {
    va_list va ;

    va_start (va, icExtraColumns) ;
    for (Nix = 0 ; Nix < icExtraColumns ; Nix++)
      {type = va_arg (va, GType) ; exp_array_insert_vals (ar, &type, 1, 1, -1) ;}
    va_end (va) ;
    }

  ls = gtk_list_store_newv (ar->icUsed, (GType *)(ar->data)) ;

  exp_array_free (ar) ;

  for (llItr = design->lstLastLayer ; llItr != NULL ; llItr = llItr->prev)
    {
    gtk_list_store_append (ls, &itr) ;
    gtk_list_store_set (ls, &itr,
      LAYER_MODEL_COLUMN_ICON, layer_pixmap_stock_id[(QCAD_LAYER (llItr->data))->type],
      LAYER_MODEL_COLUMN_NAME, (QCAD_LAYER (llItr->data))->pszDescription,
      LAYER_MODEL_COLUMN_LAYER, llItr->data, -1) ;
    }

  return ls ;
  }

GtkTreeStore *design_bus_layout_tree_store_new (BUS_LAYOUT *bus_layout, int row_types, int icExtraColumns, ...)
  {
  GtkTreeStore *ts = NULL ;
  EXP_ARRAY *ar = NULL ;
  GType type = 0 ;
  va_list va ;
  BUS *bus = NULL ;
  int bus_type = -1 ;
  int cell_type = -1 ;
  GtkTreeIter gtiBus, gtiCell ;
  int Nix = -1, Nix1 = -1, idx = -1 ;
  EXP_ARRAY *cell_list = NULL ;
  char *pszStockCell = NULL, *pszStockBus = NULL ;
  QCADCell *cell = NULL ;

  if (NULL == bus_layout) return NULL ;

  ar = exp_array_new (sizeof (GType), 1) ;

  type = G_TYPE_STRING  ; exp_array_insert_vals (ar, &type, 1, 1, -1) ;
  type = G_TYPE_STRING  ; exp_array_insert_vals (ar, &type, 1, 1, -1) ;
  type = G_TYPE_INT     ; exp_array_insert_vals (ar, &type, 1, 1, -1) ;
  type = G_TYPE_INT     ; exp_array_insert_vals (ar, &type, 1, 1, -1) ;
  type = G_TYPE_POINTER ; exp_array_insert_vals (ar, &type, 1, 1, -1) ;

  if (icExtraColumns > 0)
    {
    va_start (va, icExtraColumns) ;
    for (Nix = 0 ; Nix < icExtraColumns ; Nix++)
      {type = va_arg (va, GType) ; exp_array_insert_vals (ar, &type, 1, 1, -1) ;}
    va_end (va) ;
    }

  ts = gtk_tree_store_newv (ar->icUsed, (GType *)(ar->data)) ;

  exp_array_free (ar) ;

// Add all the buses
  for (Nix = 0 ; Nix < bus_layout->buses->icUsed ; Nix++)
    {
    bus = &(exp_array_index_1d (bus_layout->buses, BUS, Nix)) ;
    if (bus->bus_function & row_types)
      {
      if (QCAD_CELL_INPUT == bus->bus_function)
        {
        bus_type = ROW_TYPE_BUS_INPUT ;
        cell_type = ROW_TYPE_CELL_INPUT ;
        cell_list = bus_layout->inputs ;
        pszStockCell = QCAD_STOCK_CELL_INPUT ;
        pszStockBus = QCAD_STOCK_BUS_INPUT ;
        cell = NULL ;
        }
      else
        {
        bus_type = ROW_TYPE_BUS_OUTPUT ;
        cell_type = ROW_TYPE_CELL_OUTPUT ;
        cell_list = bus_layout->outputs ;
        pszStockCell = QCAD_STOCK_CELL_OUTPUT ;
        pszStockBus = QCAD_STOCK_BUS_OUTPUT ;
        cell = NULL ;
        }
      gtk_tree_store_append (ts, &gtiBus, NULL) ;
      gtk_tree_store_set (ts, &gtiBus,
        BUS_LAYOUT_MODEL_COLUMN_ICON, pszStockBus,
        BUS_LAYOUT_MODEL_COLUMN_NAME, bus->pszName,
        BUS_LAYOUT_MODEL_COLUMN_TYPE, bus_type,
        BUS_LAYOUT_MODEL_COLUMN_INDEX, Nix,
        BUS_LAYOUT_MODEL_COLUMN_CELL, cell, -1) ;

      for (Nix1 = 0 ; Nix1 < bus->cell_indices->icUsed ; Nix1++)
        {
        idx = exp_array_index_1d (bus->cell_indices, int, Nix1) ;
        gtk_tree_store_append (ts, &gtiCell, &gtiBus) ;
        gtk_tree_store_set (ts, &gtiCell,
          BUS_LAYOUT_MODEL_COLUMN_ICON, pszStockCell,
          BUS_LAYOUT_MODEL_COLUMN_NAME, qcad_cell_get_label (QCAD_CELL (exp_array_index_1d (cell_list, BUS_LAYOUT_CELL, idx).cell)),
          BUS_LAYOUT_MODEL_COLUMN_TYPE, cell_type,
          BUS_LAYOUT_MODEL_COLUMN_INDEX, idx,
          BUS_LAYOUT_MODEL_COLUMN_CELL, exp_array_index_1d ((QCAD_CELL_INPUT == bus->bus_function ? bus_layout->inputs : bus_layout->outputs), BUS_LAYOUT_CELL, idx).cell,
          -1) ;
        }
      }
    }

// Add whatever remaining free inputs and outputs to their respective lists
  if (row_types & ROW_TYPE_INPUT)
    for (Nix = 0 ; Nix < bus_layout->inputs->icUsed ; Nix++)
      if (!(exp_array_index_1d (bus_layout->inputs, BUS_LAYOUT_CELL, Nix).bIsInBus))
        {
        cell = QCAD_CELL (exp_array_index_1d (bus_layout->inputs, BUS_LAYOUT_CELL, Nix).cell) ;
        gtk_tree_store_append (ts, &gtiCell, NULL) ;
        gtk_tree_store_set (ts, &gtiCell,
          BUS_LAYOUT_MODEL_COLUMN_ICON, QCAD_STOCK_CELL_INPUT,
          BUS_LAYOUT_MODEL_COLUMN_NAME, qcad_cell_get_label (cell),
          BUS_LAYOUT_MODEL_COLUMN_TYPE, ROW_TYPE_CELL_INPUT,
          BUS_LAYOUT_MODEL_COLUMN_INDEX, Nix, 
          BUS_LAYOUT_MODEL_COLUMN_CELL, cell, -1) ;
        }

  if (row_types & ROW_TYPE_OUTPUT)
    for (Nix = 0 ; Nix < bus_layout->outputs->icUsed ; Nix++)
      if (!(exp_array_index_1d (bus_layout->outputs, BUS_LAYOUT_CELL, Nix).bIsInBus))
        {
        cell = QCAD_CELL (exp_array_index_1d (bus_layout->outputs, BUS_LAYOUT_CELL, Nix).cell) ;
        gtk_tree_store_append (ts, &gtiCell, NULL) ;
        gtk_tree_store_set (ts, &gtiCell,
          BUS_LAYOUT_MODEL_COLUMN_ICON, QCAD_STOCK_CELL_OUTPUT,
          BUS_LAYOUT_MODEL_COLUMN_NAME, qcad_cell_get_label (cell),
          BUS_LAYOUT_MODEL_COLUMN_TYPE, ROW_TYPE_CELL_OUTPUT,
          BUS_LAYOUT_MODEL_COLUMN_INDEX, Nix, 
          BUS_LAYOUT_MODEL_COLUMN_CELL, cell, -1) ;
        }

  return ts ;
  }

// Used for copying selections
QCADDesignObject *design_selection_create_from_selection (DESIGN *design, GdkWindow *window, GdkFunction rop)
  {
  GList *lstLayer = NULL, *lstSelObj = NULL ;
  QCADLayer *layer = NULL ;
  QCADDesignObject *obj = NULL, *ret = NULL ;

  for (lstLayer = design->lstLayers ; lstLayer != NULL ; lstLayer = lstLayer->next)
    if (LAYER_STATUS_ACTIVE == (layer = (QCAD_LAYER (lstLayer->data)))->status)
      if (NULL != layer->lstSelObjs)
        {
        qcad_layer_selection_create_from_selection (layer) ;
        if (NULL != layer->lstSelObjs && NULL == ret)
          ret = QCAD_DESIGN_OBJECT (layer->lstSelObjs->data) ;
        for (lstSelObj = layer->lstSelObjs ; lstSelObj != NULL ; lstSelObj = lstSelObj->next)
          if (QCAD_IS_CELL (obj = QCAD_DESIGN_OBJECT (lstSelObj->data)))
            g_signal_connect (G_OBJECT (obj), "cell-function-changed", (GCallback)cell_function_changed, design) ;
        }

  design_rebuild_io_lists (design) ;

  return ret ;
  }

EXP_ARRAY *design_selection_subtract_window (DESIGN *design, GdkDrawable *dst, GdkFunction rop, WorldRectangle *rcWorld)
  {
  GList *lstLayer = NULL ;
  QCADLayer *layer = NULL ;
  EXP_ARRAY *ar = NULL ;

  for (lstLayer = design->lstLayers ; lstLayer != NULL ; lstLayer = lstLayer->next)
    if (LAYER_STATUS_ACTIVE == ((layer = (QCAD_LAYER (lstLayer->data))))->status)
      ar = qcad_layer_selection_subtract_window (layer, dst, rop, rcWorld, ar) ;
  return design_selection_object_array_add_weak_pointers (ar) ;
  }

EXP_ARRAY *design_selection_release (DESIGN *design, GdkDrawable *dst, GdkFunction rop)
  {
  GList *lstLayer = NULL ;
  QCADLayer *layer = NULL ;
  EXP_ARRAY *ar = NULL ;

  for (lstLayer = design->lstLayers ; lstLayer != NULL ; lstLayer = lstLayer->next)
    if (LAYER_STATUS_ACTIVE == ((layer = (QCAD_LAYER (lstLayer->data))))->status)
      ar = qcad_layer_selection_release (layer, dst, rop, ar) ;

  return design_selection_object_array_add_weak_pointers (ar) ;
  }
#endif /* def GTK_GUI */

//Calculates bounding box around design //
gboolean design_get_extents (DESIGN *design, WorldRectangle *extents, gboolean bSelection)
  {
  WorldRectangle ext = {0.0} ;
  GList *lstLayer = NULL ;
  gboolean bHaveBaseline = FALSE ;
  double x, y ;

  extents->xWorld =
  extents->yWorld =
  extents->cxWorld =
  extents->cyWorld = 0.0 ;
  for (lstLayer = design->lstLayers ; lstLayer != NULL ; lstLayer = lstLayer->next)
    if (LAYER_STATUS_VISIBLE == (QCAD_LAYER (lstLayer->data))->status ||
        LAYER_STATUS_ACTIVE  == (QCAD_LAYER (lstLayer->data))->status)
      {
      if (bHaveBaseline)
        {
        if (qcad_layer_get_extents ((QCAD_LAYER (lstLayer->data)), &ext, bSelection))
          {
          x = extents->xWorld ;
          y = extents->yWorld ;
          extents->xWorld = MIN (extents->xWorld, ext.xWorld) ;
          extents->yWorld = MIN (extents->yWorld, ext.yWorld) ;
          extents->cxWorld = MAX (extents->cxWorld + x, ext.cxWorld + ext.xWorld) - extents->xWorld ;
          extents->cyWorld = MAX (extents->cyWorld + y, ext.cyWorld + ext.yWorld) - extents->yWorld ;
          }
        }
      else
      if ((bHaveBaseline = qcad_layer_get_extents (QCAD_LAYER (lstLayer->data), extents, bSelection)))
        memcpy (&ext, extents, sizeof (WorldRectangle)) ;
      }
  return bHaveBaseline ;
  }//get_extents

//SELECTIONS:
//Creating and destroying selections

EXP_ARRAY *design_selection_get_object_array (DESIGN *design)
  {
  GList *llItr = NULL ;
  QCADLayer *layer = NULL ;
  EXP_ARRAY *ar = NULL ;

  for (llItr = design->lstLayers ; llItr != NULL ; llItr = llItr->next)
    if (LAYER_STATUS_ACTIVE == (layer = QCAD_LAYER (llItr->data))->status)
      ar = qcad_layer_selection_get_object_array (layer, ar) ;

  return design_selection_object_array_add_weak_pointers (ar) ;
  }

EXP_ARRAY *design_selection_create_from_window (DESIGN *design, WorldRectangle *rcWorld)
  {
  GList *lstLayer = NULL, *lstObj = NULL ;
  QCADLayer *layer = NULL ;
  QCADDesignObject *obj = NULL, *obj_child = NULL ;
  EXP_ARRAY *ar = NULL ;

  for (lstLayer = design->lstLayers ; lstLayer != NULL ; lstLayer = lstLayer->next)
    {
    // This function is not responsible for destroying an existing selection.
    if (LAYER_STATUS_ACTIVE == (layer = QCAD_LAYER (lstLayer->data))->status)
      for (lstObj = layer->lstObjs ; lstObj != NULL ; lstObj = lstObj->next)
        if (NULL != lstObj->data)
          {
          if (qcad_design_object_select_test (obj = QCAD_DESIGN_OBJECT (lstObj->data), rcWorld, SELECTION_CONTAINMENT))
            ar = design_select_single_object (obj, ar) ;
          else
          if (QCAD_IS_COMPOUND_DO (obj))
            for (obj_child = qcad_compound_do_first (QCAD_COMPOUND_DO (obj)) ;; obj_child = qcad_compound_do_next (QCAD_COMPOUND_DO (obj)))
              {
              if (NULL != obj_child)
                if (qcad_design_object_select_test (obj_child, rcWorld, SELECTION_CONTAINMENT))
                  ar = design_select_single_object (obj_child, ar) ;
              if (qcad_compound_do_last (QCAD_COMPOUND_DO (obj))) break ;
              }
          }
    }

  return design_selection_object_array_add_weak_pointers (ar) ;
  }

static EXP_ARRAY *design_select_single_object (QCADDesignObject *obj, EXP_ARRAY *ar)
  {
  if (NULL == ar)
    ar = exp_array_new (sizeof (QCADDesignObject *), 1) ;
  qcad_design_object_set_selected (QCAD_DESIGN_OBJECT (obj), TRUE) ;
  exp_array_insert_vals (ar, &obj, 1, 1, -1) ;

  return ar ;
  }

EXP_ARRAY *design_selection_add_window (DESIGN *design, WorldRectangle *rcWorld)
  {
  GList *lstLayer = NULL, *lstObj = NULL ;
  QCADDesignObject *obj = NULL, *obj_child = NULL ;
  QCADLayer *layer = NULL ;
  EXP_ARRAY *ar = NULL ;

  for (lstLayer = design->lstLayers ; lstLayer != NULL ; lstLayer = lstLayer->next)
    if (LAYER_STATUS_ACTIVE == ((layer = (QCAD_LAYER (lstLayer->data))))->status)

      for (lstObj = layer->lstObjs ; lstObj != NULL ; lstObj = lstObj->next)
        if (NULL != lstObj->data)
          if (!((obj = QCAD_DESIGN_OBJECT (lstObj->data))->bSelected))
            {
            if (qcad_design_object_select_test (obj, rcWorld, SELECTION_CONTAINMENT))
              ar = design_select_single_object (obj, ar) ;
            else
            if (QCAD_IS_COMPOUND_DO (obj))
              for (obj_child = qcad_compound_do_first (QCAD_COMPOUND_DO (obj)) ;; obj_child = qcad_compound_do_next (QCAD_COMPOUND_DO (obj)))
                {
                if (NULL != obj_child)
                  if (!obj_child->bSelected)
                    if (qcad_design_object_select_test (obj_child, rcWorld, SELECTION_CONTAINMENT))
                      ar = design_select_single_object (obj_child, ar) ;
                if (qcad_compound_do_last (QCAD_COMPOUND_DO (obj))) break ;
                }
            }

  return design_selection_object_array_add_weak_pointers (ar) ;
  }

EXP_ARRAY *design_selection_create_object_array (DESIGN *design)
  {
  EXP_ARRAY *ar = NULL ;
  GList *llItrLayer = NULL ;
  GList *llItrSelObj = NULL ;
  QCADDesignObject *obj = NULL ;

  if (NULL == design) return NULL ;

  for (llItrLayer = design->lstLayers ; llItrLayer != NULL ; llItrLayer = llItrLayer->next)
    if (NULL != llItrLayer->data)
      for (llItrSelObj = QCAD_LAYER (llItrLayer->data)->lstSelObjs ; llItrSelObj != NULL ; llItrSelObj = llItrSelObj->next)
        if (NULL != (obj = (QCADDesignObject *)llItrSelObj->data))
          if (QCAD_IS_DESIGN_OBJECT (obj))
            {
            if (NULL == ar)
              ar = exp_array_new (sizeof (QCADDesignObject *), 1) ;
            exp_array_insert_vals (ar, &obj, 1, 1, -1) ;
            }

  return design_selection_object_array_add_weak_pointers (ar) ;
  }

EXP_ARRAY *design_selection_destroy (DESIGN *design)
  {
  EXP_ARRAY *ar = NULL ;
  int Nix ;
  QCADDesignObject *obj = NULL ;

  if (NULL == design) return NULL ;

  if (NULL == (ar = design_selection_create_object_array (design))) return NULL ;

  for (Nix = 0 ; Nix < ar->icUsed ; Nix++)
    {
    // Ref the object so removing it from the layer doesn't destroy it.
    // After all, the array we return should contain something other than garbage.

    g_object_ref (obj = exp_array_index_1d (ar, QCADDesignObject *, Nix)) ;
    qcad_do_container_remove (QCAD_DO_CONTAINER (qcad_layer_from_object (obj)), QCAD_DESIGN_OBJECT (obj)) ;
    }

  return ar ;
  }

void design_selection_object_array_free (EXP_ARRAY *ar)
  {
  int Nix ;
  QCADDesignObject **pobj = NULL ;

  for (Nix = 0 ; Nix < ar->icUsed ; Nix++)
    if (NULL != (*(pobj = &exp_array_index_1d (ar, QCADDesignObject *, Nix))))
      g_object_remove_weak_pointer (G_OBJECT (*pobj), (gpointer *)pobj) ;

  exp_array_free (ar) ;
  }

#ifdef STDIO_FILEIO
//design selection IO
void design_selection_serialize (DESIGN *design, FILE *pfile)
  {
  GList *lstLayer = NULL ;

  fprintf (pfile, "[TYPE:DESIGN]\n") ;
  for (lstLayer = design->lstLastLayer ; lstLayer != NULL ; lstLayer = lstLayer->prev)
    qcad_layer_selection_serialize (QCAD_LAYER (lstLayer->data), pfile) ;
  fprintf (pfile, "[#TYPE:DESIGN]\n") ;
  }
#endif /* def STDIO_FILEIO */

GList *design_selection_get_type_list (DESIGN *design)
  {
  GList *lstRet = NULL ;
  GList *llItrLayer = NULL, *llItrSelObj =NULL ;
  for (llItrLayer = design->lstLayers ; llItrLayer != NULL ; llItrLayer = llItrLayer->next)
    for (llItrSelObj = (QCAD_LAYER (llItrLayer->data))->lstSelObjs ; llItrSelObj != NULL ; llItrSelObj = llItrSelObj->next)
      if (NULL != llItrSelObj->data)
        lstRet = qcad_design_object_add_types (QCAD_DESIGN_OBJECT (llItrSelObj->data), lstRet) ;

  return lstRet ;
  }

QCADDesignObject *design_selection_transform (DESIGN *design, double m11, double m12, double m21, double m22)
  {
  GList *llItrLayers ;
  GList *llItrSelObj ;
  QCADDesignObject *obj = NULL ;

  for (llItrLayers = design->lstLayers ; llItrLayers != NULL ; llItrLayers = llItrLayers->next)
    for (llItrSelObj = (QCAD_LAYER (llItrLayers->data))->lstSelObjs ; llItrSelObj != NULL ; llItrSelObj = llItrSelObj->next)
      if (NULL != llItrSelObj->data)
        qcad_design_object_transform (obj = QCAD_DESIGN_OBJECT (llItrSelObj->data), m11, m12, m21, m22) ;

  return obj ;
  }

void design_selection_set_cell_display_mode (DESIGN *design, int display_mode)
  {
  GList *lstLayer = NULL, *lstObj = NULL ;
  QCADLayer *layer = NULL ;

  for (lstLayer = design->lstLayers ; NULL != lstLayer ; lstLayer = lstLayer->next)
    {
    layer = (QCAD_LAYER (lstLayer->data)) ;
    if (LAYER_TYPE_CELLS == layer->type)
      for (lstObj = layer->lstSelObjs ; NULL != lstObj ; lstObj = lstObj->next)
        if (NULL != lstObj->data)
          if (QCAD_IS_CELL (lstObj->data))
        	  qcad_cell_set_display_mode (QCAD_CELL (lstObj->data), display_mode) ;
    }
  }

GList *design_selection_get_input_cells (DESIGN *design, int *picCells)
  {
  GList *lstLayer = NULL, *lstObj = NULL, *lstFirstCell = NULL ;
  QCADLayer *layer = NULL ;

  (*picCells) = 0 ;

  for (lstLayer = design->lstLayers ; NULL != lstLayer ; lstLayer = lstLayer->next)
    {
    layer = (QCAD_LAYER (lstLayer->data)) ;
    if (LAYER_TYPE_CELLS == layer->type)
      for (lstObj = layer->lstSelObjs ; NULL != lstObj ; lstObj = lstObj->next)
        if (NULL != lstObj->data)
          if (QCAD_CELL_INPUT == QCAD_CELL (lstObj->data)->cell_function)
            {
            lstFirstCell = g_list_prepend (lstFirstCell, lstObj->data) ;
            (*picCells)++ ;
            }
    }

  return lstFirstCell ;
  }

QCADDesignObject *design_selection_hit_test (DESIGN *design, int x, int y)
  {
  GList *lstLayer = NULL, *lstSelObj = NULL ;
  QCADLayer *layer = NULL ;
  QCADDesignObject *obj = NULL ;

  for (lstLayer = design->lstLayers ; lstLayer != NULL ; lstLayer = lstLayer->next)
    if (LAYER_STATUS_ACTIVE == ((layer = (QCAD_LAYER (lstLayer->data))))->status)
      for (lstSelObj = layer->lstSelObjs ; lstSelObj != NULL ; lstSelObj = lstSelObj->next)
        if (NULL != (obj = qcad_design_object_hit_test (QCAD_DESIGN_OBJECT (lstSelObj->data), x, y)))
          return obj ;
  return NULL ;
  }

void design_selection_move (DESIGN *design, double dxWorld, double dyWorld)
  {
  GList *lstLayer = NULL, *lstSelObj = NULL ;
  QCADLayer *layer = NULL ;

  for (lstLayer = design->lstLayers ; lstLayer != NULL ; lstLayer = lstLayer->next)
    if (LAYER_STATUS_ACTIVE == ((layer = (QCAD_LAYER (lstLayer->data))))->status)
      for (lstSelObj = layer->lstSelObjs ; lstSelObj != NULL ; lstSelObj = lstSelObj->next)
        qcad_design_object_move (lstSelObj->data, dxWorld, dyWorld) ;
  }

//END SELECTIONS

#ifdef STDIO_FILEIO
void design_serialize (DESIGN *design, FILE *pfile)
  {
  GList *lstLayer = NULL ;

  fprintf (pfile, "[TYPE:DESIGN]\n") ;
  for (lstLayer = design->lstLastLayer ; lstLayer != NULL ; lstLayer = lstLayer->prev)
    qcad_design_object_serialize (QCAD_DESIGN_OBJECT (lstLayer->data), pfile) ;
  if (design->bus_layout->buses->icUsed > 0)
    design_bus_layout_serialize (design->bus_layout, pfile) ;
  fprintf (pfile, "[#TYPE:DESIGN]\n") ;
  }

void design_bus_layout_serialize (BUS_LAYOUT *bus_layout, FILE *pfile)
  {
  int Nix, Nix1 ;
  BUS *bus = NULL ;

  fprintf (pfile, "[TYPE:BUS_LAYOUT]\n") ;
  for (Nix = 0 ; Nix < bus_layout->buses->icUsed ; Nix++)
    {
    bus = &(exp_array_index_1d (bus_layout->buses, BUS, Nix)) ;
    if (bus->cell_indices->icUsed > 0)
      {
      fprintf (pfile, "[TYPE:BUS]\n") ;
      fprintf (pfile, "pszName=%s\n", bus->pszName) ;
      fprintf (pfile, "bus_function=%d\n", bus->bus_function) ;
      fprintf (pfile, "[BUS_DATA]\n") ;
      fprintf (pfile, "%d", exp_array_index_1d (bus->cell_indices, int, 0)) ;
      for (Nix1 = 1 ; Nix1 < bus->cell_indices->icUsed ; Nix1++)
        fprintf (pfile, " %d", exp_array_index_1d (bus->cell_indices, int, Nix1)) ;
      fprintf (pfile, "\n") ;
      fprintf (pfile, "[#BUS_DATA]\n") ;
      fprintf (pfile, "[#TYPE:BUS]\n") ;
      }
    }
  fprintf (pfile, "[#TYPE:BUS_LAYOUT]\n") ;
  }

gboolean design_unserialize (DESIGN **pdesign, FILE *pfile)
  {
  QCADLayer *layer = NULL ;
  char *pszLine = NULL ;
  gboolean bEOF = FALSE ;
  GList *llItr = NULL ;

  (*pdesign) = g_malloc0 (sizeof (DESIGN)) ;
  (*pdesign)->lstLayers = NULL ;
  (*pdesign)->lstLastLayer = NULL ;
  (*pdesign)->bus_layout = NULL ;

  if (!SkipPast (pfile, '\0', "[TYPE:DESIGN]", NULL))
    {
    (*pdesign) = design_destroy ((*pdesign)) ;
    return FALSE ;
    }

  while (TRUE)
    {
    //peek the next line
    if ((bEOF = (NULL == (pszLine = ReadLine (pfile, '\0', TRUE))))) break ;
    if (!strcmp (pszLine, "[#TYPE:DESIGN]"))
      {
      g_free (pszLine) ;
      break ;
      }

    if (!strcmp (pszLine, "[TYPE:" QCAD_TYPE_STRING_LAYER "]"))
      {
      set_progress_bar_fraction (get_file_percent (pfile)) ;
      if (NULL != (layer = QCAD_LAYER (qcad_design_object_new_from_stream (pfile))))
        {
        g_signal_connect (G_OBJECT (layer), "added",   (GCallback)qcad_layer_design_object_added,   (*pdesign)) ;
        g_signal_connect (G_OBJECT (layer), "removed", (GCallback)qcad_layer_design_object_removed, (*pdesign)) ;
        (*pdesign)->lstLayers = g_list_prepend ((*pdesign)->lstLayers, layer) ;
        if (NULL == (*pdesign)->lstLastLayer)
          (*pdesign)->lstLastLayer = (*pdesign)->lstLayers ;
        if (LAYER_TYPE_CELLS == layer->type)
          for (llItr = layer->lstObjs ; llItr != NULL ; llItr = llItr->next)
            if (QCAD_IS_CELL (llItr->data))
              g_signal_connect (G_OBJECT (llItr->data), "cell-function-changed", (GCallback)cell_function_changed, (*pdesign)) ;
        }
      }
    else
    if (!strcmp (pszLine, "[TYPE:BUS_LAYOUT]"))
      (*pdesign)->bus_layout = design_bus_layout_unserialize (pfile) ;

    g_free (pszLine) ;
    //having peeked the line, consume it
    g_free (ReadLine (pfile, '\0', FALSE)) ;
    }

  if (bEOF)
    (*pdesign) = design_destroy ((*pdesign)) ;
  else
    {
    if (NULL == (*pdesign)->bus_layout)
      (*pdesign)->bus_layout = design_bus_layout_new () ;
    design_rebuild_io_lists ((*pdesign)) ;
    if (NULL == (*pdesign)->lstCurrentLayer)
      (*pdesign)->lstCurrentLayer = (*pdesign)->lstLayers ;
    }

  return (NULL != (*pdesign)) ;
  }

BUS_LAYOUT *design_bus_layout_unserialize (FILE *pfile)
  {
  char *pszLine = NULL ;
  BUS_LAYOUT *bus_layout = NULL ;

  if (!SkipPast (pfile, '\0', "[TYPE:BUS_LAYOUT]", NULL))
    return NULL ;

  bus_layout = design_bus_layout_new () ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (pfile, '\0', TRUE)))
      break ;

    if (!strcmp (pszLine, "[#TYPE:BUS_LAYOUT]"))
      {
      g_free (pszLine) ;
      break ;
      }

    if (!strcmp (pszLine, "[TYPE:BUS]"))
      design_bus_layout_bus_unserialize (bus_layout->buses, pfile) ;

    g_free (pszLine) ;
    //having peeked the line, consume it
    g_free (ReadLine (pfile, '\0', FALSE)) ;
    }

  return bus_layout ;
  }

static gboolean design_bus_layout_bus_unserialize (EXP_ARRAY *buses, FILE *pfile)
  {
  BUS *bus = NULL ;
  gboolean bRet = TRUE ;
  char *pszLine = NULL, *pszValue = NULL ;

  if (!SkipPast (pfile, '\0', "[TYPE:BUS]", NULL))
    return FALSE ;

  exp_array_insert_vals (buses, NULL, 1, 1, -1) ;
  bus = &(exp_array_index_1d (buses, BUS, buses->icUsed - 1)) ;

  bus->cell_indices = exp_array_new (sizeof (int), 1) ;
  bus->pszName = NULL ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (pfile, '\0', TRUE)))
      {
      bRet = FALSE ;
      break ;
      }

    if (!strcmp (pszLine, "[#TYPE:BUS]"))
      {
      g_free (pszLine) ;
      break ;
      }

    if (!strcmp (pszLine, "[BUS_DATA]"))
      {
      if (!design_bus_layout_bus_data_unserialize (bus->cell_indices, pfile))
        {
        g_free (pszLine) ;
        return FALSE ;
        }
      }
    else
      {
      tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

      if (!strcmp (pszLine, "pszName"))
        bus->pszName = g_strdup (pszValue) ;
      else
      if (!strcmp (pszLine, "bus_function"))
        bus->bus_function = atoi (pszValue) ;
      }

    g_free (pszLine) ;
    //having peeked the line, consume it
    g_free (ReadLine (pfile, '\0', FALSE)) ;
    }

  return bRet ;
  }

static gboolean design_bus_layout_bus_data_unserialize (EXP_ARRAY *cell_indices, FILE *pfile)
  {
  gboolean bRet = TRUE ;
  char *pszLine = NULL, *pszValue = NULL, *pszItr = NULL ;
  int cell_idx = -1 ;

  if (!SkipPast (pfile, '\0', "[BUS_DATA]", NULL))
    return FALSE ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (pfile, '\0', TRUE)))
      {
      bRet = FALSE ;
      break ;
      }

    if (!strcmp (pszLine, "[#BUS_DATA]"))
      {
      g_free (pszLine) ;
      break ;
      }
    else
      {
      pszItr = pszLine ;
      while (NULL != (pszValue = next_space_separated_value (&pszItr)))
        {
        if (-1 == (cell_idx = atoi (pszValue)))
          fprintf (stderr, "Warning: adding index -1 to bus cell list\n") ;
        exp_array_insert_vals (cell_indices, &cell_idx, 1, 1, -1) ;
        }
      }

    g_free (pszLine) ;
    //having peeked the line, consume it
    g_free (ReadLine (pfile, '\0', FALSE)) ;
    }

  return bRet ;
  }
#endif /* def STDIO_FILEIO */

void design_selection_set_cell_host_name (DESIGN *design, char *pszHostName)
  {
  GList *lstCurrentLayer = NULL, *lstCurrentCell = NULL ;
  QCADLayer *layer = NULL ;

  for (lstCurrentLayer = design->lstLayers ; lstCurrentLayer != NULL ; lstCurrentLayer = lstCurrentLayer->next)
    if (LAYER_TYPE_CELLS == (QCAD_LAYER (lstCurrentLayer->data))->type)
      for (lstCurrentCell = layer->lstSelObjs ; lstCurrentCell != NULL ; lstCurrentCell = lstCurrentCell->next)
        if (QCAD_IS_CELL (lstCurrentCell->data))
          qcad_cell_set_host_name (QCAD_CELL (lstCurrentCell->data), pszHostName) ;
  }

gboolean design_selection_drop (DESIGN *design)
  {
  // For each layer, make sure that the selected objects in that layer do not overlap with non-selected
  // objects in that same layer.
  WorldRectangle ext = {0.0} ;
  QCADLayer *layer = NULL ;
  GList *llLayer = NULL ;//, *llObj = NULL, *llSelObj = NULL ;
//  QCADDesignObject *obj = NULL ;
//  gboolean bNoOverlap = TRUE ;
//  GList *lstDropList = NULL ;

  if (!design_get_extents (design, &ext, TRUE)) return TRUE ;

  for (llLayer = design->lstLayers ; llLayer != NULL ; llLayer = llLayer->next)
    if (LAYER_STATUS_ACTIVE == (layer = (QCAD_LAYER (llLayer->data)))->status)
      if (!qcad_layer_selection_drop (layer)) return FALSE ;
/*
      {
      // Assemble a list of objects overlapping with our selection
      for (llObj = layer->lstObjs, lstDropList = NULL ; llObj != NULL ; llObj = llObj->next)
        if (NULL != llObj->data)
          if (!(obj = QCAD_DESIGN_OBJECT (llObj->data))->bSelected)
            if (qcad_design_object_select_test (obj, &ext, SELECTION_INTERSECTION))
              if (!(obj->bSelected))
                lstDropList = g_list_prepend (lstDropList, obj) ;

      // Sometimes it's OK for objects to overlap, and other times it's not. The following rules
      // (only one rule so far - "cells in the same layer mustn't overlap") determine whether the
      // selection can be dropped

      // Rules for cell layers
      if (LAYER_TYPE_CELLS == layer->type)
        {
        for (llObj = lstDropList ; NULL != llObj && bNoOverlap ; llObj = llObj->next)
          for (llSelObj = layer->lstSelObjs ; NULL != llSelObj && bNoOverlap ; llSelObj = llSelObj->next)
            if (NULL != llSelObj->data)
              if (qcad_design_object_overlaps (QCAD_DESIGN_OBJECT (llObj->data), QCAD_DESIGN_OBJECT (llSelObj->data)) &&
                QCAD_IS_CELL (llObj->data) && QCAD_IS_CELL (llSelObj->data))
#ifdef DBG_OVERLAP
                {
                fprintf (stderr, "design_selection_drop:Object 0x%08X of type %s overlaps with object 0x%08X of type %s\n",
                  (int)(llObj->data), g_type_name (G_TYPE_FROM_INSTANCE (llObj->data)),
                  (int)(llSelObj->data), g_type_name (G_TYPE_FROM_INSTANCE (llSelObj->data))) ;
#endif // def DBG_OVERLAP
                bNoOverlap = FALSE ;
#ifdef DBG_OVERLAP
                }
#endif // def DBG_OVERLAP
        }
      else
      if (LAYER_TYPE_SUBSTRATE == layer->type)
        for (llObj = lstDropList ; NULL != llObj && bNoOverlap ; llObj = llObj->next)
          for (llSelObj = layer->lstSelObjs ; NULL != llSelObj && bNoOverlap ; llSelObj = llSelObj->next)
            if (NULL != llSelObj->data)
              if (qcad_design_object_overlaps (QCAD_DESIGN_OBJECT (llObj->data), QCAD_DESIGN_OBJECT (llSelObj->data)) &&
                QCAD_IS_SUBSTRATE (llObj->data) && QCAD_IS_SUBSTRATE (llSelObj->data))
                  bNoOverlap = FALSE ;

      g_list_free (lstDropList) ;
      lstDropList = NULL ;
      }

  return bNoOverlap ;
*/
  return TRUE ;
  }

void design_dump (DESIGN *design, FILE *pfile)
  {
  GList *lstLayer = NULL ;

  for (lstLayer = design->lstLayers ; lstLayer != NULL ; lstLayer = lstLayer->next)
    {
    fprintf (pfile, "Dumping layer at list node 0x%0" PTRDISPLAYWIDTH PRIXPTR ":\n", (uintptr_t)lstLayer) ;
    qcad_layer_dump ((QCAD_LAYER (lstLayer->data)), pfile) ;
    }

  fprintf (pfile, "bus_layout:\n") ;
  design_bus_layout_dump (design->bus_layout, pfile) ;
  }

void design_bus_layout_dump (BUS_LAYOUT *bus_layout, FILE *pfile)
  {
  int Nix, Nix1 ;
  BUS *bus = NULL ;

  design_bus_layout_cell_list_dump (bus_layout->inputs,  "bus_layout->inputs",  pfile) ;
  design_bus_layout_cell_list_dump (bus_layout->outputs, "bus_layout->outputs", pfile) ;

  fprintf (pfile, "bus_layout->buses:%d:\n", bus_layout->buses->icUsed) ;
  for (Nix = 0 ; Nix < bus_layout->buses->icUsed ; Nix++)
    {
    bus = &(exp_array_index_1d (bus_layout->buses, BUS, Nix)) ;
    fprintf (pfile, "%s Bus \"%s\":%d:\n", QCAD_CELL_INPUT == bus->bus_function ? "Input" : "Output", bus->pszName, bus->cell_indices->icUsed) ;
    for (Nix1 = 0; Nix1 < bus->cell_indices->icUsed ; Nix1++)
      fprintf (pfile, "Cell %d\n", exp_array_index_1d (bus->cell_indices, int, Nix1)) ;
    }
  }

void design_bus_layout_cell_list_dump (EXP_ARRAY *cell_list, char *pszVarName, FILE *pfile)
  {
  int Nix ;

  fprintf (pfile, "%s:%d:\n", pszVarName, cell_list->icUsed) ;
  for (Nix = 0 ; Nix < cell_list->icUsed ; Nix++)
    fprintf (pfile, "{%s,%s}\n",
      qcad_cell_get_label (exp_array_index_1d (cell_list, BUS_LAYOUT_CELL, Nix).cell),
      exp_array_index_1d (cell_list, BUS_LAYOUT_CELL, Nix).bIsInBus ? "Used" : "Unused") ;
  }

QCADDesignObject *design_selection_get_anchor (DESIGN *design)
  {
  GList *lstLayerItr = NULL ;
  GList *llItr = NULL ;

  for (lstLayerItr = design->lstLayers ; lstLayerItr != NULL ; lstLayerItr = lstLayerItr->next)
    for (llItr = QCAD_LAYER (lstLayerItr->data)->lstSelObjs ; llItr != NULL ; llItr = llItr->next)
      if (NULL != llItr->data)
        return QCAD_DESIGN_OBJECT (llItr->data) ;

  return NULL ;
  }

gboolean design_scale_cells (DESIGN *design, double scale)
  {
  QCADDesignObject *obj = NULL ;
  double dxMin = 0, dyMin = 0 ;
  QCADLayer *layer = NULL ;
  GList *llItr = NULL, *llItrObj = NULL ;
  gboolean bHaveCells = FALSE, bHaveBaseline = FALSE ;

  for (llItr = design->lstLayers ; llItr != NULL ; llItr = llItr->next)
    if (LAYER_TYPE_CELLS == (layer = QCAD_LAYER (llItr->data))->type)
      for (llItrObj = layer->lstObjs ; llItrObj != NULL ; llItrObj = llItrObj->next)
        if (NULL != llItrObj->data)
          if (QCAD_IS_CELL (obj = QCAD_DESIGN_OBJECT (llItrObj->data)))
            {
            bHaveCells = TRUE ;
            if (bHaveBaseline)
              {
              dxMin = MIN (dxMin, obj->bounding_box.xWorld) ;
              dyMin = MIN (dxMin, obj->bounding_box.yWorld) ;
              }
            else
              {
              bHaveBaseline = TRUE ;
              dxMin = obj->bounding_box.xWorld ;
              dyMin = obj->bounding_box.yWorld ;
              }
            }

  for (llItr = design->lstLayers ; llItr != NULL ; llItr = llItr->next)
    if (LAYER_TYPE_CELLS == (layer = QCAD_LAYER (llItr->data))->type)
      for (llItrObj = layer->lstObjs ; llItrObj != NULL ; llItrObj = llItrObj->next)
        if (NULL != llItrObj->data)
          if (QCAD_IS_CELL (obj = QCAD_DESIGN_OBJECT (llItrObj->data)))
            qcad_cell_scale (QCAD_CELL (obj), scale, dxMin, dyMin) ;

  return bHaveCells ;
  }

void design_selection_objects_foreach (DESIGN *design, DesignObjectCallback cb, gpointer data)
  {
  GList *llLayerItr = NULL, *llObjItr = NULL ;

  for (llLayerItr = design->lstLayers ; llLayerItr != NULL ; llLayerItr = llLayerItr->next)
    if (LAYER_STATUS_ACTIVE == (QCAD_LAYER (llLayerItr->data))->status)
      for (llObjItr = (QCAD_LAYER (llLayerItr->data))->lstSelObjs ; llObjItr != NULL ; llObjItr = llObjItr->next)
        if (NULL != llObjItr->data)
          (*cb) (design, QCAD_DESIGN_OBJECT (llObjItr->data), data) ;
  }

// We trust that the llLayerOrder payload is, does, in fact, consist
// of the pointers to all the layers in the design passed in
void design_set_layer_order (DESIGN *design, GList *llLayerOrder)
  {
  GList *llCopy = g_list_copy (llLayerOrder), *lstCurrentLayer = NULL, *llItr ;
  GList *lstLastLayer = g_list_last (llCopy) ;

  for (llItr = llCopy ; llItr != NULL ; llItr = llItr->next)
    if (llItr->data == design->lstCurrentLayer->data)
      {
      lstCurrentLayer = llItr ;
      break ;
      }

  g_list_free (design->lstLayers) ;
  design->lstLayers = llCopy ;
  design->lstLastLayer = lstLastLayer ;
  design->lstCurrentLayer = lstCurrentLayer ;
  }

QCADCell *design_bus_layout_iter_first (BUS_LAYOUT *bus_layout, BUS_LAYOUT_ITER *bus_layout_iter, QCADCellFunction cell_function, int *pidxMaster)
  {
  int Nix ;
  QCADCell *cellRet = NULL ;

  if (!(QCAD_CELL_INPUT == cell_function || QCAD_CELL_OUTPUT == cell_function || QCAD_CELL_NORMAL == cell_function) ||
      NULL == bus_layout ||
      NULL == bus_layout_iter)
    {
    if (NULL != pidxMaster) (*pidxMaster) = -1 ;
    return NULL ;
    }
  bus_layout_iter->bus_layout = bus_layout ;
  bus_layout_iter->cell_function = cell_function ;
  bus_layout_iter->cell_list =
    (QCAD_CELL_INPUT  == cell_function) ? bus_layout->inputs :
    (QCAD_CELL_NORMAL == cell_function) ? bus_layout->inputs : bus_layout->outputs ;
  bus_layout_iter->buses = bus_layout->buses ;

  for (Nix = 0 ; Nix < bus_layout_iter->buses->icUsed ; Nix++)
    if (exp_array_index_1d (bus_layout_iter->buses, BUS, Nix).bus_function == cell_function || QCAD_CELL_NORMAL == cell_function)
      break ;

  bus_layout_iter->idxBus = Nix ;
  bus_layout_iter->idxBusCell = 0 ;

  if (bus_layout_iter->idxBus == bus_layout_iter->buses->icUsed)
    {
    bus_layout_iter->idxCell = -1 ;
    design_bus_layout_next_unassigned_cell (bus_layout_iter) ;
    }
  else
    bus_layout_iter->idxCell = -1 ;

  cellRet = design_bus_layout_iter_this (bus_layout_iter, pidxMaster) ;

  return cellRet ;
  }

QCADCell *design_bus_layout_iter_next (BUS_LAYOUT_ITER *bus_layout_iter, int *pidxMaster)
  {
  BUS *bus = NULL ;
  int Nix = -1, idx = -1 ;
  QCADCell *cellRet = NULL ;

  while (bus_layout_iter->idxBus < bus_layout_iter->buses->icUsed)
    {
    bus = &(exp_array_index_1d (bus_layout_iter->buses, BUS, bus_layout_iter->idxBus)) ;
    if (bus_layout_iter->idxBusCell < bus->cell_indices->icUsed)
      {
      for (Nix = bus_layout_iter->idxBusCell + 1 ; Nix < bus->cell_indices->icUsed ; Nix++)
      	{
				idx = exp_array_index_1d (bus->cell_indices, int, Nix) ;
      	if ((exp_array_index_1d (bus_layout_iter->cell_list, BUS_LAYOUT_CELL, idx).cell)->cell_function == bus_layout_iter->cell_function)
	        break ;
	      }

      // The assignment below is intensional
      if ((bus_layout_iter->idxBusCell = Nix) < bus->cell_indices->icUsed)
      	break ;
      else
      	{
        bus_layout_iter->idxBusCell = -1 ;
      	for (bus_layout_iter->idxBus++ ; bus_layout_iter->idxBus < bus_layout_iter->buses->icUsed ; bus_layout_iter->idxBus++)
	        if (exp_array_index_1d (bus_layout_iter->buses, BUS, bus_layout_iter->idxBus).bus_function == bus_layout_iter->cell_function)
	          break ;
  	    }
      }
    }

  if (bus_layout_iter->idxBus == bus_layout_iter->buses->icUsed)
    design_bus_layout_next_unassigned_cell (bus_layout_iter) ;

  cellRet = design_bus_layout_iter_this (bus_layout_iter, pidxMaster) ;
  return cellRet ;
  }

static void design_bus_layout_next_unassigned_cell (BUS_LAYOUT_ITER *bus_layout_iter)
  {
  int Nix ;

  for (Nix = bus_layout_iter->idxCell + 1 ; Nix < bus_layout_iter->cell_list->icUsed ; Nix++)
    if (!(exp_array_index_1d (bus_layout_iter->cell_list, BUS_LAYOUT_CELL, Nix).bIsInBus))
      break ;

  if ((Nix == bus_layout_iter->cell_list->icUsed) &&
      (QCAD_CELL_NORMAL == bus_layout_iter->cell_function) &&
      (bus_layout_iter->cell_list == bus_layout_iter->bus_layout->inputs))
    {
    bus_layout_iter->cell_list = bus_layout_iter->bus_layout->outputs ;
    bus_layout_iter->idxCell = -1 ;
    design_bus_layout_next_unassigned_cell (bus_layout_iter) ;
    }

  bus_layout_iter->idxCell = Nix ;
  }

QCADCell *design_bus_layout_iter_this (BUS_LAYOUT_ITER *bus_layout_iter, int *pidxMaster)
  {
  BUS *bus = NULL ;
  QCADCell *cellRet = NULL ;

  if (bus_layout_iter->idxBus < bus_layout_iter->buses->icUsed)
    {
    bus = &(exp_array_index_1d (bus_layout_iter->buses, BUS, bus_layout_iter->idxBus)) ;
    if (bus_layout_iter->idxBusCell < bus->cell_indices->icUsed)
      (*pidxMaster) = exp_array_index_1d (bus->cell_indices, int, bus_layout_iter->idxBusCell) ;
    else
      // We should never get here (TM)
      // If we do, there's a bug in design_bus_layout_iter_{first,next}
      (*pidxMaster) = -1 ;
    }
  else
    {
    if (bus_layout_iter->idxCell < bus_layout_iter->cell_list->icUsed)
      (*pidxMaster) = bus_layout_iter->idxCell ;
    else
      // We should never get here (TM)
      // If we do, there's a bug in design_bus_layout_iter_{first,next}
      (*pidxMaster) = -1 ;
    }

  if (-1 == (*pidxMaster))
    cellRet = NULL ;
  else
    cellRet = (exp_array_index_1d (bus_layout_iter->cell_list, BUS_LAYOUT_CELL, (*pidxMaster)).cell) ;

  return cellRet ;
  }

// begin helper functions

void design_set_current_layer (DESIGN *design, QCADLayer *layer)
  {
  GList *llItr = NULL ;

  for (llItr = design->lstLayers ; llItr != NULL ; llItr = llItr->next)
    if (llItr->data == layer)
      {
      design->lstCurrentLayer = llItr ;
      break ;
      }
  }

static void design_bus_layout_remove_cell (DESIGN *design, QCADCell *cell, QCADCellFunction cell_function)
  {
  int Nix, Nix1 ;
  int idx = -1 ;
  EXP_ARRAY *cell_list = NULL ;
  BUS *bus = NULL ;

  if (NULL ==
    (cell_list = (QCAD_CELL_INPUT  == cell_function) ? design->bus_layout->inputs :
                 (QCAD_CELL_OUTPUT == cell_function) ? design->bus_layout->outputs : NULL))
    return ;

  for (idx = 0 ; idx < cell_list->icUsed ; idx++)
    if (QCAD_CELL (exp_array_index_1d (cell_list, BUS_LAYOUT_CELL, idx).cell) == cell)
      break ;

  // Cell wasn't found among the bus-forming cells
  if (idx == cell_list->icUsed) return ;

  for (Nix = 0 ; Nix < design->bus_layout->buses->icUsed ; Nix++)
    {
    bus = &exp_array_index_1d (design->bus_layout->buses, BUS, Nix) ;
    if (bus->bus_function == cell_function)
      {
      for (Nix1 = bus->cell_indices->icUsed ; Nix1 > -1 ; Nix1--)
        {
        if (exp_array_index_1d (bus->cell_indices, int, Nix1) == idx)
          exp_array_remove_vals (bus->cell_indices, 1, Nix1--, 1) ;
        else
        if (exp_array_index_1d (bus->cell_indices, int, Nix1) > idx)
          exp_array_index_1d (bus->cell_indices, int, Nix1)-- ;
        }

      if (0 == bus->cell_indices->icUsed)
        {
        g_free (bus->pszName) ;
        exp_array_free (bus->cell_indices) ;
        exp_array_remove_vals (design->bus_layout->buses, 1, Nix--, 1) ;
        }
      }
    }

  exp_array_remove_vals (cell_list, 1, idx, 1) ;
  }

void design_fix_legacy (DESIGN *design)
  {
  GList *llItrLayers = NULL, *llItrCells = NULL ;

  for (llItrLayers = design->lstLayers ; llItrLayers != NULL ; llItrLayers = llItrLayers->next)
    if (LAYER_TYPE_CELLS == (QCAD_LAYER (llItrLayers->data))->type)
      for (llItrCells = (QCAD_LAYER (llItrLayers->data))->lstObjs ; llItrCells != NULL ; llItrCells = llItrCells->next)
      	if (QCAD_IS_CELL (llItrCells->data))
      	  g_signal_connect (G_OBJECT (llItrCells->data), "cell-function-changed", (GCallback)cell_function_changed, design) ;
  }

static void qcad_layer_design_object_added (QCADLayer *layer, QCADDesignObject *obj, gpointer data)
  {
  DESIGN *design = (DESIGN *)data ;

  if (QCAD_IS_CELL (obj))
    {
    g_signal_handlers_disconnect_matched (G_OBJECT (obj), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, cell_function_changed, NULL) ;
    g_signal_connect (G_OBJECT (obj), "cell-function-changed", (GCallback)cell_function_changed, design) ;
    cell_function_changed (QCAD_CELL (obj), design) ;
    }
  }

static void qcad_layer_design_object_removed (QCADLayer *layer, QCADDesignObject *obj, gpointer data)
  {
  DESIGN *design = (DESIGN *)data ;

  if (QCAD_IS_CELL (obj))
    if (QCAD_CELL_INPUT == QCAD_CELL (obj)->cell_function || QCAD_CELL_OUTPUT == QCAD_CELL (obj)->cell_function)
      design_rebuild_io_lists (design) ;
  }

static void cell_function_changed (QCADCell *cell, DESIGN *design)
  {
  int Nix ;

  if (QCAD_CELL_NORMAL == cell->cell_function)
    {
    for (Nix = 0 ; Nix < design->bus_layout->inputs->icUsed ; Nix++)
      if (exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, Nix).cell == cell)
        {
        design_bus_layout_remove_cell (design, cell, QCAD_CELL_INPUT) ;
        return ;
        }

    for (Nix = 0 ; Nix < design->bus_layout->outputs->icUsed ; Nix++)
      if (exp_array_index_1d (design->bus_layout->outputs, BUS_LAYOUT_CELL, Nix).cell == cell)
        {
        design_bus_layout_remove_cell (design, cell, QCAD_CELL_OUTPUT) ;
        return ;
        }
    }
  else
  if (QCAD_CELL_INPUT == cell->cell_function || QCAD_CELL_OUTPUT == cell->cell_function)
    {
    EXP_ARRAY *io_list = (QCAD_CELL_INPUT == cell->cell_function) ?
      	design->bus_layout->inputs :
      	design->bus_layout->outputs ;

    for (Nix = 0 ; Nix < io_list->icUsed ; Nix++)
      if (exp_array_index_1d (io_list, BUS_LAYOUT_CELL, Nix).cell == cell)
        return ;
    }
  design_rebuild_io_lists (design) ;
  }

static void design_rebuild_io_lists (DESIGN *design)
  {
  GList *llItrLayers = NULL, *llItrCells = NULL ;
  int *pidx = NULL, *pNew = NULL ;
  EXP_ARRAY *io_list = NULL ;
  int idxIn = 0, idxOut = 0 ;
  int icInputsUsed  = 0 ;
  int icOutputsUsed = 0 ;
  int icNewInputs  = 0 ;
  int icNewOutputs = 0 ;
  int icOldCellCount = 0 ;
  int Nix, Nix1, Nix2, idx ;
  BUS_LAYOUT_CELL bus_layout_cell = {NULL, FALSE} ;
  BUS *bus = NULL ;

  idxIn =
  icInputsUsed  = design->bus_layout->inputs->icUsed ;

  idxOut =
  icOutputsUsed = design->bus_layout->outputs->icUsed ;

  for (llItrLayers = design->lstLastLayer ; llItrLayers != NULL ; llItrLayers = llItrLayers->prev)
    if (LAYER_TYPE_CELLS == (QCAD_LAYER (llItrLayers->data))->type)
      for (llItrCells = g_list_last ((QCAD_LAYER (llItrLayers->data))->lstObjs) ; llItrCells != NULL ; llItrCells = llItrCells->prev)
        if (NULL != llItrCells->data)
          {
          if (QCAD_CELL_INPUT  == QCAD_CELL (llItrCells->data)->cell_function ||
	            QCAD_CELL_OUTPUT == QCAD_CELL (llItrCells->data)->cell_function)
	          {
	          if (QCAD_CELL_INPUT  == QCAD_CELL (llItrCells->data)->cell_function)
	            {
	            pidx = &idxIn ;
	            io_list = design->bus_layout->inputs ;
              pNew = &icNewInputs ;
	            }
	          else
	            {
	            pidx = &idxOut ;
	            io_list = design->bus_layout->outputs ;
              pNew = &icNewOutputs ;
	            }
            bus_layout_cell.cell = llItrCells->data ;
	          exp_array_insert_vals (io_list, &bus_layout_cell, 1, 1, (*pidx)++) ;
            (*pNew)++ ;
	          }
          }

  for (Nix = 0 ; Nix < design->bus_layout->buses->icUsed ; Nix++)
    {
    bus = &(exp_array_index_1d (design->bus_layout->buses, BUS, Nix)) ;
    if (QCAD_CELL_INPUT  == bus->bus_function)
      {
      io_list = design->bus_layout->inputs ;
      icOldCellCount = icInputsUsed ;
      }
    else
      {
      io_list = design->bus_layout->outputs ;
      icOldCellCount = icOutputsUsed ;
      }

    for (Nix1 = 0 ; Nix1 < bus->cell_indices->icUsed ; Nix1++)
      {
      idx = exp_array_index_1d (bus->cell_indices, int, Nix1) ;
      for (Nix2 = icOldCellCount ; Nix2 < io_list->icUsed ; Nix2++)
        if (exp_array_index_1d (io_list, BUS_LAYOUT_CELL, Nix2).cell ==
	          exp_array_index_1d (io_list, BUS_LAYOUT_CELL,  idx).cell)
	        {
	        exp_array_index_1d (bus->cell_indices, int, Nix1) = Nix2 ;
	        exp_array_index_1d (io_list, BUS_LAYOUT_CELL, Nix2).bIsInBus = TRUE ;
          break ;
	        }

      if (Nix2 == io_list->icUsed)
      	exp_array_remove_vals (bus->cell_indices, 1, Nix1--, 1) ;
      }
    if (0 == bus->cell_indices->icUsed)
      {
      g_free (bus->pszName) ;
      exp_array_free (bus->cell_indices) ;
      exp_array_remove_vals (design->bus_layout->buses, 1, Nix--, 1) ;
      }
    }

  if (icInputsUsed > 0)
    exp_array_remove_vals (design->bus_layout->inputs,  1, 0, icInputsUsed) ;
  if (icOutputsUsed > 0)
    exp_array_remove_vals (design->bus_layout->outputs, 1, 0, icOutputsUsed) ;

  for (Nix = 0 ; Nix < design->bus_layout->buses->icUsed ; Nix++)
    {
    bus = &(exp_array_index_1d (design->bus_layout->buses, BUS, Nix)) ;
    if (QCAD_CELL_INPUT == bus->bus_function)
      {
      icOldCellCount = icInputsUsed ;
      io_list = design->bus_layout->inputs ;
      }
    else
      {
      icOldCellCount = icOutputsUsed ;
      io_list = design->bus_layout->outputs ;
      }
    for (Nix1 = 0 ; Nix1 < bus->cell_indices->icUsed ; Nix1++)
      {
      idx = (exp_array_index_1d (bus->cell_indices, int, Nix1) -= icOldCellCount) ;
      exp_array_index_1d (io_list, BUS_LAYOUT_CELL, idx).bIsInBus = TRUE ;
      }
    }
  }

BUS_LAYOUT *design_bus_layout_new ()
  {
  BUS_LAYOUT *bus_layout = g_malloc0 (sizeof (BUS_LAYOUT)) ;

  if (NULL == bus_layout)
    return NULL ;

  bus_layout->inputs = exp_array_new (sizeof (BUS_LAYOUT_CELL), 1) ;
  bus_layout->outputs = exp_array_new (sizeof (BUS_LAYOUT_CELL), 1) ;
  bus_layout->buses = exp_array_new (sizeof (BUS), 1) ;

  return bus_layout ;
  }

// Always returns NULL
BUS_LAYOUT *design_bus_layout_free (BUS_LAYOUT *bus_layout)
  {
  BUS *bus = NULL ;
  int Nix ;

  bus_layout->inputs = exp_array_free (bus_layout->inputs) ;
  bus_layout->outputs = exp_array_free (bus_layout->outputs) ;
  for (Nix = 0 ; Nix < bus_layout->buses->icUsed ; Nix++)
    {
    bus = &(exp_array_index_1d (bus_layout->buses, BUS, Nix)) ;
    bus->cell_indices = exp_array_free (bus->cell_indices) ;
    g_free (bus->pszName) ;
    bus->pszName = NULL ;
    }
  bus_layout->buses = exp_array_free (bus_layout->buses) ;

  g_free (bus_layout) ;

  return NULL ;
  }

EXP_ARRAY *design_selection_object_array_add_weak_pointers (EXP_ARRAY *obj_array)
  {
  int Nix ;
  QCADDesignObject **pobj = NULL ;
#ifdef DBG_WEAK_REFS
  DESIGN_OBJECT_ARRAY_WEAK_REF_STRUCT *weak_ref_struct = NULL ;
#endif /* def DBG_WEAK_REFS */


  if (NULL == obj_array) return NULL ;

  for (Nix = 0 ; Nix < obj_array->icUsed ; Nix++)
    if (NULL != (*(pobj = &exp_array_index_1d (obj_array, QCADDesignObject *, Nix))))
#ifdef DBG_WEAK_REFS
      {
      weak_ref_struct = g_malloc0 (sizeof (DESIGN_OBJECT_ARRAY_WEAK_REF_STRUCT)) ;
      weak_ref_struct->place_to_nullify = pobj ;
      weak_ref_struct->idx = Nix ;
      weak_ref_struct->object_array = obj_array ;
      g_object_weak_ref (G_OBJECT (*pobj), (GWeakNotify)design_selection_object_array_member_destroyed, weak_ref_struct) ;
      }
#else /* !def DBG_WEAK_REFS */
      g_object_add_weak_pointer (G_OBJECT (*pobj), (gpointer *)pobj) ;
#endif /* def DBG_WEAK_REFS */

  return obj_array ;
  }

#ifdef DBG_WEAK_REFS
static void design_selection_object_array_member_destroyed (gpointer data, gpointer ex_obj)
  {
  DESIGN_OBJECT_ARRAY_WEAK_REF_STRUCT *weak_ref_struct = (DESIGN_OBJECT_ARRAY_WEAK_REF_STRUCT *)data ;

  if (NULL == data || NULL == ex_obj) return ;

  fprintf (stderr, "design_selection_object_array_member_destroyed:nullifying idx %d of array 0x%08X which is 0x%08X\n",
    weak_ref_struct->idx, (int)(weak_ref_struct->object_array), (int)(weak_ref_struct->place_to_nullify)) ;

  *(weak_ref_struct->place_to_nullify) = NULL ;

  g_free (weak_ref_struct) ;
  }
#endif /* def DBG_WEAK_REFS */
