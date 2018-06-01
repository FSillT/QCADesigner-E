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
// Header for print preview: Print to a temporary file  //
// and launch a PostScript viewer with the result.      //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _PRINT_PREVIEW_H_
#define _PRINT_PREVIEW_H_

#include <gtk/gtk.h>
#include "print.h"
void do_print_preview (print_OP *pPrintOp, GtkWindow *parent, void *data, PrintFunction fcnPrint) ;

#endif /* _PRINT_PREVIEW_H_ */
