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
// The layer. This is a structure containing design     //
// objects. The kinds of objects a layer may contain    //
// depend on the kind of layer it is.                   //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef GTK_GUI
  #include <gtk/gtk.h>
#endif /* def GTK_GUI */

#include "../support.h"
#include "../fileio_helpers.h"
#include "../custom_widgets.h"
#include "QCADSubstrate.h"
#include "QCADLayer.h"
#include "QCADCell.h"
#include "QCADLabel.h"
#include "QCADCompoundDO.h"
#include "QCADDOContainer.h"
#include "objects_debug.h"

#define DBG_REFS(s)

typedef struct
  {
  QCADLayer *layer ;
  GList *llSel ;
  GList *llDeSel ;
  QCADDesignObject *parent ;
  } OBJECT_TRACK_STRUCT ;

#ifdef GTK_GUI
typedef struct
    {
    int flags ;
    GdkDrawable *dst ;
    GdkFunction rop ;
    WorldRectangle *rc ;
    } QCAD_LAYER_DRAW_PARAMS ;
#endif /* def GTK_GUI */

static GHashTable *qcad_layer_create_default_properties (LayerType type) ;
static GHashTable *qcad_layer_free_default_properties (GHashTable *ht) ;
static void qcad_layer_class_init (QCADDesignObjectClass *klass, gpointer data) ;
static void qcad_layer_instance_init (QCADDesignObject *object, gpointer data) ;
static void qcad_layer_instance_finalize (GObject *object) ;
#ifdef STDIO_FILEIO
static gboolean unserialize (QCADDesignObject *obj, FILE *pfile) ;
static void serialize (QCADDesignObject *obj, FILE *pfile) ;
#endif /* def STDIO_FILEIO */
static QCADDesignObject *hit_test (QCADDesignObject *obj, int x, int y) ;
static void copy (QCADDesignObject *src, QCADDesignObject *dst) ;

static void qcad_compound_do_interface_init (gpointer interface, gpointer interface_data) ;
static void qcad_do_container_interface_init (gpointer interface, gpointer interface_data) ;
static QCADDesignObject *qcad_layer_compound_do_first (QCADCompoundDO *container) ;
static QCADDesignObject *qcad_layer_compound_do_next (QCADCompoundDO *container) ;
static gboolean qcad_layer_compound_do_last (QCADCompoundDO *container) ;
static gboolean qcad_layer_do_container_add (QCADDOContainer *container, QCADDesignObject *obj) ;
static gboolean qcad_layer_do_container_remove (QCADDOContainer *container, QCADDesignObject *obj) ;
static void qcad_layer_track_new_object (QCADLayer *layer, QCADDesignObject *obj, GList *llDeSel, QCADDesignObject *parent) ;
#ifdef GTK_GUI
static EXP_ARRAY *qcad_layer_selection_release_object (QCADDesignObject *obj, GdkWindow *dst, GdkFunction rop, EXP_ARRAY *ar) ;
static void qcad_layer_draw_foreach (QCADDesignObject *obj, gpointer data) ;
#endif /* def GTK_GUI */

static void qcad_layer_compound_do_added (QCADCompoundDO *cdo, QCADDesignObject *obj, gpointer data) ;
static void qcad_layer_compound_do_removed (QCADCompoundDO *cdo, QCADDesignObject *obj, gpointer data) ;
static void qcad_design_object_selected (QCADDesignObject *obj, gpointer data) ;
static void qcad_design_object_destroyed (gpointer data, GObject *obj) ;

static void free_default_properties (gpointer key, gpointer value, gpointer user_data) ;

GType qcad_layer_get_type ()
  {
  static GType qcad_layer_type = 0 ;

  if (!qcad_layer_type)
    {
    static const GTypeInfo qcad_layer_info =
      {
      sizeof (QCADLayerClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_layer_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADLayer),
      0,
      (GInstanceInitFunc)qcad_layer_instance_init
      } ;

    static const GInterfaceInfo qcad_compound_do_info =
      {
      (GInterfaceInitFunc)qcad_compound_do_interface_init,
      NULL,
      NULL
      } ;

    static const GInterfaceInfo qcad_do_container_info =
      {
      (GInterfaceInitFunc)qcad_do_container_interface_init,
      NULL,
      NULL
      } ;

    if ((qcad_layer_type = g_type_register_static (QCAD_TYPE_DESIGN_OBJECT, QCAD_TYPE_STRING_LAYER, &qcad_layer_info, 0)))
      {
      g_type_add_interface_static (qcad_layer_type, QCAD_TYPE_COMPOUND_DO, &qcad_compound_do_info) ;
      g_type_add_interface_static (qcad_layer_type, QCAD_TYPE_DO_CONTAINER, &qcad_do_container_info) ;
      g_type_class_ref (qcad_layer_type) ;
      }
    }
  return qcad_layer_type ;
  }

static void qcad_layer_class_init (QCADDesignObjectClass *klass, gpointer data)
  {
#ifdef STDIO_FILEIO
  QCAD_DESIGN_OBJECT_CLASS (klass)->unserialize      = unserialize ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->serialize        = serialize ;
#endif /* def STDIO_FILEIO */
  G_OBJECT_CLASS (klass)->finalize = qcad_layer_instance_finalize ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->hit_test         = hit_test ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->copy             = copy ;
  }

