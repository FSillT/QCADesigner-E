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
// The ruler. This is a graduated object displaying the //
// distance relative to its origin. It has four differ- //
// ent orientations designed to "stick" to circuits on  //
// one of 4 sides.                                      //
//                                                      //
//////////////////////////////////////////////////////////

#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <glib-object.h>
#include "../gdk_structs.h"
#include "QCADRuler.h"
#include "QCADLabel.h"
#include "object_helpers.h"
#include "../support.h"
#include "../fileio_helpers.h"
#include "../global_consts.h"
#include "../custom_widgets.h"
#include "objects_debug.h"

#define MIN_GRADATION_DISTANCE_PIXELS 10

#define XTOP_RULER_OFFSET 2
#define YTOP_RULER_OFFSET 2

#define QCAD_RULER_CALCULATE_ORIENTATION(cxWorld,cyWorld,x,y,xRef,yRef) \
  (((cyWorld) > (cxWorld)) \
     ? ((x) >= (xRef)) \
       ? QCAD_RULER_ORIENTATION_WEST \
       : QCAD_RULER_ORIENTATION_EAST \
     : ((y) >= (yRef)) \
       ? QCAD_RULER_ORIENTATION_NORTH \
       : QCAD_RULER_ORIENTATION_SOUTH)

typedef struct
  {
  QCADLabel *lbl ;
  double dVal ;
  } GRADUATION ;

static void qcad_ruler_class_init (GObjectClass *klass, gpointer data) ;
static void qcad_ruler_instance_init (GObject *object, gpointer data) ;
static void qcad_ruler_instance_finalize (GObject *object) ;

#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop) ;
#endif /* def GTK_GUI */
#ifdef STDIO_FILEIO
static void serialize (QCADDesignObject *obj, FILE *fp) ;
static gboolean unserialize (QCADDesignObject *obj, FILE *fp) ;
#endif /* def STDIO_FILEIO */
static const char *PostScript_preamble () ;
static char *PostScript_instance (QCADDesignObject *obj, gboolean bColour) ;
static void stretch_draw_state_change (QCADStretchyObject *obj, int x, int y, int xRef, int yRef) ;
static void move (QCADDesignObject *obj, double dxWorld, double dyWorld) ;

#ifdef GTK_GUI
static QCADLabel *get_label_from_array (EXP_ARRAY *ar, int idx, double dCurrentGrad, GdkColor *clrLabel) ;
#endif /* def GTK_GUI */

GdkColor clrCyan = {0x0000, 0x0000, 0xFFFF, 0xFFFF} ;

GType qcad_ruler_get_type ()
  {
  static GType qcad_ruler_type = 0 ;

  if (!qcad_ruler_type)
    {
    static const GTypeInfo qcad_ruler_info =
      {
      sizeof (QCADRulerClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_ruler_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADRuler),
      0,
      (GInstanceInitFunc)qcad_ruler_instance_init
      } ;

    if ((qcad_ruler_type = g_type_register_static (QCAD_TYPE_STRETCHY_OBJECT, QCAD_TYPE_STRING_RULER, &qcad_ruler_info, 0)))
      g_type_class_ref (qcad_ruler_type) ;
    DBG_OO (fprintf (stderr, "Registered QCADRuler as %d\n", (int)qcad_ruler_type)) ;
    }
  return qcad_ruler_type ;
  }

static void qcad_ruler_class_init (GObjectClass *klass, gpointer data)
  {
  DBG_OO (fprintf (stderr, "QCADRuler::class_init:Entering\n")) ;
#ifdef GTK_GUI
  if (0 == clrCyan.pixel)
    gdk_colormap_alloc_color (gdk_colormap_get_system (), &clrCyan, FALSE, TRUE) ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->draw = draw ;
#endif /* def GTK_GUI */
#ifdef STDIO_FILEIO
  QCAD_DESIGN_OBJECT_CLASS (klass)->serialize = serialize ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->unserialize = unserialize ;
#endif /* def STDIO_FILEIO */
  G_OBJECT_CLASS (klass)->finalize = qcad_ruler_instance_finalize ;
  QCAD_STRETCHY_OBJECT_CLASS (klass)->stretch_draw_state_change = stretch_draw_state_change ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->move = move ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->PostScript_preamble = PostScript_preamble ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->PostScript_instance = PostScript_instance ;
  DBG_OO (fprintf (stderr, "QCADRuler::class_init:Leaving\n")) ;
  }

