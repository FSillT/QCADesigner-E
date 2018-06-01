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
// The selection renderer. Based on an EXP_PIXMAP, this //
// renders the selection in memory, and then copies the //
// EXP_PIXMAP to a real surface.                        //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "objects/object_helpers.h"
#include "exp_pixmap.h"
#include "selection_renderer.h"

static gboolean sr_configure_event (GtkWidget *widget, GdkEventConfigure *event, gpointer data) ;

extern GdkColor clrBlack ;

SELECTION_RENDERER *selection_renderer_new (GtkWidget *dst)
  {
  int cx, cy ;
  SELECTION_RENDERER *sr = NULL ;

  gdk_window_get_size (dst->window, &cx, &cy) ;

  sr = g_malloc0 (sizeof (SELECTION_RENDERER)) ;

  sr->pixmap = exp_pixmap_new (dst->window, cx, cy, -1) ;
  sr->signal_widget = dst ;
  sr->bEnabled = FALSE ;

  g_signal_connect (G_OBJECT (dst), "configure-event", (GCallback)sr_configure_event, sr) ;

  return sr ;
  }

SELECTION_RENDERER *selection_renderer_free (SELECTION_RENDERER *sr)
  {
  g_signal_handlers_disconnect_matched (G_OBJECT (sr->signal_widget),
    G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)sr_configure_event, (gpointer)sr) ;

  exp_pixmap_free (sr->pixmap) ;
  g_free (sr) ;

  return NULL ;
  }

void selection_renderer_draw (SELECTION_RENDERER *sr, DESIGN *design, GdkWindow *dst, GdkFunction rop)
  {
  GdkGC *gc = NULL ;

  if (!(sr->bEnabled)) return ;

  gc = gdk_gc_new (dst) ;
  gdk_gc_set_function (gc, rop) ;

  gdk_draw_drawable (dst, gc, sr->pixmap->pixmap,
    sr->rcVisible.x, sr->rcVisible.y,
    sr->rcVisible.x + sr->ptOffset.x, sr->rcVisible.y + sr->ptOffset.y,
    sr->rcVisible.width + 1, sr->rcVisible.height + 1) ;
  g_object_unref (gc) ;
  }

void selection_renderer_update (SELECTION_RENDERER *sr, DESIGN *design)
  {
  WorldRectangle rcWorld ;
  GdkRectangle rcDisplay = {0} ;

  rcDisplay.width = sr->pixmap->cxUsed ;
  rcDisplay.height = sr->pixmap->cyUsed ;

  exp_pixmap_clean (sr->pixmap) ;

  if ((sr->bEnabled = design_get_extents (design, &(sr->extSelection), TRUE)))
    {
    sr->ptOffset.x =
    sr->ptOffset.y = 0 ;

    world_to_real_rect (&(sr->extSelection), &(sr->rcSelection)) ;

    if (gdk_rectangle_intersect (&rcDisplay, &(sr->rcSelection), &(sr->rcVisible)))
      design_draw (design, sr->pixmap->pixmap, GDK_COPY, real_to_world_rect (&rcWorld, &(sr->rcVisible)), LAYER_DRAW_SELECTION) ;
    }
  }

void selection_renderer_move (SELECTION_RENDERER *sr, DESIGN *design, double dcxDiff, double dcyDiff)
  {
  GdkRectangle
    rcRelativeVisibleOld,
    rcRelativeVisibleNew,
    rcSelNew = {0},
    rcVisibleNew,
    rcDisplay = {0} ;

  rcDisplay.width = sr->pixmap->cxUsed ;
  rcDisplay.height = sr->pixmap->cyUsed ;

  if (!(0 == dcxDiff && 0 == dcyDiff))
    {
    design_selection_move (design, dcxDiff, dcyDiff) ;

    sr->extSelection.xWorld += dcxDiff ;
    sr->extSelection.yWorld += dcyDiff ;
    }

  world_to_real_rect (&(sr->extSelection), &rcSelNew) ;

  if (RECT_EQUALS_RECT (rcSelNew.x, rcSelNew.y, rcSelNew.width, rcSelNew.height, sr->rcSelection.x + sr->ptOffset.x, sr->rcSelection.y + sr->ptOffset.y, sr->rcSelection.width, sr->rcSelection.height))
    return ;

  if (gdk_rectangle_intersect (&rcSelNew, &rcDisplay, &rcVisibleNew))
    {
    rcRelativeVisibleOld.x = sr->rcVisible.x - sr->rcSelection.x ;
    rcRelativeVisibleOld.y = sr->rcVisible.y - sr->rcSelection.y ;
    rcRelativeVisibleOld.width = sr->rcVisible.width ;
    rcRelativeVisibleOld.height = sr->rcVisible.height ;

    rcRelativeVisibleNew.x = rcVisibleNew.x - rcSelNew.x ;
    rcRelativeVisibleNew.y = rcVisibleNew.y - rcSelNew.y ;
    rcRelativeVisibleNew.width = rcVisibleNew.width ;
    rcRelativeVisibleNew.height = rcVisibleNew.height ;

    if (RECT_IN_RECT (
      rcRelativeVisibleNew.x, rcRelativeVisibleNew.y, rcRelativeVisibleNew.width, rcRelativeVisibleNew.height,
      rcRelativeVisibleOld.x, rcRelativeVisibleOld.y, rcRelativeVisibleOld.width, rcRelativeVisibleOld.height))
      {
      sr->ptOffset.x = rcSelNew.x - sr->rcSelection.x ;
      sr->ptOffset.y = rcSelNew.y - sr->rcSelection.y ;
      return ;
      }
    }
  else
    {
    sr->ptOffset.x = rcSelNew.x - sr->rcSelection.x ;
    sr->ptOffset.y = rcSelNew.y - sr->rcSelection.y ;
    return ;
    }

  selection_renderer_update (sr, design) ;
  }

////////////////////////////////////////////////////////////

static gboolean sr_configure_event (GtkWidget *widget, GdkEventConfigure *event, gpointer data)
  {
  exp_pixmap_resize (((SELECTION_RENDERER *)data)->pixmap, event->width, event->height) ;
  return FALSE ;
  }

////////////////////////////////////////////////////////////
