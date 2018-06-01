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
// Functions responsible for creating the graph dialog  //
// display elements, including the dialog itself, as    //
// well as the trace widgets.                           //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "support.h"
#include "custom_widgets.h"
#include "global_consts.h"
#include "qcadstock.h"
#include "bus_layout_dialog.h"
#include "graph_dialog_data.h"
#include "graph_dialog_interface.h"
#include "graph_dialog_widget_data.h"
#include "graph_dialog_callbacks.h"

static gboolean create_waveform_widgets (GRAPH_DIALOG_DATA *graph_dialog_data, GtkTreeIter *itr) ;
static gboolean create_bus_widgets (GRAPH_DIALOG_DATA *graph_dialog_data, GtkTreeIter *itr, int base) ;
static GtkWidget *create_trace_drawing_area (GRAPH_DATA *graph_data, GDestroyNotify graph_data_free, GCallback graph_widget_expose, gpointer data) ;

void create_graph_dialog (graph_D *dialog)
  {
  GtkWidget *table = NULL, *toolbar = NULL, *btn = NULL, *tbl_sw = NULL, *tbl_status = NULL,
    *tbl_vp = NULL, *vscroll = NULL, *sw_tview = NULL, *btnBaseRadioSource = NULL, *statusbar = NULL ;
  GtkTreeViewColumn *col = NULL ;
  GtkCellRenderer *cr = NULL ;
	GtkAccelGroup *accel_group = NULL ;

	accel_group = gtk_accel_group_new () ;

  // The Window
  dialog->dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (dialog->dialog), 800, 600);
  gtk_window_set_title (GTK_WINDOW (dialog->dialog), _("Simulation Results"));
  gtk_window_set_modal (GTK_WINDOW (dialog->dialog), FALSE);
  gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), TRUE);

  table = gtk_table_new (3, 1, FALSE) ;
  gtk_widget_show (table) ;
  gtk_container_add (GTK_CONTAINER (dialog->dialog), table) ;

  toolbar = gtk_toolbar_new () ;
  gtk_widget_show (toolbar) ;
  gtk_table_attach (GTK_TABLE (table), toolbar, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 0, 0) ;
  gtk_toolbar_set_orientation (GTK_TOOLBAR (toolbar), GTK_ORIENTATION_HORIZONTAL) ;
  gtk_toolbar_set_tooltips (GTK_TOOLBAR (toolbar), TRUE) ;
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH) ;

  btn = gtk_toolbar_append_element (
    GTK_TOOLBAR (toolbar),
    GTK_TOOLBAR_CHILD_BUTTON,
    NULL,
    _("Close"),
    _("Close Window"),
    _("Close simulation results window."),
    gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_LARGE_TOOLBAR),
    (GCallback)btnClose_clicked,
    NULL) ;
  g_object_set_data (G_OBJECT (btn), "dlgGraphs", dialog->dialog) ;
	gtk_widget_add_accelerator (btn, "clicked", accel_group, GDK_w, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE) ;

  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar)) ;
#ifdef STDIO_FILEIO
  btn = gtk_toolbar_append_element (
    GTK_TOOLBAR (toolbar),
    GTK_TOOLBAR_CHILD_BUTTON,
    NULL,
    _("Open"),
    _("Open Simulation Results"),
    _("Open and display another set of simulation results."),
    gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_LARGE_TOOLBAR),
    (GCallback)btnOpen_clicked,
    dialog) ;
	gtk_widget_add_accelerator (btn, "clicked", accel_group, GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE) ;

  btn = gtk_toolbar_append_element (
    GTK_TOOLBAR (toolbar),
    GTK_TOOLBAR_CHILD_BUTTON,
    NULL,
    _("Save"),
    _("Save Simulation Results"),
    _("Save the displayed simulation results."),
    gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_LARGE_TOOLBAR),
    (GCallback)btnSave_clicked,
    dialog->dialog) ;
	gtk_widget_add_accelerator (btn, "clicked", accel_group, GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE) ;

  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar)) ;

  gtk_toolbar_append_element (
    GTK_TOOLBAR (toolbar),
    GTK_TOOLBAR_CHILD_BUTTON,
    NULL,
    _("Print Preview"),
    _("Preview the print layout"),
    _("Converts graphs to PostScript and runs the previewer application."),
    gtk_image_new_from_stock (GTK_STOCK_PRINT_PREVIEW, GTK_ICON_SIZE_LARGE_TOOLBAR),
    (GCallback)btnPreview_clicked,
    dialog) ;

