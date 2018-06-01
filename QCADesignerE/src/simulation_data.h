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
// Header for functions related to the simulation       //
// results data structure itself.                       //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _SIMULATION_DATA_H_
#define _SIMULATION_DATA_H_

#include "design.h"
#include "vector_table.h"
#include "objects/QCADCell.h"

struct TRACEDATA
  {
	// array containing the labels for each trace //
	char  *data_labels;

	// default color to use with trace //
	QCADCellFunction trace_function ;

	int drawtrace;

	// array containing all plot data //
	double *data;
  } ;

typedef struct
  {
	// total number of simulation samples //
	int number_samples;

	// Total number of traces //
	int number_of_traces;

	// Total number of clock zones //
	int number_of_zones;

	// property of trace //
	struct TRACEDATA *trace;

	struct TRACEDATA *clock_data ;
  }simulation_data;

typedef struct
  {
  simulation_data *sim_data ;
  BUS_LAYOUT *bus_layout ;
  gboolean bFakeIOLists ;
  } SIMULATION_OUTPUT ;

void tracedata_get_min_max (struct TRACEDATA *trace, int idxStart, int idxEnd, double *pdMin, double *pdMax) ;
simulation_data *simulation_data_destroy (simulation_data *sim_data);

#endif /* ndef _SIMULATION_DATA_H_ */
