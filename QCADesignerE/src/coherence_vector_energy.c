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
// Contents:                                            //
//                                                      //
// The coherence vector time-dependent simulation       //
// engine.                                              //
//                                                      //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// MOD_PN+:FST                                          //
//                                                      //
// Estiation of Energy dissipation added		//
// Version: 24.may.2017                                 //
//     	-modified recording based on clk1               //
//	-added "no option" for cells whose power shall  //
//	 not be recorded				//
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "objects/QCADCell.h"
#include "simulation.h"
#include "coherence_vector_energy.h"
#include "custom_widgets.h"
#include "global_consts.h"
#ifdef GTK_GUI
  #include "callback_helpers.h"
#endif /* def GTK_GUI */

// Calculates the magnitude of the 3D energy vector
#define magnitude_energy_vector(P,G) (hypot(2*(G), (P)) * over_hbar)
//(sqrt((4.0*(G)*(G) + (P)*(P))*over_hbar_sqr))
// Calculates the temperature ratio
#define temp_ratio(P,G,T) (hypot((G),(P)*0.5)/((T) * kB))

#define PI 3.14159
#define TWO_PI 6.283185
#define FOUR_PI 12.56637061

// distance between two cells = grid_size
#define CELL_DIST 20

//!Options for the coherence simulation engine
coherence_energy_OP coherence_energy_options = {1, 1e-15, 1e-16, 50e-12, 9.8e-22, 3.8e-23, 0.0, 4.0e-12, 4.0e-12, 1e-12, 80, 12.9, 11.5, EULER_METHOD, GAUSS, TRUE, FALSE, TRUE, FALSE, -1, -1} ;

typedef struct
  {
  int number_of_neighbours;
  QCADCell **neighbours;
  int *neighbour_layer;
  double *Ek;
  double lambda_x;
  double lambda_y;
  double lambda_z;
  
  // FST: Energy 
  double diss_bath; //dissipation to bath
  double diss_clk; // dissipation to clk
  double diss_io;
  double diss_in; // dissipation in  =>  LEFT side of cell (top/down is ignored)
  double diss_out; //dissipation out => RIGHT side
  double Old_Gamma;
  double Old_PEk_io;
  double Old_PEk_in;
  double Old_PEk_out;  
  double *Int_diss_bath; // integral over Energy Dissipations
  double *Int_diss_clk;
  double *Int_diss_io;
  double *Int_diss_in;
  double *Int_diss_out;
  int diss_idx;
  double old_next_clock_zone;  
  // FST: end  
  } coherence_model;

//FST
static FILE *diss_trace_file; // handle to file that shall receive trace => will be "Diss.trace"
static double old_clock[4] = {0,0,0,0}; // determination of old clk values for definition of input signal value
static int zero_mode[4] = {0,0,0,0}; // definition if input of clk_zone 0-3 is zeroed or not (during simulation)


//FST end 
  
  
#ifdef GTK_GUI
extern int STOP_SIMULATION;
#else
static int STOP_SIMULATION = 0 ;
#endif /* def GTK_GUI */

// some often used variables that can be precalculated
typedef struct
  {
  double clock_prefactor;
  double clock_shift;
  double clock_middle;
  double four_pi_over_number_samples;
  double two_pi_over_number_samples;
  double hbar_over_kBT;
  double slope_factor; // rise/fall factor of clock if RAMP is selected
  double sigma; // sigma if GAUSS is selected
  } coherence_optimizations;
  

// instance of the optimization options;
static coherence_optimizations optimization_options;

static double coherence_determine_Ek (QCADCell *cell1, QCADCell *cell2, int layer_separation, coherence_energy_OP *options);
static void coherence_refresh_all_Ek (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, coherence_energy_OP *options);
static void run_coherence_iteration (unsigned long int sample_number, unsigned long int number_samples_per_period, int clock_cycles, int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, int total_number_of_inputs, unsigned long int number_samples, const coherence_energy_OP *options, simulation_data *sim_data, int SIMULATION_TYPE, VectorTable *pvt);
static inline double lambda_ss_x (double t, double PEk, double Gamma, const coherence_energy_OP *options);
static inline double lambda_ss_y (double t, double PEk, double Gamma, const coherence_energy_OP *options);
static inline double lambda_ss_z (double t, double PEk, double Gamma, const coherence_energy_OP *options);
static inline double lambda_x_next (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_energy_OP *options);
static inline double lambda_y_next (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_energy_OP *options);
static inline double lambda_z_next (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_energy_OP *options);
static inline double slope_x (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_energy_OP *options);
static inline double slope_y (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_energy_OP *options);
static inline double slope_z (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_energy_OP *options);
static int compareCoherenceQCells (const void *p1, const void *p2) ;
static inline double calculate_clock_value (unsigned int clock_num, unsigned long int sample, unsigned long int number_samples_per_period, const coherence_energy_OP *options);
static void coherence_initialize_all_diss (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, coherence_energy_OP *options, int clock_cycles);
static inline double calculate_diss_bath(double lambda_x, double lambda_z, double Gamma, double PEk, const coherence_energy_OP *options);
static inline double calculate_diss_clk(double lambda_x, double Gamma, double Old_Gamma, double time_step);
static inline double calculate_diss_io(double lambda_z, double PEk, double Old_PEk, double time_step);


