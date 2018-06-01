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
// A (fairly) complete print settings dialog with mar-  //
// gins, Center Page, paper size, user-selectable units //
// (cm/in/pt), etc.                                     //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <math.h>
#include "design.h"
#include "support.h"
#include "print_preview.h"
#include "file_selection_window.h"
#include "print_properties_dialog.h"
#include "print_properties_dialog_interface.h"
#include "print_properties_dialog_callbacks.h"

#define MIN_MARGIN_GAP 72 /* points */
#define CEIL_EXCEPTION_EPSILON 1e-10

#define STATUS_OK 0
#define STATUS_NEED_FILE_NAME 1
#define STATUS_NEED_PIPE 2

static print_properties_D print_properties = {NULL} ;

static double conversion_matrix[3][3] =
  {
  {    1.00     , 1.00 /  2.54 , 72.00 / 2.54},
  {    2.54     ,     1.00     ,     72.00   },
  {2.54 / 72.00 , 1.00 / 72.00 ,      1.00   }
  } ;

static void init_print_design_properties_dialog (print_properties_D *dialog, GtkWindow *parent, print_design_OP *print_op, DESIGN *design) ;
static void calc_world_size (int *piCX, int *piCY, print_properties_D *dialog) ;
static void check_scale (print_properties_D *dialog, GtkAdjustment *adj) ;

extern double subs_width ;
extern double subs_height ;

// The main function
gboolean get_print_design_properties_from_user (GtkWindow *parent, print_design_OP *ppo, DESIGN *design)
  {
  gboolean bOK = FALSE ;

  if (NULL == print_properties.dlgPrintProps)
    create_print_design_properties_dialog (&print_properties, ppo) ;

  init_print_design_properties_dialog (&print_properties, parent, ppo, design) ;

  if ((bOK = (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (print_properties.dlgPrintProps)))))
      init_print_design_options (ppo, design) ;

  gtk_widget_hide (print_properties.dlgPrintProps) ;

  if (NULL != parent)
    gtk_window_present (parent) ;

  return bOK ;
  }

// initialize the dialog - whether to display it, or to simply ensure correct print_op values
static void init_print_design_properties_dialog (print_properties_D *dialog, GtkWindow *parent, print_design_OP *print_op, DESIGN *design)
  {
  int Nix ;

  if (NULL == dialog->dlgPrintProps)
    create_print_design_properties_dialog (dialog, print_op) ;

  // The static data needs to be set right away, because signals will come up empty
  g_object_set_data (G_OBJECT (dialog->dlgPrintProps), "design", design) ;
  g_object_set_data (G_OBJECT (dialog->dlgPrintProps), "dialog", dialog) ;
  g_object_set_data (G_OBJECT (dialog->dlgPrintProps), "old_units",
    (gpointer)qcad_print_dialog_get_units (QCAD_PRINT_DIALOG (dialog->dlgPrintProps))) ;

  gtk_window_set_transient_for (GTK_WINDOW (dialog->dlgPrintProps), parent) ;

  if (NULL != dialog->ppPrintedObjs)
    for (Nix = 0 ; Nix < dialog->icPrintedObjs ; Nix++)
      gtk_container_remove (GTK_CONTAINER (dialog->vbPrintedObjs), dialog->ppPrintedObjs[Nix]) ;

  fill_printed_objects_list (dialog->lstPrintedObjs, dialog, design) ;

  g_free (print_op->pbPrintedObjs) ;
  print_op->icPrintedObjs = dialog->icPrintedObjs ;
  print_op->pbPrintedObjs = g_malloc0 (dialog->icPrintedObjs * sizeof (gboolean)) ;

  // Fill in the dialog from the print_op values (must have the ppPrintedObjs filled in first !)
  gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjNanoToUnits),
    qcad_print_dialog_to_current_units (QCAD_PRINT_DIALOG (dialog->dlgPrintProps), print_op->dPointsPerNano)) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->tbtnPrintOrder), !print_op->bPrintOrderOver) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->tbtnCenter), print_op->bCenter) ;

  if (print_op->bFit)
    {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->rbFitPages), TRUE) ;
    gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjCXPages), print_op->iCXPages) ;
    gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjCYPages), print_op->iCYPages) ;
    }

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->chkColour), print_op->bColour) ;

  toggle_scale_mode (NULL, dialog->dlgPrintProps) ;
  }

