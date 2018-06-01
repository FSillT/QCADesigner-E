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
// The bistable properties dialog allows the user to    //
// set the parameters for the bistable simulation en-   //
// gine.                                                //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdlib.h>
#include <gtk/gtk.h>

#include "support.h"
#include "bistable_properties_dialog.h"

typedef struct
  {
  GtkWidget *bistable_properties_dialog;
  GtkWidget *dialog_vbox1;
  GtkWidget *table;
  GtkWidget *label1;
  GtkWidget *label2;
  GtkWidget *label3;
  GtkWidget *label4;
  GtkWidget *label5;
  GtkWidget *label6;
  GtkWidget *lblMaxIter;
  GtkWidget *max_iterations_per_sample_entry;
  GtkWidget *number_of_samples_entry;
  GtkWidget *convergence_tolerance_entry;
  GtkWidget *radius_of_effect_entry;
  GtkWidget *epsilonR_entry;
  GtkWidget *clock_high_entry;
  GtkWidget *clock_low_entry;
  GtkWidget *clock_shift_entry;
  GtkWidget *clock_amplitude_factor_entry;
  GtkWidget *layer_separation_entry;
  GtkWidget *chkRandomizeCells;
  GtkWidget *chkAnimate;
  GtkWidget *dialog_action_area1;
  GtkWidget *hbox2;
  GtkWidget *bistable_properties_ok_button;
  GtkWidget *bistable_properties_cancel_button;
  } bistable_properties_D;

static bistable_properties_D bistable_properties = {NULL};

static void create_bistable_properties_dialog (bistable_properties_D *dialog) ;

