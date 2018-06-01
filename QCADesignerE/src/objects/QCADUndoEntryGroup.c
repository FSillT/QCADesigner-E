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
// The undo entry group. An undo entry group contains a //
// bunch of undo entries, and fires each one in se-     //
// quence backwards or forwards.                        //
//                                                      //
//////////////////////////////////////////////////////////

// MAKE SURE TO SKIP OVER GList items containting NULL data !

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib-object.h>
#include "objects_debug.h"
#include "QCADUndoEntryGroup.h"

#define DBG_FINALIZE(s)
#define DBG_PUSH(s)

static void qcad_undo_entry_group_class_init (GObjectClass *klass, gpointer data) ;
static void qcad_undo_entry_group_instance_init (GObject *object, gpointer data) ;
static void qcad_undo_entry_group_instance_finalize (GObject *object) ;

static void fire (QCADUndoEntry *undo_entry, gboolean bUndo) ;

static void qcad_undo_entry_group_prepare_push (QCADUndoEntryGroup *entry_group) ;
static QCADUndoState qcad_undo_entry_group_set_cur (QCADUndoEntryGroup *entry_group, GList *new_cur) ;
static QCADUndoState qcad_undo_entry_group_get_state_from_pointer (QCADUndoEntryGroup *entry_group, GList *llPtr) ;

enum
  {
  QCAD_UNDO_ENTRY_GROUP_STATE_CHANGED_SIGNAL,
  QCAD_UNDO_ENTRY_GROUP_LAST_SIGNAL
  } ;

static guint qcad_undo_entry_group_signals[QCAD_UNDO_ENTRY_GROUP_LAST_SIGNAL] = {0} ;

GType qcad_undo_entry_group_get_type ()
  {
  static GType qcad_undo_entry_group_type = 0 ;

  if (!qcad_undo_entry_group_type)
    {
    static const GTypeInfo qcad_undo_entry_group_info =
      {
      sizeof (QCADUndoEntryGroupClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_undo_entry_group_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADUndoEntryGroup),
      0,
      (GInstanceInitFunc)qcad_undo_entry_group_instance_init
      } ;

    if ((qcad_undo_entry_group_type = g_type_register_static (QCAD_TYPE_UNDO_ENTRY, QCAD_TYPE_STRING_UNDO_ENTRY_GROUP, &qcad_undo_entry_group_info, 0)))
      g_type_class_ref (qcad_undo_entry_group_type) ;
    DBG_OO (fprintf (stderr, "Registered %s as %d\n", QCAD_TYPE_STRING_UNDO_ENTRY_GROUP, (int)qcad_undo_entry_group_type)) ;
    }
  return qcad_undo_entry_group_type ;
  }

static void qcad_undo_entry_group_class_init (GObjectClass *klass, gpointer data)
  {
  DBG_OO (fprintf (stderr, "%s::class_init:Entering.\n", QCAD_TYPE_STRING_UNDO_ENTRY_GROUP)) ;
  qcad_undo_entry_group_signals[QCAD_UNDO_ENTRY_GROUP_STATE_CHANGED_SIGNAL] =
    g_signal_new ("state-changed", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (QCADUndoEntryGroupClass, state_changed), NULL, NULL, g_cclosure_marshal_VOID__ENUM,
      G_TYPE_NONE, 1, G_TYPE_INT) ;

  QCAD_UNDO_ENTRY_CLASS (klass)->fire = fire ;

  G_OBJECT_CLASS (klass)->finalize = qcad_undo_entry_group_instance_finalize ;
  DBG_OO (fprintf (stderr, "%s::class_init:Leaving.\n", QCAD_TYPE_STRING_UNDO_ENTRY_GROUP)) ;
  }

