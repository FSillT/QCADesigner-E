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
// The layer properties dialog. This allows the user to //
// set the name of the layer, set the status of the la- //
// yer, and set the default properties for the various  //
// object types admissible in the layer.                //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include <string.h>
#include "support.h"
#include "layer_properties_dialog.h"
#include "design.h"
#include "custom_widgets.h"
#include "objects/QCADLayer.h"

#define DBG_LPD_FLAGS(s)
#define DBG_LPD_PROPS(s)

typedef struct
 {
 GtkWidget *dlg ;
 GtkWidget *cbLayerType ;
 GtkWidget *cbLayerStatus ;
 GtkWidget *txtLayerDescription ;
 GtkWidget *swObjs ;
 GtkWidget *lstObjs ;
 GtkWidget *tblObjUI ;
 GHashTable *htObjUI ;
 GHashTable *htObjCB ;
 GHashTable *htObjCBData ;
 } layer_properties_D ;

static char
  **pszLayerTypes = NULL,
  **pszLayerStati = NULL ;

static layer_properties_D layer_properties_dialog ;

static void create_layer_properties_dialog (layer_properties_D *dialog) ;
static LayerType layer_type_from_description (char *pszDescription) ;
static LayerStatus layer_status_from_description (char *pszDescription) ;
static void set_layer_type (GtkWindow *wnd, QCADLayer *layer, LayerType layer_type) ;
static void reflect_layer_type (GtkWidget *widget, gpointer data) ;
static void object_type_selected (GtkWidget *widget, gpointer data) ;
static void hide_object_UIs (gpointer key, gpointer value, gpointer data) ;

static char **define_layer_types () ;
static char **define_layer_stati () ;
extern char *layer_pixmap_stock_id[LAYER_TYPE_LAST_TYPE] ;

gboolean get_layer_properties_from_user (GtkWindow *wnd, QCADLayer *layer, gboolean bAllowLayerType, gboolean bAllowLayerStatus)
  {
  gboolean bRet = FALSE ;
  char *pszText = NULL ;
  GList *llItr = NULL ;
  void (*cb) (void *) = NULL ;
  void *current_default_properties = NULL,
       *layer_default_properties = NULL ;
  QCADDesignObjectClass *klass = NULL ;

  if (NULL == pszLayerTypes)
    pszLayerTypes = define_layer_types () ;

  if (NULL == pszLayerStati)
    pszLayerStati = define_layer_stati () ;

  if (NULL == layer_properties_dialog.dlg) {
    create_layer_properties_dialog (&layer_properties_dialog) ;
  }

  gtk_window_set_transient_for (GTK_WINDOW (layer_properties_dialog.dlg), wnd) ;

  gtk_list_select_item (GTK_LIST (GTK_COMBO (layer_properties_dialog.cbLayerType)->list), layer->type) ;
  gtk_list_select_item (GTK_LIST (GTK_COMBO (layer_properties_dialog.cbLayerStatus)->list), layer->status) ;
  gtk_entry_set_text (GTK_ENTRY (layer_properties_dialog.txtLayerDescription), NULL == layer->pszDescription ? "" : layer->pszDescription) ;

  gtk_widget_set_sensitive (layer_properties_dialog.cbLayerType, bAllowLayerType) ;
  gtk_widget_set_sensitive (layer_properties_dialog.cbLayerStatus, bAllowLayerStatus) ;

  reflect_layer_type (NULL, &layer_properties_dialog) ;

  if ((bRet = (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (layer_properties_dialog.dlg)))))
    {
    if (NULL != layer->pszDescription)
      g_free (layer->pszDescription) ;
    layer->pszDescription = gtk_editable_get_chars (GTK_EDITABLE (layer_properties_dialog.txtLayerDescription), 0, -1) ;

    layer->status =
      layer_status_from_description (pszText =
        gtk_editable_get_chars (GTK_EDITABLE (GTK_COMBO (layer_properties_dialog.cbLayerStatus)->entry), 0, -1)) ;
    g_free (pszText) ;

    set_layer_type (wnd, layer,
      layer_type_from_description (pszText =
        gtk_editable_get_chars (GTK_EDITABLE (GTK_COMBO (layer_properties_dialog.cbLayerType)->entry), 0, -1))) ;
    g_free (pszText) ;

    // Grab the layer-specific default properties for each object type in this layer
    for (llItr = g_hash_table_lookup (qcad_layer_object_containment_rules (), (gpointer)(layer->type)) ; llItr != NULL ; llItr = llItr->next)
      {
      if (NULL != (cb = (void (*) (void *))g_hash_table_lookup (layer_properties_dialog.htObjCB, llItr->data)))
        {
        klass = QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek ((GType)(llItr->data))) ;
        current_default_properties = qcad_design_object_class_get_properties (klass) ;
        (*cb) (g_hash_table_lookup (layer_properties_dialog.htObjCBData, llItr->data)) ;
        if (NULL != (layer_default_properties = g_hash_table_lookup (layer->default_properties, llItr->data)))
          qcad_design_object_class_destroy_properties (klass, layer_default_properties) ;
        g_hash_table_replace (layer->default_properties, llItr->data, qcad_design_object_class_get_properties (klass)) ;
        qcad_design_object_class_set_properties (klass, current_default_properties) ;
        }
      }

    DBG_LPD_FLAGS (fprintf (stderr, "get_layer_propeties_from_user:layer->status = %d, layer->type = %d\n",
      layer->status, layer->type)) ;
    }

  gtk_widget_hide (layer_properties_dialog.dlg) ;

  return bRet ;
  }