static void qcad_compound_do_interface_init (gpointer interface, gpointer interface_data)
  {
  QCADCompoundDOClass *klass = (QCADCompoundDOClass *)interface ;

  klass->first  = qcad_layer_compound_do_first ;
  klass->next   = qcad_layer_compound_do_next ;
  klass->last   = qcad_layer_compound_do_last ;
  }

static void qcad_do_container_interface_init (gpointer interface, gpointer interface_data)
  {
  QCADDOContainerClass *klass = (QCADDOContainerClass *)interface ;

  klass->add    = qcad_layer_do_container_add ;
  klass->remove = qcad_layer_do_container_remove ;
  }

static void qcad_layer_instance_init (QCADDesignObject *object, gpointer data)
  {
  QCADLayer *layer = QCAD_LAYER (object) ;

  layer->type = LAYER_TYPE_CELLS ;
  layer->status = LAYER_STATUS_ACTIVE ;
  layer->pszDescription = g_strdup (_("Untitled Layer")) ;
  layer->lstObjs =
  layer->lstSelObjs = NULL ;
  layer->default_properties = qcad_layer_create_default_properties (LAYER_TYPE_CELLS) ;
  #ifdef ALLOW_UNSERIALIZE_OVERLAP
  layer->bAllowOverlap = FALSE ;
  #endif /* def ALLOW_UNSERIALIZE_OVERLAP */
  }

static void qcad_layer_instance_finalize (GObject *object)
  {
  GList *llItr = NULL, *llNext = NULL ;
  QCADLayer *layer = QCAD_LAYER (object) ;
  void (*parent_finalize) (GObject *object) ;

  DBG_OO (fprintf (stderr, "QCADLayer::instance_finalize:Layer %s:destroying lstObjs\n", layer->pszDescription)) ;

  for (llItr = layer->lstObjs ; llItr != NULL ; )
    {
    llNext = llItr->next ;
    if (NULL != llItr->data)
      {
      DBG_REFS (fprintf (stderr, "QCADLayer::instance_finalize:unref-ing object 0x%08X\n", (int)(llItr->data))) ;
      g_object_unref (G_OBJECT (llItr->data)) ;
      }
    llItr = llNext ;
    }

  g_free (layer->pszDescription) ;

  g_list_free (layer->lstObjs) ;
  g_list_free (layer->lstSelObjs) ;

  DBG_OO (fprintf (stderr, "QCADLayer::instance_finalize: Calling parent\n")) ;
  if (NULL != (parent_finalize = G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_LAYER)))->finalize))
    (*parent_finalize) (object) ;
  }

static gboolean qcad_layer_do_container_add (QCADDOContainer *container, QCADDesignObject *obj)
  {
  GList *lstIter = NULL ;
  OBJECT_TRACK_STRUCT *ots = NULL ;
  QCADLayer *layer = NULL ;
  QCADDesignObject *obj_child = NULL ;

  if (!QCAD_IS_LAYER (container)) return FALSE ;
  layer = QCAD_LAYER (container) ;
  if (NULL == obj) return FALSE ;
  ots = g_object_get_data (G_OBJECT (obj), "ots") ;

  if (NULL != ots)
    {
    if (NULL != ots->parent)
      if (QCAD_IS_DO_CONTAINER (ots->parent))
        return qcad_do_container_add (QCAD_DO_CONTAINER (ots->parent), obj) ;

    // If the layers don't match, we need to move the object to this layer
    if (ots->layer == layer)
      {
      // If we're re-adding the object to its layer, then we merely simulate its (de)selection
      if (NULL != ots->layer)
        {
        DBG_REFS (fprintf (stderr, "qcad_layer_add_object:refin-inf object 0x%08X so as to re-add to layer\n", (int)obj)) ;
        g_object_ref (G_OBJECT (obj)) ;
        if (NULL != ots->llDeSel)
          ots->llDeSel->data = obj ;
        qcad_design_object_selected (obj, ots) ;
        return TRUE ;
        }
      }
    }

  // Rules for cells
  if (QCAD_IS_CELL (obj))
    {
    if (layer->type != LAYER_TYPE_CELLS)
      return FALSE ;
    else
    // If the object is selected, we don't care that it overlaps, because it's floating
    if (!(obj->bSelected))
      {
#ifdef ALLOW_UNSERIALIZE_OVERLAP
      if (!(layer->bAllowOverlap))
#endif /* def ALLOW_UNSERIALIZE_OVERLAP */
      for (lstIter = layer->lstObjs ; lstIter != NULL ; lstIter = lstIter->next)
        if (qcad_design_object_overlaps (obj, QCAD_DESIGN_OBJECT (lstIter->data)))
          return FALSE ;
      }
    }
  else
  if (QCAD_IS_SUBSTRATE (obj))
    {
    if (layer->type != LAYER_TYPE_SUBSTRATE)
      return FALSE ;
    else
    // If the object is selected, we don't care that it overlaps, because it's floating
    if (!(obj->bSelected))
      {
      for (lstIter = layer->lstObjs ; lstIter != NULL ; lstIter = lstIter->next)
        if (qcad_design_object_overlaps (obj, QCAD_DESIGN_OBJECT (lstIter->data)))
          return FALSE ;
      }
    }
  else
  if (IS_QCAD_LABEL (obj) &&
      (!(LAYER_TYPE_DRAWING == layer->type && obj->bounding_box.cxWorld > 10 && obj->bounding_box.cyWorld > 10)))
    return FALSE ;

  qcad_layer_track_new_object (layer, obj, layer->lstObjs = g_list_prepend (layer->lstObjs, obj), NULL) ;
  if (QCAD_IS_COMPOUND_DO (obj))
    {
    g_signal_connect (G_OBJECT (obj), "added", (GCallback)qcad_layer_compound_do_added, layer) ;
    g_signal_connect (G_OBJECT (obj), "removed", (GCallback)qcad_layer_compound_do_removed, layer) ;
    for (obj_child = qcad_compound_do_first (QCAD_COMPOUND_DO (obj)) ; ; obj_child = qcad_compound_do_next (QCAD_COMPOUND_DO (obj)))
      {
      if (NULL != obj_child)
        qcad_layer_track_new_object (layer, obj_child, NULL, obj) ;
      if (qcad_compound_do_last (QCAD_COMPOUND_DO (obj))) break ;
      }
    }

  return TRUE ;
  }

