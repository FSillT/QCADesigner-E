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
// This is a place to store widgets that are too        //
// complex to be maintained within the various user     //
// interface elements and have a high chance of being   //
// reused.                                              //
//                                                      //
//////////////////////////////////////////////////////////

// GTK includes //
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef GTK_GUI
  #include <gtk/gtk.h>
#endif /* def GTK_GUI */

#include "support.h"
#include "exp_array.h"
#include "global_consts.h"
#include "custom_widgets.h"

#ifdef GTK_GUI

static void signal_isb_adj_value_changed (GtkAdjustment *adj, gpointer data) ;
static void signal_isb_spn_text_changed (GtkWidget *widget, gpointer data) ;
static void switch_pix (GtkWidget *tbtn, gpointer user_data) ;
static void pop_cursor_priv (GtkWidget *widget) ;
static void push_cursor_priv (GtkWidget *widget, GdkCursor *cursor) ;
static gboolean turn_off_background_pixmap (GtkWidget *widget, gpointer data) ;

static GtkWidget *tvHistory = NULL ;
static GtkWidget *swHistory = NULL ;
static GtkWidget *progress = NULL ;
static GtkProgressBar *progress_bar = NULL ;

void swap_container_contents (GtkWidget *src, GtkWidget *dst)
  {
  GList *lstSrc = gtk_container_get_children (GTK_CONTAINER (src)),
        *lstDst = gtk_container_get_children (GTK_CONTAINER (dst)),
        *lstItr = NULL ;

  for (lstItr = lstSrc ; lstItr != NULL ; lstItr = lstItr->next)
    {
    g_object_ref (lstItr->data) ;
    gtk_container_remove (GTK_CONTAINER (src), lstItr->data) ;
    }

  for (lstItr = lstDst ; lstItr != NULL ; lstItr = lstItr->next)
    {
    g_object_ref (lstItr->data) ;
    gtk_container_remove (GTK_CONTAINER (dst), lstItr->data) ;
    }

  for (lstItr = lstSrc ; lstItr != NULL ; lstItr = lstItr->next)
    {
    gtk_container_add (GTK_CONTAINER (dst), lstItr->data) ;
    g_object_unref (lstItr->data) ;
    }

  for (lstItr = lstDst ; lstItr != NULL ; lstItr = lstItr->next)
    {
    gtk_container_add (GTK_CONTAINER (src), lstItr->data) ;
    g_object_unref (lstItr->data) ;
    }
  }

