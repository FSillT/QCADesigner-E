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
// Dealing with mouse handlers.                         //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "mouse_handlers.h"
#include "QCADDesignObject.h"

static MOUSE_HANDLERS mhOld = {NULL, NULL, NULL} ;

static void switch_mouse_handlers (GtkWidget *widget, MOUSE_HANDLERS *dst, MOUSE_HANDLERS *src, gpointer data) ;

GType current_type ;
DropFunction drop_function = NULL ;

void set_mouse_handlers (GType type, MOUSE_HANDLERS *pmh, gpointer data, GtkWidget *widget, DropFunction fcnDrop)
  {
  MOUSE_HANDLERS dst = {NULL}, src = {NULL} ;
  QCADDesignObjectClass *klass = NULL ;

  memcpy (&src, &mhOld, sizeof (MOUSE_HANDLERS)) ;

  current_type = type ;
  drop_function = fcnDrop ;

  if (0 != type)
    {
    klass = QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (type)) ;
    memcpy (&dst, &(klass->mh), sizeof (MOUSE_HANDLERS)) ;
    }
  else
  if (NULL != pmh)
    memcpy (&dst, pmh, sizeof (MOUSE_HANDLERS)) ;

  memcpy (&mhOld, &dst, sizeof (MOUSE_HANDLERS)) ;

  switch_mouse_handlers (widget, &dst, &src, data) ;
  }

static void switch_mouse_handlers (GtkWidget *widget, MOUSE_HANDLERS *dst, MOUSE_HANDLERS *src, gpointer data)
  {
  if (NULL != src->button_pressed)
    g_signal_handlers_disconnect_matched (G_OBJECT (widget), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)(src->button_pressed), NULL) ;
  if (NULL != src->motion_notify)
    g_signal_handlers_disconnect_matched (G_OBJECT (widget), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)(src->motion_notify), NULL) ;
  if (NULL != src->button_released)
    g_signal_handlers_disconnect_matched (G_OBJECT (widget), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)(src->button_released), NULL) ;

  if (NULL != dst->button_pressed)
    g_signal_connect (G_OBJECT (widget), "button_press_event", dst->button_pressed, data) ;
  if (NULL != dst->motion_notify)
    g_signal_connect (G_OBJECT (widget), "motion_notify_event", dst->motion_notify, data) ;
  if (NULL != dst->button_released)
    g_signal_connect (G_OBJECT (widget), "button_release_event", dst->button_released, data) ;
  }