static void qcad_layer_compound_do_removed (QCADCompoundDO *cdo, QCADDesignObject *obj, gpointer data)
  {
  OBJECT_TRACK_STRUCT *ots = g_object_get_data (G_OBJECT (obj), "ots") ;

  if (NULL == ots) return ;

  if (NULL != ots->llSel)
    if (NULL != ots->layer)
      if (NULL != ots->layer->lstSelObjs)
        {
        ots->layer->lstSelObjs = g_list_delete_link (ots->layer->lstSelObjs, ots->llSel) ;
        ots->llSel = NULL ;
        }
  }

static void qcad_layer_compound_do_added (QCADCompoundDO *cdo, QCADDesignObject *obj, gpointer data)
  {qcad_layer_track_new_object (QCAD_LAYER (data), obj, NULL, QCAD_DESIGN_OBJECT (cdo)) ;}

static void qcad_layer_track_new_object (QCADLayer *layer, QCADDesignObject *obj, GList *llDeSel, QCADDesignObject *parent)
  {
  gboolean bTransfer = FALSE ;
  OBJECT_TRACK_STRUCT *ots = NULL ;

  if (NULL == layer || NULL == obj) return ;

  ots = g_object_get_data (G_OBJECT (obj), "ots") ;

  // We only transfer an object from other layers if the rules allow us to do so. The rules are observed above.
  // By now, if (NULL != ots) then the layers are guaranteed not to match, because the matching case is handled
  // before the rules. Thus if (NULL != ots) then we are certainly transferring an object from another layer here.
  if (!(bTransfer = (NULL != ots)))
    {
    ots = g_malloc0 (sizeof (OBJECT_TRACK_STRUCT)) ;
    g_object_set_data (G_OBJECT (obj), "ots", ots) ;
    g_signal_connect (G_OBJECT (obj), "selected", (GCallback)qcad_design_object_selected, ots) ;
    g_object_weak_ref (G_OBJECT (obj), qcad_design_object_destroyed, ots) ;
    }

  ots->llDeSel = llDeSel ;
  // Before setting ots->layer, we need to remove the weak reference to the current containing layer
  if (bTransfer && NULL != ots->layer)
    g_object_remove_weak_pointer (G_OBJECT (ots->layer), (gpointer *)&(ots->layer)) ;
  ots->layer = layer ;
  ots->parent = parent ;
  DBG_REFS (fprintf (stderr, "Ref-ing object 0x%08X so as to newly add it to layer 0x%08X\n", (int)obj, (int)layer)) ;
  g_object_ref (G_OBJECT (obj)) ;
  g_object_add_weak_pointer (G_OBJECT (layer), (gpointer *)&(ots->layer)) ;

  // Simulate a selected event on the object so we may NULL out the appropriate list link
  qcad_design_object_selected (obj, ots) ;
  }

static gboolean qcad_layer_do_container_remove (QCADDOContainer *container, QCADDesignObject *obj)
  {
  OBJECT_TRACK_STRUCT *ots = NULL ;

  if (NULL == obj) return FALSE ;

  if (NULL == (ots = g_object_get_data (G_OBJECT (obj), "ots"))) return FALSE ;

  if (NULL != ots->parent)
    if (QCAD_IS_DO_CONTAINER (ots->parent))
      return qcad_do_container_remove (QCAD_DO_CONTAINER (ots->parent), obj) ;

  ots->llDeSel->data = NULL ;

  if (NULL != ots->llSel)
    if (NULL != ots->layer)
      {
      ots->layer->lstSelObjs = g_list_delete_link (ots->layer->lstSelObjs, ots->llSel) ;
      ots->llSel = NULL ;
      }

  DBG_REFS (fprintf (stderr, "qcad_layer_remove_object:Finally, unref-ing object 0x%08X\n", (int)obj)) ;
  g_object_unref (G_OBJECT (obj)) ;

  return TRUE ;
  }

