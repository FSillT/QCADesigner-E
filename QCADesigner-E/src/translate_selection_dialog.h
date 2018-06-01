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
// Header for the selection translation dialog. This    //
// allows a user to translate the currently selected    //
// objects by a fixed (cx,cy) nanometer amount.         //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _TRANSLATE_SELECTION_DIALOG_H_
#define _TRANSLATE_SELECTION_DIALOG_H_

#include <gtk/gtk.h>

gboolean get_translation_from_user (GtkWindow *parent, double *pdx, double *pdy) ;

#endif /* ndef _TRANSLATE_SELECTION_DIALOG_H_ */
