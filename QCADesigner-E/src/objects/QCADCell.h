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
// Header for the QCA cell.                             //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADCell_H_
#define _OBJECTS_QCADCell_H_

#include <glib-object.h>
#include "../gdk_structs.h"
#include "../exp_array.h"
#include "QCADDesignObject.h"
#include "QCADLabel.h"
#include "QCADUndoEntry.h"
#include "mouse_handler_struct.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CLOCK_INC(c) (((c)+1)%4)
#define CLOCK_DEC(c) (0==(c)?3:(c)-1)

typedef enum
  {
  QCAD_CELL_MODE_NORMAL,
  QCAD_CELL_MODE_CROSSOVER,
  QCAD_CELL_MODE_VERTICAL
  } QCADCellMode ;

typedef enum
  {
  QCAD_CELL_NORMAL,
  QCAD_CELL_INPUT,
  QCAD_CELL_OUTPUT,
  QCAD_CELL_FIXED,
  QCAD_CELL_LAST_FUNCTION
  } QCADCellFunction ;

typedef struct
  {
  double cxCell ;
  double cyCell ;
  double dot_diameter ;
  int clock ;
  int mode ;
  gboolean ignore_energy; // ignore energy dissipation
  } QCADCellOptions ;

typedef struct
  {
  // absolute world qdot coords //
  double x;
  double y;

  // qdot diameter //
  double diameter;

  // qdot charge //
  double charge;

  // quantum spin of charge within dot //
  float spin;

  // electrostatic potential induced by all other cells on this dot
  double potential;
  } QCADCellDot;

typedef struct
  {
  QCADDesignObject parent_instance ;
  int id ;
  QCADCellOptions cell_options ;
  QCADCellFunction cell_function ;
  void *cell_model ;
  QCADCellDot *cell_dots ;
  int number_of_dots ;
  QCADLabel *label ;
  gboolean bLabelRemoved ;
  char *host_name ;  
  } QCADCell ;

typedef struct
  {
  /* public */
  QCADDesignObjectClass parent_class ;
  QCADCellOptions default_cell_options ;

  /* signals */
  void (*cell_function_changed) (GObject *object, gpointer user_data) ;
  } QCADCellClass ;

GType qcad_cell_get_type () ;
GType qcad_cell_function_get_type () ;
GType qcad_cell_mode_get_type () ;


#define QCAD_TYPE_STRING_CELL "QCADCell"
#define QCAD_TYPE_CELL (qcad_cell_get_type ())
#define QCAD_CELL(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_CELL, QCADCell))
#define QCAD_CELL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_CELL, QCADCellClass))
#define QCAD_IS_CELL(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_CELL))
#define QCAD_IS_CELL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_CELL))
#define QCAD_CELL_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_CELL, QCADCellClass))

#define QCAD_TYPE_STRING_CELL_FUNCTION "QCADCellFunction"
#define QCAD_TYPE_CELL_FUNCTION (qcad_cell_function_get_type ())

#define QCAD_TYPE_STRING_CELL_MODE "QCADCellMode"
#define QCAD_TYPE_CELL_MODE (qcad_cell_mode_get_type ())

///////////////////////////////////////////////////////////////////////////////

// World coordinates
QCADDesignObject *qcad_cell_new (double x, double y) ;
QCADDesignObject *qcad_cell_new_with_function (QCADCellFunction cell_function, char *pszLabel) ;
#ifdef GTK_GUI
void qcad_cell_drexp_array (GdkDrawable *dst, GdkFunction rop, GtkOrientation orientation, double dRangeBeg, double dRangeEnd, double dOtherCoord) ;
#endif /* def GTK_GUI */
EXP_ARRAY *qcad_cell_create_array (gboolean bHorizontal, double dRangeBeg, double dRangeEnd, double dOtherCoord) ;
double qcad_cell_calculate_polarization (QCADCell *cell) ;

void qcad_cell_set_polarization (QCADCell *cell, double dPolarization) ;
void qcad_cell_set_clock (QCADCell *cell, int iClock) ;
void qcad_cell_set_display_mode (QCADCell *cell, int display_mode) ;
void qcad_cell_set_function (QCADCell *cell, QCADCellFunction function) ;
void qcad_cell_set_label (QCADCell *cell, char *pszLabel) ;
void qcad_cell_set_host_name (QCADCell *cell, char *pszHostName) ;
void qcad_cell_rotate_dots (QCADCell *cell, double angle) ;
const char *qcad_cell_get_label (QCADCell *cell) ;
void qcad_cell_scale (QCADCell *cell, double dScale, double dxOrigin, double dyOrigin) ;
void qcad_cell_set_ignore_energy (QCADCell *cell, gboolean ignore_energy);


#ifdef __cplusplus
}
#endif
#endif /* _OBJECTS_QCADCell_H_ */
