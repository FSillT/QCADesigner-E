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
// Header for QCADDesignObject: The base class for all  //
// the design objects. This class provides printing,    //
// extents calculation, drawing, moving, pretty much    //
// everything for a selectable, printable, design       //
// object.                                              //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADDesignObject_H_
#define _OBJECTS_QCADDesignObject_H_

#include <stdio.h>
#include <stdarg.h>
#ifdef GTK_GUI
  #include <gtk/gtk.h>
#else
  #include "../gdk_structs.h"
#endif
#include "mouse_handler_struct.h"
#include "object_helpers.h"
#ifdef UNDO_REDO
  #include "QCADUndoEntry.h"
#endif /* def UNDO_REDO */
#include "../exp_array.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
  {
  GObject parent_instance ;

  /* public */

  /* private */
  gdouble x, y ;
  gboolean bSelected ;
  GdkColor clr ;
  WorldRectangle bounding_box ;
  } QCADDesignObject ;

typedef enum
  {
  SELECTION_CONTAINMENT,
  SELECTION_INTERSECTION
  } QCADSelectionMethod ;

typedef struct QCADDesignObjectClass
  {
  /* polymorphic behaviour */
  GObjectClass parent_class ;
#ifdef STDIO_FILEIO
  void (*serialize) (QCADDesignObject *obj, FILE *fp) ;
  gboolean (*unserialize) (QCADDesignObject *obj, FILE *fp) ;
#endif /* def STDIO_FILEIO */
  const char *(*PostScript_preamble) () ;
  char *(*PostScript_instance) (QCADDesignObject *obj, gboolean bColour) ;
  GList *(*add_unique_types) (QCADDesignObject *obj, GList *lst) ;
  void (*copy) (QCADDesignObject *objSrc, QCADDesignObject *objDst) ;
  void (*get_bounds_box) (QCADDesignObject *obj, WorldRectangle *rcWorld) ;
  gboolean (*set_selected) (QCADDesignObject *obj, gboolean bSelected) ;
  void (*move) (QCADDesignObject *obj, double dxDelta, double dyDelta) ;
  QCADDesignObject *(*hit_test) (QCADDesignObject *obj, int xReal, int yReal) ;
  gboolean (*select_test) (QCADDesignObject *obj, WorldRectangle *rc, QCADSelectionMethod method) ;
#ifdef GTK_GUI
  void (*draw) (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop) ;
  GCallback (*default_properties_ui) (struct QCADDesignObjectClass *klass, void *default_options, GtkWidget **pTopContainer, gpointer *pData) ;
#ifdef UNDO_REDO
  gboolean (*properties) (QCADDesignObject *obj, GtkWidget *parent, QCADUndoEntry **pentry) ;
#else
  gboolean (*properties) (QCADDesignObject *obj, GtkWidget *parent) ;
#endif /* def UNDO_REDO */
#endif /* def GTK_GUI */
  void *(*default_properties_get) (struct QCADDesignObjectClass *klass) ;
  void (*default_properties_set) (struct QCADDesignObjectClass *klass, void *props) ;
  void (*default_properties_destroy) (struct QCADDesignObjectClass *klass, void *props) ;
  void (*transform) (QCADDesignObject *obj, double m11, double m12, double m21, double m22) ;

  /* signal handlers */
  void (*selected) (GObject *obj, gpointer data) ;

  GdkColor clrSelected ;
  GdkColor clrDefault ;
  MOUSE_HANDLERS mh ;
  } QCADDesignObjectClass ;

GType qcad_design_object_get_type () ;

#define QCAD_TYPE_STRING_DESIGN_OBJECT "QCADDesignObject"
#define QCAD_TYPE_DESIGN_OBJECT (qcad_design_object_get_type ())
#define QCAD_DESIGN_OBJECT(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_DESIGN_OBJECT, QCADDesignObject))
#define QCAD_DESIGN_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_DESIGN_OBJECT, QCADDesignObjectClass))
#define QCAD_IS_DESIGN_OBJECT(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_DESIGN_OBJECT))
#define QCAD_IS_DESIGN_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_DESIGN_OBJECT))
#define QCAD_DESIGN_OBJECT_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_DESIGN_OBJECT, QCADDesignObjectClass))

///////////////////////////////////////////////////////////////////////////////

#ifdef STDIO_FILEIO
void qcad_design_object_serialize (QCADDesignObject *obj, FILE *fp) ;
QCADDesignObject *qcad_design_object_new_from_stream (FILE *fp) ;
#endif /* def STDIO_FILEIO */
QCADDesignObject *qcad_design_object_new_from_object (QCADDesignObject *src) ;
void qcad_design_object_move (QCADDesignObject *obj, double dxDelta, double dyDelta) ;
void qcad_design_object_move_to (QCADDesignObject *obj, double xWorld, double yWorld) ;
void qcad_design_object_get_bounds_box (QCADDesignObject *obj, WorldRectangle *rcWorld) ;
gboolean qcad_design_object_set_selected (QCADDesignObject *obj, gboolean bSelected) ;
gboolean qcad_design_object_overlaps (QCADDesignObject *obj1, QCADDesignObject *obj2) ;
gboolean qcad_design_object_select_test (QCADDesignObject *obj, WorldRectangle *rc, QCADSelectionMethod method) ;
QCADDesignObject *qcad_design_object_hit_test (QCADDesignObject *obj, int x, int y) ;
#ifdef GTK_GUI
void qcad_design_object_draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop) ;
GCallback qcad_design_object_class_get_properties_ui (QCADDesignObjectClass *klass, void *default_propeties, GtkWidget **pTopContainer, gpointer *pData) ;
#ifdef UNDO_REDO
gboolean qcad_design_object_get_properties (QCADDesignObject *obj, GtkWidget *parent, QCADUndoEntry **pentry) ;
#else
gboolean qcad_design_object_get_properties (QCADDesignObject *obj, GtkWidget *parent) ;
#endif /* def UNDO_REDO */
#endif /* def GTK_GUI */
void *qcad_design_object_class_get_properties (QCADDesignObjectClass *klass) ;
void qcad_design_object_class_set_properties (QCADDesignObjectClass *klass, void *props) ;
void qcad_design_object_class_destroy_properties (QCADDesignObjectClass *klass, void *props) ;
const char *qcad_design_object_get_PostScript_preamble (QCADDesignObject *obj) ;
char *qcad_design_object_get_PostScript_instance (QCADDesignObject *obj, gboolean bColour) ;
GList *qcad_design_object_add_types (QCADDesignObject *obj, GList *lst) ;
void qcad_design_object_transform (QCADDesignObject *obj, double m11, double m12, double m21, double m22) ;

EXP_ARRAY *qcad_design_object_get_state_array (QCADDesignObject *obj, ...) ;
void qcad_design_object_state_array_free (EXP_ARRAY *state_array) ;
#ifdef UNDO_REDO
QCADUndoEntry *qcad_design_object_get_state_undo_entry (QCADDesignObject *obj, EXP_ARRAY *state_before, EXP_ARRAY *state_after) ;
#endif /* def UNDO_REDO */

#ifdef __cplusplus
}
#endif
#endif /* _OBJECTS_QCADDesignObject_H_ */
