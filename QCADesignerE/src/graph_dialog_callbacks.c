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
// Callbacks for the graph dialog. This is where all    //
// the dialog behaviour is done.                        //
//                                                      //
//////////////////////////////////////////////////////////

#include "support.h"
#include "fileio.h"
#include "print.h"
#include "generic_utils.h"
#include "global_consts.h"
#include "custom_widgets.h"
#include "bus_layout_dialog.h"
#include "print_preview.h"
#include "file_selection_window.h"
#include "print_graph_properties_dialog.h"
#include "honeycomb_thresholds_dialog.h"
#include "graph_dialog.h"
#include "graph_dialog_interface.h"
#include "graph_dialog_widget_data.h"
#include "graph_dialog_callbacks.h"

// x values from user hilighting 
static int x_beg = -1, x_old = -1, x_cur = -1 ;
static int beg_sample = -1 ;
static gboolean bCancelStretch = FALSE ;

static void recalculate_honeycombs (GRAPH_DIALOG_DATA *gdd, gboolean bCalcHoneycombArrays, graph_D *dialog) ;
static void set_trace_widget_visible (GtkWidget *trace, gboolean bVisible) ;
static void set_ruler_values (GtkWidget *ruler, int cxGiven, int cx, int old_offset, int xOffset, int icSamples) ;
static void draw_trace_reference_lines (GdkDrawable *dst, int cx, int cy) ;
static void print_graph_data_init (PRINT_GRAPH_DATA *print_graph_data, GRAPH_DIALOG_DATA *gdd) ;
void reflect_scale_change (GRAPH_DIALOG_DATA *dialog_data) ;

static print_graph_OP print_graph_options = {{612, 792, 72, 72, 72, 72, TRUE, TRUE, NULL}, TRUE, TRUE, 1, 1} ;

// Extremely hacky solution to the problem of making all traces the same size
gboolean graph_widget_one_time_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data)
  {
  GtkWidget *trace = NULL ;
  graph_D *dialog = (graph_D *)data ;
  GRAPH_DIALOG_DATA *gdd = NULL ;
  GtkTreeIter itr ;

  if (NULL == dialog) return FALSE ;

  if (NULL == (gdd = g_object_get_data (G_OBJECT (dialog->dialog), "graph_dialog_data"))) return FALSE ;

  if (gdd->bOneTime)
    {
    gdd->bOneTime = FALSE ;
    if (gtk_tree_model_get_iter_first (gdd->model, &itr))
      {
      while (TRUE)
        {
        gtk_tree_model_get (gdd->model, &itr,
          GRAPH_MODEL_COLUMN_TRACE, &trace, -1) ;
        if (GTK_WIDGET_VISIBLE (trace))
          {
          gtk_widget_hide (trace) ;
          gtk_widget_show (trace) ;
          }
        if (!gtk_tree_model_iter_next_dfs (gdd->model, &itr)) break ;
        }
      }
    }

  g_signal_handlers_block_matched (G_OBJECT (widget), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)graph_widget_one_time_expose, NULL) ;

  return FALSE ;
  }

void btnShowBase_clicked (GtkWidget *widget, gpointer data)
  {
  graph_D *dialog = (graph_D *)data ;
  int base = (int)g_object_get_data (G_OBJECT (widget), "base") ;
  GRAPH_DIALOG_DATA *gdd = NULL ;

  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) return ;
  if (0 == base) return ;
  if (NULL == dialog) return ;
  if (NULL == (gdd = g_object_get_data (G_OBJECT (dialog->dialog), "graph_dialog_data"))) return ;

  gdd->base = base ;

  recalculate_honeycombs (gdd, FALSE, dialog) ;
  }

void model_visible_toggled (GtkCellRenderer *cr, char *pszTreePath, gpointer data)
  {
  GtkTreePath *tp = NULL ;
  graph_D *dialog = (graph_D *)data ;
  GRAPH_DIALOG_DATA *gdd = NULL ;
  GtkTreeIter itr ;
  gboolean bModelVisible ;
  GtkWidget *trace = NULL, *ruler = NULL, *ui = NULL ;

  if (NULL == dialog) return ;
  if (NULL == (gdd = g_object_get_data (G_OBJECT (dialog->dialog), "graph_dialog_data"))) return ;
  if (NULL == (tp = gtk_tree_path_new_from_string (pszTreePath))) return ;

  gtk_tree_model_get_iter (gdd->model, &itr, tp) ;

  gtk_tree_model_get (gdd->model, &itr,
    GRAPH_MODEL_COLUMN_VISIBLE, &bModelVisible,
    GRAPH_MODEL_COLUMN_TRACE, &trace,
    GRAPH_MODEL_COLUMN_RULER, &ruler,
    GRAPH_MODEL_COLUMN_UI, &ui, -1) ;

  bModelVisible = !bModelVisible ;

  set_trace_widget_visible (trace, bModelVisible) ;
  GTK_WIDGET_SET_VISIBLE (ruler, bModelVisible) ;
  GTK_WIDGET_SET_VISIBLE (ui, bModelVisible) ;

  gtk_tree_store_set (GTK_TREE_STORE (gdd->model), &itr, GRAPH_MODEL_COLUMN_VISIBLE, bModelVisible, -1) ;

  gtk_tree_path_free (tp) ;
  }