//-------------------------------------------------------------------//
// -- this is the main simulation procedure -- //
//-------------------------------------------------------------------//
simulation_data *run_coherence_energy_simulation (int SIMULATION_TYPE, DESIGN *design, coherence_energy_OP *options, VectorTable *pvt)
  {
  int i, j, k, l, q, number_of_cell_layers, *number_of_cells_in_layer;
  QCADCell ***sorted_cells = NULL ;
  int total_number_of_inputs = design->bus_layout->inputs->icUsed;
  unsigned long int sample;
  unsigned long int number_samples;
  //number of points to record in simulation results //
  //simulations can have millions of points and there is no need to plot them all //
  unsigned long int number_recorded_samples = 3000;
  unsigned long int record_interval;
  double PEk = 0;
  gboolean stable;
  double old_lambda_x;
  double old_lambda_y;
  double old_lambda_z;
  time_t start_time, end_time;
  simulation_data *sim_data = NULL ;
  // for randomization
  int Nix, Nix1, idxCell1, idxCell2 ;
  QCADCell *swap = NULL ;
  BUS_LAYOUT_ITER bli ;
  double dPolarization = 2.0 ;
  int idxMasterBitOrder = -1.0 ;
  unsigned long int number_samples_per_period = (unsigned long int)ceil ((double)(options->clock_period) / (double)(options->time_step));
  unsigned long int number_samples_per_in_period = (unsigned long int)ceil ((double)(options->input_period) / (double)(options->time_step));
    
  int clock_cycles = (int) ceil( (double) options->duration / (double) options->clock_period );
    
  STOP_SIMULATION = FALSE;
  
  //FST: open file
  if ((diss_trace_file = fopen ("Diss.trace", "w+")) == NULL) {
    printf("Diss.trace cann't be open!\n");
    exit(1);
  }
  //FST_end

  // -- get the starting time for the simulation -- //
  if ((start_time = time (NULL)) < 0)
    fprintf (stderr, "Could not get start time\n");

  // determine the number of samples from the user options //
  number_samples = (unsigned long int)(ceil (options->duration/options->time_step));
  
  // if the number of samples is larger then the number of recorded samples then change the
  // time step to ensure only number_recorded_samples is used //
  if (number_recorded_samples >= number_samples)
    {
    number_recorded_samples = number_samples;
    record_interval = 1;
    }
  else
    record_interval = (unsigned long int)ceil ((double)(number_samples - 1) / (double)(number_recorded_samples));

  //fill in some of the optimizations
  optimization_options.clock_prefactor = (options->clock_high - options->clock_low);
  optimization_options.clock_shift = (options->clock_high + options->clock_low) * 0.5;
  optimization_options.clock_middle = (options->clock_high + options->clock_low) * 0.5;
  optimization_options.four_pi_over_number_samples = FOUR_PI / (double)number_samples;
  optimization_options.two_pi_over_number_samples = TWO_PI / (double)number_samples;
  optimization_options.hbar_over_kBT = hbar / (kB * options->T);
  optimization_options.slope_factor = (options->clock_high - options->clock_low) / (options->t_slope_ramp) * options->time_step;
  optimization_options.sigma = options->t_slope_ramp / (3.0 * options->time_step);
  
  // -- spit out some messages for possible debugging -- //
  command_history_message ("About to start the coherence vector energy simulation with %d samples\n", number_samples);
  command_history_message ("%d samples will be recorded for graphing.\n", number_recorded_samples);
  set_progress_bar_visible (TRUE) ;
  set_progress_bar_label ("Coherence vector energy simulation:") ;
  set_progress_bar_fraction (0.0) ;

  // Fill in the cell arrays necessary for conducting the simulation
  simulation_inproc_data_new (design, &number_of_cell_layers, &number_of_cells_in_layer, &sorted_cells) ;

  // determine which cells are inputs and which are outputs //
  for(i = 0; i < number_of_cell_layers; i++)
    for(j = 0; j < number_of_cells_in_layer[i]; j++)
      {
      // attach the model parameters to each of the simulation cells //
      sorted_cells[i][j]->cell_model = g_malloc0 (sizeof(coherence_model));

      // -- Clear the model pointers so they are not dangling -- //
      ((coherence_model *)sorted_cells[i][j]->cell_model)->neighbours = NULL;
      ((coherence_model *)sorted_cells[i][j]->cell_model)->Ek = NULL;
    
      
      }

  // if we are performing a vector table simulation we consider only the activated inputs //
  if (VECTOR_TABLE == SIMULATION_TYPE)
    for (total_number_of_inputs = 0, Nix = 0 ; Nix < pvt->inputs->icUsed ; Nix++)
      {
      if (exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).active_flag)
        total_number_of_inputs++ ;
      else
        // Kill the input flag for inactive inputs, so they may be correctly simulated
        exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).input->cell_function = QCAD_CELL_NORMAL ;
      }

  // write message to the codissmmand history window //
  command_history_message ("Simulation found %d inputs %d outputs\n", total_number_of_inputs, design->bus_layout->outputs->icUsed) ;

  // -- Allocate memory to hold the simulation data -- //
  sim_data = g_malloc0 (sizeof(simulation_data)) ;

  // -- Initialize the simualtion data structure -- //
  // FST was: sim_data->number_of_traces = design->bus_layout->inputs->icUsed + design->bus_layout->outputs->icUsed;
  sim_data->number_of_traces = design->bus_layout->inputs->icUsed + 4*design->bus_layout->outputs->icUsed;

  // set the number of simulation samples to be the desired number of recorded samples //
  sim_data->number_samples = number_recorded_samples;

  // allocate the memory for each trace //
  sim_data->trace = g_malloc0 (sizeof (struct TRACEDATA) * sim_data->number_of_traces);

  // create and initialize the inputs into the sim data structure //
  for (i = 0; i < design->bus_layout->inputs->icUsed; i++)
    {
    sim_data->trace[i].data_labels = g_strdup (qcad_cell_get_label (exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, i).cell));
    sim_data->trace[i].drawtrace = TRUE;
    sim_data->trace[i].trace_function = QCAD_CELL_INPUT;
    sim_data->trace[i].data = g_malloc0 (sim_data->number_samples * sizeof (double));
    }

  // create and initialize the outputs into the sim data structure //
  for (i = 0; i < design->bus_layout->outputs->icUsed; i++)
    { 
    int j = i + total_number_of_inputs;   	
    sim_data->trace[j].data_labels = g_strdup (qcad_cell_get_label (exp_array_index_1d (design->bus_layout->outputs, BUS_LAYOUT_CELL, i).cell));
    sim_data->trace[j].drawtrace = TRUE;
    sim_data->trace[j].trace_function = QCAD_CELL_OUTPUT;
    sim_data->trace[j].data = g_malloc0 (sim_data->number_samples * sizeof (double));
   }

  // create and initialize the clock data //
  sim_data->clock_data = g_malloc0 (sizeof (struct TRACEDATA) * 4);

  for (i = 0; i < 4; i++)
    {
    sim_data->clock_data[i].data_labels = g_strdup_printf ("CLOCK %d", i);
    sim_data->clock_data[i].drawtrace = 1;
    sim_data->clock_data[i].trace_function = QCAD_CELL_FIXED;
    if (NULL == (sim_data->clock_data[i].data = g_malloc0 (sim_data->number_samples * sizeof (double))))
      printf("Could not allocate memory for clock data\n");

    // fill in the clock data for the simulation results //
    for (j = 0; j<sim_data->number_samples; j++)
      //printf("j=%d, j*record_interval = %d\n",j,j*record_interval);
      sim_data->clock_data[i].data[j] = calculate_clock_value(i, j * record_interval, number_samples_per_period, options);
    }

  // -- refresh all the kink energies and neighbours-- //
  coherence_refresh_all_Ek (number_of_cell_layers, number_of_cells_in_layer, sorted_cells, options);
  
  // FST: initialize all Diss
  coherence_initialize_all_diss (number_of_cell_layers, number_of_cells_in_layer, sorted_cells, options, clock_cycles);

  // -- sort the cells with respect to the neighbour count -- //
  // -- this is done so that majority gates are evalulated last -- //
  // -- to ensure that all the signals have arrived first -- //
  // -- kept getting wrong answers without this -- //

  // The following line causes a segfault when the design consists of a single cell
//  printf("The Ek to the first cells neighbour is %e [eV]\n",((coherence_model *)sorted_cells[0][0]->cell_model)->Ek[0]/1.602e-19);

  // randomize the cells in the design as to minimize any numerical problems associated //
  // with having cells simulated in some predefined order: //
  // for each layer ...
  for (Nix = 0 ; Nix < number_of_cell_layers ; Nix++)
    // ...perform as many swaps as there are cells therein
    for (Nix1 = 0 ; Nix1 < number_of_cells_in_layer[Nix] ; Nix1++)
      {
      idxCell1 = rand () % number_of_cells_in_layer[Nix] ;
      idxCell2 = rand () % number_of_cells_in_layer[Nix] ;

      swap = sorted_cells[Nix][idxCell1] ;
      sorted_cells[Nix][idxCell1] = sorted_cells[Nix][idxCell2] ;
      sorted_cells[Nix][idxCell2] = swap ;
      }

  if (EXHAUSTIVE_VERIFICATION == SIMULATION_TYPE)
    for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
      qcad_cell_set_polarization (exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, i).cell, 
        sim_data->trace[i].data[0] = -1) ;
  else
