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
// Header for functions for creating vector table in-   //
// terface elements, including the dialog itself and    //
// adding a new vector.                                 //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _VECTOR_TABLE_OPTIONS_DIALOG_INTERFACE_H_
#define _VECTOR_TABLE_OPTIONS_DIALOG_INTERFACE_H_

#include <gtk/gtk.h>

typedef struct
  {
  GtkWidget *dialog;
  GtkWidget *tbtnVT ;
  GtkWidget *tbtnExhaustive ;
  GtkWidget *btnOpen ;
  GtkWidget *btnSave ;
  GtkWidget *btnInsert ;
  GtkWidget *btnDelete ;
  GtkWidget *btnAdd ;
  GtkWidget *tblVT ;
  GtkWidget *sw ;
  GtkWidget *tv ;
  GtkCellRenderer *crActive ;
  } vector_table_options_D ;

void create_vector_table_options_dialog (vector_table_options_D *pnvto) ;
GtkTreeViewColumn *add_vector_to_dialog (vector_table_options_D *dialog, VectorTable *pvt, int idx) ;

#endif /* _VECTOR_TABLE_OPTIONS_DIALOG_INTERFACE_H_ */
