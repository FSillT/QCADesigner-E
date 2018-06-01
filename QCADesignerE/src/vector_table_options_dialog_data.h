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
// Header for applying the data to the vector table     //
// dialog and harvesting the data from the dialog, as   //
// well as a function to set widgets appropriately      //
// (in)sensitive.                                       //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _VECTOR_TABLE_OPTIONS_DIALOG_DATA_H_
#define _VECTOR_TABLE_OPTIONS_DIALOG_DATA_H_

#include "design.h"
#include "vector_table.h"
#include "vector_table_options_dialog_interface.h"

enum
  {
  VECTOR_TABLE_MODEL_COLUMN_ACTIVE = BUS_LAYOUT_MODEL_COLUMN_LAST,
  VECTOR_TABLE_MODEL_COLUMN_LAST
  } ;

void VectorTableToDialog (vector_table_options_D *dialog, BUS_LAYOUT *bus_layout, int *sim_type, VectorTable *pvt) ;
void DialogToVectorTable (vector_table_options_D *dialog) ;
void vector_table_options_dialog_reflect_state (vector_table_options_D *dialog) ;

#endif /* _VECTOR_TABLE_OPTIONS_DIALOG_DATA_H_ */
