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
// The honeycomb threshhold dialog. This is where the   //
// user picks the (lower,upper) threshholds for inter-  //
// preting the waveform data points as (logic 0,        //
// logic 1, indeterminate). It is used by the graph     //
// dialog.                                              //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include "support.h"

typedef struct
  {
  GtkWidget *dialog ;
  GtkWidget *lower_threshold_spin ;
  GtkWidget *upper_threshold_spin ;
  } honeycomb_thresholds_D ;

static honeycomb_thresholds_D honeycomb_thresholds_dialog = {NULL} ;

static void create_honeycomb_thresholds_dialog (honeycomb_thresholds_D *dialog) ;

static void spn_value_changed (GtkWidget *widget, gpointer data) ;

gboolean get_honeycomb_thresholds_from_user (GtkWindow *parent, double *pdThreshLower, double *pdThreshUpper)
  {
  gboolean bApply = FALSE ;

  if (NULL == honeycomb_thresholds_dialog.dialog)
    create_honeycomb_thresholds_dialog (&honeycomb_thresholds_dialog) ;

  gtk_window_set_transient_for (GTK_WINDOW (honeycomb_thresholds_dialog.dialog), parent) ;

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (honeycomb_thresholds_dialog.lower_threshold_spin), (*pdThreshLower)) ;
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (honeycomb_thresholds_dialog.upper_threshold_spin), (*pdThreshUpper)) ;

  bApply = (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (honeycomb_thresholds_dialog.dialog))) ;

  gtk_widget_hide (honeycomb_thresholds_dialog.dialog) ;

  if (bApply)
    {
    (*pdThreshLower) = gtk_spin_button_get_value (GTK_SPIN_BUTTON (honeycomb_thresholds_dialog.lower_threshold_spin)) ;
    (*pdThreshUpper) = gtk_spin_button_get_value (GTK_SPIN_BUTTON (honeycomb_thresholds_dialog.upper_threshold_spin)) ;
    }

  return bApply ;
  }

static void create_honeycomb_thresholds_dialog (honeycomb_thresholds_D *dialog)
  {
  GtkWidget *tblThresh = NULL, *lbl = NULL ;

  dialog->dialog = gtk_dialog_new () ;
  gtk_window_set_title (GTK_WINDOW (dialog->dialog), _("Logic Thresholds")) ;
  gtk_window_set_modal (GTK_WINDOW (dialog->dialog), TRUE) ;
  gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), FALSE) ;

  tblThresh = gtk_table_new (2, 2, FALSE) ;
  gtk_widget_show (tblThresh) ;
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog->dialog)->vbox), tblThresh, TRUE, TRUE, 0) ;
  gtk_container_set_border_width (GTK_CONTAINER (tblThresh), 2) ;

  lbl = gtk_label_new (_("Upper Threshold [1]:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tblThresh), lbl, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  dialog->upper_threshold_spin = gtk_spin_button_new (
    GTK_ADJUSTMENT (gtk_adjustment_new (0.5, -1.0, 1.0, 0.005, 0.05, 0.05)), 0.005, 3) ;
  gtk_widget_show (dialog->upper_threshold_spin) ;
  gtk_table_attach (GTK_TABLE (tblThresh), dialog->upper_threshold_spin, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->upper_threshold_spin), TRUE) ;

  lbl = gtk_label_new (_("Lower Threshold [0]:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tblThresh), lbl, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  dialog->lower_threshold_spin = gtk_spin_button_new (
    GTK_ADJUSTMENT (gtk_adjustment_new (-0.5, -1.0, 1.0, 0.005, 0.05, 0.05)), 0.005, 3) ;
  gtk_widget_show (dialog->lower_threshold_spin) ;
  gtk_table_attach (GTK_TABLE (tblThresh), dialog->lower_threshold_spin, 1, 2, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->lower_threshold_spin), TRUE) ;

  gtk_dialog_add_button (GTK_DIALOG (dialog->dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->dialog), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->dialog), GTK_RESPONSE_OK) ;

  g_signal_connect (G_OBJECT (dialog->lower_threshold_spin), "value-changed", (GCallback)spn_value_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->upper_threshold_spin), "value-changed", (GCallback)spn_value_changed, dialog) ;
  }

static void spn_value_changed (GtkWidget *widget, gpointer data)
  {
  static gboolean bIgnoreChange = FALSE ;

  honeycomb_thresholds_D *dialog = (honeycomb_thresholds_D *)data ;
  double dLower = gtk_spin_button_get_value (GTK_SPIN_BUTTON (dialog->lower_threshold_spin)) ;
  double dUpper = gtk_spin_button_get_value (GTK_SPIN_BUTTON (dialog->upper_threshold_spin)) ;

  if (dLower > dUpper)
    {
    bIgnoreChange = TRUE ;
    if (widget == dialog->lower_threshold_spin)
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->upper_threshold_spin), dLower) ;
    else // widget == dialog->upper_threshold_spin
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->lower_threshold_spin), dUpper) ;
    bIgnoreChange = FALSE ;
    }
  }