//  if (VECTOR_TABLE == SIMULATION_TYPE)
    for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
      if (exp_array_index_1d (pvt->inputs, VT_INPUT, i).active_flag)
        qcad_cell_set_polarization (exp_array_index_1d (pvt->inputs, VT_INPUT, i).input,
          sim_data->trace[i].data[0] = exp_array_index_2d (pvt->vectors, gboolean, 0, i) ? 1 : -1) ;

  // Converge the steady state coherence vector for each cell so that the simulation starts without any transients //
  stable = FALSE;
  k = 0;
  while (!stable)
    {
    stable = TRUE;

    for (i = 0; i < number_of_cell_layers; i++)
      for (j = 0; j < number_of_cells_in_layer[i]; j++)
        {
        if (((QCAD_CELL_INPUT == sorted_cells[i][j]->cell_function)||
             (QCAD_CELL_FIXED == sorted_cells[i][j]->cell_function)))
          {
          j++;
          continue;
          }

        PEk = 0;
        // Calculate the sum of neighboring polarizations * the kink energy between them//
        for (q = 0; q < ((coherence_model *)sorted_cells[i][j]->cell_model)->number_of_neighbours; q++)
          PEk += (qcad_cell_calculate_polarization (((coherence_model *)sorted_cells[i][j]->cell_model)->neighbours[q])) * ((coherence_model *)sorted_cells[i][j]->cell_model)->Ek[q];

        old_lambda_x = ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_x;
        old_lambda_y = ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_y;
        old_lambda_z = ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_z;

        ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_x = lambda_ss_x(0, PEk, sim_data->clock_data[sorted_cells[i][j]->cell_options.clock].data[0], options);
        ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_y = lambda_ss_y(0, PEk, sim_data->clock_data[sorted_cells[i][j]->cell_options.clock].data[0], options);
        ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_z = lambda_ss_z(0, PEk, sim_data->clock_data[sorted_cells[i][j]->cell_options.clock].data[0], options);

        qcad_cell_set_polarization(sorted_cells[i][j], ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_z);

        // if the lambda values are different by more then the tolerance then they have not converged //
        stable =
          !(fabs (((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_x - old_lambda_x) > 1e-7 ||
            fabs (((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_y - old_lambda_y) > 1e-7 ||
            fabs (((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_z - old_lambda_z) > 1e-7) ;
        }
    k++;
    }

  command_history_message ("It took %d iterations to converge the initial steady state polarization\n", k);

  // perform the iterations over all samples //
  for (sample = 0; sample < number_samples; sample++)
    {
    if (0 == sample % 10000)
      {
      // Update the progress bar
      set_progress_bar_fraction ((float) sample / (float) number_samples) ;
      // redraw the design if the user wants it to appear animated //
#ifdef DESIGNER
      if(options->animate_simulation)
        {
        redraw_async(NULL);
        gdk_flush () ;
        }
#endif /* def DESIGNER */
      }
      // -- for each of the inputs -- //

    if (EXHAUSTIVE_VERIFICATION == SIMULATION_TYPE) 
      for (idxMasterBitOrder = 0, design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i), idxMasterBitOrder++)
        {
	  //determine clock-zone of input signal
	  int input_clk_num = exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, i).cell->cell_options.clock;
	  if ((zero_mode[input_clk_num] == 0) || (options->zero_mode_act == FALSE) )	  {
	    
	    // number_samples_per_period depends on No. of input signal
	    unsigned long int number_samples_per_in_period_input = number_samples_per_in_period * pow(2.0,(1+idxMasterBitOrder));
	    
	    /*
	    // sample = sample shifted by 1/4 or 3/4 *clock_phase mod samples_per_period_this_input;
	    // 1/4 | 3/4: input changes when related clock crosses -1 -> 1
	    input_clk_num = ((input_clk_num == 0) || (input_clk_num == 2))? input_clk_num + 3 : input_clk_num + 1;
	    unsigned sample_input = (sample + input_clk_num * (number_samples_per_period>>2) ) % number_samples_per_in_period_input;
	    */
	    
	    //input_clk_num = ((input_clk_num == 0) || (input_clk_num == 2))? input_clk_num + 3 : input_clk_num + 1;
	    unsigned sample_input = (sample + input_clk_num * (number_samples_per_period>>2) ) % number_samples_per_in_period_input;
	    
	    // if low
	    if (sample_input < (number_samples_per_in_period_input>>1))
	      dPolarization = -1;
	    else
	      dPolarization = 1;
      
	    // state is based on sin-function, nr. of input and freq. of clock signal
	    // double offset = ((double) input_clk * 0.25 * ((double) number_samples_per_period));
	    // //double offset = ((double) (input_clk + 3) * 0.5 * ((double) number_samples_per_period));
	    // dPolarization = (-sin (pow(2.0,(-1.0*idxMasterBitOrder)) * ((double) PI) * (((double) (sample + offset)) / ((double) number_samples_per_period)) )) > 0 ? 1 : -1;
	    
	    //double tmp = ((double) sample) / ((double) number_samples_per_period);
	    //printf("iclok: %d, sample: %lu, nspp: %lu, pow: %f, Pij/nspp: %g, PI*in: %f, dPol: %f\n",input_clk, sample, number_samples_per_period, pow(2.0,(-1.0*idxMasterBitOrder)), tmp ,(PI) * input_clk*0.5, dPolarization);	      
	  } else
	    dPolarization = 0 ;
	  
	  qcad_cell_set_polarization (exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, i).cell, dPolarization) ;
	  if (0 == sample % record_interval)
	    sim_data->trace[i].data[sample/record_interval] = dPolarization ;
        }
    else
//    if (VECTOR_TABLE == SIMULATION_TYPE)
      for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
        if (exp_array_index_1d (pvt->inputs, VT_INPUT, i).active_flag)
          {
          qcad_cell_set_polarization (exp_array_index_1d (pvt->inputs, VT_INPUT, i).input,
            dPolarization = exp_array_index_2d (pvt->vectors, gboolean, (sample*pvt->vectors->icUsed) / number_samples, i) ? 1 : -1) ;
          if (0 == sample % record_interval)
            sim_data->trace[i].data[sample/record_interval] = dPolarization ;
          }

    if (0 == sample % record_interval)
      {
      for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
        sim_data->trace[i].data[sample/record_interval] =
          qcad_cell_calculate_polarization (exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, i).cell) ;
      }

    // Randomize cells so as to minimize numerical error
    if (options->randomize_cells)
      // for each layer ...
      for (Nix = 0 ; Nix < number_of_cell_layers ; Nix++)
        // ...perform as many swaps as there are cells therein
        for (Nix1 = 0 ; Nix1 < number_of_cells_in_layer[Nix] ; Nix1++)
          {
          idxCell1 = rand () % number_of_cells_in_layer[Nix] ;
          idxCell2 = rand () % number_of_cells_in_layer[Nix] ;

          swap = sorted_cells[Nix][idxCell1] ;
          sorted_cells[Nix][idxCell1] = sorted_cells[Nix][idxCell2] ;
          sorted_cells[Nix][idxCell2] = swap ;
          }

    // -- run the iteration with the given clock value -- //
    run_coherence_iteration (sample, number_samples_per_period, clock_cycles, number_of_cell_layers, number_of_cells_in_layer, sorted_cells, total_number_of_inputs, number_samples, options, sim_data, SIMULATION_TYPE, pvt);
    
    
    // -- Set the cell polarizations to the lambda_z value -- //
    for (k = 0; k < number_of_cell_layers; k++)
      for (l = 0; l < number_of_cells_in_layer[k]; l++)
        {
        // don't simulate the input and fixed cells //
        if (((QCAD_CELL_INPUT == sorted_cells[k][l]->cell_function) ||
             (QCAD_CELL_FIXED == sorted_cells[k][l]->cell_function)))
          continue;
        if (fabs (((coherence_model *)sorted_cells[k][l]->cell_model)->lambda_z) > 1.0)
          {
          command_history_message ("I had to abort the simulation at iteration %d because the polarization = %e was diverging.\nPossible cause is the time step is too large.\nAlternatively, you can decrease the relaxation time to reduce oscillations.\n",sample, ((coherence_model *)sorted_cells[k][l]->cell_model)->lambda_z);
          command_history_message ("time step was set to %e\n", options->time_step);
          return sim_data;
          }
        qcad_cell_set_polarization (sorted_cells[k][l], ((coherence_model *)sorted_cells[k][l]->cell_model)->lambda_z);  // was lambda_z
        
        }

    // -- collect all the output data from the simulation -- //
    if (0 == sample % record_interval)
      for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_OUTPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i)) {
        sim_data->trace[total_number_of_inputs + i].data[sample/record_interval] =
        	qcad_cell_calculate_polarization (exp_array_index_1d (design->bus_layout->outputs, BUS_LAYOUT_CELL, i).cell) ;      
	}
  if (TRUE == STOP_SIMULATION) return sim_data;

  }
  
  
  // FST: print Energy
  //int sort[100][100]; // array of cells
  int **sort;
  int c, idx;
  double x_min, y_min;
  int dim_x, dim_y; // dimensions of x and y coordinates
  int sort_x, sort_y; // x,y coods of cell in sort array

  command_history_message("\n********* Energy Dissipation in eV ********** \n");
    
  
  // sort array
  // 1. Determine design sizes (over rcWorld)
  // 2. Determin dimensions of x and y in number of cells
  // 3. dynamic array sort[dim_x][dim_y]
  WorldRectangle rcWorld ;
  design_get_extents (design, &rcWorld, FALSE) ;
  dim_x = (rcWorld.cxWorld / CELL_DIST) + 2;
  dim_y = (rcWorld.cyWorld / CELL_DIST) + 2;
  
  sort = malloc (sizeof (int *) * dim_x);
  for (i = 0; i < dim_x; i++) {
    //*(sort + i) = malloc (sizeof (int) * dim_y);
   sort[i] =  malloc (sizeof (int) * dim_y);
  }
  
  double *E_error_total; // complete error of energy dissipation of all cells
  double *E_bath_total; //complete error dissipation
  double *E_clk_total;
  double *E_error; // sum of E_bath, E_clk, E_io = Error
  int max_idx = 1;
  E_error_total = NULL;
  E_error = NULL;
  E_bath_total = NULL;
  E_clk_total = NULL;
  idx = clock_cycles+3;
  E_error = g_malloc0 (sizeof (double) * idx);
  E_error_total = g_malloc0 (sizeof (double) * idx);
  E_bath_total = g_malloc0 (sizeof (double) * idx);
  E_clk_total = g_malloc0 (sizeof (double) * idx);
  
  if ((E_error_total == NULL)||(E_error == NULL) || (E_bath_total == NULL) || (E_clk_total == NULL) ) {
    printf ("memory allocation error for E_error\n");
    exit (1);
  }
  
  for (k = 0; k < number_of_cell_layers; k++) {
    
    //FST: Order cells into array
    //1st: Determine x_min, y_min
    if (number_of_cells_in_layer[k] > 0) {
      x_min = QCAD_DESIGN_OBJECT(sorted_cells[k][0])->x;
      y_min = QCAD_DESIGN_OBJECT(sorted_cells[k][0])->y;
    }
    for ( i = 1; i < number_of_cells_in_layer[k]; i++) {
      if (x_min > QCAD_DESIGN_OBJECT(sorted_cells[k][i])->x) 
	x_min = QCAD_DESIGN_OBJECT(sorted_cells[k][i])->x;
      if (y_min > QCAD_DESIGN_OBJECT(sorted_cells[k][i])->y) 
	y_min = QCAD_DESIGN_OBJECT(sorted_cells[k][i])->y;
    }
    
    //2nd: reset array
    for ( sort_x = 0; sort_x < dim_x; sort_x++)
      for ( sort_y = 0; sort_y < dim_y; sort_y++) 
	sort[sort_x][sort_y] = -1;    	
      
   
    // 3rd place cell into array
    // distance is fixed: dX=20, dy=20 = CELL_DIST
    for ( c = 0; c < number_of_cells_in_layer[k]; c++) {
      sort_x = 1+(QCAD_DESIGN_OBJECT(sorted_cells[k][c])->x - x_min) / CELL_DIST;
      sort_y = 1+(QCAD_DESIGN_OBJECT(sorted_cells[k][c])->y - y_min) / CELL_DIST;
      sort[sort_x][sort_y] = c;      
    }
        
    //determine total E_diss, E_clk, E_error
    for (sort_x = 1; sort_x < dim_x; sort_x++) {
      for (sort_y = 1; sort_y < dim_y; sort_y++) {	
	
	if (sort[sort_x][sort_y] != -1){ // cell exists?
	  l = sort[sort_x][sort_y];  
	  
	  // don't consider last value from clk 0
	  if (sorted_cells[k][l]->cell_options.clock == 0) {
	    ((coherence_model *)sorted_cells[k][l]->cell_model)->diss_idx--;
	  }
	  
	  //discard cells which have "ignore_energy" option activated
	  if ( sorted_cells[k][l]->cell_options.ignore_energy == FALSE) {
	    
	    // determine max_index
	    if (max_idx < ((coherence_model *)sorted_cells[k][l]->cell_model)->diss_idx){
	      max_idx = ((coherence_model *)sorted_cells[k][l]->cell_model)->diss_idx;
	    }
	    
	    for (idx = 2; idx < ((coherence_model *)sorted_cells[k][l]->cell_model)->diss_idx;idx++) {
	      E_error_total[idx] += ((coherence_model *)sorted_cells[k][l]->cell_model)->Int_diss_bath[idx];
	      E_bath_total[idx] += ((coherence_model *)sorted_cells[k][l]->cell_model)->Int_diss_bath[idx];
	    }
	      
	    for (idx = 2; idx < ((coherence_model *)sorted_cells[k][l]->cell_model)->diss_idx;idx++){
	      E_error_total[idx] += ((coherence_model *)sorted_cells[k][l]->cell_model)->Int_diss_clk[idx]; 
	      E_clk_total[idx] += ((coherence_model *)sorted_cells[k][l]->cell_model)->Int_diss_clk[idx];
	    }
	      
	    for (idx = 2; idx < ((coherence_model *)sorted_cells[k][l]->cell_model)->diss_idx;idx++) {
	      E_error_total[idx] += ((coherence_model *)sorted_cells[k][l]->cell_model)->Int_diss_io[idx]; 
	    }
	  } else
	    command_history_message("Ignore for total energy/error: Layer: %d, Cell [ %d ] [ %d ] (%.1e ; %.1e)\n",k,sort_x,sort_y,
	      	    QCAD_DESIGN_OBJECT(sorted_cells[k][l])->x,QCAD_DESIGN_OBJECT(sorted_cells[k][l])->y);
	}	  	  
      }
    }
    
    if (options->display_cell_diss == TRUE) {
      // 1st cell starts with 1 (not 0)
      for (sort_x = 1; sort_x < dim_x; sort_x++) {
	for (sort_y = 1; sort_y < dim_y; sort_y++) {	
	  if (sort[sort_x][sort_y] != -1) {
	    l = sort[sort_x][sort_y];
	    command_history_message("\nLayer: %d, Cell [ %d ] [ %d ] (%.1e ; %.1e), idx = %d\n",k,sort_x,sort_y,
	      	    QCAD_DESIGN_OBJECT(sorted_cells[k][l])->x,QCAD_DESIGN_OBJECT(sorted_cells[k][l])->y,((coherence_model *)sorted_cells[k][l]->cell_model)->diss_idx );
	  
	    
	    //print all Energy values
	    // [0] not because contains current value
	    // [1] not because contains 1st value which might be only partial
	    command_history_message("E_bath:\t");
	    for (idx = 2; idx < ((coherence_model *)sorted_cells[k][l]->cell_model)->diss_idx;idx++) {
	      command_history_message("%.4e\t", ((coherence_model *)sorted_cells[k][l]->cell_model)->Int_diss_bath[idx] / QCHARGE);
	      E_error[idx] = ((coherence_model *)sorted_cells[k][l]->cell_model)->Int_diss_bath[idx];  
	    }

	    command_history_message("\nE_clk:\t"); 
	    for (idx = 2; idx < ((coherence_model *)sorted_cells[k][l]->cell_model)->diss_idx;idx++) {
	      command_history_message("%.4e\t", ((coherence_model *)sorted_cells[k][l]->cell_model)->Int_diss_clk[idx] / QCHARGE);
	      E_error[idx] += ((coherence_model *)sorted_cells[k][l]->cell_model)->Int_diss_clk[idx];
	    }
		    
	    command_history_message("\nE_io:\t");
	    for (idx = 2; idx < ((coherence_model *)sorted_cells[k][l]->cell_model)->diss_idx;idx++) {
	      command_history_message("%.4e\t", ((coherence_model *)sorted_cells[k][l]->cell_model)->Int_diss_io[idx] / QCHARGE);
	      E_error[idx] += ((coherence_model *)sorted_cells[k][l]->cell_model)->Int_diss_io[idx]; 
	    }
	      
	    command_history_message("\nE_in:\t");
	    for (idx = 2; idx < ((coherence_model *)sorted_cells[k][l]->cell_model)->diss_idx;idx++) {
		command_history_message("%.4e\t", ((coherence_model *)sorted_cells[k][l]->cell_model)->Int_diss_in[idx] / QCHARGE);
	    }
	    
	    command_history_message("\nE_out:\t"); 
	    for (idx = 2; idx < ((coherence_model *)sorted_cells[k][l]->cell_model)->diss_idx;idx++) {
	      command_history_message("%.4e\t", ((coherence_model *)sorted_cells[k][l]->cell_model)->Int_diss_out[idx] / QCHARGE);
	    }
	    
	    
	    //write E_error and reset E_error
	    command_history_message("\nE_Error:\t");
	    for (idx = 2; idx < max_idx;idx++) {
	      command_history_message("%.4e\t", E_error[idx] / QCHARGE);
	      E_error[idx] = 0;
	    }
	      
	  
	    command_history_message("\n");
	    
	    // Ek
	    unsigned int num_neighbours = ((coherence_model *)sorted_cells[k][l]->cell_model)->number_of_neighbours;
	    int q;
	    for (q = 0 ; q < num_neighbours ; q++)
	      command_history_message("Ek[%d] = %e\n",q, ((coherence_model *)sorted_cells[k][l]->cell_model)->Ek[q] / QCHARGE);		
	    	
	  }
	}
      }
    }
     
  }
  // Print 
  //command_history_message("\nEnergy measured starting from %.0e s (sample %d in results graph)",
  //			   options->clock_period,  (int) (3000*(options->clock_period/options->duration)) );
  command_history_message("\nE_bath_total:\t");
  double E_bath_all = 0;
  for (idx = 2; idx < max_idx;idx++) {
    command_history_message("%.4e\t", E_bath_total[idx] / QCHARGE);
    E_bath_all += E_bath_total[idx] / QCHARGE;
  }
      
  command_history_message("\nE_clk_total:\t");
  double E_clk_all = 0;
  for (idx = 2; idx < max_idx;idx++) {
    command_history_message("%.4e\t", E_clk_total[idx] / QCHARGE);
    E_clk_all += E_clk_total[idx] / QCHARGE;
  }
      
  command_history_message("\nE_Error_total:\t");
  double E_error_all = 0;
  for (idx = 2; idx < max_idx;idx++) {
    command_history_message("%.4e\t ", E_error_total[idx] / QCHARGE);
    E_error_all += E_error_total[idx] / QCHARGE;
  }
       
  command_history_message("\n\nSum_bath: %.2e (Er: %.2e)\n", E_bath_all, E_error_all );
  command_history_message("Avg_bath: %.2e (Er: %.2e)", (E_bath_all)/(max_idx-2.0), E_error_all/(max_idx-2.0) );
  command_history_message("\nSum_clk: %.2e, Avg_clk: %.2e\n", E_clk_all,E_clk_all/(max_idx-2.0) );
  //command_history_message("\n\nSum: %.2e, Avg (Er: %.2e)\n", E_bath_all + E_clk_all, (E_bath_all+E_clk_all)/(max_idx-2.0) );
      
  command_history_message("\n********************************************************\n");
  printf("\nAll printed \n");
  //FST_end
  
  // Free the neigbours and Ek array introduced by this simulation//
  for (k = 0; k < number_of_cell_layers; k++)
    for (l = 0; l < number_of_cells_in_layer[k]; l++)
      {
      g_free (((coherence_model *)sorted_cells[k][l]->cell_model)->neighbours);
      g_free (((coherence_model *)sorted_cells[k][l]->cell_model)->neighbour_layer);
      g_free (((coherence_model *)sorted_cells[k][l]->cell_model)->Ek);
      }

  simulation_inproc_data_free (&number_of_cell_layers, &number_of_cells_in_layer, &sorted_cells) ;

  // Restore the input flag for the inactive inputs
  if (VECTOR_TABLE == SIMULATION_TYPE)
    for (i = 0 ; i < pvt->inputs->icUsed ; i++)
      exp_array_index_1d (pvt->inputs, BUS_LAYOUT_CELL, i).cell->cell_function = QCAD_CELL_INPUT ;

  // -- get and print the total simulation time -- //
  if ((end_time = time (NULL)) < 0)
    fprintf (stderr, "Could not get end time\n");

  command_history_message ("Total simulation time: %g s\n", (double)(end_time - start_time));
 
  set_progress_bar_visible (FALSE) ;
  
  fclose(diss_trace_file);
  
  //free sort
  for (i = 0; i < dim_x; i++)
    free (sort[i]);
  free (sort);

  
  return sim_data;
  }//run_coherence

