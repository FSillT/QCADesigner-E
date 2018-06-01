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
// A set of function responsible for creating           //
// QCADUndoEntry objects and hooking up the appropriate //
// callbacks. It is essentially a set of convenience    //
// functions for the Undo/Redo system.                  //
//                                                      //
//////////////////////////////////////////////////////////

#include <string.h>
#include <gdk/gdk.h>
#include "design.h"
#include "selection_renderer.h"
#include "exp_array.h"
#include "selection_undo.h"
#include "objects/QCADCell.h"
#include "objects/QCADUndoEntry.h"
#include "objects/QCADUndoEntryGroup.h"
#include "objects/QCADDOContainer.h"

#define DBG_UNDO_SEL_APPLY(s)

//#define DBG_FREE_STRUCTS
#ifdef DBG_FREE_STRUCTS
  #define DBG_FREE(s) s
#else
  #define DBG_FREE(s)
#endif

enum
  {
  SELECTION_UNDO_MOVE,
  SELECTION_UNDO_ALTERED,
  SELECTION_UNDO_EXISTENCE,
  SELECTION_UNDO_CLOCK,
  SELECTION_UNDO_STATE,
  SELECTION_UNDO_TRANSFORM,
  OBJECT_UNDO_STATE
  } ;

typedef struct
  {
  int type ;
  SELECTION_RENDERER *sr ;
  DESIGN *design ;
  GdkWindow *dst ;
  } UndoSelection ;

typedef struct
  {
  UndoSelection undo ;
  double dxOffset ;
  double dyOffset ;
  } UndoSelectionMove ;

typedef struct
  {
  UndoSelection undo ;
  double mtxFore[2][2] ;
  double mtxBack[2][2] ;
  } UndoSelectionTransform ;

typedef struct
  {
  UndoSelection undo ;
  EXP_ARRAY *objs ;
  } UndoSelectionObjArray ;

typedef struct
  {
  UndoSelectionObjArray ar_undo ;
  char *pszState ;
  GValueArray *old_state ;
  GValue new_state ;
  } UndoSelectionState ;

typedef struct
  {
  UndoSelectionObjArray ar_undo ;
  int relative_clock_change ;
  } UndoSelectionClock ;

typedef struct
  {
  UndoSelectionObjArray ar_undo ;
  gboolean bAdded ;
  } UndoSelectionAltered ;

typedef struct
  {
  UndoSelectionObjArray ar_undo ;
  gboolean bCreated ;
  } UndoSelectionExistence ;

static void undo_sel_apply (QCADUndoEntry *entry, gboolean bUndo, gpointer data) ;

static void undo_selection_object_array_free (gpointer data) ;
static void undo_selection_state_free (gpointer data) ;

#ifdef DBG_FREE_STRUCTS
static char *selection_undo_descriptions[] =
  {
  "SELECTION_UNDO_MOVE",
  "SELECTION_UNDO_ALTERED",
  "SELECTION_UNDO_EXISTENCE",
  "SELECTION_UNDO_CLOCK",
  "OBJECT_UNDO_STATE"
  } ;
#endif

void push_undo_selection_transform (DESIGN *design, SELECTION_RENDERER *sr, GdkWindow *dst, double mtxFore[2][2], double mtxBack[2][2])
  {
  UndoSelectionTransform *pust = NULL ;

  if (NULL == design || NULL == sr || NULL == dst) return ;

  if (NULL == (pust = g_malloc0 (sizeof (UndoSelectionTransform)))) return ;

  pust->undo.type   = SELECTION_UNDO_TRANSFORM ;
  pust->undo.sr     = sr ;
  pust->undo.design = design ;
  pust->undo.dst    = dst ;
  memcpy (pust->mtxFore, mtxFore, 4 * sizeof (double)) ;
  memcpy (pust->mtxBack, mtxBack, 4 * sizeof (double)) ;

  qcad_undo_entry_group_push (qcad_undo_entry_group_get_default (),
    qcad_undo_entry_new_with_callbacks ((GCallback)undo_sel_apply, pust, (GDestroyNotify)g_free)) ;
  }