static void qcad_ruler_instance_init (GObject *object, gpointer data)
  {
  DBG_OO (fprintf (stderr, "QCADRuler::instance_init:Entering\n")) ;
  QCAD_RULER (object)->labels = exp_array_new (sizeof (GRADUATION), 1) ;
  memcpy (&(QCAD_DESIGN_OBJECT (object)->clr), &clrCyan, sizeof (GdkColor)) ;
  QCAD_RULER (object)->ruler_bounding_box.xWorld =
  QCAD_RULER (object)->ruler_bounding_box.yWorld =
  QCAD_RULER (object)->ruler_bounding_box.cxWorld =
  QCAD_RULER (object)->ruler_bounding_box.cyWorld = 0.0 ;
  DBG_OO (fprintf (stderr, "QCADRuler::instance_init:Leaving\n")) ;
  }

static void qcad_ruler_instance_finalize (GObject *object)
  {
  int Nix ;
  QCADRuler *ruler = QCAD_RULER (object) ;

  DBG_OO (fprintf (stderr, "QCADRuler::instance_finalize:Entering\n")) ;
  for (Nix = 0 ; Nix < ruler->labels->icUsed ; Nix++)
    g_object_unref (G_OBJECT (exp_array_index_1d (ruler->labels, GRADUATION, Nix).lbl)) ;
  exp_array_free (ruler->labels) ;
  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_RULER)))->finalize (object) ;
  DBG_OO (fprintf (stderr, "QCADRuler::instance_finalize:Leaving\n")) ;
  }

///////////////////////////////////////////////////////////////////////////////

