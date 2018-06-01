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
// This file contains the callback functions for main   //
// window UI elements.                                  //
//                                                      //
//////////////////////////////////////////////////////////

// -- includes -- //
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
  #include <unistd.h>
#endif /* ndef WIN32 */
#include <math.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "simulation.h"
#include "design.h"
#ifdef STDIO_FILEIO
  #include "fileio.h"
  #include "fileio_helpers.h"
  #include "recent_files.h"
#endif /* def STDIO_FILEIO */
#include "print_preview.h"
#include "print.h"
#include "vector_table.h"
#include "callback_helpers.h"
#include "actions.h"
#include "objects/object_helpers.h"
#include "objects/QCADDOContainer.h"
#include "objects/QCADCompoundDO.h"
#ifdef UNDO_REDO
  #include "selection_undo.h"
  #include "objects/QCADUndoEntryGroup.h"
#endif /* def UNDO_REDO */
#include "objects/QCADRuler.h"
#include "objects/mouse_handlers.h"
#include "actions/action_handlers.h"
#include "global_consts.h"
#include "qcadstock.h"
#include "objects/QCADClockCombo.h"
#include "generic_utils.h"

// dialogs and windows used //
#include "about.h"
#include "file_selection_window.h"
#include "scale_dialog.h"
#include "print_properties_dialog.h"
#include "sim_engine_setup_dialog.h"
#include "graph_dialog.h"
#include "custom_widgets.h"
#include "layer_properties_dialog.h"
#include "layer_order_dialog.h"
#include "translate_selection_dialog.h"
#include "layer_mapping_dialog.h"
#include "bus_layout_dialog.h"
#include "vector_table_options_dialog.h"

#define DBG_CB(s)
#define DBG_CB_HERE(s)
#define DBG_LAYER(s)
#define DBG_MERGE(s)

#define PAN_STEP 10

typedef struct
  {
  DESIGN *design ;
  double dScale ;
  } UNDO_SCALE_PARAMS ;

typedef struct
  {
  gboolean bHaveIdler ;
  GdkRegion *rgn ;
  } REDRAW_ASYNC_PARAMS ;

extern VectorTable *pvt ;
extern main_W main_window ;
extern gboolean STOP_SIMULATION;
extern GdkColor clrYellow ;
extern GdkColor clrBlack ;

static print_design_OP print_options =
  {{612, 792, 72, 72, 72, 72, TRUE, TRUE, NULL}, 3, TRUE, FALSE, TRUE, TRUE, NULL, 0, 1, 1, 1} ;

extern char *layer_pixmap_stock_id[LAYER_TYPE_LAST_TYPE] ;

static project_OP project_options =
  {
  NULL,                          // The design
  {0, 0x0000, 0xffff, 0xffff},   // clrCyan
  {0, 0xffff, 0xffff, 0xffff},   // clrWhite
//!Current simulation engine.
  COHERENCE_VECTOR_ENERGY,                 // SIMULATION_ENGINE
//!Maximum random response function shift.
  0.0,                      // max_response_shift
//!Probability that a design cell will be affected by the random response function shift.
  0.0,                      // affected_cell_probability
//!Has the design been altered ?
  FALSE,                    // bDesignAltered
//!Currently selected cell type
  1,                        // selected_cell_type
//!General CAD options
  TRUE,                     // SHOW_GRID
//!Current project file name.
  NULL,                     // pszCurrentFName
//!Current simulation type.
  EXHAUSTIVE_VERIFICATION,
// When copying cells or importing a block, the newly created cells must first be placed
// before they can be added to the undo stack.  Upon dropping them onto the design without
// overlap, when set, this variable causes a "Create" event to be added to the undo stack,
// rather than a "Move" event.
  FALSE,
// The vector table
  NULL,
  &main_window,
  NULL
  } ;

static GtkWidget *selected_layer_item = NULL ;

static void propagate_motion_to_rulers (GtkWidget *widget, GdkEventMotion *event) ;
static void tabula_rasa (GtkWindow *wndMain) ; /* "Clean slate" - reset QCADesigner for a new project */
#ifdef STDIO_FILEIO
static gboolean DoSave (GtkWindow *parent, int fFileOp) ;
static gboolean SaveDirtyUI (GtkWindow *parent, char *szMsg) ;
static QCADSubstrate *find_snap_source (DESIGN *design) ;
static QCADDesignObject *merge_selection (DESIGN *design, DESIGN *block, EXP_ARRAY *layer_mappings) ;
#endif /* def STDIO_FILEIO */
static void layers_combo_fill_from_design (main_W *wndMain, DESIGN *design) ;
static GtkWidget *layers_combo_add_layer (GtkCombo *combo, GList *layer) ;
static void layers_combo_refresh_item (GtkWidget *item) ;
static void layers_combo_item_set_check_buttons (GtkWidget *item, QCADLayer *layer, GtkWidget *chkVisible, GtkWidget *chkActive) ;
static gboolean redraw_async_cb (REDRAW_ASYNC_PARAMS *parms) ;
static void reflect_layer_status (QCADLayer *layer) ;
static gboolean drop_single_object_cb (QCADDesignObject *obj) ;
static gboolean drop_single_object_with_undo_cb (QCADDesignObject *obj) ;
static void layer_apply_default_properties (gpointer key, gpointer value, gpointer user_data) ;
static void rotate_single_cell_cb (DESIGN *design, QCADDesignObject *obj, gpointer data) ;
static void destroy_notify_layer_combo_item_layer_data (QCADLayer *layer) ;
static void set_current_design (DESIGN *new_design, QCADSubstrate *subs) ;
static void snap_source_is_gone (gpointer data, QCADSubstrate *subs) ;
static void layer_status_change (GtkWidget *widget, gpointer data) ;
static void layer_select (GtkWidget *widget, gpointer data) ;
static void cell_function_changed (QCADCell *cell, gpointer data) ;
static void move_selection_to_pointer (QCADDesignObject *anchor) ;
static void place_popup_menu (GtkMenu *menu, int *x, int *y, gboolean *push_in, gpointer data) ;
static void mirror_selection_direction_chosen (GtkWidget *widget, gpointer data) ;
static void cell_display_mode_chosen (GtkWidget *widget, gpointer data) ;
static void real_coords_from_rulers (int *px, int *py) ;
#ifdef UNDO_REDO
static void undo_state_changed (QCADUndoEntryGroup *stack, QCADUndoState state, gpointer data) ;
static void scale_cells_undo_apply (QCADUndoEntry *entry, gboolean bUndo, gpointer data) ;
#endif /* def UNDO_REDO */
static void qcad_layer_design_object_added (QCADLayer *layer, QCADDesignObject *obj, gpointer data) ;
static void qcad_layer_design_object_removed (QCADLayer *layer, QCADDesignObject *obj, gpointer data) ;

static ACTION actions[ACTION_LAST_ACTION] =
  {
  // ACTION_SELECT
  {0,
  {
  (GCallback)button_pressed_ACTION_SELECT,
  (GCallback)motion_notify_ACTION_SELECT,
  (GCallback)button_released_ACTION_SELECT
  },
  (gpointer)&project_options, NULL},

  // ACTION_QCELL
  {0, // Needs to be set to QCAD_TYPE_CELL in main_window_show
    {NULL, NULL, NULL},
  NULL, drop_single_object_with_undo_cb},

  // ACTION_ARRAY
  {0,
  {
  (GCallback)button_pressed_ACTION_ARRAY,
  (GCallback)motion_notify_ACTION_ARRAY,
  (GCallback)button_released_ACTION_ARRAY
  },
  (gpointer)&project_options, drop_single_object_cb},

  // ACTION_OVAL
  {0, {NULL, NULL, NULL}, NULL, NULL},

  // ACTION_POLYGON
  {0, {NULL, NULL, NULL}, NULL, NULL},

  // ACTION_SUBSTRATE
  {0, // Needs to be set to QCAD_TYPE_SUBSTRATE in main_window_show
    {NULL, NULL, NULL},
  NULL, drop_single_object_with_undo_cb},

  // ACTION_LABEL
  {0, // Needs to be set to QCAD_TYPE_LABEL in main_window_show
    {NULL, NULL, NULL},
  NULL, drop_single_object_with_undo_cb},

  // ACTION_RULER
  {0, // Needs to be set to QCAD_TYPE_RULER in main_window_show
    {NULL, NULL, NULL},
  NULL, drop_single_object_with_undo_cb},

  // ACTION_PAN
  {0,
    {
    (GCallback)button_pressed_ACTION_PAN,
    (GCallback)motion_notify_ACTION_PAN,
    NULL
    },
  NULL, NULL},

  // ACTION_CHOOSE_SNAP_SOURCE
  {0, {NULL, NULL, NULL}, NULL, NULL},

  // ACTION_ROTATE
  {0,
    {
    (GCallback)button_pressed_ACTION_ROTATE,
    NULL,
    NULL
    },
  (gpointer)&project_options, NULL}
  } ;

static int xRef, yRef, xOld, yOld ; // zoom window coordinates

void main_window_show (GtkWidget *widget, gpointer data)
  {
  DESIGN *new_design = NULL ;
  QCADSubstrate *subs = NULL ;
  // This vector table is NEVER CLEARED ! Attach the appropriate signal and clear it there
  // (like destroy_event, or hide, or something)
  pvt = project_options.pvt = VectorTable_new () ;
  project_options.bScrolling = FALSE ;

#ifdef UNDO_REDO
  g_signal_connect (G_OBJECT (qcad_undo_entry_group_get_default ()), "state-changed", (GCallback)undo_state_changed, NULL) ;
#endif /* def UNDO_REDO */
#ifdef STDIO_FILEIO
  fill_recent_files_menu (main_window.recent_files_menu, GTK_SIGNAL_FUNC (file_operations), (gpointer)FILEOP_OPEN_RECENT) ;
#endif /* def STDIO_FILEIO */
  // The .type fields need to be initialized here, because type numbers are assigned at runtime
      actions[ACTION_QCELL].type = QCAD_TYPE_CELL ;
  actions[ACTION_SUBSTRATE].type = QCAD_TYPE_SUBSTRATE ;
      actions[ACTION_LABEL].type = QCAD_TYPE_LABEL ;
      actions[ACTION_RULER].type = QCAD_TYPE_RULER ;

  gdk_color_alloc (gdk_colormap_get_system (), &(project_options.clrWhite)) ;
  gdk_color_alloc (gdk_colormap_get_system (), &(project_options.clrCyan)) ;

  print_options.pbPrintedObjs = NULL ;
  print_options.icPrintedObjs = 0 ;

  new_design = design_new (&subs) ;
  set_current_design (new_design, subs) ;
  action_button_clicked (main_window.default_action_button, ACTION_SELECT) ;

  gtk_paned_set_position (GTK_PANED (main_window.vpaned1), gtk_paned_get_position (GTK_PANED (main_window.vpaned1))) ;
  }

#ifdef UNDO_REDO
static void undo_state_changed (QCADUndoEntryGroup *stack, QCADUndoState state, gpointer data)
  {
  gtk_widget_set_sensitive (main_window.undo_menu_item, state & QCAD_CAN_UNDO) ;
  gtk_widget_set_sensitive (main_window.redo_menu_item, state & QCAD_CAN_REDO) ;
  }
#endif /* def UNDO_REDO */

// This function gets called during a resize
gboolean configure_event (GtkWidget *widget, GdkEventConfigure *event, gpointer user_data)
  {
  int x, y ;
  DBG_CB_HERE (fprintf (stderr, "Entering main_window_configure_event\n")) ;
  set_client_dimension (event->width, event->height) ;
  real_coords_from_rulers (&x, &y) ;
  setup_rulers (x, y) ;
  // This function needs to return a value.
  // this is the source of one of the compiler warnings.
  return FALSE;
  }

void on_hide_layers_menu_item_activate (GtkWidget *widget, gpointer data)
  {
  GtkWidget *combo_item = NULL ;
  GtkWidget *chkVisible = NULL ;
  GList *llItr = NULL ;

  for (llItr = project_options.design->lstLayers ; llItr != NULL ; llItr = llItr->next)
    if (llItr != project_options.design->lstCurrentLayer)
      if (NULL != (combo_item = QCAD_LAYER (llItr->data)->combo_item))
        if (NULL != (chkVisible = g_object_get_data (G_OBJECT (combo_item), "chkVisible")))
          gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkVisible), FALSE) ;

  if (NULL != (combo_item = QCAD_LAYER (project_options.design->lstCurrentLayer->data)->combo_item))
    if (NULL != (chkVisible = g_object_get_data (G_OBJECT (combo_item), "chkVisible")))
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkVisible), TRUE) ;

  layer_selected (NULL, NULL) ;
  }