#endif /* def STDIO_FILEIO */
  gtk_toolbar_append_element (
    GTK_TOOLBAR (toolbar),
    GTK_TOOLBAR_CHILD_BUTTON,
    NULL,
    _("Print"),
    _("Print Graphs"),
    _("Converts graphs to PostScript and prints them to a file or a printer."),
    gtk_image_new_from_stock (GTK_STOCK_PRINT, GTK_ICON_SIZE_LARGE_TOOLBAR),
    (GCallback)btnPrint_clicked,
    dialog) ;

  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar)) ;

  gtk_toolbar_append_element (
    GTK_TOOLBAR (toolbar),
    GTK_TOOLBAR_CHILD_BUTTON,
    NULL,
    _("Reset Zoom"),
    _("Un-stretch Traces"),
    _("Reset the stretch on the traces"),
    gtk_image_new_from_stock (GTK_STOCK_ZOOM_100, GTK_ICON_SIZE_LARGE_TOOLBAR),
    (GCallback)btnZoom100_clicked,
    dialog) ;

  // This will separate the layers combo from the clocks combo
  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar)) ;

  gtk_toolbar_append_element (
    GTK_TOOLBAR (toolbar),
    GTK_TOOLBAR_CHILD_BUTTON,
    NULL,
    _("Thresholds..."),
    _("Set Thresholds"),
    _("Set thresholds for interpreting logical bits from polarizations."),
    gtk_image_new_from_stock (GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_LARGE_TOOLBAR),
    (GCallback)btnThresh_clicked,
    dialog) ;

  // This will separate the layers combo from the clocks combo
  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar)) ;

  g_object_set_data (G_OBJECT (
    btnBaseRadioSource =gtk_toolbar_append_element (
      GTK_TOOLBAR (toolbar),
      GTK_TOOLBAR_CHILD_RADIOBUTTON,
      NULL,
      _("Decimal"),
      _("Show Values As Decimal"),
      _("Display honeycomb values in decimal."),
      gtk_image_new_from_stock (QCAD_STOCK_GRAPH_DEC, GTK_ICON_SIZE_LARGE_TOOLBAR),
      (GCallback)btnShowBase_clicked,
      dialog)),
    "base", (gpointer)10) ;

  g_object_set_data (G_OBJECT (
    gtk_toolbar_append_element (
      GTK_TOOLBAR (toolbar),
      GTK_TOOLBAR_CHILD_RADIOBUTTON,
      btnBaseRadioSource,
      _("Binary"),
      _("Show Values As Binary"),
      _("Display honeycomb values in binary."),
      gtk_image_new_from_stock (QCAD_STOCK_GRAPH_BIN, GTK_ICON_SIZE_LARGE_TOOLBAR),
      (GCallback)btnShowBase_clicked,
      dialog)),
    "base", (gpointer)2) ;

  g_object_set_data (G_OBJECT (
    gtk_toolbar_append_element (
      GTK_TOOLBAR (toolbar),
      GTK_TOOLBAR_CHILD_RADIOBUTTON,
      btnBaseRadioSource,
      _("Hex"),
      _("Show Values As Hexadecimal"),
      _("Display honeycomb values in hexadecimal."),
      gtk_image_new_from_stock (QCAD_STOCK_GRAPH_HEX, GTK_ICON_SIZE_LARGE_TOOLBAR),
      (GCallback)btnShowBase_clicked,
      dialog)),
    "base", (gpointer)16) ;

  dialog->hpaned = gtk_hpaned_new () ;
  gtk_widget_show (dialog->hpaned) ;
  gtk_table_attach (GTK_TABLE (table), dialog->hpaned, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;

  sw_tview = gtk_scrolled_window_new (NULL, NULL) ;
  gtk_widget_show (sw_tview) ;
  gtk_paned_add1 (GTK_PANED (dialog->hpaned), sw_tview) ;
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw_tview), GTK_SHADOW_IN) ;
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw_tview), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC) ;

  dialog->tview = create_bus_layout_tree_view (TRUE, _("Trace"), GTK_SELECTION_SINGLE) ;
  gtk_tree_view_append_column (GTK_TREE_VIEW (dialog->tview), col = gtk_tree_view_column_new ()) ;
  gtk_tree_view_column_set_title (col, _("Visible")) ;
  gtk_tree_view_column_pack_start (col, cr = gtk_cell_renderer_toggle_new (), TRUE) ;
  gtk_tree_view_column_add_attribute (col, cr, "active", GRAPH_MODEL_COLUMN_VISIBLE) ;
  g_object_set (G_OBJECT (cr), "activatable", TRUE, NULL) ;
  gtk_cell_renderer_toggle_set_active (GTK_CELL_RENDERER_TOGGLE (cr), TRUE) ;
  gtk_widget_show (dialog->tview) ;
  gtk_container_add (GTK_CONTAINER (sw_tview), dialog->tview) ;

  tbl_sw = gtk_table_new (2, 2, FALSE) ;
  gtk_widget_show (tbl_sw) ;
  gtk_paned_add2 (GTK_PANED (dialog->hpaned), tbl_sw) ;
  gtk_table_set_row_spacings (GTK_TABLE (tbl_sw), 2) ;
  gtk_table_set_col_spacings (GTK_TABLE (tbl_sw), 2) ;

  dialog->vp = gtk_viewport_new (NULL, NULL) ;
  gtk_widget_show (dialog->vp) ;
  gtk_table_attach (GTK_TABLE (tbl_sw), dialog->vp, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0) ;
  gtk_viewport_set_shadow_type (GTK_VIEWPORT (dialog->vp), GTK_SHADOW_IN) ;
  gtk_widget_set_size_request (dialog->vp, 0, 0) ;

  tbl_vp = gtk_table_new (1, 1, FALSE) ;
  gtk_widget_show (tbl_vp) ;
  gtk_container_add (GTK_CONTAINER (dialog->vp), tbl_vp) ;

  dialog->table_of_traces = gtk_table_new (1, 2, FALSE) ;
  gtk_widget_show (dialog->table_of_traces) ;
  gtk_table_attach (GTK_TABLE (tbl_vp), dialog->table_of_traces, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(0), 0, 0) ;

  dialog->hscroll = gtk_hscrollbar_new (NULL) ;
  gtk_widget_show (dialog->hscroll) ;
  gtk_table_attach (GTK_TABLE (tbl_sw), dialog->hscroll, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 0, 0) ;

  vscroll = gtk_vscrollbar_new (NULL) ;
  gtk_widget_show (vscroll) ;
  gtk_table_attach (GTK_TABLE (tbl_sw), vscroll, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0) ;
  gtk_viewport_set_vadjustment (GTK_VIEWPORT (dialog->vp), gtk_range_get_adjustment (GTK_RANGE (vscroll))) ;

  tbl_status = gtk_table_new (1, 2, FALSE) ;
  gtk_widget_show (tbl_status) ;
  gtk_table_attach (GTK_TABLE (table), tbl_status, 0, 1, 2, 3,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 0, 0) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl_status), 2) ;

  dialog->lbl_status = gtk_label_new ("") ;
  gtk_widget_show (dialog->lbl_status) ;
  gtk_table_attach (GTK_TABLE (tbl_status), dialog->lbl_status, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 0, 0) ;

  statusbar = gtk_statusbar_new () ;
  gtk_widget_show (statusbar) ;
  gtk_table_attach (GTK_TABLE (tbl_status), statusbar, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 0, 0) ;

  gtk_window_add_accel_group (GTK_WINDOW (dialog->dialog), accel_group) ;

  g_signal_connect (G_OBJECT (cr), "toggled", (GCallback)model_visible_toggled, dialog) ;
  g_signal_connect (G_OBJECT (dialog->tview), "row-expanded", (GCallback)set_bus_expanded, (gpointer)TRUE) ;
  g_signal_connect (G_OBJECT (dialog->tview), "row-collapsed", (GCallback)set_bus_expanded, (gpointer)FALSE) ;
  g_signal_connect (G_OBJECT (gtk_range_get_adjustment (GTK_RANGE (dialog->hscroll))), "value-changed", (GCallback)hscroll_adj_value_changed, dialog->dialog) ;
  g_signal_connect (G_OBJECT (dialog->vp), "scroll-event", (GCallback)viewport_scroll, dialog) ;
  g_signal_connect (G_OBJECT (dialog->dialog), "delete-event", (GCallback)btnClose_clicked, NULL) ;
  }

