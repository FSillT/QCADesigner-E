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
// Header for the undo entry. This is the basis of the  //
// undo system. An undo entry group contains a bunch of //
// these, and fires each one in sequence backwards or   //
// forwards.                                            //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADUndoEntry_H_
#define _OBJECTS_QCADUndoEntry_H_

#ifdef UNDO_REDO

#include <stdio.h>
#include <glib-object.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
  {
  GObject parent_instance ;
  } QCADUndoEntry ;

typedef struct QCADUndoEntryClass
  {
  GObjectClass parent_class ;
  void (*apply) (GObject *object, gboolean bUndo, gpointer user_data) ;
  void (*fire) (QCADUndoEntry *entry, gboolean bUndo) ;
  } QCADUndoEntryClass ;

GType qcad_undo_entry_get_type () ;

#define QCAD_TYPE_STRING_UNDO_ENTRY "QCADUndoEntry"
#define QCAD_TYPE_UNDO_ENTRY (qcad_undo_entry_get_type ())
#define QCAD_UNDO_ENTRY(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_UNDO_ENTRY, QCADUndoEntry))
#define QCAD_UNDO_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_UNDO_ENTRY, QCADUndoEntryClass))
#define QCAD_IS_UNDO_ENTRY(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_UNDO_ENTRY))
#define QCAD_IS_UNDO_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_UNDO_ENTRY))
#define QCAD_UNDO_ENTRY_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_UNDO_ENTRY, QCADUndoEntryClass))

///////////////////////////////////////////////////////////////////////////////

QCADUndoEntry *qcad_undo_entry_new () ;
QCADUndoEntry *qcad_undo_entry_new_with_callbacks (GCallback callback, gpointer data, GDestroyNotify destroy_data) ;
void qcad_undo_entry_signal_connect (QCADUndoEntry *entry, GCallback callback, gpointer data, GDestroyNotify destroy_data) ;
void qcad_undo_entry_fire (QCADUndoEntry *entry, gboolean bUndo) ;

#ifdef __cplusplus
}
#endif
#endif /* def UNDO_REDO */
#endif /* _OBJECTS_QCADUndoEntry_H_ */
