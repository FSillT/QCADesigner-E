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
// The pixel <--> nanometer transformation functions,   //
// as well as a transformation stack.                   //
//                                                      //
//////////////////////////////////////////////////////////

#include "object_helpers.h"
#include "../generic_utils.h"
#include "../exp_array.h"
#include "QCADSubstrate.h"

typedef struct
  {
  int subs_top_x ;
  int subs_top_y ;
  int cxClientArea ;
  int cyClientArea ;
  double scale ;
  } TRANSFORMATION ;

static EXP_ARRAY *transformation_stack = NULL ;

static int subs_top_x = 0, subs_top_y = 0 ;
static double scale = 1 ; // pixels / nm
static int cxClientArea = 0, cyClientArea = 0 ;

static QCADSubstrate *snap_source = NULL ;

static void snap_source_is_gone (gpointer data, GObject *obj) ;

void push_transformation ()
  {
  TRANSFORMATION xform = {-1} ;

  xform.subs_top_x = subs_top_x ;
  xform.subs_top_y = subs_top_y ;
  xform.cxClientArea = cxClientArea ;
  xform.cyClientArea = cyClientArea ;
  xform.scale = scale ;

  if (NULL == transformation_stack)
    transformation_stack = exp_array_new (sizeof (TRANSFORMATION), 1) ;

  exp_array_insert_vals (transformation_stack, &xform, 1, 1, -1) ;
  }

void pop_transformation ()
  {
  subs_top_x =
    exp_array_index_1d (transformation_stack, TRANSFORMATION, transformation_stack->icUsed - 1).subs_top_x ;
  subs_top_y =
    exp_array_index_1d (transformation_stack, TRANSFORMATION, transformation_stack->icUsed - 1).subs_top_y ;
  cxClientArea =
    exp_array_index_1d (transformation_stack, TRANSFORMATION, transformation_stack->icUsed - 1).cxClientArea ;
  cyClientArea =
    exp_array_index_1d (transformation_stack, TRANSFORMATION, transformation_stack->icUsed - 1).cyClientArea ;
  scale =
    exp_array_index_1d (transformation_stack, TRANSFORMATION, transformation_stack->icUsed - 1).scale ;
  }

GdkRectangle *world_to_real_rect (WorldRectangle *rcWorld, GdkRectangle *rcReal)
  {
  rcReal->x = world_to_real_x (rcWorld->xWorld) ;
  rcReal->y = world_to_real_y (rcWorld->yWorld) ;
  rcReal->width = world_to_real_cx (rcWorld->cxWorld) ;
  rcReal->height = world_to_real_cy (rcWorld->cyWorld) ;

  return rcReal ;
  }

WorldRectangle *real_to_world_rect (WorldRectangle *rcWorld, GdkRectangle *rcReal)
  {
  rcWorld->xWorld = real_to_world_x (rcReal->x) ;
  rcWorld->yWorld = real_to_world_y (rcReal->y) ;
  rcWorld->cxWorld = real_to_world_cx (rcReal->width) ;
  rcWorld->cyWorld = real_to_world_cy (rcReal->height) ;

  return rcWorld ;
  }

int world_to_real_x (double xWorld) {return (int)NINT (scale * xWorld + subs_top_x) ;}

int world_to_real_y (double yWorld) {return (int)NINT (scale * yWorld + subs_top_y) ;}

int world_to_real_cx (double cxWorld) {return (int)NINT (cxWorld * scale) ;}

int world_to_real_cy (double cyWorld) {return (int)NINT (cyWorld * scale) ;}

double real_to_world_x (int xReal) {return (double)((xReal - subs_top_x) / scale) ;}

double real_to_world_y (int yReal) {return (double)((yReal - subs_top_y) / scale) ;}

double real_to_world_cx (int cxReal) {return (double)(cxReal / scale) ;}

double real_to_world_cy (int cyReal) {return (double)(cyReal / scale) ;}

