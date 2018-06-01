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
// Header for the graph printing properties dialog.     //
// This dialog, derived from the general print dialog,  //
// allows the user to specify parameters governing the  //
// printing of graph traces.                            //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _PRINT_GRAPH_PROPERTIES_DIALOG_H_
#define _PRINT_GRAPH_PROPERTIES_DIALOG_H_

#include "print.h"
#include "simulation.h"
#include <gtk/gtk.h>

gboolean get_print_graph_properties_from_user (GtkWindow *parent, print_graph_OP *pPO, PRINT_GRAPH_DATA *print_graph_data) ;
void init_print_graph_options (print_graph_OP *pPO, PRINT_GRAPH_DATA *print_graph_data) ;

#endif /* _PRINT_GRAPH_PROPERTIES_DIALOG_H_ */