static void qcad_undo_entry_group_instance_init (GObject *object, gpointer data)
  {
  DBG_OO (fprintf (stderr, "%s::instance_init:Entering.\n", QCAD_TYPE_STRING_UNDO_ENTRY_GROUP)) ;
  QCAD_UNDO_ENTRY_GROUP (object)->current_group = NULL ;

  QCAD_UNDO_ENTRY_GROUP (object)->llBeg =
  QCAD_UNDO_ENTRY_GROUP (object)->llCur =
  QCAD_UNDO_ENTRY_GROUP (object)->llEnd = g_list_prepend (NULL, NULL) ;
  DBG_OO (fprintf (stderr, "%s::instance_init:Leaving.\n", QCAD_TYPE_STRING_UNDO_ENTRY_GROUP)) ;
  }

static void qcad_undo_entry_group_instance_finalize (GObject *object)
  {
  GList *llItr = NULL ;
  void (*parent_finalize) (GObject *obj) =
    G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_UNDO_ENTRY_GROUP)))->finalize ;

  for (llItr = QCAD_UNDO_ENTRY_GROUP (object)->llBeg ; llItr != NULL ; llItr = llItr->next)
    if (NULL != llItr->data)
      {
      DBG_FINALIZE (fprintf (stderr, "qcad_undo_entry_group_instance_finalize:Before unref-in entry 0x%08X, the group looks like this:\n", (int)(llItr->data))) ;
      DBG_FINALIZE (qcad_undo_entry_group_dump (QCAD_UNDO_ENTRY_GROUP (object), stderr, 0)) ;
      g_object_unref (llItr->data) ;
      DBG_FINALIZE (fprintf (stderr, "qcad_undo_entry_group_instance_finalize:After unref-in entry 0x%08X, the group looks like this:\n", (int)(llItr->data))) ;
      DBG_FINALIZE (qcad_undo_entry_group_dump (QCAD_UNDO_ENTRY_GROUP (object), stderr, 0)) ;
      }

  g_list_free (QCAD_UNDO_ENTRY_GROUP (object)->llBeg) ;

  if (NULL != parent_finalize)
    (*parent_finalize) (object) ;
  DBG_OO (fprintf (stderr, "%s::instance_finalize:Leaving\n", QCAD_TYPE_STRING_UNDO_ENTRY_GROUP)) ;
  }

///////////////////////////////////////////////////////////////////////////////

QCADUndoEntryGroup *qcad_undo_entry_group_get_default ()
  {
  static QCADUndoEntryGroup *default_entry_group = NULL ;

  if (NULL == default_entry_group)
    {
    default_entry_group = qcad_undo_entry_group_new () ;
    // Add this weak pointer so, when somebody deletes the default entry group, it will be as
    // if it had never been created.
    g_object_add_weak_pointer (G_OBJECT (default_entry_group), (gpointer)&default_entry_group) ;
    }

  return default_entry_group ;
  }

QCADUndoEntryGroup *qcad_undo_entry_group_new ()
  {return g_object_new (QCAD_TYPE_UNDO_ENTRY_GROUP, NULL) ;}

void qcad_undo_entry_group_push (QCADUndoEntryGroup *entry_group, QCADUndoEntry *entry)
  {
  DBG_PUSH (fprintf (stderr, "qcad_undo_entry_group_push:About to push into the following group:\n")) ;
  DBG_PUSH (qcad_undo_entry_group_dump (entry_group, stderr, 0)) ;
  if (NULL == entry_group->current_group)
    {
    qcad_undo_entry_group_prepare_push (entry_group) ;
    entry_group->llBeg = g_list_prepend (entry_group->llBeg, entry) ;
    g_object_add_weak_pointer (G_OBJECT (entry), &(entry_group->llBeg->data)) ;
    qcad_undo_entry_group_set_cur (entry_group, entry_group->llBeg) ;
    }
  else
    qcad_undo_entry_group_push (entry_group->current_group, entry) ;
  DBG_PUSH (fprintf (stderr, "qcad_undo_entry_group_push:The group now looks like this:\n")) ;
  DBG_PUSH (qcad_undo_entry_group_dump (entry_group, stderr, 0)) ;
  }

