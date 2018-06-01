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
// Header for the callback functions for the vector     //
// table options dialog.                                //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _VECTOR_TABLE_OPTIONS_DIALOG_CALLBACKS_H_
#define _VECTOR_TABLE_OPTIONS_DIALOG_CALLBACKS_H_

#include <gtk/gtk.h>

void vector_table_options_dialog_btnOpen_clicked (GtkWidget *widget, gpointer data) ;
void vector_table_options_dialog_btnSave_clicked (GtkWidget *widget, gpointer data) ;
void vector_table_options_dialog_btnSimType_clicked (GtkWidget *widget, gpointer data) ;
void vector_table_options_dialog_btnClose_clicked (GtkWidget *widget, gpointer data) ;
void vector_table_options_dialog_btnAdd_clicked (GtkWidget *widget, gpointer data) ;
void vector_table_options_dialog_btnDelete_clicked (GtkWidget *widget, gpointer data) ;
void vt_model_active_toggled (GtkCellRenderer *cr, char *pszTreePath, gpointer data) ;
void vector_column_clicked (GtkObject *obj, gpointer data) ;
void vector_data_func (GtkTreeViewColumn *col, GtkCellRenderer *cr, GtkTreeModel *model, GtkTreeIter *itr, gpointer data) ;
void tree_view_style_set (GtkWidget *widget, GtkStyle *old_style, gpointer data) ;
void vector_value_edited (GtkCellRendererText *cr, char *pszPath, char *pszNewText, gpointer data) ;
void vector_value_editing_started (GtkCellRendererText *cr, GtkCellEditable *editable, char *pszPath, gpointer data) ;

#endif /* def _VECTOR_TABLE_OPTIONS_DIALOG_CALLBACKS_H_ */
