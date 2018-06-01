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
// Header file for the graph dialog. This displays the  //
// waveforms and the bus values as present in the raw   //
// waveform data. The bus values are interpreted from   //
// the waveform data and an (upper,lower) threshhold    //
// pair chosen by the user. The graph dialog also       //
// allows the user to load and save simulation data.    //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _NEW_GRAPH_DIALOG_H_
#define _NEW_GRAPH_DIALOG_H_

#ifdef GTK_GUI

#include <gtk/gtk.h>
#include "simulation.h"
#include "graph_dialog_interface.h"
#include "graph_dialog_data.h"

void show_graph_dialog (GtkWindow *parent, SIMULATION_OUTPUT *sim_output, gboolean bOKToFree, gboolean bModal) ;

void apply_graph_dialog_data (graph_D *dialog, GRAPH_DIALOG_DATA *dialog_data) ;

#endif /* def GTK_GUI */
#endif /* _NEW_GRAPH_DIALOG_H_ */
