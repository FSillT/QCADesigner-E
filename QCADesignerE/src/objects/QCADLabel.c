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
// QCADLabel: A simple text label to accompany cells    //
// or to stand on its own in a drawing layer.           //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdarg.h>
#include <string.h>
#include <glib-object.h>
#include "../gdk_structs.h"
#include "QCADLabel.h"
#include "object_helpers.h"
#include "../support.h"
#include "../fileio_helpers.h"
#include "../custom_widgets.h"
#include "../global_consts.h"
#ifdef GTK_GUI
  #include "../exp_pixmap.h"
#endif /* def GTK_GUI */
#include "objects_debug.h"

#define XTOP_LABEL_OFFSET 2
#define YTOP_LABEL_OFFSET 2

#define CYFONT 12 /* nanometers */

#ifdef GTK_GUI
typedef struct
  {
  GtkWidget *dlg ;
  GtkWidget *txtLabel ;
  } PROPERTIES ;
#endif /* def GTK_GUI */

static void qcad_label_class_init (GObjectClass *klass, gpointer data) ;
static void qcad_label_instance_init (GObject *object, gpointer data) ;
static void qcad_label_instance_finalize (GObject *object) ;

static void copy (QCADDesignObject *src, QCADDesignObject *dst) ;
#ifdef STDIO_FILEIO
static void serialize (QCADDesignObject *obj, FILE *fp) ;
static gboolean unserialize (QCADDesignObject *obj, FILE *fp) ;
#endif /* def STDIO_FILEIO */
#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop) ;
#ifdef UNDO_REDO
static gboolean properties (QCADDesignObject *obj, GtkWidget *parent, QCADUndoEntry **pentry) ;
#else
static gboolean properties (QCADDesignObject *obj, GtkWidget *parent) ;
#endif /* def UNDO_REDO */
#endif /* def GTK_GUI */
static const char *PostScript_preamble () ;
static char *PostScript_instance (QCADDesignObject *obj, gboolean bColour) ;

#ifdef GTK_GUI
void create_properties_dialog (PROPERTIES *dialog) ;
#endif /* def GTK_GUI */

//extern GdkFont *font ;
extern GdkColor clrBlue ;

GType qcad_label_get_type ()
  {
  static GType qcad_label_type = 0 ;

  if (!qcad_label_type)
    {
    static const GTypeInfo qcad_label_info =
      {
      sizeof (QCADLabelClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_label_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADLabel),
      0,
      (GInstanceInitFunc)qcad_label_instance_init
      } ;

    if ((qcad_label_type = g_type_register_static (QCAD_TYPE_STRETCHY_OBJECT, QCAD_TYPE_STRING_LABEL, &qcad_label_info, 0)))
      g_type_class_ref (qcad_label_type) ;
    DBG_OO (fprintf (stderr, "Registered QCADLabel as %d\n", (int)qcad_label_type)) ;
    }
  return qcad_label_type ;
  }

static void qcad_label_class_init (GObjectClass *klass, gpointer data)
  {
  DBG_OO (fprintf (stderr, "QCADLabel::class_init:Entering\n")) ;
#ifdef GTK_GUI
  if (0 == clrBlue.pixel)
    gdk_colormap_alloc_color (gdk_colormap_get_system (), &clrBlue, FALSE, TRUE) ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->draw = draw ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->properties = properties ;
#endif /* def GTK_GUI */
#ifdef STDIO_FILEIO
  QCAD_DESIGN_OBJECT_CLASS (klass)->serialize = serialize ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->unserialize = unserialize ;
#endif /* def STDIO_FILEIO */
  G_OBJECT_CLASS (klass)->finalize = qcad_label_instance_finalize ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->copy = copy ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->PostScript_preamble = PostScript_preamble ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->PostScript_instance = PostScript_instance ;
  DBG_OO (fprintf (stderr, "QCADLabel::class_init:Leaving\n")) ;
  }