void hscroll_adj_value_changed (GtkAdjustment *adj, gpointer data)
  {
  GtkTreeIter itr ;
  GtkWidget *drawing_area = NULL, *ruler = NULL ;
  GRAPH_DATA *graph_data = NULL ;
  GRAPH_DIALOG_DATA *gdd = g_object_get_data (G_OBJECT (data), "graph_dialog_data") ;
  int cx, cy, old_offset = -1 ;

  if (NULL == gdd) return ;

  if (!gtk_tree_model_get_iter_first (gdd->model, &itr)) return ;

  while (TRUE)
    {
    gtk_tree_model_get (gdd->model, &itr,
      GRAPH_MODEL_COLUMN_TRACE, &drawing_area,
      GRAPH_MODEL_COLUMN_RULER, &ruler, -1) ;
    if (NULL != drawing_area)
      if (NULL != (graph_data = g_object_get_data (G_OBJECT (drawing_area), "graph_data")))
        {
        old_offset = gdd->xOffset ;
        gdd->xOffset = -(adj->value) ;
        gtk_widget_queue_draw (drawing_area) ;
        if (NULL != drawing_area->window)
          {
          gdk_window_get_size (drawing_area->window, &cx, &cy) ;
          set_ruler_values (ruler, graph_data->cxGiven, cx, old_offset, gdd->xOffset, gdd->sim_data->number_samples) ;
          }
        }
    if (!gtk_tree_model_iter_next_dfs (gdd->model, &itr)) break ;
    }
  }

gboolean waveform_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data)
  {
  GdkGC *gc = NULL ;
  WAVEFORM_DATA *wf = g_object_get_data (G_OBJECT (widget), "graph_data") ;
  GRAPH_DIALOG_DATA *gdd = (GRAPH_DIALOG_DATA *)data ;
  int Nix ;
  int idxStart = -1 ;
  int ic = 0 ;
  int cx, cy ;

  if (NULL == wf) return FALSE ;
  if (NULL == gdd) return FALSE ;

  gdk_window_get_size (widget->window, &cx, &cy) ;

  if (wf->graph_data.bNeedCalc)
    calculate_waveform_coords (wf, gdd->sim_data->number_samples) ;

  gc = gdk_gc_new (widget->window) ;

  gdk_gc_set_foreground (gc, &(wf->graph_data.clr)) ;
  gdk_gc_set_background (gc, &(wf->graph_data.clr)) ;

  for (Nix = 0 ; Nix < wf->arPoints->icUsed ; Nix++)
    exp_array_index_1d (wf->arPoints, GdkPoint, Nix).x += gdd->xOffset ;

  for (idxStart = 0 ; idxStart < wf->arPoints->icUsed ; idxStart++)
    if (exp_array_index_1d (wf->arPoints, GdkPoint, idxStart).x >= 0)
      break ;

  idxStart = CLAMP (idxStart - 1, 0, wf->arPoints->icUsed - 1) ;

  for (Nix = idxStart ; Nix < wf->arPoints->icUsed ; Nix++, ic++)
    if (exp_array_index_1d (wf->arPoints, GdkPoint, Nix).x >= cx)
      break ;

  ic = CLAMP (ic + 1, 0, wf->arPoints->icUsed - idxStart) ;

  draw_trace_reference_lines (widget->window, cx, cy) ;

  gdk_draw_lines (widget->window, gc, (GdkPoint *)&(exp_array_index_1d (wf->arPoints, GdkPoint, idxStart)), ic);

  for (Nix = wf->arPoints->icUsed - 1 ; Nix > -1 ; Nix--)
    exp_array_index_1d (wf->arPoints, GdkPoint, Nix).x -= gdd->xOffset ;

  g_object_unref (gc) ;
  return FALSE ;
  }

