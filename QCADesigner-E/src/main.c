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
// The file containing main(). It is responsible for    //
// initializing gtk, dealing with the command line,     //
// loading stock items, and creating the main window.   //
//                                                      //
//////////////////////////////////////////////////////////

#ifdef WIN32
  #include <windows.h>
#endif /* ifdef WIN32 */

#include <locale.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "about.h"
#include "print.h"
#include "bistable_simulation.h"
#include "recent_files.h"
#include "vector_table.h"
#include "qcadstock.h"
#include "global_consts.h"
#include "gtk_preamble.h"

#define DBG_MAIN(s)

//!Print options
print_design_OP print_options ;

extern main_W main_window ;

#ifndef WIN32
  // Can't use WinMain without Win32
  #undef QCAD_NO_CONSOLE
#endif  /* ifndef WIN32 */

#ifdef QCAD_NO_CONSOLE
// Use WinMain and set argc and argv to reasonable values
int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, char *pszCmdLine, int iCmdShow)
#else /* ifndef QCAD_NO_CONSOLE */
// Normally, we have a console
int main (int argc, char *argv[])
#endif /* def QCAD_NO_CONSOLE */
  {
  setlocale(LC_ALL,"C");
struct lconv * lc;
  lc=localeconv();
  printf ("Local Currency Symbol: %s\n",lc->decimal_point);
  GtkWindow *wndAbout = NULL ;
#ifdef WIN32
#ifdef QCAD_NO_CONSOLE
  int argc = 0 ;
  char **argv = NULL ;
#endif
#endif /* ifdef WIN32 */

#ifdef QCAD_NO_CONSOLE
  gtk_preamble (&argc, &argv, "QCADesigner", pszCmdLine) ;
#else
  gtk_preamble (&argc, &argv, "QCADesigner") ;
#endif /* def QCAD_NO_CONSOLE */

wndAbout = show_about_dialog (&(main_window.main_window), TRUE) ;

  // -- Create the main window and the about dialog -- //
  create_main_window (&main_window);

  gtk_window_set_transient_for (wndAbout, GTK_WINDOW (main_window.main_window)) ;

  DBG_MAIN (fprintf (stderr, "Created main window\n")) ;

  // -- Show the main window and the about dialog -- //
  gtk_widget_show (main_window.main_window);

  DBG_MAIN (fprintf (stderr, "Show(ing/n) about dialog\n")) ;
#ifdef STDIO_FILEIO
  // The first command line argument is assumed to be a file name
  if (argc >= 2)
    file_operations ((GtkWidget *)argv[1], (gpointer)FILEOP_CMDLINE) ;
  else
    file_operations (main_window.main_window, (gpointer)FILEOP_AUTOLOAD) ;
#endif /* def STDIO_FILEIO */
  // -- LET'S GO -- //
  gtk_main ();

  // -- Exit -- //
  return 0;
  }//main

