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
// Header for a recent files menu based on a file       //
// containing the list of files considered recent by    //
// the program, and a GtkMenu widget.                   //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _RECENT_FILES_H_
#define _RECENT_FILES_H_

#ifdef STDIO_FILEIO

#include <gtk/gtk.h>

void fill_recent_files_menu (GtkWidget *menu, GtkSignalFunc pfn, gpointer data) ;
void add_to_recent_files (GtkWidget *menu, char *pszFName, GtkSignalFunc pfn, gpointer data) ;
void remove_recent_file (GtkWidget *menu, char *pszFName, GtkSignalFunc pfn, gpointer data) ;

#endif /* def STDIO_FILEIO */
#endif /* _RECENT_FILES_H_ */
