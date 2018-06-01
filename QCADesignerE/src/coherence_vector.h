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
// Header file for the coherence vector time-dependent  //
// simulation engine.                                   //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _COHERENCE_SIMULATION_H_
#define _COHERENCE_SIMULATION_H_

#include <stdlib.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "vector_table.h"
#include "simulation.h"

// Physical Constants //
#define hbar 1.05457266e-34
#define over_hbar 9.48252e33
#define hbar_sqr 1.11212e-68
#define over_hbar_sqr 8.99183e67
#define kB 1.381e-23
#define over_kB 7.24296e22
#define E 1.602e-19

typedef struct
  {
	double T;
	double relaxation;
	double time_step;
	double duration;
	double clock_high;
	double clock_low;
	double clock_shift;
	double clock_amplitude_factor;
	double radius_of_effect;
	double epsilonR;
	double layer_separation;
	int algorithm;
	gboolean randomize_cells;
	gboolean animate_simulation;
  } coherence_OP;

void coherence_options_dump (coherence_OP *coherence_options, FILE *pfile) ;
simulation_data *run_coherence_simulation(int SIMULATION_TYPE, DESIGN *design, coherence_OP *options, VectorTable *pvt);

#endif /* _COHERENCE_SIMULATION_H_ */
