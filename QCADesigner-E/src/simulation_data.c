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
// Functions related to the simulation results data     //
// structure itself.                                    //
//                                                      //
//////////////////////////////////////////////////////////

#include "simulation_data.h"

void tracedata_get_min_max (struct TRACEDATA *trace, int idxStart, int idxEnd, double *pdMin, double *pdMax)
  {
  int Nix ;

  if (NULL == trace) return ;

  *pdMin = *pdMax = trace->data[idxStart] ;

  for (Nix = idxStart + 1 ; Nix < idxEnd ; Nix++)
    {
    if (trace->data[Nix] < *pdMin) *pdMin = trace->data[Nix] ;
    if (trace->data[Nix] > *pdMax) *pdMax = trace->data[Nix] ;
    }

  // If there is no range, use default range
  if (*pdMin == *pdMax)
    {
    *pdMin = -1 ;
    *pdMax =  1 ;
    }
  }

// This function always returns NULL
simulation_data *simulation_data_destroy (simulation_data *sim_data)
  {
  int i;

  if (NULL == sim_data) return NULL;

  for (i = 0; i < sim_data->number_of_traces; i++)
    {
    g_free (sim_data->trace[i].data);
    g_free (sim_data->trace[i].data_labels);
    }

  g_free (sim_data->trace);

  if (NULL != sim_data->clock_data)
    {
    for (i = 0; i < sim_data->number_of_zones; i++)
      {
      g_free (sim_data->clock_data[i].data);
      g_free (sim_data->clock_data[i].data_labels);
      }

    g_free (sim_data->clock_data);
    }

  g_free (sim_data);
  return NULL ;
  }
