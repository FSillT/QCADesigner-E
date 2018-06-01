#include <gtk/gtk.h>
#include "../support.h"
#include "../custom_widgets.h"
#include "QCADCellRendererLayerList.h"

G_BEGIN_DECLS
extern void g_cclosure_user_marshal_VOID__STRING_POINTER (GClosure     *closure,
                                                          GValue       *return_value,
                                                          guint         n_param_values,
                                                          const GValue *param_values,
                                                          gpointer      invocation_hint,
                                                          gpointer      marshal_data);

G_END_DECLS

enum
  {
  QCADCRLL_PROP_DESIGN = 1,
  QCADCRLL_PROP_TEMPLATE,
  QCADCRLL_PROP_LAYER
  } ;

enum
  {
  QCADCRLL_SIGNAL_LAYER_CHANGED,
  QCADCRLL_SIGNAL_LAST
  } ;

static void qcad_cell_renderer_layer_list_class_init (QCADCellRendererLayerListClass *klass) ;
static void qcad_cell_renderer_layer_list_instance_init (QCADCellRendererLayerList *instance) ;
static void qcad_cell_renderer_layer_list_get_property (GObject *object, guint param_id, GValue *value, GParamSpec *pspec) ;
static void qcad_cell_renderer_layer_list_set_property (GObject *object, guint param_id, const GValue *value, GParamSpec *pspec) ;
static GtkCellEditable *qcad_cell_renderer_layer_list_start_editing (GtkCellRenderer *cell, GdkEvent *event, GtkWidget *widget, const gchar *path, GdkRectangle *background_area, GdkRectangle *cell_area, GtkCellRendererState flags) ;
static void qcad_layer_list_editable_select_layer (QCADLayerListEditable *lle, QCADLayer *layer) ;
static void qcad_cell_renderer_layer_list_editing_done (GtkCellEditable *ce, gpointer data) ;

static guint qcad_cell_renderer_layer_list_signals[QCADCRLL_SIGNAL_LAST] = {0} ;

GType qcad_cell_renderer_layer_list_get_type ()
  {
  static GType cell_renderer_layer_list_type = 0 ;

  if (0 == cell_renderer_layer_list_type)
    {
    static GTypeInfo cell_renderer_layer_list_info =
      {
      sizeof (QCADCellRendererLayerListClass),
      NULL,
      NULL,
      (GClassInitFunc)qcad_cell_renderer_layer_list_class_init,
      NULL,
      NULL,
      sizeof (QCADCellRendererLayerList),
      0,
      (GInstanceInitFunc)qcad_cell_renderer_layer_list_instance_init
      } ;
    if ((cell_renderer_layer_list_type = g_type_register_static (GTK_TYPE_CELL_RENDERER_TEXT, QCAD_TYPE_STRING_CELL_RENDERER_LAYER_LIST, &cell_renderer_layer_list_info, 0)))
      g_type_class_ref (cell_renderer_layer_list_type) ;
    }

  return cell_renderer_layer_list_type ;
  }