static char **define_layer_types ()
  {
  char **pszTypes = g_malloc0 (LAYER_TYPE_LAST_TYPE * sizeof (char *)) ;

  pszTypes[LAYER_TYPE_SUBSTRATE] = _("Substrate") ;
  pszTypes[LAYER_TYPE_CELLS] = _("Cells") ;
  pszTypes[LAYER_TYPE_CLOCKING] = _("Clocking") ;
  pszTypes[LAYER_TYPE_DRAWING] = _("Drawing") ;

  return pszTypes ;
  }

static char **define_layer_stati ()
  {
  char **pszStati = g_malloc0 (LAYER_STATUS_LAST_STATUS * sizeof (char *)) ;

  pszStati[LAYER_STATUS_ACTIVE] = _("Active") ;
  pszStati[LAYER_STATUS_VISIBLE] = _("Visible") ;
  pszStati[LAYER_STATUS_HIDDEN] = _("Hidden") ;

  return pszStati ;
  }

static LayerType layer_type_from_description (char *pszDescription)
  {
  return (!strcmp (pszDescription, _("Substrate")) ? LAYER_TYPE_SUBSTRATE :
          !strcmp (pszDescription, _("Cells"))     ? LAYER_TYPE_CELLS :
          !strcmp (pszDescription, _("Clocking")) ?  LAYER_TYPE_CLOCKING :
          !strcmp (pszDescription, _("Drawing")) ?   LAYER_TYPE_DRAWING :
          LAYER_TYPE_LAST_TYPE) ;
  }

static LayerStatus layer_status_from_description (char *pszDescription)
  {
  return (!strcmp (pszDescription, _("Active")) ? LAYER_STATUS_ACTIVE :
          !strcmp (pszDescription, _("Visible")) ? LAYER_STATUS_VISIBLE :
          !strcmp (pszDescription, _("Hidden")) ? LAYER_STATUS_HIDDEN : LAYER_STATUS_LAST_STATUS) ;
  }

