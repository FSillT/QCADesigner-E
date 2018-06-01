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
// Header for a tree view container that allows "freeze //
// columns". That is, the horizontal scrolling does not //
// scroll the entire tree view but, instead, it hides   //
// and shows columns as appropriate, keeping the first  //
// n columns always visible.                            //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADTreeViewContainer_H_
#define _OBJECTS_QCADTreeViewContainer_H_

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
  {
  GtkScrolledWindowClass parent_klass ;
  } QCADTreeViewContainerClass ;

typedef struct
  {
  GtkScrolledWindow parent_instance ;
  int n_frozen_columns ;
  GtkAdjustment *fake_hadj ;
  } QCADTreeViewContainer ;

GType qcad_tree_view_container_get_type () ;

GtkWidget *qcad_tree_view_container_new () ;

void qcad_tree_view_container_freeze_columns (QCADTreeViewContainer *tvc, int n_columns) ;

#define QCAD_TYPE_STRING_TREE_VIEW_CONTAINER "QCADTreeViewContainer"
#define QCAD_TYPE_TREE_VIEW_CONTAINER (qcad_tree_view_container_get_type ())
#define QCAD_TREE_VIEW_CONTAINER(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_TREE_VIEW_CONTAINER, QCADTreeViewContainer))
#define QCAD_TREE_VIEW_CONTAINER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_TREE_VIEW_CONTAINER, QCADTreeViewContainerClass))
#define QCAD_IS_TREE_VIEW_CONTAINER(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_TREE_VIEW_CONTAINER))
#define QCAD_IS_TREE_VIEW_CONTAINER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_TREE_VIEW_CONTAINER_VT))
#define QCAD_TREE_VIEW_CONTAINER_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_TREE_VIEW_CONTAINER, QCADTreeViewContainerClass))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* def _OBJECTS_QCADTreeViewContainer_H_ */