gboolean drawing_area_button_pressed (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
  {
  // Return to default action
  if (3 == event->button)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (main_window.default_action_button), TRUE) ;
  else
  // Zoom
  if (2 == event->button)
    {
    xOld = xRef = event->x ;
    yOld = yRef = event->y ;
    }
  return FALSE ;
  }

gboolean drawing_area_motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
  {
  int x, y ;
  GdkModifierType mask ;

  propagate_motion_to_rulers (widget, event) ;

  if (event->state & GDK_BUTTON2_MASK)
    {
    GdkGC *gc = gdk_gc_new (widget->window) ;

    gdk_gc_set_function (gc, GDK_XOR) ;
    gdk_gc_set_foreground (gc, &(project_options.clrWhite)) ;
    if (ABS (xRef - xOld) > 0 && ABS (yRef - yOld) > 0)
      gdk_draw_rectangle (widget->window, gc, FALSE, MIN (xRef, xOld), MIN (yRef, yOld), ABS (xRef - xOld), ABS (yRef - yOld)) ;
    xOld = event->x ;
    yOld = event->y ;
    if (ABS (xRef - xOld) > 0 && ABS (yRef - yOld) > 0)
      gdk_draw_rectangle (widget->window, gc, FALSE, MIN (xRef, xOld), MIN (yRef, yOld), ABS (xRef - xOld), ABS (yRef - yOld)) ;

    g_object_unref (gc) ;
    }

  gdk_window_get_pointer (widget->window, &x, &y, &mask) ;

  return FALSE;
  }

gboolean drawing_area_button_released (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
  {
  if (event->state & GDK_BUTTON2_MASK && !((event->state & GDK_BUTTON3_MASK) || (event->state & GDK_BUTTON1_MASK)))
    {
    GdkGC *gc = gdk_gc_new (widget->window) ;

    gdk_gc_set_function (gc, GDK_XOR) ;
    gdk_gc_set_foreground (gc, &(project_options.clrWhite)) ;
    if (ABS (xRef - xOld) > 0 && ABS (yRef - yOld) > 0)
      gdk_draw_rectangle (widget->window, gc, FALSE, MIN (xRef, xOld), MIN (yRef, yOld), ABS (xRef - xOld), ABS (yRef - yOld)) ;

    g_object_unref (gc) ;

    zoom_window (MIN (xRef, xOld), MIN (yRef, yOld), MAX (xRef, xOld), MAX (yRef, yOld)) ;
    reflect_zoom () ;
    }
  return FALSE ;
  }

void toggle_alt_display_button_clicked (GtkWidget *widget, gpointer data)
  {
  static GtkWidget *mnu = NULL ;
  static GdkPoint pt = {-1, -1} ;

  if (NULL == mnu)
    {
    GtkWidget *mnui = NULL, *sep = NULL, *img = NULL ;

    mnu = gtk_menu_new () ;

    mnui = gtk_image_menu_item_new_with_label (_("Crossover")) ;
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (mnui),
    img = gtk_image_new_from_stock (QCAD_STOCK_CELL_ALT_CROSSOVER, QCAD_ICON_SIZE_SIDE_TOOLBAR)) ;
    gtk_widget_show (mnui) ;
    gtk_widget_show (img) ;
    gtk_container_add (GTK_CONTAINER (mnu), mnui) ;
    g_signal_connect (G_OBJECT (mnui), "activate", (GCallback)cell_display_mode_chosen, (gpointer)QCAD_CELL_MODE_CROSSOVER) ;

    sep = gtk_menu_item_new () ;
    gtk_widget_show (sep) ;
    gtk_container_add (GTK_CONTAINER (mnu), sep) ;

    mnui = gtk_image_menu_item_new_with_label (_("Normal")) ;
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (mnui),
    img = gtk_image_new_from_stock (QCAD_STOCK_CELL, QCAD_ICON_SIZE_SIDE_TOOLBAR)) ;
    gtk_widget_show (mnui) ;
    gtk_widget_show (img) ;
    gtk_container_add (GTK_CONTAINER (mnu), mnui) ;
    g_signal_connect (G_OBJECT (mnui), "activate", (GCallback)cell_display_mode_chosen, (gpointer)QCAD_CELL_MODE_NORMAL) ;

    sep = gtk_menu_item_new () ;
    gtk_widget_show (sep) ;
    gtk_container_add (GTK_CONTAINER (mnu), sep) ;

    mnui = gtk_image_menu_item_new_with_label (_("Vertical")) ;
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (mnui),
    img = gtk_image_new_from_stock (QCAD_STOCK_CELL_ALT_VERTICAL, QCAD_ICON_SIZE_SIDE_TOOLBAR)) ;
    gtk_widget_show (mnui) ;
    gtk_widget_show (img) ;
    gtk_container_add (GTK_CONTAINER (mnu), mnui) ;
    g_signal_connect (G_OBJECT (mnui), "activate", (GCallback)cell_display_mode_chosen, (gpointer)QCAD_CELL_MODE_VERTICAL) ;
    }

  gtk_widget_get_root_origin (widget, &(pt.x), &(pt.y)) ;

  gtk_menu_popup (GTK_MENU (mnu), NULL, NULL, (GtkMenuPositionFunc)place_popup_menu, &pt, 1, gtk_get_current_event_time ()) ;
  }

static void cell_display_mode_chosen (GtkWidget *widget, gpointer data)
  {
  EXP_ARRAY *objs = NULL ;
  GValue val ;

  selection_renderer_draw (project_options.srSelection, project_options.design, main_window.drawing_area->window, GDK_XOR) ;
  if (NULL != (objs = design_selection_get_object_array (project_options.design)))
    {
    memset (&val, 0, sizeof (GValue)) ;
    g_value_set_enum (g_value_init (&val, QCAD_TYPE_CELL_MODE), (int)data) ;
#ifdef UNDO_REDO
    push_undo_selection_state (project_options.design, project_options.srSelection, main_window.drawing_area->window, objs, "mode", &val) ;
#endif /* def UNDO_REDO */
    g_value_unset (&val) ;
    }
  design_selection_set_cell_display_mode (project_options.design, (int)data) ;
  selection_renderer_update (project_options.srSelection, project_options.design) ;
  selection_renderer_draw (project_options.srSelection, project_options.design, main_window.drawing_area->window, GDK_XOR) ;
  }

void add_layer_button_clicked (GtkWidget *widget, gpointer data)
  {
  QCADLayer *layer = qcad_layer_new (LAYER_TYPE_CELLS, LAYER_STATUS_VISIBLE, _("New Layer")) ;

  if (get_layer_properties_from_user (GTK_WINDOW (main_window.main_window), layer, TRUE, TRUE))
    {
    // If there was only one layer, enable its check buttons
    if (project_options.design->lstLayers == project_options.design->lstLastLayer)
      {
      GtkWidget *chk = NULL, *combo_item = (QCAD_LAYER (project_options.design->lstLayers->data))->combo_item ;

      if (NULL != combo_item)
        {
        if (NULL != (chk = g_object_get_data (G_OBJECT (combo_item), "chkVisible")))
          gtk_widget_set_sensitive (chk, TRUE) ;
        if (NULL != (chk = g_object_get_data (G_OBJECT (combo_item), "chkActive")))
          gtk_widget_set_sensitive (chk, TRUE) ;
        }
      gtk_widget_set_sensitive (main_window.remove_layer_button, TRUE) ;
      }

    design_layer_add (project_options.design, layer) ;
    (QCAD_LAYER (project_options.design->lstLayers->data))->combo_item =
      layers_combo_add_layer (GTK_COMBO (main_window.layers_combo), project_options.design->lstLayers) ;

    selected_layer_item = (QCAD_LAYER (project_options.design->lstLayers->data))->combo_item ;
    layer_selected (NULL, NULL) ;
    }
  else
    g_object_unref (layer) ;
  }

void remove_layer_button_clicked (GtkWidget *widget, gpointer data)
  {
  GList *llItem = NULL ;

  // If there is more than one layer in the design ...
  if (project_options.design->lstLastLayer != project_options.design->lstLayers)
    {
    GtkWidget *msg = NULL ;
    QCADLayer *layer = g_object_get_data (G_OBJECT (selected_layer_item), "layer") ;

    if (GTK_RESPONSE_YES == gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (main_window.main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
      _("Once deleted, a layer cannot be undeleted. Do you really want to delete layer \"%s\"?"),
        NULL == layer ? _("(null)") : NULL == layer->pszDescription ? _("Untitled") : layer->pszDescription))))
      {
      gtk_widget_hide (msg) ;
      gtk_widget_destroy (msg) ;

      llItem = g_list_prepend (NULL, selected_layer_item) ;
      gtk_list_remove_items (GTK_LIST (GTK_COMBO (main_window.layers_combo)->list), llItem) ;

      layer_selected (NULL, NULL) ;
      g_list_free (llItem) ;
      }
    else
      {
      gtk_widget_hide (msg) ;
      gtk_widget_destroy (msg) ;
      }
    }

  gtk_widget_set_sensitive (widget, (project_options.design->lstLastLayer != project_options.design->lstLayers)) ;
  }

void layer_properties_button_clicked (GtkWidget *widget, gpointer data)
  {
  QCADLayer *layer = NULL ;

  if (NULL != project_options.design->lstCurrentLayer)
    if (NULL != project_options.design->lstCurrentLayer->data)
      if (get_layer_properties_from_user (GTK_WINDOW (main_window.main_window),
        layer = (QCAD_LAYER (project_options.design->lstCurrentLayer->data)), FALSE,
        !(project_options.design->lstLayers == project_options.design->lstLastLayer)))
          {
          if (NULL != layer->combo_item)
            layers_combo_refresh_item (layer->combo_item) ;
          // Apply default properties for all object classes in this layer
          if (NULL != layer->default_properties)
            g_hash_table_foreach (layer->default_properties, layer_apply_default_properties, NULL) ;
          }
  }

void bus_layout_button_clicked (GtkWidget *widget, gpointer data)
  {get_bus_layout_from_user (GTK_WINDOW (main_window.main_window), project_options.design->bus_layout) ;}

void reorder_layers_button_clicked (GtkWidget *widget, gpointer user_data)
  {
  GtkWidget *combo_item = NULL ;
  GList *llItr = NULL, *llLayerOrder = NULL, *llListItems = NULL ;

  if (NULL != (llLayerOrder = get_layer_order_from_user (GTK_WINDOW (main_window.main_window), project_options.design)))
    {
    for (llListItems = NULL, llItr = project_options.design->lstLayers ; llItr != NULL ; llItr = llItr->next)
      if (NULL != (combo_item = (QCAD_LAYER (llItr->data)->combo_item)))
        llListItems = g_list_prepend (llListItems, combo_item) ;

    gtk_list_remove_items_no_unref (GTK_LIST (GTK_COMBO (main_window.layers_combo)->list), llListItems) ;
    g_list_free (llListItems) ;

    design_set_layer_order (project_options.design, llLayerOrder) ;

    g_list_free (llLayerOrder) ;
    for (llListItems = NULL, llItr = project_options.design->lstLayers ; llItr != NULL ; llItr = llItr->next)
      llListItems = g_list_prepend (llListItems, (QCAD_LAYER (llItr->data))->combo_item) ;
    gtk_list_append_items (GTK_LIST (GTK_COMBO (main_window.layers_combo)->list), llListItems) ;
    }

  redraw_async (NULL) ;
  selected_layer_item = (QCAD_LAYER (project_options.design->lstCurrentLayer->data))->combo_item ;
  layer_selected (NULL, NULL) ;
  }

gboolean scroll_event(GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
  {
  int iDir =
    GDK_SCROLL_UP == event->direction ? 1 :       // Mouse wheel clicked away from the user
    GDK_SCROLL_DOWN == event->direction ? -1 : 0, // Mouse wheel clicked towards the user
    idx = event->state & GDK_CONTROL_MASK ? 0 : 1,
    x = -1, y = -1,
    dist[2] = {0, 0} ;

  DBG_CB_HERE (fprintf (stderr, "Entering scroll_event\n")) ;

  dist[idx] = 10 * PAN_STEP * iDir ;

  pan (dist[0], dist[1]) ;
  project_options.bScrolling = TRUE ;
  gdk_window_scroll (main_window.drawing_area->window, dist[0], dist[1]) ;
  real_coords_from_rulers (&x, &y) ;
  x += dist[0] ;
  y += dist[1] ;
  setup_rulers (x, y) ;
  return FALSE ;
  }

gboolean expose_event(GtkWidget * widget, GdkEventExpose *event, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering expose_event\n")) ;

  if (NULL == project_options.srSelection)
    project_options.srSelection = selection_renderer_new (main_window.drawing_area) ;

  redraw_sync (event->region, FALSE) ;

  return FALSE;
  }
#ifdef STDIO_FILEIO
void on_preview_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering on_preview_menu_item_activate\n")) ;
  init_print_design_options (&print_options, project_options.design) ;
  do_print_preview ((print_OP *)&print_options, GTK_WINDOW (main_window.main_window), (void *)(project_options.design), (PrintFunction)print_world) ;
  }