#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop)
  {
  GdkGC *gc = NULL ;
  GdkRectangle rcReal ;
  QCADRuler *ruler = QCAD_RULER (obj) ;
  double
    dCurrentGrad = 0 ;
  int iPowerOfTen = 0 ;
  int idx = -1 ;
  gboolean bVertical = (QCAD_RULER_ORIENTATION_EAST == ruler->orientation || QCAD_RULER_ORIENTATION_WEST == ruler->orientation) ;
  QCADLabel *lblGrad = NULL ;

  world_to_real_rect (&(ruler->ruler_bounding_box), &rcReal) ;

  gc = gdk_gc_new (dst) ;
  gdk_gc_set_function (gc, rop) ;
  gdk_gc_set_foreground (gc, &(obj->clr)) ;

  // Graduation lines should be at least MIN_GRADATION_DISTANCE_PIXELS pixels apart
  iPowerOfTen =
    (int)ceil (log10 (bVertical
      ? real_to_world_cy (MIN_GRADATION_DISTANCE_PIXELS)
      : real_to_world_cx (MIN_GRADATION_DISTANCE_PIXELS))) ;

  ruler->icLabelsVisible = 0 ;

  if (bVertical)
    {
    int xBeg = 0, xEnd = 0, xMid = 0, yReal = 0.0 ;
    double xBegWorld, xEndWorld, xMidWorld, yWorld ;

    if (QCAD_RULER_ORIENTATION_WEST == ruler->orientation)
      {
      xBeg = world_to_real_x (xBegWorld = ruler->ruler_bounding_box.xWorld),
      xEnd = world_to_real_x (xEndWorld = ruler->ruler_bounding_box.xWorld + ruler->ruler_bounding_box.cxWorld) ;
      }
    else
      {
      xBeg = world_to_real_x (xBegWorld = ruler->ruler_bounding_box.xWorld + ruler->ruler_bounding_box.cxWorld),
      xEnd = world_to_real_x (xEndWorld = ruler->ruler_bounding_box.xWorld) ;
      }
    xMid = world_to_real_x (xMidWorld = ruler->ruler_bounding_box.xWorld + ruler->ruler_bounding_box.cxWorld / 2.0) ;

    for (dCurrentGrad = 0 ; dCurrentGrad < ruler->ruler_bounding_box.cyWorld ; dCurrentGrad += pow (10, iPowerOfTen))
      {
      idx++ ;
      idx %= 10 ;
      yReal = world_to_real_y (yWorld = ruler->ruler_bounding_box.yWorld + dCurrentGrad) ;
      gdk_draw_line (dst, gc, xBeg, yReal, (0 == idx ? xEnd : xMid), yReal) ;

      if (0 == idx)
        {
        lblGrad = get_label_from_array (ruler->labels, (ruler->icLabelsVisible)++, dCurrentGrad, &(obj->clr)) ;
        qcad_label_shrinkwrap (lblGrad) ;
        qcad_design_object_move_to (QCAD_DESIGN_OBJECT (lblGrad), xEndWorld, yWorld) ;
        qcad_design_object_draw (QCAD_DESIGN_OBJECT (lblGrad), dst, rop) ;
        }
      }
    gdk_draw_line (dst, gc, xBeg, rcReal.y, xBeg, rcReal.y + rcReal.height) ;

    lblGrad = get_label_from_array (ruler->labels, (ruler->icLabelsVisible)++, ruler->ruler_bounding_box.cyWorld, &(obj->clr)) ;
    qcad_label_shrinkwrap (lblGrad) ;
    qcad_design_object_move_to (QCAD_DESIGN_OBJECT (lblGrad), xBegWorld, ruler->ruler_bounding_box.yWorld + ruler->ruler_bounding_box.cyWorld) ;
    qcad_design_object_draw (QCAD_DESIGN_OBJECT (lblGrad), dst, rop) ;
    }
  else
    {
    int yBeg = 0, yEnd = 0, yMid = 0, xReal = 0.0 ;
    double yBegWorld, yEndWorld, yMidWorld, xWorld ;

    if (QCAD_RULER_ORIENTATION_NORTH == ruler->orientation)
      {
      yBeg = world_to_real_y (yBegWorld = ruler->ruler_bounding_box.yWorld),
      yEnd = world_to_real_y (yEndWorld = ruler->ruler_bounding_box.yWorld + ruler->ruler_bounding_box.cyWorld) ;
      }
    else
      {
      yBeg = world_to_real_y (yBegWorld = ruler->ruler_bounding_box.yWorld + ruler->ruler_bounding_box.cyWorld),
      yEnd = world_to_real_y (yEndWorld = ruler->ruler_bounding_box.yWorld) ;
      }
    yMid = world_to_real_y (yMidWorld = ruler->ruler_bounding_box.yWorld + ruler->ruler_bounding_box.cyWorld / 2.0) ;

    for (dCurrentGrad = 0 ; dCurrentGrad < ruler->ruler_bounding_box.cxWorld ; dCurrentGrad += pow (10, iPowerOfTen))
      {
      idx++ ;
      idx %= 10 ;
      xReal = world_to_real_x (xWorld = ruler->ruler_bounding_box.xWorld + dCurrentGrad) ;
      gdk_draw_line (dst, gc, xReal, yBeg, xReal, (0 == idx ? yEnd : yMid)) ;

      if (0 == idx)
        {
        lblGrad = get_label_from_array (ruler->labels, (ruler->icLabelsVisible)++, dCurrentGrad, &(obj->clr)) ;
        qcad_label_shrinkwrap (lblGrad) ;
        qcad_design_object_move_to (QCAD_DESIGN_OBJECT (lblGrad), xWorld, yEndWorld) ;
        qcad_design_object_draw (QCAD_DESIGN_OBJECT (lblGrad), dst, rop) ;
        }
      }
    gdk_draw_line (dst, gc, rcReal.x, yBeg, rcReal.x + rcReal.width, yBeg) ;

    lblGrad = get_label_from_array (ruler->labels, (ruler->icLabelsVisible)++, ruler->ruler_bounding_box.cxWorld, &(obj->clr)) ;
    qcad_label_shrinkwrap (lblGrad) ;
    qcad_design_object_move_to (QCAD_DESIGN_OBJECT (lblGrad), ruler->ruler_bounding_box.xWorld + ruler->ruler_bounding_box.cxWorld, yBegWorld) ;
    qcad_design_object_draw (QCAD_DESIGN_OBJECT (lblGrad), dst, rop) ;
    }

  g_object_unref (gc) ;
  }
