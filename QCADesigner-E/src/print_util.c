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
// Small wrapper functions for opening and closing      //
// print file descriptors based on whether they were    //
// process pipes or files.                              //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <math.h>
#include "print.h"

FILE *OpenPrintStream (print_OP *pPO)
  {
  if (NULL == pPO) return NULL ;

  if (pPO->bPrintFile)
    return fopen (pPO->pszPrintString, "w") ;
  else
    return popen (pPO->pszPrintString, "w") ;
  }

void ClosePrintStream  (FILE *pfile, print_OP *pPO)
  {
  if (NULL == pfile || NULL == pPO) return ;

  if (pPO->bPrintFile)
    fclose (pfile) ;
  else
    pclose (pfile) ;
  }
