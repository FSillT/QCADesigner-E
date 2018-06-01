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
// Header file for the file selection window. This      //
// wraps up a GtkFileSelection. After all, all we want  //
// is something that'll return a string containing the  //
// absolute path to a user-selected file name.          //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _FILE_SELECTION_WINDOW_H_
#define _FILE_SELECTION_WINDOW_H_

#include <gtk/gtk.h>

void set_file_selection_file_name (char *pszFName) ;

gchar *get_file_name_from_user (GtkWindow *parent, char *pszWinTitle, char *pszFName, gboolean bOverwritePrompt) ;

gchar *get_external_app (GtkWindow *parent, char *pszWinTitle, char *pszCfgFName, char *pszDefaultContents, gboolean bForceNew) ;

#endif /* _FILE_SELECTION_WINDOW_H_ */