#endif /* def STDIO_FILEIO */
void on_scale_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data)
  {
  double scale = 1.0 ;

  get_scale_from_user (GTK_WINDOW (main_window.main_window), &scale) ;

  if (design_scale_cells (project_options.design, scale))
#ifdef UNDO_REDO
    {
    UNDO_SCALE_PARAMS *params = NULL ;

    params = g_malloc0 (sizeof (UNDO_SCALE_PARAMS)) ;

    params->design = project_options.design ;
    params->dScale = scale ;

    qcad_undo_entry_group_push (qcad_undo_entry_group_get_default (),
      qcad_undo_entry_new_with_callbacks ((GCallback)scale_cells_undo_apply, params, (GDestroyNotify)g_free)) ;
    }
#else
  ;
#endif /* UNDO_REDO */
  selection_renderer_update (project_options.srSelection, project_options.design) ;
  redraw_sync (NULL, FALSE) ;
  }

#ifdef UNDO_REDO
static void scale_cells_undo_apply (QCADUndoEntry *entry, gboolean bUndo, gpointer data)
  {
  UNDO_SCALE_PARAMS *params = (UNDO_SCALE_PARAMS *)data ;

  if (0 == params->dScale && bUndo) return ;

  design_scale_cells (params->design, bUndo ? 1.0 / params->dScale : params->dScale) ;
  }
#endif /* UNDO_REDO */

void on_show_tb_icons_menu_item_activate (GtkMenuItem * menuitem, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering on_show_tb_icons_menu_item_activate\n")) ;
  gtk_toolbar_set_style (GTK_TOOLBAR (main_window.toolbar),
    gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (menuitem)) ?
    GTK_TOOLBAR_BOTH_HORIZ : GTK_TOOLBAR_TEXT) ;
  }

// toggle the snap to grid option //
void on_snap_to_grid_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering on_snap_to_grid_menu_item_activate\n")) ;
  set_snap_source (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (menuitem)) ?
    g_object_get_data (G_OBJECT (menuitem), "snap_source") : NULL) ;
  }

// toggle the show grid option //
void on_show_grid_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering on_show_grid_menu_item_activate\n")) ;
  project_options.SHOW_GRID = ((GtkCheckMenuItem *) menuitem)->active;
  }

void on_clock_increment_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  gboolean bFoundSelection = FALSE ;
  GList *lstItrLayer = NULL, *lstItrSel = NULL ;
  EXP_ARRAY *arSelObjs = NULL ;

  arSelObjs = design_selection_get_object_array (project_options.design) ;
#ifdef UNDO_REDO
  if (NULL != arSelObjs)
    push_undo_selection_clock (project_options.design, project_options.srSelection, main_window.drawing_area->window, arSelObjs, 1, TRUE) ;
#endif /* def UNDO_REDO */

  for (lstItrLayer = project_options.design->lstLayers ; lstItrLayer != NULL ; lstItrLayer = lstItrLayer->next)
    if (LAYER_TYPE_CELLS == (QCAD_LAYER (lstItrLayer->data))->type)
      for (lstItrSel = (QCAD_LAYER (lstItrLayer->data))->lstSelObjs ; lstItrSel != NULL ; lstItrSel = lstItrSel->next)
       if (NULL != lstItrSel->data)
         {
         project_options.bDesignAltered = bFoundSelection = TRUE ;
         qcad_cell_set_clock (QCAD_CELL (lstItrSel->data), CLOCK_INC (QCAD_CELL (lstItrSel->data)->cell_options.clock)) ;
         }

  if (bFoundSelection)
    {
    GdkRegion *rgn = NULL ;
    GdkRectangle rcReal = {0} ;
    WorldRectangle rcWorld ;

    if (design_get_extents (project_options.design, &rcWorld, TRUE))
      world_to_real_rect (&rcWorld, &rcReal) ;
    else
      gdk_window_get_size (main_window.drawing_area->window, &(rcReal.width), &(rcReal.height)) ;

    rgn = gdk_region_new () ;
    gdk_region_union_with_rect (rgn, &rcReal) ;

    // redraw_async takes care of destroying rgn
    redraw_async (rgn) ;
    }

}//on_clock_increment_menu_item_activate

void file_operations (GtkWidget *widget, gpointer user_data)
  {
  DESIGN *the_new_design = NULL ;
  int fFileOp = (int)user_data ;
#ifdef STDIO_FILEIO
  char *pszFName = NULL, *pszCurrent = (NULL == project_options.pszCurrentFName ? "" : project_options.pszCurrentFName) ;
  GdkCursor *cursor = NULL ;

  if (FILEOP_SAVE == fFileOp || FILEOP_SAVE_AS == fFileOp)
    {
    DoSave (GTK_WINDOW (main_window.main_window), fFileOp) ;
    return ;
    }

  if (FILEOP_OPEN == fFileOp || FILEOP_OPEN_RECENT == fFileOp || FILEOP_NEW == fFileOp || FILEOP_CLOSE == fFileOp)
    if (!(SaveDirtyUI (GTK_WINDOW (main_window.main_window),
      FILEOP_OPEN_RECENT == fFileOp ||
             FILEOP_OPEN == fFileOp ? _("You have made changes to your design.  Opening another design will discard those changes. Save first ?") :
              FILEOP_NEW == fFileOp ? _("You have made changes to your design.  Starting a new design will discard those changes.  Save first ?") :
                                      _("You have made changes to your design.  Closing your design will discard those changes.  Save first ?"))))
      return ;
#endif /* def STDIO_FILEIO */
  if (FILEOP_NEW == fFileOp || FILEOP_CLOSE == fFileOp)
    {
    QCADSubstrate *subs = NULL ;

    tabula_rasa (GTK_WINDOW (main_window.main_window)) ;

    the_new_design = design_new (&subs) ;
    set_current_design (the_new_design, subs) ;
    return ;
    }
#ifdef STDIO_FILEIO
  if (FILEOP_OPEN_RECENT == fFileOp)
    pszFName = g_strdup_printf ("%s", (char *)gtk_object_get_data (GTK_OBJECT (widget), "file")) ;
  else
  if (FILEOP_CMDLINE == fFileOp)
    {
    fill_recent_files_menu (main_window.recent_files_menu, GTK_SIGNAL_FUNC (file_operations), (gpointer)FILEOP_OPEN_RECENT) ;
    pszFName = absolute_path ((char *)widget) ;
    }
  else
  if (FILEOP_AUTOLOAD == fFileOp)
    pszFName = CreateUserFName ("autosave.qca") ;
  else
    {
    if (NULL == (pszFName = get_file_name_from_user (GTK_WINDOW (main_window.main_window),
        FILEOP_OPEN == fFileOp ? _("Open Design") :
      FILEOP_IMPORT == fFileOp ? _("Import Block") :
      FILEOP_EXPORT == fFileOp ? _("Export Block") : "????????", pszCurrent, FALSE)))
      return ;
    }

  if (!(g_file_test (pszFName, G_FILE_TEST_IS_REGULAR) || g_file_test (pszFName, G_FILE_TEST_IS_SYMLINK)) &&
        g_file_test (pszFName, G_FILE_TEST_EXISTS))
    {
    if (fFileOp != FILEOP_AUTOLOAD)
      {
      GtkWidget *msg = NULL ;

      gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (main_window.main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
        _("The file \"%s\" is not a regular file.  Maybe it's a directory, or a device file.  Please choose a regular file."), pszFName))) ;

      gtk_widget_hide (msg) ;
      gtk_widget_destroy (msg) ;

      remove_recent_file (main_window.recent_files_menu, pszFName, GTK_SIGNAL_FUNC (file_operations), (gpointer)FILEOP_OPEN_RECENT) ;
      }
    else
      // Get rid of a bad autosave file
#ifdef WIN32
    DeleteFile (pszFName) ;
#else
    unlink (pszFName) ;
#endif /* def WIN32 */
    g_free (pszFName) ;
    return ;
    }
  else
  if (!g_file_test (pszFName, G_FILE_TEST_EXISTS) && FILEOP_EXPORT != fFileOp)
    {
    if (fFileOp != FILEOP_AUTOLOAD)
      {
      GtkWidget *msg = NULL ;

      gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (main_window.main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
        _("Could not locate file \"%s\"."), pszFName))) ;

      gtk_widget_hide (msg) ;
      gtk_widget_destroy (msg) ;

      remove_recent_file (main_window.recent_files_menu, pszFName, GTK_SIGNAL_FUNC (file_operations), (gpointer)FILEOP_OPEN_RECENT) ;
      }
    g_free (pszFName) ;
    return ;
    }

  if (FILEOP_OPEN == fFileOp || FILEOP_OPEN_RECENT == fFileOp || FILEOP_CMDLINE == fFileOp || FILEOP_AUTOLOAD == fFileOp)
    {
    char *pszAutoFName = NULL ;
    gboolean bLoadTheFile = TRUE ;

    if (FILEOP_AUTOLOAD == fFileOp)
      {
      GtkWidget *msg = NULL ;

      bLoadTheFile = (GTK_RESPONSE_YES == (gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (main_window.main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
        _("You appear to have an autosave file.  Would you like to recover it ?")))))) ;
      gtk_widget_hide (msg) ;
      gtk_widget_destroy (msg) ;
      }

    if (bLoadTheFile)
      {
      if (FILEOP_AUTOLOAD != fFileOp)
        {
        // check if there's a more recent autosave file, and change fFileOp to FILEOP_AUTOLOAD if so.
        pszAutoFName = g_strdup_printf ("%s~", pszFName) ;
        if (g_file_test (pszAutoFName, G_FILE_TEST_EXISTS) &&
            g_file_test (pszAutoFName, G_FILE_TEST_IS_REGULAR))
          {
          if (file_age_compare (pszFName, pszAutoFName) > 0)
            {
            GtkWidget *msg = NULL ;

            if (GTK_RESPONSE_YES == (gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (main_window.main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
              _("An autosave file more recent than the file you are opening seems to be present.  Load it instead ?"))))))
              fFileOp = FILEOP_AUTOLOAD ;
            else
              {
#ifdef WIN32
              DeleteFile (pszAutoFName) ;
#else
              unlink (pszAutoFName) ;
#endif /* def WIN32 */
              g_free (pszAutoFName) ;
              pszAutoFName = NULL ;
              }
            gtk_widget_hide (msg) ;
            gtk_widget_destroy (msg) ;
            }
          }
        else
          {
          g_free (pszAutoFName) ;
          pszAutoFName = NULL ;
          }
        }
      push_cursor (main_window.main_window, gdk_cursor_new (GDK_WATCH)) ;
      gtk_widget_set_sensitive (main_window.vbox1, FALSE) ;
      if (open_project_file ((FILEOP_AUTOLOAD == fFileOp && NULL != pszAutoFName) ? pszAutoFName : pszFName, &the_new_design))
        {
        char *pszTitle = NULL ;

        tabula_rasa (GTK_WINDOW (main_window.main_window)) ;

        if (FILEOP_AUTOLOAD == fFileOp)
          project_options.bDesignAltered = TRUE ;

        set_current_design (the_new_design, find_snap_source (the_new_design)) ;

        if (!(FILEOP_AUTOLOAD == fFileOp && NULL == pszAutoFName))
          {
          add_to_recent_files (main_window.recent_files_menu, pszFName, GTK_SIGNAL_FUNC (file_operations), (gpointer)FILEOP_OPEN_RECENT) ;
          gtk_window_set_title (GTK_WINDOW (main_window.main_window),
            pszTitle = g_strdup_printf ("%s - %s", base_name (pszFName), MAIN_WND_BASE_TITLE)) ;
          g_free (pszTitle) ;
          if (NULL != project_options.pszCurrentFName)
            g_free (project_options.pszCurrentFName) ;
          project_options.pszCurrentFName = pszFName ;
          }
        }
      else
        {
        GtkWidget *msg = NULL ;
        if (FILEOP_AUTOLOAD == fFileOp)
          gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (main_window.main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
            _("Failed to open autosave file!")))) ;
        else
          gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (main_window.main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
            _("Failed to open file \"%s\"!"), pszFName))) ;
        gtk_widget_hide (msg) ;
        gtk_widget_destroy (msg) ;
        if (FILEOP_AUTOLOAD != fFileOp)
          remove_recent_file (main_window.recent_files_menu, pszFName, GTK_SIGNAL_FUNC (file_operations), (gpointer)FILEOP_OPEN_RECENT) ;
        g_free (pszFName) ;
        }
      gtk_widget_set_sensitive (main_window.vbox1, TRUE) ;
      if (NULL != (cursor = pop_cursor (main_window.main_window)))
        gdk_cursor_unref (cursor) ;
      }
    else // !bLoadTheFile
    if (NULL != pszFName)
      // We have an autosave file, but the user doesn't want it, so we get rid of it
