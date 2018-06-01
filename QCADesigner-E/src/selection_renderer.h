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
// Header for the selection renderer. Based on an       //
// EXP_PIXMAP, this renders the selection in memory,    //
// and then copies the EXP_PIXMAP to a real surface.    //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _SELECTION_RENDERER_H_
#define _SELECTION_RENDERER_H_

#include <gtk/gtk.h>
#include "design.h"
#include "exp_pixmap.h"

typedef struct
  {
  GtkWidget *signal_widget ;
  EXP_PIXMAP *pixmap ;
  WorldRectangle extSelection ;
  GdkRectangle rcSelection ;
  GdkRectangle rcVisible ;
  GdkPoint ptOffset ;
  gboolean bEnabled ;
  } SELECTION_RENDERER ;

SELECTION_RENDERER *selection_renderer_new (GtkWidget *dst) ;
SELECTION_RENDERER *selection_renderer_free   (SELECTION_RENDERER *sr) ;
void                selection_renderer_update (SELECTION_RENDERER *sr, DESIGN *design) ;
void                selection_renderer_draw   (SELECTION_RENDERER *sr, DESIGN *design, GdkWindow *dst, GdkFunction rop) ;
void                selection_renderer_move   (SELECTION_RENDERER *sr, DESIGN *design, double dcxDiff, double dcyDiff) ;

#endif /* _SELECTION_RENDERER_H_ */