void attach_graph_widgets (graph_D *dialog, GtkWidget *table, GtkWidget *trace, GtkWidget *ruler, GtkWidget *ui, int idxTbl)
  {
  gtk_widget_show (trace) ;
  gtk_widget_set_redraw_on_allocate (trace, FALSE) ;
  gtk_table_attach (GTK_TABLE (table), trace,
    TRACE_TABLE_MIN_X + 2,
    TRACE_TABLE_MIN_X + 3,
    TRACE_TABLE_MIN_Y + ((idxTbl << 1) + 1),
    TRACE_TABLE_MIN_Y + ((idxTbl + 1) << 1),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 2) ;
  gtk_widget_show (ruler) ;
  gtk_table_attach (GTK_TABLE (table), ruler,
    TRACE_TABLE_MIN_X + 2,
    TRACE_TABLE_MIN_X + 3,
    TRACE_TABLE_MIN_Y + (idxTbl << 1),
    TRACE_TABLE_MIN_Y + ((idxTbl << 1) + 1),
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 2) ;
  gtk_widget_show (ui) ;
  gtk_table_attach (GTK_TABLE (table), ui,
    TRACE_TABLE_MIN_X + 1,
    TRACE_TABLE_MIN_X + 2,
    TRACE_TABLE_MIN_Y + ((idxTbl << 1) + 1),
    TRACE_TABLE_MIN_Y + ((idxTbl + 1) << 1),
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 2) ;

  g_signal_connect (G_OBJECT (trace), "expose-event",        (GCallback)graph_widget_one_time_expose, dialog) ;

  set_window_icon (GTK_WINDOW (dialog->dialog), "graph_dialog") ;
  }