// -- completes one simulation iteration performs the approximations until the entire design has stabalized -- //
static void run_coherence_iteration (unsigned long int sample_number, unsigned long int number_samples_per_period, int clock_cycles, int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, int total_number_of_inputs, unsigned long int number_samples, const coherence_energy_OP *options, simulation_data *sim_data, int SIMULATION_TYPE, VectorTable *pvt)
{
  unsigned int i,j,q;
  double lambda_x_new;
  double lambda_y_new;
  double lambda_z_new;
  double lambda_x;
  double lambda_y;
  double lambda_z;
  double clock_value;
  double PEk;
  unsigned int num_neighbours;
  
  double t = options->time_step * (double)sample_number;
  
  //FST
  double dt = options->time_step; // dt for Integration of Energy
  
  //save old GAMMA/PEks value for energy estimation
  double old_clock_value;
  double old_next_clock_zone; // clock for dissipation integral
  double old_PEk_io;
  double old_PEk_in;
  double old_PEk_out;
  double PEk_in;
  double PEk_out;
  int cell_x; 
  int cell_y;
  int cell_x_nb;
  int idx;
  QCADCell *cell;
  
  double Pol_in;
  double Pol_out;
  
  // calculate once all 4 clocks for all cells
  double clock_values[4]; 
  for (i = 0; i<4; i++)
    clock_values[i] = calculate_clock_value(i, sample_number, number_samples_per_period, options);
  
  
  // loop through all the cells in the design //
  for (i = 0 ; i < number_of_cell_layers ; i++)
    for (j = 0 ; j < number_of_cells_in_layer[i] ; j++)
      {
      // don't simulate the input and fixed cells //
      if (((QCAD_CELL_INPUT == sorted_cells[i][j]->cell_function) ||
           (QCAD_CELL_FIXED == sorted_cells[i][j]->cell_function)))
        continue;
      
      clock_value = clock_values[sorted_cells[i][j]->cell_options.clock];
          
      PEk = 0;
      // Calculate the sum of neighboring polarizations //
      num_neighbours = ((coherence_model *)sorted_cells[i][j]->cell_model)->number_of_neighbours;
      for (q = 0 ; q < num_neighbours ; q++)
        PEk += (qcad_cell_calculate_polarization (((coherence_model *)sorted_cells[i][j]->cell_model)->neighbours[q]))*((coherence_model *)sorted_cells[i][j]->cell_model)->Ek[q];

      // FST
      //Old Gamma, old PEk
      old_clock_value = ((coherence_model *)sorted_cells[i][j]->cell_model)->Old_Gamma;
      ((coherence_model *)sorted_cells[i][j]->cell_model)->Old_Gamma = clock_value;
      old_PEk_io = ((coherence_model *)sorted_cells[i][j]->cell_model)->Old_PEk_io;
      ((coherence_model *)sorted_cells[i][j]->cell_model)->Old_PEk_io = PEk;
      
      // FST: determine PEk of left cells = IN and OUT
      cell_x = QCAD_DESIGN_OBJECT(sorted_cells[i][j])->x;
      cell_y = QCAD_DESIGN_OBJECT(sorted_cells[i][j])->y;
      PEk_in = 0;
      PEk_out = 0;
      Pol_in = 0;
      Pol_out = 0;
      for (q = 0 ; q < num_neighbours ; q++) {
	cell = ((coherence_model *)sorted_cells[i][j]->cell_model)->neighbours[q];
	cell_x_nb = QCAD_DESIGN_OBJECT(cell)->x;
		
	if  ( cell_x_nb < cell_x )  { // cell_in
	  PEk_in += (qcad_cell_calculate_polarization (((coherence_model *)sorted_cells[i][j]->cell_model)->neighbours[q]))*((coherence_model *)sorted_cells[i][j]->cell_model)->Ek[q];
	  Pol_in += (qcad_cell_calculate_polarization (((coherence_model *)sorted_cells[i][j]->cell_model)->neighbours[q]));
	}
	if  ( cell_x_nb > cell_x ) { //cell_out
	  PEk_out += (qcad_cell_calculate_polarization (((coherence_model *)sorted_cells[i][j]->cell_model)->neighbours[q]))*((coherence_model *)sorted_cells[i][j]->cell_model)->Ek[q];
	  Pol_out += (qcad_cell_calculate_polarization (((coherence_model *)sorted_cells[i][j]->cell_model)->neighbours[q]));
	}
      }
	
      old_PEk_in  = ((coherence_model *)sorted_cells[i][j]->cell_model)->Old_PEk_in;
      old_PEk_out = ((coherence_model *)sorted_cells[i][j]->cell_model)->Old_PEk_out;
      ((coherence_model *)sorted_cells[i][j]->cell_model)->Old_PEk_in  = PEk_in;
      ((coherence_model *)sorted_cells[i][j]->cell_model)->Old_PEk_out = PEk_out;
      //FST_end
      
      lambda_x = ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_x;
      lambda_y = ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_y;
      lambda_z = ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_z;

      lambda_x_new = lambda_x_next (t, PEk, clock_value, lambda_x, lambda_y, lambda_z, options);
      lambda_y_new = lambda_y_next (t, PEk, clock_value, lambda_x, lambda_y, lambda_z, options);
      lambda_z_new = lambda_z_next (t, PEk, clock_value, lambda_x, lambda_y, lambda_z, options);

      ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_x = lambda_x_new;
      ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_y = lambda_y_new;
      ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_z = lambda_z_new;
      

      // FST
      ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_bath = calculate_diss_bath(lambda_x_new, lambda_z_new, clock_value, PEk, options);      
      ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_clk  = calculate_diss_clk(lambda_x_new, clock_value, old_clock_value, options->time_step);
      ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_io   = calculate_diss_io(lambda_z, PEk, old_PEk_io, options->time_step);
      ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_in   = calculate_diss_io(lambda_z, PEk_in, old_PEk_in, options->time_step);
      ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_out  = calculate_diss_io(lambda_z, PEk_out, old_PEk_out, options->time_step);
	
	
      // estimate clock of neigbour clock zone
      double next_clk_zone = clock_values[(sorted_cells[i][j]->cell_options.clock + 1) % 4];
      old_next_clock_zone = ((coherence_model *)sorted_cells[i][j]->cell_model)->old_next_clock_zone;
      ((coherence_model *)sorted_cells[i][j]->cell_model)->old_next_clock_zone = next_clk_zone;
      
      
      // reset diss_integrals when new clock phase starts
      // search for begin of new phase
      // use next clock zone as reference in order to avoid alingnment problems
      // use middle of clock zones
      //if (((old_next_clock_zone == options->clock_low) && (next_clk_zone > options->clock_low))
      //printf("CZ %d = %e (m= %e) \n",sorted_cells[i][j]->cell_options.clock,next_clk_zone,optimization_options.clock_middle );
      if (((old_next_clock_zone < optimization_options.clock_middle) && (next_clk_zone > optimization_options.clock_middle )) 	        
	){
	  //|| ((old_next_clock_zone == options->clock_high) && (next_clk_zone < options->clock_high)) ) {
	  	   
	  //store result at index diss_idx
	  idx = ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_idx;
	  
	  //printf("crossed\n"); 
	  
	  // idx is -1 if clock-zone != 1
	  // Idea: reference input is clock 0 
	  // only what happens after is recorded => cells in other clkzones are only recorded after
	  if (idx != -1) {
	    ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_bath[idx] = ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_bath[0] * dt /  options->relaxation;
	    ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_clk[idx] = ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_clk[0] * dt;
	    ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_io[idx] = ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_io[0] * dt;
	    ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_in[idx] =  ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_in[0] * dt; 
	    ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_out[idx] =  ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_out[0] * dt;
	    idx++;
	  } else {
	    idx = 1;
	  }	  
	  //printf("ins: %d, assert %d\n",idx,clock_cycles+1 );
	  g_assert (idx < (clock_cycles+3));	  
	  ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_idx = idx;
	  ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_bath[0] = 0;
	  ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_clk[0] = 0;
	  ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_io[0] = 0;
	  ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_in[0] = 0;
	  ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_out[0] = 0;
	  
	  
      }
      
      ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_bath[0] += ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_bath;
      ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_clk[0] += ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_clk;
      ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_io[0] += ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_io;
      ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_in[0] += ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_in;
      ((coherence_model *)sorted_cells[i][j]->cell_model)->Int_diss_out[0] += ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_out;
      //if (j == 3)  printf("%e %e\n",((coherence_model *)sorted_cells[i][j]->cell_model)->diss_bath, ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_clk );
	
	
      // ******************************
      // Plotting for evauation
      if ((cell_x==options->diss_trace_cood_x) && (cell_y==options->diss_trace_cood_y) ) {
	  double Gamma = clock_value;
	  double l_ss_x = -2.0 * Gamma / hypot(2*Gamma, PEk) * tanh (temp_ratio (PEk, Gamma, options->T));
	  double l_ss_z = PEk / hypot(2*Gamma, PEk) * tanh (temp_ratio (PEk, Gamma, options->T));
	  double sl_z = options->time_step * slope_z (t, PEk, Gamma, lambda_x, lambda_y, lambda_z, options);
	  double Pol = qcad_cell_calculate_polarization ((coherence_model *)sorted_cells[i][j]);
	  	  
	  //printf("%e\t%e\t%e\t%e\t%e\t%e\t%e\t%e\t%e\t%e\n",t,lambda_x_new,l_ss_x,l_ss_x-lambda_x_new,lambda_z_new,l_ss_z,l_ss_z-lambda_z_new,lambda_y_new,((coherence_model *)sorted_cells[i][j]->cell_model)->diss_bath,PEk);
	  /* for gnu-plot
	  #1 t
	  #2 lambda_z
	  #3 lambda_z_new
	  #4 lambda_z-lambda_z_new
	  #5 l_ss_z
	  #6 sl_z
	  #7 l_ss_z-lambda_z_new
	  #8 l_ss_x-lambda_x_new
	  #9 ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_bath
	  #10 PEk_in
	  #11 ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_in 
	  #12 ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_out 
	  #13 ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_clk)
	  #14 clock_value
	  #15 Pol	  	   
	  #16 Pol_in
	  #17 Pol_out
	  #18 PEk
	  */
	  fprintf(diss_trace_file,"%e\t%e\t%e\t%e\t%e\t%e\t%e\t%e\t%e\t%e\t%e\t%e\t%e\t%e\t%e\t%e\t%e\t%e\n",t,lambda_z,lambda_z_new, 
		  lambda_z-lambda_z_new, l_ss_z, sl_z, l_ss_z-lambda_z_new, l_ss_x-lambda_x_new, ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_bath, 
		  PEk_in, ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_in, ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_out, 
		  ((coherence_model *)sorted_cells[i][j]->cell_model)->diss_clk, clock_value, Pol, Pol_in, Pol_out, PEk); 
	}
	// ******************************
	//FST_end
	
      }
  }//run_iteration