static void qcad_cell_renderer_layer_list_class_init (QCADCellRendererLayerListClass *klass)
  {
  GObjectClass *object_klass = G_OBJECT_CLASS (klass) ;
  GtkCellRendererClass *cr_klass = GTK_CELL_RENDERER_CLASS (klass) ;

  object_klass->get_property = qcad_cell_renderer_layer_list_get_property ;
  object_klass->set_property = qcad_cell_renderer_layer_list_set_property ;
  cr_klass->start_editing    = qcad_cell_renderer_layer_list_start_editing ;

  g_object_class_install_property (object_klass, QCADCRLL_PROP_DESIGN,
    g_param_spec_pointer ("design", _("Design"), _("The design"), G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (object_klass, QCADCRLL_PROP_LAYER,
    g_param_spec_pointer ("layer", _("Layer"), _("The layer"), G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (object_klass, QCADCRLL_PROP_TEMPLATE,
    g_param_spec_pointer ("template", _("Template"), _("Template for new layer"), G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  qcad_cell_renderer_layer_list_signals[QCADCRLL_SIGNAL_LAYER_CHANGED] =
    g_signal_new ("layer-changed", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (QCADCellRendererLayerListClass, layer_changed), NULL, NULL, 
        g_cclosure_user_marshal_VOID__STRING_POINTER, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_POINTER) ;
  }

static void qcad_cell_renderer_layer_list_instance_init (QCADCellRendererLayerList *instance)
  {
  instance->layer    = NULL ;
  instance->design   = NULL ;
  instance->template = NULL ;
  }

static void qcad_cell_renderer_layer_list_get_property (GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
  {
  switch (param_id)
    {
    case QCADCRLL_PROP_DESIGN:
      g_value_set_pointer (value, QCAD_CELL_RENDERER_LAYER_LIST (object)->design) ;
      break ;

    case QCADCRLL_PROP_LAYER:
      g_value_set_pointer (value, QCAD_CELL_RENDERER_LAYER_LIST (object)->layer) ;
      break ;

    case QCADCRLL_PROP_TEMPLATE:
      g_value_set_pointer (value, QCAD_CELL_RENDERER_LAYER_LIST (object)->template) ;
      break ;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
  }

static void qcad_cell_renderer_layer_list_set_property (GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
  {
  switch (param_id)
    {
    case QCADCRLL_PROP_DESIGN:
      QCAD_CELL_RENDERER_LAYER_LIST (object)->design = g_value_get_pointer (value) ;
      g_object_notify (object, "design") ;
      break ;

    case QCADCRLL_PROP_TEMPLATE:
      QCAD_CELL_RENDERER_LAYER_LIST (object)->template = g_value_get_pointer (value) ;
      g_object_notify (object, "template") ;
      break ;

    case QCADCRLL_PROP_LAYER:
      {
      char *psz = NULL ;
      QCADLayer *layer = NULL ;

      QCAD_CELL_RENDERER_LAYER_LIST (object)->layer = (layer = g_value_get_pointer (value)) ;
      g_object_notify (object, "layer") ;
      g_object_set (object, "text", psz = (NULL == layer ? _("Create New...") : layer->pszDescription), NULL) ;
      break ;
      }
    }
  }

static GtkCellEditable *qcad_cell_renderer_layer_list_start_editing (GtkCellRenderer *cell, GdkEvent *event, GtkWidget *widget, const gchar *path, GdkRectangle *background_area, GdkRectangle *cell_area, GtkCellRendererState flags)
  {
  QCADLayerListEditable *lle = NULL ;
  QCADCellRendererLayerList *qcadcrll = QCAD_CELL_RENDERER_LAYER_LIST (cell) ;

#if (GTK_MINOR_VERSION >= 6)
  if (!cell->sensitive) return NULL ;
#endif

  if (NULL == qcadcrll->template) return NULL ;

  lle = QCAD_LAYER_LIST_EDITABLE (qcad_layer_list_editable_new (qcadcrll->design, qcadcrll->template->type)) ;

  g_object_set_data_full (G_OBJECT (lle), "path", g_strdup (path), g_free) ;
  g_signal_connect (G_OBJECT (lle), "editing-done", (GCallback)qcad_cell_renderer_layer_list_editing_done, cell) ;

  qcad_layer_list_editable_select_layer (lle, qcadcrll->layer) ;

  gtk_widget_show (GTK_WIDGET (lle)) ;

  return GTK_CELL_EDITABLE (lle) ;
  }

static void qcad_cell_renderer_layer_list_editing_done (GtkCellEditable *ce, gpointer data)
  {
  if (!(QCAD_LAYER_LIST_EDITABLE (ce)->editing_cancelled))
    g_signal_emit (data, qcad_cell_renderer_layer_list_signals[QCADCRLL_SIGNAL_LAYER_CHANGED], 0, 
      g_object_get_data (G_OBJECT (ce), "path"), g_object_get_data (G_OBJECT (ce), "layer")) ;
  }

GtkCellRenderer *qcad_cell_renderer_layer_list_new ()
  {return GTK_CELL_RENDERER (g_object_new (QCAD_TYPE_CELL_RENDERER_LAYER_LIST, NULL)) ;}

static void gtk_cell_editable_interface_init (gpointer iface, gpointer iface_data) ;
static void qcad_layer_list_editable_instance_init (QCADLayerListEditable *qcadlle) ;
static void qcad_layer_list_editable_start_editing (GtkCellEditable *ce, GdkEvent *event) ;
static void qcadlle_mnui_activate (GtkWidget *widget, gpointer data) ;
static void qcadlle_mnu_hide (GtkWidget *widget, gpointer data) ;
static void place_popup_menu (GtkMenu *menu, int *x, int *y, gboolean *push_in, gpointer data) ;

GType qcad_layer_list_editable_get_type ()
  {
  static GType qcad_layer_list_editable_type = 0 ;

  if (0 == qcad_layer_list_editable_type)
    {
    static GTypeInfo qcad_layer_list_editable_info =
      {
      sizeof (QCADLayerListEditableClass),
      NULL,
      NULL,
      (GClassInitFunc)NULL,
      NULL,
      NULL,
      sizeof (QCADLayerListEditable),
      0,
      (GInstanceInitFunc)qcad_layer_list_editable_instance_init
      } ;

    static GInterfaceInfo gtk_cell_editable_info =
      {
      (GInterfaceInitFunc)gtk_cell_editable_interface_init,
      NULL,
      NULL
      } ;

    if ((qcad_layer_list_editable_type = g_type_register_static (GTK_TYPE_EVENT_BOX, QCAD_TYPE_STRING_LAYER_LIST_EDITABLE, &qcad_layer_list_editable_info, 0)))
      {
      g_type_add_interface_static (qcad_layer_list_editable_type, GTK_TYPE_CELL_EDITABLE, &gtk_cell_editable_info) ;
      g_type_class_ref (qcad_layer_list_editable_type) ;
      }
    }

  return qcad_layer_list_editable_type ;
  }

static void gtk_cell_editable_interface_init (gpointer iface, gpointer iface_data)
  {
  GtkCellEditableIface *ce_klass = (GtkCellEditableIface *)iface ;

  ce_klass->start_editing = qcad_layer_list_editable_start_editing ;
  }

static void qcad_layer_list_editable_instance_init (QCADLayerListEditable *qcadlle)
  {qcadlle->editing_cancelled = TRUE ;}

static void qcad_layer_list_editable_start_editing (GtkCellEditable *ce, GdkEvent *event)
  {
  GtkWidget *widget = GTK_WIDGET (ce) ;
  GdkPoint pt ;
  GtkMenu *mnu = g_object_get_data (G_OBJECT (widget), "mnu") ;

  // Wait for size_allocate to complete
  while (-1 == widget->allocation.x || -1 == widget->allocation.y)
    gtk_main_iteration () ;

  if (NULL == mnu) return ;

  gtk_widget_get_root_origin (widget, &(pt.x), &(pt.y)) ;

  gtk_menu_popup (mnu, NULL, NULL, (GtkMenuPositionFunc)place_popup_menu, &pt, 1, gtk_get_current_event_time ()) ;
  }

static void qcadlle_mnu_hide (GtkWidget *widget, gpointer data)
  {
  gtk_cell_editable_editing_done (GTK_CELL_EDITABLE (data)) ;
  gtk_cell_editable_remove_widget (GTK_CELL_EDITABLE (data)) ;
  }

static void qcadlle_mnui_activate (GtkWidget *widget, gpointer data)
  {
  QCAD_LAYER_LIST_EDITABLE (data)->editing_cancelled = FALSE ;
  g_object_set_data (G_OBJECT (data), "layer", g_object_get_data (G_OBJECT (widget), "layer")) ;
  g_signal_handlers_block_matched (g_object_get_data (G_OBJECT (data), "mnu"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, qcadlle_mnu_hide, data) ;
  gtk_cell_editable_editing_done (GTK_CELL_EDITABLE (data)) ;
  // Why does the editable get destroyed in the process of editing_done ?
  //gtk_cell_editable_remove_widget (GTK_CELL_EDITABLE (data)) ;
  }

GtkCellEditable *qcad_layer_list_editable_new (DESIGN *design, int layer_type)
  {
  GtkWidget *mnu = NULL, *mnui = NULL ;
  GList *llItr = NULL ;
  QCADLayerListEditable *lle = NULL ;

  lle = g_object_new (QCAD_TYPE_LAYER_LIST_EDITABLE, "above-child", TRUE, "visible-window", FALSE, NULL) ;
  GTK_WIDGET_UNSET_FLAGS (GTK_WIDGET (lle), GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;

  mnu = gtk_menu_new () ;
  gtk_widget_show (mnu) ;

  mnui = gtk_menu_item_new_with_label (_("Create New...")) ;
  gtk_widget_show (mnui) ;
  gtk_container_add (GTK_CONTAINER (mnu), mnui) ;
  g_signal_connect (G_OBJECT (mnui), "activate", (GCallback)qcadlle_mnui_activate, lle) ;

  for (llItr = design->lstLayers ; llItr != NULL ; llItr = llItr->next)
    if (QCAD_LAYER (llItr->data)->type == layer_type)
      {
      mnui = gtk_menu_item_new_with_label (NULL == QCAD_LAYER (llItr->data)->pszDescription ? _("Untitled Layer") : QCAD_LAYER (llItr->data)->pszDescription) ;
      gtk_widget_show (mnui) ;
      gtk_container_add (GTK_CONTAINER (mnu), mnui) ;
      g_object_set_data (G_OBJECT (mnui), "layer", llItr->data) ;
      g_signal_connect (G_OBJECT (mnui), "activate", (GCallback)qcadlle_mnui_activate, lle) ;
      }

  g_object_set_data (G_OBJECT (lle), "mnu", mnu) ;

  g_signal_connect (G_OBJECT (mnu), "selection-done",    (GCallback)qcadlle_mnu_hide, lle) ;

  return GTK_CELL_EDITABLE (lle) ;
  }

static void place_popup_menu (GtkMenu *menu, int *x, int *y, gboolean *push_in, gpointer data)
  {
  (*x) = ((GdkPoint *)data)->x ;
  (*y) = ((GdkPoint *)data)->y ;
  (*push_in) = TRUE ;
  }

static void qcad_layer_list_editable_select_layer (QCADLayerListEditable *lle, QCADLayer *layer)
  {
  GtkWidget *mnu = NULL ;
  GList *llMnuis = NULL, *llItr = NULL ;

  if (NULL == lle) return ;

  if (NULL == (mnu = g_object_get_data (G_OBJECT (lle), "mnu"))) return ;

  for (llMnuis = llItr = gtk_container_get_children (GTK_CONTAINER (mnu)) ; llItr != NULL ; llItr = llItr->next)
    {
    if (g_object_get_data (G_OBJECT (llItr->data), "layer") == layer)
      {
      g_signal_handlers_block_matched (llItr->data, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, qcadlle_mnui_activate, lle) ;
      gtk_menu_item_activate (GTK_MENU_ITEM (llItr->data)) ;
      g_signal_handlers_unblock_matched (llItr->data, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, qcadlle_mnui_activate, lle) ;
      }
    }
  if (NULL != llMnuis)
    g_list_free (llMnuis) ;
  }

void
g_cclosure_user_marshal_VOID__STRING_POINTER (GClosure     *closure,
                                              GValue       *return_value,
                                              guint         n_param_values,
                                              const GValue *param_values,
                                              gpointer      invocation_hint,
                                              gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__STRING_POINTER) (gpointer     data1,
                                                     gpointer     arg_1,
                                                     gpointer     arg_2,
                                                     gpointer     data2);
  register GMarshalFunc_VOID__STRING_POINTER callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_get_object (param_values + 0);
    }
  else
    {
      data1 = g_value_get_object (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__STRING_POINTER) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            (char *)g_value_get_string (param_values + 1),
            g_value_get_pointer (param_values + 2),
            data2);
}
