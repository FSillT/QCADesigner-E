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
// Header for an alternative interface for specifying   //
// vector tables, which works with the new vector table //
// library.                                             //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _VECTOR_TABLE_OPTIONS_DIALOG_H_
#define _NEW_VECTOR_TABLE_OPTIONS_DIALOG_H_

#include <gtk/gtk.h>
#include "vector_table.h"

void get_vector_table_options_from_user (GtkWindow *parent, BUS_LAYOUT *bus_layout, int *sim_type, VectorTable *pvt) ;

#endif /* _VECTOR_TABLE_OPTIONS_DIALOG_H_ */
