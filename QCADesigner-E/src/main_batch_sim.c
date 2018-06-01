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
// A batch mode simulator aimed specifically at         //
// verifying outputs against their corresponding        //
// inputs.                                              //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "fileio.h"
#include "design.h"
#include "graph_dialog.h"
#include "global_consts.h"
#include "simulation.h"
#include "simulation_data.h"
#include "coherence_vector.h"
#include "coherence_vector_energy.h"
#include "graph_dialog_widget_data.h"
#include "bistable_simulation.h"

extern bistable_OP bistable_options ;
extern coherence_OP coherence_options ;
extern coherence_energy_OP coherence_energy_options ;

static void randomize_design_cells (GRand *rnd, DESIGN *design, double dMinRadius, double dMaxRadius) ;

static int determine_success (HONEYCOMB_DATA *hcIn, HONEYCOMB_DATA *hcOut) ;

static void parse_cmdline (int argc, char **argv, int *sim_engine, char **pszSimOptsFName, char **pszFName, int *number_of_sims, double *dTolerance) ;

int main (int argc, char **argv)
  {
  static char *pszSimEngine = NULL, *pszSimOptsFName = NULL, *pszFName = NULL ;
  int number_of_sims = -1 ;

  int sim_engine = BISTABLE ;
  int Nix, Nix1, Nix2 ;
  DESIGN *design = NULL, *working_design = NULL ;
  simulation_data *sim_data = NULL ;
  GRand *rnd = NULL ;
  GdkColor clr = {0, 0, 0, 0} ;
  EXP_ARRAY *input_hcs = NULL, *output_hcs = NULL, *hc_ar = NULL ;
  HONEYCOMB_DATA *hc = NULL ;
  BUS *bus = NULL ;
  struct TRACEDATA *the_trace = NULL ;
  double dTolerance = -1.0 ;
  EXP_ARRAY *icSuccesses = NULL ;
  int icOutputBuses = 0 ;

  parse_cmdline (argc, argv, &sim_engine, &pszSimOptsFName, &pszFName, &number_of_sims, &dTolerance) ;

#ifdef GTK_GUI
  gtk_init (&argc, &argv) ;
#else
  g_type_init () ;
#endif /* def GTK_GUI */

  if (pszSimEngine != NULL)
    sim_engine =
      !strncmp (pszSimEngine, "BISTABLE", sizeof ("BISTABLE") - 1)
        ? BISTABLE
        : !strncmp (pszSimEngine, "COHERENCE_VECTOR", sizeof ("COHERENCE_VECTOR") - 1)
          ? COHERENCE_VECTOR
          : !strncmp (pszSimEngine, "COHERENCE_VECTOR_ENERGY", sizeof ("COHERENCE_VECTOR_ENERGY") - 1)
	    ? COHERENCE_VECTOR_ENERGY
	    : BISTABLE /* default */ ;

  if (!open_project_file (pszFName, &design))
    {
    fprintf (stderr, "Failed to open the circuit file !\n") ;
    return 3 ;
    }
  else
  if (NULL == design)
    {
    fprintf (stderr, "Failed to open the circuit file !\n") ;
    return 3 ;
    }

  if (BISTABLE == sim_engine)
    {
    bistable_OP *bo = NULL ;

    if (NULL == (bo = open_bistable_options_file (pszSimOptsFName)))
      {
      fprintf (stderr, "Failed to open simulation options file !\n") ;
      return 2 ;
      }
    bistable_options_dump (bo, stderr) ;
    memcpy (&bistable_options, bo, sizeof (bistable_OP)) ;
    }
  else
  if (COHERENCE_VECTOR == sim_engine)
    {
    coherence_OP *co = NULL ;

    if (NULL == (co = open_coherence_options_file (pszSimOptsFName)))
      {
      fprintf (stderr, "Failed to open simulation options file !\n") ;
      return 2 ;
      }
    coherence_options_dump (co, stderr) ;
    memcpy (&coherence_options, co, sizeof (coherence_OP)) ;
    }
    
  if (COHERENCE_VECTOR_ENERGY == sim_engine)
    {
    coherence_energy_OP *co = NULL ;

    if (NULL == (co = open_coherence_energy_options_file (pszSimOptsFName)))
      {
      fprintf (stderr, "Failed to open simulation options file !\n") ;
      return 2 ;
      }
    coherence_energy_options_dump (co, stderr) ;
    memcpy (&coherence_energy_options, co, sizeof (coherence_energy_OP)) ;
    }

  printf ("Running %d simulations with a radial tolerance of %lf\n", number_of_sims, dTolerance) ;

  rnd = g_rand_new () ;

  icSuccesses = exp_array_new (sizeof (int), 1) ;
  for (Nix = 0 ; Nix < design->bus_layout->buses->icUsed ; Nix++)
    if (QCAD_CELL_OUTPUT == exp_array_index_1d (design->bus_layout->buses, BUS, Nix).bus_function)
      icOutputBuses++ ;
  exp_array_insert_vals (icSuccesses, NULL, icOutputBuses, 1, 0) ;
  for (Nix = 0 ; Nix < icSuccesses->icUsed ; Nix++)
    exp_array_index_1d (icSuccesses, int, Nix) = 0 ;

  for (Nix = 0 ; Nix < number_of_sims ; Nix++)
    {
    fprintf (stderr, "Running simulation %d\n", Nix) ;
    if (NULL != (working_design = design_copy (design)))
      {
      randomize_design_cells (rnd, working_design, 0.0, dTolerance) ;

      if (NULL != (sim_data = run_simulation (sim_engine, EXHAUSTIVE_VERIFICATION, working_design, NULL)))
        {
        input_hcs = exp_array_new (sizeof (HONEYCOMB_DATA *), 1) ;
        output_hcs = exp_array_new (sizeof (HONEYCOMB_DATA *), 1) ;
        // For each bus, create the appropriate HONEYCOMB_DATA, fill it in with
        // the TRACEDATA structures, and place each HONEYCOMB_DATA into its
        // appropriate EXP_ARRAY.
        for (Nix1 = 0 ; Nix1 < working_design->bus_layout->buses->icUsed ; Nix1++)
          {
          bus = &exp_array_index_1d (working_design->bus_layout->buses, BUS, Nix1) ;
          hc_ar = (QCAD_CELL_INPUT == bus->bus_function ? input_hcs : output_hcs) ;
          hc = honeycomb_data_new (&clr) ;
          for (Nix2 = 0 ; Nix2 < bus->cell_indices->icUsed ; Nix2++)
            {
            the_trace = &(sim_data->trace[exp_array_index_1d (bus->cell_indices, int, Nix2) + (QCAD_CELL_INPUT == bus->bus_function ? 0 : working_design->bus_layout->inputs->icUsed)]) ;
            exp_array_insert_vals (hc->arTraces, &the_trace, 1, -1) ;
            }
          calculate_honeycomb_array (hc, sim_data->number_samples, -0.5, 0.5, 2) ;
          exp_array_insert_vals (hc_ar, &hc, 1, -1) ;
          }

        // Compare the output honeycombs to the input honeycombs
        for (Nix1 = 0 ; Nix1 < output_hcs->icUsed ; Nix1++)
          exp_array_index_1d (icSuccesses, int, Nix1) +=
            determine_success (
              exp_array_index_1d (input_hcs, HONEYCOMB_DATA *, Nix1),
              exp_array_index_1d (output_hcs, HONEYCOMB_DATA *, Nix1)) ;

        // Print out the results
        for (Nix1 = 0 ; Nix1 < MAX (input_hcs->icUsed, output_hcs->icUsed) ; Nix1++)
          {
          if (Nix1 < input_hcs->icUsed)
            {
            hc = exp_array_index_1d (input_hcs, HONEYCOMB_DATA *, Nix1) ;
            fprintf (stderr, "First trace in this bus is \"%s\"\n", exp_array_index_1d (hc->arTraces, struct TRACEDATA *, 0)->data_labels) ;
            for (Nix2 = 0 ; Nix2 < hc->arHCs->icUsed ; Nix2++)
              fprintf (stderr, "%d ", (int)(exp_array_index_1d (hc->arHCs, HONEYCOMB, Nix2).value)) ;
            fprintf (stderr, "\n") ;
            }
          if (Nix1 < output_hcs->icUsed)
            {
            hc = exp_array_index_1d (output_hcs, HONEYCOMB_DATA *, Nix1) ;
            fprintf (stderr, "First trace in this bus is \"%s\"\n", exp_array_index_1d (hc->arTraces, struct TRACEDATA *, 0)->data_labels) ;
            for (Nix2 = 0 ; Nix2 < hc->arHCs->icUsed ; Nix2++)
              fprintf (stderr, "%d ", (int)(exp_array_index_1d (hc->arHCs, HONEYCOMB, Nix2).value)) ;
            fprintf (stderr, "\n") ;
            }
          }

        fprintf (stderr, "*******************\n") ;
        for (Nix1 = 0 ; Nix1 < input_hcs->icUsed ; Nix1++)
          honeycomb_data_free (exp_array_index_1d (input_hcs, HONEYCOMB_DATA *, Nix1)) ;
        input_hcs = exp_array_free (input_hcs) ;
        for (Nix1 = 0 ; Nix1 < output_hcs->icUsed ; Nix1++)
          honeycomb_data_free (exp_array_index_1d (output_hcs, HONEYCOMB_DATA *, Nix1)) ;
        output_hcs = exp_array_free (output_hcs) ;
        sim_data = simulation_data_destroy (sim_data) ;
        }
      working_design = design_destroy (working_design) ;
      }
    }

  for (Nix = 0 ; Nix < icSuccesses->icUsed ; Nix++)
    printf ("success_rate[%d] = %.2lf%%\n", Nix, (double)(exp_array_index_1d (icSuccesses, int, Nix)) / ((double)(number_of_sims)) * 100.0) ;

  g_rand_free (rnd) ;

  return 0 ;
  }

