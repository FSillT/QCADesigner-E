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
// Header file listing functions from callbacks.c used  //
// by other UI related files like the various actions   //
// in actions/*.c . The project_OP structure is a       //
// collection of all the static variables used by       //
// callbacks.c . This structure could form the basis    //
// for an object-ified main window.                     //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _CALLBACK_HELPERS_H_
#define _CALLBACK_HELPERS_H_

#include <gdk/gdk.h>
#include "design.h"
#include "interface.h"
#include "vector_table.h"
#include "objects/QCADSubstrate.h"
#include "selection_renderer.h"
#include "simulation.h"

typedef struct
  {
  DESIGN *design ;
  GdkColor clrCyan ;
  GdkColor clrWhite ;
  int SIMULATION_ENGINE ;
  float max_response_shift ;
  float affected_cell_probability ;
  gboolean bDesignAltered ;
  int selected_cell_type ;
  gboolean SHOW_GRID ;
  char *pszCurrentFName ;
  int SIMULATION_TYPE ;
  gboolean drop_new_cells ;
  VectorTable *pvt ;
  main_W *main_window ;
  simulation_data *sim_data;
  gboolean bScrolling ;
  SELECTION_RENDERER *srSelection ;
  } project_OP ;

void setup_rulers (int x, int y) ;
void redraw_async (GdkRegion *rgn) ;
void reflect_zoom () ;
void redraw_sync (GdkRegion *rgn, gboolean bDestroyRegion) ;

#endif /* _CALLBACK_HELPERS_H_ */