static void create_layer_properties_dialog (layer_properties_D *dialog)
  {
  int Nix, idx = 0 ;
  GtkWidget
    *img = NULL,
    *item = NULL,
    *tbl = NULL,
    *tblMain = NULL,
    *hbox = NULL,
    *frm = NULL,
    *objUI = NULL,
    *lbl = NULL ;
  GList *lstComboItems = NULL ;
  GCallback cbObj = NULL ;
  gpointer data = NULL ;
  QCADDesignObjectClass *klass = NULL ;
  GType *types = NULL ;
  guint icChildren = 0 ;

  DBG_LPD_PROPS (fprintf (stderr, "create_layer_properties_dialog:Entering\n")) ;

  dialog->dlg = gtk_dialog_new () ;
  gtk_window_set_title (GTK_WINDOW (dialog->dlg), _("Layer Properties")) ;
  gtk_window_set_resizable (GTK_WINDOW (dialog->dlg), TRUE) ;

  tblMain = gtk_table_new (2, 1, FALSE) ;
  gtk_widget_show (tblMain) ;
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog->dlg)->vbox), tblMain, TRUE, FALSE, FALSE) ;
  gtk_container_set_border_width (GTK_CONTAINER (tblMain), 2) ;

  tbl = gtk_table_new (3, 2, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_table_attach (GTK_TABLE (tblMain), tbl, 0, 1, 0, 1,
    (GtkAttachOptions)0,
    (GtkAttachOptions)0, 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;

  DBG_LPD_PROPS (fprintf (stderr, "create_layer_properties_dialog:Continuing...\n")) ;

  lbl = gtk_label_new (_("Layer Type:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  dialog->cbLayerType = gtk_combo_new () ;
  gtk_widget_show (dialog->cbLayerType) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->cbLayerType, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_combo_set_value_in_list (GTK_COMBO (dialog->cbLayerType), TRUE, FALSE) ;
  gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (dialog->cbLayerType)->entry), FALSE) ;

  DBG_LPD_PROPS (fprintf (stderr, "create_layer_properties_dialog:Continuing...\n")) ;

  lstComboItems = NULL ;
  for (Nix = 0 ; Nix < LAYER_TYPE_LAST_TYPE ; Nix++)
    {
    item = gtk_list_item_new () ;
    gtk_widget_show (item) ;

    hbox = gtk_hbox_new (FALSE, 2) ;
    gtk_widget_show (hbox) ;
    gtk_container_add (GTK_CONTAINER (item), hbox) ;
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2) ;

    img = gtk_image_new_from_stock (layer_pixmap_stock_id[Nix], GTK_ICON_SIZE_MENU) ;
    gtk_widget_show (img) ;
    gtk_box_pack_start (GTK_BOX (hbox), img, FALSE, TRUE, 0) ;

    lbl = gtk_label_new (pszLayerTypes[Nix]) ;
    gtk_widget_show (lbl) ;
    gtk_box_pack_start (GTK_BOX (hbox), lbl, TRUE, TRUE, 0) ;
    gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT) ;
    gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5) ;

    lstComboItems = NULL ;
    lstComboItems = g_list_append (lstComboItems, item) ;

    gtk_list_append_items (GTK_LIST (GTK_COMBO (dialog->cbLayerType)->list), lstComboItems) ;

    gtk_combo_set_item_string (GTK_COMBO (dialog->cbLayerType), GTK_ITEM (item), pszLayerTypes[Nix]) ;

    g_signal_connect (G_OBJECT (item), "select", (GCallback)reflect_layer_type, dialog) ;
    }

  DBG_LPD_PROPS (fprintf (stderr, "create_layer_properties_dialog:Continuing...\n")) ;

  lbl = gtk_label_new (_("Layer Status:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  dialog->cbLayerStatus = gtk_combo_new () ;
  gtk_widget_show (dialog->cbLayerStatus) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->cbLayerStatus, 1, 2, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_combo_set_value_in_list (GTK_COMBO (dialog->cbLayerStatus), TRUE, FALSE) ;
  gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (dialog->cbLayerStatus)->entry), FALSE) ;
  lstComboItems = NULL ;
  for (Nix = 0 ; Nix < LAYER_STATUS_LAST_STATUS ; Nix++)
    lstComboItems = g_list_append (lstComboItems, pszLayerStati[Nix]) ;
  gtk_combo_set_popdown_strings (GTK_COMBO (dialog->cbLayerStatus), lstComboItems) ;

  lbl = gtk_label_new (_("Layer Description:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 2, 3,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  dialog->txtLayerDescription = gtk_entry_new () ;
  gtk_widget_show (dialog->txtLayerDescription) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->txtLayerDescription, 1, 2, 2, 3,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->txtLayerDescription), TRUE) ;

  frm = gtk_frame_new (_("Object Properties")) ;
  gtk_widget_show (frm) ;
  gtk_table_attach (GTK_TABLE (tblMain), frm, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (frm), 2) ;

  tbl = gtk_table_new (1, 2, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_container_add (GTK_CONTAINER (frm), tbl) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;

  dialog->swObjs = gtk_scrolled_window_new (
    GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1, 1, 1, 1)),
    GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1, 1, 1, 1))) ;
  gtk_widget_show (dialog->swObjs) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->swObjs, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (dialog->swObjs), 2) ;
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (dialog->swObjs), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC) ;

  dialog->lstObjs = gtk_list_new () ;
  gtk_widget_show (dialog->lstObjs) ;
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (dialog->swObjs), dialog->lstObjs) ;

  dialog->tblObjUI = gtk_table_new (1, 1, FALSE) ;
  gtk_widget_show (dialog->tblObjUI) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->tblObjUI, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;

  dialog->htObjUI = g_hash_table_new (NULL, NULL) ;
  dialog->htObjCB = g_hash_table_new (NULL, NULL) ;
  dialog->htObjCBData = g_hash_table_new (NULL, NULL) ;

  // Fill the hash tables with the UIs and callbacks, respectively, for each of the classes

  types = g_type_children (QCAD_TYPE_DESIGN_OBJECT, &icChildren) ;

  lstComboItems = NULL ;

  for (Nix = 0 ; Nix < icChildren && 0 != types[Nix] ; Nix++)
    {
    cbObj = NULL ;
    objUI = NULL ;
    if (NULL != (klass = QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (types[Nix]))))
      {
      // FIXME: Pass current layer-stored properties to this function, so the UI may properly init itself
      cbObj = qcad_design_object_class_get_properties_ui (klass, NULL, &objUI, &data) ;
      if (!(NULL == cbObj || NULL == objUI))
        {
        g_hash_table_insert (dialog->htObjUI, (gpointer)types[Nix], objUI) ;
        g_hash_table_insert (dialog->htObjCB, (gpointer)types[Nix], (gpointer)cbObj) ;
        g_hash_table_insert (dialog->htObjCBData, (gpointer)types[Nix], data) ;

        gtk_table_attach (GTK_TABLE (dialog->tblObjUI), objUI, 0, 1, idx, idx + 1,
          (GtkAttachOptions)0,
          (GtkAttachOptions)0, 2, 2) ;
        idx++ ;
        }
      }
    }

  g_free (types) ;

  gtk_dialog_add_button (GTK_DIALOG (dialog->dlg), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->dlg), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->dlg), GTK_RESPONSE_OK) ;
  }