gboolean honeycomb_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data)
  {
  int Nix ;
  GdkGC *gc = NULL ;
  HONEYCOMB_DATA *hc = g_object_get_data (G_OBJECT (widget), "graph_data") ;
  GRAPH_DIALOG_DATA *gdd = (GRAPH_DIALOG_DATA *)data ;
  int cx, cy, cxText, cyText ;
  HONEYCOMB *hcCur = NULL ;
  char *psz = NULL ;

  if (NULL == hc) return FALSE ;

  gdk_window_get_size (widget->window, &cx, &cy) ;

  if (hc->graph_data.bNeedCalc)
    calculate_honeycomb_coords (hc, gdd->sim_data->number_samples) ;

  draw_trace_reference_lines (widget->window, cx, cy) ;

  gc = gdk_gc_new (widget->window) ;
  gdk_gc_set_foreground (gc, &(hc->graph_data.clr)) ;
  gdk_gc_set_background (gc, &(hc->graph_data.clr)) ;

  gdk_draw_line (widget->window, gc, 0, cy >> 1, cx, cy >> 1) ;
  if (hc->arHCs->icUsed > 0)
    {
    for (Nix = 0 ; Nix < hc->arHCs->icUsed ; Nix++)
      {
      hcCur = &(exp_array_index_1d (hc->arHCs, HONEYCOMB, Nix)) ;
      hcCur->pts[0].x += gdd->xOffset ;
      hcCur->pts[1].x += gdd->xOffset ;
      hcCur->pts[2].x += gdd->xOffset ;
      hcCur->pts[3].x += gdd->xOffset ;
      hcCur->pts[4].x += gdd->xOffset ;
      hcCur->pts[5].x += gdd->xOffset ;
      }

    for (Nix = 0 ; Nix < hc->arHCs->icUsed ; Nix++)
      {
      hcCur = &(exp_array_index_1d (hc->arHCs, HONEYCOMB, Nix)) ;
      if (RECT_INTERSECT_RECT (0, 0, cx, cy, hcCur->pts[0].x, 0, hcCur->pts[3].x - hcCur->pts[0].x + 1, cy))
        break ;
      }

    for (; Nix < hc->arHCs->icUsed ; Nix++)
      {
      hcCur = &(exp_array_index_1d (hc->arHCs, HONEYCOMB, Nix)) ;
      if (!RECT_INTERSECT_RECT (0, 0, cx, cy, hcCur->pts[0].x, 0, hcCur->pts[3].x - hcCur->pts[0].x + 1, cy))
        break ;

      gdk_gc_set_foreground (gc, clr_idx_to_clr_struct (HONEYCOMB_BACKGROUND)) ;
      gdk_gc_set_background (gc, clr_idx_to_clr_struct (HONEYCOMB_BACKGROUND)) ;
      gdk_draw_polygon (widget->window, gc, TRUE, hcCur->pts, 6) ;
      gdk_gc_set_foreground (gc, &(hc->graph_data.clr)) ;
      gdk_gc_set_background (gc, &(hc->graph_data.clr)) ;
      get_string_dimensions (psz = strdup_convert_to_base (hcCur->value, gdd->base), FONT_STRING, &cxText, &cyText) ;
      draw_string (widget->window, gc, FONT_STRING,
        (hcCur->pts[3].x + hcCur->pts[0].x - cxText) >> 1,
        (hcCur->pts[1].y + hcCur->pts[4].y - cyText) >> 1, psz) ;
      g_free (psz) ;
      gdk_draw_polygon (widget->window, gc, FALSE, hcCur->pts, 6) ;
      }

    for (Nix = 0 ; Nix < hc->arHCs->icUsed ; Nix++)
      {
      hcCur = &(exp_array_index_1d (hc->arHCs, HONEYCOMB, Nix)) ;
      hcCur->pts[0].x -= gdd->xOffset ;
      hcCur->pts[1].x -= gdd->xOffset ;
      hcCur->pts[2].x -= gdd->xOffset ;
      hcCur->pts[3].x -= gdd->xOffset ;
      hcCur->pts[4].x -= gdd->xOffset ;
      hcCur->pts[5].x -= gdd->xOffset ;
      }
    }

  g_object_unref (gc) ;
  return FALSE ;
  }

gboolean graph_widget_size_allocate (GtkWidget *widget, GtkAllocation *alloc, gpointer data)
  {
  GtkTreeIter itr ;
  GtkWidget *widget_to_size = NULL ;
  GtkWidget *ruler = NULL ;
  GtkWidget *hscroll = g_object_get_data (G_OBJECT (widget), "hscroll") ;
  int *pcx = NULL, *pcy = NULL, *pic = NULL ;
  GRAPH_DIALOG_DATA *dialog_data = (GRAPH_DIALOG_DATA *)data ;
  int model_column = -1 ;
  GRAPH_DATA *graph_data = NULL ;
  GtkAdjustment *adj = NULL ;

  if (NULL == dialog_data) return FALSE ;

  if (GTK_IS_DRAWING_AREA (widget))
    {
    pcx = &(dialog_data->cxDrawingArea) ;
    pcy = &(dialog_data->cyDrawingArea) ;
    pic = &(dialog_data->icDrawingArea) ;
    model_column = GRAPH_MODEL_COLUMN_TRACE ;
    }
  else
    {
    pcx = &(dialog_data->cxUIWidget) ;
    pcy = &(dialog_data->cyUIWidget) ;
    pic = &(dialog_data->icUIWidget) ;
    model_column = GRAPH_MODEL_COLUMN_UI ;
    }

  if ((*pic) == dialog_data->icGraphLines) return FALSE ;

  (*pcx) = MAX ((*pcx), alloc->width) ;
  (*pcy) = MAX ((*pcy), alloc->height) ;
  (*pic)++ ;

  if ((*pic) == dialog_data->icGraphLines)
    {
    if (!gtk_tree_model_get_iter_first (dialog_data->model, &itr)) return FALSE ;
    while (TRUE)
      {
      gtk_tree_model_get (dialog_data->model, &itr, model_column, &widget_to_size, -1) ;
      gtk_widget_set_size_request (widget_to_size, (GRAPH_MODEL_COLUMN_TRACE == model_column) ? -1 : (*pcx), (NULL == pcy) ? -1 : (*pcy)) ;
      if (!gtk_tree_model_iter_next_dfs (dialog_data->model, &itr)) break ;
      }
    }

  if (GTK_IS_DRAWING_AREA (widget))
    {
    dialog_data->cxMaxGiven = dialog_data->cxDrawingArea ;
    dialog_data->cyMaxGiven = dialog_data->cyDrawingArea ;
    if (!gtk_tree_model_get_iter_first (dialog_data->model, &itr)) return FALSE ;
    while (TRUE)
      {
      gtk_tree_model_get (dialog_data->model, &itr, GRAPH_MODEL_COLUMN_TRACE, &widget_to_size, -1) ;
      if (NULL != (graph_data = g_object_get_data (G_OBJECT (widget_to_size), "graph_data")))
        {
        dialog_data->cxMaxGiven = MAX (dialog_data->cxMaxGiven, graph_data->cxWanted) ;
        dialog_data->cyMaxGiven = MAX (dialog_data->cyMaxGiven, graph_data->cyWanted) ;
        }
      if (!gtk_tree_model_iter_next_dfs (dialog_data->model, &itr)) break ;
      }
    if (!gtk_tree_model_get_iter_first (dialog_data->model, &itr)) return FALSE ;
    while (TRUE)
      {
      gtk_tree_model_get (dialog_data->model, &itr, GRAPH_MODEL_COLUMN_TRACE, &widget_to_size,
        GRAPH_MODEL_COLUMN_RULER, &ruler, -1) ;
      if (NULL != (graph_data = g_object_get_data (G_OBJECT (widget_to_size), "graph_data")))
        {
        graph_data->cxGiven = dialog_data->cxMaxGiven * dialog_data->dScale ;
        graph_data->cyGiven = dialog_data->cyMaxGiven ;
        graph_data->bNeedCalc = TRUE ;
        set_ruler_values (ruler, dialog_data->cxMaxGiven * dialog_data->dScale, dialog_data->cxDrawingArea, dialog_data->xOffset, dialog_data->xOffset, dialog_data->sim_data->number_samples) ;
        }
      if (!gtk_tree_model_iter_next_dfs (dialog_data->model, &itr)) break ;
      }
    if (NULL != hscroll)
      if (NULL != (adj = gtk_range_get_adjustment (GTK_RANGE (hscroll))))
        {
        adj->lower = 0 ;
        adj->upper = dialog_data->cxMaxGiven * dialog_data->dScale ;
        adj->page_increment =
        adj->page_size = dialog_data->cxDrawingArea ;
        adj->step_increment = adj->page_size / 10.0 ;
        adj->value = CLAMP (adj->value, adj->lower, adj->upper - adj->page_size) ;
        gtk_adjustment_changed (adj) ;
        gtk_adjustment_value_changed (adj) ;
        }
    dialog_data->icDrawingArea =
    dialog_data->cxDrawingArea =
    dialog_data->cyDrawingArea = 0 ;
    }
  return FALSE ;
  }