#ifdef WIN32
      DeleteFile (pszFName) ;
#else
      unlink (pszFName) ;
#endif /* def WIN32 */
    return ;
    }
  else
  if (FILEOP_EXPORT == fFileOp)
    {
    export_block (pszFName, project_options.design) ;
    g_free (pszFName) ;
    return ;
    }
  else
  if (FILEOP_IMPORT == fFileOp)
    {
    DESIGN *sel = NULL;
    push_cursor (main_window.main_window, gdk_cursor_new (GDK_WATCH)) ;
    gtk_widget_set_sensitive (main_window.vbox1, FALSE) ;
    if (open_project_file (pszFName, &sel))
      {
      QCADDesignObject *obj = NULL ;
      EXP_ARRAY *layer_mappings = NULL ;

      if (NULL == (layer_mappings = get_layer_mapping_from_user(main_window.main_window, project_options.design, sel)))
	      sel = design_destroy (sel);
      else
      	{
        design_selection_release (project_options.design, main_window.drawing_area->window, GDK_COPY) ;
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (main_window.default_action_button), TRUE) ;

        if (NULL != (obj = merge_selection (project_options.design, sel, layer_mappings)))
          move_selection_to_pointer (obj) ;
        sel = design_destroy (sel) ;
        exp_array_free (layer_mappings) ;
        }
      }
    else
      {
      GtkWidget *msg = NULL ;
      gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (main_window.main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
        "Failed to import block from file \"%s\"!", pszFName))) ;
      gtk_widget_hide (msg) ;
      gtk_widget_destroy (msg) ;
      }
    gtk_widget_set_sensitive (main_window.vbox1, TRUE) ;
    if (NULL != (cursor = pop_cursor (main_window.main_window)))
      gdk_cursor_unref (cursor) ;
    g_free (pszFName) ;
    return ;
    }
#endif /* def STDIO_FILEIO */
  }

#ifdef STDIO_FILEIO
void on_load_output_from_file_menu_item_activate(GtkMenuItem *menuitem, gpointer user_data)
  {
  static char *pszFName = NULL ;
  char *pszTempFName = NULL ;
  SIMULATION_OUTPUT *sim_output = NULL ;
  DBG_CB_HERE (fprintf (stderr, "Entering on_load_output_from_file_menu_item_activate\n")) ;

  if (NULL == (pszTempFName = get_file_name_from_user (GTK_WINDOW (main_window.main_window), _("Open Simulation Results"), pszFName, FALSE)))
    return ;

  if (NULL != pszFName) g_free (pszFName) ;

  pszFName = pszTempFName ;

  if (NULL != (sim_output = open_simulation_output_file (pszFName)))
    {
    // Give the contents of sim_output to graph_dialog, and allow it to destroy said contents
    show_graph_dialog (GTK_WINDOW (main_window.main_window), sim_output, TRUE, FALSE) ;
    // Since graph_dialog will destroy the contents of sim_output, we can now free the wrapper shallowly
    g_free (sim_output) ;
    }
  }

void on_save_output_to_file_menu_item_activate(GtkMenuItem *menuitem, gpointer user_data)
  {
  static char *pszFName = NULL ;
  char *pszTempFName = NULL ;
  SIMULATION_OUTPUT sim_output = {NULL} ;

  sim_output.sim_data = project_options.sim_data ;
  sim_output.bus_layout = project_options.design->bus_layout ;
  sim_output.bFakeIOLists = FALSE ;
  DBG_CB_HERE (fprintf (stderr, "Entering on_save_output_to_file_menu_item_activate\n")) ;

  if (NULL == project_options.sim_data) { gdk_beep () ; return ; }

  if (NULL == (pszTempFName = get_file_name_from_user (GTK_WINDOW (main_window.main_window), _("Save Simulation Results"), pszFName, TRUE)))
    return ;

  if (NULL != pszFName) g_free (pszFName) ;

  pszFName = pszTempFName ;

  create_simulation_output_file (pszFName, &sim_output) ;
  }
#endif /* def STDIO_FILEIO */

void on_simulation_type_setup_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_simulation_type_setup_menu_item_activate\n")) ;
  get_vector_table_options_from_user (GTK_WINDOW (main_window.main_window), project_options.design->bus_layout, &(project_options.SIMULATION_TYPE), pvt) ;
}  //on_simulation_properties_menu_item_activate

void on_simulation_engine_setup_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_simulation_engine_setup_menu_item_activate\n")) ;
get_sim_engine_from_user (GTK_WINDOW (main_window.main_window), &(project_options.SIMULATION_ENGINE)) ;
}  //on_simulation_engine_setup_menu_item_activate

void on_reset_zoom_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data)
  {
  reset_zoom () ;
  reflect_zoom () ;
  }

void on_zoom_in_menu_item_activate(GtkMenuItem *menuitem, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering on_zoom_in_menu_item_activate\n")) ;
  zoom_in();
  reflect_zoom () ;
  }

void on_zoom_out_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_zoom_out_menu_item_activate\n")) ;
    zoom_out();
    reflect_zoom () ;
}

void on_zoom_extents_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  WorldRectangle rcWorld ;
  GdkRectangle rcReal ;
  if (design_get_extents (project_options.design, &rcWorld, FALSE))
    {
    world_to_real_rect (&rcWorld, &rcReal) ;
    zoom_window (rcReal.x, rcReal.y, rcReal.x + rcReal.width, rcReal.y + rcReal.height) ;
    reflect_zoom () ;
    }
  else
    command_history_message (_("Cannot zoom to the design extents, because the design is empty")) ;
  }

void on_zoom_layer_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  WorldRectangle rcWorld ;
  GdkRectangle rcReal ;
  if (qcad_layer_get_extents (QCAD_LAYER (project_options.design->lstCurrentLayer->data), &rcWorld, FALSE))
    {
    world_to_real_rect (&rcWorld, &rcReal) ;
    zoom_window (rcReal.x, rcReal.y, rcReal.x + rcReal.width, rcReal.y + rcReal.height) ;
    reflect_zoom () ;
    }
  else
    command_history_message (_("Cannot zoom to the layer extents, because the design is empty")) ;
  }

void on_mirror_button_clicked (GtkButton *button, gpointer user_data)
  {
  static GdkPoint pt = {0, 0} ;
  static GtkWidget *mnu = NULL ;

  if (NULL == mnu)
    {
    GtkWidget *img = NULL, *mnui = NULL ;
    // Create the mirror popup menu
    mnu = gtk_menu_new () ;

    mnui = gtk_image_menu_item_new_with_label (_("Vertically")) ;
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (mnui),
      img = gtk_image_new_from_stock (QCAD_STOCK_MIRROR_VERTICAL, QCAD_ICON_SIZE_SIDE_TOOLBAR)) ;
    gtk_widget_show (img) ;
    gtk_widget_show (mnui) ;
    gtk_container_add (GTK_CONTAINER (mnu), mnui) ;
    g_signal_connect (G_OBJECT (mnui), "activate", (GCallback)mirror_selection_direction_chosen, (gpointer)TRUE) ;

    mnui = gtk_menu_item_new () ;
    gtk_widget_show (mnui) ;
    gtk_container_add (GTK_CONTAINER (mnu), mnui) ;

    mnui = gtk_image_menu_item_new_with_label (_("Horizontally")) ;
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (mnui),
      img = gtk_image_new_from_stock (QCAD_STOCK_MIRROR_HORIZONTAL, QCAD_ICON_SIZE_SIDE_TOOLBAR)) ;
    gtk_widget_show (img) ;
    gtk_widget_show (mnui) ;
    gtk_container_add (GTK_CONTAINER (mnu), mnui) ;
    g_signal_connect (G_OBJECT (mnui), "activate", (GCallback)mirror_selection_direction_chosen, (gpointer)FALSE) ;
    }

  gtk_widget_get_root_origin (GTK_WIDGET (button), &(pt.x), &(pt.y)) ;

  gtk_menu_popup (GTK_MENU (mnu), NULL, NULL, (GtkMenuPositionFunc)place_popup_menu, &pt, 1, gtk_get_current_event_time ()) ;
  }

static void mirror_selection_direction_chosen (GtkWidget *widget, gpointer data)
  {
  WorldRectangle rcWorld ;
  double xWorld, yWorld ;
  QCADDesignObject *obj = NULL ;
  double mtx[2][2] = {{1, 0}, {0, 1}} ;

  if ((gboolean)data)
    {
    mtx[0][0] = -1 ; mtx[0][1] =  0 ;
    mtx[1][0] =  0 ; mtx[1][1] =  1 ;
    }
  else
    {
    mtx[0][0] =  1 ; mtx[0][1] =  0 ;
    mtx[1][0] =  0 ; mtx[1][1] = -1 ;
    }

  design_get_extents (project_options.design, &rcWorld, TRUE) ;

  xWorld = rcWorld.xWorld + rcWorld.cxWorld / 2.0 ;
  yWorld = rcWorld.yWorld + rcWorld.cyWorld / 2.0 ;

  selection_renderer_draw (project_options.srSelection, project_options.design, main_window.drawing_area->window, GDK_XOR) ;

  if (NULL != (obj = design_selection_transform (project_options.design, mtx[0][0], mtx[0][1], mtx[1][0], mtx[1][1])))
    {
#ifdef UNDO_REDO
    push_undo_selection_transform (project_options.design, project_options.srSelection, main_window.drawing_area->window, mtx, mtx) ;
#endif /* def UNDO_REDO */
    design_get_extents (project_options.design, &rcWorld, TRUE) ;
    design_selection_move (project_options.design,
      xWorld - (rcWorld.xWorld + rcWorld.cxWorld / 2.0),
      yWorld - (rcWorld.yWorld + rcWorld.cyWorld / 2.0)) ;
    move_selection_to_pointer (obj) ;
    }
  }

static void place_popup_menu (GtkMenu *menu, int *x, int *y, gboolean *push_in, gpointer data)
  {
  (*x) = ((GdkPoint *)data)->x ;
  (*y) = ((GdkPoint *)data)->y ;
  (*push_in) = TRUE ;
  }

void on_copy_cell_button_clicked (GtkButton *button, gpointer user_data)
  {
  EXP_ARRAY *arSelObjs = NULL ;
  EXP_ARRAY *arNewObjs = NULL ;
  QCADDesignObject *obj = NULL ;

  if (!design_selection_drop (project_options.design))
    {
    gdk_beep () ;
    command_history_message (_("The selection must be able to drop before it can be copied.\n")) ;
    return ;
    }

  selection_renderer_draw (project_options.srSelection, project_options.design, main_window.drawing_area->window, GDK_XOR) ;

  arSelObjs = design_selection_get_object_array (project_options.design) ;

  if (NULL != (obj = design_selection_create_from_selection (project_options.design, main_window.drawing_area->window, GDK_COPY)))
    {
    WorldRectangle rcWorld ;
    GdkRectangle rcReal ;

    arNewObjs = design_selection_get_object_array (project_options.design) ;
#ifdef UNDO_REDO
    if (!(NULL == arNewObjs || NULL == arSelObjs))
      {
      qcad_undo_entry_group_push_group (qcad_undo_entry_group_get_default (), qcad_undo_entry_group_new ()) ;
      push_undo_selection_altered (project_options.design, project_options.srSelection, main_window.drawing_area->window, arSelObjs, FALSE) ;
      push_undo_selection_existence (project_options.design, project_options.srSelection, main_window.drawing_area->window, arNewObjs, TRUE) ;
      push_undo_selection_altered (project_options.design, project_options.srSelection, main_window.drawing_area->window,
        design_selection_object_array_add_weak_pointers (exp_array_copy (arNewObjs)), TRUE) ;
      qcad_undo_entry_group_close (qcad_undo_entry_group_get_default ()) ;
      }
#endif /* def UNDO_REDO */

    design_get_extents (project_options.design, &rcWorld, TRUE) ;
    world_to_real_rect (&rcWorld, &rcReal) ;
    design_draw (project_options.design, main_window.drawing_area->window, GDK_COPY, &rcWorld, LAYER_DRAW_NON_SELECTION) ;
    move_selection_to_pointer (obj) ;
    // For now - we'll have to have a VectorTable function that merges new inputs into the table.
    VectorTable_fill (project_options.pvt, project_options.design) ;
    }

  if (NULL == arSelObjs || NULL == arNewObjs)
    {
    if (NULL != arSelObjs)
      design_selection_object_array_free (arSelObjs) ;
    else
    if (NULL != arNewObjs)
      design_selection_object_array_free (arNewObjs) ;
    }
  }

