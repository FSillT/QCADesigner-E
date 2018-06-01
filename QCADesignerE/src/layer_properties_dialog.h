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
// The header for the layer properties dialog. This     //
// allows the user to set the name of the layer, set    //
// the status of the layer, and set the default         //
// properties for the various object types admissible   //
// in the layer.                                        //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _ADD_LAYER_DIALOG_H_
#define _ADD_LAYER_DIALOG_H_

#include "design.h"

gboolean get_layer_properties_from_user (GtkWindow *wnd, QCADLayer *layer, gboolean bAllowLayerType, gboolean bAllowLayerStatus) ;

#endif /* _ADD_LAYER_DIALOG_H_ */
