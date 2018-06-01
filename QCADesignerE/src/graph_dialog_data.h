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
// Data for the graph dialog. This structure serves the //
// same purpose as the GtkTreeModel does for a          //
// GtkTreeView. It provides a centralized way for the   //
// graph dialog to store the information it fills       //
// itself out with. This header declares the structure  //
// and its constructor/destructor.                      //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _GRAPH_DIALOG_DATA_H_
#define _GRAPH_DIALOG_DATA_H_

#include "simulation_data.h"
#include "bus_layout_dialog.h"

enum
  {
  GRAPH_MODEL_COLUMN_VISIBLE = BUS_LAYOUT_MODEL_COLUMN_LAST,
  GRAPH_MODEL_COLUMN_RULER,
  GRAPH_MODEL_COLUMN_TRACE,
  GRAPH_MODEL_COLUMN_UI,
  GRAPH_MODEL_COLUMN_LAST
  } ;

typedef struct
  {
  simulation_data *sim_data ;
  BUS_LAYOUT *bus_layout ;
  gboolean bFreeSourceData ;
  gboolean bFakeCells ;
  GtkTreeModel *model ;
  double dHCThreshLower ;
  double dHCThreshUpper ;
  int icDrawingArea ;
  int icUIWidget ;
  int cxDrawingArea ;
  int cyDrawingArea ;
  int cxUIWidget ;
  int cyUIWidget ;
  int cxMaxGiven ;
  int cyMaxGiven ;
  int bOneTime ;
  int icGraphLines ;
  int base ;
  double dScale ;
  int xOffset ;
  } GRAPH_DIALOG_DATA ;

GRAPH_DIALOG_DATA *graph_dialog_data_new (SIMULATION_OUTPUT *sim_output, gboolean bOKToFree, double dThreshLower, double dThreshUpper, int base) ;
void graph_dialog_data_free (GRAPH_DIALOG_DATA *gdd) ;

#endif /* def _GRAPH_DIALOG_DATA_H_ */