void on_print_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering on_print_menu_item_activate\n")) ;
  if (get_print_design_properties_from_user (GTK_WINDOW (main_window.main_window), &print_options, project_options.design))
    print_world (&print_options, project_options.design) ;
  }

// quit QCADesigner selected from menu
gboolean on_quit_menu_item_activate(GtkWidget *main_window, gpointer user_data)
  {
#ifdef STDIO_FILEIO
  if (!SaveDirtyUI (GTK_WINDOW (main_window),
    _("You have made changes to your design.  Quitting QCADesigner will discard those changes.  Save first ?")))
      return TRUE ;
#endif /* def STDIO_FILEIO */
  project_options.design = design_destroy (project_options.design) ;
#ifdef UNDO_REDO
  g_object_unref (qcad_undo_entry_group_get_default ()) ;
#endif /* def UNDO_REDO */
  gtk_main_quit () ;
  return FALSE ;
  }

#ifdef UNDO_REDO
void on_undo_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering on_undo_menu_item_activate\n")) ;
  qcad_undo_entry_group_undo (qcad_undo_entry_group_get_default ()) ;
  redraw_async (NULL) ;
  }

void on_redo_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_redo_menu_item_activate\n")) ;
  qcad_undo_entry_group_redo (qcad_undo_entry_group_get_default ()) ;
  redraw_async (NULL) ;
}
#endif /* def UNDO_REDO */

void on_delete_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  GdkRegion *rgn = NULL ;
  gboolean bHaveExtents = FALSE ;
  GdkRectangle rcReal = {0} ;
  WorldRectangle rcWorld = {0.0} ;
  EXP_ARRAY *arDeletedObjs = NULL ;

  DBG_CB_HERE (fprintf (stderr, "Entering on_delete_menu_item_activate\n")) ;
  selection_renderer_draw (project_options.srSelection, project_options.design, main_window.drawing_area->window, GDK_XOR) ;
  bHaveExtents = design_get_extents (project_options.design, &rcWorld, TRUE) ;
  arDeletedObjs = design_selection_destroy (project_options.design) ;
  selection_renderer_update (project_options.srSelection, project_options.design) ;
  cell_function_changed (NULL, NULL) ;

#ifdef UNDO_REDO
  // TEMPORARY
  if ((project_options.bDesignAltered = (NULL != arDeletedObjs)))
    push_undo_selection_existence (project_options.design, project_options.srSelection, main_window.drawing_area->window, arDeletedObjs, FALSE) ;
#endif /* def UNDO_REDO */

  selection_renderer_update (project_options.srSelection, project_options.design) ;

  if (bHaveExtents)
    world_to_real_rect (&rcWorld, &rcReal) ;
  else
    gdk_window_get_size (main_window.drawing_area->window, &(rcReal.width), &(rcReal.height)) ;

  rgn = gdk_region_new () ;
  gdk_region_union_with_rect (rgn, &rcReal) ;

  // redraw_async takes care of detroying rgn
  redraw_async (rgn) ;
  }

void on_start_simulation_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  gboolean bSimOK = TRUE ;
  DBG_CB_HERE (fprintf (stderr, "Entering on_start_simulation_menu_item_activate\n")) ;

  if (VECTOR_TABLE == project_options.SIMULATION_TYPE)
    {
    if (NULL == pvt) bSimOK = FALSE ;
    else
    if (NULL == pvt->vectors) bSimOK = FALSE ;
    else
    if (0 == pvt->vectors->icUsed) bSimOK = FALSE ;
    }

  if (!bSimOK)
    {
    GtkWidget *msg = NULL ;

    gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (main_window.main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, _("Cannot simulate with an empty vector table !")))) ;

    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;
    return ;
    }

  project_options.sim_data = run_simulation (project_options.SIMULATION_ENGINE, project_options.SIMULATION_TYPE, project_options.design, pvt);

  if (NULL != project_options.sim_data)
    {
    SIMULATION_OUTPUT sim_output = {project_options.sim_data, project_options.design->bus_layout, FALSE} ;
    gtk_widget_set_sensitive (GTK_WIDGET (project_options.main_window->show_simulation_results_menu_item), TRUE);
#ifdef STDIO_FILEIO
    gtk_widget_set_sensitive (GTK_WIDGET (project_options.main_window->save_output_to_file_menu_item), TRUE);
#endif /* def STDIO_FILEIO */
    show_graph_dialog (GTK_WINDOW (main_window.main_window), &sim_output, FALSE, FALSE) ;
    }
  }

void on_stop_simulation_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering on_stop_simulation_menu_item_activate\n")) ;
  STOP_SIMULATION = TRUE;
  }

void on_show_simulation_results_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  SIMULATION_OUTPUT sim_output = {project_options.sim_data, project_options.design->bus_layout, FALSE} ;
  DBG_CB_HERE (fprintf (stderr, "Entering on_reset_simulation_menu_item_activate\n")) ;
  show_graph_dialog (GTK_WINDOW (main_window.main_window), &sim_output, FALSE, FALSE) ;
  }

void on_contents_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  char *pszCmdLine = NULL ;
  #ifdef WIN32
  char *pszBrowser =
    get_external_app (GTK_WINDOW (main_window.main_window), _("Please Select Web Browser"), "browser",
      "C:\\Program Files\\Internet Explorer\\iexplore.exe", FALSE) ;
  #else
  char *pszBrowser =
    get_external_app (GTK_WINDOW (main_window.main_window), _("Please Select Web Browser"), "browser",
      "/usr/bin/mozilla", FALSE) ;
  #endif

  if (NULL == pszBrowser) return ;

  #ifdef WIN32
  pszCmdLine = g_strdup_printf ("%s file://%s%s..%sshare%sdoc%s%s-%s%smanual%sindex.html",
    pszBrowser, getenv ("MY_PATH"), G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S, 
      G_DIR_SEPARATOR_S, PACKAGE, VERSION, G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S) ;
  #else
  pszCmdLine = g_strdup_printf ("%s %s%cdoc%cQCADesigner-%s%cmanual%cindex.html", pszBrowser,
    PACKAGE_DATA_DIR, G_DIR_SEPARATOR, G_DIR_SEPARATOR, VERSION, G_DIR_SEPARATOR, G_DIR_SEPARATOR) ;
  #endif

  RunCmdLineAsync (pszCmdLine, NULL) ;

  g_free (pszCmdLine) ;
  }

void on_about_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering on_about_menu_item_activate\n")) ;
  show_about_dialog (&(main_window.main_window), FALSE) ;
  }

void rotate_selection_menu_item_activate (GtkWidget *widget, gpointer user_data)
  {
  WorldRectangle rcWorld = {0.0} ;
  double xWorld, yWorld ;
  QCADDesignObject *obj = NULL ;

  design_get_extents (project_options.design, &rcWorld, TRUE) ;
  xWorld = rcWorld.xWorld + rcWorld.cxWorld / 2.0 ;
  yWorld = rcWorld.yWorld + rcWorld.cyWorld / 2.0 ;

  selection_renderer_draw (project_options.srSelection, project_options.design, main_window.drawing_area->window, GDK_XOR) ;
  if (NULL != (obj = design_selection_transform (project_options.design, 0, -1, 1, 0)))
    {
#ifdef UNDO_REDO
    double mtxFore[2][2] = {{0, -1}, { 1, 0}} ;
    double mtxBack[2][2] = {{0,  1}, {-1, 0}} ;

    push_undo_selection_transform (project_options.design, project_options.srSelection, main_window.drawing_area->window, mtxFore, mtxBack) ;
#endif /* def UNDO_REDO */
    design_get_extents (project_options.design, &rcWorld, TRUE) ;
    design_selection_move (project_options.design,
      xWorld - (rcWorld.xWorld + rcWorld.cxWorld / 2.0),
      yWorld - (rcWorld.yWorld + rcWorld.cyWorld / 2.0)) ;
    move_selection_to_pointer (obj) ;
    }
  }

void on_clocks_combo_changed (GtkWidget *widget, gpointer data)
  {
  GList *lstItrLayer = NULL, *lstItrSel = NULL ;
  int idxClock = qcad_clock_combo_get_clock (QCAD_CLOCK_COMBO (data)) ;
  gboolean bFoundSelection = FALSE ;

  if (idxClock > -1 && idxClock < 4)
    {
    EXP_ARRAY *arSelObjs = NULL ;
    arSelObjs = design_selection_get_object_array (project_options.design) ;
#ifdef UNDO_REDO
    if (NULL != arSelObjs)
      push_undo_selection_clock (project_options.design, project_options.srSelection, main_window.drawing_area->window, arSelObjs, idxClock, FALSE) ;
#endif /* def UNDO_REDO */
    for (lstItrLayer = project_options.design->lstLayers ; lstItrLayer != NULL ; lstItrLayer = lstItrLayer->next)
      if (LAYER_TYPE_CELLS == (QCAD_LAYER (lstItrLayer->data))->type)
        for (lstItrSel = (QCAD_LAYER (lstItrLayer->data))->lstSelObjs ; lstItrSel != NULL ; lstItrSel = lstItrSel->next)
          if (NULL != lstItrSel->data)
            {
            project_options.bDesignAltered =
            bFoundSelection = TRUE ;
            qcad_cell_set_clock (QCAD_CELL (lstItrSel->data), idxClock) ;
            }

    // Set the default clock in the default_cell_properties of the QCADCellClass, if the current layer is a cell layer
    if (LAYER_TYPE_CELLS == (QCAD_LAYER (project_options.design->lstCurrentLayer->data))->type)
      QCAD_CELL_CLASS (g_type_class_peek (QCAD_TYPE_CELL))->default_cell_options.clock = idxClock ;
    }
  }


static void layer_select (GtkWidget *widget, gpointer data)
  {selected_layer_item = widget ;}

void layer_selected (GtkWidget *widget, gpointer data)
  {
  QCADLayer *layer = NULL ;

  if (NULL == selected_layer_item) return ;

  layer = QCAD_LAYER (g_object_get_data (G_OBJECT (selected_layer_item), "layer")) ;

  design_set_current_layer (project_options.design, layer) ;

  if (NULL == layer) return ;

  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (main_window.layers_combo)->entry),
    NULL == layer->pszDescription ? "" : layer->pszDescription) ;

  reflect_layer_status (layer) ;

  // Apply default properties for all object classes in this layer
  if (NULL != layer->default_properties)
    g_hash_table_foreach (layer->default_properties, layer_apply_default_properties, NULL) ;

  gtk_widget_queue_draw (main_window.toolbar) ;
  }

static void layer_status_change (GtkWidget *widget, gpointer data)
  {
  GtkWidget
    *chkActivate = GTK_WIDGET (g_object_get_data (G_OBJECT (data), "chkActive")),
    *chkVisible  = GTK_WIDGET (g_object_get_data (G_OBJECT (data), "chkVisible")) ;
  QCADLayer *layer = QCAD_LAYER (g_object_get_data (G_OBJECT (data), "layer")) ;
  gboolean bActive = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ;

  if (NULL == layer) return ;

  // Attempting to deactivate either checkbox means that we are
  // required to drop those selected objects that lie on this layer. 
  // If we fail to drop these objects, we must let the user know that
  // she cannot deactivate the layer.
  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) && NULL != layer->lstSelObjs)
    {
    if (!qcad_layer_selection_drop (layer))
      {
      GtkWidget *msg = NULL ;

      gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (main_window.main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
        _("Cannot %s the layer at this time.\n\nPlease ensure that your selection can be dropped before attempting to %s a layer."),
          chkActivate == widget ? _("deactivate") : _("hide"), chkActivate == widget ? _("deactivate") : _("hide")))) ;

      gtk_widget_hide (msg) ;
      gtk_widget_destroy (msg) ;

      // "quietly" re-activate the toggle button
      g_signal_handlers_block_matched (G_OBJECT (widget), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)layer_status_change, data) ;
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), TRUE) ;
      g_signal_handlers_unblock_matched (G_OBJECT (widget), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)layer_status_change, data) ;

      return ;
      }
    else
      {
      EXP_ARRAY *arSelObjs = NULL ;

      if (NULL != (arSelObjs = qcad_layer_selection_release (layer, main_window.drawing_area->window, GDK_COPY, NULL)))
        {
        arSelObjs = design_selection_object_array_add_weak_pointers (arSelObjs) ;
        selection_renderer_update (project_options.srSelection, project_options.design) ;
        ACTION_SELECT_sel_changed (NULL, project_options.design) ;
#ifdef UNDO_REDO
        push_undo_selection_altered (project_options.design, project_options.srSelection, main_window.drawing_area->window, arSelObjs, FALSE) ;
#endif /* def UNDO_REDO */
        }
      }
    }

  if (chkVisible == widget)
    {
    if (bActive)
      {
      layer->status = LAYER_STATUS_VISIBLE ;
      gtk_widget_set_sensitive (chkActivate, TRUE) ;
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkActivate),
        (gboolean)g_object_get_data (G_OBJECT (widget), "bWasActive")) ;
      }
    else
      {
      layer->status = LAYER_STATUS_HIDDEN ;
      g_object_set_data (G_OBJECT (widget), "bWasActive",
        (gpointer)gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (chkActivate))) ;
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkActivate), FALSE) ;
      gtk_widget_set_sensitive (chkActivate, FALSE) ;
      }
    }
  else
  if (chkActivate == widget)
    layer->status = bActive ? LAYER_STATUS_ACTIVE :
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (chkVisible)) ?
      	LAYER_STATUS_VISIBLE : LAYER_STATUS_HIDDEN ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (main_window.default_action_button), TRUE) ;

  reflect_layer_status (layer) ;

  DBG_LAYER (fprintf (stderr, "layer_status_change:Exiting\n")) ;
  }

