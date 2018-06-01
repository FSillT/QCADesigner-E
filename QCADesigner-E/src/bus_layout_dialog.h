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
// Header file for the bus layout dialog. This allows   //
// the user to group individual inputs into input       //
// buses, and individual outputs into output buses,     //
// respectively. The user's choices are encoded in a    //
// BUS_LAYOUT structure, which is part of the DESIGN    //
// structure.                                           //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _BUS_LAYOUT_DIALOG_H_
#define _BUS_LAYOUT_DIALOG_H_

#include <gtk/gtk.h>
#include "design.h"

void get_bus_layout_from_user (GtkWindow *parent, BUS_LAYOUT *bus_layout) ;
GtkWidget *create_bus_layout_tree_view (gboolean bColsVisible, char *pszColumnTitle, GtkSelectionMode sel_mode) ;
gboolean gtk_tree_model_iter_next_dfs (GtkTreeModel *model, GtkTreeIter *itr) ;
void bus_layout_tree_model_dump (GtkTreeModel *model, FILE *pfile) ;

#endif /* _BUS_LAYOUT_DIALOG_H_ */
