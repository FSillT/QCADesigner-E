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
// Header file for an "expanding array" implementation. //
// Unlike GArray, this array does not shrink. Its size  //
// at any given time is equal to its size during peak   //
// usage.                                               //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _EXP_ARRAY_H_
#define _EXP_ARRAY_H_

#include <stdio.h>
#include <glib.h>

typedef struct
  {
  void *data ;
  int icUsed ;
  int icAvail ;
  int cbSize ;
  int icDimensions ;
  } EXP_ARRAY ;

EXP_ARRAY *exp_array_new (int cbElementSize, int icDimensions) ;
EXP_ARRAY *exp_array_copy (EXP_ARRAY *exp_array) ;
EXP_ARRAY *exp_array_free (EXP_ARRAY *exp_array) ;
void exp_array_insert_vals (EXP_ARRAY *exp_array, void *data, int icElements, int iDimension, ...) ;
void exp_array_remove_vals (EXP_ARRAY *exp_array, int icDimPairs, ...) ;
void exp_array_dump (EXP_ARRAY *exp_array, FILE *pfile, int icIndent) ;
void print_hex_bytes (char *bytes, int icBytes, int icInitBytes, int icCols, FILE *pfile, int icIndent) ;
guint exp_array_crc32 (EXP_ARRAY *exp_array) ;

//#define exp_array_index_1d(a,t,i) (((t*) (a)->data) [(i)])
//#define exp_array_index_2d(a,t,r,c) (((t*) (exp_array_index_1d((a),EXP_ARRAY *,(r)))->data) [(c)])
#define exp_array_index_1d(a,t,i) (((t*) (a)->data) [((-1 == (i)) ? (((a)->icUsed) - 1) : (i))])
#define exp_array_index_2d(a,t,r,c) (((t*) (exp_array_index_1d((a),EXP_ARRAY *,(r)))->data) [((-1 == (c)) ? (((exp_array_index_1d((a),EXP_ARRAY *,(r)))->icUsed) - 1) : (c))])

#endif /* _EXP_ARRAY_H_ */
