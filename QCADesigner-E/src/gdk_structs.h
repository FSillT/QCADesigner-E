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
// Structures required even when there's no GTK. Thus,  //
// they are redefined here.                             //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _GDK_STRUCTS_
#define _GDK_STRUCTS_

#ifndef GTK_GUI

#include <glib.h>

typedef struct
  {
  guint pixel ;
  guint red ;
  guint green ;
  guint blue ;
  } GdkColor ;

typedef struct
  {
  gint x ;
  gint y ;
  guint width ;
  guint height ;
  } GdkRectangle ;

typedef struct
  {
  gint x ;
  gint y ;
  } GdkPoint ;

#else /* def GTK_GUI */

#include <gdk/gdk.h>

#endif /* ndef GTK_GUI */

#endif /* _GDK_STRUCTS_ */
