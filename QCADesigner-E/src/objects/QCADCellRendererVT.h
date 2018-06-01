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
// Header for a hybrid text/checkbox cell renderer used //
// by the vector table tree view. The decision whether  //
// to render as text is driven by the "row-type"        //
// attribute.                                           //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADCellRendererVT_H_
#define _OBJECTS_QCADCellRendererVT_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
  {
  GtkCellRendererText parent ;

  int row_type ;
  long long value ;
#if (GTK_MINOR_VERSION <= 4)
  gboolean sensitive ;
#endif
  } QCADCellRendererVT ;

typedef struct
  {
  GtkCellRendererTextClass parent_class ;
  void (*toggled) (QCADCellRendererVT *cr, const gchar *pszPath) ;
  void (*clicked) (QCADCellRendererVT *cr) ;
  void (*editing_started) (QCADCellRendererVT *cr, GtkCellEditable *ce, char *pszPath) ;
  } QCADCellRendererVTClass ;

GType qcad_cell_renderer_vt_get_type () ;
GtkCellRenderer *qcad_cell_renderer_vt_new () ;

#define QCAD_TYPE_STRING_CELL_RENDERER_VT "QCADCellRendererVT"
#define QCAD_TYPE_CELL_RENDERER_VT (qcad_cell_renderer_vt_get_type ())
#define QCAD_CELL_RENDERER_VT(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_CELL_RENDERER_VT, QCADCellRendererVT))
#define QCAD_CELL_RENDERER_VT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_CELL_RENDERER_VT, QCADCellRendererVTClass))
#define QCAD_IS_CELL_RENDERER_VT(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_CELL_RENDERER_VT))
#define QCAD_IS_CELL_RENDERER_VT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_CELL_RENDERER_VT))
#define QCAD_CELL_RENDERER_VT_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_CELL_RENDERER_VT, QCADCellRendererVTClass))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* def _OBJECTS_QCADCellRendererVT_H_ */