void action_button_clicked (GtkWidget *widget, gpointer data)
  {
  static int action = ACTION_LAST_ACTION ;
  int idx = (int)data ;
  GdkCursor *cursor = NULL ;

  if (NULL != widget)
    if (GTK_IS_TOGGLE_BUTTON (widget))
      if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
        return ;

  // Transitions
  if (ACTION_PAN == action && ACTION_PAN != idx)
    if (NULL != (cursor = pop_cursor (main_window.drawing_area)))
      gdk_cursor_unref (cursor) ;

  // Preamble for each action
  if (ACTION_ROTATE == idx)
    {
    selection_renderer_draw (project_options.srSelection, project_options.design, main_window.drawing_area->window, GDK_XOR) ;
    design_selection_objects_foreach (project_options.design, rotate_single_cell_cb, NULL) ;
    selection_renderer_update (project_options.srSelection, project_options.design) ;
    selection_renderer_draw (project_options.srSelection, project_options.design, main_window.drawing_area->window, GDK_XOR) ;
    }
  else
  if (ACTION_PAN == idx)
    push_cursor (main_window.drawing_area, gdk_cursor_new (GDK_FLEUR)) ;

  set_mouse_handlers (actions[idx].type, &(actions[idx].mh), actions[idx].data, main_window.drawing_area, actions[idx].drop_function) ;

  action = idx ;
  }

static void rotate_single_cell_cb (DESIGN *design, QCADDesignObject *obj, gpointer data)
  {
  if (QCAD_IS_CELL (obj))
    qcad_cell_rotate_dots (QCAD_CELL (obj), PI / 4.0) ;
  }

///////////////////////////////////////////////////////////////////
///////////////////////// HELPERS /////////////////////////////////
///////////////////////////////////////////////////////////////////

void setup_rulers (int x, int y)
  {
  double world_x1, world_y1, world_x2, world_y2, world_x, world_y ;
  int xOffset = 0, yOffset = 0,
    xFrame = main_window.drawing_area_frame->allocation.x,
    yFrame = main_window.drawing_area_frame->allocation.y,
    cxFrame = main_window.drawing_area_frame->allocation.width,
    cyFrame = main_window.drawing_area_frame->allocation.height,
    xDA = main_window.drawing_area->allocation.x,
    yDA = main_window.drawing_area->allocation.y ;

  world_x1 = real_to_world_x (xOffset = xFrame - xDA) ;
  world_y1 = real_to_world_y (yOffset = yFrame - yDA) ;
  world_x = real_to_world_x (x - xOffset) ;
  world_y = real_to_world_y (y - yOffset) ;
  world_x2 = real_to_world_x (xOffset + cxFrame + 1) ;
  world_y2 = real_to_world_y (yOffset + cyFrame + 1) ;

  world_x = CLAMP (world_x, world_x1, world_x2) ;
  world_y = CLAMP (world_y, world_y1, world_y2) ;

  set_ruler_scale (GTK_RULER (main_window.horizontal_ruler), world_x1, world_x2) ;
  set_ruler_scale (GTK_RULER (main_window.horizontal_ruler), world_y1, world_y2) ;

  gtk_ruler_set_range (GTK_RULER (main_window.horizontal_ruler), world_x1, world_x2, world_x, world_x2) ;
  gtk_ruler_set_range (GTK_RULER (main_window.vertical_ruler), world_y1, world_y2, world_y, world_y2) ;

  gdk_flush () ;
  }

static void propagate_motion_to_rulers (GtkWidget *widget, GdkEventMotion *event)
  {
  double lower, upper, position, max_size,
    xWorld = real_to_world_x (event->x),
    yWorld = real_to_world_y (event->y) ;

  gtk_ruler_get_range (GTK_RULER (main_window.horizontal_ruler), &lower, &upper, &position, &max_size) ;
  gtk_ruler_set_range (GTK_RULER (main_window.horizontal_ruler), lower, upper, xWorld, max_size) ;

  gtk_ruler_get_range (GTK_RULER (main_window.vertical_ruler), &lower, &upper, &position, &max_size) ;
  gtk_ruler_set_range (GTK_RULER (main_window.vertical_ruler), lower, upper, yWorld, max_size) ;
  }

void redraw_async (GdkRegion *rgn)
  {
  static REDRAW_ASYNC_PARAMS redraw_async_params = {FALSE, NULL} ;

  if (!(redraw_async_params.bHaveIdler))
    {
    redraw_async_params.bHaveIdler = TRUE ;
    redraw_async_params.rgn = (NULL == rgn ? NULL : gdk_region_copy (rgn)) ;
    g_idle_add ((GSourceFunc)redraw_async_cb, &(redraw_async_params)) ;
    }

  if (NULL != rgn)
    gdk_region_destroy (rgn) ;
  }

void on_translate_selection_button_clicked (GtkWidget *widget, gpointer user_data)
  {
  double dx = 0, dy = 0 ;
  QCADDesignObject *anchor = NULL ;

  if (NULL != (anchor = design_selection_get_anchor (project_options.design)))
    if (get_translation_from_user (GTK_WINDOW (main_window.main_window), &dx, &dy))
      if (!(0.0 == dx && 0.0 == dy))
        {
        EXP_ARRAY *objs = NULL ;

        objs = design_selection_get_object_array (project_options.design) ;
#ifdef UNDO_REDO
        if (NULL != objs)
          push_undo_selection_move (project_options.design, project_options.srSelection, main_window.drawing_area->window, dx, dy) ;
#endif /* def UNDO_REDO */
        selection_renderer_draw (project_options.srSelection, project_options.design, main_window.drawing_area->window, GDK_XOR) ;
        design_selection_move (project_options.design, dx, dy) ;
        move_selection_to_pointer (anchor) ;
        }
  }

///////////////////////////////////////////////////////////////////
///////////////////// STATIC HELPERS //////////////////////////////
///////////////////////////////////////////////////////////////////

static gboolean drop_single_object_with_undo_cb (QCADDesignObject *obj)
  {
  gboolean bRet = FALSE ;

  if ((bRet = drop_single_object_cb (obj)))
    {
#ifdef UNDO_REDO
    EXP_ARRAY *ar = exp_array_new (sizeof (QCADDesignObject *), 1) ;

    exp_array_insert_vals (ar, &obj, 1, 1, 0) ;

    ar = design_selection_object_array_add_weak_pointers (ar) ;

    push_undo_selection_existence (project_options.design, project_options.srSelection, main_window.drawing_area->window, ar, TRUE) ;
#endif /* def UNDO_REDO */
    }
  return bRet ;
  }

static gboolean drop_single_object_cb (QCADDesignObject *obj)
  {
  gboolean bRet = FALSE ;

  if (NULL == obj) return FALSE ;

  if (NULL == project_options.design->lstCurrentLayer->data) return FALSE ;

  if ((bRet = qcad_do_container_add (QCAD_DO_CONTAINER (project_options.design->lstCurrentLayer->data), obj)))
    g_object_unref (G_OBJECT (obj)) ;

  if (bRet)
    {
    qcad_design_object_draw (obj, main_window.drawing_area->window, GDK_COPY) ;
    project_options.bDesignAltered = TRUE ;
    }
  else
    {
    gdk_beep () ;
    redraw_async (NULL) ;
    }

  return bRet ;
  }

static gboolean redraw_async_cb (REDRAW_ASYNC_PARAMS *parms)
  {
  redraw_sync (parms->rgn, TRUE) ;

  return (parms->bHaveIdler = FALSE) ;
  }

void redraw_sync (GdkRegion *rgn, gboolean bDestroyRegion)
  {
  WorldRectangle rcWorld ;
  GdkRectangle *rcRgn = NULL ;
  int icRects = 0 ;
  int Nix ;

  if (NULL == rgn)
    {
    GdkRectangle rc = {0} ;

    gdk_window_get_size (main_window.drawing_area->window, &(rc.width), &(rc.height)) ;
    rgn = gdk_region_new () ;
    gdk_region_union_with_rect (rgn, &rc) ;
    bDestroyRegion = TRUE ;
    }

  gdk_region_get_rectangles (rgn, &rcRgn, &icRects) ;

  gdk_window_begin_paint_region (main_window.drawing_area->window, rgn) ;

  for (Nix = 0 ; Nix < icRects ; Nix++)
    design_draw (project_options.design, main_window.drawing_area->window, GDK_COPY,
      real_to_world_rect (&rcWorld, &(rcRgn[Nix])), LAYER_DRAW_NON_SELECTION) ;

  if (NULL != project_options.srSelection)
    {
    selection_renderer_move (project_options.srSelection, project_options.design, 0.0, 0.0) ;
    selection_renderer_draw (project_options.srSelection, project_options.design, main_window.drawing_area->window, GDK_XOR) ;
    }

  gdk_window_end_paint (main_window.drawing_area->window) ;

  if (bDestroyRegion) gdk_region_destroy (rgn) ;

  project_options.bScrolling = FALSE ;
  }

