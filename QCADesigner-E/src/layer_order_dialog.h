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
// Header file for the layer order dialog, which allows //
// the user to change the vertical order of layers.     //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _LAYER_ORDER_DIALOG_H_
#define _LAYER_ORDER_DIALOG_H_

#include <gtk/gtk.h>
#include "design.h"

GList *get_layer_order_from_user (GtkWindow *parent, DESIGN *design) ;

#endif /* ndef _LAYER_ORDER_DIALOG_H_ */
