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
// Header for the stretchy object. This is the base     //
// class for all stretchable objects such as labels,    //
// substrates, and rulers.                              //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADStretchyObject_H_
#define _OBJECTS_QCADStretchyObject_H_

#include <glib-object.h>
#include "../gdk_structs.h"
#ifdef GTK_GUI
  #include "../exp_pixmap.h"
#endif /* def GTK_GUI */
#include "QCADDesignObject.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
  {
  QCADDesignObject parent_instance ;
  } QCADStretchyObject ;

typedef struct
  {
  // public
  QCADDesignObjectClass parent_class ;

  // used by the mouse handlers to draw a new object
#ifdef GTK_GUI
  EXP_PIXMAP *epm ;
#endif /* def GTK_GUI */
  QCADStretchyObject *tmpobj ;
  int xRef ;
  int yRef ;

  // Function used to recalculate the state of the object between 2 consecutive drawings
  // when dragging the object upon creation or resize
  void (*stretch_draw_state_change) (QCADStretchyObject *tmpobj, int x, int y, int xRef, int yRef) ;

  } QCADStretchyObjectClass ;

GType qcad_stretchy_object_get_type () ;

#define QCAD_TYPE_STRING_STRETCHY_OBJECT "QCADStretchyObject"
#define QCAD_TYPE_STRETCHY_OBJECT (qcad_stretchy_object_get_type ())
#define QCAD_STRETCHY_OBJECT(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_STRETCHY_OBJECT, QCADStretchyObject))
#define QCAD_STRETCHY_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_STRETCHY_OBJECT, QCADStretchyObjectClass))
#define IS_QCAD_STRETCHY_OBJECT(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_STRETCHY_OBJECT))
#define IS_QCAD_STRETCHY_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_STRETCHY_OBJECT))
#define QCAD_STRETCHY_OBJECT_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_STRETCHY_OBJECT, QCADStretchyObjectClass))

#ifdef __cplusplus
}
#endif
#endif /* _OBJECTS_QCADStretchyObject_H_ */