//-------------------------------------------------------------------//
// -- refreshes the array of Ek values for each cell in the design this is done to speed up the simulation
// since we can assume no design changes durring the simulation we can precompute all the Ek values then
// use them as necessary throughout the simulation -- //
static void coherence_refresh_all_Ek (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, coherence_energy_OP *options)
  {
  int icNeighbours = 0 ;
  coherence_model *cell_model = NULL ;
  int i,j,k;

  // calculate the Ek for each cell //
  for(i = 0 ; i < number_of_cell_layers ; i++)
    for(j = 0 ; j < number_of_cells_in_layer[i] ; j++)
      {
      // free up memory from previous simulations //
      g_free ((cell_model = (coherence_model *)sorted_cells[i][j]->cell_model)->neighbours);
      g_free (cell_model->Ek);
      g_free (cell_model->neighbour_layer);
      cell_model->neighbours = NULL;
      cell_model->neighbour_layer = NULL;
      cell_model->Ek = NULL;

      // select all neighbours within the provided radius //
      cell_model->number_of_neighbours = icNeighbours =
        select_cells_in_radius(sorted_cells, sorted_cells[i][j], ((coherence_energy_OP *)options)->radius_of_effect, i, number_of_cell_layers, number_of_cells_in_layer,
             ((coherence_energy_OP *)options)->layer_separation, &(cell_model->neighbours), (int **)&(cell_model->neighbour_layer));

      //printf("number of neighbors = %d\n", icNeighbours);

      if (icNeighbours > 0)
        {
        cell_model->Ek = g_malloc0 (sizeof (double) * icNeighbours);

        // ensure no memory allocation error has ocurred //
        if (((coherence_model *)sorted_cells[i][j]->cell_model)->neighbours == NULL ||
            ((coherence_model *)sorted_cells[i][j]->cell_model)->Ek == NULL)
          //printf ("memory allocation error in refresh_all_Ek()\n");
          exit (1);

        for (k = 0; k < icNeighbours; k++)
          //if(cell_model->neighbours[k]==NULL)printf("Null neighbour prior to passing into determine Ek for k = %d\n", k);
          // set the Ek of this cell and its neighbour //
          cell_model->Ek[k] = coherence_determine_Ek (sorted_cells[i][j], cell_model->neighbours[k], ABS(i-cell_model->neighbour_layer[k]), options);
          //printf("Ek = %e\n", cell_model->Ek[k]/1.602e-19);
        }
      }
  }//refresh_all_Ek