static void randomize_design_cells (GRand *rnd, DESIGN *design, double dMinRadius, double dMaxRadius)
  {
  double dRadius = -1.0, dAngle = 0.0 ;
  double dx = 0.0, dy = 0.0 ;
  GList *llItr = NULL, *llItrObj = NULL ;
  QCADLayer *layer = NULL ;

  if (NULL == rnd || NULL == design) return ;

  for (llItr = design->lstLayers ; llItr != NULL ; llItr = llItr->next)
    if (LAYER_TYPE_CELLS == (layer = QCAD_LAYER (llItr->data))->type)
      for (llItrObj = layer->lstObjs ; llItrObj != NULL ; llItrObj = llItrObj->next)
        if (NULL != llItrObj->data)
          {
          dRadius = g_rand_double_range (rnd, dMinRadius, dMaxRadius) ;
          dAngle = g_rand_double_range (rnd, 0, 2.0 * PI) ;

          dx = dRadius * cos (dAngle) ;
          dy = dRadius * sin (dAngle) ;

          qcad_design_object_move (QCAD_DESIGN_OBJECT (llItrObj->data), dx, dy) ;
          }
  }

static void parse_cmdline (int argc, char **argv, int *sim_engine, char **pszSimOptsFName, char **pszFName, int *number_of_sims, double *dTolerance)
  {
  int icParms = 0 ;
  int Nix ;

  // defaults
  (*sim_engine) = BISTABLE ;

  for (Nix = 0 ; Nix < argc ; Nix++)
    {
    if (!strncmp (argv[Nix], "-f", 2))
      {
      if (++Nix < argc)
        {
        (*pszFName) = argv[Nix] ;
        icParms++ ;
        }
      }
    else
    if (!strncmp (argv[Nix], "-e", 2))
      {
      if (++Nix < argc)
        {
        (*sim_engine) =
          !strncmp (argv[Nix], "BISTABLE", 8)
            ? BISTABLE
            : !strncmp (argv[Nix], "COHERENCE_VECTOR", 16)
              ? COHERENCE_VECTOR
              : !strncmp (argv[Nix], "COHERENCE_VECTOR_ENERGY", 23)
		? COHERENCE_VECTOR_ENERGY
		: BISTABLE /* default */ ;
        icParms++ ;
        }
      }
    else
    if (!strncmp (argv[Nix], "-o", 2))
      {
      if (++Nix < argc)
        {
        (*pszSimOptsFName) = argv[Nix] ;
        icParms++ ;
        }
      }
    else
    if (!strncmp (argv[Nix], "-n", 2))
      {
      if (++Nix < argc)
        {
        (*number_of_sims) = atoi (argv[Nix]) ;
        icParms++ ;
        }
      }
    else
    if (!strncmp (argv[Nix], "-t", 2))
      {
      if (++Nix < argc)
        {
        (*dTolerance) = g_ascii_strtod (argv[Nix], NULL) ;
        icParms++ ;
        }
      }
    }

  if (icParms < 5)
    {
    printf (
      "Usage:\n"
      "batch_sim -f qca_file [-e [BISTABLE]|COHERENCE_VECTOR|COHERENCE_VECTOR_ENERGY] -o engine_options_file -n number_of_simulations -t radial_tolerance\n") ;
    exit (1) ;
    }
  }

