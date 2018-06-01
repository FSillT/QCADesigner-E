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
// File selection window. This wraps up a               //
// GtkFileSelection. After all, all we want is          //
// something that'll return a string containing the ab- //
// solute path to a user-selected file name.            //
//                                                      //
//////////////////////////////////////////////////////////
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

#include "support.h"
#include "file_selection_window.h"
#include "fileio_helpers.h"
#include "custom_widgets.h"
#include "global_consts.h"

#define DBG_FSW(s) s

#define QCA_CELL_LIBRARY PACKAGE_DATA_DIR G_DIR_SEPARATOR_S PACKAGE G_DIR_SEPARATOR_S "qca_cell_library"

static gboolean filesel_ok_button_activate (GtkWidget *widget, GdkEventButton *ev, gpointer data) ;
static char *get_default_file_name (GtkFileSelection *file_sel, char *pszFName) ;

static GtkWidget *file_selection = NULL ;

gchar *get_file_name_from_user (GtkWindow *parent, char *pszWinTitle, char *pszFName, gboolean bOverwritePrompt)
  {
  gchar *pszRet = NULL ;
  gulong handlerID = 0 ;

  if (NULL == file_selection)
    file_selection = gtk_file_selection_new (pszWinTitle) ;

  gtk_window_set_transient_for (GTK_WINDOW (file_selection), parent) ;
  gtk_window_set_title (GTK_WINDOW (file_selection), pszWinTitle) ;

  if (NULL != (pszRet = get_default_file_name (GTK_FILE_SELECTION (file_selection), pszFName)))
    {
    gtk_file_selection_set_filename (GTK_FILE_SELECTION (file_selection), pszRet) ;
    g_free (pszRet) ;
    pszRet = NULL ;
    }

  if (bOverwritePrompt)
    handlerID = g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (file_selection)->ok_button),
      "button_release_event", G_CALLBACK (filesel_ok_button_activate), file_selection) ;

  if ((GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (file_selection))))
    pszRet = g_strdup_printf ("%s", gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selection))) ;
  gtk_widget_hide (file_selection) ;

  if (bOverwritePrompt)
    g_signal_handler_disconnect (G_OBJECT (GTK_FILE_SELECTION (file_selection)->ok_button), handlerID) ;

  return pszRet ;
  }