void chkPrintedObj_toggled (GtkWidget *widget, gpointer user_data)
  {
  QCADLayer *layer = NULL ;
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  int cx = -1, cy = -1 ;

  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
    {
    calc_world_size (&cx, &cy, dialog) ;
    if (0 == cx || 0 == cy)
      {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), TRUE) ;
      gdk_beep () ;
      return ;
      }
    }

  if (NULL != (layer = g_object_get_data (G_OBJECT (widget), "layer")))
    g_object_set_data (G_OBJECT (layer), "print_layer", (gpointer)gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) ;

  check_scale (dialog, NULL) ;
  }

void units_changed (GtkWidget *widget, gpointer data)
  {
  print_properties_D *dialog = (print_properties_D *)g_object_get_data (G_OBJECT (widget), "dialog") ;
  QCADPrintDialogUnits
    old_units = (QCADPrintDialogUnits)g_object_get_data (G_OBJECT (widget), "old_units"),
    new_units = qcad_print_dialog_get_units (QCAD_PRINT_DIALOG (widget)) ;
  char *pszShortString = qcad_print_dialog_get_units_short_string (QCAD_PRINT_DIALOG (widget)) ;

  gtk_label_set_text (GTK_LABEL (dialog->lblScale), pszShortString) ;

  gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjNanoToUnits),
    gtk_adjustment_get_value (GTK_ADJUSTMENT (dialog->adjNanoToUnits)) *
      conversion_matrix[old_units][new_units]) ;

  g_object_set_data (G_OBJECT (widget), "old_units", (gpointer)new_units) ;
  }

// Used when the print properties dialog is to be initialized but not displayed - like
// when the user clicks "Preview" before ever having used the dialog.  Also used for
// filling out the print_OP structure
void init_print_design_options (print_design_OP *pPrintOp, DESIGN *design)
  {
  int Nix ;

  if (NULL == print_properties.dlgPrintProps)
    {
    create_print_design_properties_dialog (&print_properties, pPrintOp) ;
    init_print_design_properties_dialog (&print_properties, NULL, pPrintOp, design) ;
    }

  qcad_print_dialog_get_options (QCAD_PRINT_DIALOG (print_properties.dlgPrintProps), &(pPrintOp->po)) ;

  // points per nanometer
  pPrintOp->dPointsPerNano = qcad_print_dialog_from_current_units (QCAD_PRINT_DIALOG (print_properties.dlgPrintProps),
    gtk_adjustment_get_value (GTK_ADJUSTMENT (print_properties.adjNanoToUnits))) ;

  g_free (pPrintOp->pbPrintedObjs) ;
  pPrintOp->icPrintedObjs = print_properties.icPrintedObjs ;
  pPrintOp->pbPrintedObjs = g_malloc0 (print_properties.icPrintedObjs * sizeof (gboolean)) ;

  // The various layers
  for (Nix = 0 ; Nix < print_properties.icPrintedObjs ; Nix++)
    pPrintOp->pbPrintedObjs[Nix] =
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (print_properties.ppPrintedObjs[Nix])) ;

  // Print over than down ?
  pPrintOp->bPrintOrderOver = !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (print_properties.tbtnPrintOrder)) ;

  // Center on pages ?
  pPrintOp->bCenter = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (print_properties.tbtnCenter)) ;

  // Number of horizontal pages - takes precedence over the scaling factor
  pPrintOp->iCXPages = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (print_properties.spnCXPages)) ;

  // Number of vertical pages - takes precedence over the scaling factor
  pPrintOp->iCYPages = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (print_properties.spnCYPages)) ;

  pPrintOp->bFit = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (print_properties.rbFitPages)) ;

  pPrintOp->bColour = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (print_properties.chkColour)) ;
  }

void on_tbtnPrintOrder_toggled (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  gtk_label_set_text (GTK_LABEL (dialog->lblPrintOrder),
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ?
    _("Down, then over") : _("Over, then down")) ;
  }

void on_tbtnCenter_toggled (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  gtk_label_set_text (GTK_LABEL (dialog->lblCenter), gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ? _("Center") : _("Do Not Center")) ;
  }

