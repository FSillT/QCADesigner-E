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
// Mouse handlers for drawing a horizontal or vertical  //
// array of cells.                                      //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include "array.h"
#include "../design.h"
#ifdef UNDO_REDO
  #include "../selection_undo.h"
#endif /* def UNDO_REDO */
#include "../callback_helpers.h"
#include "../objects/object_helpers.h"
#include "../objects/QCADCell.h"

static double x0Grid, y0Grid, x1Grid, y1Grid ;
static GtkOrientation orientation ;

extern gboolean (*drop_function) (QCADDesignObject *obj) ;

gboolean button_pressed_ACTION_ARRAY (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  double xWorld = real_to_world_x (event->x), yWorld = real_to_world_y (event->y) ;
  if (1 != event->button) return FALSE ;

  world_to_grid_pt (&xWorld, &yWorld) ;

  x0Grid = x1Grid = xWorld ;
  y0Grid = y1Grid = yWorld ;

  orientation = GTK_ORIENTATION_HORIZONTAL ;

  qcad_cell_drexp_array (widget->window, GDK_XOR, orientation, x0Grid, x1Grid, y0Grid) ;
  return FALSE ;
  }

gboolean motion_notify_ACTION_ARRAY (GtkWidget *widget, GdkEventMotion *event, gpointer data)
  {
  double xWorld = real_to_world_x (event->x), yWorld = real_to_world_y (event->y) ;

  if (event->state & GDK_BUTTON1_MASK)
    {
    world_to_grid_pt (&xWorld, &yWorld) ;

    if (!(xWorld == x1Grid && yWorld == y1Grid))
      {
      // Draw the old array in order to erase it
      if (orientation == GTK_ORIENTATION_HORIZONTAL)
        qcad_cell_drexp_array (widget->window, GDK_XOR, orientation, x0Grid, x1Grid, y0Grid) ;
      else
        qcad_cell_drexp_array (widget->window, GDK_XOR, orientation, y0Grid, y1Grid, x0Grid) ;

      x1Grid = xWorld ;
      y1Grid = yWorld ;
      orientation = ABS (x0Grid - x1Grid) > ABS (y0Grid - y1Grid) ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL ;

      // Draw the new array
      if (orientation == GTK_ORIENTATION_HORIZONTAL)
        qcad_cell_drexp_array (widget->window, GDK_XOR, orientation, x0Grid, x1Grid, y0Grid) ;
      else
        qcad_cell_drexp_array (widget->window, GDK_XOR, orientation, y0Grid, y1Grid, x0Grid) ;
      }
    }

  return FALSE ;
  }

gboolean button_released_ACTION_ARRAY (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  double param[3] = {0, 0, 0} ;
  EXP_ARRAY *cells = NULL ;
  int Nix ;
#ifdef UNDO_REDO
  project_OP *project_options = (project_OP *)data ;
#endif /* def UNDO_REDO */
  QCADDesignObject *obj = NULL ;

  if (2 == event->button) return FALSE ;

  // Draw the old array in order to erase it
  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    qcad_cell_drexp_array (widget->window, GDK_XOR, orientation, param[0] = x0Grid, param[1] = x1Grid, param[2] = y0Grid) ;
  else
    qcad_cell_drexp_array (widget->window, GDK_XOR, orientation, param[0] = y0Grid, param[1] = y1Grid, param[2] = x0Grid) ;

  if (NULL != (cells = qcad_cell_create_array (orientation == GTK_ORIENTATION_HORIZONTAL, param[0], param[1], param[2])))
    {
    for (Nix = 0 ; Nix < cells->icUsed ; Nix++)
      if (!((*drop_function) (obj = QCAD_DESIGN_OBJECT (exp_array_index_1d (cells, QCADCell *, Nix)))))
        {
        g_object_unref (G_OBJECT (obj)) ;
        exp_array_remove_vals (cells, 1, Nix, 1) ;
        Nix-- ;
        }

    if (cells->icUsed > 0)
      {
      design_selection_object_array_add_weak_pointers (cells) ;
#ifdef UNDO_REDO
      push_undo_selection_existence (project_options->design, project_options->srSelection, widget->window, cells, TRUE) ;
#endif /* def UNDO_REDO */
      }
    else
      exp_array_free (cells) ;
    }

  return FALSE ;
  }
