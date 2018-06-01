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
// Header for the compound QCADDesignObject interface.  //
// Iterate over objects contained in an object and      //
// issue "added" and "removed" signals. This, and       //
// QCADDOContainer need to be cleaned up.               //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADCompoundDO_H_
#define _OBJECTS_QCADCompoundDO_H_

#include <glib-object.h>
#include "QCADDesignObject.h"

#define QCAD_TYPE_STRING_COMPOUND_DO "QCADCompoundDO"
#define QCAD_TYPE_COMPOUND_DO (qcad_compound_do_get_type ())
#define QCAD_COMPOUND_DO(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_COMPOUND_DO, QCADCompoundDO))
#define QCAD_COMPOUND_DO_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_COMPOUND_DO, QCADCompoundDOClass))
#define QCAD_IS_COMPOUND_DO(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_COMPOUND_DO))
#define QCAD_IS_COMPOUND_DO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_COMPOUND_DO))
#define QCAD_COMPOUND_DO_GET_CLASS(object) (G_TYPE_INSTANCE_GET_INTERFACE ((object), QCAD_TYPE_COMPOUND_DO, QCADCompoundDOClass))

typedef struct _QCADCompoundDO QCADCompoundDO ; /* dummy object structure */

typedef struct
  {
  GTypeInterface parent_interface ;

  /* signals promised by this interface */
  void (*added)   (QCADCompoundDO *container, QCADDesignObject *obj, gpointer data) ;
  void (*removed) (QCADCompoundDO *container, QCADDesignObject *obj, gpointer data) ;

  /* functions promised by this interface */
  QCADDesignObject *(*first) (QCADCompoundDO *compound_do) ;
  QCADDesignObject *(*next)  (QCADCompoundDO *compound_do) ;
  gboolean          (*last)  (QCADCompoundDO *compound_do) ;
  } QCADCompoundDOClass ;

GType qcad_compound_do_get_type () ;

QCADDesignObject *qcad_compound_do_first  (QCADCompoundDO *container) ;
QCADDesignObject *qcad_compound_do_next   (QCADCompoundDO *container) ;
gboolean          qcad_compound_do_last   (QCADCompoundDO *container) ;

#endif /* _OBJECTS_QCADCompoundDO_H_ */
