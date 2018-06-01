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
// Mouse handlers for window- and click-selecting       //
// design objects, as well as moving selections around. //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include "../callback_helpers.h"
#include "../custom_widgets.h"
#ifdef UNDO_REDO
#include "../selection_undo.h"
#include "objects/QCADUndoEntry.h"
#include "objects/QCADUndoEntryGroup.h"
#endif /* def UNDO_REDO */

extern GdkColor clrBlack ;

static int xRef = 0, xOld = 0, yRef = 0, yOld = 0, xOffset = 0, yOffset = 0 ;
static double dxOrig = 0.0, dyOrig = 0.0 ;
static int icSelObjs = 0 ;
static gboolean bHaveWindow = FALSE ;
static gboolean bSelectionNeedsSnap = TRUE ;
static QCADDesignObject *objSelRef = NULL ;

static gboolean selection_needs_snap_p (DESIGN *design) ;
static void clean_extents (GdkWindow *dst, GdkRectangle *rcReal) ;

gboolean button_pressed_ACTION_SELECT (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  QCADDesignObject *objNewSelRef = NULL ;
  project_OP *project_options = (project_OP *)data ;

  // If we double-click on an object, bring up its properties
  if (1 == event->button && GDK_2BUTTON_PRESS == event->type && !(event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)))
    {
    QCADDesignObject *obj = NULL ;

    bHaveWindow = FALSE ;

    if (NULL != (obj = design_hit_test (project_options->design, event->x, event->y)))
      {
#ifdef UNDO_REDO
      QCADUndoEntry *entry = NULL ;

      if (qcad_design_object_get_properties (obj, gtk_widget_get_toplevel (widget), &entry))
#else
      if (qcad_design_object_get_properties (obj, gtk_widget_get_toplevel (widget)))
#endif /* def UNDO_REDO */
        {
        project_options->bDesignAltered = TRUE ;
        selection_renderer_draw (project_options->srSelection, project_options->design, widget->window, GDK_XOR) ;
        selection_renderer_update (project_options->srSelection, project_options->design) ;
        selection_renderer_draw (project_options->srSelection, project_options->design, widget->window, GDK_XOR) ;
        }
#ifdef UNDO_REDO
      if (NULL != entry)
        {
        qcad_undo_entry_group_push (qcad_undo_entry_group_get_default (), entry) ;
        if (obj->bSelected)
          track_undo_object_update (project_options->design, project_options->srSelection, widget->window, entry) ;
        }
#endif /* def UNDO_REDO */
      objSelRef = NULL ;
      }
    return FALSE ;
    }

  // If we have a sel ref object that means we still haven't dropped our selection, so we cannot start a new
  // selection, nor can we draw a window.

  objNewSelRef = design_selection_hit_test (project_options->design, event->x, event->y) ;

  if (NULL != objSelRef) 
    {
    if (NULL != objNewSelRef)
      objSelRef = objNewSelRef ;
    return FALSE ;
    }

  if (NULL != (objSelRef = objNewSelRef))
    {
    bHaveWindow = FALSE ;

    dxOrig = objSelRef->x ;
    dyOrig = objSelRef->y ;

    xOffset = world_to_real_x (objSelRef->x) - event->x ;
    yOffset = world_to_real_y (objSelRef->y) - event->y ;

    return FALSE ;
    }

  // Otherwise, if we don't already have a window, then we are creating a new window
  if (!bHaveWindow)
    if ((bHaveWindow = (1 == event->button)))
      {
      xRef = xOld = event->x ;
      yRef = yOld = event->y ;
      return FALSE ;
      }

  return FALSE ;
  }