gboolean create_graph_widgets (GRAPH_DIALOG_DATA *graph_dialog_data, GtkTreeIter *itr)
  {
  int row_type = -1 ;

  gtk_tree_model_get (graph_dialog_data->model, itr, BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type, -1) ;

  if (ROW_TYPE_CELL & row_type)
    return create_waveform_widgets (graph_dialog_data, itr) ;
  else
  if (ROW_TYPE_BUS & row_type)
    return create_bus_widgets (graph_dialog_data, itr, graph_dialog_data->base) ;
  else
  if (ROW_TYPE_CLOCK == row_type)
    return create_waveform_widgets (graph_dialog_data, itr) ;
  else
    return FALSE ;
  }

static gboolean create_waveform_widgets (GRAPH_DIALOG_DATA *graph_dialog_data, GtkTreeIter *itr)
  {
  int idx = -1, row_type ;
  GtkWidget *tbl = NULL, *lbl = NULL ;
  GtkWidget *trace_ui_widget = NULL, *trace_ruler_widget = NULL, *trace_drawing_widget = NULL ;
  WAVEFORM_DATA *wf = NULL ;
  double dMin = -1.0, dMax = 1.0 ;
  char *psz = NULL ;

  gtk_tree_model_get (graph_dialog_data->model, itr,
    BUS_LAYOUT_MODEL_COLUMN_INDEX, &idx,
    BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type, -1) ;

  wf = waveform_data_new (
    ROW_TYPE_CLOCK == row_type
      ? &(graph_dialog_data->sim_data->clock_data[idx])
      : &(graph_dialog_data->sim_data->trace[idx + (row_type & ROW_TYPE_INPUT ? 0 : graph_dialog_data->bus_layout->inputs->icUsed)]),
      clr_idx_to_clr_struct (ROW_TYPE_CLOCK == row_type ? RED : (row_type & ROW_TYPE_INPUT) ? BLUE : YELLOW),
      ROW_TYPE_CLOCK == row_type) ;

  tracedata_get_min_max (wf->trace, 0, graph_dialog_data->sim_data->number_samples, &dMin, &dMax) ;

  trace_ui_widget = gtk_frame_new (NULL) ;
  tbl = gtk_table_new (3, 1, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_container_add (GTK_CONTAINER (trace_ui_widget), tbl) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;

  lbl = gtk_label_new (psz = g_strdup_printf ("max: %6.2e", dMax)) ;
  g_free (psz) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), 0, 0) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.0) ;

  gtk_tree_model_get (graph_dialog_data->model, itr, BUS_LAYOUT_MODEL_COLUMN_NAME, &psz, -1) ;
  lbl = gtk_label_new (psz) ;
  g_free (psz) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), 0, 0) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  lbl = gtk_label_new (psz = g_strdup_printf ("min: %6.2e", dMin)) ;
  g_free (psz) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 2, 3,
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), 0, 0) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 1.0) ;

  trace_drawing_widget = create_trace_drawing_area ((GRAPH_DATA *)wf, (GDestroyNotify)waveform_data_free, (GCallback)waveform_expose, graph_dialog_data) ;

  trace_ruler_widget = gtk_hruler_new () ;

  g_object_set_data (G_OBJECT (trace_drawing_widget), "ruler", trace_ruler_widget) ;

  g_signal_connect (G_OBJECT (trace_ruler_widget),   "motion-notify-event", (GCallback)graph_widget_motion_notify, graph_dialog_data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "size-allocate",       (GCallback)graph_widget_size_allocate, graph_dialog_data) ;
  g_signal_connect (G_OBJECT (trace_ui_widget),      "size-allocate",       (GCallback)graph_widget_size_allocate, graph_dialog_data) ;

  gtk_tree_store_set (GTK_TREE_STORE (graph_dialog_data->model), itr,
    GRAPH_MODEL_COLUMN_VISIBLE, TRUE,
    GRAPH_MODEL_COLUMN_RULER,   trace_ruler_widget,
    GRAPH_MODEL_COLUMN_TRACE,   trace_drawing_widget,
    GRAPH_MODEL_COLUMN_UI,      trace_ui_widget, -1) ;

  return gtk_tree_model_iter_next (graph_dialog_data->model, itr) ;
  }

