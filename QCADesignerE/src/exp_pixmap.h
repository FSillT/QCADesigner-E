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
// Header file for an "expanding pixmap"                //
// implementation. This pixmap does not shrink.         //
// Instead, when a pixmap with a larger area is needed, //
// a new one is created. the useful size of the pixmap  //
// is stored in a (cxUsed,cyUsed) pair in the           //
// structure. Thus, like the EXP_ARRAY structure, the   //
// size of the EXP_PIXMAP at any given time is that it  //
// had at its peak usage.                               //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _EXP_PIXMAP_H_
#define _EXP_PIXMAP_H_

#include <gdk/gdk.h>

typedef struct
  {
  GdkPixmap *pixmap ;
  int cxUsed ;
  int cyUsed ;
  int cxAvail ;
  int cyAvail ;
  } EXP_PIXMAP ;

EXP_PIXMAP *exp_pixmap_new (GdkWindow *window, int cx, int cy, int depth) ;
EXP_PIXMAP *exp_pixmap_cond_new (EXP_PIXMAP *epm, GdkWindow *window, int cx, int cy, int depth) ;
void        exp_pixmap_resize (EXP_PIXMAP *exp_pixmap, int cxNew, int cyNew) ;
void        exp_pixmap_clean (EXP_PIXMAP *exp_pixmap) ;
EXP_PIXMAP *exp_pixmap_free (EXP_PIXMAP *exp_pixmap) ;

#endif /* _EXP_PIXMAP_H_ */