// Calculate the world size (in nanos)
static void calc_world_size (int *piCX, int *piCY, print_properties_D *dialog)
  {
  int Nix ;
  WorldRectangle layer_extents = {0.0} ;

  (*piCX) = (*piCY) = 0 ;

  for (Nix = 0 ; Nix < dialog->icPrintedObjs ; Nix++)
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->ppPrintedObjs[Nix])))
      {
      qcad_layer_get_extents (QCAD_LAYER (g_object_get_data (G_OBJECT (dialog->ppPrintedObjs[Nix]), "layer")), &layer_extents, FALSE) ;
      (*piCX) = MAX ((*piCX), layer_extents.cxWorld) ;
      (*piCY) = MAX ((*piCY), layer_extents.cyWorld) ;
      }
  }

// So far, there are only 3 hardcoded layers
void fill_printed_objects_list (GtkWidget *ls, print_properties_D *dialog, DESIGN *design)
  {
  gboolean bPrintLayer = TRUE ;
  int Nix ;
  GList *lstItr = NULL ;

  if (NULL != dialog->ppPrintedObjs)
    g_free (dialog->ppPrintedObjs) ;
   dialog->icPrintedObjs = 0 ;

  // Count the layers in the design
  for (lstItr = design->lstLayers ; lstItr != NULL ; lstItr = lstItr->next)
    dialog->icPrintedObjs++ ;

  dialog->ppPrintedObjs = g_malloc0 (dialog->icPrintedObjs * sizeof (GtkWidget *)) ;

  for (lstItr = design->lstLayers, Nix = 0 ; lstItr != NULL ; lstItr = lstItr->next, Nix++)
    {
    dialog->ppPrintedObjs[Nix] = gtk_check_button_new_with_label ((QCAD_LAYER (lstItr->data))->pszDescription) ;
    g_object_set_data (G_OBJECT (dialog->ppPrintedObjs[Nix]), "layer", lstItr->data) ;
    if (!(gboolean)g_object_get_data (G_OBJECT (lstItr->data), "set_printed"))
      {
      g_object_set_data (G_OBJECT (lstItr->data), "set_printed", (gpointer)TRUE) ;
      g_object_set_data (G_OBJECT (lstItr->data), "print_layer", (gpointer)TRUE) ;
      }
    else
      bPrintLayer = (gboolean)g_object_get_data (G_OBJECT (lstItr->data), "print_layer") ;
    gtk_widget_show (dialog->ppPrintedObjs[Nix]) ;
    gtk_box_pack_start (GTK_BOX (dialog->vbPrintedObjs), dialog->ppPrintedObjs[Nix], FALSE, FALSE, 2) ;

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->ppPrintedObjs[Nix]),
      (LAYER_STATUS_ACTIVE  == (QCAD_LAYER (lstItr->data))->status ||
       LAYER_STATUS_VISIBLE == (QCAD_LAYER (lstItr->data))->status) && bPrintLayer) ;

    g_signal_connect (G_OBJECT (dialog->ppPrintedObjs[Nix]), "toggled", (GCallback)chkPrintedObj_toggled, dialog->dlgPrintProps) ;
    }
  }

void toggle_scale_mode (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  gboolean bAuto = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->rbFitPages)) ;

  if (NULL != widget)
    if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) return ;

  gtk_widget_set_sensitive (dialog->fmScale, !bAuto) ;
  gtk_widget_set_sensitive (dialog->lblNanoIs, !bAuto) ;
  gtk_widget_set_sensitive (dialog->spnNanoToUnits, !bAuto) ;
  gtk_widget_set_sensitive (dialog->lblScale, !bAuto) ;

  gtk_widget_set_sensitive (dialog->fmFit, bAuto) ;
  gtk_widget_set_sensitive (dialog->spnCXPages, bAuto) ;
  gtk_widget_set_sensitive (dialog->spnCYPages, bAuto) ;
  gtk_widget_set_sensitive (dialog->lblPgsWide, bAuto) ;
  gtk_widget_set_sensitive (dialog->lblPgsTall, bAuto) ;

  check_scale (dialog, NULL) ;
  }