static gboolean create_bus_widgets (GRAPH_DIALOG_DATA *graph_dialog_data, GtkTreeIter *itr, int base)
  {
  int idx, bus_type, offset = 0 ;
  GtkTreeIter itrChildren ;
  GtkWidget *trace_drawing_widget = NULL, *trace_ui_widget = NULL, *trace_ruler_widget = NULL, *tbl = NULL, *lbl = NULL ;
  HONEYCOMB_DATA *hc = NULL ;
  char *pszBusName = NULL ;
  struct TRACEDATA *trace = NULL ;

  if (!gtk_tree_model_iter_children (graph_dialog_data->model, &itrChildren, itr)) return FALSE ;

  gtk_tree_model_get (graph_dialog_data->model, itr, BUS_LAYOUT_MODEL_COLUMN_TYPE, &bus_type, -1) ;

  hc = honeycomb_data_new (clr_idx_to_clr_struct ((bus_type & ROW_TYPE_OUTPUT) ? YELLOW : BLUE)) ;

  if (bus_type & ROW_TYPE_OUTPUT) offset = graph_dialog_data->bus_layout->inputs->icUsed ;

  while (TRUE)
    {
    gtk_tree_model_get (graph_dialog_data->model, &itrChildren, BUS_LAYOUT_MODEL_COLUMN_INDEX, &idx, -1) ;
    trace = &((graph_dialog_data->sim_data->trace)[idx + offset]) ;
    exp_array_insert_vals (hc->arTraces, &trace, 1, 1, -1) ;
    if (!gtk_tree_model_iter_next (graph_dialog_data->model, &itrChildren)) break ;
    }

  calculate_honeycomb_array (hc, graph_dialog_data->sim_data->number_samples, graph_dialog_data->dHCThreshLower, graph_dialog_data->dHCThreshUpper, base) ;

  gtk_tree_model_get (graph_dialog_data->model, itr, BUS_LAYOUT_MODEL_COLUMN_NAME, &pszBusName, -1) ;

  trace_ui_widget = gtk_frame_new (NULL) ;
  tbl = gtk_table_new (1, 1, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_container_add (GTK_CONTAINER (trace_ui_widget), tbl) ;

  lbl = gtk_label_new (pszBusName) ;
  g_free (pszBusName) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5) ;

  trace_drawing_widget = create_trace_drawing_area ((GRAPH_DATA *)hc, (GDestroyNotify)honeycomb_data_free, (GCallback)honeycomb_expose, graph_dialog_data) ;

  trace_ruler_widget = gtk_hruler_new () ;

  g_object_set_data (G_OBJECT (trace_drawing_widget), "ruler", trace_ruler_widget) ;

  g_signal_connect (G_OBJECT (trace_ruler_widget),   "motion-notify-event", (GCallback)graph_widget_motion_notify, graph_dialog_data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "size-allocate",       (GCallback)graph_widget_size_allocate, graph_dialog_data) ;
  g_signal_connect (G_OBJECT (trace_ui_widget),      "size-allocate",       (GCallback)graph_widget_size_allocate, graph_dialog_data) ;

  gtk_tree_store_set (GTK_TREE_STORE (graph_dialog_data->model), itr,
    GRAPH_MODEL_COLUMN_VISIBLE, TRUE,
    GRAPH_MODEL_COLUMN_RULER,   trace_ruler_widget,
    GRAPH_MODEL_COLUMN_TRACE,   trace_drawing_widget,
    GRAPH_MODEL_COLUMN_UI,      trace_ui_widget, -1) ;

  if (gtk_tree_model_iter_children (graph_dialog_data->model, &itrChildren, itr))
    while (create_waveform_widgets (graph_dialog_data, &itrChildren)) ;

  return gtk_tree_model_iter_next (graph_dialog_data->model, itr) ;
  }