// FST: initialize all Energy dissipation arrays
static void coherence_initialize_all_diss (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, coherence_energy_OP *options, int clock_cycles)
  {
  coherence_model *cell_model = NULL ;
  int i,j;
  
  for(i = 0 ; i < number_of_cell_layers ; i++)
    for(j = 0 ; j < number_of_cells_in_layer[i] ; j++) {
      // free up memory from previous simulations //
      cell_model = (coherence_model *)sorted_cells[i][j]->cell_model;
      g_free (cell_model->Int_diss_bath);
      g_free (cell_model->Int_diss_clk);
      g_free (cell_model->Int_diss_io);
      g_free (cell_model->Int_diss_in);
      g_free (cell_model->Int_diss_out);
      
      cell_model->Int_diss_bath = NULL;
      cell_model->Int_diss_clk = NULL;
      cell_model->Int_diss_io = NULL;
      cell_model->Int_diss_in = NULL;
      cell_model->Int_diss_out = NULL;
      
      // idx is -1 if clock-zone != 1
      // Idea: input is clock 0 
      // only what happens after is recorded => cells in other clkzones are only recorded after
      if  (sorted_cells[i][j]->cell_options.clock == 0) {
	cell_model->diss_idx = 1;
      } else {
	cell_model->diss_idx = -1;
      }
           
      
      // allocate memory
      // size depends on clock cycles per similation
      cell_model->Int_diss_bath = g_malloc0 (sizeof (double) * (clock_cycles+3));
      cell_model->Int_diss_clk = g_malloc0 (sizeof (double) * (clock_cycles+3));
      cell_model->Int_diss_io = g_malloc0 (sizeof (double) * (clock_cycles+3));
      cell_model->Int_diss_in = g_malloc0 (sizeof (double) * (clock_cycles+3));
      cell_model->Int_diss_out = g_malloc0 (sizeof (double) *(clock_cycles+3));
      
      // ensure no memory allocation error has ocurred //
      if ((cell_model->Int_diss_bath == NULL) ||
	  (cell_model->Int_diss_clk == NULL) ||
	  (cell_model->Int_diss_io == NULL) ||
	  (cell_model->Int_diss_in == NULL) ||
	  (cell_model->Int_diss_out == NULL) ) {
	  printf ("memory allocation error in initialize_all_diss()\n");
          exit (1);
      }
    }
  }//initialize_all_diss

 