QCADLayer *qcad_layer_new (LayerType type, LayerStatus status, char *pszDescription)
  {
  QCADLayer *layer = NULL ;

  if (NULL != (layer = g_object_new (QCAD_TYPE_LAYER, NULL)))
    {
    layer->status = status ;
    if (NULL != layer->pszDescription)
      g_free (layer->pszDescription) ;
    layer->pszDescription = g_strdup (pszDescription) ;
    if (type != layer->type)
      {
      layer->type = type ;
      qcad_layer_free_default_properties (layer->default_properties) ;
      layer->default_properties = qcad_layer_create_default_properties (type) ;
      }
    }

  return layer ;
  }

gboolean qcad_layer_selection_drop (QCADLayer *layer)
  {
  QCADDesignObject *obj = NULL ;
  GList *llOverlap = NULL, *llItr = NULL, *llItrSel = NULL ;
  WorldRectangle ext = {0.0} ;
  gboolean bNoOverlap = TRUE ;

  if (!qcad_layer_get_extents (layer, &ext, TRUE)) return TRUE ;

  // Assemble a list of objects lying within the extents of this layer's selection
  for (llItr = layer->lstObjs ; llItr != NULL ; llItr = llItr->next)
    if (NULL != llItr->data)
      if (!(obj = QCAD_DESIGN_OBJECT (llItr->data))->bSelected)
        if (qcad_design_object_select_test (obj, &ext, SELECTION_INTERSECTION))
          llOverlap = g_list_prepend (llOverlap, obj) ;

  // Sometimes it's OK for objects to overlap, and other times it's not. The following rules
  // (only one rule so far - "cells in the same layer mustn't overlap") determine whether the
  // selection can be dropped

  // Rules for cell layers
  if (LAYER_TYPE_CELLS == layer->type)
    {
    for (llItr = llOverlap ; llItr != NULL && bNoOverlap ; llItr = llItr->next)
      for (llItrSel = layer->lstSelObjs ; llItrSel != NULL && bNoOverlap ; llItrSel = llItrSel->next)
        if (!(NULL == llItrSel->data || NULL == llItr->data))
          if (qcad_design_object_overlaps (QCAD_DESIGN_OBJECT (llItr->data), QCAD_DESIGN_OBJECT (llItrSel->data)) && 
            (QCAD_IS_CELL (llItr->data) && QCAD_IS_CELL (llItrSel->data)))
            bNoOverlap = FALSE ;
    }
  else
  if (LAYER_TYPE_SUBSTRATE == layer->type)
    for (llItr = llOverlap ; llItr != NULL && bNoOverlap ; llItr = llItr->next)
      for (llItrSel = layer->lstSelObjs ; llItrSel != NULL && bNoOverlap ; llItrSel = llItrSel->next)
        if (!(NULL == llItrSel->data || NULL == llItr->data))
          if (qcad_design_object_overlaps (QCAD_DESIGN_OBJECT (llItr->data), QCAD_DESIGN_OBJECT (llItrSel->data)) && 
            (QCAD_IS_SUBSTRATE (llItr->data) && QCAD_IS_SUBSTRATE (llItrSel->data)))
            bNoOverlap = FALSE ;

  if (NULL != llOverlap)
    g_list_free (llOverlap) ;

  return bNoOverlap ;
  }

void qcad_layer_objects_foreach (QCADLayer *layer, gboolean bSelObjects, gboolean bDeepIter, void (*iter_func) (QCADDesignObject *obj, gpointer data), gpointer data)
  {
  GList *llItr = NULL ;
  QCADDesignObject *obj_child = NULL ;

  if (NULL == layer) return ;

  for (llItr = bSelObjects ? layer->lstSelObjs : layer->lstObjs ; NULL != llItr ; llItr = llItr->next)
    {
    (*iter_func) (llItr->data, data) ;
    if (bDeepIter)
      if (NULL != llItr->data)
        if (QCAD_IS_COMPOUND_DO (llItr->data))
          for (obj_child = qcad_compound_do_first (QCAD_COMPOUND_DO (llItr->data)) ;; obj_child = qcad_compound_do_next (QCAD_COMPOUND_DO (llItr->data)))
            {
            (*iter_func) (obj_child, data) ;
            if (qcad_compound_do_last (QCAD_COMPOUND_DO (llItr->data))) break ;
            }
    }
  }

#ifdef GTK_GUI
EXP_ARRAY *qcad_layer_selection_subtract_window (QCADLayer *layer, GdkWindow *dst, GdkFunction rop, WorldRectangle *rcWorld, EXP_ARRAY *ar)
  {
  QCADDesignObject *obj = NULL ;
  GList *llItr = NULL, *llNext = NULL ;

  llItr = layer->lstSelObjs ;
  while (NULL != llItr)
    {
    llNext = llItr->next ;
    if (qcad_design_object_select_test ((obj = QCAD_DESIGN_OBJECT (llItr->data)), rcWorld, SELECTION_CONTAINMENT))
      ar = qcad_layer_selection_release_object (obj, dst, rop, ar) ;
    llItr = llNext ;
    }
  return ar ;
  }

EXP_ARRAY *qcad_layer_selection_release (QCADLayer *layer, GdkWindow *dst, GdkFunction rop, EXP_ARRAY *ar)
  {
  if (NULL == layer || NULL == dst) return ar ;

  while (NULL != layer->lstSelObjs)
    ar = qcad_layer_selection_release_object (QCAD_DESIGN_OBJECT (layer->lstSelObjs->data), dst, rop, ar) ;
  return ar ;
  }