void qcad_undo_entry_group_push_group (QCADUndoEntryGroup *undo_entry_group, QCADUndoEntryGroup *undo_entry_group_child)
  {
  if (NULL == undo_entry_group->current_group)
    {
    qcad_undo_entry_group_push (undo_entry_group, QCAD_UNDO_ENTRY (undo_entry_group_child)) ;
    undo_entry_group->current_group = undo_entry_group_child ;
    }
  else
    qcad_undo_entry_group_push_group (undo_entry_group->current_group, undo_entry_group_child) ;
  }

gboolean qcad_undo_entry_group_close (QCADUndoEntryGroup *undo_entry_group)
  {
  if (NULL == undo_entry_group->current_group)
    {
    if (NULL != undo_entry_group->llBeg)
      if (NULL == undo_entry_group->llBeg->data)
        // Self-destruct if empty upon closure
        g_object_unref (undo_entry_group) ;
    return TRUE ;
    }
  else
  if (qcad_undo_entry_group_close (undo_entry_group->current_group))
    {
    undo_entry_group->current_group = NULL ;
    return TRUE ;
    }
  return FALSE ;
  }

QCADUndoState qcad_undo_entry_group_undo (QCADUndoEntryGroup *entry_group)
  {
  if (NULL == entry_group->llCur) return 0 ;

  DBG_OO_UNDO (fprintf (stderr, "qcad_undo_entry_group_undo: Entering\n")) ;

  qcad_undo_entry_fire (QCAD_UNDO_ENTRY (entry_group->llCur->data), TRUE) ;

  return qcad_undo_entry_group_set_cur (entry_group, entry_group->llCur->next) ;
  }

QCADUndoState qcad_undo_entry_group_redo (QCADUndoEntryGroup *entry_group)
  {
  QCADUndoState state = 0 ;

  if (NULL == entry_group->llCur) return 0 ;

  state = qcad_undo_entry_group_set_cur (entry_group, entry_group->llCur->prev) ;

  qcad_undo_entry_fire (QCAD_UNDO_ENTRY (entry_group->llCur->data), FALSE) ;

  return state ;
  }

void qcad_undo_entry_group_dump (QCADUndoEntryGroup *entry_group, FILE *pfile, int icIndent)
  {
  GList *llItr = NULL ;

  fprintf (pfile, "%*scurrent_child = 0x%08X\n", icIndent, " ", (int)(entry_group->current_group)) ;
  fprintf (pfile, "%*s|0x%08X|->\n", icIndent, " ", (int)(entry_group->llBeg)) ;
  for (llItr = entry_group->llBeg ; NULL != llItr ; llItr = llItr->next)
    {
    fprintf (pfile, "%*s<-|0x%08X|0x%08X|0x%08X|->", icIndent, " ", (int)(llItr->prev), (int)(llItr->data), (int)(llItr->next)) ;
    fprintf (pfile, "%s\n", (entry_group->llCur == llItr) ? "llCur" : "") ;
    if (NULL != llItr->data)
      if (QCAD_IS_UNDO_ENTRY_GROUP (llItr->data))
        qcad_undo_entry_group_dump (QCAD_UNDO_ENTRY_GROUP (llItr->data), pfile, icIndent + 2) ;
    }
  fprintf (pfile, "%*s<-|0x%08X|\n", icIndent, " ", (int)(entry_group->llEnd)) ;
  }

///////////////////////////////////////////////////////////////////////////////

static void fire (QCADUndoEntry *undo_entry, gboolean bUndo)
  {
  QCADUndoEntryGroup *group = NULL ;

  if (!QCAD_IS_UNDO_ENTRY_GROUP (undo_entry))
    {
    g_print ("WARNING: I am asked to fire an undo entry that is not a QCADUndoEntryGroup!\n") ;
    return ;
    }

  group = QCAD_UNDO_ENTRY_GROUP (undo_entry) ;

  if (bUndo)
    while ((qcad_undo_entry_group_undo (group) & QCAD_CAN_UNDO) != 0) ;
  else
    while ((qcad_undo_entry_group_redo (group) & QCAD_CAN_REDO) != 0) ;
  }

///////////////////////////////////////////////////////////////////////////////