// Creates a button whose label is a pixmap
GtkWidget *create_pixmap_button (GtkWidget *pixmap, gchar *text, gboolean bToggle)
  {
  GtkWidget *btn ;
  GtkWidget *vbox1, *label1 ;

  btn = bToggle ? gtk_toggle_button_new () : gtk_button_new () ;
  gtk_widget_ref (btn) ;
  gtk_object_set_data_full (GTK_OBJECT (btn), "btn", btn, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (btn) ;

  vbox1 = gtk_table_new (2, 1, FALSE) ;
  gtk_widget_ref (vbox1) ;
  gtk_object_set_data_full (GTK_OBJECT (btn), "vbox1", vbox1, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox1) ;
  gtk_container_add (GTK_CONTAINER (btn), vbox1) ;

  gtk_object_set_data_full (GTK_OBJECT (btn), "pixmap", pixmap, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (pixmap) ;
  gtk_table_attach (GTK_TABLE (vbox1), pixmap, 0, 1, 0, 1,
    (GtkAttachOptions)(0),
    (GtkAttachOptions)(0), 2, 2) ;

  if (NULL != text)
    {
    label1 = gtk_label_new (text) ;
    gtk_widget_ref (label1) ;
    gtk_object_set_data_full (GTK_OBJECT (btn), "label1", label1, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label1) ;
    gtk_table_attach (GTK_TABLE (vbox1), label1, 1, 2, 0, 1,
      (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
      (GtkAttachOptions)(0), 2, 2) ;
    gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT) ;
    gtk_misc_set_alignment (GTK_MISC (label1), 0.0, 0.5) ;
    }
  return btn ;
  }

void pixmap_button_toggle_pixmap (GtkWidget *btn, gboolean bShow)
  {
  GtkWidget *pixmap = NULL ;
  if (NULL != (pixmap = gtk_object_get_data (GTK_OBJECT (btn), "pixmap")))
    {
    if (bShow)
      gtk_widget_show (pixmap) ;
    else
      gtk_widget_hide (pixmap) ;
    }
  }

// Creates a toggle button with a "down" pixmap and an "up" pixmap
// both pixmaps are added to a vbox, but one of them is always hidden.  A signal
// handler is connected to the "toggled" signal to hide the visible pixmap and
// unhide the hidden one
GtkWidget *create_two_pixmap_toggle_button (GtkWidget *pix1, GtkWidget *pix2, gchar *pszLbl)
  {
  GtkWidget *btn ;
  GtkWidget *vbox1, *label1 ;

  btn = gtk_toggle_button_new () ;
  gtk_widget_ref (btn) ;
  gtk_object_set_data_full (GTK_OBJECT (btn), "btn", btn, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (btn) ;

  vbox1 = gtk_vbox_new (FALSE, 0) ;
  gtk_widget_ref (vbox1) ;
  gtk_object_set_data_full (GTK_OBJECT (btn), "vbox1", vbox1, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox1) ;
  gtk_container_add (GTK_CONTAINER (btn), vbox1) ;
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 2) ;

  gtk_widget_ref (GTK_WIDGET (pix1)) ;
  gtk_object_set_data_full (GTK_OBJECT (btn), "pix1", pix1, (GtkDestroyNotify) gtk_widget_unref) ;
  gtk_widget_show (GTK_WIDGET (pix1)) ;
  gtk_box_pack_start (GTK_BOX (vbox1), GTK_WIDGET (pix1), FALSE, FALSE, 0) ;
  gtk_widget_ref (GTK_WIDGET (pix2)) ;
  gtk_object_set_data_full (GTK_OBJECT (btn), "pix2", pix2, (GtkDestroyNotify) gtk_widget_unref) ;
  gtk_widget_show (GTK_WIDGET (pix2)) ;
  gtk_box_pack_start (GTK_BOX (vbox1), GTK_WIDGET (pix2), FALSE, FALSE, 0) ;
  gtk_widget_hide (GTK_WIDGET (pix2)) ;

  if (NULL != pszLbl)
    {
    label1 = gtk_label_new (pszLbl) ;
    gtk_widget_ref (label1) ;
    gtk_object_set_data_full (GTK_OBJECT (btn), "label1", label1, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label1) ;
    gtk_box_pack_end (GTK_BOX (vbox1), label1, TRUE, TRUE, 0) ;
    gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_RIGHT) ;
    gtk_misc_set_alignment (GTK_MISC (label1), 0.5, 0.5) ;
    }

  gtk_signal_connect (GTK_OBJECT (btn), "toggled", GTK_SIGNAL_FUNC (switch_pix), vbox1) ;

  return btn ;
  }

static void switch_pix (GtkWidget *tbtn, gpointer user_data)
  {
  GtkWidget *pix_old = NULL, *pix_new = NULL ;
  gboolean bActive = FALSE ;

  bActive = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (tbtn)) ;
  pix_old = gtk_object_get_data (GTK_OBJECT (tbtn), bActive ? "pix1" : "pix2") ;
  pix_new = gtk_object_get_data (GTK_OBJECT (tbtn), bActive ? "pix2" : "pix1") ;
  gtk_widget_hide (pix_old) ;
  gtk_widget_show (pix_new) ;
  }

GtkWidget *gtk_button_new_with_stock_image (gchar *pszStock, gchar *pszLabel)
  {
  GtkWidget *btn = NULL, *hbox = NULL, *align = NULL, *label = NULL, *img = NULL ;

  btn = gtk_button_new () ;
  gtk_widget_show (btn) ;
  GTK_WIDGET_SET_FLAGS (btn, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;

  align = gtk_alignment_new (0.5, 0.5, 0, 0) ;
  gtk_widget_show (align) ;
  gtk_container_add (GTK_CONTAINER (btn), align) ;

  hbox = gtk_hbox_new (FALSE, 0) ;
  gtk_widget_show (hbox) ;
  gtk_container_add (GTK_CONTAINER (align), hbox) ;
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0) ;

  img = gtk_image_new_from_stock (pszStock, GTK_ICON_SIZE_BUTTON) ;
  gtk_widget_show (img) ;
  gtk_box_pack_start (GTK_BOX (hbox), img, FALSE, FALSE, 0) ;

  label = gtk_label_new (pszLabel) ;
  gtk_widget_show (label) ;
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0) ;

  return btn ;
  }

