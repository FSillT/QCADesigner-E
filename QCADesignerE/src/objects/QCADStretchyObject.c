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
// The stretchy object. This is the base class for all  //
// stretchable objects such as labels, substrates, and  //
// rulers.                                              //
//                                                      //
//////////////////////////////////////////////////////////

#include <string.h>
#include <glib-object.h>
#ifdef GTK_GUI
  #include "../exp_pixmap.h"
#endif
#include "../gdk_structs.h"
#include "QCADStretchyObject.h"
#include "QCADRuler.h"
#include "object_helpers.h"
#include "../custom_widgets.h"
#include "../global_consts.h"
#include "../fileio_helpers.h"
#include "objects_debug.h"

static void qcad_stretchy_object_class_init (GObjectClass *klass, gpointer data) ;

#ifdef STDIO_FILEIO
static void serialize (QCADDesignObject *obj, FILE *fp) ;
static gboolean unserialize (QCADDesignObject *obj, FILE *fp) ;
#endif /* def STDIO_FILEIO */
#ifdef GTK_GUI
#ifdef DESIGNER
static gboolean button_pressed (GtkWidget *widget, GdkEventButton *button, gpointer data) ;
static gboolean motion_notify (GtkWidget *widget, GdkEventMotion *button, gpointer data) ;
static gboolean button_released (GtkWidget *widget, GdkEventButton *button, gpointer data) ;
#endif /* def DESIGNER */
#endif /* def GTK_GUI */
static void stretch_draw_state_change (QCADStretchyObject *obj, int x, int y, int xRef, int yRef) ;

#ifdef GTK_GUI
#ifdef DESIGNER
extern GType current_type ;
extern void (*drop_function) (QCADDesignObject *obj) ;
#endif /* def DESIGNER */
#endif /* def GTK_GUI */

GType qcad_stretchy_object_get_type ()
  {
  static GType qcad_stretchy_object_type = 0 ;

  if (!qcad_stretchy_object_type)
    {
    static const GTypeInfo qcad_stretchy_object_info =
      {
      sizeof (QCADStretchyObjectClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_stretchy_object_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADStretchyObject),
      0,
      NULL
      } ;

    if ((qcad_stretchy_object_type = g_type_register_static (QCAD_TYPE_DESIGN_OBJECT, QCAD_TYPE_STRING_STRETCHY_OBJECT, &qcad_stretchy_object_info, 0)))
      g_type_class_ref (qcad_stretchy_object_type) ;
    DBG_OO (fprintf (stderr, "Registered QCADStretchyObject as %d\n", (int)qcad_stretchy_object_type)) ;
    }
  return qcad_stretchy_object_type ;
  }

static void qcad_stretchy_object_class_init (GObjectClass *klass, gpointer data)
  {
  DBG_OO (fprintf (stderr, "QCADStretchyObject::class_init:Entering\n")) ;
  QCAD_STRETCHY_OBJECT_CLASS (klass)->stretch_draw_state_change = stretch_draw_state_change ;
#ifdef STDIO_FILEIO
  QCAD_DESIGN_OBJECT_CLASS (klass)->serialize = serialize ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->unserialize = unserialize ;
#endif /* def STDIO_FILEIO */
#ifdef GTK_GUI
#ifdef DESIGNER
  QCAD_DESIGN_OBJECT_CLASS (klass)->mh.button_pressed = (GCallback)button_pressed ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->mh.motion_notify = (GCallback)motion_notify ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->mh.button_released = (GCallback)button_released ;
#endif /* def DESIGNER */
  QCAD_STRETCHY_OBJECT_CLASS (klass)->epm = NULL ;
#else
  QCAD_DESIGN_OBJECT_CLASS (klass)->mh.button_pressed = NULL ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->mh.motion_notify = NULL ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->mh.button_released = NULL ;
#endif /* def GTK_GUI */
  DBG_OO (fprintf (stderr, "QCADStretchyObject::class_init:Leaving\n")) ;
  }

///////////////////////////////////////////////////////////////////////////////
#ifdef STDIO_FILEIO
static void serialize (QCADDesignObject *obj, FILE *fp)
  {
  /* output object type */
  fprintf(fp, "[TYPE:%s]\n", QCAD_TYPE_STRING_STRETCHY_OBJECT);

  /* call parent serialize function */
  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_STRETCHY_OBJECT)))->serialize (obj, fp) ;

  /* output variables */

  /* output end of object */
  fprintf(fp, "[#TYPE:%s]\n", QCAD_TYPE_STRING_STRETCHY_OBJECT);
  }

