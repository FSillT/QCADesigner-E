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
// Header for the clock combo. This is really a GtkHBox //
// containing a GtkOptionMenu. However, its "changed"   //
// signal gets emmitted even when the user re-selects   //
// the same menu item as was already selected.          //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADClockCombo_H_
#define _OBJECTS_QCADClockCombo_H_

#include <glib-object.h>
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
  {
  GtkHBox parent_instance ;
  GtkWidget *widget ;
  } QCADClockCombo ;

typedef struct
  {
  /* public */
  GtkHBoxClass parent_class ;

  /* signals */
  void (*changed) (GtkWidget *widget, gpointer data) ;
  } QCADClockComboClass ;

GType qcad_clock_combo_get_type () ;

#define QCAD_TYPE_STRING_CLOCK_COMBO "QCADClockCombo"
#define QCAD_TYPE_CLOCK_COMBO (qcad_clock_combo_get_type ())
#define QCAD_CLOCK_COMBO(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_CLOCK_COMBO, QCADClockCombo))
#define QCAD_CLOCK_COMBO_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_CLOCK_COMBO, QCADClockComboClass))
#define IS_QCAD_CLOCK_COMBO(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_CLOCK_COMBO))
#define IS_QCAD_CLOCK_COMBO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_CLOCK_COMBO))
#define QCAD_CLOCK_COMBO_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_CLOCK_COMBO, QCADClockComboClass))

GtkWidget *qcad_clock_combo_new () ;
int qcad_clock_combo_get_clock (QCADClockCombo *qcc) ;
void qcad_clock_combo_set_clock (QCADClockCombo *qcc, int clock) ;

#ifdef __cplusplus
}
#endif
#endif /* _OBJECTS_QCADStretchyObject_H_ */