//-------------------------------------------------------------------//
// Determines the Kink energy of one cell with respect to another this is defined as the energy of those
// cells having opposite polarization minus the energy of those two cells having the same polarization -- //
static double coherence_determine_Ek (QCADCell * cell1, QCADCell * cell2, int layer_separation, coherence_energy_OP *options)
  {
  int k;
  int j;

  double distance = 0;
  double Constant = 1 / (4 * PI * EPSILON * options->epsilonR);

  double charge1[4] = { -HALF_QCHARGE,  HALF_QCHARGE, -HALF_QCHARGE,  HALF_QCHARGE };
  double charge2[4] = {  HALF_QCHARGE, -HALF_QCHARGE,  HALF_QCHARGE, -HALF_QCHARGE };
  
  double EnergyDiff = 0;
  double EnergySame = 0;
  
  g_assert (cell1 != NULL);
  g_assert (cell2 != NULL);
  g_assert (cell1 != cell2);

  for (k = 0; k < cell1->number_of_dots; k++)
    for (j = 0; j < cell2->number_of_dots; j++)
      {
      // determine the distance between the dots //
      // printf("layer seperation = %d\n", layer_seperation);
      distance = determine_distance (cell1, cell2, k, j, (double)layer_separation * ((coherence_energy_OP *)options)->layer_separation);
      g_assert (distance != 0);

      EnergyDiff += Constant * (charge1[k] * charge2[j]) / (distance*1e-9);
      EnergySame += Constant * (charge1[k] * charge1[j]) / (distance*1e-9);
    
      }//for other dots
 
      //printf("Ek = %e \n", (EnergyDiff - EnergySame));

  return EnergyDiff - EnergySame;
  }// coherence_determine_Ek


//-------------------------------------------------------------------//
// Calculates the clock data at a particular sample 
// modified by FST
static inline double calculate_clock_value (unsigned int clock_num, unsigned long int sample, unsigned long int number_samples_per_period, const coherence_energy_OP *options)
  {
  double clock = 0;
  
  // increase by one full period, to avoid negative values in 1st clock cycle
  sample = sample + number_samples_per_period;
  // sample = shifted by 1/4*clock_phase mod samples_per_period
  sample = (sample - clock_num * (number_samples_per_period>>2) ) % number_samples_per_period;
 
  
  //determine if zero-mode of inputs in following clock-zone has to activated
  // rule:
  // clk0 -> defines zero_mode for inputs in clk1 
  // ... 
  // clk3 -> defines zero_mode for inputs in clk0
  
  //+1: if number_samples_per_period is not even
  if ((sample == 0) || (sample == 1)) // rising slope => zero mode on
    zero_mode[(clock_num)%4] = 1;
  
  if ((sample == (number_samples_per_period>>1)) || (sample == 1+(number_samples_per_period>>1))) // falling slope => zero mode off
    zero_mode[(clock_num)%4] = 0;
    
   
  if (options->clock_type == COS) {    
      clock = optimization_options.clock_prefactor *
      cos ( (double) TWO_PI * sample / number_samples_per_period - PI * (double)clock_num * 0.5) + optimization_options.clock_shift + options->clock_shift;
  }

  if (options->clock_type == RAMP) {
    
    // if rising slope
    if (sample < (number_samples_per_period>>1))
      clock = options->clock_low + optimization_options.slope_factor * ((double) sample) + options->clock_shift;
    else { // fall slope
      sample = sample - (number_samples_per_period>>1);
      clock = options->clock_high - optimization_options.slope_factor * ((double) sample) + options->clock_shift;
    }
  }
   
  if (options->clock_type == GAUSS) {
    
    // if rising slope
    if (sample < (number_samples_per_period>>1))
      if ((sample == 0) || (sample == 1) ) // ==1: if number_samples_per_period is not even
	clock = options->clock_low + options->clock_shift; // assures that clock reaches at least once low      
      else
	clock = options->clock_high - (options->clock_high - options->clock_low) * exp (-0.5* (pow (((double) sample) / optimization_options.sigma, 2))) + options->clock_shift;	
    else {
      sample = sample - (number_samples_per_period>>1);
      if ((sample == 0) || (sample == 1))
	clock = options->clock_high + options->clock_shift; // assures that clock reaches  at least once  high
      else
        clock = options->clock_low + (options->clock_high - options->clock_low) * exp (-0.5* (pow (((double) sample) / optimization_options.sigma, 2))) + options->clock_shift;
    }    
  } 
    
  // Saturate the clock at the clock high and low values  
  clock = CLAMP (clock, options->clock_low, options->clock_high);
  
  //if ((old_clock[clock_num] < options->clock_high) && (clock == options->clock_high)) // end rising edge of clock => zero mode is on 
  //if ((old_clock[clock_num] < optimization_options.clock_middle) && (clock > optimization_options.clock_middle))// middle rising edge of clock => zero mode is on 
   // zero_mode[(clock_num)%4] = 0;
  
  //if ((old_clock[clock_num] > options->clock_low) && (clock == options->clock_low)) // end falling edge of clock3 => zero mode is off
 // if ((old_clock[clock_num] > optimization_options.clock_middle) && (clock < optimization_options.clock_middle)) // middle falling edge of clock3 => zero mode is off
    //zero_mode[(clock_num)%4] = 1;      
  
  old_clock[clock_num] = clock;
  
  return clock;
  }// calculate_clock_value

//-------------------------------------------------------------------//

// Next value of lambda x with choice of algorithm
static inline double lambda_x_next (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_energy_OP *options)
  {
  double k1 = options->time_step * slope_x (t, PEk, Gamma, lambda_x, lambda_y, lambda_z, options);
  double k2, k3, k4;
  
  if (RUNGE_KUTTA == options->algorithm)
    {
    k2 = options->time_step * slope_x (t, PEk, Gamma, lambda_x + k1/2, lambda_y, lambda_z, options);
    k3 = options->time_step * slope_x (t, PEk, Gamma, lambda_x + k2/2, lambda_y, lambda_z, options);
    k4 = options->time_step * slope_x (t, PEk, Gamma, lambda_x + k3,   lambda_y, lambda_z, options);
    return lambda_x + k1/6 + k2/3 + k3/3 + k4/6;
    }
  else
  if (EULER_METHOD == options->algorithm)
    return lambda_x + k1;
  else
    command_history_message ("coherence vector undefined algorithm\n");

  return 0;
  }

// Next value of lambda y with choice of algorithm
static inline double lambda_y_next (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_energy_OP *options)
  {
  double k1 = options->time_step * slope_y (t, PEk, Gamma, lambda_x, lambda_y, lambda_z, options);
  double k2, k3, k4;

  if (RUNGE_KUTTA == options->algorithm)
    {
    k2 = options->time_step * slope_y (t, PEk, Gamma, lambda_x, lambda_y + k1/2, lambda_z, options);
    k3 = options->time_step * slope_y (t, PEk, Gamma, lambda_x, lambda_y + k2/2, lambda_z, options);
    k4 = options->time_step * slope_y (t, PEk, Gamma, lambda_x, lambda_y + k3,   lambda_z, options);
    return lambda_y + k1/6 + k2/3 + k3/3 + k4/6;
    }
  else
  if (EULER_METHOD == options->algorithm)
    return lambda_y + k1;
  else
    command_history_message("coherence vector undefined algorithm\n");

  return 0;
  }// FST how many clock cycles shall be stored in Energy arry
#define CLKCYCLES 

// Next value of lambda z with choice of algorithm
static inline double lambda_z_next (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_energy_OP *options)
  {
  double k1 = options->time_step * slope_z (t, PEk, Gamma, lambda_x, lambda_y, lambda_z, options);
  double k2, k3, k4;
 
  if (RUNGE_KUTTA == options->algorithm)
    {
    k2 = options->time_step * slope_z(t, PEk, Gamma, lambda_x, lambda_y, lambda_z + k1/2, options);
    k3 = options->time_step * slope_z(t, PEk, Gamma, lambda_x, lambda_y, lambda_z + k2/2, options);
    k4 = options->time_step * slope_z(t, PEk, Gamma, lambda_x, lambda_y, lambda_z + k3,   options);
    return lambda_z + k1/6 + k2/3 + k3/3 + k4/6;
    }
  else
  if (EULER_METHOD == options->algorithm) {
    //double tmp = lambda_z + k1;
    //if ((tmp == lambda_z)&&(k1 != 0.0)) {
    //printf("Error: k1 != slope*tau, k1 = %e, lz = %e, lz+k1 = %e\n", k1,lambda_z, lambda_z + k1 );
    //  while (tmp == lambda_z) {
    //	k1 = 2*k1;
    //  tmp = lambda_z + k1;
    //  }     
    return lambda_z + k1;
  }else
    command_history_message("coherence vector undefined algorithm\n");

  return 0;
  }