gboolean motion_notify_ACTION_SELECT (GtkWidget *widget, GdkEventMotion *event, gpointer data)
  {
  project_OP *project_options = (project_OP *)data ;

  // Nothing happens if the left button is not down
  if (!(event->state & GDK_BUTTON1_MASK)) return FALSE ;

  if (bHaveWindow)
    {
    GdkGC *gc = gdk_gc_new (widget->window) ;

    gdk_gc_set_function (gc, GDK_XOR) ;
    gdk_gc_set_foreground (gc, &(project_options->clrWhite)) ;
    if (ABS (xRef - xOld) > 0 && ABS (yRef - yOld) > 0)
      gdk_draw_rectangle (widget->window, gc, FALSE, MIN (xRef, xOld), MIN (yRef, yOld), ABS (xRef - xOld), ABS (yRef - yOld)) ;
    xOld = event->x ;
    yOld = event->y ;
    if (ABS (xRef - xOld) > 0 && ABS (yRef - yOld) > 0)
      gdk_draw_rectangle (widget->window, gc, FALSE, MIN (xRef, xOld), MIN (yRef, yOld), ABS (xRef - xOld), ABS (yRef - yOld)) ;

    g_object_unref (gc) ;

    return FALSE ;
    }

  if (!((project_options->bScrolling) || NULL == objSelRef))
    {
    double xWorld = 0.0, yWorld = 0.0 ;

    xWorld = real_to_world_x (event->x + xOffset) ;
    yWorld = real_to_world_y (event->y + yOffset) ;
    if (bSelectionNeedsSnap)
      world_to_grid_pt (&xWorld, &yWorld) ;

    if (!(xWorld == objSelRef->x && yWorld == objSelRef->y))
      {
      selection_renderer_draw (project_options->srSelection, project_options->design, widget->window, GDK_XOR) ;
      selection_renderer_move (project_options->srSelection, project_options->design, xWorld - objSelRef->x, yWorld - objSelRef->y) ;
      selection_renderer_draw (project_options->srSelection, project_options->design, widget->window, GDK_XOR) ;
      }

    return FALSE ;
    }

  return FALSE ;
  }

gboolean button_released_ACTION_SELECT (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  project_OP *project_options = (project_OP *)data ;

  // We've been moving a selection around
  if (!(bHaveWindow || NULL == objSelRef))
    {
#ifdef UNDO_REDO
    if (!(objSelRef->x == dxOrig && objSelRef->y == dyOrig))
      {
      push_undo_selection_move (project_options->design, project_options->srSelection, widget->window, objSelRef->x - dxOrig, objSelRef->y - dyOrig) ;
      dxOrig = objSelRef->x ;
      dyOrig = objSelRef->y ;
      }
#endif /* def UNDO_REDO */
    if (design_selection_drop (project_options->design))
      {
      project_options->bDesignAltered = TRUE ;
      objSelRef = NULL ;
      }
    }

  if (bHaveWindow)
    {
    GdkRectangle rcReal =
      {
      MIN (xRef, xOld),
      MIN (yRef, yOld),
      ABS (xRef - xOld),
      ABS (yRef - yOld)
      } ;

    // Finish drawing the selection rectangle
    if (rcReal.width > 0 && rcReal.height > 0)
      {
      GdkGC *gc = NULL ;

      gc = gdk_gc_new (widget->window) ;
      gdk_gc_set_function (gc, GDK_XOR) ;
      gdk_gc_set_foreground (gc, &(project_options->clrWhite)) ;
      gdk_draw_rectangle (widget->window, gc, FALSE, rcReal.x, rcReal.y, rcReal.width, rcReal.height) ;
      g_object_unref (gc) ;
      }

    bHaveWindow = FALSE ;

    xOffset = 0 ;
    yOffset = 0 ;

    // Manage the selection
    if (1 == event->button)
      {
      WorldRectangle rcWorld ;
      EXP_ARRAY *arNewObjs = NULL ;
      gboolean bSelIsNew = FALSE ;

      real_to_world_rect (&rcWorld, &rcReal) ;
      if (event->state & GDK_SHIFT_MASK)
        {
        if ((bSelIsNew = (NULL != (arNewObjs = design_selection_add_window (project_options->design, &rcWorld)))))
          {
#ifdef UNDO_REDO
          push_undo_selection_altered (project_options->design, project_options->srSelection, widget->window, arNewObjs, TRUE) ;
#endif /* def UNDO_REDO */
          icSelObjs += arNewObjs->icUsed ;
          }
        }
      else
      if (event->state & GDK_CONTROL_MASK)
        {
        if ((bSelIsNew = (NULL != (arNewObjs = design_selection_subtract_window (project_options->design, widget->window, GDK_COPY, &rcWorld)))))
          {
#ifdef UNDO_REDO
          push_undo_selection_altered (project_options->design, project_options->srSelection, widget->window, arNewObjs, FALSE) ;
#endif /* def UNDO_REDO */
          icSelObjs -= arNewObjs->icUsed ;
          }
        }
      else
        {
#ifdef UNDO_REDO
        QCADUndoEntryGroup *group = NULL ;
#endif /* def UNDO_REDO */
        EXP_ARRAY *arReleasedObjs = NULL ;

        selection_renderer_draw (project_options->srSelection, project_options->design, widget->window, GDK_XOR) ;
        arReleasedObjs = design_selection_release (project_options->design, widget->window, GDK_COPY) ;
        if ((bSelIsNew = (NULL != (arNewObjs = design_selection_create_from_window (project_options->design, &rcWorld)))))
          icSelObjs = arNewObjs->icUsed ;

#ifdef UNDO_REDO
        if (!(NULL == arReleasedObjs || NULL == arNewObjs))
          {
          qcad_undo_entry_group_push_group (qcad_undo_entry_group_get_default (), group = qcad_undo_entry_group_new ()) ;
          push_undo_selection_altered (project_options->design, project_options->srSelection, widget->window, arReleasedObjs, FALSE) ;
          push_undo_selection_altered (project_options->design, project_options->srSelection, widget->window, arNewObjs, TRUE) ;
          qcad_undo_entry_group_close (qcad_undo_entry_group_get_default ()) ;
          }
        else
        if (NULL != arReleasedObjs)
          push_undo_selection_altered (project_options->design, project_options->srSelection, widget->window, arReleasedObjs, FALSE) ;
        else
        if (NULL != arNewObjs)
          push_undo_selection_altered (project_options->design, project_options->srSelection, widget->window, arNewObjs, TRUE) ;
#endif /* def UNDO_REDO */
        }

      selection_renderer_update (project_options->srSelection, project_options->design) ;

      if (bSelIsNew)
        {
        design_get_extents (project_options->design, &rcWorld, TRUE) ;

        if (!(event->state & GDK_CONTROL_MASK))
          {
          clean_extents (widget->window, world_to_real_rect (&rcWorld, &rcReal)) ;
          design_draw (project_options->design, widget->window, GDK_COPY, &rcWorld, LAYER_DRAW_NON_SELECTION) ;
          selection_renderer_draw (project_options->srSelection, project_options->design, widget->window, GDK_XOR) ;
          }

        command_history_message ("Selection extents: (%.2lf,%.2lf)[%.2lfx%.2lf] = %.2lf nm^2 = %.2lf um^2 Objects selected: %d\n",
          rcWorld.xWorld, rcWorld.yWorld, rcWorld.cxWorld, rcWorld.cyWorld,
          rcWorld.cxWorld * rcWorld.cyWorld,
          (rcWorld.cxWorld * rcWorld.cyWorld) / 1.0e6, icSelObjs) ;

        bSelectionNeedsSnap = selection_needs_snap_p (project_options->design) ;
        }
      }

    return FALSE ;
    }

  return FALSE ;
  }

