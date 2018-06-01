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
// The mouse handler structure. For some reason this    //
// to be in a header file on its own ... *shrug* .      //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_MOUSE_HANDLER_STRUCT_H_
#define _OBJECTS_MOUSE_HANDLER_STRUCT_H_

#include <glib.h>

typedef struct
  {
  GCallback button_pressed ;
  GCallback motion_notify ;
  GCallback button_released ;
  } MOUSE_HANDLERS ;

#endif /* _OBJECTS_MOUSE_HANDLER_STRUCT_H_ */
