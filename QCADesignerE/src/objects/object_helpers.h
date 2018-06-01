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
// The header for the pixel <--> nanometer              //
// transformation functions, as well as for the         //
// transformation stack.                                //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_OBJECT_HELPERS_H_
#define _OBJECTS_OBJECT_HELPERS_H_

#ifdef GTK_GUI
  #include <gtk/gtk.h>
#else
  #include "../gdk_structs.h"
#endif
#include "../hsl2rgb.h"

#define NINT(d) (((d) < 0) ? \
  ((((d) - ((int)(d))) > -0.5) ? ((int)(d)) : (((int)(d)) - 1)) : \
  ((((d) - ((int)(d))) <  0.5) ? ((int)(d)) : (((int)(d)) + 1)))

#define PT_IN_RECT(x,y,xTop,yTop,cx,cy) \
  (((x) >= (xTop) && (x) <= ((xTop)+(cx))) && (((y) >= (yTop)) && ((y) <= ((yTop)+(cy)))))

#define RECT_IN_RECT(xTop1,yTop1,cx1,cy1,xTop2,yTop2,cx2,cy2) \
  ((PT_IN_RECT((xTop1),(yTop1),(xTop2),(yTop2),(cx2),(cy2))) && \
   (PT_IN_RECT((xTop1),((yTop1)+(cy1)),(xTop2),(yTop2),(cx2),(cy2))) && \
   (PT_IN_RECT(((xTop1)+(cx1)),(yTop1),(xTop2),(yTop2),(cx2),(cy2))) && \
   (PT_IN_RECT(((xTop1)+(cx1)),((yTop1)+(cy1)),(xTop2),(yTop2),(cx2),(cy2))))

#define RECT_INTERSECT_RECT(xTop1,yTop1,cx1,cy1,xTop2,yTop2,cx2,cy2) \
  ((MIN (((xTop1)+(cx1)) + 1, ((xTop2)+(cx2)) + 1) - MAX (xTop1, xTop2) > 0) && \
   (MIN (((yTop1)+(cy1)) + 1, ((yTop2)+(cy2)) + 1) - MAX (yTop1, yTop2) > 0))

#define RECT_EQUALS_RECT(xTop1,yTop1,cx1,cy1,xTop2,yTop2,cx2,cy2) \
  ((xTop1 == xTop2) && (yTop1 == yTop2) && (cx1 == cx2) && (cy1 == cy2))

typedef struct
  {
  double xWorld ;
  double yWorld ;
  double cxWorld ;
  double cyWorld ;
  } WorldRectangle ;

int world_to_real_x (double xWorld) ;
int world_to_real_y (double yWorld) ;
int world_to_real_cx (double cxWorld) ;
int world_to_real_cy (double cyWorld) ;
double real_to_world_x (int xReal) ;
double real_to_world_y (int yReal) ;
double real_to_world_cx (int cxReal) ;
double real_to_world_cy (int cyReal) ;
void world_to_grid_pt (double *pxWorld, double *pyWorld) ;
void pan (int cx, int cy) ;
gboolean is_real_point_visible (int xReal, int yReal) ;
void set_client_dimension (int cxNew, int cyNew) ;
void reset_zoom () ;
void zoom_in ();
void zoom_out ();
void zoom_window (int top_x, int top_y, int bot_x, int bot_y) ;
void set_snap_source (void *new_snap_source) ;
gboolean is_real_rect_visible (GdkRectangle *rc) ;
void get_world_viewport (double *pxMin, double *pyMin, double *pxMax, double *pyMax) ;
void push_transformation () ;
void pop_transformation () ;
WorldRectangle *real_to_world_rect (WorldRectangle *rcWorld, GdkRectangle *rcReal) ;
GdkRectangle *world_to_real_rect (WorldRectangle *rcWorld, GdkRectangle *rcReal) ;
void world_rect_union (WorldRectangle *rc1, WorldRectangle *rc2, WorldRectangle *rcDst) ;

#endif /* _OBJECTS_OBJECT_HELPERS_H_ */