static inline double slope_x (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_energy_OP *options) {
  // FST: improved calculation: remove unnecessarry over_hbar 
  // double mag = magnitude_energy_vector (PEk, Gamma);
  // return (-(2.0 * Gamma * over_hbar / mag * tanh (optimization_options.hbar_over_kBT * mag) + lambda_x) / options->relaxation + (PEk * lambda_y * over_hbar));
  //Gamma = (options->clock_low + options->clock_high) - Gamma;
  double mag_noh = hypot(2.0*Gamma, PEk);
  // return (-(2.0 * Gamma / mag_noh * tanh (mag_noh/(kB*options->T)) + lambda_x) / options->relaxation + (PEk * lambda_y * over_hbar)); // mod. old
  return ( (PEk * lambda_y * over_hbar) - (lambda_x + (2.0 * Gamma / mag_noh * tanh (temp_ratio (PEk, Gamma, options->T)) ) ) / options->relaxation  );

}

static inline double slope_y (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_energy_OP *options)
  { 
     //old: {return -(options->relaxation * (PEk * lambda_x + 2.0 * Gamma * lambda_z) + hbar * lambda_y) / (options->relaxation * hbar);}
    return over_hbar*(PEk*lambda_x + 2.0*Gamma*lambda_z) - lambda_y / options->relaxation;
  }


static inline double slope_z (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_energy_OP *options) {
  // FST: improved calculation: remove unnecessarry over_hbar
  // #define magnitude_energy_vector(P,G) (hypot(2*(G), (P)) * over_hbar)
  // double mag = magnitude_energy_vector (PEk, Gamma);
  // return (PEk * tanh (optimization_options.hbar_over_kBT * mag) + mag * (2.0 * Gamma * options->relaxation * lambda_y - hbar * lambda_z)) / (options->relaxation * hbar * mag);
  
    double mag_noh = hypot(2.0*Gamma, PEk);
  // mod: return (PEk * tanh (mag_noh/(2*kB*options->T)) + mag_noh * (over_hbar * 2.0 * Gamma * options->relaxation * lambda_y - lambda_z)) / (options->relaxation * mag_noh);
    return -2.0*Gamma*over_hbar*lambda_y - (lambda_z - PEk/mag_noh * tanh (temp_ratio (PEk, Gamma, options->T)) ) / options->relaxation;
  }

//-------------------------------------------------------------------------------------------------------------------------//

// Steady-State Coherence Vector X component
// FST: missing inversion!
static inline double lambda_ss_x(double t, double PEk, double Gamma, const coherence_energy_OP *options) {
  // FST: improved calculation: remove unnecessarry over_hbar
  // old return -2.0 * Gamma * over_hbar / magnitude_energy_vector(PEk, Gamma) * tanh (temp_ratio (PEk, Gamma, options->T));
  // old_mod: return -2.0 * Gamma / mag_noh * tanh (temp_ratio (PEk, Gamma, options->T));
  double mag_noh = hypot(2.0*Gamma, PEk);
  return ((2.0 * Gamma) / mag_noh) * tanh (temp_ratio (PEk, Gamma, options->T));
}

// Steady-State Coherence Vector y component
static inline double lambda_ss_y (double t, double PEk, double Gamma, const coherence_energy_OP *options)
  {return 0.0;}

// Steady-State Coherence Vector Z component
// FST: missing inversion!
static inline double lambda_ss_z(double t, double PEk, double Gamma, const coherence_energy_OP *options) {
  // FST: improved calculation: remove unnecessarry over_hbarcell_model->cell_options.clock
  // old: return PEk * over_hbar / magnitude_energy_vector (PEk, Gamma) * tanh (temp_ratio (PEk, Gamma, options->T));
  double mag_noh = hypot(2.0*Gamma, PEk);
  //old_mod: return PEk / mag_noh * tanh (temp_ratio (PEk, Gamma, options->T));
  return ((-1.0 * PEk) / mag_noh) * tanh (temp_ratio (PEk, Gamma, options->T));
}
  
// FST: integrand of Energy dissipation to bath 
// HV_1 = HV_x = -2.0 * Gamma * over_hbar
// HV_2 = HV_y = 0.0
// HV_3 = HV_z = PEk * over_hbar 
// hbar can be removed
// tau is only removed later => faster processing
// verify sign: now: -G_x, - G_z
static inline double calculate_diss_bath(double lambda_x, double lambda_z, double Gamma, double PEk, const coherence_energy_OP *options) 
 { 
   double mag_noh = hypot(2.0*Gamma, PEk);
   double l_ss_x = -2.0 * Gamma / mag_noh * tanh (temp_ratio (PEk, Gamma, options->T));
   double l_ss_z = PEk / mag_noh * tanh (temp_ratio (PEk, Gamma, options->T));
   return ( 0.5 * (-2.0*Gamma*(l_ss_x-lambda_x) + PEk*(l_ss_z-lambda_z)) );
   //return ( 0.5 * ( 2.0 * Gamma * lambda_x - PEk * lambda_z + tanh (temp_ratio (PEk, Gamma, options->T)) * hypot(2*Gamma, PEk) ));
 }
     
 
// FST: integrand of Energy dissipation to clock
// HV_1 = -2.0 * Gamma * over_hbar
// dHV_1/dt = -2.0 * over_hbar * dGAMMA/dt
// hbar can be removed
static inline double calculate_diss_clk(double lambda_x, double Gamma, double Old_Gamma, double time_step)
  { 
    //printf("%e\n",(Gamma-Old_Gamma)/time_step);     
    return (-1.0 * lambda_x * (Gamma-Old_Gamma)/time_step); }
  
// FST: integrand of Energy dissipation to clock
// HV_3 = PEk * over_hbar
// dHV_3/dt = over_hbar * dPEk/dt
// hbar can be removed???
static inline double calculate_diss_io(double lambda_z, double PEk, double Old_PEk, double time_step) {
  return ( 0.5 * lambda_z * (PEk-Old_PEk)/(time_step) ); 
  }
  

static int compareCoherenceQCells (const void *p1, const void *p2)
  {
  return
    ((coherence_model *)((*((QCADCell **)(p1)))->cell_model))->number_of_neighbours >
    ((coherence_model *)((*((QCADCell **)(p2)))->cell_model))->number_of_neighbours ?  1 :
    ((coherence_model *)((*((QCADCell **)(p1)))->cell_model))->number_of_neighbours <
    ((coherence_model *)((*((QCADCell **)(p2)))->cell_model))->number_of_neighbours ? -1 : 0 ;
  }//compareSortStructs

void coherence_energy_options_dump (coherence_energy_OP *coherence_energy_options, FILE *pfile)
  {
	fprintf (stderr, "coherence_energy_options_dump:\n") ;
	fprintf (stderr, "coherence_energy_options->T                         = %e [K]\n",  coherence_energy_options->T) ;
	fprintf (stderr, "coherence_energy_options->relaxation                = %e [s]\n",  coherence_energy_options->relaxation) ;
	fprintf (stderr, "coherence_energy_options->time_step                 = %e [s]\n",  coherence_energy_options->time_step) ;
	fprintf (stderr, "coherence_energy_options->duration                  = %e [s]\n",  coherence_energy_options->duration) ;
	fprintf (stderr, "coherence_energy_options->clock_high                = %e [J]\n",  coherence_energy_options->clock_high) ;
	fprintf (stderr, "coherence_energy_options->clock_low                 = %e [J]\n",  coherence_energy_options->clock_low) ;
	fprintf (stderr, "coherence_energy_options->clock_period              = %e [s]\n",  coherence_energy_options->clock_period) ;
	fprintf (stderr, "coherence_energy_options->input_period              = %e [s]\n",  coherence_energy_options->input_period) ;
	fprintf (stderr, "coherence_energy_options->t_slope_ramp              = %e [s]\n",  coherence_energy_options->t_slope_ramp) ;
	fprintf (stderr, "coherence_energy_options->radius_of_effect          = %e [nm]\n", coherence_energy_options->radius_of_effect) ;
	fprintf (stderr, "coherence_energy_options->epsilonR                  = %e\n",      coherence_energy_options->epsilonR) ;
	fprintf (stderr, "coherence_energy_options->layer_separation          = %e [nm]\n", coherence_energy_options->layer_separation) ;
	fprintf (stderr, "coherence_energy_options->algorithm                 = %d\n",      coherence_energy_options->algorithm) ;
	fprintf (stderr, "coherence_energy_options->clock_type                = %d\n",      coherence_energy_options->clock_type) ;
	fprintf (stderr, "coherence_energy_options->randomize_cells           = %s\n",      coherence_energy_options->randomize_cells ? "TRUE" : "FALSE") ;
	fprintf (stderr, "coherence_energy_options->animate_simulation        = %s\n",      coherence_energy_options->animate_simulation ? "TRUE" : "FALSE") ;
	fprintf (stderr, "coherence_energy_options->zero_mode_act             = %s\n",      coherence_energy_options->zero_mode_act ? "TRUE" : "FALSE") ;
	fprintf (stderr, "coherence_energy_options->display_cell_diss         = %s\n",      coherence_energy_options->display_cell_diss ? "TRUE" : "FALSE") ;	
  }
