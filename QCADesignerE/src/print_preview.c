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
// Print preview: Print to a temporary file and launch  //
// a PostScript viewer with the result.                 //
//                                                      //
//////////////////////////////////////////////////////////

#ifdef WIN32
  #include <windows.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "file_selection_window.h"
#include "print.h"
#include "print_preview.h"
#include "fileio_helpers.h"
#include "support.h"
#include "generic_utils.h"

#define DBG_PP(s)

void do_print_preview (print_OP *ppo, GtkWindow *parent, void *data, PrintFunction fcnPrint)
  {
  char *pszWinTitle = _("Please Select PostScript Viewer") ;
  char *pszPrintString = ppo->pszPrintString ;
  gboolean bPrintFile = ppo->bPrintFile ;
  FILE *pfile = NULL ;
  int ic = 0, Nix, fd = -1 ;
  char *pszCfgFile = NULL, *pszPreviewer = NULL, *pszFromFile = NULL, *pszFName = NULL, *psz = NULL ;
#ifdef WIN32
  char szBuf[MAX_PATH] = "" ;
#endif
  
  pszCfgFile = CreateUserFName ("previewer") ;
  if (NULL == (pfile = fopen (pszCfgFile, "r")))
    pszPreviewer = get_file_name_from_user (parent, pszWinTitle, "", FALSE) ;
  else
    {
    if (NULL == (pszFromFile = ReadLine (pfile, 0, FALSE)))
      {
      fclose (pfile) ;
      pszPreviewer = get_file_name_from_user (parent, pszWinTitle, "", FALSE) ;
      }
    else
      pszPreviewer = g_strdup_printf ("%s", pszFromFile) ;
    g_free (pszFromFile) ;
    fclose (pfile) ;
    }

  if (NULL == pszPreviewer)
    {
    ppo->pszPrintString = pszPrintString ;
    ppo->bPrintFile = bPrintFile ;
    return ;
    }

  if (0 == pszPreviewer[0]) // grabbed empty string from file
    {
    g_free (pszPreviewer) ;
    pszPreviewer = get_file_name_from_user (parent, pszWinTitle, "", FALSE) ;
    }
  // The user must have cancelled out of the file selection box
  if (NULL == pszPreviewer)
    {
    ppo->pszPrintString = pszPrintString ;
    ppo->bPrintFile = bPrintFile ;
    return ;
    }


  if (0 == pszPreviewer[0])
    {
    // After much effort, a command line for a previewer could not be conjured up.
    // Give the user the bad news
    GtkWidget *msg = NULL ;
    gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (parent, 
      GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
      "Unable to find previewer !"))) ;
    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;
    g_free (pszPreviewer) ;
    ppo->pszPrintString = pszPrintString ;
    ppo->bPrintFile = bPrintFile ;
    return ;
    }

  ic = strlen (pszPreviewer) ;
  for (Nix = 0 ; Nix < ic ; Nix++)
    if ('\r' == pszPreviewer[Nix] || '\n' == pszPreviewer[Nix])
      pszPreviewer[Nix] = 0 ;

  // Save the previewer to the config file
  if (NULL != (pfile = fopen (pszCfgFile, "w")))
    {
    fprintf (pfile, "%s\n", pszPreviewer) ;
    fclose (pfile) ;
    }

#ifdef WIN32
  // In Windoze, we need to perform the extra step of grabbing
  // the DOS-style path corresponding to the previewer path   
  GetShortPathName (pszPreviewer, szBuf, MAX_PATH) ;
  g_free (pszPreviewer) ;
  pszPreviewer = g_strdup_printf ("%s", szBuf) ;
#endif

// At this point, we have the command line for the previewer.
// Now, let's concentrate on the temporary file name for the preview.   

  ppo->pszPrintString = CreateUserFName ("previewXXXXXX") ;
#ifdef WIN32
  mktemp (ppo->pszPrintString) ;
  pfile = fopen (ppo->pszPrintString, "w") ;
  fd = (NULL == pfile ? -1 : -2) ;
  if (NULL != pfile) fclose (pfile) ;

  // In Windoze, we need to perform the extra step of grabbing
  // the DOS-style path corresponding to the preview file name   
  GetShortPathName (ppo->pszPrintString, szBuf, MAX_PATH) ;
  g_free (ppo->pszPrintString) ;
  ppo->pszPrintString = g_strdup_printf ("%s", szBuf) ;
#else
  fd = mkstemp (ppo->pszPrintString) ;
#endif /* WIN32 */

  if (-1 == fd)
    {
    ///After much effort, a temporary file could not be conjured up.
    // Give the user the bad news   
    GtkWidget *msg = NULL ;
    gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (parent, 
      GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
      "Unable to create temporary file for preview !"))) ;
    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;
    g_free (pszPreviewer) ;
    g_free (ppo->pszPrintString) ;
    ppo->pszPrintString = pszPrintString ;
    ppo->bPrintFile = bPrintFile ;
    return ;
    }

  if (-2 != fd) close (fd) ;
  (*fcnPrint) (ppo, data) ;

  pszFName = ppo->pszPrintString ;

  // Restore the 2 ppo fields that were mutilated   
  ppo->pszPrintString = pszPrintString ;
  ppo->bPrintFile = bPrintFile ;

  psz = g_strdup_printf ("%s %s", pszPreviewer, pszFName) ;
  g_free (pszPreviewer) ;
  pszPreviewer = psz ;

  // RunPreviewer takes care of freeing pszPreviewer and pszFName
  RunCmdLineAsync (pszPreviewer, pszFName) ;

  g_free (pszCfgFile) ;
  }

