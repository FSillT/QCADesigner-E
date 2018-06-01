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
// The simulation type setup dialog. This dialog allows //
// the user to choose whether to perform an exhaustive  //
// simulation or to create a custom vector table.       //
//                                                      //
//////////////////////////////////////////////////////////

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include "global_consts.h"
#include "support.h"
#include "vector_table_options_dialog.h"
#include "sim_type_setup_dialog.h"
#include "vector_table.h"

#define DBG_STS(s)

typedef struct
  {
  GtkWidget *simulation_type_dialog;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GSList    *vbox1_group;
  GtkWidget *digital_verif_radio;
  GtkWidget *vector_table_radio;
  GtkWidget *dialog_action_area1;
  GtkWidget *hbox1;
  GtkWidget *options_button;
  GtkWidget *simulation_type_cancel_button;
  GtkWidget *simulation_type_ok_button;
  } sim_type_setup_D;

static sim_type_setup_D sim_type_setup_dialog = {NULL} ;

static void on_options_button_clicked (GtkButton *button, gpointer user_data);
static void on_vector_table_radio_toggled (GtkButton *button, gpointer user_data);
static void create_sim_type_dialog (sim_type_setup_D *dialog);

void get_sim_type_from_user (GtkWindow *parent, int *piSimType, VectorTable *pvt)
  {
  if (NULL == sim_type_setup_dialog.simulation_type_dialog)
    create_sim_type_dialog (&sim_type_setup_dialog) ;

  g_object_set_data (G_OBJECT (sim_type_setup_dialog.simulation_type_dialog), "pvt", pvt) ;
  gtk_window_set_transient_for (GTK_WINDOW (sim_type_setup_dialog.simulation_type_dialog), parent) ;

  if (VECTOR_TABLE == *piSimType && pvt->inputs->icUsed > 0)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sim_type_setup_dialog.vector_table_radio), TRUE) ;
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sim_type_setup_dialog.digital_verif_radio), TRUE) ;

  gtk_widget_set_sensitive (sim_type_setup_dialog.vector_table_radio, (pvt->inputs->icUsed > 0)) ;
  gtk_widget_set_sensitive (sim_type_setup_dialog.options_button, ((VECTOR_TABLE == *piSimType) && (pvt->inputs->icUsed > 0))) ;

  if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (sim_type_setup_dialog.simulation_type_dialog)))
    if (NULL != piSimType)
      *piSimType = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sim_type_setup_dialog.digital_verif_radio)) ?
        EXHAUSTIVE_VERIFICATION : VECTOR_TABLE ;

  gtk_widget_hide (sim_type_setup_dialog.simulation_type_dialog) ;

  gtk_window_present (parent) ;
  }

static void create_sim_type_dialog (sim_type_setup_D *dialog)
  {
  GtkWidget *imgProps = NULL, *hbox = NULL, *lbl = NULL ;

  if (NULL != dialog->simulation_type_dialog) return ;

  dialog->simulation_type_dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog->simulation_type_dialog), _("Set Simulation Type"));
  gtk_window_set_resizable (GTK_WINDOW (dialog->simulation_type_dialog), FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->simulation_type_dialog), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->simulation_type_dialog)->vbox;
  gtk_widget_show (dialog->dialog_vbox1);

  dialog->vbox1 = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (dialog->vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->vbox1), 2) ;
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->vbox1, TRUE, TRUE, 0);

  dialog->digital_verif_radio = gtk_radio_button_new_with_label (dialog->vbox1_group, _("Exhaustive Verification"));
  dialog->vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (dialog->digital_verif_radio));
  gtk_widget_show (dialog->digital_verif_radio);
  gtk_table_attach (GTK_TABLE (dialog->vbox1), dialog->digital_verif_radio, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->vector_table_radio = gtk_radio_button_new_with_label (dialog->vbox1_group, _("Vector Table"));
  dialog->vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (dialog->vector_table_radio));
  gtk_widget_show (dialog->vector_table_radio);
  gtk_table_attach (GTK_TABLE (dialog->vbox1), dialog->vector_table_radio, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->options_button = gtk_button_new ();
  hbox = gtk_hbox_new (FALSE, 2) ;
  gtk_widget_show (hbox) ;
  imgProps = gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_SMALL_TOOLBAR) ;
  gtk_widget_show (imgProps) ;
  gtk_box_pack_start (GTK_BOX (hbox), imgProps, FALSE, TRUE, 0) ;
  lbl = gtk_label_new (_("Options")) ;
  gtk_widget_show (lbl) ;
  gtk_box_pack_start (GTK_BOX (hbox), lbl, TRUE, TRUE, 0) ;
  gtk_container_add (GTK_CONTAINER (dialog->options_button), hbox) ;
  gtk_widget_show (dialog->options_button);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog->simulation_type_dialog)->action_area), dialog->options_button) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->simulation_type_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->simulation_type_dialog), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->simulation_type_dialog), GTK_RESPONSE_OK) ;

  g_signal_connect (G_OBJECT (dialog->vector_table_radio), "toggled", (GCallback)on_vector_table_radio_toggled, dialog->options_button);
  g_signal_connect (G_OBJECT (dialog->options_button),     "clicked", (GCallback)on_options_button_clicked,     dialog->simulation_type_dialog);
  }

static void on_vector_table_radio_toggled (GtkButton *button, gpointer user_data)
  {gtk_widget_set_sensitive (GTK_WIDGET (user_data), gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button))) ;}

static void on_options_button_clicked (GtkButton *button, gpointer user_data)
  {get_vector_table_options_from_user (GTK_WINDOW (user_data), (VectorTable *)g_object_get_data (G_OBJECT (user_data), "pvt")) ;}
