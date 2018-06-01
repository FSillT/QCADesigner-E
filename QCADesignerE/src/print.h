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
// Header file for the QCADesigner design PostScript    //
// printer.                                             //
// Completion Date: June 2003                           //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _PRINT_H_
#define _PRINT_H_

#include "simulation.h"
#include "objects/QCADPrintDialog.h"
#include "design.h"

typedef struct
  {
  print_OP po ;
  // Units are in points == 1/72 inches
  double dPointsPerNano ;
  gboolean bPrintOrderOver ;
  gboolean bCenter ;
  gboolean bFit ;
  gboolean bColour ;
  gboolean *pbPrintedObjs ;
  int icPrintedObjs ;
  int iCXPages ;
  int iCYPages ;
  } print_design_OP ;

typedef struct
  {
  print_OP po ;
  // Units are in points == 1/72 inches
  gboolean bPrintClr ;
  gboolean bPrintOrderOver ;
  int iCXPages ;
  int iCYPages ;
  } print_graph_OP ;

typedef struct
  {
  simulation_data *sim_data ;
  BUS_LAYOUT *bus_layout ;
  EXP_ARRAY *bus_traces ; // HONEYCOMB_DATA *
  int honeycomb_base ;
  } PRINT_GRAPH_DATA ;

typedef void (*PrintFunction) (print_OP *pPO, void *data) ;

void print_world (print_design_OP *pPrintOpts, DESIGN *design) ;
void print_graphs (print_graph_OP *pPrintOpts, PRINT_GRAPH_DATA *print_graph_data) ;

#endif /*_PRINT_H_*/
