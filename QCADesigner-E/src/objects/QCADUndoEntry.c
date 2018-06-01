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
// The undo entry. This is the basis of the undo        //
// system. An undo entry group contains a bunch of      //
// these, and fires each one in sequence backwards or   //
// forwards.                                            //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib-object.h>
#include "QCADUndoEntry.h"
#include "objects_debug.h"

static void qcad_undo_entry_class_init (GObjectClass *klass, gpointer data) ;
static void qcad_undo_entry_instance_init (GObject *object, gpointer data) ;
static void qcad_undo_entry_instance_finalize (GObject *object) ;

static void fire (QCADUndoEntry *undo_entry, gboolean bUndo) ;

enum
  {
  QCAD_UNDO_ENTRY_APPLY_SIGNAL,
  QCAD_UNDO_ENTRY_LAST_SIGNAL
  } ;

static guint qcad_undo_entry_signals[QCAD_UNDO_ENTRY_LAST_SIGNAL] = {0} ;

GType qcad_undo_entry_get_type ()
  {
  static GType qcad_undo_entry_type = 0 ;

  if (!qcad_undo_entry_type)
    {
    static const GTypeInfo qcad_undo_entry_info =
      {
      sizeof (QCADUndoEntryClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_undo_entry_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADUndoEntry),
      0,
      (GInstanceInitFunc)qcad_undo_entry_instance_init
      } ;

    if ((qcad_undo_entry_type = g_type_register_static (G_TYPE_OBJECT, QCAD_TYPE_STRING_UNDO_ENTRY, &qcad_undo_entry_info, 0)))
      g_type_class_ref (qcad_undo_entry_type) ;
    DBG_OO (fprintf (stderr, "Registered %s as %d\n", QCAD_TYPE_STRING_UNDO_ENTRY, (int)qcad_undo_entry_type)) ;
    }
  return qcad_undo_entry_type ;
  }

static void qcad_undo_entry_class_init (GObjectClass *klass, gpointer data)
  {
  DBG_OO (fprintf (stderr, "%s::class_init:Entering.\n", QCAD_TYPE_STRING_UNDO_ENTRY)) ;
  qcad_undo_entry_signals[QCAD_UNDO_ENTRY_APPLY_SIGNAL] =
    g_signal_new ("apply", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (QCADUndoEntryClass, apply), NULL, NULL, g_cclosure_marshal_VOID__BOOLEAN,
      G_TYPE_NONE, 1, G_TYPE_BOOLEAN) ;

  QCAD_UNDO_ENTRY_CLASS (klass)->fire = fire ;

  G_OBJECT_CLASS (klass)->finalize = qcad_undo_entry_instance_finalize ;
  DBG_OO (fprintf (stderr, "%s::class_init:Leaving.\n", QCAD_TYPE_STRING_UNDO_ENTRY)) ;
  }

static void qcad_undo_entry_instance_init (GObject *object, gpointer data)
  {
  DBG_OO (fprintf (stderr, "%s::instance_init:Entering.\n", QCAD_TYPE_STRING_UNDO_ENTRY)) ;
  DBG_OO (fprintf (stderr, "%s::instance_init:Leaving.\n", QCAD_TYPE_STRING_UNDO_ENTRY)) ;
  }

static void qcad_undo_entry_instance_finalize (GObject *object)
  {
  void (*parent_finalize) (GObject *obj) =
    G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_UNDO_ENTRY)))->finalize ;

  if (NULL != parent_finalize)
    (*parent_finalize) (object) ;
  DBG_OO (fprintf (stderr, QCAD_TYPE_STRING_UNDO_ENTRY "::instance_finalize:Leaving\n")) ;
  }

///////////////////////////////////////////////////////////////////////////////

QCADUndoEntry *qcad_undo_entry_new_with_callbacks (GCallback callback, gpointer data, GDestroyNotify destroy_data)
  {
  QCADUndoEntry *entry = qcad_undo_entry_new () ;

  qcad_undo_entry_signal_connect (entry, callback, data, destroy_data) ;

  return entry ;
  }

void qcad_undo_entry_signal_connect (QCADUndoEntry *entry, GCallback callback, gpointer data, GDestroyNotify destroy_data)
  {
  g_signal_connect (G_OBJECT (entry), "apply", callback, data) ;

  g_object_weak_ref (G_OBJECT (entry), (GWeakNotify)destroy_data, data) ;
  }

QCADUndoEntry *qcad_undo_entry_new ()
  {return g_object_new (QCAD_TYPE_UNDO_ENTRY, NULL) ;}

void qcad_undo_entry_fire (QCADUndoEntry *undo_entry, gboolean bUndo)
  {QCAD_UNDO_ENTRY_GET_CLASS (undo_entry)->fire (undo_entry, bUndo) ;}

///////////////////////////////////////////////////////////////////////////////

static void fire (QCADUndoEntry *undo_entry, gboolean bUndo)
  {g_signal_emit (G_OBJECT (undo_entry), qcad_undo_entry_signals[QCAD_UNDO_ENTRY_APPLY_SIGNAL], 0, bUndo) ;}

///////////////////////////////////////////////////////////////////////////////
