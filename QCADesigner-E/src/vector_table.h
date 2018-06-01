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
// Header for the implementation of a vector table      //
// structure, together with a set of functions to       //
// manipulate the structure more easily.                //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _VECTOR_TABLE_H_
#define _VECTOR_TABLE_H_

#include <glib.h>
#include "objects/QCADCell.h"
#include "exp_array.h"
#include "design.h"

typedef enum
  {
  VTL_FILE_FAILED  = -3,
  VTL_MAGIC_FAILED = -2,
  VTL_SHORT        = -1,
  VTL_OK           =  0,
  VTL_TRUNC        =  1
  } VTL_RESULT ;

typedef struct
  {
  QCADCell *input ;
  gboolean active_flag ;
  } VT_INPUT ;

typedef struct
  {
  char *pszFName ;
  EXP_ARRAY *vectors ;
  EXP_ARRAY *inputs ;

  } VectorTable ;

VectorTable *VectorTable_new () ;
VectorTable *VectorTable_free (VectorTable *pvt) ;
VectorTable *VectorTable_copy (VectorTable *pvt) ;
void VectorTable_fill (VectorTable *pvt, DESIGN *design) ;
void VectorTable_add_input (VectorTable *pvt, QCADCell *new_input) ;
void VectorTable_del_input (VectorTable *pvt, QCADCell *old_input) ;
void VectorTable_add_inputs (VectorTable *pvt, DESIGN *design) ;
void VectorTable_update_inputs (VectorTable *pvt, QCADCell *pqc) ;
int VectorTable_add_vector (VectorTable *pvt, int idx) ;
void VectorTable_del_vector (VectorTable *pvt, int idx) ;
gboolean VectorTable_save (VectorTable *pvt) ;
VTL_RESULT VectorTable_load (VectorTable *pvt) ;
void VectorTable_dump (VectorTable *pvt, FILE *pfile, int icIndent) ;
void VectorTable_empty (VectorTable *pvt) ;
int VectorTable_find_input_idx (VectorTable *pvt, QCADCell *cell) ;

#endif /* _VECTOR_TABLE_H_ */