// This function is responsible for recording the offset the selection has moved for later undo purposes.
// There is a subtle point to be made here: We do not remember all the objects that were moved. Instead,
// we trust that the selection present at the time of the undo is identical to the selection present at
// the time this undo event was created. Thus, we must keep track of all selection alteration events and
// add them to the undo stack.
void push_undo_selection_move (DESIGN *design, SELECTION_RENDERER *sr, GdkWindow *dst, double dxOffset, double dyOffset)
  {
  UndoSelectionMove *pusm = NULL ;

  if (NULL == design || NULL == sr || NULL == dst || (0.0 == dxOffset && 0.0 == dyOffset)) return ;

  if (NULL == (pusm = g_malloc0 (sizeof (UndoSelectionMove)))) return ;

  pusm->undo.type   = SELECTION_UNDO_MOVE ;
  pusm->undo.sr     = sr ;
  pusm->undo.design = design ;
  pusm->undo.dst    = dst ;
  pusm->dxOffset    = dxOffset ;
  pusm->dyOffset    = dyOffset ;

  qcad_undo_entry_group_push (qcad_undo_entry_group_get_default (),
    qcad_undo_entry_new_with_callbacks ((GCallback)undo_sel_apply, pusm, (GDestroyNotify)g_free)) ;
  }

// ... and in the spirit of the previous comment, here's how we keep track of selection alterations :o)
void push_undo_selection_altered (DESIGN *design, SELECTION_RENDERER *sr, GdkWindow *dst, EXP_ARRAY *objs, gboolean bAdded)
  {
  UndoSelectionAltered *pusa = NULL ;

  if (NULL == design || NULL == sr || NULL == dst || NULL == objs) return ;

  if (NULL == (pusa = g_malloc0 (sizeof (UndoSelectionAltered)))) return ;

  pusa->ar_undo.undo.type   = SELECTION_UNDO_ALTERED ;
  pusa->ar_undo.undo.sr     = sr ;
  pusa->ar_undo.undo.design = design ;
  pusa->ar_undo.undo.dst    = dst ;
  pusa->ar_undo.objs        = objs ;
  pusa->bAdded              = bAdded ;

  qcad_undo_entry_group_push (qcad_undo_entry_group_get_default (),
    qcad_undo_entry_new_with_callbacks ((GCallback)undo_sel_apply, pusa, (GDestroyNotify)undo_selection_object_array_free)) ;
  }

void push_undo_selection_existence (DESIGN *design, SELECTION_RENDERER *sr, GdkWindow *dst, EXP_ARRAY *objs, gboolean bCreated)
  {
  UndoSelectionExistence *puse = NULL ;

  if (NULL == design || NULL == sr || NULL == dst || NULL == objs) return ;

  if (NULL == (puse = g_malloc0 (sizeof (UndoSelectionExistence)))) return ;

  puse->ar_undo.undo.type   = SELECTION_UNDO_EXISTENCE ;
  puse->ar_undo.undo.sr     = sr ;
  puse->ar_undo.undo.design = design ;
  puse->ar_undo.undo.dst    = dst ;
  puse->ar_undo.objs        = objs ;
  puse->bCreated            = bCreated ;

  qcad_undo_entry_group_push (qcad_undo_entry_group_get_default (),
    qcad_undo_entry_new_with_callbacks ((GCallback)undo_sel_apply, puse, (GDestroyNotify)undo_selection_object_array_free)) ;
  }