static gboolean unserialize (QCADDesignObject *obj, FILE *fp)
  {
  char *pszLine = NULL ;
  gboolean bStopReading = FALSE, bParentInit = FALSE ;

  if (!SkipPast (fp, '\0', "[TYPE:" QCAD_TYPE_STRING_STRETCHY_OBJECT "]", NULL)) return FALSE ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (fp, '\0', TRUE)))
      break ;

    if (!strcmp (pszLine, "[#TYPE:" QCAD_TYPE_STRING_STRETCHY_OBJECT "]"))
      {
      g_free (pszLine) ;
      break ;
      }

    if (!bStopReading)
      {
      if (!strncmp (pszLine, "[TYPE:", 6))
        {
        if (!(bParentInit = QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_STRETCHY_OBJECT)))->unserialize (obj, fp)))
          bStopReading = TRUE ;
        }
      // No other options to make "else if" statements about
      }
    g_free (pszLine) ;
    g_free (ReadLine (fp, '\0', FALSE)) ;
    }

  return bParentInit ;
  }
#endif /* def STDIO_FILEIO */
#ifdef GTK_GUI
#ifdef DESIGNER
static gboolean button_pressed (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  QCADStretchyObjectClass *klass = QCAD_STRETCHY_OBJECT_CLASS (g_type_class_peek (current_type)) ;

  if (1 == event->button)
    {
    GObject *obj = g_object_new (current_type, NULL) ;
    if (!IS_QCAD_STRETCHY_OBJECT (obj))
      {
      g_object_unref (obj) ;
      return FALSE ;
      }
    else
      {
      klass->stretch_draw_state_change (klass->tmpobj = QCAD_STRETCHY_OBJECT (obj), event->x, event->y, klass->xRef = event->x, klass->yRef = event->y) ;
      qcad_design_object_draw (QCAD_DESIGN_OBJECT (klass->tmpobj), widget->window, GDK_XOR) ;
      }
    }
  return FALSE ;
  }

static gboolean motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer data)
  {
  QCADStretchyObjectClass *klass = QCAD_STRETCHY_OBJECT_CLASS (g_type_class_peek (current_type)) ;

  if ((event->state & GDK_BUTTON1_MASK) && (NULL != klass->tmpobj))
    {
    QCADDesignObject *tmpobj = QCAD_DESIGN_OBJECT (klass->tmpobj) ;
    qcad_design_object_draw (tmpobj, widget->window, GDK_XOR) ;
    klass->stretch_draw_state_change (klass->tmpobj, event->x, event->y, klass->xRef, klass->yRef) ;
    qcad_design_object_draw (tmpobj, widget->window, GDK_XOR) ;
    }

  return FALSE ;
  }

static gboolean button_released (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  QCADStretchyObjectClass *klass = QCAD_STRETCHY_OBJECT_CLASS (g_type_class_peek (current_type)) ;
  QCADDesignObject *tmpobj = NULL ;

  if (1 != event->button) return FALSE ;

  if (NULL == klass->tmpobj) return FALSE ;

  tmpobj = QCAD_DESIGN_OBJECT (klass->tmpobj) ;
  qcad_design_object_draw (tmpobj, widget->window, GDK_XOR) ;
  klass->stretch_draw_state_change (klass->tmpobj, event->x, event->y, klass->xRef, klass->yRef) ;
  // if it's a ruler, get rid of it upon release - this is a temporary hack until I can finish the ruler
  if (IS_QCAD_RULER (tmpobj))
    g_object_unref (tmpobj) ;
  else
    {
    qcad_design_object_draw (tmpobj, widget->window, GDK_XOR) ;
    if (NULL != drop_function) (*drop_function) (QCAD_DESIGN_OBJECT (klass->tmpobj)) ;
    }

  return FALSE ;
  }
#endif /* def DESIGNER */
#endif /* def GTK_GUI */
static void stretch_draw_state_change (QCADStretchyObject *obj, int x, int y, int xRef, int yRef)
  {
  QCAD_DESIGN_OBJECT (obj)->bounding_box.xWorld = real_to_world_x (MIN (xRef, x)) ;
  QCAD_DESIGN_OBJECT (obj)->bounding_box.yWorld = real_to_world_y (MIN (yRef, y)) ;
  QCAD_DESIGN_OBJECT (obj)->bounding_box.cxWorld = real_to_world_cx (ABS (xRef - x) + 1) ;
  QCAD_DESIGN_OBJECT (obj)->bounding_box.cyWorld = real_to_world_cy (ABS (yRef - y) + 1) ;
  QCAD_DESIGN_OBJECT (obj)->x = QCAD_DESIGN_OBJECT (obj)->bounding_box.xWorld + QCAD_DESIGN_OBJECT (obj)->bounding_box.cxWorld / 2.0 ;
  QCAD_DESIGN_OBJECT (obj)->y = QCAD_DESIGN_OBJECT (obj)->bounding_box.yWorld + QCAD_DESIGN_OBJECT (obj)->bounding_box.cyWorld / 2.0 ;
  }