static void qcad_label_instance_init (GObject *object, gpointer data)
  {
  DBG_OO (fprintf (stderr, "QCADLabel::instance_init:Entering\n")) ;
  QCAD_LABEL (object)->psz = g_strdup (_("Label")) ;
  QCAD_LABEL (object)->bNeedsEPMDraw = TRUE ;
  QCAD_LABEL (object)->bShrinkWrap = TRUE ;
#ifdef GTK_GUI
  QCAD_LABEL (object)->epm = NULL ;
#endif /* def GTK_GUI */
  QCAD_DESIGN_OBJECT (object)->x =
  QCAD_DESIGN_OBJECT (object)->y =
  QCAD_DESIGN_OBJECT (object)->bounding_box.xWorld =
  QCAD_DESIGN_OBJECT (object)->bounding_box.yWorld =
  QCAD_DESIGN_OBJECT (object)->bounding_box.cxWorld =
  QCAD_DESIGN_OBJECT (object)->bounding_box.cyWorld = 0.0 ;
  memcpy (&(QCAD_DESIGN_OBJECT (object)->clr), &clrBlue, sizeof (GdkColor)) ;
  qcad_label_shrinkwrap (QCAD_LABEL (object)) ;
  DBG_OO (fprintf (stderr, "QCADLabel::instance_init:Leaving\n")) ;
  }

static void qcad_label_instance_finalize (GObject *object)
  {
  DBG_OO (fprintf (stderr, "QCADLabel::instance_finalize:Entering\n")) ;
  g_free (QCAD_LABEL (object)->psz) ;
#ifdef GTK_GUI
  exp_pixmap_free (QCAD_LABEL (object)->epm) ;
#endif /* def GTK_GUI */
  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_LABEL)))->finalize (object) ;
  DBG_OO (fprintf (stderr, "QCADLabel::instance_finalize:Leaving\n")) ;
  }

///////////////////////////////////////////////////////////////////////////////

static void copy (QCADDesignObject *src, QCADDesignObject *dst)
  {
  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_LABEL)))->copy (src, dst) ;
  QCAD_LABEL (dst)->psz = g_strdup (QCAD_LABEL (src)->psz) ;
  QCAD_LABEL (dst)->bNeedsEPMDraw = TRUE ;
  }

#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop)
  {
  GdkGC *gc = NULL ;
  char *pszFont = NULL ;
  GdkRectangle rc = {0} ;
  GdkRectangle rcDst = {0} ;
  GdkRectangle rcDraw = {0} ;
  QCADLabel *label = NULL ;

  if ((label = QCAD_LABEL (obj))->bShrinkWrap)
    qcad_label_shrinkwrap (label) ;

  world_to_real_rect (&(obj->bounding_box), &rc) ;
  gdk_drawable_get_size (dst, &(rcDst.width), &(rcDst.height)) ;
  if (!gdk_rectangle_intersect (&rc, &rcDst, &rcDraw)) return ;

  gc = gdk_gc_new (dst) ;
  gdk_gc_set_function (gc, rop) ;
  gdk_gc_set_clip_rectangle (gc, &rc) ;
  if (GDK_COPY == rop)
    {
    gdk_gc_set_foreground (gc, obj->bSelected ? &(QCAD_DESIGN_OBJECT_GET_CLASS (obj)->clrSelected) : &(obj->clr)) ;
    draw_string (dst, gc, pszFont = g_strdup_printf ("Courier %d", world_to_real_cy (CYFONT)),
      rc.x + XTOP_LABEL_OFFSET, rc.y + YTOP_LABEL_OFFSET, label->psz) ;
    g_free (pszFont) ;
    }
  else
    {
    if (label->bNeedsEPMDraw)
      {
      GdkGC *gcEPM = NULL ;

      label->bNeedsEPMDraw = FALSE ;
      label->epm = exp_pixmap_cond_new (label->epm, dst, rcDraw.width, rcDraw.height, -1) ;
      exp_pixmap_clean (label->epm) ;
      gcEPM = gdk_gc_new (label->epm->pixmap) ;
      gdk_gc_set_foreground (gcEPM, obj->bSelected ? &(QCAD_DESIGN_OBJECT_GET_CLASS (obj)->clrSelected) : &(obj->clr)) ;
      draw_string (label->epm->pixmap, gcEPM, pszFont = g_strdup_printf ("Courier %d", world_to_real_cy (CYFONT)),
        XTOP_LABEL_OFFSET, YTOP_LABEL_OFFSET, label->psz) ;
      g_free (pszFont) ;
      g_object_unref (gcEPM) ;
      }
    gdk_draw_drawable (dst, gc, label->epm->pixmap, 0, 0, rc.x, rc.y, rcDraw.width, rcDraw.height) ;
    }

  if (obj->bSelected)
    gdk_draw_rectangle (dst, gc, FALSE, rc.x, rc.y, rc.width - 1, rc.height - 1) ;
  gdk_gc_unref (gc) ;
  }

