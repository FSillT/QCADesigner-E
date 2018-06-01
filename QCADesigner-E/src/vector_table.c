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
// This file is the implementation of a vector table    //
// structure, together with a set of functions to       //
// manipulate the structure more easily.                //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vector_table.h"
#include "fileio_helpers.h"

#define DBG_VT(s)

static char *pszMagicCookie = "%%VECTOR TABLE%%" ;

#ifdef STDIO_FILEIO
static void GetVTSizes (FILE *pfile, int *picInputs, int *picVectors) ;
static gboolean CheckMagic (FILE *pfile) ;
static void ReadVector (FILE *pfile, EXP_ARRAY *vector) ;
#endif /* def STDIO_FILEIO */

int VectorTable_find_input_idx (VectorTable *pvt, QCADCell *cell)
  {
  int Nix ;

  for (Nix = 0 ; Nix < pvt->inputs->icUsed ; Nix++)
    if (exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).input == cell)
      return Nix ;

  return -1 ;
  }

VectorTable *VectorTable_new ()
  {
  VectorTable *pvt = g_malloc0 (sizeof (VectorTable)) ;

  pvt->pszFName = NULL ;
  pvt->inputs = exp_array_new (sizeof (VT_INPUT), 1) ;
  pvt->vectors = exp_array_new (sizeof (gboolean), 2) ;

  return pvt ;
  }

VectorTable *VectorTable_copy (VectorTable *pvt)
  {
  VectorTable *pvtNew = g_malloc0 (sizeof (VectorTable)) ;

  pvtNew->pszFName = g_strdup (pvt->pszFName) ;
  pvtNew->inputs = exp_array_copy (pvt->inputs) ;
  pvtNew->vectors = exp_array_copy (pvt->vectors) ;

  return pvtNew ;
  }

VectorTable *VectorTable_free (VectorTable *pvt)
  {
  g_free (pvt->pszFName) ;
  exp_array_free (pvt->inputs) ;
  exp_array_free (pvt->vectors) ;
  g_free (pvt) ;

  return NULL ;
  }

void VectorTable_empty (VectorTable *pvt)
  {
  exp_array_remove_vals (pvt->inputs, 1, 0, pvt->inputs->icUsed) ;
  exp_array_remove_vals (pvt->vectors, 1, 0, pvt->vectors->icUsed) ;
  }

// Replace the inputs in the vector table with those from design, and clean out the vectors
void VectorTable_fill (VectorTable *pvt, DESIGN *design)
  {
  DBG_VT (fprintf (stderr, "Entering VectorTable_fill\n")) ;
  VectorTable_empty (pvt) ;
  VectorTable_add_inputs (pvt, design) ;
  }

// Grab all the inputs out of the linked list starting at first_cell, and append
// them to the array of inputs (does not check for duplicates)
void VectorTable_add_inputs (VectorTable *pvt, DESIGN *design)
  {
  int Nix ;

  for (Nix = 0 ; Nix < design->bus_layout->inputs->icUsed ; Nix++)
    VectorTable_add_input (pvt, QCAD_CELL (exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, Nix).cell)) ;
  }

// Append a single input to the array and pad its corresponding vectors with 0s
void VectorTable_add_input (VectorTable *pvt, QCADCell *new_input)
  {
  VT_INPUT vti = {new_input, TRUE} ;

  DBG_VT (fprintf (stderr, "Entering VectorTable_add_input\n")) ;

  if (NULL == pvt || NULL == new_input) return ;

  exp_array_insert_vals (pvt->inputs, &vti, 1, 1, -1) ;
  // I should somehow move this loop into exp_array
  // That is, write a function that blankets a range of indices with one value
  exp_array_insert_vals (pvt->vectors, NULL, pvt->vectors->icUsed, 1, 0, -1) ;

  DBG_VT (fprintf (stderr, "Exiting VectorTable_add_input.  pvt now looks like this:\n")) ;
  DBG_VT (VectorTable_dump (pvt, stderr, 0)) ;
  }

void VectorTable_del_input (VectorTable *pvt, QCADCell *old_input)
  {
  int idx = -1, Nix ;
  DBG_VT (fprintf (stderr, "Entering VectorTable_del_input\n")) ;

  for (Nix = 0 ; Nix < pvt->inputs->icUsed ; Nix++)
    if (exp_array_index_1d (pvt->inputs, VT_INPUT, idx = Nix).input == old_input)
      exp_array_remove_vals (pvt->inputs, 1, Nix, 1) ;

  if (Nix == pvt->inputs->icUsed) return ;

  exp_array_remove_vals (pvt->vectors, 2, 0, pvt->vectors->icUsed, Nix, 1) ;

  DBG_VT (fprintf (stderr, "Exiting VectorTable_del_input.  pvt now looks like this:\n")) ;
  DBG_VT (VectorTable_dump (pvt, stderr, 0)) ;
  }