void btnPrint_clicked (GtkWidget *widget, gpointer user_data)
  {
  PRINT_GRAPH_DATA pgd = {NULL, NULL, NULL, 10} ;
  graph_D *dialog = (graph_D *)user_data ;
  GRAPH_DIALOG_DATA *gdd = (GRAPH_DIALOG_DATA *)g_object_get_data (G_OBJECT (dialog->dialog), "graph_dialog_data") ;

  print_graph_data_init (&pgd, gdd) ;

  if (get_print_graph_properties_from_user (GTK_WINDOW (dialog->dialog), &print_graph_options, &pgd))
    print_graphs (&print_graph_options, &pgd) ;

  exp_array_free (pgd.bus_traces) ;
  }

void btnZoom100_clicked (GtkWidget *widget, gpointer user_data)
  {
  graph_D *dialog = (graph_D *)user_data ;
  GRAPH_DIALOG_DATA *gdd = NULL ;

  if (NULL == user_data) return ;

  if (NULL == (gdd = g_object_get_data (G_OBJECT (dialog->dialog), "graph_dialog_data"))) return ;

  gdd->dScale = 1.0 ;
  reflect_scale_change (gdd) ;
  }

#ifdef STDIO_FILEIO
void btnPreview_clicked (GtkWidget *widget, gpointer user_data)
  {
  PRINT_GRAPH_DATA pgd = {NULL, NULL, NULL, 10} ;
  graph_D *dialog = (graph_D *)user_data ;
  GRAPH_DIALOG_DATA *gdd = (GRAPH_DIALOG_DATA *)g_object_get_data (G_OBJECT (dialog->dialog), "graph_dialog_data") ;

  if (NULL == gdd) return ;

  print_graph_data_init (&pgd, gdd) ;

  init_print_graph_options (&print_graph_options, &pgd) ;

  do_print_preview ((print_OP *)(&print_graph_options), GTK_WINDOW (dialog->dialog), &pgd, (PrintFunction)print_graphs) ;

  exp_array_free (pgd.bus_traces) ;
  }

void btnOpen_clicked (GtkWidget *widget, gpointer user_data)
  {
  graph_D *dialog = (graph_D *)user_data ;
  static char *pszFName = NULL ;
  char *pszTempFName = NULL ;
  SIMULATION_OUTPUT *sim_output = NULL ;
  GRAPH_DIALOG_DATA *gdd = NULL ;
  double dThreshLower = -0.5, dThreshUpper = 0.5 ;
  int base = 10 ;

  if (NULL == (pszTempFName = get_file_name_from_user (GTK_WINDOW (dialog->dialog), _("Open Simulation Results"), pszFName, FALSE)))
    return ;

  if (NULL != pszFName) g_free (pszFName) ;

  pszFName = pszTempFName ;

  if (NULL == (sim_output = open_simulation_output_file (pszFName)))
    {
    GtkWidget *msg = NULL ;

    gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (dialog->dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
      _("Failed to open simulation data file %s !\n"), pszFName))) ;

    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;
    return ;
    }

  if (NULL != (gdd = g_object_get_data (G_OBJECT (dialog->dialog), "graph_dialog_data")))
    {
    dThreshLower = gdd->dHCThreshLower ;
    dThreshUpper = gdd->dHCThreshUpper ;
    base = gdd->base ;
    }

  g_object_set_data_full (G_OBJECT (dialog->dialog), "graph_dialog_data",
    gdd = graph_dialog_data_new (sim_output, TRUE, dThreshLower, dThreshUpper, base),
    (GDestroyNotify)graph_dialog_data_free) ;

  apply_graph_dialog_data (dialog, gdd) ;
  }

