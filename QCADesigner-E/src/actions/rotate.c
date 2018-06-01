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
// Mouse handlers for rotating individual objects.      //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include "rotate.h"
#include "../global_consts.h"
#include "../design.h"
#include "../objects/object_helpers.h"
#include "../callback_helpers.h"
#include "../selection_renderer.h"


gboolean button_pressed_ACTION_ROTATE (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  project_OP *project_options = (project_OP *)data ;
  QCADDesignObject *htobj = NULL ;
  GdkRegion *rgn = NULL ;

  if (NULL != (htobj = QCAD_DESIGN_OBJECT (design_hit_test (project_options->design, event->x, event->y))))
    if (QCAD_IS_CELL (htobj))
      {
      GdkRectangle rcReal ;
      selection_renderer_draw (project_options->srSelection, project_options->design, widget->window, GDK_XOR) ;
      qcad_cell_rotate_dots (QCAD_CELL (htobj), PI / 4.0) ;
      selection_renderer_update (project_options->srSelection, project_options->design) ;
      selection_renderer_draw (project_options->srSelection, project_options->design, widget->window, GDK_XOR) ;

      world_to_real_rect (&(htobj->bounding_box), &rcReal) ;

      rgn = gdk_region_new () ;
      gdk_region_union_with_rect (rgn, &rcReal) ;

      // redraw_async takes care of destroying rgn
      redraw_async (rgn) ;
      }

  return FALSE ;
  }
