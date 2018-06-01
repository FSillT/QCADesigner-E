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
// Header for small wrapper functions for opening and   //
// closing print file descriptors based on whether they //
// were process pipes or files.                         //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _PRINT_UTIL_H_
#define _PRINT_UTIL_H_

#include "print.h"

#define PS_RED    "1.00 0.00 0.00"
#define PS_ORANGE "0.83 0.44 0.00"
#define PS_YELLOW "0.66 0.66 0.00"
#define PS_GREEN  "0.00 0.50 0.00"
#define PS_BLUE   "0.21 0.39 0.70"
#define PS_BLACK  "0.00 0.00 0.00"
#define PS_WHITE  "1.00 1.00 1.00"
#define PS_HCFILL "0.90 0.90 0.90"

FILE *OpenPrintStream (print_OP *pPO) ;

void ClosePrintStream  (FILE *pfile, print_OP *pPO) ;

#endif /* _PRINT_UTIL_H_ */