void btnSave_clicked (GtkWidget *widget, gpointer user_data)
  {
  GRAPH_DIALOG_DATA *gdd = g_object_get_data (G_OBJECT (user_data), "graph_dialog_data") ;
  static char *pszFName = NULL ;
  char *pszTempFName = NULL ;
  SIMULATION_OUTPUT sim_output = {NULL, NULL} ;

  if (NULL == gdd) return ;

  if (NULL == (pszTempFName = get_file_name_from_user (GTK_WINDOW (user_data), _("Save Simulation Results"), pszFName, TRUE)))
    return ;

  if (NULL != pszFName) g_free (pszFName) ;

  pszFName = pszTempFName ;

  sim_output.sim_data = gdd->sim_data ;
  sim_output.bus_layout = gdd->bus_layout ;

  create_simulation_output_file (pszFName, &sim_output) ;
  }
#endif /* def STDIO_FILEIO */

void btnClose_clicked (GtkWidget *widget, gpointer user_data)
  {
  GtkWidget *dlgGraphs = GTK_WIDGET (g_object_get_data (G_OBJECT (widget), "dlgGraphs")) ;

  // Since widget can be one of btnClose (if called via "clicked") or the main window
  // (if called via delete_event), we must make sure we have a pointer to the main
  // window.  Since btnClose has a data member dlgGraphs and the main window does not,
  // and the main window /is/ widget when called via delete_event, the following will
  // make sure that dlgGraphs always points to the main window
  if (NULL == dlgGraphs) dlgGraphs = widget ;

  gtk_widget_hide (GTK_WIDGET (dlgGraphs)) ;
  }

void btnThresh_clicked (GtkWidget *widget, gpointer user_data)
  {
  graph_D *dialog = (graph_D *)user_data ;
  GRAPH_DIALOG_DATA *gdd = NULL ;
  double lower, upper ;

  if (NULL == dialog) return ;
  if (NULL == (gdd = g_object_get_data (G_OBJECT (dialog->dialog), "graph_dialog_data"))) return ;

  lower = gdd->dHCThreshLower ;
  upper = gdd->dHCThreshUpper ;

  if (get_honeycomb_thresholds_from_user (dialog->dialog, &lower, &upper))
    {
    gdd->bOneTime = TRUE ;
    gdd->dHCThreshLower = lower ;
    gdd->dHCThreshUpper = upper ;
    recalculate_honeycombs (gdd, TRUE, dialog) ;
    }
  }

void set_bus_expanded (GtkTreeView *tview, GtkTreeIter *itrBus, GtkTreePath *tpath, gpointer data)
  {
  GtkTreeIter itr ;
  GtkTreeModel *tm = gtk_tree_view_get_model (tview) ;
  gboolean bVisible = (gboolean)data ;
  gboolean bModelVisible = TRUE ;
  GtkWidget *trace = NULL, *ruler = NULL, *ui = NULL ;

  if (NULL == tm) return ;

  if (!gtk_tree_model_iter_children (tm, &itr, itrBus)) return ;

  while (TRUE)
    {
    gtk_tree_model_get (tm, &itr,
      GRAPH_MODEL_COLUMN_VISIBLE, &bModelVisible,
      GRAPH_MODEL_COLUMN_TRACE, &trace,
      GRAPH_MODEL_COLUMN_RULER, &ruler,
      GRAPH_MODEL_COLUMN_UI, &ui, -1) ;

    set_trace_widget_visible (trace, bVisible && bModelVisible) ;
    GTK_WIDGET_SET_VISIBLE (ruler, bVisible && bModelVisible) ;
    GTK_WIDGET_SET_VISIBLE (ui, bVisible && bModelVisible) ;
    if (!gtk_tree_model_iter_next (tm, &itr)) return ;
    }
  }

gboolean viewport_scroll (GtkWidget *widget, GdkEventScroll *event, gpointer data)
  {
  graph_D *dialog = (graph_D *)data ;
  double dNewVal ;
  GtkAdjustment *adj = NULL ;
  GdkScrollDirection scroll_direction =
    (GDK_SCROLL_UP   == event->direction && (event->state & GDK_CONTROL_MASK)) ? GDK_SCROLL_LEFT  :
    (GDK_SCROLL_DOWN == event->direction && (event->state & GDK_CONTROL_MASK)) ? GDK_SCROLL_RIGHT : event->direction ;

  if (GDK_SCROLL_UP == scroll_direction || GDK_SCROLL_DOWN == scroll_direction)
    {
    adj = gtk_viewport_get_vadjustment (GTK_VIEWPORT (widget)) ;
    dNewVal = adj->value +
      (adj->step_increment * ((GDK_SCROLL_UP == scroll_direction) ? (-1.0) : (1.0))) ;
    gtk_adjustment_set_value (adj, CLAMP (dNewVal, adj->lower, adj->upper - adj->page_size)) ;
    }
  else
  if (GDK_SCROLL_LEFT == scroll_direction || GDK_SCROLL_RIGHT == scroll_direction)
    {
    adj = gtk_range_get_adjustment (GTK_RANGE (dialog->hscroll)) ;
    dNewVal = adj->value +
      (adj->step_increment * ((GDK_SCROLL_LEFT == scroll_direction) ? (-1.0) : (1.0))) ;
    gtk_adjustment_set_value (adj, CLAMP (dNewVal, adj->lower, adj->upper - adj->page_size)) ;
    }
  return FALSE ;
  }

