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
// This file contains the creation code for the "Print  //
// Properties" dialog, where the user chooses the para- //
// meters for printing the design.                      //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include "support.h"
#include "custom_widgets.h"
#include "print.h"
#include "print_properties_dialog_interface.h"
#include "print_properties_dialog_callbacks.h"

/* Create it */
void create_print_design_properties_dialog (print_properties_D *dialog, print_design_OP *pPO)
  {
  GtkWidget *tbl = NULL ;

  if (NULL != dialog->dlgPrintProps) return ;

  /* The dialog window */
  dialog->dlgPrintProps = GTK_WIDGET (qcad_print_dialog_new ((print_OP *)pPO)) ;
  gtk_window_set_title (GTK_WINDOW (dialog->dlgPrintProps), _("Printer Setup"));

#ifdef WIN32
  // Can't do command line printing in Win32
  gtk_widget_set_sensitive (QCAD_PRINT_DIALOG (dialog->dlgPrintProps)->rbPrintPipe, FALSE) ;
#endif

  /* Tab 3 - Scale */
  dialog->tblScale = gtk_table_new (2, 3, FALSE);
  gtk_widget_show (dialog->tblScale);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->tblScale), 2);

  dialog->fmScale = gtk_frame_new (_("Scale"));
  gtk_widget_show (dialog->fmScale);
  gtk_table_attach (GTK_TABLE (dialog->tblScale), dialog->fmScale, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 2);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->fmScale), 2);

  dialog->table2 = gtk_table_new (1, 3, FALSE);
  gtk_widget_show (dialog->table2);
  gtk_container_add (GTK_CONTAINER (dialog->fmScale), dialog->table2);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->table2), 2);

  dialog->lblNanoIs = gtk_label_new (_("1 nm is"));
  gtk_widget_show (dialog->lblNanoIs);
  gtk_table_attach (GTK_TABLE (dialog->table2), dialog->lblNanoIs, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 2);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblNanoIs), 1, 0.5);

  dialog->lblScale = gtk_label_new (_("cm"));
  gtk_widget_show (dialog->lblScale);
  gtk_table_attach (GTK_TABLE (dialog->table2), dialog->lblScale, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 2);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblScale), 0, 0.5);

  dialog->adjNanoToUnits = GTK_ADJUSTMENT (gtk_adjustment_new (1, 0.00, 100, 0.01, 1, 1));
  dialog->spnNanoToUnits = gtk_spin_button_new (GTK_ADJUSTMENT (dialog->adjNanoToUnits), 1, 3);
  gtk_widget_show (dialog->spnNanoToUnits);
  gtk_table_attach (GTK_TABLE (dialog->table2), dialog->spnNanoToUnits, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 2);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (dialog->spnNanoToUnits), TRUE);

  dialog->fmFit = gtk_frame_new (_("Number of Pages"));
  gtk_widget_show (dialog->fmFit);
  gtk_table_attach (GTK_TABLE (dialog->tblScale), dialog->fmFit, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 2);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->fmFit), 2);

  dialog->table3 = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (dialog->table3);
  gtk_container_add (GTK_CONTAINER (dialog->fmFit), dialog->table3);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->table3), 2);

  dialog->lblPgsWide = gtk_label_new (_("page(s) wide"));
  gtk_widget_show (dialog->lblPgsWide);
  gtk_table_attach (GTK_TABLE (dialog->table3), dialog->lblPgsWide, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 2);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblPgsWide), 0, 0.5);

  dialog->adjCYPages = GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 100, 1, 10, 10));
  dialog->spnCYPages = gtk_spin_button_new (GTK_ADJUSTMENT (dialog->adjCYPages), 1, 0);
  gtk_widget_show (dialog->spnCYPages);
  gtk_table_attach (GTK_TABLE (dialog->table3), dialog->spnCYPages, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 2);

  dialog->adjCXPages = GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 100, 1, 10, 10));
  dialog->spnCXPages = gtk_spin_button_new (GTK_ADJUSTMENT (dialog->adjCXPages), 1, 0);
  gtk_widget_show (dialog->spnCXPages);
  gtk_table_attach (GTK_TABLE (dialog->table3), dialog->spnCXPages, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 0);

  dialog->lblPgsTall = gtk_label_new (_("page(s) tall"));
  gtk_widget_show (dialog->lblPgsTall);
  gtk_table_attach (GTK_TABLE (dialog->table3), dialog->lblPgsTall, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 2);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblPgsTall), 0, 0.5);

  dialog->rbFixedScale = gtk_radio_button_new_with_label (dialog->grpScaleOpts, _("Fixed"));
  dialog->grpScaleOpts = gtk_radio_button_group (GTK_RADIO_BUTTON (dialog->rbFixedScale));
  gtk_widget_show (dialog->rbFixedScale);
  gtk_table_attach (GTK_TABLE (dialog->tblScale), dialog->rbFixedScale, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_SHRINK), 2, 2);

  dialog->rbFitPages = gtk_radio_button_new_with_label (dialog->grpScaleOpts, _("Fit"));
  dialog->grpScaleOpts = gtk_radio_button_group (GTK_RADIO_BUTTON (dialog->rbFitPages));
  gtk_widget_show (dialog->rbFitPages);
  gtk_table_attach (GTK_TABLE (dialog->tblScale), dialog->rbFitPages, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_SHRINK), 2, 2);

  dialog->vbScale = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (dialog->vbScale);
  gtk_table_attach (GTK_TABLE (dialog->tblScale), dialog->vbScale, 2, 3, 0, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 2, 2);

  dialog->fmPrintOrder = gtk_frame_new (_("Print Order"));
  gtk_widget_show (dialog->fmPrintOrder);
  gtk_box_pack_start (GTK_BOX (dialog->vbScale), dialog->fmPrintOrder, TRUE, TRUE, 2);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->fmPrintOrder), 2);

  dialog->table4 = gtk_table_new (2, 1, FALSE);
  gtk_widget_show (dialog->table4);
  gtk_container_add (GTK_CONTAINER (dialog->fmPrintOrder), dialog->table4);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->table4), 2);

  dialog->tbtnPrintOrder = create_two_pixmap_toggle_button (
    create_pixmap (dialog->dlgPrintProps, "print_over_then_down.png"),
    create_pixmap (dialog->dlgPrintProps, "print_down_then_over.png"), NULL) ;
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "tbtnPrintOrder", dialog->tbtnPrintOrder,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->tbtnPrintOrder);
  gtk_table_attach (GTK_TABLE (dialog->table4), dialog->tbtnPrintOrder, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (0), 2, 2);

  dialog->lblPrintOrder = gtk_label_new (_("Over, then down"));
  gtk_widget_show (dialog->lblPrintOrder);
  gtk_table_attach (GTK_TABLE (dialog->table4), dialog->lblPrintOrder, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_SHRINK),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblPrintOrder), 0.5, 0.5);

  dialog->fmCenter = gtk_frame_new (_("Center On Pages"));
  gtk_widget_show (dialog->fmCenter);
  gtk_box_pack_start (GTK_BOX (dialog->vbScale), dialog->fmCenter, TRUE, TRUE, 2);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->fmCenter), 2);

  dialog->tblCenter = gtk_table_new (2, 1, FALSE);
  gtk_widget_show (dialog->tblCenter);
  gtk_container_add (GTK_CONTAINER (dialog->fmCenter), dialog->tblCenter);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->tblCenter), 2);

  dialog->tbtnCenter = create_two_pixmap_toggle_button (
    create_pixmap (dialog->dlgPrintProps, "no_center_on_pages.png"),
    create_pixmap (dialog->dlgPrintProps, "center_on_pages.png"), NULL) ;
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "tbtnCenter", dialog->tbtnCenter,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->tbtnCenter);
  gtk_table_attach (GTK_TABLE (dialog->tblCenter), dialog->tbtnCenter, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (0), 2, 2);

  dialog->lblCenter = gtk_label_new (_("Do Not Center"));
  gtk_widget_show (dialog->lblCenter);
  gtk_table_attach (GTK_TABLE (dialog->tblCenter), dialog->lblCenter, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_SHRINK),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblCenter), 0.5, 0.5);

  qcad_print_dialog_add_page (QCAD_PRINT_DIALOG (dialog->dlgPrintProps), dialog->tblScale, _("Scale")) ;

  /* Tab 4 - Printed Objects */
  tbl = gtk_table_new (2, 1, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;

  dialog->scrwPrintedObjs = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (dialog->scrwPrintedObjs);
  gtk_table_attach (GTK_TABLE (tbl), dialog->scrwPrintedObjs, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (dialog->scrwPrintedObjs), 2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (dialog->scrwPrintedObjs), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  dialog->vpPrintedObjs = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (dialog->vpPrintedObjs);
  gtk_container_add (GTK_CONTAINER (dialog->scrwPrintedObjs), dialog->vpPrintedObjs);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->vpPrintedObjs), 2);

  dialog->vbPrintedObjs = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (dialog->vbPrintedObjs);
  gtk_container_add (GTK_CONTAINER (dialog->vpPrintedObjs), dialog->vbPrintedObjs);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->vbPrintedObjs), 2);

  dialog->chkColour = gtk_check_button_new_with_label (_("Print Colours")) ;
  gtk_widget_show (dialog->chkColour) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->chkColour, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 2) ;

  qcad_print_dialog_add_page (QCAD_PRINT_DIALOG (dialog->dlgPrintProps), tbl, _("Printed Objects")) ;

  g_signal_connect (G_OBJECT (dialog->adjNanoToUnits), "value_changed", (GCallback)validate_value_change,     dialog->dlgPrintProps) ;
  g_signal_connect (G_OBJECT (dialog->adjCXPages),     "value_changed", (GCallback)validate_value_change,     dialog->dlgPrintProps) ;
  g_signal_connect (G_OBJECT (dialog->adjCYPages),     "value_changed", (GCallback)validate_value_change,     dialog->dlgPrintProps) ;
  g_signal_connect (G_OBJECT (dialog->dlgPrintProps),  "changed",       (GCallback)validate_value_change,     dialog->dlgPrintProps) ;
  g_signal_connect (G_OBJECT (dialog->tbtnPrintOrder), "toggled",       (GCallback)on_tbtnPrintOrder_toggled, dialog->dlgPrintProps) ;
  g_signal_connect (G_OBJECT (dialog->tbtnCenter),     "toggled",       (GCallback)on_tbtnCenter_toggled,     dialog->dlgPrintProps) ;
  g_signal_connect (G_OBJECT (dialog->rbFixedScale),   "toggled",       (GCallback)toggle_scale_mode,         dialog->dlgPrintProps) ;
  g_signal_connect (G_OBJECT (dialog->rbFitPages),     "toggled",       (GCallback)toggle_scale_mode,         dialog->dlgPrintProps) ;
  g_signal_connect (G_OBJECT (dialog->dlgPrintProps),  "units_changed", (GCallback)units_changed,             dialog->dlgPrintProps) ;
  g_signal_connect (G_OBJECT (dialog->dlgPrintProps),  "preview",       (GCallback)user_wants_print_preview,  dialog->dlgPrintProps) ;
  }
