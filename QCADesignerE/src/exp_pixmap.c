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
// An "expanding pixmap" implementation. This pixmap    //
// does not shrink. Instead, when a pixmap with a       //
// larger area is needed, a new one is created. the     //
// useful size of the pixmap is stored in a             //
// (cxUsed,cyUsed) pair in the structure. Thus, like    //
// the EXP_ARRAY structure, the size of the EXP_PIXMAP  //
// at any given time is that it had at its peak usage.  //
//                                                      //
//////////////////////////////////////////////////////////

#include <gdk/gdk.h>
#include "exp_pixmap.h"

extern GdkColor clrBlack ;

EXP_PIXMAP *exp_pixmap_new (GdkWindow *window, int cx, int cy, int depth)
  {
  EXP_PIXMAP *exp_pixmap = NULL ;

  exp_pixmap = g_malloc0 (sizeof (EXP_PIXMAP)) ;

  exp_pixmap->pixmap = gdk_pixmap_new (window,
    exp_pixmap->cxUsed = exp_pixmap->cxAvail = cx,
    exp_pixmap->cyUsed = exp_pixmap->cyAvail = cy, depth) ;

  return exp_pixmap ;
  }

EXP_PIXMAP *exp_pixmap_cond_new (EXP_PIXMAP *epm, GdkWindow *window, int cx, int cy, int depth)
  {
  if (NULL == epm)
    return exp_pixmap_new (window, cx, cy, depth) ;
  else
    exp_pixmap_resize (epm, cx, cy) ;
  return epm ;
  }

void exp_pixmap_resize (EXP_PIXMAP *exp_pixmap, int cxNew, int cyNew)
  {
  GdkPixmap *new_pixmap = NULL ;

  if (cxNew > exp_pixmap->cxAvail || cyNew > exp_pixmap->cyAvail)
    {
    GdkGC *gc = NULL ;

    new_pixmap = gdk_pixmap_new (exp_pixmap->pixmap, cxNew, cyNew, -1) ;
    gc = gdk_gc_new (new_pixmap) ;
    gdk_draw_drawable (new_pixmap, gc, exp_pixmap->pixmap, 0, 0, 0, 0, -1, -1) ;
    g_object_unref (exp_pixmap->pixmap) ;
    exp_pixmap->pixmap = new_pixmap ;

    exp_pixmap->cxAvail = cxNew ;
    exp_pixmap->cyAvail = cyNew ;
    }

  exp_pixmap->cxUsed = cxNew ;
  exp_pixmap->cyUsed = cyNew ;
  }

EXP_PIXMAP *exp_pixmap_free (EXP_PIXMAP *exp_pixmap)
  {
  if (NULL == exp_pixmap) return NULL ;

  g_object_unref (exp_pixmap->pixmap) ;
  g_free (exp_pixmap) ;

  return NULL ;
  }

void exp_pixmap_clean (EXP_PIXMAP *exp_pixmap)
  {
  GdkGC *gc = NULL ;

  gc = gdk_gc_new (exp_pixmap->pixmap) ;
  gdk_gc_set_function (gc, GDK_COPY) ;
  gdk_gc_set_foreground (gc, &clrBlack) ;
  gdk_gc_set_background (gc, &clrBlack) ;
  gdk_draw_rectangle (exp_pixmap->pixmap, gc, TRUE, 0, 0, exp_pixmap->cxUsed, exp_pixmap->cyUsed) ;
  g_object_unref (gc) ;
  }