void push_undo_selection_state (DESIGN *design, SELECTION_RENDERER *sr, GdkWindow *dst, EXP_ARRAY *objs, char *state, GValue *value)
  {
  UndoSelectionState *puss = NULL ;
  int Nix ;
  GValue val ;
  QCADDesignObject *obj = NULL ;
  GParamSpec *param_spec = NULL ;

  if (NULL == design || NULL == sr || NULL == dst || NULL == objs || NULL == state) return ;

  if (NULL == (puss = g_malloc0 (sizeof (UndoSelectionState)))) return ;

  puss->ar_undo.undo.type   = SELECTION_UNDO_STATE ;
  puss->ar_undo.undo.sr     = sr ;
  puss->ar_undo.undo.design = design ;
  puss->ar_undo.undo.dst    = dst ;
  puss->ar_undo.objs        = objs ;
  puss->pszState            = g_strdup (state) ;
  puss->old_state           = g_value_array_new (objs->icUsed) ;
  memset (&(puss->new_state), 0, sizeof (GValue)) ;
  g_value_copy (value, g_value_init (&(puss->new_state), G_VALUE_TYPE (value))) ;

  for (Nix = 0 ; Nix < objs->icUsed ; Nix++)
    {
    memset (&val, 0, sizeof (GValue)) ;
    if (NULL != (param_spec = g_object_class_find_property (G_OBJECT_CLASS (QCAD_DESIGN_OBJECT_GET_CLASS (QCAD_DESIGN_OBJECT (obj = exp_array_index_1d (objs, QCADDesignObject *, Nix)))), state)))
      {
      g_value_init (&val, param_spec->value_type) ;
      g_object_get_property (G_OBJECT (obj), state, &val) ;
      }
    else
      g_value_init (&val, G_TYPE_INT) ;
    g_value_array_append (puss->old_state, &val) ;
    g_value_unset (&val) ;
    }

  qcad_undo_entry_group_push (qcad_undo_entry_group_get_default (),
    qcad_undo_entry_new_with_callbacks ((GCallback)undo_sel_apply, puss, (GDestroyNotify)undo_selection_state_free)) ;
  }

void push_undo_selection_clock (DESIGN *design, SELECTION_RENDERER *sr, GdkWindow *dst, EXP_ARRAY *objs, int clock_new, gboolean bRelative)
  {
  UndoSelectionClock *pusc = NULL ;

  if (NULL == design || NULL == sr || NULL == dst || NULL == objs) return ;

  if (bRelative)
    {
    if (NULL == (pusc = g_malloc0 (sizeof (UndoSelectionClock)))) return ;

    pusc->ar_undo.undo.type     = SELECTION_UNDO_CLOCK ;
    pusc->ar_undo.undo.sr       = sr ;
    pusc->ar_undo.undo.design   = design ;
    pusc->ar_undo.undo.dst      = dst ;
    pusc->ar_undo.objs          = objs ;
    pusc->relative_clock_change = clock_new ;

    qcad_undo_entry_group_push (qcad_undo_entry_group_get_default (),
      qcad_undo_entry_new_with_callbacks ((GCallback)undo_sel_apply, pusc, (GDestroyNotify)undo_selection_object_array_free)) ;
    }
  else
    {
    GValue val ;

    memset (&val, 0, sizeof (GValue)) ;
    g_value_init (&val, G_TYPE_UINT) ;
    g_value_set_uint (&val, (guint)clock_new) ;

    push_undo_selection_state (design, sr, dst, objs, "clock", &val) ;

    g_value_unset (&val) ;
    }
  }

void track_undo_object_update (DESIGN *design, SELECTION_RENDERER *sr, GdkWindow *dst, QCADUndoEntry *entry)
  {
  UndoSelection *undo = NULL ;

  if (NULL == (undo = g_malloc0 (sizeof (UndoSelection)))) return ;

  undo->type   = OBJECT_UNDO_STATE ;
  undo->design = design ;
  undo->sr     = sr ;
  undo->dst    = dst ;

  qcad_undo_entry_signal_connect (entry, (GCallback)undo_sel_apply, undo, (GDestroyNotify)g_free) ;
  }

static void undo_selection_state_free (gpointer data)
  {
  UndoSelectionState *puss = (UndoSelectionState *)data ;

  g_free (puss->pszState) ;

  g_value_unset (&(puss->new_state)) ;

  g_value_array_free (puss->old_state) ;

  undo_selection_object_array_free (data) ;
  }