static EXP_ARRAY *qcad_layer_selection_release_object (QCADDesignObject *obj, GdkWindow *dst, GdkFunction rop, EXP_ARRAY *ar)
  {
  if (NULL == ar)
    ar = exp_array_new (sizeof (QCADDesignObject *), 1) ;
  qcad_design_object_set_selected (obj, FALSE) ;
  exp_array_insert_vals (ar, &obj, 1, 1, -1) ;
  qcad_design_object_draw (obj, dst, rop) ;

  return ar ;
  }

void qcad_layer_draw (QCADLayer *layer, GdkDrawable *dst, GdkFunction rop, WorldRectangle *rc, int flags)
  {
  QCAD_LAYER_DRAW_PARAMS cb_parms = {flags, dst, rop, rc} ;

  if (NULL == layer || NULL == dst) return ;

  if (!(LAYER_STATUS_VISIBLE == layer->status || LAYER_STATUS_ACTIVE  == layer->status)) return ;

  qcad_layer_objects_foreach (layer, (LAYER_DRAW_SELECTION == flags && LAYER_STATUS_ACTIVE == layer->status), TRUE, qcad_layer_draw_foreach, &cb_parms) ;
  }

static void qcad_layer_draw_foreach (QCADDesignObject *obj, gpointer data)
  {
  QCAD_LAYER_DRAW_PARAMS *cb_params = (QCAD_LAYER_DRAW_PARAMS *)data ;

  if (NULL == obj) return ;

  if (NULL != cb_params->rc)
    if (!qcad_design_object_select_test (obj, cb_params->rc, SELECTION_INTERSECTION))
      return ;

  if ((LAYER_DRAW_NON_SELECTION == cb_params->flags && !obj->bSelected) ||
      (LAYER_DRAW_SELECTION     == cb_params->flags &&  obj->bSelected) ||
      (LAYER_DRAW_EVERYTHING    == cb_params->flags))
    qcad_design_object_draw (obj, cb_params->dst, cb_params->rop) ;
  }
#endif /* def GTK_GUI */

EXP_ARRAY *qcad_layer_selection_get_object_array (QCADLayer *layer, EXP_ARRAY *ar)
  {
  GList *llItr = NULL ;

  for (llItr = layer->lstSelObjs ; llItr != NULL ; llItr = llItr->next)
    {
    if (NULL == ar)
      ar = exp_array_new (sizeof (QCADDesignObject *), 1) ;
    exp_array_insert_vals (ar, &(llItr->data), 1, 1, -1) ;
    }
  return ar ;
  }

void qcad_layer_dump (QCADLayer *layer, FILE *pfile)
  {
  GList *lstSelObj = NULL, *lstObj = NULL ;

  fprintf (pfile, "Layer \"%s\"(0x%08X):\n", layer->pszDescription, (int)layer) ;
#ifdef GTK_GUI
  fprintf (pfile, "combo_item = 0x%08X\n", (int)(layer->combo_item)) ;
#endif /* def GTK_GUI */
  if (NULL != layer->lstObjs)
    {
    fprintf (stderr, "lstObjs:|0x%08X|->", (int)(layer->lstObjs)) ;
    for (lstObj = layer->lstObjs ; lstObj != NULL ; lstObj = lstObj->next)
      {
      if (LAYER_TYPE_CELLS == layer->type)
        {
        if (NULL != lstObj->data)
          {
          if (NULL != lstObj->next)
            fprintf (pfile, "|[%.2lf,%.2lf]|0x%08X|->", QCAD_DESIGN_OBJECT (lstObj->data)->x, QCAD_DESIGN_OBJECT (lstObj->data)->y, (int)(lstObj->next)) ;
          else
            fprintf (pfile, "|[%.2lf,%.2lf]|0x%08X|\n", QCAD_DESIGN_OBJECT (lstObj->data)->x, QCAD_DESIGN_OBJECT (lstObj->data)->y, (int)(lstObj->next)) ;
          }
        else
          {
          if (NULL != lstObj->next)
            fprintf (pfile, "|0x%08X|0x%08X|->", (int)(lstObj->data), (int)(lstObj->next)) ;
          else
            fprintf (pfile, "|0x%08X|0x%08X|\n", (int)(lstObj->data), (int)(lstObj->next)) ;
          }
        }
      else
        {
        if (NULL != lstObj->next)
          fprintf (pfile, "|0x%08X|0x%08X|->", (int)(lstObj->data), (int)(lstObj->next)) ;
        else
          fprintf (pfile, "|0x%08X|0x%08X|\n", (int)(lstObj->data), (int)(lstObj->next)) ;
        }
      }
    }
  else
    fprintf (stderr, "lstObjs:|0x%08X|\n", (int)(layer->lstObjs)) ;

  if (NULL == layer->lstSelObjs)
    fprintf (stderr, "lstSelObjs:|0x%08X|\n", (int)(layer->lstSelObjs)) ;
  else
    {
    fprintf (stderr, "lstSelObjs:|0x%08X|->", (int)(layer->lstSelObjs)) ;
    for (lstSelObj = layer->lstSelObjs ; lstSelObj != NULL ; lstSelObj = lstSelObj->next)
      {
	    if (NULL != lstSelObj->next)
        fprintf (pfile, "|0x%08X|0x%08X|->", (int)(lstSelObj->data), (int)(lstSelObj->next)) ;
      else
        fprintf (pfile, "|0x%08X|0x%08X|\n", (int)(lstSelObj->data), (int)(lstSelObj->next)) ;
      }
    }
  }