GtkWidget *gtk_spin_button_new_infinite (GtkAdjustment *adj, gdouble climb_rate, guint digits, ISBDirection direction)
  {
  GtkWidget *ret = gtk_spin_button_new (adj, climb_rate, digits) ;
  g_signal_connect (adj, "value_changed", (GCallback)signal_isb_adj_value_changed, (gpointer)direction) ;
  g_signal_connect (ret, "changed",       (GCallback)signal_isb_spn_text_changed,  (gpointer)direction) ;
  return ret ;
  }

void set_widget_background_colour (GtkWidget *widget, int r, int g, int b)
  {
  GdkColor clr = {0, r, g, b} ;

  gtk_widget_modify_bg (widget, GTK_STATE_NORMAL,      &clr) ;
  gtk_widget_modify_bg (widget, GTK_STATE_ACTIVE,      &clr) ;
  gtk_widget_modify_bg (widget, GTK_STATE_PRELIGHT,    &clr) ;
  gtk_widget_modify_bg (widget, GTK_STATE_SELECTED,    &clr) ;
  gtk_widget_modify_bg (widget, GTK_STATE_INSENSITIVE, &clr) ;

  g_signal_connect_after (G_OBJECT (widget), "style_set", (GCallback)turn_off_background_pixmap, NULL) ;
  g_signal_connect_after (G_OBJECT (widget), "realize",   (GCallback)turn_off_background_pixmap, NULL) ;
  }

void scrolled_window_set_size (GtkScrolledWindow *sw, GtkWidget *rqWidget, double dcxMaxScreenPercent, double dcyMaxScreenPercent)
  {
  GtkPolicyType hpol, vpol ;
  GList *llChildren = gtk_container_get_children (GTK_CONTAINER (sw)) ;
  int xScr, yScr, cxScr, cyScr, depth ;
  GtkRequisition rq = {-1, -1} ;

  if (NULL == llChildren) return ;

  gtk_scrolled_window_get_policy (sw, &hpol, &vpol) ;
  gtk_scrolled_window_set_policy (sw, GTK_POLICY_NEVER, GTK_POLICY_NEVER) ;

  gdk_window_get_geometry (gdk_get_default_root_window (), &xScr, &yScr, &cxScr, &cyScr, &depth) ;

  gtk_widget_size_request (rqWidget, &rq) ;

  gtk_widget_set_size_request (GTK_WIDGET (llChildren->data),
    MIN (cxScr * dcxMaxScreenPercent, rq.width),
    MIN (cyScr * dcyMaxScreenPercent, rq.height)) ;

  gtk_scrolled_window_set_policy (sw, hpol, vpol) ;
  }

static void signal_isb_adj_value_changed (GtkAdjustment *adj, gpointer data)
  {
  ISBDirection direction = (ISBDirection)data ;

  if (direction & ISB_DIR_UP)
    while (adj->value >= adj->upper * 0.9)
      adj->upper += adj->page_increment ;

  if (direction & ISB_DIR_DN)
    while (adj->value <= adj->lower * 0.9)
      adj->lower -= adj->page_increment ;
  }

static void signal_isb_spn_text_changed (GtkWidget *widget, gpointer data)
  {
  GtkAdjustment *adj = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (widget)) ;
  ISBDirection direction = (ISBDirection)data ;
  char *pszText = gtk_editable_get_chars (GTK_EDITABLE (widget), 0, -1) ;
  double dVal = (NULL == pszText ? 0.0 : atof (pszText)) ;

  g_free (pszText) ;

  if (direction & ISB_DIR_UP)
    adj->upper = MAX (dVal, adj->upper) ;
  else
  if (direction & ISB_DIR_DN)
    adj->lower = MIN (dVal, adj->lower) ;
  }

