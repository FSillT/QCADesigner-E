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
// Compound QCADDesignObject interface. Iterate over    //
// objects contained in an object and issue "added" and //
// "removed" signals. This, and QCADDOContainer need to //
// be cleaned up.                                       //
//                                                      //
//////////////////////////////////////////////////////////

#include <glib-object.h>
#include "QCADCompoundDO.h"
#include "QCADDesignObject.h"

static void qcad_compound_do_added   (QCADCompoundDO *container, QCADDesignObject *obj, gpointer data) ;
static void qcad_compound_do_removed (QCADCompoundDO *container, QCADDesignObject *obj, gpointer data) ;
static void qcad_compound_do_class_base_init (gpointer klass) ;

enum
  {
  QCAD_COMPOUND_DO_ADDED_SIGNAL,
  QCAD_COMPOUND_DO_REMOVED_SIGNAL,
  QCAD_COMPOUND_DO_LAST_SIGNAL
  } ;

static guint qcad_compound_do_signals[QCAD_COMPOUND_DO_LAST_SIGNAL] = {0} ;

GType qcad_compound_do_get_type ()
  {
  static GType qcad_compound_do_type = 0 ;

  if (0 == qcad_compound_do_type)
    {
    static GTypeInfo qcad_compound_do_info =
      {
      sizeof (QCADCompoundDOClass),
      (GBaseInitFunc)qcad_compound_do_class_base_init,
      NULL,
      NULL,
      NULL,
      NULL,
      0,
      0,
      NULL
      } ;

    qcad_compound_do_type = g_type_register_static (G_TYPE_INTERFACE, QCAD_TYPE_STRING_COMPOUND_DO, &qcad_compound_do_info, 0) ;
    }

  return qcad_compound_do_type ;
  }

static void qcad_compound_do_class_base_init (gpointer klass)
  {
  static gboolean bFirstCall = TRUE ;

  if (bFirstCall)
    {
    ((QCADCompoundDOClass *)klass)->added = qcad_compound_do_added ;
    ((QCADCompoundDOClass *)klass)->removed = qcad_compound_do_removed ;

    qcad_compound_do_signals[QCAD_COMPOUND_DO_ADDED_SIGNAL] =
      g_signal_new ("added", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET (QCADCompoundDOClass, added), NULL, NULL, g_cclosure_marshal_VOID__OBJECT,
          G_TYPE_NONE, 1, G_TYPE_OBJECT) ;

    qcad_compound_do_signals[QCAD_COMPOUND_DO_REMOVED_SIGNAL] =
      g_signal_new ("removed", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET (QCADCompoundDOClass, removed), NULL, NULL, g_cclosure_marshal_VOID__OBJECT,
          G_TYPE_NONE, 1, G_TYPE_OBJECT) ;

    bFirstCall = FALSE ;
    }
  }

static void qcad_compound_do_added   (QCADCompoundDO *container, QCADDesignObject *obj, gpointer data) {}
static void qcad_compound_do_removed (QCADCompoundDO *container, QCADDesignObject *obj, gpointer data) {}

QCADDesignObject *qcad_compound_do_first (QCADCompoundDO *cdo)
  {return QCAD_COMPOUND_DO_GET_CLASS (cdo)->first (cdo) ;}

QCADDesignObject *qcad_compound_do_next (QCADCompoundDO *cdo)
  {return QCAD_COMPOUND_DO_GET_CLASS (cdo)->next (cdo) ;}

gboolean qcad_compound_do_last (QCADCompoundDO *cdo)
  {return QCAD_COMPOUND_DO_GET_CLASS (cdo)->last (cdo) ;}