#ifdef STDIO_FILEIO
void qcad_layer_selection_serialize (QCADLayer *layer, FILE *pfile)
  {
  GList *lstSelObj = NULL ;

  if (NULL == layer || NULL == pfile) return ;

  if (LAYER_STATUS_ACTIVE == layer->status && NULL != layer->lstSelObjs)
    {
    fprintf (pfile, "[TYPE:" QCAD_TYPE_STRING_LAYER "]\n") ;
    fprintf (pfile, "type=%d\n", layer->type) ;
    fprintf (pfile, "status=%d\n", layer->status) ;
    fprintf (pfile, "pszDescription=%s\n", NULL == layer->pszDescription ? "" : layer->pszDescription) ;
    for (lstSelObj = layer->lstSelObjs ; lstSelObj != NULL ; lstSelObj = lstSelObj->next)
      qcad_design_object_serialize (QCAD_DESIGN_OBJECT (lstSelObj->data), pfile) ;
    fprintf (pfile, "[#TYPE:" QCAD_TYPE_STRING_LAYER "]\n") ;
    }
  }
#endif /* def STDIO_FILEIO */

void qcad_layer_selection_create_from_selection (QCADLayer *layer)
  {
  GList *llItr = NULL, *llOldObjs = NULL, *llNext = NULL ;
  QCADDesignObject *obj = NULL ;
  if (NULL == layer) return ;

  llOldObjs = layer->lstSelObjs ;

  for (llItr = layer->lstSelObjs ; llItr != NULL ; llItr = llItr->next)
    if (NULL != (obj = qcad_design_object_new_from_object (QCAD_DESIGN_OBJECT (llItr->data))))
      {
      qcad_layer_do_container_add (QCAD_DO_CONTAINER (layer), obj) ;
      DBG_REFS (fprintf (stderr, "qcad_layer_selection_create_from_selection:Added object 0x%08X to layer 0x%08X, so unref-ing it\n", (int)obj, (int)layer)) ;
      g_object_unref (obj) ;
      }

  llItr = llOldObjs ;
  while (NULL != llItr)
    {
    llNext = llItr->next ;
    qcad_design_object_set_selected (QCAD_DESIGN_OBJECT (llItr->data), FALSE) ;
    llItr = llNext ;
    }
  }

gboolean qcad_layer_get_extents (QCADLayer *layer, WorldRectangle *extents, gboolean bSelection)
  {
  gboolean bHaveBaseLine = FALSE ;
  WorldRectangle rcWorld ;
  GList *lstObj = NULL, *llMaster = NULL ;
  double x, y ;

  if (NULL == layer || NULL == extents) return FALSE ;

  llMaster = (bSelection ? layer->lstSelObjs : layer->lstObjs) ;

  if (NULL != (lstObj = llMaster))
    {
    while (NULL != lstObj)
      if (NULL == lstObj->data)
        lstObj = lstObj->next ;
      else
        break ;
    if (NULL != lstObj)
      {
      qcad_design_object_get_bounds_box (QCAD_DESIGN_OBJECT (lstObj->data), extents) ;
      bHaveBaseLine = TRUE ;

      for (lstObj = lstObj->next ; lstObj != NULL ; lstObj = lstObj->next)
        if (NULL != lstObj->data)
          {
          x = extents->xWorld ;
          y = extents->yWorld ;
          qcad_design_object_get_bounds_box (QCAD_DESIGN_OBJECT (lstObj->data), &rcWorld) ;
          extents->xWorld = MIN (extents->xWorld, rcWorld.xWorld) ;
          extents->yWorld = MIN (extents->yWorld, rcWorld.yWorld) ;
          extents->cxWorld = MAX (x + extents->cxWorld, rcWorld.xWorld + rcWorld.cxWorld) - extents->xWorld ;
          extents->cyWorld = MAX (y + extents->cyWorld, rcWorld.yWorld + rcWorld.cyWorld) - extents->yWorld ;
          }
      }
    }
  return bHaveBaseLine ;
  }

QCADLayer *qcad_layer_from_object (QCADDesignObject *obj)
  {
  OBJECT_TRACK_STRUCT *ots = NULL ;

  if (NULL == obj) return NULL ;

  if (NULL != (ots = g_object_get_data (G_OBJECT (obj), "ots"))) return ots->layer ;

  return NULL ;
  }

static void qcad_design_object_destroyed (gpointer data, GObject *obj)
  {
  OBJECT_TRACK_STRUCT *ots = (OBJECT_TRACK_STRUCT *)data ;

  if (NULL == data || NULL == obj) return ;

  // If the object is destroyed before the layer is destroyed
  if (NULL != ots->layer)
    {
    ots->layer->lstObjs = g_list_delete_link (ots->layer->lstObjs, ots->llDeSel) ;
    if (NULL != ots->llSel)
      ots->layer->lstSelObjs = g_list_delete_link (ots->layer->lstSelObjs, ots->llSel) ;

    g_object_remove_weak_pointer (G_OBJECT (ots->layer), (gpointer *)&(ots->layer)) ;
    }

  g_free (ots) ;
  }