static void undo_selection_object_array_free (gpointer data)
  {
  UndoSelectionObjArray *pusoa = (UndoSelectionObjArray *)data ;

  if (NULL == pusoa) return ;

  DBG_FREE (fprintf (stderr, "undo_selection_object_array_free:Freeing event of type %s\n", selection_undo_descriptions[pusoa->undo.type])) ;
  DBG_FREE (fprintf (stderr, "The object array looks like this:\n")) ;
  DBG_FREE (exp_array_dump (pusoa->objs, stderr, 0)) ;

  design_selection_object_array_free (pusoa->objs) ;

  g_free (pusoa) ;
  }

// Selection-related Undo/Redo
static void undo_sel_apply (QCADUndoEntry *entry, gboolean bUndo, gpointer data)
  {
  UndoSelection *pus = (UndoSelection *)data ;

  DBG_UNDO_SEL_APPLY (fprintf (stderr, "undo_sel_apply:Entering:Design looks like this:\n")) ;
  DBG_UNDO_SEL_APPLY (design_dump (pus->design, stderr)) ;

  selection_renderer_draw (pus->sr, pus->design, pus->dst, GDK_XOR) ;

  switch (pus->type)
    {
    case SELECTION_UNDO_CLOCK :
      {
      int Nix ;
      QCADDesignObject *obj = NULL ;
      UndoSelectionClock *pusc = (UndoSelectionClock *)pus ;
      guint clock_current =  -1 ;

      for (Nix = 0 ; Nix < pusc->ar_undo.objs->icUsed ; Nix++)
        if (NULL != (obj = exp_array_index_1d (pusc->ar_undo.objs, QCADDesignObject *, Nix)))
          if (QCAD_IS_CELL (obj))
            {
            g_object_get (G_OBJECT (obj), "clock", &clock_current, NULL) ;
            g_object_set (G_OBJECT (obj), "clock",
              (bUndo
                ? (pusc->relative_clock_change > 0
                  ? (CLOCK_DEC (clock_current))
                  : (CLOCK_INC (clock_current)))
                : (pusc->relative_clock_change > 0
                  ? (CLOCK_INC (clock_current))
                  : (CLOCK_DEC (clock_current)))), NULL) ;
            }
      break ;
      }

    case SELECTION_UNDO_MOVE:
      {
      UndoSelectionMove *pusm = (UndoSelectionMove *)pus ;

      DBG_UNDO_SEL_APPLY (fprintf (stderr, "undo_sel_apply:SELECTION_UNDO_MOVE:offset = (%lf,%lf)\n", pusm->dxOffset, pusm->dyOffset)) ;

      if (bUndo)
        selection_renderer_move (pus->sr, pus->design, -(pusm->dxOffset), -(pusm->dyOffset)) ;
      else
        selection_renderer_move (pus->sr, pus->design, pusm->dxOffset, pusm->dyOffset) ;

      break ;
      }

    case SELECTION_UNDO_ALTERED:
      {
      QCADDesignObject *obj = NULL ;
      int Nix ;
      UndoSelectionAltered *pusa = (UndoSelectionAltered *)pus ;
      gboolean bSelected =
        (bUndo && !pusa->bAdded) || (!bUndo && pusa->bAdded) ; // bUndo XOR pusa->bAdded

      DBG_UNDO_SEL_APPLY (fprintf (stderr, "undo_sel_apply:SELECTION_UNDO_ALTERED:bSelected = %s\n", bSelected ? "TRUE" : "FALSE")) ;

      for (Nix = 0 ; Nix < pusa->ar_undo.objs->icUsed ; Nix++)
        if (NULL != (obj = exp_array_index_1d (pusa->ar_undo.objs, QCADDesignObject *, Nix)))
          qcad_design_object_set_selected (QCAD_DESIGN_OBJECT (obj), bSelected) ;

      break ;
      }

    case SELECTION_UNDO_TRANSFORM:
      {
      WorldRectangle rcWorld ;
      double xWorld, yWorld ;
      UndoSelectionTransform *pust = (UndoSelectionTransform *)pus ;

      design_get_extents (pus->design, &rcWorld, TRUE) ;
      xWorld = rcWorld.xWorld + rcWorld.cxWorld / 2.0 ;
      yWorld = rcWorld.yWorld + rcWorld.cyWorld / 2.0 ;

      if (bUndo)
        design_selection_transform (pus->design, pust->mtxBack[0][0], pust->mtxBack[0][1], pust->mtxBack[1][0], pust->mtxBack[1][1]) ;
      else
        design_selection_transform (pus->design, pust->mtxFore[0][0], pust->mtxFore[0][1], pust->mtxFore[1][0], pust->mtxFore[1][1]) ;

      design_get_extents (pus->design, &rcWorld, TRUE) ;
      design_selection_move (pus->design,
        xWorld - (rcWorld.xWorld + rcWorld.cxWorld / 2.0),
        yWorld - (rcWorld.yWorld + rcWorld.cyWorld / 2.0)) ;

      break ;
      }

    case SELECTION_UNDO_STATE:
      {
      UndoSelectionState *puss = (UndoSelectionState *)pus ;
      QCADDesignObject *obj = NULL ;
      int Nix ;

      for (Nix = 0 ; Nix < puss->ar_undo.objs->icUsed ; Nix++)
        if (NULL != (obj = exp_array_index_1d (puss->ar_undo.objs, QCADDesignObject *, Nix)))
          if (NULL != g_object_class_find_property (G_OBJECT_CLASS (QCAD_DESIGN_OBJECT_GET_CLASS (QCAD_DESIGN_OBJECT (obj))), puss->pszState))
            g_object_set_property (G_OBJECT (obj), puss->pszState, bUndo ? g_value_array_get_nth (puss->old_state, Nix) : &(puss->new_state)) ;

      break ;
      }

    case SELECTION_UNDO_EXISTENCE:
      {
      int Nix ;
      QCADDesignObject *obj = NULL ;
      UndoSelectionExistence *puse = (UndoSelectionExistence *)pus ;
      gboolean bCreate = (bUndo && !puse->bCreated) || (!bUndo && puse->bCreated) ; // bUndo XOR puse->bCreated

      DBG_UNDO_SEL_APPLY (fprintf (stderr, "undo_sel_apply:SELECTION_UNDO_EXISTENCE:bCreate = %s\n", bCreate ? "TRUE" : "FALSE")) ;

      for (Nix = 0 ; Nix < puse->ar_undo.objs->icUsed ; Nix++)
        if (NULL != (obj = exp_array_index_1d (puse->ar_undo.objs, QCADDesignObject *, Nix)))
          {
          obj = QCAD_DESIGN_OBJECT (obj) ;
          if (bCreate)
            {
            qcad_do_container_add (QCAD_DO_CONTAINER (qcad_layer_from_object (obj)), obj) ;
            DBG_UNDO_SEL_APPLY (fprintf (stderr, "undo_sel_apply:SELECTION_UNDO_EXISTENCE:bCreate:unref-ing object 0x%08X after adding it to the layer\n", (int)obj)) ;
            g_object_unref (obj) ;
            }
          else
            {
            DBG_UNDO_SEL_APPLY (fprintf (stderr, "undo_sel_apply:SELECTION_UNDO_EXISTENCE:!bCreate:ref-ing object 0x%08X before removing it from layer\n", (int)obj)) ;
            g_object_ref (obj) ;
            qcad_do_container_remove (QCAD_DO_CONTAINER (qcad_layer_from_object (obj)), obj) ;
            }
          DBG_UNDO_SEL_APPLY (fprintf (stderr, "The design after undo-%s cells looks like this:\n", bCreate ? "adding" : "removing")) ;
          DBG_UNDO_SEL_APPLY (design_dump (pus->design, stderr)) ;
          }
      break ;
      }

    case OBJECT_UNDO_STATE: // Nothing to do when only the object state has changed
    default:
      break ;
    }

  selection_renderer_update (pus->sr, pus->design) ;
  selection_renderer_draw (pus->sr, pus->design, pus->dst, GDK_XOR) ;

  DBG_UNDO_SEL_APPLY (fprintf (stderr, "undo_sel_apply:After performing undo, design looks like this:\n")) ;
  DBG_UNDO_SEL_APPLY (design_dump (pus->design, stderr)) ;
  DBG_UNDO_SEL_APPLY (fprintf (stderr, "undo_sel_apply:Leaving\n")) ;
  }
