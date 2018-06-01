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
// Header for the scale dialog. This dialog allows the  //
// user to enter a scaling factor which then applies to //
// all cells in the selection, thereby increasing the   //
// dot diameter and the dot spacing.                    //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _SCLAE_DIALOG_H_
#define _SCALE_DIALOG_H_

#include <gtk/gtk.h>

void get_scale_from_user (GtkWindow *parent, double *pdGridSpacing) ;

#endif /* _SCALE_DIALOG_H_ */
