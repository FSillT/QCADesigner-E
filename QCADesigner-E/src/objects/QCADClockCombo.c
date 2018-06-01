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
// The clock combo. This is really a GtkHBox containing //
// a GtkOptionMenu. However, its "changed" signal gets  //
// emmitted even when the user re-selects the same menu //
// item as was already selected.                        //
//                                                      //
//////////////////////////////////////////////////////////

#include <string.h>
#include <glib-object.h>
#include <gdk/gdk.h>
#include "objects_debug.h"
#include "support.h"
#include "QCADClockCombo.h"

static void qcad_clock_combo_class_init (GObjectClass *klass, gpointer data) ;
static void qcad_clock_combo_instance_init (GObject *object, gpointer data) ;
static void qcad_clock_combo_item_activate (GtkWidget *widget, gpointer data) ;

enum
  {
  QCAD_CLOCK_COMBO_CHANGED_SIGNAL,
  QCAD_CLOCK_COMBO_LAST_SIGNAL
  } ;

static guint qcad_clock_combo_signals[QCAD_CLOCK_COMBO_LAST_SIGNAL] = {0} ;

GType qcad_clock_combo_get_type ()
  {
  static GType qcad_clock_combo_type = 0 ;

  if (!qcad_clock_combo_type)
    {
    static const GTypeInfo qcad_clock_combo_info =
      {
      sizeof (QCADClockComboClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_clock_combo_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADClockCombo),
      0,
      (GInstanceInitFunc)qcad_clock_combo_instance_init
      } ;

    if ((qcad_clock_combo_type = g_type_register_static (GTK_TYPE_HBOX, QCAD_TYPE_STRING_CLOCK_COMBO, &qcad_clock_combo_info, 0)))
      g_type_class_ref (qcad_clock_combo_type) ;
    DBG_OO (fprintf (stderr, "Registered QCADClockCombo as %d\n", (int)qcad_stretchy_object_type)) ;
    }
  return qcad_clock_combo_type ;
  }

static void qcad_clock_combo_class_init (GObjectClass *klass, gpointer data)
  {
  DBG_OO (fprintf (stderr, "QCADClockCombo::class_init:Entering\n")) ;
  qcad_clock_combo_signals[QCAD_CLOCK_COMBO_CHANGED_SIGNAL] =
    g_signal_new ("changed", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (QCADClockComboClass, changed), NULL, NULL, g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0) ;
  DBG_OO (fprintf (stderr, "QCADClockCombo::class_init:Leaving\n")) ;
  }

static void qcad_clock_combo_instance_init (GObject *object, gpointer data)
  {
  GtkWidget *item = NULL, *mnu = NULL ;
  QCADClockCombo *qcc = QCAD_CLOCK_COMBO (object) ;
  DBG_OO (fprintf (stderr, "QCADClockCombo::instance_init:Entering\n")) ;

  qcc->widget = gtk_option_menu_new () ;
  gtk_box_pack_start (GTK_BOX (object), qcc->widget, FALSE, TRUE, 0) ;
  gtk_widget_show (qcc->widget) ;
  g_object_set_data (G_OBJECT (qcc->widget), "clock", (gpointer)-1) ;

  mnu = gtk_menu_new () ;
  gtk_widget_show (mnu) ;
  gtk_option_menu_set_menu (GTK_OPTION_MENU (qcc->widget), mnu) ;

  item = gtk_menu_item_new_with_label (_("Clock 0")) ;
  gtk_widget_show (item) ;
  gtk_container_add (GTK_CONTAINER (mnu), item) ;
  g_object_set_data (G_OBJECT (item), "clock", (gpointer)0) ;
  g_signal_connect (G_OBJECT (item), "activate", (GCallback)qcad_clock_combo_item_activate, qcc) ;

  item = gtk_menu_item_new_with_label (_("Clock 1")) ;
  gtk_widget_show (item) ;
  gtk_container_add (GTK_CONTAINER (mnu), item) ;
  g_object_set_data (G_OBJECT (item), "clock", (gpointer)1) ;
  g_signal_connect (G_OBJECT (item), "activate", (GCallback)qcad_clock_combo_item_activate, qcc) ;

  item = gtk_menu_item_new_with_label (_("Clock 2")) ;
  gtk_widget_show (item) ;
  gtk_container_add (GTK_CONTAINER (mnu), item) ;
  g_object_set_data (G_OBJECT (item), "clock", (gpointer)2) ;
  g_signal_connect (G_OBJECT (item), "activate", (GCallback)qcad_clock_combo_item_activate, qcc) ;

  item = gtk_menu_item_new_with_label (_("Clock 3")) ;
  gtk_widget_show (item) ;
  gtk_container_add (GTK_CONTAINER (mnu), item) ;
  g_object_set_data (G_OBJECT (item), "clock", (gpointer)3) ;
  g_signal_connect (G_OBJECT (item), "activate", (GCallback)qcad_clock_combo_item_activate, qcc) ;

  gtk_option_menu_set_history (GTK_OPTION_MENU (qcc->widget), 0) ;
  DBG_OO (fprintf (stderr, "QCADClockCombo::instance_init:Leaving\n")) ;
  }

///////////////////////////////////////////////////////////////////////////////

GtkWidget *qcad_clock_combo_new ()
  {return g_object_new (QCAD_TYPE_CLOCK_COMBO, NULL) ;}

int qcad_clock_combo_get_clock (QCADClockCombo *qcc)
  {return (int)g_object_get_data (G_OBJECT (qcc->widget), "clock") ;}

void qcad_clock_combo_set_clock (QCADClockCombo *qcc, int clock)
  {
  g_object_set_data (G_OBJECT (qcc->widget), "clock", (gpointer)(clock = CLAMP (clock, 0, 3))) ;
  gtk_option_menu_set_history (GTK_OPTION_MENU (qcc->widget), clock) ;
  }

///////////////////////////////////////////////////////////////////////////////

static void qcad_clock_combo_item_activate (GtkWidget *widget, gpointer data)
  {
  GtkWidget *ui = NULL ;

  if (NULL == widget || NULL == data) return ;

  if (!IS_QCAD_CLOCK_COMBO (data)) return ;

  ui = QCAD_CLOCK_COMBO (data)->widget ;

  g_object_set_data (G_OBJECT (ui), "clock", g_object_get_data (G_OBJECT (widget), "clock")) ;

  g_signal_emit (G_OBJECT (data), qcad_clock_combo_signals[QCAD_CLOCK_COMBO_CHANGED_SIGNAL], 0) ;
  }
