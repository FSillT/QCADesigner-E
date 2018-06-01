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
// Header for dealing with mouse handlers.              //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_MOUSE_HANDLERS_H_
#define _OBJECTS_MOUSE_HANDLERS_H_

#ifdef GTK_GUI
  #include <gtk/gtk.h>
#endif /* def GTK_GUI */
#include "QCADDesignObject.h"
#include "mouse_handler_struct.h"

typedef gboolean (*DropFunction) (QCADDesignObject *obj) ;

#ifdef GTK_GUI
void set_mouse_handlers (GType type, MOUSE_HANDLERS *pmh, gpointer data, GtkWidget *widget, DropFunction fcnDrop) ;
#endif /* def GTK_GUI */

#endif /* _OBJECTS_MOUSE_HANDLERS_H_ */