gboolean graph_widget_button_press (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  double lower, upper, position, max_size ;
  GtkWidget *ruler = g_object_get_data (G_OBJECT (widget), "ruler") ;

  if (bCancelStretch) return FALSE ;

  if ((bCancelStretch = (3 == event->button)))
    if (x_beg != x_old)
      {
      int cx, cy ;
      GdkGC *gc = NULL ;

      gc = gdk_gc_new (widget->window) ;
      gdk_gc_set_function (gc, GDK_XOR) ;
      gdk_gc_set_foreground (gc, clr_idx_to_clr_struct (WHITE)) ;
      gdk_gc_set_background (gc, clr_idx_to_clr_struct (WHITE)) ;
      gdk_window_get_size (widget->window, &cx, &cy) ;
      gdk_draw_rectangle (widget->window, gc, TRUE, MIN (x_beg, x_old), 0, ABS (x_beg - x_old), cy) ;
      x_old = event->x ;
      g_object_unref (gc) ;
      }

  if (1 != event->button) return FALSE ;

  gtk_ruler_get_range (GTK_RULER (ruler), &lower, &upper, &position, &max_size) ;
  beg_sample = CLAMP ((int)position, 0, ((GRAPH_DIALOG_DATA *)data)->sim_data->number_samples) ;
  
  x_beg = 
  x_old =
  x_cur = event->x ;
  return TRUE ;
  }

gboolean graph_widget_motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer data)
  {
  GdkGC *gc = NULL ;
  int cx = 0, cy = 0 ;
  int x, y ;
  GdkModifierType mask ;
  double lower, upper, position, max_size ;
  char *psz = NULL ;
  GtkWidget *label = g_object_get_data (G_OBJECT (widget), "label") ;
  GtkWidget *ruler = NULL ;
  double current_position = 0.0 ;
  gboolean bHaveRuler = FALSE, bHaveRange = FALSE ;

  GtkTreeIter itr ;
  GRAPH_DIALOG_DATA *gdd = (GRAPH_DIALOG_DATA *)data ;
  GtkWidget *trace = NULL ;

  if (NULL == gdd) return FALSE ;

  if ((bHaveRuler = GTK_IS_RULER (widget)))
    gtk_ruler_get_range (GTK_RULER (widget), &lower, &upper, &current_position, &max_size) ;
  else
    {
    ruler = g_object_get_data (G_OBJECT (widget), "ruler") ;
    gdk_window_get_size (widget->window, &cx, &cy) ;
    gtk_ruler_get_range (GTK_RULER (ruler), &lower, &upper, &current_position, &max_size) ;
    current_position = lower + (upper - lower) * (((double)(event->x)) / (double)cx) ;
    }

  current_position = CLAMP (current_position, 0, gdd->sim_data->number_samples) ;

  if (!gtk_tree_model_get_iter_first (gdd->model, &itr)) return FALSE ;
  while (TRUE)
    {
    gtk_tree_model_get (gdd->model, &itr,
      GRAPH_MODEL_COLUMN_RULER, &ruler,
      GRAPH_MODEL_COLUMN_TRACE, &trace, -1) ;
    if (ruler != widget)
      {
      gdk_window_get_size (trace->window, &cx, &cy) ;
      gtk_ruler_get_range (GTK_RULER (ruler), &lower, &upper, &position, &max_size) ;
      gtk_ruler_set_range (GTK_RULER (ruler), lower, upper, current_position, max_size) ;
      }
    if (!gtk_tree_model_iter_next_dfs (gdd->model, &itr)) break ;
    }

  if (!(bHaveRuler || NULL == (ruler = g_object_get_data (G_OBJECT (widget), "ruler")) || bCancelStretch))
    {
    gtk_ruler_get_range (GTK_RULER (ruler), &lower, &upper, &position, &max_size) ;

    if ((bHaveRange = (event->state & GDK_BUTTON1_MASK)))
      {
      gc = gdk_gc_new (widget->window) ;
      gdk_gc_set_function (gc, GDK_XOR) ;
      gdk_gc_set_foreground (gc, clr_idx_to_clr_struct (WHITE)) ;
      gdk_gc_set_background (gc, clr_idx_to_clr_struct (WHITE)) ;
      gdk_window_get_size (widget->window, &cx, &cy) ;

      if (x_beg != x_old)
        gdk_draw_rectangle (widget->window, gc, TRUE, MIN (x_beg, x_old), 0, ABS (x_beg - x_old), cy) ;
      x_old = event->x ;
      if (x_beg != x_old)
        gdk_draw_rectangle (widget->window, gc, TRUE, MIN (x_beg, x_old), 0, ABS (x_beg - x_old), cy) ;

      g_object_unref (gc) ;

      gtk_label_set_text (GTK_LABEL (label),
        psz = g_strdup_printf ("%s %d - %d", _("Sample"), MIN (beg_sample, (int)position), MAX (beg_sample, (int)position))) ;
      g_free (psz) ;
      }
    }

  if (!(NULL == label || NULL == ruler || bHaveRange))
    {
    gtk_label_set_text (GTK_LABEL (label),
      psz = g_strdup_printf ("%s %d", _("Sample"), (int)current_position)) ;
    g_free (psz) ;
    }

  gdk_window_get_pointer (widget->window, &x, &y, &mask) ;

  return FALSE ;
  }