#ifdef UNDO_REDO
static gboolean properties (QCADDesignObject *obj, GtkWidget *parent, QCADUndoEntry **pentry)
#else
static gboolean properties (QCADDesignObject *obj, GtkWidget *parent)
#endif /* UNDO_REDO */
  {
  static PROPERTIES dialog = {NULL} ;
  gboolean bRet = FALSE ;

  if (!IS_QCAD_LABEL (obj)) return FALSE ;

  if (NULL == dialog.dlg)
    create_properties_dialog (&dialog) ;

  gtk_window_set_transient_for (GTK_WINDOW (dialog.dlg), GTK_WINDOW (parent)) ;
  gtk_entry_set_text (GTK_ENTRY (dialog.txtLabel), QCAD_LABEL (obj)->psz) ;
  gtk_widget_grab_focus (dialog.txtLabel) ;

  if ((bRet = (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (dialog.dlg)))))
    {
    if (NULL != QCAD_LABEL (obj)->psz)
      g_free (QCAD_LABEL (obj)->psz) ;
    QCAD_LABEL (obj)->psz = gtk_editable_get_chars (GTK_EDITABLE (dialog.txtLabel), 0, -1) ;
    QCAD_LABEL (obj)->bNeedsEPMDraw = TRUE ;
    }
#ifdef UNDO_REDO
  if (NULL != pentry)
    (*pentry) = NULL ;
#endif /* def UNDO_REDO */
  gtk_widget_hide (dialog.dlg) ;
  return bRet ;
  }
#endif /* def GTK_GUI */
#ifdef STDIO_FILEIO
static void serialize (QCADDesignObject *obj, FILE *fp)
  {
  // output object type
  fprintf(fp, "[TYPE:%s]\n", QCAD_TYPE_STRING_LABEL);

  // call parent serialize function
  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_LABEL)))->serialize (obj, fp) ;

  // output variables
  fprintf (fp, "psz=%s\n", QCAD_LABEL (obj)->psz) ;

  // output end of object
  fprintf(fp, "[#TYPE:%s]\n", QCAD_TYPE_STRING_LABEL);
  }

static gboolean unserialize (QCADDesignObject *obj, FILE *fp)
  {
  char *pszLine = NULL, *pszValue = NULL ;
  gboolean bStopReading = FALSE, bParentInit = FALSE ;

  if (!SkipPast (fp, '\0', "[TYPE:" QCAD_TYPE_STRING_LABEL "]", NULL)) return FALSE ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (fp, '\0', TRUE)))
      break ;

    if (!strcmp (pszLine, "[#TYPE:" QCAD_TYPE_STRING_LABEL "]"))
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

      else
      if (!strcmp (pszLine, "psz"))
        {
        QCAD_LABEL (obj)->psz = g_strdup (pszValue) ;
        QCAD_LABEL (obj)->bNeedsEPMDraw = TRUE ;
        }
      }
    g_free (pszLine) ;
    g_free (ReadLine (fp, '\0', FALSE)) ;
    }

  if (bParentInit)
    qcad_label_shrinkwrap (QCAD_LABEL (obj)) ;

  return bParentInit ;
  }
#endif /* STDIO_FILEIO */

static const char *PostScript_preamble ()
  {
  return
    "% x y cx cy r g b (label) QCADLabel\n"
    "/QCADLabel\n"
    "  {\n"
    "  gsave\n"
    "  /label exch def\n"
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
//    "  linewidth epsilon gt\n"
//    "    {\n"
//    "    newpath\n"
//    "    x y moveto\n"
//    "    x cx add y lineto\n"
//    "    x cx add y cy sub lineto\n"
//    "    x y cy sub lineto\n"
//    "    closepath stroke\n"
//    "    }\n"
//    "  if\n"
    "\n"
    "  x y moveto\n"
    "  (" PS_FONT ") findfont labelfontsize scalefont setfont\n"
    "  label txtlt\n"
    "  grestore\n"
    "  } def\n" ;
  }

