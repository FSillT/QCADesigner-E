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
// Header file for converting a GdkColor structure      //
// containing an RGB value to one containing an HSL     //
// value and vice versa. The members of the GdkColor    //
// structure (.red, .green, .blue) are abused to hold,  //
// H, S, and L, respectively.                           //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _HSL2RGB_H_
#define _HSL2RGB_H_

#ifdef GTK_GUI
  #include <gtk/gtk.h>
#else
  #include "gdk_structs.h"
#endif

void HSLToRGB (GdkColor *clr) ;
void RGBToHSL (GdkColor *clr) ;

#endif /* _HSL2RGB_H_ */