void get_bistable_properties_from_user (GtkWindow *parent, bistable_OP *pbo)
  {
  char sz[16] = "" ;
  if (NULL == bistable_properties.bistable_properties_dialog)
    create_bistable_properties_dialog (&bistable_properties) ;

  gtk_window_set_transient_for (GTK_WINDOW (bistable_properties.bistable_properties_dialog), parent) ;

  g_snprintf (sz, 16, "%d", pbo->number_of_samples) ;
  gtk_entry_set_text (GTK_ENTRY (bistable_properties.number_of_samples_entry), sz) ;

  g_snprintf (sz, 16, "%d", pbo->max_iterations_per_sample) ;
  gtk_entry_set_text (GTK_ENTRY (bistable_properties.max_iterations_per_sample_entry), sz) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bistable_properties.chkAnimate),
    pbo->animate_simulation) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bistable_properties.chkRandomizeCells),
    pbo->randomize_cells) ;

  g_snprintf (sz, 16, "%f", pbo->convergence_tolerance) ;
  gtk_entry_set_text (GTK_ENTRY (bistable_properties.convergence_tolerance_entry), sz) ;

  g_snprintf (sz, 16, "%f", pbo->convergence_tolerance) ;
  gtk_entry_set_text (GTK_ENTRY (bistable_properties.convergence_tolerance_entry), sz) ;

  g_snprintf (sz, 16, "%f", pbo->radius_of_effect) ;
  gtk_entry_set_text (GTK_ENTRY (bistable_properties.radius_of_effect_entry), sz) ;

  g_snprintf (sz, 16, "%f", pbo->epsilonR) ;
  gtk_entry_set_text (GTK_ENTRY (bistable_properties.epsilonR_entry), sz) ;

  g_snprintf (sz, 16, "%e", pbo->clock_high) ;
  gtk_entry_set_text (GTK_ENTRY (bistable_properties.clock_high_entry), sz) ;

  g_snprintf (sz, 16, "%e", pbo->clock_low) ;
  gtk_entry_set_text (GTK_ENTRY (bistable_properties.clock_low_entry), sz) ;

  g_snprintf (sz, 16, "%e", pbo->clock_shift) ;
  gtk_entry_set_text (GTK_ENTRY (bistable_properties.clock_shift_entry), sz) ;

  g_snprintf (sz, 16, "%f", pbo->clock_amplitude_factor) ;
  gtk_entry_set_text (GTK_ENTRY (bistable_properties.clock_amplitude_factor_entry), sz) ;

  g_snprintf (sz, 16, "%f", pbo->layer_separation) ;
  gtk_entry_set_text (GTK_ENTRY (bistable_properties.layer_separation_entry), sz) ;

  if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (bistable_properties.bistable_properties_dialog)))
    {
    pbo->max_iterations_per_sample = atoi (gtk_entry_get_text (GTK_ENTRY (bistable_properties.max_iterations_per_sample_entry))) ;
    pbo->number_of_samples =         atoi (gtk_entry_get_text (GTK_ENTRY (bistable_properties.number_of_samples_entry))) ;
    pbo->convergence_tolerance =     atof (gtk_entry_get_text (GTK_ENTRY (bistable_properties.convergence_tolerance_entry))) ;
    pbo->radius_of_effect =          atof (gtk_entry_get_text (GTK_ENTRY (bistable_properties.radius_of_effect_entry))) ;
    pbo->epsilonR =                  atof (gtk_entry_get_text (GTK_ENTRY (bistable_properties.epsilonR_entry))) ;
    pbo->clock_high =                atof (gtk_entry_get_text (GTK_ENTRY (bistable_properties.clock_high_entry))) ;
    pbo->clock_low =                 atof (gtk_entry_get_text (GTK_ENTRY (bistable_properties.clock_low_entry))) ;
    pbo->clock_shift =               atof (gtk_entry_get_text (GTK_ENTRY (bistable_properties.clock_shift_entry))) ;
    pbo->clock_amplitude_factor =    atof (gtk_entry_get_text (GTK_ENTRY (bistable_properties.clock_amplitude_factor_entry))) ;
    pbo->layer_separation =          atof (gtk_entry_get_text (GTK_ENTRY (bistable_properties.layer_separation_entry))) ;
    pbo->animate_simulation =        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (bistable_properties.chkAnimate)) ;
    pbo->randomize_cells =           gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (bistable_properties.chkRandomizeCells)) ;
    }

  gtk_widget_hide (bistable_properties.bistable_properties_dialog) ;

  if (NULL != parent)
    gtk_window_present (parent) ;
  }