// Add a vector at idxWanted - cause idxWanted to become the new vector - meaning that the vector currently
// at idxWanted moves down one   
int VectorTable_add_vector (VectorTable *pvt, int idxWanted)
  {
  int Nix ;

  exp_array_insert_vals (pvt->vectors, NULL, pvt->inputs->icUsed, 2, idxWanted, 0) ;
  for (Nix = 0 ; Nix < pvt->inputs->icUsed ; Nix++)
    exp_array_index_2d (pvt->vectors, gboolean, idxWanted, Nix) = FALSE ;

  return idxWanted ;
  }

// Delete a vector, and bring the vectors below it up by one
void VectorTable_del_vector (VectorTable *pvt, int idx)
  {
  DBG_VT (fprintf (stderr, "Entering VectorTable_del_vector\n")) ;

  exp_array_remove_vals (pvt->vectors, 1, idx, 1) ;

  DBG_VT (fprintf (stderr, "Exiting VectorTable_del_vector\n")) ;
  }

// Write the vector table to the file whose name is contained in the VectorTable structure
gboolean VectorTable_save (VectorTable *pvt)
  {
  int Nix, Nix1 ;
  FILE *pfile = NULL ;

  if (NULL == pvt->pszFName) return FALSE ;

  pfile = fopen (pvt->pszFName, "w") ;

  DBG_VT (fprintf (stderr, "Entering VectorTable_save\n")) ;

  if (NULL == pfile)
    {
    fprintf (stderr, "Unable to open vector table file.\n") ;
    return FALSE ;
    }

  fprintf (pfile, "%s\n", pszMagicCookie) ;

  fprintf (pfile,
    "# This is a vector table file.  All text beginning with a '#' and up to the\n"
    "# end of the line will be ignored.  The first vector is the list of active\n"
    "# inputs.  The inputs this vector table was constructed for are listed below\n"
    "# from Most Significant Bit to Least Significant Bit:\n") ;

  for (Nix = 0 ; Nix < pvt->inputs->icUsed ; Nix++)
    fprintf (pfile, "# %s\n", qcad_cell_get_label (QCAD_CELL (exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).input))) ;

  fprintf (pfile, "# The following vector is the active input mask.\n") ;

  for (Nix = 0 ; Nix < pvt->inputs->icUsed ; Nix++)
    fprintf (pfile, "%d", exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).active_flag ? 1 : 0) ;
  fprintf (pfile, "\n") ;

  fprintf (pfile, "# Here are the vectors:\n") ;

  for (Nix = 0 ; Nix < pvt->vectors->icUsed ; Nix++)
    {
    for (Nix1 = 0 ; Nix1 < pvt->inputs->icUsed ; Nix1++)
      fprintf (pfile, "%d", exp_array_index_2d (pvt->vectors, gboolean, Nix, Nix1) ? 1 : 0) ;
    fprintf (pfile, "\n") ;
    }

  fclose (pfile) ;

  DBG_VT (fprintf (stderr, "Exiting VectorTable_save\n")) ;

  return TRUE ;
  }

#ifdef STDIO_FILEIO
// Fill in the VectorTable structure passed to me from the file whose name
// is contained within the structure
VTL_RESULT VectorTable_load (VectorTable *pvt)
  {
  int icInputs = 0, icVectors = 0, Nix ;
  FILE *pfile = fopen (pvt->pszFName, "r") ;
  VTL_RESULT ret = VTL_OK ;
  EXP_ARRAY *vector = NULL ;

  if (NULL == pfile) return VTL_FILE_FAILED ; 
  if (!CheckMagic (pfile)) return VTL_MAGIC_FAILED ;

  GetVTSizes (pfile, &icInputs, &icVectors) ;

  ret = icInputs < pvt->inputs->icUsed ? VTL_SHORT :
      	icInputs > pvt->inputs->icUsed ? VTL_TRUNC : VTL_OK ;

  SkipPast (pfile, '#', pszMagicCookie, NULL) ;

  //Read active flags
  ReadVector (pfile, vector = exp_array_new (sizeof (gboolean), 1)) ;
  for (Nix = 0 ; Nix < MIN (vector->icUsed, pvt->inputs->icUsed) ; Nix++)
    exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).active_flag = 
      exp_array_index_1d (vector, gboolean, Nix) ;

  // Remove all vectors
  exp_array_remove_vals (pvt->vectors, 2, 0, pvt->vectors->icUsed, 0, pvt->inputs->icUsed) ;

  while (TRUE)
    {
    ReadVector (pfile, vector) ;
    if (0 == vector->icUsed) break ;

    exp_array_insert_vals (pvt->vectors, NULL, pvt->inputs->icUsed, 2, -1, 0) ;

    for (Nix = 0 ; Nix < MAX (pvt->inputs->icUsed, vector->icUsed) ; Nix++)
      if (Nix >= pvt->inputs->icUsed)
        break ;
      else
        exp_array_index_2d (pvt->vectors, gboolean, -1, Nix) = 
          (Nix < vector->icUsed) ? exp_array_index_1d (vector, gboolean, Nix) : FALSE ;
    }

  exp_array_free (vector) ;

  fclose (pfile) ;

  return ret ;
  }
#endif /* def STDIO_FILEIO */

