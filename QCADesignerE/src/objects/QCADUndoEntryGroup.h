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
// Header for the undo entry group. An undo entry group //
// contains a bunch of undo entries, and fires each one //
// in sequence backwards or forwards.                   //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADUndoEntryGroup_H_
#define _OBJECTS_QCADUndoEntryGroup_H_

#ifdef UNDO_REDO

#include <stdio.h>
#include <glib-object.h>
#include <glib.h>
#include "QCADUndoEntry.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum
  {
  QCAD_CAN_UNDO = 1 << 0,
  QCAD_CAN_REDO = 1 << 1
  } QCADUndoState ;


typedef struct QCADUndoEntryGroup
  {
  QCADUndoEntry parent_instance ;

  struct QCADUndoEntryGroup *current_group ;

  GList *llBeg ;
  GList *llCur ;
  GList *llEnd ;
  } QCADUndoEntryGroup ;

typedef struct QCADUndoEntryGroupClass
  {
  QCADUndoEntryClass parent_class ;

  /* signals */
  void (*state_changed) (GObject *object, QCADUndoState state, gpointer user_data) ;

  } QCADUndoEntryGroupClass ;

GType qcad_undo_entry_group_get_type () ;

#define QCAD_TYPE_STRING_UNDO_ENTRY_GROUP "QCADUndoEntryGroup"
#define QCAD_TYPE_UNDO_ENTRY_GROUP (qcad_undo_entry_group_get_type ())
#define QCAD_UNDO_ENTRY_GROUP(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_UNDO_ENTRY_GROUP, QCADUndoEntryGroup))
#define QCAD_UNDO_ENTRY_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_UNDO_ENTRY_GROUP, QCADUndoEntryGroupClass))
#define QCAD_IS_UNDO_ENTRY_GROUP(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_UNDO_ENTRY_GROUP))
#define QCAD_IS_UNDO_ENTRY_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_UNDO_ENTRY_GROUP))
#define QCAD_UNDO_ENTRY_GROUP_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_UNDO_ENTRY_GROUP, QCADUndoEntryGroupClass))

///////////////////////////////////////////////////////////////////////////////

QCADUndoEntryGroup *qcad_undo_entry_group_new () ;
QCADUndoEntryGroup *qcad_undo_entry_group_get_default () ;
gboolean qcad_undo_entry_group_close (QCADUndoEntryGroup *undo_entry_group) ;
void qcad_undo_entry_group_push (QCADUndoEntryGroup *undo_entry_group, QCADUndoEntry *undo_entry) ;
void qcad_undo_entry_group_push_group (QCADUndoEntryGroup *undo_entry_group, QCADUndoEntryGroup *undo_entry_group_child) ;
QCADUndoState qcad_undo_entry_group_undo (QCADUndoEntryGroup *entry_group) ;
QCADUndoState qcad_undo_entry_group_redo (QCADUndoEntryGroup *entry_group) ;
void qcad_undo_entry_group_dump (QCADUndoEntryGroup *entry_group, FILE *pfile, int icIndent) ;

#ifdef __cplusplus
}
#endif
#endif /* def UNDO_REDO */
#endif /* _OBJECTS_QCADUndoEntryGroup_H_ */
