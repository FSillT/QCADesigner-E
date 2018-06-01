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
// Header file for enumerating actions available from   //
// the UI, as well as the structure that array-ifies    //
// them in callbacks.c                                  //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _ACTIONS_H_
#define _ACTIONS_H_

#include "objects/mouse_handler_struct.h"
#include "objects/mouse_handlers.h"
#include "objects/QCADDesignObject.h"

enum
  {
  ACTION_SELECT = 0,
  ACTION_QCELL,
  ACTION_ARRAY,
  ACTION_OVAL,
  ACTION_POLYGON,
  ACTION_SUBSTRATE,
  ACTION_LABEL,
  ACTION_RULER,
  ACTION_PAN,
  ACTION_CHOOSE_SNAP_SOURCE,
  ACTION_ROTATE,
  ACTION_LAST_ACTION
  } ;

typedef struct
  {
  GType type ;
  MOUSE_HANDLERS mh ;
  gpointer data ;
  DropFunction drop_function ;
  } ACTION ;

#endif /* _ACTIONS_H_ */