void world_to_grid_pt (double *pxWorld, double *pyWorld)
  {
  if (NULL == snap_source) return ;
  else
    qcad_substrate_snap_point (snap_source, pxWorld, pyWorld) ;
  }

// cx and cy are in real coordinates
void pan (int cx, int cy)
  {
  subs_top_x += cx ;
  subs_top_y += cy ;
  }

void set_client_dimension (int cxNew, int cyNew)
  {
  cxClientArea = cxNew ;
  cyClientArea = cyNew ;
  }

void set_snap_source (void *new_snap_source)
  {
  if (NULL != (snap_source = new_snap_source))
    g_object_weak_ref (G_OBJECT (new_snap_source), (GWeakNotify)snap_source_is_gone, NULL) ;
  }

static void snap_source_is_gone (gpointer data, GObject *obj)
  {
  if (snap_source == (QCADSubstrate *)obj)
    snap_source = NULL ;
  }

gboolean is_real_point_visible (int xReal, int yReal)
  {return (xReal >= 0 && xReal < cxClientArea && yReal >= 0 && yReal < cxClientArea) ;}

gboolean is_real_rect_visible (GdkRectangle *rc)
  {return (!(rc->x + rc->width < 0 || rc->y + rc->height < 0 || rc->x >= cxClientArea || rc->y >= cyClientArea)) ;}

//!Zooms out a little
void zoom_out ()
  {zoom_window(-30, -30, cxClientArea + 30, cyClientArea + 30);}

//!Zooms in a little
void zoom_in ()
  {zoom_window(30, 30, cxClientArea - 30, cyClientArea - 30);}

void reset_zoom ()
  {scale = 1.0 ;}

//!Zooms to the provided window dimensions.
void zoom_window (int top_x, int top_y, int bot_x, int bot_y)
  {
  int xMin, yMin, xMax, yMax, cxWindow, cyWindow ;
  double
    dcxWindow = (double)(cxWindow = (xMax = MAX (top_x, bot_x)) - (xMin = MIN (top_x, bot_x))),
    dcyWindow = (double)(cyWindow = (yMax = MAX (top_y, bot_y)) - (yMin = MIN (top_y, bot_y))),
    dx = 0, dy = 0,
    dcxArea = (double)cxClientArea,
    dcyArea = (double)cyClientArea,
    scale_factor = 0;

  fit_rect_inside_rect (dcxArea, dcyArea, &dx, &dy, &dcxWindow, &dcyWindow) ;

  scale_factor = dcxWindow / (double)cxWindow ;

  pan (
    dx - ((xMin - subs_top_x) * scale_factor + subs_top_x),
    dy - ((yMin - subs_top_y) * scale_factor + subs_top_y)) ;

  scale = scale * scale_factor ;
  }

void get_world_viewport (double *pxMin, double *pyMin, double *pxMax, double *pyMax)
  {
  (*pxMin) = real_to_world_x (0) ;
  (*pyMin) = real_to_world_y (0) ;
  (*pxMax) = real_to_world_x (cxClientArea - 1) ;
  (*pyMax) = real_to_world_y (cyClientArea - 1) ;
  }

void world_rect_union (WorldRectangle *rc1, WorldRectangle *rc2, WorldRectangle *rcDst)
  {
  double xWorld, yWorld ;

  if (NULL == rc1 || NULL == rc2 || NULL == rcDst) return ;

  xWorld = MIN (rc1->xWorld, rc2->xWorld) ;
  yWorld = MIN (rc1->yWorld, rc2->yWorld) ;
  rcDst->cxWorld = MAX (rc1->xWorld + rc1->cxWorld, rc2->xWorld + rc2->cxWorld) - xWorld ;
  rcDst->cyWorld = MAX (rc1->yWorld + rc1->cyWorld, rc2->yWorld + rc2->cyWorld) - yWorld ;
  rcDst->xWorld = xWorld ;
  rcDst->yWorld = yWorld ;
  }
