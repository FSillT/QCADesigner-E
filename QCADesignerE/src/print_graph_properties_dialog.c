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
// This dialog, derived from the general print dialog,  //
// allows the user to specify parameters governing the  //
// printing of graph traces.                            //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include "support.h"
#include "simulation.h"
#include "objects/QCADPrintDialog.h"
#include "print_graph_properties_dialog.h"
#include "print.h"
#include "print_preview.h"

typedef struct
  {
  GtkWidget *dlgPrintGraphProps ;
  GtkWidget *chkPrintClr ;
  } print_graph_properties_D ;

static print_graph_properties_D print_graph_props = {NULL} ;

static void create_print_graph_properties_dialog (print_graph_properties_D *dialog, print_OP *pPO) ;
static void graphs_preview (GtkWidget *widget, gpointer user_data) ;
static void chkPrintClr_activate (GtkWidget *widget, gpointer data) ;

gboolean get_print_graph_properties_from_user (GtkWindow *parent, print_graph_OP *pPO, PRINT_GRAPH_DATA *print_graph_data)
  {
  gboolean bRet = FALSE ;

  if (NULL == print_graph_props.dlgPrintGraphProps)
    create_print_graph_properties_dialog (&print_graph_props, &(pPO->po)) ;

  g_object_set_data (G_OBJECT (print_graph_props.dlgPrintGraphProps), "print_graph_data", print_graph_data) ;

  gtk_window_set_transient_for (GTK_WINDOW (print_graph_props.dlgPrintGraphProps), parent) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (print_graph_props.chkPrintClr), pPO->bPrintClr) ;

  if ((bRet = (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (print_graph_props.dlgPrintGraphProps)))))
    init_print_graph_options (pPO, print_graph_data) ;

  gtk_widget_hide (print_graph_props.dlgPrintGraphProps) ;

  return bRet ;
  }

static void create_print_graph_properties_dialog (print_graph_properties_D *dialog, print_OP *pPO)
  {
  GtkWidget *tbl = NULL, *widget = NULL ;

  dialog->dlgPrintGraphProps = qcad_print_dialog_new (pPO) ;
  gtk_window_set_title (GTK_WINDOW (dialog->dlgPrintGraphProps), _("Printer Setup")) ;

  widget = tbl = gtk_table_new (1, 1, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;

  widget = dialog->chkPrintClr = gtk_check_button_new_with_label (_("Print Colours")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tbl), widget, 0, 1, 0, 1,
    (GtkAttachOptions)(0),
    (GtkAttachOptions)(0), 2, 2) ;

  qcad_print_dialog_add_page (QCAD_PRINT_DIALOG (dialog->dlgPrintGraphProps), tbl, _("Document Options")) ;
  g_signal_connect (G_OBJECT (dialog->dlgPrintGraphProps), "preview", (GCallback)graphs_preview, dialog->dlgPrintGraphProps) ;
  g_signal_connect (G_OBJECT (dialog->chkPrintClr), "activate", (GCallback)chkPrintClr_activate, pPO) ;
  }

void init_print_graph_options (print_graph_OP *pPO, PRINT_GRAPH_DATA *print_graph_data)
  {
  if (NULL == print_graph_props.dlgPrintGraphProps)
    create_print_graph_properties_dialog (&print_graph_props, &(pPO->po)) ;

  qcad_print_dialog_get_options (QCAD_PRINT_DIALOG (print_graph_props.dlgPrintGraphProps), &(pPO->po)) ;

  pPO->bPrintClr = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (print_graph_props.chkPrintClr)) ;
  }

static void chkPrintClr_activate (GtkWidget *widget, gpointer data)
  {((print_graph_OP *)data)->bPrintClr = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ;}

static void graphs_preview (GtkWidget *widget, gpointer user_data)
  {
  print_graph_OP po = {{0, 0, 0, 0, 0, 0, FALSE, TRUE, NULL}, TRUE, TRUE, 1, 1} ;
  PRINT_GRAPH_DATA *print_graph_data = (PRINT_GRAPH_DATA *)g_object_get_data (G_OBJECT (user_data), "print_graph_data") ;

  init_print_graph_options (&po, print_graph_data) ;

  do_print_preview ((print_OP *)&po, GTK_WINDOW (user_data), print_graph_data, (PrintFunction)print_graphs) ;
  }
