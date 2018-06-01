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
// Header file for mouse handlers for window- and       //
// click-selecting design objects, as well as moving    //
// selections around.                                   //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _ACTIONS_SELECT_H_
#define _ACTIONS_SELECT_H_

#include <gtk/gtk.h>
#include "../objects/QCADDesignObject.h"

gboolean button_pressed_ACTION_SELECT (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
gboolean motion_notify_ACTION_SELECT (GtkWidget *widget, GdkEventMotion *event, gpointer data) ;
gboolean button_released_ACTION_SELECT (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
void ACTION_SELECT_sel_changed (QCADDesignObject *anchor, DESIGN *design) ;

#endif /* _ACTIONS_SELECT_H_ */
