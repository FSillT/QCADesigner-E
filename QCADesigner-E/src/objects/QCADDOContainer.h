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
// Header fo QCADDOContainer: An interface providing    //
// "add" and "remove" functions for a compound DO.      //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECT_QCADDDoContainer_H_
#define _OBJECT_QCADDDoContainer_H_

#include <glib-object.h>
#include "QCADDesignObject.h"

#define QCAD_TYPE_STRING_DO_CONTAINER "QCADDOContainer"
#define QCAD_TYPE_DO_CONTAINER (qcad_do_container_get_type ())
#define QCAD_DO_CONTAINER(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_DO_CONTAINER, QCADDOContainer))
#define QCAD_DO_CONTAINER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_DO_CONTAINER, QCADDOContainerClass))
#define QCAD_IS_DO_CONTAINER(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_DO_CONTAINER))
#define QCAD_IS_DO_CONTAINER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_DO_CONTAINER))
#define QCAD_DO_CONTAINER_GET_CLASS(object) (G_TYPE_INSTANCE_GET_INTERFACE ((object), QCAD_TYPE_DO_CONTAINER, QCADDOContainerClass))

typedef struct _QCADDOContainer QCADDOContainer ; /* dummy object structure */

typedef struct
  {
  GTypeInterface parent_interface ;

  /* functions promised by this interface */
  gboolean          (*add)    (QCADDOContainer *container, QCADDesignObject *obj) ;
  gboolean          (*remove) (QCADDOContainer *container, QCADDesignObject *obj) ;

  } QCADDOContainerClass ;

GType qcad_do_container_get_type () ;

gboolean          qcad_do_container_add    (QCADDOContainer *container, QCADDesignObject *obj) ;
gboolean          qcad_do_container_remove (QCADDOContainer *container, QCADDesignObject *obj) ;

#endif /* _OBJECT_QCADDDoContainer_H_ */
