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
// Header for a set of function responsible for         //
// creating QCADUndoEntry objects and hooking up the    //
// appropriate callbacks. It is essentially a set of    //
// convenience functions for the Undo/Redo system.      //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _SELECTION_UNDO_H_
#define _SELECTION_UNDO_H_

#ifdef UNDO_REDO

#include <gdk/gdk.h>
#include "design.h"
#include "selection_renderer.h"
#include "exp_array.h"

void push_undo_selection_move      (DESIGN *design, SELECTION_RENDERER *sr, GdkWindow *dst, double dxOffset, double dyOffset) ;
void push_undo_selection_transform (DESIGN *design, SELECTION_RENDERER *sr, GdkWindow *dst, double mtxFore[2][2], double mtxBack[2][2]) ;
void push_undo_selection_altered   (DESIGN *design, SELECTION_RENDERER *sr, GdkWindow *dst, EXP_ARRAY *objs, gboolean bAdded) ;
void push_undo_selection_existence (DESIGN *design, SELECTION_RENDERER *sr, GdkWindow *dst, EXP_ARRAY *objs, gboolean bCreated) ;
void push_undo_selection_clock     (DESIGN *design, SELECTION_RENDERER *sr, GdkWindow *dst, EXP_ARRAY *objs, int clock_new, gboolean bRelative) ;
void push_undo_selection_state     (DESIGN *design, SELECTION_RENDERER *sr, GdkWindow *dst, EXP_ARRAY *objs, char *state, GValue *value) ;
void track_undo_object_update      (DESIGN *design, SELECTION_RENDERER *sr, GdkWindow *dst, QCADUndoEntry *entry) ;

#endif /* def UNDO_REDO */
#endif /* _SELECTION_UNDO_H_ */
