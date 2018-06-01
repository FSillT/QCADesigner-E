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
// Header for the simulation engine setup dialog. This  //
// dialog allows the user to choose from among the      //
// available simulation engines.                        //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _SIM_ENGINE_SETUP_DIALOG_H_
#define _SIM_ENGINE_SETUP_DIALOG_H_

#include <gtk/gtk.h>

void get_sim_engine_from_user (GtkWindow *parent, int *piSimEng) ;

#endif /* _SIM_ENGINE_SETUP_DIALOG_H_ */
