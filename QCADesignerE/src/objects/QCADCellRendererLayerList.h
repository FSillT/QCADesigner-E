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
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADCellRendererLayerList_H_
#define _OBJECTS_QCADCellRendererLayerList_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../design.h"

typedef struct
  {
  GtkCellRendererText parent ;

  DESIGN *design ;
  QCADLayer *layer ;
  QCADLayer *template ;
  } QCADCellRendererLayerList ;

typedef struct
  {
  GtkCellRendererTextClass parent_class ;
  void (*layer_changed) (GtkCellRenderer *cell, char *pszPath, QCADLayer *layer) ;
  } QCADCellRendererLayerListClass ;

GType qcad_cell_renderer_layer_list_get_type () ;
GtkCellRenderer *qcad_cell_renderer_layer_list_new () ;

#define QCAD_TYPE_STRING_CELL_RENDERER_LAYER_LIST "QCADCellRendererLayerList"
#define QCAD_TYPE_CELL_RENDERER_LAYER_LIST (qcad_cell_renderer_layer_list_get_type ())
#define QCAD_CELL_RENDERER_LAYER_LIST(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_CELL_RENDERER_LAYER_LIST, QCADCellRendererLayerList))
#define QCAD_CELL_RENDERER_LAYER_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_CELL_RENDERER_LAYER_LIST, QCADCellRendererLayerListClass))
#define QCAD_IS_CELL_RENDERER_LAYER_LIST(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_CELL_RENDERER_LAYER_LIST))
#define QCAD_IS_CELL_RENDERER_LAYER_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_CELL_RENDERER_LAYER_LIST))
#define QCAD_CELL_RENDERER_LAYER_LIST_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_CELL_RENDERER_LAYER_LIST, QCADCellRendererLayerListClass))

typedef struct
  {
  GtkEventBox parent;

  gboolean editing_cancelled ;
  } QCADLayerListEditable ;

typedef struct
  {
  GtkEventBoxClass parent_class;
  void (*editing_done) (GtkCellEditable *ce) ;
  void (*remove_widget) (GtkCellEditable *ce) ;

  void (*start_editing) (GtkCellEditable *ce, GdkEvent *ev) ;
  } QCADLayerListEditableClass ;

GType qcad_layer_list_editable_get_type () ;
GtkCellEditable *qcad_layer_list_editable_new (DESIGN *design, int layer_type) ;

#define QCAD_TYPE_STRING_LAYER_LIST_EDITABLE "QCADLayerListEditable"
#define QCAD_TYPE_LAYER_LIST_EDITABLE (qcad_layer_list_editable_get_type ())
#define QCAD_LAYER_LIST_EDITABLE(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_LAYER_LIST_EDITABLE, QCADLayerListEditable))
#define QCAD_LAYER_LIST_EDITABLE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_LAYER_LIST_EDITABLE, QCADLayerListEditableClass))
#define QCAD_IS_LAYER_LIST_EDITABLE(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_LAYER_LIST_EDITABLE))
#define QCAD_IS_LAYER_LIST_EDITABLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_LAYER_LIST_EDITABLE))
#define QCAD_LAYER_LIST_EDITABLE_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_LAYER_LIST_EDITABLE, QCADLayerListEditableClass))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* def _OBJECTS_QCADCellRendererLayerList_H_ */
