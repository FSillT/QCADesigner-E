//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: walus@atips.ca                                //
// WEB: http://qcadesigner.ca/                          //
//////////////////////////////////////////////////////////
//******************************************************//
//*********** PLEASE DO NOT REFORMAT THIS CODE *********//
//******************************************************//
// If your editor wraps long lines disable it or don't  //
// save the core files that way.                        //
// Any independent files you generate format as you wish//
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
// Mouse handlers for panning the scene.                //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include "pan.h"
#include "../objects/object_helpers.h"
#include "../callback_helpers.h"

static int x0, y0, x1, y1 ;

gboolean button_pressed_ACTION_PAN (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  if (1 != event->button) return FALSE ;

  x0 = x1 = event->x ;
  y0 = y1 = event->y ;

  return FALSE ;
  }

gboolean motion_notify_ACTION_PAN (GtkWidget *widget, GdkEventMotion *event, gpointer data)
  {
  int cxPan, cyPan ;

  setup_rulers (event->x, event->y) ;

  if (event->state & GDK_BUTTON1_MASK)
    {
    x1 = event->x ;
    y1 = event->y ;
    cxPan = x1 - x0 ;
    cyPan = y1 - y0 ;

    if (!(0 == cxPan && 0 == cyPan))
      {
      pan (cxPan, cyPan) ;
      gdk_window_scroll (widget->window, cxPan, cyPan) ;

      x0 = x1 ;
      y0 = y1 ;
      }
    }
  return FALSE ;
  }