static char *PostScript_instance (QCADDesignObject *obj, gboolean bColour)
  {
  QCADLabel *lbl = QCAD_LABEL (obj) ;
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

  return g_strdup_printf ("%lf nmx %lf nmy %lf nm %lf nm %lf %lf %lf (%s) QCADLabel",
    obj->bounding_box.xWorld, obj->bounding_box.yWorld, obj->bounding_box.cxWorld, obj->bounding_box.cyWorld, r, g, b, lbl->psz) ;
  }

QCADLabel *qcad_label_new (char *psz, ...)
  {
  QCADLabel *lbl = NULL ;
  va_list va ;

  va_start (va, psz) ;
  lbl = qcad_label_vnew (psz, va) ;
  va_end (va) ;
  return lbl ;
  }

QCADLabel *qcad_label_vnew (char *psz, va_list va)
  {
  QCADLabel *lbl = g_object_new (QCAD_TYPE_LABEL, NULL) ;

  qcad_label_vset_text (lbl, psz, va) ;
  return lbl ;
  }

void qcad_label_set_text (QCADLabel *label, char *psz, ...)
  {
  va_list va ;

  va_start (va, psz) ;
  qcad_label_vset_text (label, psz, va) ;
  va_end (va) ;
  }

void qcad_label_vset_text (QCADLabel *label, char *psz, va_list va)
  {
  if (NULL != label->psz)
    g_free (label->psz) ;
  label->psz = g_strdup_vprintf (psz, va) ;
  label->bNeedsEPMDraw = TRUE ;
  if (label->bShrinkWrap)
    qcad_label_shrinkwrap (label) ;
  }

void qcad_label_shrinkwrap (QCADLabel *label)
  {
  // This function has no effect if there's no Gtk, because there's no way to measure the (cx,cy) of a
  // string without Gdk
  #ifdef GTK_GUI
  int cx, cy ;
  char *pszFont ;

  get_string_dimensions (label->psz, pszFont = g_strdup_printf ("Courier %d", world_to_real_cy (CYFONT)), &cx, &cy) ;
  g_free (pszFont) ;

  QCAD_DESIGN_OBJECT (label)->bounding_box.cxWorld = real_to_world_cx (cx + 2.0 * XTOP_LABEL_OFFSET) ;
  QCAD_DESIGN_OBJECT (label)->bounding_box.cyWorld = real_to_world_cy (cy + 2.0 * YTOP_LABEL_OFFSET) ;
  QCAD_DESIGN_OBJECT (label)->x = QCAD_DESIGN_OBJECT (label)->bounding_box.cxWorld / 2.0 + QCAD_DESIGN_OBJECT (label)->bounding_box.xWorld ;
  QCAD_DESIGN_OBJECT (label)->y = QCAD_DESIGN_OBJECT (label)->bounding_box.cyWorld / 2.0 + QCAD_DESIGN_OBJECT (label)->bounding_box.yWorld ;
  #endif /* def GTK_GUI */
  }

#ifdef GTK_GUI
void create_properties_dialog (PROPERTIES *dialog)
  {
  GtkWidget *tbl = NULL, *lbl = NULL ;

  dialog->dlg = gtk_dialog_new () ;
  gtk_window_set_title (GTK_WINDOW (dialog->dlg), _("Label Properties")) ;
  gtk_window_set_resizable (GTK_WINDOW (dialog->dlg), FALSE) ;

  tbl = gtk_table_new (1, 2, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog->dlg)->vbox), tbl, TRUE, TRUE, 0) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;

  lbl = gtk_label_new (_("Text:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  dialog->txtLabel = gtk_entry_new () ;
  gtk_widget_show (dialog->txtLabel) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->txtLabel, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->txtLabel), TRUE) ;

  gtk_dialog_add_button (GTK_DIALOG (dialog->dlg), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->dlg), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->dlg), GTK_RESPONSE_OK) ;
  }
#endif /* def GTK_GUI */