static GtkWidget *create_trace_drawing_area (GRAPH_DATA *graph_data, GDestroyNotify graph_data_free, GCallback graph_widget_expose, gpointer data)
  {
  GtkWidget *trace_drawing_widget = gtk_drawing_area_new () ;
  gtk_widget_add_events (trace_drawing_widget, GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK) ;
  set_widget_background_colour (trace_drawing_widget, 0, 0, 0) ;
  g_object_set_data_full (G_OBJECT (trace_drawing_widget), "graph_data", graph_data, (GDestroyNotify)graph_data_free) ;
  gtk_widget_set_size_request (trace_drawing_widget, -1, TRACE_MIN_CY) ;
  gtk_widget_add_events (trace_drawing_widget,
    GDK_EXPOSURE_MASK |
    GDK_POINTER_MOTION_MASK |
    GDK_POINTER_MOTION_HINT_MASK |
    GDK_BUTTON_PRESS_MASK |
    GDK_BUTTON_RELEASE_MASK |
    GDK_ENTER_NOTIFY_MASK |
    GDK_LEAVE_NOTIFY_MASK) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "expose-event",         (GCallback)graph_widget_expose,         data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "button-press-event",   (GCallback)graph_widget_button_press,   data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "motion-notify-event",  (GCallback)graph_widget_motion_notify,  data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "button-release-event", (GCallback)graph_widget_button_release, data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "enter-notify-event",   (GCallback)graph_widget_enter_notify,   data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "leave-notify-event",   (GCallback)graph_widget_leave_notify,   data) ;

  return trace_drawing_widget ;
  }
