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
// Header to QCADLabel: A simple text label to          //
// accompany cells or to stand on its own in a drawing  //
// layer.                                               //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADLabel_H_
#define _OBJECTS_QCADLabel_H_

#include <stdarg.h>
#include <glib-object.h>
#include "../gdk_structs.h"
#ifdef GTK_GUI
  #include "../exp_pixmap.h"
#endif /* def GTK_GUI */
#include "QCADStretchyObject.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
  {
  QCADStretchyObject parent_instance ;
  char *psz ;
  gboolean bShrinkWrap ;
  gboolean bNeedsEPMDraw ;
#ifdef GTK_GUI
  EXP_PIXMAP *epm ;
#endif /* def GTK_GUI */
  } QCADLabel ;

typedef struct
  {
  /* public */
  QCADStretchyObjectClass parent_class ;
  } QCADLabelClass ;

GType qcad_label_get_type () ;

QCADLabel *qcad_label_new (char *psz, ...) ;
QCADLabel *qcad_label_vnew (char *psz, va_list va) ;
void qcad_label_shrinkwrap (QCADLabel *lbl) ;
void qcad_label_set_text (QCADLabel *label, char *psz, ...) ;
void qcad_label_vset_text (QCADLabel *label, char *psz, va_list va) ;

#define QCAD_TYPE_STRING_LABEL "QCADLabel"
#define QCAD_TYPE_LABEL (qcad_label_get_type ())
#define QCAD_LABEL(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_LABEL, QCADLabel))
#define QCAD_LABEL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_LABEL, QCADLabelClass))
#define IS_QCAD_LABEL(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_LABEL))
#define IS_QCAD_LABEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_LABEL))
#define QCAD_LABEL_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_LABEL, QCADLabelClass))

#ifdef __cplusplus
}
#endif
#endif /* _OBJECTS_QCADLabel_H_ */