// Make sure the scale and the number of pages tall/wide agree
static void check_scale (print_properties_D *dialog, GtkAdjustment *adj)
  {
  double dcxPg, dcyPg, tmp,
    dNanoToUnits = gtk_adjustment_get_value (GTK_ADJUSTMENT (dialog->adjNanoToUnits)) ;
  int
    icxWorld = 0, icyWorld = 0, /* in nanos */
    icxPages = 0, icyPages = 0 ;
  print_OP po = {0} ;

  qcad_print_dialog_get_options (QCAD_PRINT_DIALOG (dialog->dlgPrintProps), &po) ;
  if (NULL != po.pszPrintString) g_free (po.pszPrintString) ;

  calc_world_size (&icxWorld, &icyWorld, dialog) ;

  dcxPg = po.dPaperCX - po.dLMargin - po.dRMargin ;
  dcyPg = po.dPaperCY - po.dTMargin - po.dBMargin ;

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->rbFitPages)))
    {
    if (adj == dialog->adjCXPages)
      dNanoToUnits =
        qcad_print_dialog_to_current_units (QCAD_PRINT_DIALOG (dialog->dlgPrintProps),
          (dcxPg * gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCXPages)))) / icxWorld ;
    else
    if (adj == dialog->adjCYPages)
      dNanoToUnits =
        qcad_print_dialog_to_current_units (QCAD_PRINT_DIALOG (dialog->dlgPrintProps),
          (dcyPg * gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCYPages)))) / icyWorld ;
    else
      dNanoToUnits = MIN (
        qcad_print_dialog_to_current_units (QCAD_PRINT_DIALOG (dialog->dlgPrintProps),
          (dcxPg * gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCXPages)))) / icxWorld,
        qcad_print_dialog_to_current_units (QCAD_PRINT_DIALOG (dialog->dlgPrintProps),
          (dcyPg * gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCYPages)))) / icyWorld) ;

    dNanoToUnits = floor (dNanoToUnits * 1000.0) / 1000.0 ;

    gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjNanoToUnits), dNanoToUnits) ;
    }

  tmp = qcad_print_dialog_from_current_units (QCAD_PRINT_DIALOG (dialog->dlgPrintProps), icxWorld * dNanoToUnits) / dcxPg ;
  icxPages = (int)((fabs (tmp - (double)((int)tmp)) > CEIL_EXCEPTION_EPSILON) ? ceil (tmp) : tmp) ;
  tmp = qcad_print_dialog_from_current_units (QCAD_PRINT_DIALOG (dialog->dlgPrintProps), icyWorld * dNanoToUnits) / dcyPg ;
  icyPages = (int)((fabs (tmp - (double)((int)tmp)) > CEIL_EXCEPTION_EPSILON) ? ceil (tmp) : tmp) ;

  g_signal_handlers_block_matched ((gpointer)dialog->adjCXPages, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)validate_value_change, NULL) ;
  g_signal_handlers_block_matched ((gpointer)dialog->adjCYPages, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)validate_value_change, NULL) ;
  if (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCXPages)) != icxPages)
    gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjCXPages), icxPages) ;
  if (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCYPages)) != icyPages)
    gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjCYPages), icyPages) ;
  g_signal_handlers_unblock_matched ((gpointer)dialog->adjCXPages, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)validate_value_change, NULL) ;
  g_signal_handlers_unblock_matched ((gpointer)dialog->adjCYPages, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)validate_value_change, NULL) ;
  }

// Make sure all spin buttons everywhere always have correct values
void validate_value_change (GtkAdjustment *adj_changed, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  check_scale (dialog, adj_changed) ;
  }

void user_wants_print_preview (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  DESIGN *design = (DESIGN *)gtk_object_get_data (GTK_OBJECT (user_data), "design") ;
  print_design_OP po ;

  po.pbPrintedObjs = g_malloc0 (dialog->icPrintedObjs * sizeof (gboolean)) ;
  po.icPrintedObjs = dialog->icPrintedObjs ;

  init_print_design_options (&po, design) ;

  do_print_preview ((print_OP *)&po, GTK_WINDOW (dialog->dlgPrintProps), (void *)design, (PrintFunction)print_world) ;

  g_free (po.pbPrintedObjs) ;
  }