static void create_bistable_properties_dialog (bistable_properties_D *dialog)
  {
  if (NULL != dialog->bistable_properties_dialog) return ;

  dialog->bistable_properties_dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog->bistable_properties_dialog), _("Bistable Options"));
  gtk_window_set_resizable (GTK_WINDOW (dialog->bistable_properties_dialog), FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->bistable_properties_dialog), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->bistable_properties_dialog)->vbox;
  gtk_widget_show (dialog->dialog_vbox1);

  dialog->table = gtk_table_new (7, 2, FALSE);
  gtk_widget_show (dialog->table);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->table), 2);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->table, TRUE, TRUE, 0);

  dialog->label1 = gtk_label_new (_("Number Of Samples:"));
  gtk_widget_show (dialog->label1);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->label1, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label1), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label1), 1, 0.5);

  dialog->number_of_samples_entry = gtk_entry_new ();
  gtk_widget_show (dialog->number_of_samples_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->number_of_samples_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->number_of_samples_entry), TRUE) ;

  dialog->label2 = gtk_label_new (_("Convergence Tolerance:"));
  gtk_widget_show (dialog->label2);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->label2, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label2), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label2), 1, 0.5);

  dialog->convergence_tolerance_entry = gtk_entry_new ();
  gtk_widget_show (dialog->convergence_tolerance_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->convergence_tolerance_entry, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->convergence_tolerance_entry), TRUE) ;

  dialog->label3 = gtk_label_new (_("Radius of Effect [nm]:"));
  gtk_widget_show (dialog->label3);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->label3, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label3), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label3), 1, 0.5);

  dialog->radius_of_effect_entry = gtk_entry_new ();
  gtk_widget_show (dialog->radius_of_effect_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->radius_of_effect_entry, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->radius_of_effect_entry), TRUE) ;

  dialog->label4 = gtk_label_new (_("Relative Permittivity:"));
  gtk_widget_show (dialog->label4);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->label4, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label4), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label4), 1, 0.5);

  dialog->epsilonR_entry = gtk_entry_new ();
  gtk_widget_show (dialog->epsilonR_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->epsilonR_entry, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->epsilonR_entry), TRUE) ;

  dialog->label5 = gtk_label_new (_("Clock High:"));
  gtk_widget_show (dialog->label5);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->label5, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label5), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label5), 1, 0.5);

  dialog->clock_high_entry = gtk_entry_new ();
  gtk_widget_show (dialog->clock_high_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->clock_high_entry, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->clock_high_entry), TRUE) ;

  dialog->label6 = gtk_label_new (_("Clock Low:"));
  gtk_widget_show (dialog->label6);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->label6, 0, 1, 5, 6,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label6), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label6), 1, 0.5);

  dialog->clock_low_entry = gtk_entry_new ();
  gtk_widget_show (dialog->clock_low_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->clock_low_entry, 1, 2, 5, 6,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->clock_low_entry), TRUE) ;

  dialog->label6 = gtk_label_new (_("Clock Shift:"));
  gtk_widget_show (dialog->label6);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->label6, 0, 1, 6, 7,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label6), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label6), 1, 0.5);

  dialog->clock_shift_entry = gtk_entry_new ();
  gtk_widget_show (dialog->clock_shift_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->clock_shift_entry, 1, 2, 6, 7,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->clock_shift_entry), TRUE) ;

  dialog->label6 = gtk_label_new (_("Clock Amplitude Factor:"));
  gtk_widget_show (dialog->label6);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->label6, 0, 1, 7, 8,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label6), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label6), 1, 0.5);

  dialog->clock_amplitude_factor_entry = gtk_entry_new ();
  gtk_widget_show (dialog->clock_amplitude_factor_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->clock_amplitude_factor_entry, 1, 2, 7, 8,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->clock_amplitude_factor_entry), TRUE) ;

  dialog->label6 = gtk_label_new (_("Layer Separation:"));
  gtk_widget_show (dialog->label6);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->label6, 0, 1, 8, 9,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label6), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label6), 1, 0.5);

  dialog->layer_separation_entry = gtk_entry_new ();
  gtk_widget_show (dialog->layer_separation_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->layer_separation_entry, 1, 2, 8, 9,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->layer_separation_entry), TRUE) ;

  dialog->lblMaxIter = gtk_label_new (_("Maximum Iterations Per Sample:"));
  gtk_widget_show (dialog->lblMaxIter);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->lblMaxIter, 0, 1, 9, 10,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lblMaxIter), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblMaxIter), 1, 0.5);

  dialog->max_iterations_per_sample_entry = gtk_entry_new ();
  gtk_widget_show (dialog->max_iterations_per_sample_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->max_iterations_per_sample_entry, 1, 2, 9, 10,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->max_iterations_per_sample_entry), TRUE) ;

  dialog->chkRandomizeCells = gtk_check_button_new_with_label (_("Randomize Simulation Order")) ;
  gtk_widget_show (dialog->chkRandomizeCells) ;
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->chkRandomizeCells, 0, 2, 11, 12,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->chkAnimate = gtk_check_button_new_with_label (_("Animate")) ;
  gtk_widget_show (dialog->chkAnimate) ;
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->chkAnimate, 0, 2, 12, 13,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  gtk_dialog_add_button (GTK_DIALOG (dialog->bistable_properties_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->bistable_properties_dialog), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->bistable_properties_dialog), GTK_RESPONSE_OK) ;
  }
