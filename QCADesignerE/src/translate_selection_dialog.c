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
// Selection translation dialog. This allows a user to  //
// translate the currently selected objects by a fixed  //
// (cx,cy) nanometer amount.                            //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include "support.h"
#include "custom_widgets.h"
#include "translate_selection_dialog.h"

typedef struct
  {
  GtkWidget *dlg ;
  GtkAdjustment *adjX ;
  GtkAdjustment *adjY ;
  } translate_selection_D ;

static translate_selection_D dialog ;

static void create_translate_dialog (translate_selection_D *dialog) ;

gboolean get_translation_from_user (GtkWindow *parent, double *pdx, double *pdy)
  {
  gboolean bRet = FALSE ;

  if (NULL == dialog.dlg)
    create_translate_dialog (&dialog) ;

  gtk_window_set_transient_for (GTK_WINDOW (dialog.dlg), parent) ;
  gtk_adjustment_set_value (dialog.adjX, (*pdx)) ;
  gtk_adjustment_set_value (dialog.adjY, (*pdy)) ;
  if ((bRet = (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (dialog.dlg)))))
    {
    (*pdx) = gtk_adjustment_get_value (dialog.adjX) ;
    (*pdy) = gtk_adjustment_get_value (dialog.adjY) ;
    }
  gtk_widget_hide (dialog.dlg) ;

  gtk_window_present (parent) ;
  return bRet ;
  }

static void create_translate_dialog (translate_selection_D *dialog)
  {
  GtkWidget *lbl = NULL, *tbl = NULL, *spn = NULL ;

  dialog->dlg = gtk_dialog_new () ;
  gtk_window_set_title (GTK_WINDOW (dialog->dlg), _("Translate Selection")) ;
  gtk_window_set_resizable (GTK_WINDOW (dialog->dlg), FALSE) ;

  tbl = gtk_table_new (2, 3, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog->dlg)->vbox), tbl) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;

  lbl = gtk_label_new (_("Horizontal:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  dialog->adjX = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, -10000.0, 10000.0, 1.0, 5.0, 5.0)) ;
  spn = gtk_spin_button_new_infinite (dialog->adjX, 1.0, 1, ISB_DIR_UP | ISB_DIR_DN) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (tbl), spn, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_entry_set_activates_default (GTK_ENTRY (spn), TRUE) ;

  lbl = gtk_label_new ("nm") ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 2, 3, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;

  lbl = gtk_label_new (_("Vertical:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  dialog->adjY = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, -10000.0, 10000.0, 1.0, 5.0, 5.0)) ;
  spn = gtk_spin_button_new_infinite (dialog->adjY, 1.0, 1, ISB_DIR_UP | ISB_DIR_DN) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (tbl), spn, 1, 2, 1, 2,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_entry_set_activates_default (GTK_ENTRY (spn), TRUE) ;

  lbl = gtk_label_new ("nm") ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 2, 3, 1, 2,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;

  gtk_dialog_add_button (GTK_DIALOG (dialog->dlg), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->dlg), GTK_STOCK_OK,     GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->dlg), GTK_RESPONSE_OK) ;
  }
