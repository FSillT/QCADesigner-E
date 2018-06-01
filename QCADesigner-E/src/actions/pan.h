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
// Header file for mouse handlers for panning the scene.//
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _ACTIONS_PAN_H_
#define _ACTIONS_PAN_H_

#include <gtk/gtk.h>

gboolean button_pressed_ACTION_PAN (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
gboolean motion_notify_ACTION_PAN (GtkWidget *widget, GdkEventMotion *event, gpointer data) ;

#endif /* _ACTIONS_PAN_H_ */