gboolean graph_widget_button_release (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  GdkGC *gc = NULL ;
  int cx = 0, cy = 0 ;
  double lower, upper, position, max_size ;
  GtkWidget *ruler = g_object_get_data (G_OBJECT (widget), "ruler") ;
  GtkWidget *hscroll = g_object_get_data (G_OBJECT (widget), "hscroll") ;
  GRAPH_DIALOG_DATA *graph_dialog_data = (GRAPH_DIALOG_DATA *)data ;
  int end_sample = -1 ;
  GtkAdjustment *adj = NULL ;

  if (1 != event->button) return FALSE ;

  if (bCancelStretch)
    {
    bCancelStretch = FALSE ;
    return FALSE ;
    }

  if (x_beg != x_old)
    {
    gc = gdk_gc_new (widget->window) ;
    gdk_gc_set_function (gc, GDK_XOR) ;
    gdk_gc_set_foreground (gc, clr_idx_to_clr_struct (WHITE)) ;
    gdk_gc_set_background (gc, clr_idx_to_clr_struct (WHITE)) ;
    gdk_window_get_size (widget->window, &cx, &cy) ;
    gdk_draw_rectangle (widget->window, gc, TRUE, MIN (x_beg, x_old), 0, ABS (x_beg - x_old), cy) ;
    x_old = event->x ;
    g_object_unref (gc) ;
    }

  if (NULL != ruler)
    gtk_ruler_get_range (GTK_RULER (ruler), &lower, &upper, &position, &max_size) ;
  // desired range goes from beg_sample to (int)position

  end_sample = CLAMP ((int)position, 0, graph_dialog_data->sim_data->number_samples) ;

  graph_dialog_data->dScale = 
    (((double)(graph_dialog_data->sim_data->number_samples)) / 
      ((double)(ABS (end_sample - beg_sample)))) *
        (((double)cx)/((double)(graph_dialog_data->cxMaxGiven))) ;
  graph_dialog_data->dScale = MAX (1.0, graph_dialog_data->dScale) ;

  graph_dialog_data->xOffset = 
    (((double)beg_sample / ((double)(graph_dialog_data->sim_data->number_samples))) *
      ((double)(graph_dialog_data->cxMaxGiven * graph_dialog_data->dScale))) ;

  if (NULL != hscroll)
    if (NULL != (adj = gtk_range_get_adjustment (GTK_RANGE (hscroll))))
      {
      adj->value = graph_dialog_data->xOffset ;
      gtk_adjustment_value_changed (adj) ;
      }

  reflect_scale_change (graph_dialog_data) ;

  return TRUE ;
  }

gboolean graph_widget_enter_notify (GtkWidget *widget, GdkEventCrossing *event, gpointer data)
  {
  char *psz = NULL ;
  GtkWidget *label = g_object_get_data (G_OBJECT (widget), "label") ;
  GtkWidget *ruler = g_object_get_data (G_OBJECT (widget), "ruler") ;

  if (!(NULL == label || NULL == ruler))
    {
    double lower, upper, position, max_size ;

    gtk_ruler_get_range (GTK_RULER (ruler), &lower, &upper, &position, &max_size) ;
    gtk_label_set_text (GTK_LABEL (label),
      psz = g_strdup_printf ("%s %d", _("Sample"), (int)position)) ;
    g_free (psz) ;
    }

  return FALSE ;
  }

gboolean graph_widget_leave_notify (GtkWidget *widget, GdkEventCrossing *event, gpointer data)
  {
  GtkWidget *label = g_object_get_data (G_OBJECT (widget), "label") ;

  if (NULL != widget)
    gtk_label_set_text (GTK_LABEL (label), "") ;

  return FALSE ;
  }

////////////////////////////////////////////////////////////////////////////////

static void recalculate_honeycombs (GRAPH_DIALOG_DATA *gdd, gboolean bCalcHoneycombArrays, graph_D *dialog)
  {
  GtkTreeIter itr ;
  int row_type ;
  GtkWidget *trace = NULL ;
  HONEYCOMB_DATA *hc = NULL ;
  int cxOldWanted = 0 ;

  if (!gtk_tree_model_get_iter_first (gdd->model, &itr)) return ;
  while (TRUE)
    {
    gtk_tree_model_get (gdd->model, &itr,
      GRAPH_MODEL_COLUMN_TRACE, &trace,
      BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type, -1) ;
    if (row_type & ROW_TYPE_BUS)
      if (NULL != (hc = g_object_get_data (G_OBJECT (trace), "graph_data")))
        {
        hc->graph_data.bNeedCalc = TRUE ;
        cxOldWanted = hc->graph_data.cxWanted ;

        if (bCalcHoneycombArrays)
          calculate_honeycomb_array (hc, gdd->sim_data->number_samples, gdd->dHCThreshLower, gdd->dHCThreshLower, gdd->base) ;
        else
          hc->graph_data.cxWanted = calculate_honeycomb_cxWanted (hc, gdd->sim_data->number_samples, gdd->base) ;

        if (hc->graph_data.cxWanted > hc->graph_data.cxGiven)
          {
          gdd->bOneTime = TRUE ;
          g_signal_connect (G_OBJECT (trace), "expose-event", (GCallback)graph_widget_one_time_expose, dialog) ;
          }
        gtk_widget_queue_draw (trace) ;
        }

    if (!gtk_tree_model_iter_next (gdd->model, &itr)) return ;
    }
  }

