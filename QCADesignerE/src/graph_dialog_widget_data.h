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
// Header file for functions related to maintaining     //
// HONEYCOMB_DATA and WAVEFORM_DATA structures          //
// necessary for drawing the various traces in the      //
// graph dialog.                                        //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _GRAPH_DIALOG_WIDGET_DATA_H_
#define _GRAPH_DIALOG_WIDGET_DATA_H_

#include "exp_array.h"
#include "objects/QCADCell.h"

enum
  {
  GRAPH_DATA_TYPE_BUS,
  GRAPH_DATA_TYPE_CELL
  } ;

typedef struct
  {
  int data_type ;
  gboolean bNeedCalc ;
  int cxGiven ;
  int cyGiven ;
  int cxWanted ;
  int cyWanted ;
//  int xOffset ;
  GdkColor clr ;
  gboolean bVisible ;
  } GRAPH_DATA ;

typedef struct
  {
  int idxBeg ;
  int idxEnd ;
  GdkPoint pts[6] ;
  long long unsigned int value ;
  } HONEYCOMB ;

typedef struct
  {
  GRAPH_DATA graph_data ;
  EXP_ARRAY *arTraces ; // struct TRACEDATA *
  EXP_ARRAY *arHCs ; // HONEYCOMB
  int icHCSamples ;
  } HONEYCOMB_DATA ;

typedef struct
  {
  GRAPH_DATA graph_data ;
  struct TRACEDATA *trace ;
  EXP_ARRAY *arPoints ; // GdkPoint
  gboolean bStretch ;
  } WAVEFORM_DATA ;

// Percentage of trace cyPixels to use for top and bottom padding
#define MIN_MAX_OFFSET 0.05
// The angle between the horizontal and the side wall of the honeycomb
#define HONEYCOMB_ANGLE ((80.0 * PI) / 180.0)
// The font to use for the honeycomb text
#define FONT_STRING "Courier 14"

//void fit_graph_data_to_window (GRAPH_DATA *graph_data, int cxWindow, int cxWanted, int beg_sample, int end_sample, int icSamples) ;

WAVEFORM_DATA *waveform_data_new (struct TRACEDATA *trace, GdkColor *clr, gboolean bStretch) ;
void calculate_waveform_coords (WAVEFORM_DATA *hc, int icSamples) ;
void waveform_data_free (WAVEFORM_DATA *wf) ;

HONEYCOMB_DATA *honeycomb_data_new (GdkColor *clr) ;
void calculate_honeycomb_array (HONEYCOMB_DATA *hc, int icSamples, double dThreshLower, double dThreshUpper, int base) ;
#ifdef GTK_GUI
int calculate_honeycomb_cxWanted (HONEYCOMB_DATA *hc, int icSamples, int base) ;
#endif /* def GTK_GUI */
void calculate_honeycomb_coords (HONEYCOMB_DATA *hc, int icSamples) ;
void honeycomb_data_free (HONEYCOMB_DATA *hc) ;

#endif /* ndef _GRAPH_DIALOG_WIDGET_DATA_H_ */
