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
// Header for GTK preamble. This is where we call       //
// gtk_init, register all the stock items, add the      //
// pixmap directories.                                  //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _GTK_PREAMBLE_H_
#define _GTK_PREAMBLE_H_

#ifdef GTK_GUI

#include "support.h"

#ifdef WIN32
  #define QCAD_NO_CONSOLE
#endif /* def WIN32 */

#ifdef QCAD_NO_CONSOLE
void gtk_preamble (int *pargc, char ***pargv, char *pszBaseName, char *pszCmdLine) ;
#else
void gtk_preamble (int *pargc, char ***pargv, char *pszBaseName) ;
#endif /* def QCAD_NO_CONSOLE */

#endif /* def GTK_GUI */

#endif /* def _GTK_PREAMBLE_H_ */
