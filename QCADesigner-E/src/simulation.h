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
// Header for the main entry point for the simulation   //
// engines, as well as functions common to all engines. //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include "simulation_data.h"

simulation_data *run_simulation (int sim_engine, int sim_type, DESIGN *design, VectorTable *pvt);
int select_cells_in_radius (QCADCell ***sorted_cells, QCADCell *cell, double world_radius, int the_cells_layer, int number_of_cell_layers, int *number_of_cells_in_layer, double layer_ation, QCADCell ***p_selected_cells, int **p_neighbour_layer);
double determine_distance (QCADCell * cell1, QCADCell * cell2, int dot_cell_1, int dot_cell_2, double layer_separation);
void simulation_inproc_data_new (DESIGN *design, int *p_number_of_cell_layers, int **p_number_of_cells_in_layer, QCADCell ****p_sorted_cells) ;
void simulation_inproc_data_free (int *p_number_of_cell_layers, int **p_number_of_cells_in_layer, QCADCell ****p_sorted_cells) ;

#endif /* _SIMULATION_H_ */