GtkWidget *command_history_create ()
  {
  if (NULL != swHistory) return swHistory ;

  swHistory = gtk_scrolled_window_new (NULL, NULL) ;
  gtk_widget_show (swHistory) ;
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (swHistory), GTK_SHADOW_IN) ;
  tvHistory = gtk_text_view_new () ;
  gtk_widget_show (tvHistory) ;
  gtk_container_add (GTK_CONTAINER (swHistory), tvHistory) ;
  gtk_text_view_set_editable (GTK_TEXT_VIEW (tvHistory), FALSE) ;
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (tvHistory), FALSE);
  return swHistory ;
  }

void draw_string (GdkDrawable *dst, GdkGC *gc, char *pszFont, int x, int y, char *psz)
  {
  PangoContext *context = NULL ;
  PangoLayout *layout = NULL ;
  PangoFontDescription *fontdesc = NULL ;

  context = gdk_pango_context_get () ;
  fontdesc = pango_font_description_from_string (pszFont) ;
  pango_context_set_font_description (context, fontdesc) ;
  layout = pango_layout_new (context) ;
  pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER) ;
  pango_layout_set_text (layout, psz, -1) ;
  gdk_draw_layout (dst, gc, x, y, layout) ;
  pango_font_description_free (fontdesc) ;
  g_object_unref (layout) ;
  }

void get_string_dimensions (char *psz, char *pszFont, int *pcx, int *pcy)
  {
  PangoContext *context = NULL ;
  PangoLayout *layout = NULL ;
  PangoFontDescription *fontdesc = NULL ;

  context = gdk_pango_context_get () ;
  fontdesc = pango_font_description_from_string (pszFont) ;
  pango_context_set_font_description (context, fontdesc) ;
  layout = pango_layout_new (context) ;
  pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER) ;
  pango_layout_set_text (layout, psz, -1) ;
  pango_layout_get_pixel_size (layout, pcx, pcy) ;
  pango_font_description_free (fontdesc) ;
  g_object_unref (layout) ;
  }

void gtk_widget_get_root_origin (GtkWidget *widget, int *px, int *py)
  {
  int x_root, y_root ;

  gdk_window_get_origin (widget->window, &x_root, &y_root) ;
  (*px) = x_root + widget->allocation.x ;
  (*py) = y_root + widget->allocation.y ;
  }


GtkWidget *create_labelled_progress_bar ()
  {
  GtkWidget *lbl = NULL ;

  if (NULL != progress) return progress ;

  progress = gtk_table_new (1, 2, FALSE) ;

  lbl = gtk_label_new ("") ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (progress), lbl, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 0) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5) ;

  progress_bar = GTK_PROGRESS_BAR (gtk_progress_bar_new ()) ;
  gtk_widget_show (GTK_WIDGET (progress_bar)) ;
  gtk_table_attach (GTK_TABLE (progress), GTK_WIDGET (progress_bar), 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 0) ;
  gtk_progress_bar_set_orientation (progress_bar, GTK_PROGRESS_LEFT_TO_RIGHT) ;

  g_object_set_data (G_OBJECT (progress), "lbl", lbl) ;

  return progress ;
  }

void push_cursor (GtkWidget *widget, GdkCursor *cursor)
  {
  EXP_ARRAY *cursor_stack = g_object_get_data (G_OBJECT (widget), "cursor_stack") ;

  if (NULL != widget->window)
    {
    if (NULL == cursor_stack)
      {
      cursor_stack = exp_array_new (sizeof (GdkCursor *), 1) ;
      g_object_set_data (G_OBJECT (widget), "cursor_stack", cursor_stack) ;
      }

    exp_array_insert_vals (cursor_stack, &cursor, 1, 1, -1) ;

    gdk_window_set_cursor (widget->window, cursor) ;
    }

  if (GTK_IS_CONTAINER (widget))
    gtk_container_foreach (GTK_CONTAINER (widget), (GtkCallback)push_cursor_priv, cursor) ;
  }

static void push_cursor_priv (GtkWidget *widget, GdkCursor *cursor)
  {
  EXP_ARRAY *cursor_stack = g_object_get_data (G_OBJECT (widget), "cursor_stack") ;

  if (widget->window != NULL)
    {
    if (NULL == cursor_stack)
      {
      cursor_stack = exp_array_new (sizeof (GdkCursor *), 1) ;
      g_object_set_data (G_OBJECT (widget), "cursor_stack", cursor_stack) ;
      }

    exp_array_insert_vals (cursor_stack, &cursor, 1, 1, -1) ;

    gdk_cursor_ref (cursor) ;

    gdk_window_set_cursor (widget->window, cursor) ;
    }

  if (GTK_IS_CONTAINER (widget))
    gtk_container_foreach (GTK_CONTAINER (widget), (GtkCallback)push_cursor_priv, cursor) ;
  }