#ifdef STDIO_FILEIO
static gboolean SaveDirtyUI (GtkWindow *parent, char *szMsg)
  {
  int msgVal = GTK_RESPONSE_CANCEL ;

  if (project_options.bDesignAltered)
    {
    GtkWidget *msg = NULL ;
    msg = gtk_message_dialog_new (parent, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, szMsg) ;
    gtk_dialog_add_action_widget (GTK_DIALOG (msg), gtk_button_new_with_stock_image (GTK_STOCK_NO, _("Don't save")), GTK_RESPONSE_NO) ;
    gtk_dialog_add_button (GTK_DIALOG (msg), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
    gtk_widget_grab_default (gtk_dialog_add_button (GTK_DIALOG (msg), GTK_STOCK_SAVE, GTK_RESPONSE_YES)) ;

    msgVal = gtk_dialog_run (GTK_DIALOG (msg)) ;

    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;

    if (GTK_RESPONSE_YES == msgVal)
      return DoSave (parent, FILEOP_SAVE) ;
    else
    if (GTK_RESPONSE_NO == msgVal)
      {
      char *pszAutoFName = (NULL == project_options.pszCurrentFName) ?
        CreateUserFName ("autosave.qca") :
        g_strdup_printf ("%s~", project_options.pszCurrentFName) ;

#ifdef WIN32
      DeleteFile (pszAutoFName) ;
#else
      unlink (pszAutoFName) ;
#endif /* def WIN32 */

      g_free (pszAutoFName) ;

      return TRUE ;
      }
    if (GTK_RESPONSE_NO != msgVal)
      return FALSE ;
    }
  return TRUE ;
  }

static gboolean DoSave (GtkWindow *parent, int fFileOp)
  {
  char *pszFName = project_options.pszCurrentFName, *pszCurrent = (NULL == project_options.pszCurrentFName ? "" : project_options.pszCurrentFName) ;
  char *pszAutoFName = (NULL == project_options.pszCurrentFName) ?
    CreateUserFName ("autosave.qca") :
    g_strdup_printf ("%s~", project_options.pszCurrentFName) ;

  if (FILEOP_AUTOSAVE == fFileOp)
    {
    if (project_options.bDesignAltered)
      create_file (pszAutoFName, project_options.design) ;

    g_free (pszAutoFName) ;

    return TRUE ;
    }

  if ((NULL == project_options.pszCurrentFName) || (FILEOP_SAVE_AS == fFileOp))
    if (NULL == (pszFName = get_file_name_from_user (parent, _("Save As"), pszCurrent, TRUE)))
      {
      g_free (pszAutoFName) ;
      return FALSE ;
      }

  if (create_file (pszFName, project_options.design))
    {
    char *pszTitle = NULL ;
    project_options.bDesignAltered = FALSE ;
#ifdef WIN32
    DeleteFile (pszAutoFName) ;
#else
    unlink (pszAutoFName) ;
#endif /* def WIN32 */

    gtk_window_set_title (parent, pszTitle = g_strdup_printf ("%s - %s", base_name (pszFName), MAIN_WND_BASE_TITLE)) ;
    g_free (pszTitle) ;
    add_to_recent_files (main_window.recent_files_menu, pszFName, GTK_SIGNAL_FUNC (file_operations), (gpointer)FILEOP_OPEN_RECENT) ;
    if (NULL != project_options.pszCurrentFName && project_options.pszCurrentFName != pszFName)
      g_free (project_options.pszCurrentFName) ;
    project_options.pszCurrentFName = pszFName ;
    g_free (pszAutoFName) ;
    return TRUE ;
    }
  else
    {
    GtkWidget *msg = NULL ;
    gtk_dialog_run (GTK_DIALOG (
      msg = gtk_message_dialog_new (parent, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
        _("Failed to create file \"%s\" !"), pszFName))) ;
    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;
    }
  if (pszFName != project_options.pszCurrentFName) g_free (pszFName) ;
  return FALSE ;
  }
#endif /* def STDIO_FILEIO */

static void layer_apply_default_properties (gpointer key, gpointer value, gpointer user_data)
  {
  QCADDesignObjectClass *klass = g_type_class_peek ((GType)key) ;

  if (NULL != klass)
    qcad_design_object_class_set_properties (klass, (void *)value) ;
  }

// Does whatever else must be done to restore QCADesigner to its initial, pristine state
static void tabula_rasa (GtkWindow *wndMain)
  {
  char *psz = NULL ;
  DBG_CB_HERE (fprintf (stderr, "Entering tabula_rasa\n")) ;

  project_options.design = design_destroy (project_options.design) ;

  // Destroy the old undo stack and connect to a new one.
#ifdef UNDO_REDO
  g_object_unref (qcad_undo_entry_group_get_default ()) ;
  g_signal_connect (G_OBJECT (qcad_undo_entry_group_get_default ()), "state-changed", (GCallback)undo_state_changed, NULL) ;
  gtk_widget_set_sensitive (main_window.undo_menu_item, FALSE) ;
  gtk_widget_set_sensitive (main_window.redo_menu_item, FALSE) ;
#endif /* def UNDO_REDO */

  project_options.bDesignAltered = FALSE ;
  g_free (project_options.pszCurrentFName) ;
  project_options.pszCurrentFName = NULL ;
  psz = g_strdup_printf ("%s - %s", _("Untitled"), MAIN_WND_BASE_TITLE) ;
  gtk_window_set_title (wndMain, psz) ;
  g_free (psz) ;
  gtk_list_clear_items (GTK_LIST (GTK_COMBO (main_window.layers_combo)->list), 0, -1) ;
  set_snap_source (NULL) ;
  g_object_set_data (G_OBJECT (main_window.snap_to_grid_menu_item), "snap_source", NULL) ;
  gtk_widget_set_sensitive (main_window.snap_to_grid_menu_item, FALSE) ;
  simulation_data_destroy(project_options.sim_data);
  project_options.sim_data = NULL;
  gtk_widget_set_sensitive (GTK_WIDGET (project_options.main_window->show_simulation_results_menu_item), FALSE);
#ifdef STDIO_FILEIO
  gtk_widget_set_sensitive (GTK_WIDGET (project_options.main_window->save_output_to_file_menu_item), FALSE);
#endif /* def STDIO_FILEIO */
  }

static void layers_combo_fill_from_design (main_W *wndMain, DESIGN *new_design)
  {
  int icLayers = 0 ;
  GList *lstTmp = NULL ;
  GtkRequisition rq ;
  int cxMax = 0 ;
  QCADLayer *layer = NULL ;

  // Empty the combo
  gtk_list_clear_items (GTK_LIST (GTK_COMBO (wndMain->layers_combo)->list), 0, -1) ;

  for (lstTmp = new_design->lstLastLayer; lstTmp != NULL ; lstTmp = lstTmp->prev)
    {
    (QCAD_LAYER (lstTmp->data))->combo_item = layers_combo_add_layer (GTK_COMBO (wndMain->layers_combo), lstTmp) ;
    gtk_widget_size_request ((QCAD_LAYER (lstTmp->data))->combo_item, &rq) ;
    cxMax = MAX (cxMax, rq.width) ;
    icLayers++ ;
    }

  gtk_widget_set_size_request (wndMain->layers_combo, cxMax, -1) ;

  if (1 == icLayers)
    {
    GtkWidget
      *chkVisible = g_object_get_data (G_OBJECT ((QCAD_LAYER (new_design->lstLayers->data))->combo_item), "chkVisible"),
      *chkActive = g_object_get_data (G_OBJECT ((QCAD_LAYER (new_design->lstLayers->data))->combo_item), "chkActive") ;
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkVisible), TRUE) ;
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkActive), TRUE) ;

    gtk_widget_set_sensitive (chkVisible, FALSE) ;
    gtk_widget_set_sensitive (chkActive, FALSE) ;
    }

  if (NULL != new_design->lstLayers)
    {
    selected_layer_item = ((layer = QCAD_LAYER (new_design->lstLayers->data))->combo_item) ;
    layer_selected (GTK_COMBO (main_window.layers_combo)->popwin, NULL) ;
    }
  }

static GtkWidget *layers_combo_add_layer (GtkCombo *combo, GList *layer)
  {
  GtkWidget *item = NULL, *img = NULL, *chkVisible = NULL, *chkActive = NULL, *vsep = NULL, *lbl = NULL, *tbl = NULL ;//,

  item = gtk_list_item_new () ;
  gtk_widget_show (item) ;
  g_object_set_data_full (G_OBJECT (item), "layer", layer->data, (GDestroyNotify)destroy_notify_layer_combo_item_layer_data) ;
  g_object_set_data (G_OBJECT (item), "combo", combo) ;

  tbl = gtk_table_new (1, 8, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_container_add (GTK_CONTAINER (item), tbl) ;

  img = gtk_image_new_from_stock (layer_pixmap_stock_id[(QCAD_LAYER (layer->data))->type], GTK_ICON_SIZE_MENU) ;
  gtk_widget_show (img) ;
  gtk_table_attach (GTK_TABLE (tbl), img, 0, 1, 0, 1,
    (GtkAttachOptions)(0),
    (GtkAttachOptions)(0), 2, 2) ;

  chkVisible = gtk_check_button_new_with_label (_("Visible")) ;
  g_object_set_data (G_OBJECT (item), "chkVisible", chkVisible) ;
  gtk_widget_show (chkVisible) ;
  gtk_table_attach (GTK_TABLE (tbl), chkVisible, 1, 2, 0, 1,
    (GtkAttachOptions)(0),
    (GtkAttachOptions)(0), 2, 2) ;

  chkActive = gtk_check_button_new_with_label (_("Active")) ;
  g_object_set_data (G_OBJECT (item), "chkActive", chkActive) ;
  gtk_widget_show (chkActive) ;
  gtk_table_attach (GTK_TABLE (tbl), chkActive, 2, 3, 0, 1,
    (GtkAttachOptions)(0),
    (GtkAttachOptions)(0), 2, 2) ;

  vsep = gtk_vseparator_new () ;
  gtk_widget_show (vsep) ;
  gtk_table_attach (GTK_TABLE (tbl), vsep, 3, 4, 0, 1,
    (GtkAttachOptions)(0),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;

  lbl = gtk_label_new (NULL == (QCAD_LAYER (layer->data))->pszDescription ? "" : (QCAD_LAYER (layer->data))->pszDescription) ;
  g_object_set_data (G_OBJECT (item), "lbl", lbl) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 4, 5, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(0), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5) ;

  layers_combo_item_set_check_buttons (item, (QCAD_LAYER (layer->data)), chkVisible, chkActive) ;

  gtk_container_add (GTK_CONTAINER (combo->list), item) ;

  g_signal_connect (G_OBJECT (chkVisible), "toggled",   (GCallback)layer_status_change,              item) ;
  g_signal_connect (G_OBJECT (chkActive),  "toggled",   (GCallback)layer_status_change,              item) ;
  g_signal_connect (G_OBJECT (item),       "select",    (GCallback)layer_select,                     item) ;
  // This isn't really UI stuff, so it should go elsewhere, but this is the crucial function for adding a
  // layer to the UI
  g_signal_connect (G_OBJECT (layer->data),"added",     (GCallback)qcad_layer_design_object_added,   NULL) ;
  g_signal_connect (G_OBJECT (layer->data),"removed",   (GCallback)qcad_layer_design_object_removed, NULL) ;
  return item ;
  }

static void layers_combo_item_set_check_buttons (GtkWidget *item, QCADLayer *layer, GtkWidget *chkVisible, GtkWidget *chkActive)
  {
  if (NULL == item || NULL == layer || NULL == chkVisible || NULL == chkActive) return ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkVisible),
    (LAYER_STATUS_ACTIVE  == layer->status || LAYER_STATUS_VISIBLE == layer->status)) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkActive), (LAYER_STATUS_ACTIVE == layer->status)) ;

  g_object_set_data (G_OBJECT (chkVisible), "bWasActive", (gpointer)(LAYER_STATUS_ACTIVE == layer->status)) ;
  }

static void layers_combo_refresh_item (GtkWidget *item)
  {
  GtkWidget *lbl = NULL ;
  QCADLayer *layer = g_object_get_data (G_OBJECT (item), "layer") ;

  if (NULL == item) return ;

  if (NULL != layer)
    {
    if (NULL != (lbl = g_object_get_data (G_OBJECT (item), "lbl")))
      gtk_label_set_text (GTK_LABEL (lbl),
        NULL == layer->pszDescription ? "" : layer->pszDescription) ;

    layers_combo_item_set_check_buttons (item, layer,
      g_object_get_data (G_OBJECT (item), "chkVisible"),
      g_object_get_data (G_OBJECT (item), "chkActive")) ;
    }
  }

static void qcad_layer_design_object_added (QCADLayer *layer, QCADDesignObject *obj, gpointer data)
  {
  if (QCAD_IS_CELL (obj))
    {
    g_signal_connect (G_OBJECT (obj), "cell-function-changed", (GCallback)cell_function_changed, NULL) ;
    if (QCAD_CELL_INPUT == QCAD_CELL (obj)->cell_function)
      VectorTable_update_inputs (project_options.pvt, QCAD_CELL (obj)) ;
    cell_function_changed (NULL, NULL) ;
    }
  }

static void qcad_layer_design_object_removed (QCADLayer *layer, QCADDesignObject *obj, gpointer data)
  {
  if (QCAD_IS_CELL (obj))
    if (QCAD_CELL_INPUT == QCAD_CELL (obj)->cell_function)
      VectorTable_del_input (project_options.pvt, QCAD_CELL (obj)) ;
  }

static void cell_function_changed (QCADCell *cell, gpointer data)
  {
  gboolean bHaveIO = FALSE ;

  if (NULL != cell)
    VectorTable_update_inputs (project_options.pvt, cell) ;

  gtk_widget_set_sensitive (main_window.bus_layout_menu_item,
    bHaveIO = (project_options.design->bus_layout->inputs->icUsed > 0 ||
               project_options.design->bus_layout->outputs->icUsed > 0)) ;

  gtk_widget_set_sensitive (main_window.bus_layout_button, bHaveIO) ;
  }

static void destroy_notify_layer_combo_item_layer_data (QCADLayer *layer)
  {
  if (NULL == layer) return ;
  if (NULL == project_options.design) return ;

  fprintf (stderr, "destroy_notify_layer_combo_item_layer_data: combo_item is being destroyed !\n") ;

  if (NULL != (layer = design_layer_remove (project_options.design, (layer))))
    {
    project_options.bDesignAltered = TRUE ;
    if (NULL != layer->combo_item)
      selected_layer_item = layer->combo_item ;
    }
  }

