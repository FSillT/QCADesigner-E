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
// The scale dialog. This dialog allows the user to     //
// enter a scaling factor which then applies to all     //
// cells in the selection, thereby increasing the dot   //
// diameter and the dot spacing.                        //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdlib.h>
#include <gtk/gtk.h>

// At most 3 orders of magnitude either way
#define SCALE_LOWER 0.001
#define SCALE_UPPER 1000

#include "support.h"
#include "scale_dialog.h"

typedef struct
  {
  gint INPUT;
  GtkWidget *scale_dialog;
  GtkWidget *scale_dialog_spin ;
  GtkAdjustment *scale_dialog_adj;
  } scale_D;

//!Dialog for changing the grid scale.
static scale_D scale = {0, NULL};

static void scale_dialog_adj_value_changed (GtkAdjustment *adj, gpointer data) ;

static void create_scale_dialog (scale_D *dialog);

void get_scale_from_user (GtkWindow *parent, double *pdScale)
  {
  if (NULL == scale.scale_dialog)
    create_scale_dialog (&scale) ;

  gtk_widget_grab_focus (scale.scale_dialog_spin) ;
  gtk_window_set_transient_for (GTK_WINDOW (scale.scale_dialog), parent) ;

  if (NULL != pdScale)
    gtk_adjustment_set_value (scale.scale_dialog_adj, (*pdScale)) ;

  if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (scale.scale_dialog)))
    if (NULL != pdScale)
     *pdScale = gtk_adjustment_get_value (scale.scale_dialog_adj) ;
  gtk_widget_hide (scale.scale_dialog) ;
  }

static void create_scale_dialog (scale_D *dialog)
  {
  GtkWidget *dialog_vbox1 = NULL ;
  GtkWidget *hbox1 = NULL ;
  GtkWidget *label1 = NULL ;

  if (NULL != dialog->scale_dialog) return ;

  dialog->scale_dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog->scale_dialog), _("Enter Scale Factor"));
  gtk_window_set_resizable (GTK_WINDOW (dialog->scale_dialog), FALSE) ;
  gtk_window_set_modal (GTK_WINDOW (dialog->scale_dialog), TRUE) ;

  dialog_vbox1 = GTK_DIALOG (dialog->scale_dialog)->vbox;
  gtk_widget_show (dialog_vbox1);

  hbox1 = gtk_table_new (1, 2, FALSE);
  gtk_widget_show (hbox1);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), hbox1, TRUE, TRUE, 0);

  label1 = gtk_label_new (_("Scale Factor:"));
  gtk_widget_show (label1);
  gtk_table_attach (GTK_TABLE (hbox1), label1, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (label1), 1, 0.5);

  dialog->scale_dialog_spin = gtk_spin_button_new (dialog->scale_dialog_adj =
    GTK_ADJUSTMENT (gtk_adjustment_new (1.0, SCALE_LOWER, SCALE_UPPER, 1.0, 10.0, 10.0)), 1.0, 3) ;
  gtk_widget_show (dialog->scale_dialog_spin);
  gtk_table_attach (GTK_TABLE (hbox1), dialog->scale_dialog_spin, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->scale_dialog_spin), TRUE) ;

  g_signal_connect (G_OBJECT (dialog->scale_dialog_adj), "value-changed", (GCallback)scale_dialog_adj_value_changed, dialog->scale_dialog_spin) ;

  gtk_dialog_add_button (GTK_DIALOG (dialog->scale_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->scale_dialog), GTK_STOCK_OK,     GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->scale_dialog), GTK_RESPONSE_OK) ;
  }

static void scale_dialog_adj_value_changed (GtkAdjustment *adj, gpointer data)
  {
  double val = gtk_adjustment_get_value (adj) ;

  if (val <= 2.0)
    gtk_spin_button_set_increments (GTK_SPIN_BUTTON (data), 0.001, 0.01) ;
  else
    gtk_spin_button_set_increments (GTK_SPIN_BUTTON (data), 1.0, 10.0) ;
  }