static void qcad_undo_entry_group_prepare_push (QCADUndoEntryGroup *entry_group)
  {
  GList *llItr = NULL ;
  if (entry_group->llCur == entry_group->llBeg || NULL == entry_group->llCur) return ;
  // detach list from llBeg up to and not including llCur
  entry_group->llCur->prev->next = NULL ;
  entry_group->llCur->prev = NULL ;
  // destroy same list
  for (llItr = entry_group->llBeg ; llItr != NULL ; llItr = llItr->next)
    if (NULL != llItr->data)
      g_object_unref (llItr->data) ;
  g_list_free (entry_group->llBeg) ;
  // llCur becomes the beginning of the list
  entry_group->llBeg = entry_group->llCur ;

  DBG_OO_UNDO (fprintf (stderr, "qcad_undo_entry_group_prepare_push: After whacking unredoable elements, the group looks like this:\n")) ;
  }

static QCADUndoState qcad_undo_entry_group_get_state_from_pointer (QCADUndoEntryGroup *entry_group, GList *llPtr)
  {
  QCADUndoState state = QCAD_CAN_UNDO | QCAD_CAN_REDO ;

  if (llPtr == entry_group->llBeg)
    state &= ~QCAD_CAN_REDO ;
  if (llPtr == entry_group->llEnd)
    state &= ~QCAD_CAN_UNDO ;

  return state ;
  }

static QCADUndoState qcad_undo_entry_group_set_cur (QCADUndoEntryGroup *entry_group, GList *new_cur)
  {
  gboolean bEmitSignal = FALSE ;
  QCADUndoState state = 0, old_state = 0 ;

  if (NULL == entry_group || NULL == new_cur)
    return 0 ;

  DBG_OO_UNDO (fprintf (stderr, "qcad_undo_entry_group_set_cur: Attempting to move llCur from 0x%08X to 0x%08X\n", (int)(entry_group->llCur), (int)new_cur)) ;

  state = qcad_undo_entry_group_get_state_from_pointer (entry_group, new_cur) ;

  DBG_OO_UNDO (fprintf (stderr, "qcad_undo_entry_group_set_cur: The state at new_cur is %d\n", state)) ;

  if ((bEmitSignal = (((state & 0x1) ^ ((state >> 1) & 0x1)) != 0)))
    {
    DBG_OO_UNDO (fprintf (stderr, "qcad_undo_entry_group_set_cur: Extreme case: state = %d, so emitting signal\n", state)) ;
    // If I'm going into an extreme state, signal
    g_signal_emit (entry_group, qcad_undo_entry_group_signals[QCAD_UNDO_ENTRY_GROUP_STATE_CHANGED_SIGNAL], 0, state) ;
    }
  else
    {
    // llCur should never be NULL, so disable all Undo/Redo by reporting them impossible
    if (NULL == entry_group->llCur)
      return 0 ;

    old_state = qcad_undo_entry_group_get_state_from_pointer (entry_group, entry_group->llCur) ;
    DBG_OO_UNDO (fprintf (stderr, "qcad_undo_entry_group_set_cur: Non-extreme case: old_state = %d and state = %d\n", old_state, state)) ;
    // If I'm going from an extreme state into a non-extreme state, signal
    if ((((state & 0x1) ^ ((state >> 1) & 0x1)) == 0) &&
      (((old_state & 0x1) ^ ((old_state >> 1) & 0x1)) != 0))
      {
      DBG_OO_UNDO (fprintf (stderr, "qcad_undo_entry_group_set_cur: Non-extreme case: going extreme, so emitting signal\n")) ;
      g_signal_emit (entry_group, qcad_undo_entry_group_signals[QCAD_UNDO_ENTRY_GROUP_STATE_CHANGED_SIGNAL], 0, state) ;
      }
    }

  if (NULL != new_cur)
    entry_group->llCur = new_cur ;

  DBG_OO_UNDO (fprintf (stderr, "qcad_undo_entry_group_set_cur: Returning state = %d\n", state)) ;

  return state ;
  }