GdkCursor *pop_cursor (GtkWidget *widget)
  {
  GdkCursor *ret = NULL ;
  EXP_ARRAY *cursor_stack = g_object_get_data (G_OBJECT (widget), "cursor_stack") ;

  if (NULL != widget->window)
    {
    if (NULL == cursor_stack)
      {
      cursor_stack = exp_array_new (sizeof (GdkCursor *), 1) ;
      g_object_set_data (G_OBJECT (widget), "cursor_stack", cursor_stack) ;
      }

    if (cursor_stack->icUsed > 0)
      {
      ret = exp_array_index_1d (cursor_stack, GdkCursor *, cursor_stack->icUsed - 1) ;
      exp_array_remove_vals (cursor_stack, 1, cursor_stack->icUsed - 1, 1) ;
      }

    gdk_window_set_cursor (widget->window,
      (cursor_stack->icUsed > 0) ?
        exp_array_index_1d (cursor_stack, GdkCursor *, cursor_stack->icUsed - 1) : NULL) ;
    }

  if (GTK_IS_CONTAINER (widget))
    gtk_container_foreach (GTK_CONTAINER (widget), (GtkCallback)pop_cursor_priv, NULL) ;

  return ret ;
  }

static void pop_cursor_priv (GtkWidget *widget)
  {
  GdkCursor *ret = NULL ;
  EXP_ARRAY *cursor_stack = g_object_get_data (G_OBJECT (widget), "cursor_stack") ;

  if (NULL != widget->window)
    {
    if (NULL == cursor_stack)
      {
      cursor_stack = exp_array_new (sizeof (GdkCursor *), 1) ;
      g_object_set_data (G_OBJECT (widget), "cursor_stack", cursor_stack) ;
      }

    if (cursor_stack->icUsed > 0)
      {
      ret = exp_array_index_1d (cursor_stack, GdkCursor *, cursor_stack->icUsed - 1) ;
      exp_array_remove_vals (cursor_stack, 1, cursor_stack->icUsed - 1, 1) ;
      }

    gdk_window_set_cursor (widget->window,
      (cursor_stack->icUsed > 0) ?
        exp_array_index_1d (cursor_stack, GdkCursor *, cursor_stack->icUsed - 1) : NULL) ;

    if (NULL != ret)
      gdk_cursor_unref (ret) ;
    }

  if (GTK_IS_CONTAINER (widget))
    gtk_container_foreach (GTK_CONTAINER (widget), (GtkCallback)pop_cursor_priv, NULL) ;
  }

static gboolean turn_off_background_pixmap (GtkWidget *widget, gpointer data)
  {
  if (NULL != widget->window)
    gdk_window_set_back_pixmap (widget->window, NULL, FALSE) ;

  return TRUE ;
  }

void set_ruler_scale (GtkRuler *ruler, double dLower, double dUpper)
  {
  return ;
  }


