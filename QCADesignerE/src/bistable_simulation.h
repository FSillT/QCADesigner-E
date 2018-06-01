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
// Contents:                                            //
//                                                      //
// Header file for the bistable simulation engine. This //
// engine treats the circuit in a time-independent      //
// fashion, as a system capable of 2 basis states.      //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _BISTABLE_SIMULATION_H_
#define _BISTABLE_SIMULATION_H_

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "design.h"
#include "vector_table.h"
#include "simulation.h"

typedef struct
  {
	int number_of_samples;
	gboolean animate_simulation;
	double convergence_tolerance;
	double radius_of_effect;
	double epsilonR;
	double clock_high;
	double clock_low;
	double clock_shift;
	double clock_amplitude_factor;
	int max_iterations_per_sample;
	double layer_separation;
	gboolean randomize_cells;
  } bistable_OP;

simulation_data *run_bistable_simulation (int SIMULATION_TYPE, DESIGN *design, bistable_OP *options, VectorTable *pvt);
void bistable_options_dump (bistable_OP *bistable_options, FILE *pfile) ;

#endif /* _BISTABLE_SIMULATION_H_ */
