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
// A recent files menu based on a file containing the   //
// list of files considered recent by the program, and  //
// a GtkMenu widget.                                    //
//                                                      //
//////////////////////////////////////////////////////////

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <errno.h>
#include "global_consts.h"
#include "recent_files.h"
#include "fileio_helpers.h"
#include "vector_table.h"

#define MAX_RECENT_FILES 10

#define DBG_RF(s)

static char **ppszRecentFiles = NULL ;
static int icRecentFiles = 0 ;
static char *pszRecentFName = NULL ;

extern char current_file_name[PATH_LENGTH] ;

static void ScrollRecentFiles (char **ppszRecentFiles, int idxStart, int idxEnd) ;
static void BuildRecentFilesMenu (GtkWidget *menu, GtkSignalFunc pfn, gpointer data) ;
static void SaveRecentFiles (char **ppszRecentFiles, int icRecentFiles) ;
static void RemoveRecentFile (char *pszFName) ;

// Initialize the menu widget from the file
void fill_recent_files_menu (GtkWidget *menu, GtkSignalFunc pfn, gpointer data)
  {
  FILE *pfile = NULL ;
  char *pszFName = NULL ;

  if (NULL == pszRecentFName)
    pszRecentFName = CreateUserFName ("recent") ;

  // If ppszRecentFiles is not NULL, it means we have either already read the recent files,
  // or there are no recent files stored. In the latter case, it's safe to read the file again.
  if (NULL != ppszRecentFiles)
    return ;

  if (NULL == (pfile = fopen (pszRecentFName, "r")))
    return ;

  while (NULL != (pszFName = ReadLine (pfile, '\0', FALSE)))
    {
    ppszRecentFiles = g_realloc (ppszRecentFiles, ++icRecentFiles * sizeof (char *)) ;
    ppszRecentFiles[icRecentFiles - 1] = pszFName ;
    }
  fclose (pfile) ;
  BuildRecentFilesMenu (menu, pfn, data) ;
  }

// Called upon all successful opens and saves
void add_to_recent_files (GtkWidget *menu, char *pszFName, GtkSignalFunc pfn, gpointer data)
  {
  int Nix ;
  char *pszFound = NULL ;

  for (Nix = 0 ; Nix < icRecentFiles ; Nix++)
    if (!strcmp (pszFName, ppszRecentFiles[Nix]))
      break ;

  if (MAX_RECENT_FILES == Nix)
    {
    ScrollRecentFiles (ppszRecentFiles, 0, Nix - 2) ;
    ppszRecentFiles[0] = g_strdup (pszFName) ;
    }
  else 
  if (icRecentFiles == Nix)
    {
    ppszRecentFiles = realloc (ppszRecentFiles, ++icRecentFiles * sizeof (char *)) ;
    ScrollRecentFiles (ppszRecentFiles, 0, icRecentFiles - 2) ;
    ppszRecentFiles[0] = g_strdup (pszFName) ;
    }
  else
    {
    pszFound = ppszRecentFiles[Nix] ;
    ScrollRecentFiles (ppszRecentFiles, 0, Nix - 1) ;
    ppszRecentFiles[0] = pszFound ;
    }
  BuildRecentFilesMenu (menu, pfn, data) ;
  SaveRecentFiles (ppszRecentFiles, icRecentFiles) ;
  }

// If it fails to open ... 
void remove_recent_file (GtkWidget *menu, char *pszFName, GtkSignalFunc pfn, gpointer data)
  {
  RemoveRecentFile (pszFName) ;
  BuildRecentFilesMenu (menu, pfn, data) ;
  }

// Need to crunch recent files together if one happens to be deleted from the middle
// or a middle one becomes most recent, etc.   
static void ScrollRecentFiles (char **ppszRecentFiles, int idxStart, int idxEnd)
  {
  int Nix ;
  for (Nix = idxEnd ; Nix > idxStart - 1 ; Nix--)
    ppszRecentFiles[Nix + 1] = ppszRecentFiles[Nix] ;
  }

// Build the menu from the list of strings
static void BuildRecentFilesMenu (GtkWidget *menu, GtkSignalFunc pfn, gpointer data)
  {
  int Nix ;
  char *pszBaseName = NULL ;
  GtkWidget *menu_item = NULL ;

  gtk_container_foreach (GTK_CONTAINER (menu), (GtkCallback)gtk_widget_destroy, NULL) ;

  for (Nix = 0 ; Nix < icRecentFiles ; Nix++)
    {
    pszBaseName = base_name (ppszRecentFiles[Nix]) ;

    menu_item = gtk_menu_item_new_with_label (pszBaseName) ;
    gtk_widget_ref (menu_item) ;
    gtk_object_set_data (GTK_OBJECT (menu_item), "file", ppszRecentFiles[Nix]) ;
    gtk_object_set_data (GTK_OBJECT (menu_item), "parent", menu) ;
    gtk_widget_show (menu_item) ;
    gtk_container_add (GTK_CONTAINER (menu), menu_item) ;
    gtk_signal_connect (GTK_OBJECT (menu_item), "activate", pfn, data) ;
    }
  }

// Create ~/.qcadesigner/recent
static void SaveRecentFiles (char **ppszRecentFiles, int icRecentFiles)
  {
  FILE *pfile ;
  int Nix ;

  pfile = fopen (pszRecentFName, "w") ;

  for (Nix = 0 ; Nix < icRecentFiles ; Nix++)
    fprintf (pfile, "%s\n", ppszRecentFiles[Nix]) ;

  fclose (pfile) ;
  }

// Remove a recent file from the array
static void RemoveRecentFile (char *pszFName)
  {
  int Nix ;

  for (Nix = 0 ; Nix < icRecentFiles ; Nix++)
    if (!strcmp (ppszRecentFiles[Nix], pszFName))
      break ;

  if (icRecentFiles == Nix)
    return ;

  g_free (ppszRecentFiles[Nix]) ;

  for (; Nix < icRecentFiles - 1 ; Nix++)
      ppszRecentFiles[Nix] = ppszRecentFiles[Nix + 1] ;

  ppszRecentFiles = realloc (ppszRecentFiles, --icRecentFiles * sizeof (char *)) ;

  SaveRecentFiles (ppszRecentFiles, icRecentFiles) ;
  }