#endif /* def GTK_GUI */
#ifdef STDIO_FILEIO
static void serialize (QCADDesignObject *obj, FILE *fp)
  {
  // output object type
  fprintf(fp, "[TYPE:%s]\n", QCAD_TYPE_STRING_RULER);

  // call parent serialize function
  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_RULER)))->serialize (obj, fp) ;

  // output end of object
  fprintf(fp, "[#TYPE:%s]\n", QCAD_TYPE_STRING_RULER);
  }

static gboolean unserialize (QCADDesignObject *obj, FILE *fp)
  {
  char *pszLine = NULL, *pszValue = NULL ;
  gboolean bStopReading = FALSE, bParentInit = FALSE ;

  if (!SkipPast (fp, '\0', "[TYPE:" QCAD_TYPE_STRING_RULER "]", NULL)) return FALSE ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (fp, '\0', TRUE)))
      break ;

    if (!strcmp (pszLine, "[#TYPE:" QCAD_TYPE_STRING_RULER "]"))
      {
      g_free (pszLine) ;
      break ;
      }

    if (!bStopReading)
      {
      tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

      if (!strncmp (pszLine, "[TYPE:", 6))
        {
        if (!(bParentInit = QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_STRETCHY_OBJECT)))->unserialize (obj, fp)))
          bStopReading = TRUE ;
        }
      }
    g_free (pszLine) ;
    g_free (ReadLine (fp, '\0', FALSE)) ;
    }

  return bParentInit ;
  }
#endif /* def STDIO_FILEIO */

static void move (QCADDesignObject *obj, double dxWorld, double dyWorld)
  {
  int Nix ;
  QCADRuler *ruler = QCAD_RULER (obj) ;

  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_RULER)))->move (obj, dxWorld, dyWorld) ;

  for (Nix = 0 ; Nix < ruler->icLabelsVisible ; Nix++)
    qcad_design_object_move (QCAD_DESIGN_OBJECT (exp_array_index_1d (ruler->labels, GRADUATION, Nix).lbl), dxWorld, dyWorld) ;
  }

static const char *PostScript_preamble ()
  {
  return
    "% x y cx cy r g b QCADRuler\n"
    "/QCADRuler\n"
    "  {\n"
    "  gsave\n"
    "  /b exch def\n"
    "  /g exch def\n"
    "  /r exch def\n"
    "  /cy exch def\n"
    "  /cx exch def\n"
    "  /y exch def\n"
    "  /x exch def\n"
    "\n"
    "  newpath\n"
    "  x y moveto\n"
    "  x cx add y lineto\n"
    "  x cx add y cy sub lineto\n"
    "  x y cy sub lineto\n"
    "  closepath clip\n"
    "\n"
    "  r g b setrgbcolor\n"
    "\n"
    "  linewidth epsilon gt\n"
    "    {\n"
    "    newpath\n"
    "    x y moveto\n"
    "    x cx add y lineto\n"
    "    x cx add y cy sub lineto\n"
    "    x y cy sub lineto\n"
    "    closepath stroke\n"
    "    }\n"
    "  if\n"
    "\n"
    "  grestore\n"
    "  } def\n" ;
  }

