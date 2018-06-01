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
// Header for the honeycomb threshhold dialog. This is  //
// where the user picks the (lower,upper) threshholds   //
// for interpreting the waveform data points as (logic  //
// 0, logic 1, indeterminate). It is used by the graph  //
// dialog.                                              //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _HONEYCOMB_THRESHOLDS_DIALOG_H_
#define _HONEYCOMB_THRESHOLDS_DIALOG_H_

gboolean get_honeycomb_thresholds_from_user (GtkWidget *parent, double *pdThreshLower, double *pdThreshUpper) ;

#endif /* _HONEYCOMB_THRESHOLDS_DIALOG_H_ */