void ACTION_SELECT_sel_changed (QCADDesignObject *anchor, DESIGN *design)
  {
  if (NULL == anchor)
    {
    EXP_ARRAY *arSelObjs = design_selection_get_object_array (design) ;

    if (NULL == arSelObjs)
      {
      objSelRef = NULL ;
      return ;
      }

    icSelObjs = arSelObjs->icUsed ;
    anchor = exp_array_index_1d (arSelObjs, QCADDesignObject *, 0) ;
    design_selection_object_array_free (arSelObjs) ;
    }
  // If we cannot drop the selection, we must simulate a previous failed drop
  if (!design_selection_drop (design))
    {
    bHaveWindow = FALSE ;
    bSelectionNeedsSnap = selection_needs_snap_p (design) ;
    objSelRef = anchor ;
    dxOrig = anchor->x ;
    dyOrig = anchor->y ;
    }
  }

// Design rules: determine if any of the object types in the current selection require snap.
static gboolean selection_needs_snap_p (DESIGN *design)
  {
  GList *lstSelTypes = design_selection_get_type_list (design), *llItr = NULL ;

  for (llItr = lstSelTypes ; llItr != NULL ; llItr = llItr->next)
    if (QCAD_TYPE_CELL      == (G_TYPE_FROM_INSTANCE (llItr->data)) ||
        QCAD_TYPE_SUBSTRATE == (G_TYPE_FROM_INSTANCE (llItr->data)))
      return TRUE ;

  return FALSE ;
  }

static void clean_extents (GdkWindow *dst, GdkRectangle *rcReal)
  {
  GdkGC *gc = NULL ;

  gc = gdk_gc_new (dst) ;
  gdk_gc_set_foreground (gc, &clrBlack) ;
  gdk_gc_set_background (gc, &clrBlack) ;
  gdk_draw_rectangle (dst, gc, TRUE, rcReal->x, rcReal->y, rcReal->width + 1, rcReal->height + 1) ;
  g_object_unref (gc) ;
  }