static void set_trace_widget_visible (GtkWidget *trace, gboolean bVisible)
  {
  GRAPH_DATA *graph_data = NULL ;

  if (NULL != (graph_data = g_object_get_data (G_OBJECT (trace), "graph_data")))
    {
    graph_data->bVisible = bVisible ;
    if (GRAPH_DATA_TYPE_CELL == graph_data->data_type)
      ((WAVEFORM_DATA *)graph_data)->trace->drawtrace = bVisible ;
    }

  GTK_WIDGET_SET_VISIBLE (trace, bVisible) ;
  }

static void set_ruler_values (GtkWidget *ruler, int cxGiven, int cx, int old_offset, int xOffset, int icSamples)
  {
  char *psz = NULL ;
  double lower, upper, position, max_size ;
  double dxInc = ((double)(icSamples)) / ((double)(cxGiven - 1)) ;
  double old_lower = -old_offset * dxInc ;
  GtkWidget *label = g_object_get_data (G_OBJECT (ruler), "label") ;

  gtk_ruler_get_range (GTK_RULER (ruler), &lower, &upper, &position, &max_size) ;
  lower = -xOffset * dxInc ;
  upper = (-xOffset + cx) * dxInc ;
  position += lower - old_lower ;
  max_size = icSamples ;
  gtk_ruler_set_range (GTK_RULER (ruler), lower, upper, position, max_size) ;

  gtk_label_set_text (GTK_LABEL (label),
    psz = g_strdup_printf ("%s %d", _("Sample"), (int)position)) ;
  g_free (psz) ;

  set_ruler_scale (GTK_RULER (ruler), lower, upper) ;
  }

static void draw_trace_reference_lines (GdkDrawable *dst, int cx, int cy)
  {
  GdkGC *gc = NULL ;
  GdkColor *clrGreen = NULL ;
  int cyOffset = cy * MIN_MAX_OFFSET ;

  gc = gdk_gc_new (dst) ;
  gdk_gc_set_foreground (gc, clrGreen = clr_idx_to_clr_struct (GREEN)) ;
  gdk_gc_set_background (gc, clrGreen) ;
  gdk_gc_set_line_attributes (gc, 1, GDK_LINE_ON_OFF_DASH, GDK_CAP_BUTT, GDK_JOIN_MITER);

  gdk_draw_line (dst, gc, 0, cyOffset, cx, cyOffset) ;
  gdk_draw_line (dst, gc, 0, cy >> 1, cx, cy >> 1) ;
  gdk_draw_line (dst, gc, 0, cy - cyOffset, cx, cy - cyOffset) ;

  g_object_unref (gc) ;
  }

static void print_graph_data_init (PRINT_GRAPH_DATA *print_graph_data, GRAPH_DIALOG_DATA *gdd)
  {
  GtkTreeIter itr ;
  GtkWidget *trace = NULL ;
  int row_type = 0 ;
  HONEYCOMB_DATA *hc = NULL ;

  if (NULL == print_graph_data || NULL == gdd) return ;

  print_graph_data->sim_data = gdd->sim_data ;
  print_graph_data->bus_layout = gdd->bus_layout ;
  print_graph_data->honeycomb_base = gdd->base ;
  print_graph_data->bus_traces = exp_array_new (sizeof (HONEYCOMB_DATA *), 1) ;
  if (gtk_tree_model_get_iter_first (gdd->model, &itr))
    {
    while (TRUE)
      {
      gtk_tree_model_get (gdd->model, &itr,
        GRAPH_MODEL_COLUMN_TRACE, &trace,
        BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type, -1) ;

      if (ROW_TYPE_BUS & row_type)
        {
        hc = g_object_get_data (G_OBJECT (trace), "graph_data") ;
        exp_array_insert_vals (print_graph_data->bus_traces, &hc, 1, 1, -1) ;
        }

      if (!gtk_tree_model_iter_next (gdd->model, &itr)) break ;
      }
    }
  }

void reflect_scale_change (GRAPH_DIALOG_DATA *dialog_data)
  {
  GtkAllocation alloc = {0} ;
  GtkTreeIter itr ;
  GtkWidget *widget_to_size = NULL ;

  dialog_data->cxMaxGiven =
  dialog_data->cyMaxGiven = 0 ;

  if (!gtk_tree_model_get_iter_first (dialog_data->model, &itr)) return ;
  while (TRUE)
    {
    gtk_tree_model_get (dialog_data->model, &itr, GRAPH_MODEL_COLUMN_TRACE, &widget_to_size, -1) ;
    gdk_window_get_size (widget_to_size->window, &(alloc.width), &(alloc.height)) ;
    graph_widget_size_allocate (widget_to_size, &alloc, dialog_data) ;
    if (!gtk_tree_model_iter_next_dfs (dialog_data->model, &itr)) break ;
    }
  }
