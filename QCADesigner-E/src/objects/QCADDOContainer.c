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
// QCADDOContainer: An interface providing "add" and    //
// "remove" functions for a compound DO.                //
//                                                      //
//////////////////////////////////////////////////////////

#include <glib-object.h>
#include "QCADCompoundDO.h"
#include "QCADDOContainer.h"
#include "QCADDesignObject.h"

GType qcad_do_container_get_type ()
  {
  static GType qcad_do_container_type = 0 ;

  if (0 == qcad_do_container_type)
    {
    static GTypeInfo qcad_do_container_info =
      {
      sizeof (QCADDOContainerClass),
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      0,
      0,
      NULL
      } ;

    qcad_do_container_type = g_type_register_static (G_TYPE_INTERFACE, QCAD_TYPE_STRING_DO_CONTAINER, &qcad_do_container_info, 0) ;
    g_type_interface_add_prerequisite (qcad_do_container_type, QCAD_TYPE_COMPOUND_DO) ;
    }

  return qcad_do_container_type ;
  }

gboolean qcad_do_container_add (QCADDOContainer *container, QCADDesignObject *obj)
  {
  gboolean bRet = QCAD_DO_CONTAINER_GET_CLASS (container)->add (container, obj) ;
  if (bRet) g_signal_emit_by_name (container, "added", obj) ;
  return bRet ;
  }

gboolean qcad_do_container_remove (QCADDOContainer *container, QCADDesignObject *obj)
  {
  gboolean bRet = QCAD_DO_CONTAINER_GET_CLASS (container)->remove (container, obj) ;
  if (bRet) g_signal_emit_by_name (container, "removed", obj) ;
  return bRet ;
  }
