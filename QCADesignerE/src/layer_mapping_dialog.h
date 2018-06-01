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
// layer_mapping_dialog.c                               //
// 2004.10.01                                           //
// author: Mike Mazur                                   //
//                                                      //
// description                                          //
//                                                      //
// Header file for the layer mapping dialog. When       //
// importing a new block, the user must select, for     //
// each block layer, a design layer to merge the block  //
// layer's content into. Alternatively, the user can    //
// choose to create a new layer for a given block       //
// layer.                                               //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _LAYER_MAPPING_DIALOG_H_
#define _LAYER_MAPPING_DIALOG_H_

#include <gtk/gtk.h>
#include "design.h"
#include "exp_array.h"

typedef struct
  {
  QCADLayer *design_layer;
  QCADLayer *block_layer;
  } LAYER_MAPPING;

EXP_ARRAY *get_layer_mapping_from_user (GtkWidget *parent, DESIGN *design, DESIGN *block);

#endif /* _LAYER_MAPPING_DIALOG_H_ */
