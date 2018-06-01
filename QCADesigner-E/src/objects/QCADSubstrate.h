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
// Header for the substrate. This object serves no      //
// purpose (so far) other than to provide a guide for   //
// placing objects.                                     //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADSubstrate_H_
#define _OBJECTS_QCADSubstrate_H_

#include <glib-object.h>
#include "QCADStretchyObject.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
  {
  /* public */
  QCADStretchyObjectClass parent_class ;

  /* private */
  } QCADSubstrateClass ;

typedef struct
  {
  QCADStretchyObject parent_instance ;
  double grid_spacing ; // nm
  } QCADSubstrate ;

GType qcad_substrate_get_type () ;

#define QCAD_TYPE_STRING_SUBSTRATE "QCADSubstrate"
#define QCAD_TYPE_SUBSTRATE (qcad_substrate_get_type ())
#define QCAD_SUBSTRATE(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_SUBSTRATE, QCADSubstrate))
#define QCAD_SUBSTRATE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_SUBSTRATE, QCADSubstrateClass))
#define QCAD_IS_SUBSTRATE(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_SUBSTRATE))
#define QCAD_IS_SUBSTRATE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_SUBSTRATE))
#define QCAD_SUBSTRATE_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_SUBSTRATE, QCADSubstrateClass))

///////////////////////////////////////////////////////////////////////////////

QCADDesignObject *qcad_substrate_new (double x, double y, double cx, double cy, double grid_spacing) ;
void qcad_substrate_snap_point (QCADSubstrate *subs, double *px, double *py) ;

#ifdef __cplusplus
}
#endif
#endif /* _OBJECTS_QCADRectangle_H_ */