void set_window_icon (GtkWindow *window, char *pszBaseName)
  {
  // Our windows will get icons from this list
  char *psz = NULL ;
  GList *icon_list = NULL ;
#ifdef HAVE_LIBRSVG
  gchar *pszIconFile = NULL ;
#endif
#ifdef HAVE_LIBRSVG
  pszIconFile = find_pixmap_file (psz = g_strdup_printf ("%s.svg", pszBaseName)) ;
  g_free (psz) ;
  if (!((NULL == window)
    ? gtk_window_set_default_icon_from_file (pszIconFile, NULL)
    : gtk_window_set_icon_from_file (window, pszIconFile, NULL)))
    {
#endif
    icon_list = g_list_append (icon_list, create_pixbuf (psz = g_strdup_printf ("%s_8_16x16x8.png", pszBaseName))) ;
    g_free (psz) ;
    icon_list = g_list_append (icon_list, create_pixbuf (psz = g_strdup_printf ("%s_7_32x32x8.png", pszBaseName))) ;
    g_free (psz) ;
    icon_list = g_list_append (icon_list, create_pixbuf (psz = g_strdup_printf ("%s_6_48x48x8.png", pszBaseName))) ;
    g_free (psz) ;
    icon_list = g_list_append (icon_list, create_pixbuf (psz = g_strdup_printf ("%s_5_16x16x24.png", pszBaseName))) ;
    g_free (psz) ;
    icon_list = g_list_append (icon_list, create_pixbuf (psz = g_strdup_printf ("%s_4_32x32x24.png", pszBaseName))) ;
    g_free (psz) ;
    icon_list = g_list_append (icon_list, create_pixbuf (psz = g_strdup_printf ("%s_3_48x48x24.png", pszBaseName))) ;
    g_free (psz) ;
    icon_list = g_list_append (icon_list, create_pixbuf (psz = g_strdup_printf ("%s_2_16x16x24a.png", pszBaseName))) ;
    g_free (psz) ;
    icon_list = g_list_append (icon_list, create_pixbuf (psz = g_strdup_printf ("%s_1_32x32x24a.png", pszBaseName))) ;
    g_free (psz) ;
    icon_list = g_list_append (icon_list, create_pixbuf (psz = g_strdup_printf ("%s_0_48x48x24a.png", pszBaseName))) ;
    g_free (psz) ;
    if (NULL == window)
      gtk_window_set_default_icon_list (icon_list) ;
    else
      gtk_window_set_icon_list (window, icon_list) ;
    g_list_free (icon_list) ;
#ifdef HAVE_LIBRSVG
    }

  g_free (pszIconFile) ;
#endif /* def HAVE_LIBRSVG */
  }

void tree_model_row_changed (GtkTreeModel *model, GtkTreeIter *itr)
  {
  GtkTreePath *tp = NULL ;

  if (NULL == model || NULL == itr) return ;
  if (NULL == (tp = gtk_tree_model_get_path (model, itr))) return ;

  gtk_tree_model_row_changed (model, tp, itr) ;

  gtk_tree_path_free (tp) ;
  }

void gtk_widget_button_press (GtkWidget *widget, int button, int x, int y, GdkModifierType mask)
  {
  GdkEventButton *event = NULL ;
  int x_root, y_root ;

  if (NULL == widget) return ;
  if (NULL == widget->window) return ;

  gdk_window_get_origin (widget->window, &x_root, &y_root) ;

  event = (GdkEventButton *)gdk_event_new (GDK_BUTTON_PRESS) ;

  event->type = GDK_BUTTON_PRESS ;
  event->window = widget->window ;
  event->send_event = TRUE ;
  event->time = gtk_get_current_event_time () ;
  event->x = x + widget->allocation.x ;
  event->y = y + widget->allocation.y ;
  event->axes = NULL ;
  event->state = mask ;
  event->x_root = x_root + event->x ;
  event->y_root = y_root + event->y ;

  fprintf (stderr, "gtk_widget_button_press: At (%d,%d) = root:(%d,%d)\n", (int)(event->x), (int)(event->y), (int)(event->x_root), (int)(event->y_root)) ;

  gtk_main_do_event ((GdkEvent *)event) ;
  }

#endif /* def GTK_GUI */