static void qcad_design_object_selected (QCADDesignObject *obj, gpointer data)
  {
  OBJECT_TRACK_STRUCT *ots = (OBJECT_TRACK_STRUCT *)data ;

  if (NULL == obj || NULL == data) return ;

  if (NULL != ots->parent)
    if (ots->parent->bSelected && obj->bSelected)
      return ;

  if (obj->bSelected)
    ots->llSel =
    ots->layer->lstSelObjs = g_list_prepend (ots->layer->lstSelObjs, obj) ;
  else
  if (NULL != ots->llSel)
    {
    ots->layer->lstSelObjs = g_list_delete_link (ots->layer->lstSelObjs, ots->llSel) ;
    ots->llSel = NULL ;
    }
  }

static GHashTable *qcad_layer_create_default_properties (LayerType type)
  {
  GHashTable *props = NULL ;
  GList *llItr = NULL ;

  props = g_hash_table_new (NULL, NULL) ;
  for (llItr = g_hash_table_lookup (qcad_layer_object_containment_rules (), (gpointer)(type)) ; llItr != NULL ; llItr = llItr->next)
    g_hash_table_insert (props, llItr->data, qcad_design_object_class_get_properties (QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek ((GType)(llItr->data))))) ;

  return props ;
  }

// This function destroys the default_properies hash table and always returns NULL
static GHashTable *qcad_layer_free_default_properties (GHashTable *ht)
  {
  g_hash_table_foreach (ht, free_default_properties, NULL) ;
  g_hash_table_destroy (ht) ;
  return NULL ;
  }

// This is a hash table such that the keys are layer types and the data for each layer type is a
// linked list of GTypes for that particular layer type - it basically defines the
// layer_type->object_type one-to-many constraints
GHashTable *qcad_layer_object_containment_rules ()
  {
  static GHashTable *ht = NULL ;
  GList *llObjs = NULL ;

  if (NULL != ht) return ht ;

  ht = g_hash_table_new (NULL, NULL) ;

  // Substrate Layer
  llObjs = NULL ;
  llObjs = g_list_prepend (llObjs, (gpointer)QCAD_TYPE_SUBSTRATE) ;
  g_hash_table_insert (ht, (gpointer)LAYER_TYPE_SUBSTRATE, llObjs) ;

  // Cells Layer
  llObjs = NULL ;
  llObjs = g_list_prepend (llObjs, (gpointer)QCAD_TYPE_CELL) ;
  g_hash_table_insert (ht, (gpointer)LAYER_TYPE_CELLS, llObjs) ;

  // Clocking Layer
  llObjs = NULL ;
  // No objects in the clocking layer - yet
  g_hash_table_insert (ht, (gpointer)LAYER_TYPE_CLOCKING, llObjs) ;

  //Drawing Layer
  llObjs = NULL ;
  llObjs = g_list_prepend (llObjs, (gpointer)QCAD_TYPE_LABEL) ;
  g_hash_table_insert (ht, (gpointer)LAYER_TYPE_DRAWING, llObjs) ;

  return ht ;
  }

static void free_default_properties (gpointer key, gpointer value, gpointer user_data)
  {if (NULL != value) g_free (value) ;}

#ifdef STDIO_FILEIO
static gboolean unserialize (QCADDesignObject *obj, FILE *pfile)
  {
  char *pszLine = NULL, *pszValue = NULL ;
  QCADLayer *layer = NULL ;
  int iShowProgress = -1 ;

  layer = QCAD_LAYER (obj) ;

#ifdef ALLOW_UNSERIALIZE_OVERLAP
  layer->bAllowOverlap = TRUE ;
#endif /* def ALLOW_UNSERIALIZE_OVERLAP */

  if (!SkipPast (pfile, '\0', "[TYPE:" QCAD_TYPE_STRING_LAYER "]", NULL))
    return FALSE ;

  while (TRUE)
    {
    //peek the next line
    if (NULL == (pszLine = ReadLine (pfile, '\0', TRUE))) break ;
    if (!strcmp (pszLine, "[#TYPE:" QCAD_TYPE_STRING_LAYER "]"))
      {
      g_free (pszLine) ;
      break ;
      }

    tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

    if (!strcmp (pszLine, "type"))
      {
      layer->type = atoi (pszValue) ;
      if (NULL != layer->default_properties)
        qcad_layer_free_default_properties (layer->default_properties) ;
      layer->default_properties = qcad_layer_create_default_properties (layer->type) ;
      }
    else
    if (!strcmp (pszLine, "status"))
      layer->status = atoi (pszValue) ;
    if (!strcmp (pszLine, "pszDescription"))
      {
      if (NULL != layer->pszDescription)
        g_free (layer->pszDescription) ;
      layer->pszDescription = g_strdup_printf ("%s", pszValue) ;
      }
    else
    if (!strncmp (pszLine, "[TYPE:", 6))
      {
      if (NULL != (obj = qcad_design_object_new_from_stream (pfile)))
        {
        qcad_layer_do_container_add (QCAD_DO_CONTAINER (layer), obj) ;
        DBG_REFS (fprintf (stderr, "QCADLayer::unserialize:After adding object 0x%08X to layer 0x%08X, unref-ing object\n", (int)obj, (int)layer)) ;
        g_object_unref (G_OBJECT (obj)) ;
        if (!(iShowProgress = (iShowProgress + 1) % 1000))
          set_progress_bar_fraction (get_file_percent (pfile)) ;
        }
      }
    g_free (pszLine) ;
    //having peeked the line, consume it
    g_free (ReadLine (pfile, '\0', FALSE)) ;
    }

#ifdef ALLOW_UNSERIALIZE_OVERLAP
  layer->bAllowOverlap = FALSE ;
#endif /* def ALLOW_UNSERIALIZE_OVERLAP */

  return (layer->type >= 0 && layer->type < LAYER_TYPE_LAST_TYPE &&
          layer->status >= 0 && layer->status < LAYER_STATUS_LAST_STATUS) ;
  }

