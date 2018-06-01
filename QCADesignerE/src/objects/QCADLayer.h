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
// Header for the layer. This is a structure containing //
// design objects. The kinds of objects a layer may     //
// contain depend on the kind of layer it is.           //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _QCADLayer_H_
#define _QCADLayer_H_

#ifdef GTK_GUI
  #include <gtk/gtk.h>
#endif /* def GTK_GUI */
#include "QCADDesignObject.h"
#include "../exp_array.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ALLOW_UNSERIALIZE_OVERLAP

typedef enum
  {
  LAYER_TYPE_SUBSTRATE,
  LAYER_TYPE_CELLS,
  LAYER_TYPE_CLOCKING,
  LAYER_TYPE_DRAWING,
  LAYER_TYPE_LAST_TYPE
  } LayerType ;

typedef enum
  {
  LAYER_STATUS_ACTIVE,  /* Editable (=> visible) */
  LAYER_STATUS_VISIBLE, /* Non-editable */
  LAYER_STATUS_HIDDEN,  /* not shown */
  LAYER_STATUS_LAST_STATUS
  } LayerStatus ;

enum
  {
  LAYER_DRAW_SELECTION = 0,
  LAYER_DRAW_NON_SELECTION,
  LAYER_DRAW_EVERYTHING
  } ;

typedef struct
  {
  QCADDesignObject parent_instance ;

  LayerType type ;
  LayerStatus status ;
  char *pszDescription ;
  GList *lstObjs ;
  GList *lstSelObjs ;
#ifdef GTK_GUI
  GtkWidget *combo_item ;
#endif /* def GTK_GUI */
#ifdef ALLOW_UNSERIALIZE_OVERLAP
  gboolean bAllowOverlap ;
#endif /* ALLOW_UNSERIALIZE_OVERLAP */
  GHashTable *default_properties ;
  GList *llContainerIter ;
  } QCADLayer ;

typedef struct
  {
  QCADDesignObjectClass parent_class ;

  void (*object_added) (QCADLayer *layer, QCADDesignObject *object, gpointer data) ;
  void (*object_removed) (QCADLayer *layer, QCADDesignObject *object, gpointer data) ;
  } QCADLayerClass ;

GType qcad_layer_get_type () ;

#define QCAD_TYPE_STRING_LAYER "QCADLayer"
#define QCAD_TYPE_LAYER (qcad_layer_get_type ())
#define QCAD_LAYER(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_LAYER, QCADLayer))
#define QCAD_LAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_LAYER, QCADLayerClass))
#define QCAD_IS_LAYER(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_LAYER))
#define QCAD_IS_LAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_LAYER))
#define QCAD_LAYER_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_LAYER, QCADLayerClass))

QCADLayer *qcad_layer_new (LayerType type, LayerStatus status, char *psz) ;
GHashTable *qcad_layer_object_containment_rules () ;
gboolean qcad_layer_selection_drop (QCADLayer *layer) ;
#ifdef GTK_GUI
EXP_ARRAY *qcad_layer_selection_subtract_window (QCADLayer *layer, GdkWindow *dst, GdkFunction rop, WorldRectangle *rcWorld, EXP_ARRAY *ar) ;
EXP_ARRAY *qcad_layer_selection_release (QCADLayer *layer, GdkWindow *dst, GdkFunction rop, EXP_ARRAY *ar) ;
void qcad_layer_draw (QCADLayer *layer, GdkDrawable *dst, GdkFunction rop, WorldRectangle *rc, int flags) ;
#endif /* def GTK_GUI */
gboolean qcad_layer_get_extents (QCADLayer *layer, WorldRectangle *extents, gboolean bSelection) ;
void qcad_layer_dump (QCADLayer *layer, FILE *pfile) ;
#ifdef STDIO_FILEIO
void qcad_layer_selection_serialize (QCADLayer *layer, FILE *pfile) ;
#endif /* def STDIO_FILEIO */
void qcad_layer_selection_create_from_selection (QCADLayer *layer) ;
EXP_ARRAY *qcad_layer_selection_get_object_array (QCADLayer *layer, EXP_ARRAY *ar) ;
QCADLayer *qcad_layer_from_object (QCADDesignObject *obj) ;
void qcad_layer_objects_foreach (QCADLayer *layer, gboolean bSelObjects, gboolean bDeepIter, void (*iter_func) (QCADDesignObject *obj, gpointer data), gpointer data) ;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _QCADLayer_H_ */