static void reflect_layer_type (GtkWidget *widget, gpointer data)
  {
  GtkWidget *li = NULL ;
  layer_properties_D *dialog = (layer_properties_D *)data ;
  char *pszLayerType = gtk_editable_get_chars (GTK_EDITABLE (GTK_COMBO (dialog->cbLayerType)->entry), 0, -1) ;
  const char *pszObjName = NULL ;
  LayerType layer_type = layer_type_from_description (pszLayerType) ;
  GList *llObjs = g_hash_table_lookup (qcad_layer_object_containment_rules (), (gpointer)layer_type),
        *llItr = NULL, *llLstItems = NULL ;

  // Add the appropriate entries to the object list combo
  gtk_list_clear_items (GTK_LIST (dialog->lstObjs), 0, -1) ;

  g_hash_table_foreach (dialog->htObjUI, hide_object_UIs, NULL) ;

  for (llItr = llObjs ; llItr != NULL ; llItr = llItr->next)
    if (NULL != (pszObjName = g_type_name ((GType)(llItr->data))))
      {
      li = gtk_list_item_new_with_label (pszObjName) ;
      gtk_widget_show (li) ;
      llLstItems = g_list_prepend (llLstItems, li) ;
      g_signal_connect (GTK_LIST_ITEM (li), "select", (GCallback)object_type_selected, g_hash_table_lookup (dialog->htObjUI, (gpointer)llItr->data)) ;
      }

  if (NULL != llLstItems)
    {
    gtk_list_append_items (GTK_LIST (dialog->lstObjs), llLstItems) ;
    gtk_list_item_select (GTK_LIST_ITEM (llLstItems->data)) ;
    }

  scrolled_window_set_size (GTK_SCROLLED_WINDOW (dialog->swObjs), dialog->lstObjs, 0.4, 0.4) ;

  g_free (pszLayerType) ;
  }

static void object_type_selected (GtkWidget *widget, gpointer data)
  {
  if (NULL != data)
    gtk_widget_show (GTK_WIDGET (data)) ;
  }

static void set_layer_type (GtkWindow *wnd, QCADLayer *layer, LayerType layer_type)
  {
  // Eventually, before changing the layer type, validate the contents of the linked list of
  // objects contained inside the layer.  If there's a violation (there exist objects in the
  // current layer type not allowed in the desired layer), the offending objects must be
  // destroyed.  Here's the place to ask
  layer->type = layer_type ;
  }

static void hide_object_UIs (gpointer key, gpointer value, gpointer data)
  {gtk_widget_hide (GTK_WIDGET (value)) ;}