static gboolean filesel_ok_button_activate (GtkWidget *widget, GdkEventButton *ev, gpointer data)
  {
  char *pszFName = g_strdup (gtk_file_selection_get_filename (GTK_FILE_SELECTION (data))) ;

  if (g_file_test (pszFName, G_FILE_TEST_EXISTS))
    {
    int resp = GTK_RESPONSE_CANCEL ;
    GtkWidget *msg = gtk_message_dialog_new (GTK_WINDOW (data), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "A file named \"%s\" already exists.\nDo you want to replace it with the one you are saving ?", pszFName) ;

    gtk_dialog_add_button (GTK_DIALOG (msg), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
    gtk_dialog_add_action_widget (GTK_DIALOG (msg), gtk_button_new_with_stock_image (GTK_STOCK_REFRESH, "Replace"), GTK_RESPONSE_OK) ;
    resp = gtk_dialog_run (GTK_DIALOG (msg)) ;
    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;

    if (GTK_RESPONSE_OK != resp)
      gtk_button_released (GTK_BUTTON (widget)) ;
    else
      gtk_button_clicked (GTK_BUTTON (widget)) ;
    g_free (pszFName) ;
    return (GTK_RESPONSE_OK != resp) ;
    }

  g_free (pszFName) ;
  return FALSE ;
  }

void set_file_selection_file_name (char *pszFName)
  {
  if (NULL == pszFName) return ;

  if (NULL == file_selection)
    file_selection = gtk_file_selection_new ("GtkFileSelection") ;

  gtk_file_selection_set_filename (GTK_FILE_SELECTION (file_selection), pszFName) ;
  }

gchar *get_external_app (GtkWindow *parent, char *pszWinTitle, char *pszCfgFName, char *pszDefaultContents, gboolean bForceNew)
  {
  char *pszRet = NULL ;
  char *pszUserFName = NULL ;
  char *pszLine = NULL ;
  FILE *pfile = NULL ;
  int Nix, ic ;
#ifdef WIN32
  char szBuf[PATH_LENGTH] = "" ;
#endif

  pszUserFName = CreateUserFName (pszCfgFName) ;
  if (NULL == (pfile = fopen (pszUserFName, "r")))
    pszRet = get_file_name_from_user (parent, pszWinTitle, pszDefaultContents, FALSE) ;
  else
    {
    if (NULL == (pszLine = ReadLine (pfile, 0, FALSE)))
      {
      fclose (pfile) ;
      pszRet = get_file_name_from_user (parent, pszWinTitle, pszDefaultContents, FALSE) ;
      }
    else
      pszRet = g_strdup (pszLine) ;
    fclose (pfile) ;
    }

  if (NULL == pszRet)
    return NULL ;

  if (0 == pszRet[0]) // grabbed empty string from file
    {
    g_free (pszRet) ;
    pszRet = get_file_name_from_user (parent, pszWinTitle, pszDefaultContents, FALSE) ;
    }

  if (NULL == pszRet) // User clicked cancel
    return NULL ;

  if (0 == pszRet[0])
    {
    // After much effort, a command line could not be conjured up.
    // Give the user the bad news.
    GtkWidget *msg = NULL ;
    gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (parent, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
      GTK_BUTTONS_OK, "Unable to locate path!"))) ;
    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;
    g_free (pszRet) ;
    return NULL ;
    }

  ic = strlen (pszRet) ;

  for (Nix = 0 ; Nix < ic ; Nix++)
    if ('\r' == pszRet[Nix] || '\n' == pszRet[Nix])
      {
      pszRet[Nix] = 0 ;
      break ;
      }

#ifdef WIN32
  // In Windoze, we need to perform the extra step of grabbing
  // the DOS-style path corresponding to the previewer path
  GetShortPathName (pszRet, szBuf, PATH_LENGTH) ;
  g_free (pszRet) ;
  pszRet = g_strdup_printf ("%s", szBuf) ;
#endif

  /* Save the previewer to the config file */
  if (NULL != (pfile = fopen (pszUserFName, "w")))
    {
    fprintf (pfile, "%s\n", pszRet) ;
    fclose (pfile) ;
    }

  return pszRet ;
  }

static char *get_default_file_name (GtkFileSelection *file_sel, char *pszFName)
  {
  gboolean bHaveCellLib = FALSE ;
  char *pszCellLibrary = NULL ;
  char *pszOldFName = NULL ;
#ifdef WIN32
  char szPath[PATH_LENGTH] = "" ;
  g_snprintf (szPath, PATH_LENGTH, "%s%s..%sshare%s%s%sqca_cell_library", 
    getenv ("MY_PATH"), G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S, PACKAGE, G_DIR_SEPARATOR_S) ;
  pszCellLibrary = szPath ;
#else
  pszCellLibrary = QCA_CELL_LIBRARY ;
#endif /* def WIN32 */

  pszOldFName = g_strdup (gtk_file_selection_get_filename (file_sel)) ;

  bHaveCellLib = g_file_test (pszCellLibrary, G_FILE_TEST_IS_DIR) ;

  if (NULL != pszFName)
    if (0 != pszFName[0])
      return strdup (pszFName) ;

  if (g_file_test (pszOldFName, G_FILE_TEST_EXISTS) && !g_file_test (pszOldFName, G_FILE_TEST_IS_DIR))
    return pszOldFName ;

  if (bHaveCellLib)
    return g_strdup_printf ("%s%s", pszCellLibrary, G_DIR_SEPARATOR_S) ;

  if (NULL == pszFName) return NULL ;

  return strdup (pszFName) ;
  }