// Make the vector table structure appear in pfile - mostly for debugging
void VectorTable_dump (VectorTable *pvt, FILE *pfile, int icIndent)
  {
  int Nix, Nix1 ;

  int icStrlen = 8 ;

  for (Nix = 0 ; Nix < pvt->inputs->icUsed ; Nix++)
    icStrlen = MAX (icStrlen, 
      strlen (NULL == (qcad_cell_get_label (QCAD_CELL (exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).input))) ? "" : 
                      (qcad_cell_get_label (QCAD_CELL (exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).input))))) ;

  fprintf (pfile, "%*spvt->pszFName = \"%s\"\n", icIndent, "", pvt->pszFName) ;
  fprintf (pfile, "%*spvt->inputs:%d:\n%*s", icIndent, "", pvt->inputs->icUsed, icIndent + 2, "") ;
  for (Nix = 0 ; Nix < pvt->inputs->icUsed ; Nix++)
    fprintf (pfile, "%*s  ", icStrlen, qcad_cell_get_label (QCAD_CELL (exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).input))) ;
  fprintf (pfile, "\n%*s", icIndent + 2, "") ;
  for (Nix = 0 ; Nix < pvt->inputs->icUsed ; Nix++)
    fprintf (pfile, "%*s  ", icStrlen, exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).active_flag ? "1" : "0") ;
  fprintf (pfile, "\n%*spvt->vectors:%d:\n", icIndent + 2, "", pvt->vectors->icUsed) ;
  fprintf (pfile, "\n%*s", icIndent + 2, "") ;

  for (Nix = 0 ; Nix < pvt->vectors->icUsed ; Nix++)
    {
    for (Nix1 = 0 ; Nix1 < pvt->inputs->icUsed ; Nix1++)
      fprintf (pfile, "%*s  ", icStrlen, exp_array_index_2d (pvt->vectors, gboolean, Nix, Nix1) ? "1" : "0") ;
    if (Nix < pvt->vectors->icUsed - 1)
      fprintf (pfile, "\n%*s", icIndent + 2, "") ;
    }
  fprintf (pfile, "\n") ; 
  }

void VectorTable_update_inputs (VectorTable *pvt, QCADCell *pqc)
  {
  int Nix ;

  for (Nix = 0 ; Nix < pvt->inputs->icUsed ; Nix++)
    if (exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).input == pqc && pqc->cell_function != QCAD_CELL_INPUT)
      {
      VectorTable_del_input (pvt, pqc) ;
      return ;
      }

  if (Nix == pvt->inputs->icUsed && QCAD_CELL_INPUT == pqc->cell_function)
    VectorTable_add_input (pvt, pqc) ;
  }

#ifdef STDIO_FILEIO
// Check the magic string at the top of the vector file, without moving the file pointer
static gboolean CheckMagic (FILE *pfile)
  {
  int cb = ftell (pfile) ;
  char *psz = NULL ;
  gboolean bRet = FALSE ;

  fseek (pfile, 0, SEEK_SET) ;
  psz = ReadLine (pfile, '#', FALSE) ;
  bRet = !strcmp (psz, pszMagicCookie) ;
  g_free (psz) ;
  fseek (pfile, cb, SEEK_SET) ;

  return bRet ;
  }

// Run through the file, measure the width of vectors, and count them
static void GetVTSizes (FILE *pfile, int *picInputs, int *picVectors)
  {
  int Nix ;
  int cb = ftell (pfile) ;
  char *psz = NULL ;

  fseek (pfile, 0, SEEK_SET) ;

  *picInputs = *picVectors = 0 ;

  // Count the inputs
  while (!feof (pfile))
    {
    if (NULL == (psz = ReadLine (pfile, '#', FALSE))) break ;

    for (Nix = 0 ; Nix < strlen (psz) ; Nix++)
      if ('0' == psz[Nix] || '1' == psz[Nix])
        (*picInputs)++ ;

    g_free (psz) ;

    if (0 != *picInputs)
      break ;
    }

  // Count the vectors
  while (!feof (pfile))
    {
    if (NULL == (psz = ReadLine (pfile, '#', FALSE))) break ;

    for (Nix = 0 ; Nix < strlen (psz) ; Nix++)
      if ('0' == psz[Nix] || '1' == psz[Nix])
      	{
        (*picVectors)++ ;
        break ;
        }
    g_free (psz) ;
    }

  fseek (pfile, cb, SEEK_SET) ;
  }

// Read in a single vector from the file
static void ReadVector (FILE *pfile, EXP_ARRAY *vector)
  {
  int Nix ;
  char *psz ;

  if (NULL == vector) return ;
  exp_array_remove_vals (vector, 1, 0, vector->icUsed) ;

  while (!feof (pfile))
    {
    if (NULL == (psz = ReadLine (pfile, '#', FALSE))) return ;

    for (Nix = 0 ; Nix < strlen (psz) ; Nix++)
      {
      if ('0' == psz[Nix] || '1' == psz[Nix])
        {
        exp_array_insert_vals (vector, NULL, 1, 1, -1) ;
        exp_array_index_1d (vector, gboolean, -1) = ('1' == psz[Nix]) ;
        }
      }

    g_free (psz) ;

    if (vector->icUsed > 0) break ;
    }
  }
#endif /* def STDIO_FILEIO */