static void serialize (QCADDesignObject *obj, FILE *pfile)
  {
  GList *llItr = NULL ;
  QCADLayer *layer = NULL ;

  if (NULL == obj || NULL == pfile) return ;

  layer = QCAD_LAYER (obj) ;

  fprintf (pfile, "[TYPE:" QCAD_TYPE_STRING_LAYER "]\n") ;
  fprintf (pfile, "type=%d\n", layer->type) ;
  fprintf (pfile, "status=%d\n", layer->status) ;
  fprintf (pfile, "pszDescription=%s\n", NULL == layer->pszDescription ? "" : layer->pszDescription) ;
  for (llItr = g_list_last (layer->lstObjs) ; llItr != NULL ; llItr = llItr->prev)
    if (NULL != llItr->data)
      qcad_design_object_serialize (QCAD_DESIGN_OBJECT (llItr->data), pfile) ;
  fprintf (pfile, "[#TYPE:" QCAD_TYPE_STRING_LAYER "]\n") ;
  }
#endif /* def STDIO_FILEIO */

static void copy (QCADDesignObject *src, QCADDesignObject *dst)
  {
  GList *llItr = NULL ;
  QCADLayer *srcLayer = NULL, *dstLayer = NULL ;

  if (NULL == src || NULL == dst) return ;
  if (!(QCAD_IS_LAYER (src) && QCAD_IS_LAYER (dst))) return ;
  srcLayer = QCAD_LAYER (src) ;
  dstLayer = QCAD_LAYER (dst) ;

  dstLayer->type = srcLayer->type ;
  dstLayer->status = srcLayer->status ;
  dstLayer->pszDescription = (NULL == srcLayer->pszDescription ? NULL : g_strdup (srcLayer->pszDescription)) ;

  for (llItr = g_list_last (srcLayer->lstObjs) ; llItr != NULL ; llItr = llItr->prev)
    {
    if (NULL != llItr->data)
      qcad_do_container_add (QCAD_DO_CONTAINER (dstLayer),
        qcad_design_object_new_from_object (QCAD_DESIGN_OBJECT (llItr->data))) ;
    if (llItr == srcLayer->llContainerIter)
      dstLayer->llContainerIter = dstLayer->lstObjs ;
    }
#ifdef GTK_GUI
  // SHALLOW COPY of combo item
  dstLayer->combo_item = srcLayer->combo_item ;
#endif /* def GTK_GUI */
  // NOT COPYING THE DEFAULT PROPERTIES HASH TABLE !!!
  }

static QCADDesignObject *hit_test (QCADDesignObject *obj, int x, int y)
  {
  GList *llItr = NULL ;
  QCADDesignObject *hit_object = NULL ;

  if (NULL == obj) return NULL ;

  if (QCAD_LAYER (obj)->status != LAYER_STATUS_ACTIVE) return NULL ;

  for (llItr = QCAD_LAYER (obj)->lstObjs ; llItr != NULL ; llItr = llItr->next)
    if (NULL != llItr->data)
      if (NULL != (hit_object = qcad_design_object_hit_test (QCAD_DESIGN_OBJECT (llItr->data), x, y)))
        return hit_object ;

  return NULL ;
  }

static QCADDesignObject *qcad_layer_compound_do_first (QCADCompoundDO *cdo)
  {
  if (NULL == cdo) return NULL ;

  if (!QCAD_IS_LAYER (cdo)) return NULL ;

  return NULL == ((QCAD_LAYER (cdo)->llContainerIter = QCAD_LAYER (cdo)->lstObjs))
    ? NULL
    : (QCAD_LAYER (cdo)->llContainerIter)->data ;
  }

static QCADDesignObject *qcad_layer_compound_do_next (QCADCompoundDO *cdo)
  {
  if (NULL == cdo) return NULL ;

  if (!QCAD_IS_LAYER (cdo)) return NULL ;

  if (NULL == QCAD_LAYER (cdo)->llContainerIter) return NULL ;

  QCAD_LAYER (cdo)->llContainerIter = (QCAD_LAYER (cdo)->llContainerIter)->next ;

  if (NULL == QCAD_LAYER (cdo)->llContainerIter) return NULL ;

  return (QCAD_LAYER (cdo)->llContainerIter)->data ;
  }

static gboolean qcad_layer_compound_do_last (QCADCompoundDO *cdo)
  {
  if (NULL == cdo) return TRUE ;

  if (!QCAD_IS_LAYER (cdo)) return TRUE ;

  if (NULL == QCAD_LAYER (cdo)->llContainerIter) return TRUE ;

  if (NULL == (QCAD_LAYER (cdo)->llContainerIter)->next) return TRUE ;

  return FALSE ;
  }