static int determine_success (HONEYCOMB_DATA *hcdIn, HONEYCOMB_DATA *hcdOut)
  {
  int Nix ;
  int idxIn = 0 ;
  HONEYCOMB *hcIn = NULL, *hcOut = NULL ;

  if (NULL == hcdIn || NULL == hcdOut) return 0 ;
  if (NULL == hcdIn->arHCs || NULL == hcdOut->arHCs) return 0 ;
  if (0 == hcdIn->arHCs->icUsed || 0 == hcdOut->arHCs->icUsed) return 0 ;

  hcIn = &exp_array_index_1d (hcdIn->arHCs, HONEYCOMB, 0) ;

  for (Nix = 0 ; Nix < hcdOut->arHCs->icUsed ; Nix++)
    {
    hcOut = &exp_array_index_1d (hcdOut->arHCs, HONEYCOMB, Nix) ;

    // This output honeycomb may be contained withing the next input honeycomb
    if (hcOut->idxBeg > hcIn->idxEnd)
      {
      if (++idxIn == hcdIn->arHCs->icUsed) return 0 ;
      hcIn = &exp_array_index_1d (hcdIn->arHCs, HONEYCOMB, idxIn) ;
      }

    // The output honeycomb is not entirely contained within the input honeycomb
    if (hcOut->idxBeg < hcIn->idxBeg || hcOut->idxEnd > hcIn->idxEnd)
      return 0 ;

    if (hcOut->value != hcIn->value)
      return 0 ;
    }

  return 1 ;
  }
