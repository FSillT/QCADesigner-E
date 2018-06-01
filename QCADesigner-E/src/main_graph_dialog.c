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
// main() wrapper for turning the graph dialog into a   //
// standalone app. This is the "Simulation Results      //
// Viewer". It asks for a file from the user, then      //
// displays the graph dialog with the data therein,     //
// after which the graph dialog takes over.             //
//                                                      //
//////////////////////////////////////////////////////////

#ifdef WIN32
  #include <windows.h>
#endif /* def WIN32 */

#include "support.h"
#include "file_selection_window.h"
#include "graph_dialog.h"
#include "simulation_data.h"
#include "fileio.h"
#include "gtk_preamble.h"
#include "fileio_helpers.h"

#ifdef QCAD_NO_CONSOLE
int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, char *pszCmdLine, int iCmdShow)
#else
int main (int argc, char **argv)
#endif /* def QCAD_NO_CONSOLE */
  {
  char *psz = NULL ;
#ifdef QCAD_NO_CONSOLE
  char **argv = NULL ;
  int argc = 0 ;

  gtk_preamble (&argc, &argv, "graph_dialog", pszCmdLine) ;
#else
  gtk_preamble (&argc, &argv, "graph_dialog") ;
#endif /* def QCAD_NO_CONSOLE */
  SIMULATION_OUTPUT *sim_output = NULL ;

  if (1 == argc)
    {
    if (NULL == (psz = get_file_name_from_user (NULL, _("Open Simulation Results"), NULL, FALSE)))
      return 1 ;
    }
  else
    psz = absolute_path (argv[1]) ;

  if (NULL == (sim_output = open_simulation_output_file (psz)))
    {
    GtkWidget *msg = NULL ;
    gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
      _("Failed to open simulation data file %s !\n"), psz))) ;
    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;
    return 2 ;
    }

  set_file_selection_file_name (psz) ;
  g_free (psz) ;

  show_graph_dialog (NULL, sim_output, TRUE, TRUE) ;

  return 0 ;
  }