static char *PostScript_instance (QCADDesignObject *obj, gboolean bColour)
  {
  GdkColor clr = {0} ;
  double
    r = ((double)(obj->clr.red)) / 65535.0,
    g = ((double)(obj->clr.green)) / 65535.0,
    b = ((double)(obj->clr.blue)) / 65535.0 ;

  if (!bColour)
    {
    memcpy (&clr, &(obj->clr), sizeof (GdkColor)) ;
    RGBToHSL (&clr) ;
    r =
    g =
    b = ((double)(obj->clr.blue)) / 65536.0 ; // .blue has become the luminance
    }

  return g_strdup_printf ("%lf nmx %lf nmy %lf nm %lf nm %lf %lf %lf QCADRuler",
    obj->bounding_box.xWorld, obj->bounding_box.yWorld, obj->bounding_box.cxWorld, obj->bounding_box.cyWorld, r, g, b) ;
  }

QCADRuler *qcad_ruler_new ()
  {
  QCADRuler *qcad_obj = g_object_new (QCAD_TYPE_RULER, NULL) ;
  return qcad_obj ;
  }

static void stretch_draw_state_change (QCADStretchyObject *obj, int x, int y, int xRef, int yRef)
  {
  int Nix ;
  QCADRuler *ruler = QCAD_RULER (obj) ;
  int xMIN = MIN (x, xRef), xMAX = MAX (x, xRef), yMIN = MIN (y, yRef), yMAX = MAX (y, yRef) ;
  QCADStretchyObjectClass *klass = QCAD_STRETCHY_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_RULER))) ;

  if (NULL != klass->stretch_draw_state_change)
    klass->stretch_draw_state_change (obj, x, y, xRef, yRef) ;

  ruler->ruler_bounding_box.xWorld = real_to_world_x (xMIN) ;
  ruler->ruler_bounding_box.yWorld = real_to_world_y (yMIN) ;
  ruler->ruler_bounding_box.cxWorld = real_to_world_cx (xMAX - xMIN) ;
  ruler->ruler_bounding_box.cyWorld = real_to_world_cy (yMAX - yMIN) ;

  memcpy (&(QCAD_DESIGN_OBJECT (obj)->bounding_box), &(ruler->ruler_bounding_box), sizeof (WorldRectangle)) ;

  for (Nix = 0 ; Nix < ruler->icLabelsVisible ; Nix++)
    world_rect_union (&(QCAD_DESIGN_OBJECT (obj)->bounding_box), &(QCAD_DESIGN_OBJECT (exp_array_index_1d (ruler->labels, GRADUATION, Nix).lbl)->bounding_box), &(QCAD_DESIGN_OBJECT (obj)->bounding_box)) ;

  QCAD_RULER (obj)->orientation =
    QCAD_RULER_CALCULATE_ORIENTATION (
      ruler->ruler_bounding_box.cxWorld,
      ruler->ruler_bounding_box.cyWorld, x, y, xRef, yRef) ;
  }

#ifdef GTK_GUI
static QCADLabel *get_label_from_array (EXP_ARRAY *ar, int idx, double dCurrentGrad, GdkColor *clr)
  {
  GRADUATION grad = {NULL, 0.0} ;
  GRADUATION *ar_grad = NULL ;

  if (idx == ar->icUsed)
    {
    grad.lbl = qcad_label_new ("%.2lf", dCurrentGrad) ;
    grad.dVal = dCurrentGrad ;
    exp_array_insert_vals (ar, &grad, 1, 1, -1) ;
    ar_grad = &grad ;
    }
  else
  if ((ar_grad = &exp_array_index_1d (ar, GRADUATION, idx))->dVal != dCurrentGrad)
    {
    qcad_label_set_text (QCAD_LABEL (ar_grad->lbl), "%.2lf", dCurrentGrad) ;
    ar_grad->dVal = dCurrentGrad ;
    }

  memcpy (&(QCAD_DESIGN_OBJECT (ar_grad->lbl)->clr), clr, sizeof (GdkColor)) ;

  return ar_grad->lbl ;
  }
#endif /* def GTK_GUI */