GdkColor *clr_idx_to_clr_struct (int clr_idx)
  {
  static GHashTable *ht_clr = NULL ;
  GdkColor *pclr = NULL ;

  if (NULL == ht_clr)
    ht_clr = g_hash_table_new (NULL, NULL) ;

  if (NULL == (pclr = g_hash_table_lookup (ht_clr, (gpointer)clr_idx)))
    {
    pclr = g_malloc0 (sizeof (GdkColor)) ;
    switch (clr_idx)
      {
      case WHITE:
        pclr->red = 0xFFFF ;
        pclr->green = 0xFFFF ;
        pclr->blue = 0xFFFF ;
        break ;

      case GREEN:
        pclr->red = 0x0000 ;
        pclr->green = 0xFFFF ;
        pclr->blue = 0x0000 ;
        break ;

      case GREEN1:
        pclr->red = 0xF000;
        pclr->green = 0x0FFF;
        pclr->blue = 0xF000;
        break;

      case GREEN2:
        pclr->red = 0xFF00;
        pclr->green = 0xAAFF;
        pclr->blue = 0xFF00;
        break;

      case GREEN3:
        pclr->red = 0xFFF0;
        pclr->green = 0xFFFF;
        pclr->blue = 0xFFF0;
        break;

      case ORANGE:
        pclr->red = 0xFFFF;
        pclr->green = 0x6000;
        pclr->blue = 0x5000;
        break;

      case RED:
        pclr->red = 0xFFFF;
        pclr->green = 0x0000;
        pclr->blue = 0x0000;
        break;

      case BLUE:
        pclr->red = 0x0000;
        pclr->green = 0x0000;
        pclr->blue = 0xFFFF;
        break;

      case YELLOW:
        pclr->red = 0xFFFF;
        pclr->green = 0xFFFF;
        pclr->blue = 0x0000;
        break;

      case HONEYCOMB_BACKGROUND:
        pclr->red = 0x3fff ;
        pclr->green = 0x3fff ;
        pclr->blue = 0x3fff ;
        break ;

      case BLACK:
      default:
        pclr->red = 0x0000 ;
        pclr->green = 0x0000 ;
        pclr->blue = 0x0000 ;
        break ;
      }
            
#ifdef GTK_GUI
    gdk_colormap_alloc_color (gdk_colormap_get_system (), pclr, TRUE, TRUE) ;
#endif /* def GTK_GUI */
    g_hash_table_insert (ht_clr, (gpointer)clr_idx, pclr) ;
    }
  return pclr ;
  }

void set_progress_bar_visible (gboolean bVisible)
  {
  #ifdef GTK_GUI
  if (NULL == progress || NULL == progress_bar) return ;
  if (bVisible)
    {
    gtk_widget_show (progress) ;
    gtk_progress_bar_set_fraction (progress_bar, 0.0) ;
    }
  else
    gtk_widget_hide (progress) ;
  while (gtk_events_pending ())
    gtk_main_iteration () ;
  #endif /* def GTK_GUI */
  }

void set_progress_bar_label (char *psz)
  {
  #ifdef GTK_GUI
  if (NULL == progress || NULL == progress_bar) return ;

  gtk_label_set_text (g_object_get_data (G_OBJECT (progress), "lbl"), psz) ;
  while (gtk_events_pending ())
    gtk_main_iteration () ;
  #else
    fprintf (stderr, "%s\n", psz) ;
  #endif /* def GTK_GUI */
  }

void set_progress_bar_fraction (double dFraction)
  {
  #ifdef GTK_GUI
  char *psz = NULL ;

  if (NULL == progress || NULL == progress_bar) return ;

  if (gtk_progress_bar_get_fraction (progress_bar) == dFraction) return ;

  gtk_progress_bar_set_fraction (progress_bar, dFraction) ;
  gtk_progress_bar_set_text (progress_bar, psz = g_strdup_printf ("%4.1lf%%", dFraction * 100.0)) ;
  g_free (psz) ;
  while (gtk_events_pending ())
    gtk_main_iteration () ;
  #else
    fprintf (stderr, "\r\033[K%4.1lf%%\r", dFraction * 100.0) ;
  #endif /* def GTK_GUI */
  }

void command_history_message (char *pszFmt, ...)
  {
#ifdef GTK_GUI
  va_list va ;
  gchar *pszMsg = NULL ;
  GtkTextBuffer *ptbCH = NULL ;
  GtkTextIter gtiEnd ;
  GtkTextMark *markEnd ;

  if (NULL == swHistory || NULL == tvHistory) return ;

  ptbCH = gtk_text_view_get_buffer (GTK_TEXT_VIEW (tvHistory)) ;

  va_start (va, pszFmt) ;
  pszMsg = g_strdup_vprintf (pszFmt, va) ;
  va_end (va) ;

  gtk_text_buffer_get_end_iter (ptbCH, &gtiEnd) ;
  gtk_text_buffer_insert (ptbCH, &gtiEnd, pszMsg, strlen (pszMsg)) ;
  markEnd = gtk_text_buffer_create_mark (ptbCH, NULL, &gtiEnd, FALSE) ;
  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (tvHistory), markEnd) ;
  gtk_text_buffer_delete_mark (ptbCH, markEnd) ;

  while (gtk_events_pending ())
    gtk_main_iteration () ;
#else
  va_list va ;
  va_start (va, pszFmt) ;
  vfprintf (stderr, pszFmt, va) ;
  va_end (va) ;
#endif /* def GTK_GUI */
  }