static void reflect_layer_status (QCADLayer *layer)
  {
  gboolean bSensitive = (LAYER_STATUS_ACTIVE == layer->status) ;
  if (LAYER_TYPE_CELLS == layer->type)
    {
    DBG_LAYER (fprintf (stderr, "reflect_layer_status: Layer type is LAYER_TYPE_CELLS\n")) ;
    gtk_widget_hide (main_window.substrate_button) ;
    gtk_widget_hide (main_window.label_button) ;

    gtk_widget_show (main_window.toggle_alt_display_button) ;
    gtk_widget_set_sensitive (main_window.toggle_alt_display_button, bSensitive) ;
    gtk_widget_show (main_window.insert_type_1_cell_button) ;
    gtk_widget_set_sensitive (main_window.insert_type_1_cell_button, bSensitive) ;
    gtk_widget_show (main_window.insert_cell_array_button) ;
    gtk_widget_set_sensitive (main_window.insert_cell_array_button, bSensitive) ;
    gtk_widget_show (main_window.rotate_cell_button) ;
    gtk_widget_set_sensitive (main_window.rotate_cell_button, bSensitive) ;
    gtk_widget_show (main_window.clocks_combo_table) ;
    gtk_widget_set_sensitive (main_window.clocks_combo_table, bSensitive) ;
    }
  else
  if (LAYER_TYPE_CLOCKING == layer->type)
    {
    DBG_LAYER (fprintf (stderr, "reflect_layer_status: Layer type is LAYER_TYPE_CLOCKING\n")) ;
    gtk_widget_hide (main_window.insert_type_1_cell_button) ;
    gtk_widget_hide (main_window.insert_cell_array_button) ;
    gtk_widget_hide (main_window.substrate_button) ;
    gtk_widget_hide (main_window.label_button) ;
    gtk_widget_hide (main_window.toggle_alt_display_button) ;
    gtk_widget_hide (main_window.rotate_cell_button) ;
    gtk_widget_hide (main_window.clocks_combo_table) ;
    }
  else
  if (LAYER_TYPE_SUBSTRATE == layer->type)
    {
    DBG_LAYER (fprintf (stderr, "reflect_layer_status: Layer type is LAYER_TYPE_SUBSTRATE\n")) ;
    gtk_widget_hide (main_window.insert_type_1_cell_button) ;
    gtk_widget_hide (main_window.insert_cell_array_button) ;
    gtk_widget_hide (main_window.label_button) ;
    gtk_widget_hide (main_window.toggle_alt_display_button) ;
    gtk_widget_hide (main_window.rotate_cell_button) ;
    gtk_widget_hide (main_window.clocks_combo_table) ;

    gtk_widget_show (main_window.substrate_button) ;
    gtk_widget_set_sensitive (main_window.substrate_button, bSensitive) ;
    }
  else
  if (LAYER_TYPE_DRAWING == layer->type)
    {
    DBG_LAYER (fprintf (stderr, "reflect_layer_status: Layer type is LAYER_TYPE_DRAWING\n")) ;
    gtk_widget_hide (main_window.insert_type_1_cell_button) ;
    gtk_widget_hide (main_window.insert_cell_array_button) ;
    gtk_widget_hide (main_window.substrate_button) ;
    gtk_widget_hide (main_window.toggle_alt_display_button) ;
    gtk_widget_hide (main_window.rotate_cell_button) ;
    gtk_widget_hide (main_window.clocks_combo_table) ;

    gtk_widget_show (main_window.label_button) ;
    gtk_widget_set_sensitive (main_window.label_button, bSensitive) ;
    }
  redraw_async (NULL) ;
  }

#ifdef STDIO_FILEIO
static QCADSubstrate *find_snap_source (DESIGN *design)
  {
  GList *lstLayer = NULL ;
  QCADLayer *layer = NULL ;

  for (lstLayer = design->lstLayers ; lstLayer != NULL ; lstLayer = lstLayer->next)
    if (LAYER_TYPE_SUBSTRATE == (layer = (QCAD_LAYER (lstLayer->data)))->type)
      if (NULL != layer->lstObjs)
        if (QCAD_IS_SUBSTRATE (layer->lstObjs->data))
          return QCAD_SUBSTRATE (layer->lstObjs->data) ;

  return NULL ;
  }

static QCADDesignObject *merge_selection (DESIGN *design, DESIGN *block, EXP_ARRAY *layer_mappings)
  {
  int idx = -1;
  GtkWidget *chkActive = NULL, *chkVisible = NULL ;
  LAYER_MAPPING *layer_mapping = NULL ;
  QCADDesignObject *obj = NULL, *anchor = NULL ;
  GList *llIter = NULL, *llNext = NULL ;

  for(idx = layer_mappings->icUsed - 1; idx > - 1; idx--)
    {
    layer_mapping = &(exp_array_index_1d (layer_mappings, LAYER_MAPPING, idx)) ;
    if(NULL == layer_mapping->design_layer)
      {
      layer_mapping->design_layer = qcad_layer_new (
        layer_mapping->block_layer->type,
        layer_mapping->block_layer->status,
        layer_mapping->block_layer->pszDescription);
      design_layer_add (design, layer_mapping->design_layer) ;
      layer_mapping->design_layer->combo_item = layers_combo_add_layer (GTK_COMBO (main_window.layers_combo),
        design->lstLayers);
      }

    if (NULL != layer_mapping->block_layer->lstObjs)
      {
      // We must ensure that the layer whereupon we are about to drop cells
      // is, in fact, active. We must activate the layer by simulating a
      // click on the "Active" checkbox in the "Layers" combo, so that the
      // UI may reflect the change.
      chkActive = GTK_WIDGET (g_object_get_data (G_OBJECT (layer_mapping->design_layer->combo_item), "chkActive")) ;
      chkVisible = GTK_WIDGET (g_object_get_data (G_OBJECT (layer_mapping->design_layer->combo_item), "chkVisible")) ;

      if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkVisible)))
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkVisible), TRUE);

      if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkActive)))
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkActive), TRUE);

      for(llIter = layer_mapping->block_layer->lstObjs; llIter != NULL; llIter = llIter->next)
        if (NULL != llIter->data)
          qcad_design_object_set_selected (QCAD_DESIGN_OBJECT (llIter->data), TRUE) ;

      // Then we grab all objects from lstSelObjs and move them into the design layer
      // They are already selected
      llIter = layer_mapping->block_layer->lstSelObjs ;
      while (NULL != llIter)
        {
        llNext = llIter->next ;
        if (NULL != llIter->data)
          {
          // Transfer an object from the block layer to the design layer
          g_object_ref (obj = QCAD_DESIGN_OBJECT (llIter->data)) ;

          DBG_MERGE (fprintf (stderr, "Before transfering object 0x%08X to the design:\n", (int)obj)) ;
          DBG_MERGE (fprintf (stderr, "The design layer:\n")) ;
          DBG_MERGE (qcad_layer_dump (layer_mapping->design_layer, stderr)) ;
          DBG_MERGE (fprintf (stderr, "The block layer:\n")) ;
          DBG_MERGE (qcad_layer_dump (layer_mapping->block_layer, stderr)) ;
          DBG_MERGE (fprintf (stderr, "llIter = 0x%08X\n", (int)llIter)) ;

          qcad_do_container_remove (QCAD_DO_CONTAINER (layer_mapping->block_layer), obj) ;
          if (qcad_do_container_add (QCAD_DO_CONTAINER (layer_mapping->design_layer), obj))
            if (NULL == anchor)
              anchor = obj ;

          g_object_unref (obj) ;
          obj = NULL ;

          DBG_MERGE (fprintf (stderr, "After transfering object 0x%08X to the design:\n", (int)obj)) ;
          DBG_MERGE (fprintf (stderr, "The design layer:\n")) ;
          DBG_MERGE (qcad_layer_dump (layer_mapping->design_layer, stderr)) ;
          DBG_MERGE (fprintf (stderr, "The block layer:\n")) ;
          DBG_MERGE (qcad_layer_dump (layer_mapping->block_layer, stderr)) ;
          }

        llIter = llNext ;
        }
      }
    }

  if (NULL != anchor)
    {
    EXP_ARRAY *arSelObjs = NULL ;
    WorldRectangle rcSel ;

    project_options.bDesignAltered = TRUE ;

    if (design_get_extents (design, &rcSel, TRUE))
      {
      double dxMid = 0, dyMid = 0 ;
      double dxMidSel = rcSel.xWorld + rcSel.cxWorld / 2.0,
             dyMidSel = rcSel.yWorld + rcSel.cyWorld / 2.0 ;
      int cx, cy ;

      gdk_window_get_size (main_window.drawing_area->window, &cx, &cy) ;
      dxMid = real_to_world_x (cx >> 1) ;
      dyMid = real_to_world_y (cy >> 1) ;

      design_selection_move (design, dxMid - dxMidSel, dyMid - dyMidSel) ;
      }
    arSelObjs = design_selection_get_object_array (project_options.design) ;
#ifdef UNDO_REDO
    if (NULL != arSelObjs)
      push_undo_selection_existence (project_options.design, project_options.srSelection, main_window.drawing_area->window, arSelObjs, TRUE) ;
#endif /* def UNDO_REDO */
    }

  return anchor ;
  }

gboolean autosave_timer_event (gpointer data)
  {
  DoSave (NULL, FILEOP_AUTOSAVE) ;
  return TRUE ;
  }
#endif /* def STDIO_FILEIO */

static void set_current_design (DESIGN *new_design, QCADSubstrate *subs)
  {
  QCADLayer *layer = NULL ;
  QCADSubstrate *snap_source = NULL ;
  GList *llItrLayers = NULL, *llItrCells = NULL ;

  project_options.design = design_destroy (project_options.design) ;

  project_options.design = new_design ;

  if (NULL != project_options.srSelection)
    selection_renderer_update (project_options.srSelection, new_design) ;

  layers_combo_fill_from_design (&main_window, new_design) ;

  set_snap_source (snap_source = (NULL != subs && gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (main_window.snap_to_grid_menu_item)) ? subs : NULL)) ;
  g_object_set_data (G_OBJECT (main_window.snap_to_grid_menu_item), "snap_source", snap_source) ;
  gtk_widget_set_sensitive (main_window.snap_to_grid_menu_item, NULL != snap_source) ;
  if (NULL != snap_source)
    g_object_weak_ref (G_OBJECT (snap_source), (GWeakNotify)snap_source_is_gone, NULL) ;
  gtk_widget_set_sensitive (main_window.remove_layer_button, (new_design->lstLayers != new_design->lstLastLayer)) ;

  VectorTable_fill (pvt, new_design) ;

  for (llItrLayers = project_options.design->lstLayers ; llItrLayers != NULL ; llItrLayers = llItrLayers->next)
    if (LAYER_TYPE_CELLS == (QCAD_LAYER (llItrLayers->data))->type)
      for (llItrCells = (QCAD_LAYER (llItrLayers->data))->lstObjs ; llItrCells != NULL ; llItrCells = llItrCells->next)
        if (QCAD_IS_CELL (llItrCells->data))
          g_signal_connect (G_OBJECT (llItrCells->data), "cell-function-changed", (GCallback)cell_function_changed, NULL) ;

  cell_function_changed (NULL, NULL) ;

  if (NULL != (layer = QCAD_LAYER (new_design->lstCurrentLayer->data))->combo_item)
    {
    selected_layer_item = layer->combo_item ;
    layer_selected (NULL, NULL) ;
    }
  redraw_async (NULL) ;
  }

static void snap_source_is_gone (gpointer data, QCADSubstrate *subs)
  {
  if (NULL == main_window.snap_to_grid_menu_item) return ;

  if (((QCADSubstrate *)g_object_get_data (G_OBJECT (main_window.snap_to_grid_menu_item), "snap_source")) == subs)
    if (NULL != main_window.snap_to_grid_menu_item)
      {
      g_object_set_data (G_OBJECT (main_window.snap_to_grid_menu_item), "snap_source", NULL) ;
      gtk_widget_set_sensitive (main_window.snap_to_grid_menu_item, FALSE) ;
      }
  }

static void move_selection_to_pointer (QCADDesignObject *anchor)
  {
  selection_renderer_update (project_options.srSelection, project_options.design) ;
  selection_renderer_draw (project_options.srSelection, project_options.design, main_window.drawing_area->window, GDK_XOR) ;

  ACTION_SELECT_sel_changed (anchor, project_options.design) ;
  }

void reflect_zoom ()
  {
  int x, y ;
  selection_renderer_update (project_options.srSelection, project_options.design) ;
  real_coords_from_rulers (&x, &y) ;
  setup_rulers (x, y) ;
  redraw_async (NULL) ;
  }

static void real_coords_from_rulers (int *px, int *py)
  {
  double lower, upper, position, max_size ;

  gtk_ruler_get_range (GTK_RULER (main_window.horizontal_ruler), &lower, &upper, &position, &max_size) ;
  (*px) = world_to_real_x (position) ;
  gtk_ruler_get_range (GTK_RULER (main_window.vertical_ruler), &lower, &upper, &position, &max_size) ;
  (*py) = world_to_real_y (position) ;
  }
