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
// This source handles the small about window that pops //
// up when the program is started.                      //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include <support.h>
#include "about.h"

typedef struct
  {
  GtkWidget *about_dialog;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *pixmap1;
  GtkWidget *about_label;
  GtkWidget *dialog_action_area1;
  GtkWidget *about_ok_button;
  } about_D ;

static about_D about_dialog = {NULL} ;

static void create_about_dialog (about_D *about_dialog) ;
static int hide_about_dialog (gpointer data) ;

GtkWindow *show_about_dialog (GtkWidget **pparent, gboolean bSplash)
  {
  if (NULL == about_dialog.about_dialog)
    create_about_dialog (&about_dialog) ;

  if (bSplash)
    {
    g_object_set_data (G_OBJECT (about_dialog.about_dialog), "bSplashHadParent", (gpointer)FALSE) ;
    g_object_set_data (G_OBJECT (about_dialog.about_dialog), "pparent", pparent) ;

    gtk_window_set_modal (GTK_WINDOW (about_dialog.about_dialog), FALSE) ;
    gtk_dialog_set_has_separator (GTK_DIALOG (about_dialog.about_dialog), FALSE) ;
    gtk_widget_show (about_dialog.about_dialog);
    gtk_widget_hide (about_dialog.dialog_action_area1) ;
    while (gtk_events_pending ())
      gtk_main_iteration () ;
    gtk_timeout_add (50, (GtkFunction)hide_about_dialog, about_dialog.about_dialog) ;
    }
  else
    {
    if (NULL != pparent)
      if (NULL != (*pparent))
        gtk_window_set_transient_for (GTK_WINDOW (about_dialog.about_dialog), GTK_WINDOW (*pparent)) ;
    gtk_widget_show (about_dialog.dialog_action_area1) ;
    gtk_window_set_modal (GTK_WINDOW (about_dialog.about_dialog), TRUE) ;
    gtk_dialog_run (GTK_DIALOG (about_dialog.about_dialog)) ;
    gtk_widget_hide (about_dialog.about_dialog) ;
    }

  return GTK_WINDOW (about_dialog.about_dialog) ;
  }

// -- code for the little about window that pops up each time QCADesigner is started -- //

static void create_about_dialog (about_D *about_dialog)
  {
  char *psz = NULL ;

  about_dialog->about_dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (about_dialog->about_dialog), _("About QCADesigner"));
  gtk_window_set_resizable (GTK_WINDOW (about_dialog->about_dialog), FALSE);

  about_dialog->dialog_vbox1 = GTK_DIALOG (about_dialog->about_dialog)->vbox;
  gtk_widget_show (about_dialog->dialog_vbox1);

  about_dialog->vbox1 = gtk_vbox_new (FALSE, 2);
  gtk_widget_show (about_dialog->vbox1);
  gtk_box_pack_start (GTK_BOX (about_dialog->dialog_vbox1), about_dialog->vbox1, TRUE, TRUE, 0);

#ifdef HAVE_LIBRSVG
  about_dialog->pixmap1 = create_pixmap (about_dialog->about_dialog, "about.svg");
#else
  about_dialog->pixmap1 = create_pixmap (about_dialog->about_dialog, "about.png");
#endif
  gtk_widget_show (about_dialog->pixmap1);
  gtk_box_pack_start (GTK_BOX (about_dialog->vbox1), about_dialog->pixmap1, TRUE, TRUE, 0);

  psz = g_strdup_printf (
    "Extended version of: \n"
    "QCADesigner-E © 2017 Version " VERSION"E \n"
    "%s\n%s\n"
    "G. Schulhof, M. Mazur, T. Dysart, A. Vetteth\n"
    "J. Eskritt, G.A. Jullien, V.S. Dimitrov\n"
    "Dominic A. Antonelli\n"
    "Extended by:\n"
    "F. Sill Torres, P. Niemann",
    _("Protected by Copright 2005 K. Walus"),
    _("Protected by Copright 2017 F. Sill Torres"),
    _("Contributers:"));
  about_dialog->about_label = gtk_label_new (psz);
  gtk_widget_show (about_dialog->about_label);
  gtk_box_pack_start (GTK_BOX (about_dialog->vbox1), about_dialog->about_label, FALSE, FALSE, 2);
  g_free (psz) ;

  about_dialog->dialog_action_area1 = GTK_DIALOG (about_dialog->about_dialog)->action_area ;

  gtk_dialog_add_button (GTK_DIALOG (about_dialog->about_dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE) ;
  gtk_dialog_set_default_response (GTK_DIALOG (about_dialog->about_dialog), GTK_RESPONSE_CLOSE) ;
  }

static int hide_about_dialog (gpointer data)
  {
  GtkWindow **pparent = g_object_get_data (G_OBJECT (data), "pparent") ;
  gboolean bSplashHadParent = (gboolean)g_object_get_data (G_OBJECT (data), "bSplashHadParent") ;

  if (NULL != pparent)
    {
    if (NULL == (*pparent)) return TRUE ;
    if (!GTK_WIDGET_VISIBLE (GTK_WIDGET ((*pparent)))) return TRUE ;
    if (!bSplashHadParent)
      {
      g_object_set_data (G_OBJECT (data), "bSplashHadParent", (gpointer)TRUE) ;
      return TRUE ;
      }
    }

  gtk_widget_hide (GTK_WIDGET (data)) ;
  return FALSE ;
  }
