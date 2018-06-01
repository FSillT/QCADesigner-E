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
// Header for the ruler. This is a graduated object     //
// displaying the distance relative to its origin. It   //
// has four different orientations designed to "stick"  //
// to circuits on one of 4 sides.                       //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADRuler_H_
#define _OBJECTS_QCADRuler_H_

#include <glib-object.h>
#include "../gdk_structs.h"
#ifdef GTK_GUI
  #include "../exp_pixmap.h"
#endif /* def GTK_GUI */
#include "../exp_array.h"
#include "QCADStretchyObject.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum
  {
  QCAD_RULER_ORIENTATION_EAST,
  QCAD_RULER_ORIENTATION_SOUTH,
  QCAD_RULER_ORIENTATION_WEST,
  QCAD_RULER_ORIENTATION_NORTH
  } QCADRulerOrientation ;

typedef struct
  {
  QCADStretchyObject parent_instance ;
  EXP_ARRAY *labels ;
  int icLabelsVisible ;
  QCADRulerOrientation orientation ;
  WorldRectangle ruler_bounding_box ;
  } QCADRuler ;

typedef struct
  {
  // public
  QCADStretchyObjectClass parent_class ;
  // Used by the stretchy mouse handlers
  QCADRulerOrientation old_orientation ;
  } QCADRulerClass ;

GType qcad_ruler_get_type () ;

QCADRuler *qcad_ruler_new () ;

#define QCAD_TYPE_STRING_RULER "QCADRuler"
#define QCAD_TYPE_RULER (qcad_ruler_get_type ())
#define QCAD_RULER(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_RULER, QCADRuler))
#define QCAD_RULER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_RULER, QCADRulerClass))
#define IS_QCAD_RULER(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_RULER))
#define IS_QCAD_RULER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_RULER))
#define QCAD_RULER_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_RULER, QCADRulerClass))

#ifdef __cplusplus
}
#endif
#endif /* _OBJECTS_QCADRuler_H_ */
