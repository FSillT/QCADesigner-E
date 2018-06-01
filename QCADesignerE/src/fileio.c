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
// I/O. IOW, loading and saving files is implemented    //
// here.                                                //
//                                                      //
//////////////////////////////////////////////////////////

// -- includes -- //
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "fileio.h"
#include "design.h"
#include "support.h"
#include "fileio_helpers.h"
#include "global_consts.h"
#include "custom_widgets.h"
#include "objects/QCADDOContainer.h"

#define FLOATS_PER_LINE 5

#define DBG_FIO(s)

#define DBG_OPEN(s)

// ---------------------------------------------------------------------------------------- //

static gboolean legacy_open_project_file (FILE *fd, DESIGN **pdesign) ;
static void read_options(FILE *project_file, double *pgrid_spacing);
static char *get_identifier(char *buffer);
static QCADDesignObject *legacy_read_cell_from_stream (FILE *stream) ;
static void serialize_trace (FILE *fp, struct TRACEDATA *trace, int icSamples) ;
static void unserialize_trace (FILE *pfile, struct TRACEDATA *trace, int icSamples) ;
static void unserialize_trace_data (FILE *pfile, struct TRACEDATA *trace, int icSamples) ;
static coherence_OP *open_coherence_options_file_fp (FILE *fp) ;
static coherence_energy_OP *open_coherence_energy_options_file_fp (FILE *fp) ;
static bistable_OP *open_bistable_options_file_fp (FILE *fp) ;
static void build_io_tables (simulation_data *sim_data, BUS_LAYOUT *bus_layout) ;

static double qcadesigner_version = 2.0 ;

// ---------------------------------------------------------------------------------------- //

gboolean open_project_file (gchar *file_name, DESIGN **pdesign)
  {
  gulong usec = 0 ; // not really a useful variable, but g_timer_elapsed requires a pointer to it
  FILE *pfile = NULL ;
  gboolean bRet = FALSE ;
  GTimer *timer = NULL ;

  if (NULL == (pfile = file_open_and_buffer (file_name)))
    return FALSE ;

  g_timer_start (timer = g_timer_new ()) ;
  set_progress_bar_visible (TRUE) ;
  set_progress_bar_label (_("Opening File:")) ;
  set_progress_bar_fraction (0.0) ;
  bRet = open_project_file_fp (pfile, pdesign) ;
  set_progress_bar_visible (FALSE) ;

  file_close_and_unbuffer (pfile) ;

  if (bRet && NULL != timer)
    command_history_message (_("File opened in %.2lf seconds\n"), g_timer_elapsed (timer, &usec)) ;

  if (NULL != timer) g_timer_destroy (timer) ;

  DBG_OPEN (fprintf (stderr, "open_project_file:returning %s\n", bRet ? "TRUE" : "FALSE")) ;
  return bRet ;
  }

gboolean open_project_file_fp (FILE *pfile, DESIGN **pdesign)
  {
  double version = 0.0 ;
  gboolean bRet = FALSE ;

  if (!check_project_file_magic_fp (pfile, &version))
    return FALSE ;

  if (version < 2.0)
    {
    if (version > 0)
      {
      SkipPast (pfile, '\0', "[#VERSION]", NULL) ;
      bRet = legacy_open_project_file (pfile, pdesign) ;
      return bRet ;
      }
    return FALSE ;
    }

  SkipPast (pfile, '\0', "[#VERSION]", NULL) ;

  bRet = design_unserialize (pdesign, pfile) ;

  (*pdesign)->lstCurrentLayer = (*pdesign)->lstLastLayer ;

  return bRet ;
  }

gboolean check_project_file_magic_fp (FILE *pfile, double *pversion)
  {
  char *pszLine = NULL, *pszValue = NULL ;

  if (!SkipPast (pfile, '\0', "[VERSION]", NULL))
    return FALSE ;

  if (NULL == (pszLine = ReadLine (pfile, '\0', TRUE)))
    return FALSE ;

  tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

  if (!strcmp (pszLine, "qcadesigner_version"))
    (*pversion) = g_ascii_strtod (pszValue, NULL) ;

  g_free (pszLine) ;

  return TRUE ;
  }

gboolean create_file (gchar *file_name, DESIGN *design)
  {
  FILE *pfile = NULL ;

  if (NULL == (pfile = fopen (file_name, "w"))) return FALSE ;

  create_file_fp (pfile, design) ;
  fclose (pfile) ;

  return TRUE ;
  }

void create_file_fp (FILE *pfile, DESIGN *design)
  {
  fprintf (pfile, "[VERSION]\n") ;
  fprintf (pfile, "qcadesigner_version=%lf\n", qcadesigner_version) ;
  fprintf (pfile, "[#VERSION]\n") ;

  design_serialize (design, pfile) ;
  }

void export_block (char *pszFName, DESIGN *design)
  {
  FILE *pfile = NULL ;

  if (NULL == (pfile = fopen (pszFName, "w")))
    return ;

  export_block_fp (pfile, design) ;

  fclose (pfile) ;
  }

void export_block_fp (FILE *pfile, DESIGN *design)
  {
  fprintf (pfile, "[VERSION]\n") ;
  fprintf (pfile, "qcadesigner_version=%lf\n", qcadesigner_version) ;
  fprintf (pfile, "[#VERSION]\n") ;

  design_selection_serialize (design, pfile) ;
  }

// opens a selected file //
static gboolean legacy_open_project_file (FILE *pfile, DESIGN **pdesign)
  {
	FILE *project_file = pfile ;
  QCADLayer *layer = NULL ;
  QCADSubstrate *subs = NULL ;
  QCADDesignObject *obj = NULL ;
  GList *lstLayer = NULL ;

	if (project_file == NULL)
    {
	  g_print("cannot open that file!\n");
	  return FALSE ;
	  }
  else
    {
    (*pdesign) = design_new (&subs) ;
	  DBG_FIO (fprintf (stderr, "Attempting to open project file \"%s\"\n", file_name)) ;
	  // -- Read in the design options from the project file -- //
	  read_options (project_file, &(subs->grid_spacing));
	  DBG_FIO (fprintf (stderr, "After read_options\n")) ;

    layer = NULL ;

    for (lstLayer = (*pdesign)->lstLayers ; lstLayer != NULL ; lstLayer = lstLayer->next)
      if (LAYER_TYPE_CELLS == (QCAD_LAYER (lstLayer->data))->type)
        layer = (QCAD_LAYER (lstLayer->data)) ;

    if (NULL != layer)
      {
	    // -- Read in all the cells in the project file -- //
	    while(!feof(project_file))
        {
	      DBG_FIO (fprintf (stderr, "Calling qcell_new_from_stream\n")) ;
	      if (NULL == (obj = legacy_read_cell_from_stream (project_file)))
      		break ;
	      else
          qcad_do_container_add (QCAD_DO_CONTAINER (layer), obj) ;
//          layer->lstObjs = g_list_prepend (layer->lstObjs, obj) ;
	      DBG_FIO (fprintf (stderr, "Called qcell_new_from_stream\n")) ;
	      }
      }
    }

  design_fix_legacy ((*pdesign)) ;

  return TRUE ;
  }//open_project_file

//!Reads the project options from the project file
static void read_options (FILE *project_file, double *pgrid_spacing)
  {
	char *identifier = NULL;
	//The file read buffer is 80 characters long any lines in the file longer then this will
	//result in unexpected outputs.
	char *buffer = NULL ;

	// -- make sure the file is not NULL -- //
	if (project_file == NULL)
    {
		printf("Cannot extract options from a NULL file\n");
		printf("The pointer to the project file was passed as NULL to read_options()\n");
		return;
  	}

  while (TRUE)
    {
    buffer = ReadLine (project_file, 0, FALSE) ;
    if (NULL == buffer) return ;

	  if(feof(project_file))
      {
	    printf("Premature end of file reached your options may not have been loaded\n");
	    printf("End of file reached while searching for the opening [DESIGN_OPTIONS] tag\n");
      g_free (buffer) ;
	    return;
	    }

    if (!strcmp (buffer, "[DESIGN_OPTIONS]"))
      {
      g_free (buffer) ;
      break ;
      }
    g_free (buffer) ;
    }

	// -- read in the first option --//
  buffer = ReadLine (project_file, 0, FALSE) ;

	// -- keep reading in lines of the file until the end tag is reached -- //
	while (strcmp(buffer, "[#DESIGN_OPTIONS]") != 0)
    {
		// -- get the identifier from the buffer --//
		if ((identifier = get_identifier(buffer)) != NULL)
      {
			if(!strncmp(identifier, "grid_spacing", sizeof("grid_spacing")))
				(*pgrid_spacing) = g_ascii_strtod (strchr (buffer,'=') + sizeof (char), NULL);

			g_free (identifier);
			identifier = NULL;
			}

		if(feof(project_file))
      {
			printf("Premature end of file reached while trying to locate the [#DESIGN_OPTIONS] tag\n");
			return;
			}

		g_free (buffer) ;
    if (NULL == (buffer = ReadLine (project_file, 0, FALSE))) break ;
	  }
  if (NULL != buffer) g_free (buffer) ;
  }//read_options

//!Gets the variable identifier from a text string
static char *get_identifier(char *buffer)
  {
	int i = 0;
	char *identifier = NULL;
	int passed = FALSE;

	// get all the characters up the the =, this should be the entire identifier -- //
	while(i < strlen(buffer))
    {
		if(buffer[i] == '=')
      {
			passed = TRUE;
			break;
			}
	 	i++;
	 	}

	//If = was found allocate and copy the identifier to the string
	if(passed == TRUE)
    {
    identifier = calloc(80, sizeof(char));
    strncpy(identifier, buffer, i);
    }

	return identifier;
  }//get_identifier

static QCADDesignObject *legacy_read_cell_from_stream (FILE *stream)
  {
  char *pszName = NULL, *pszValue = NULL ;

  //The file read buffer is 80 characters long. Any lines in the file longer than this will
  //result in unexpected outputs.
  char *buffer = NULL;
  char *pszLabel = NULL ;
  double dPolarization ;
  QCADCell *cell = g_object_new (QCAD_TYPE_CELL, NULL);
  QCADDesignObject *obj = QCAD_DESIGN_OBJECT (cell) ;
  gboolean bInput = FALSE, bOutput = FALSE, bFixed = FALSE ;
  QCADCellFunction cell_function = 0 ;

  int clock = -1 ;

  // -- The dot that is currently being read to -- //
  int current_dot = 0;

  // -- make sure the file is not NULL -- //
  if(NULL == stream)
    {
    printf("Cannot extract the design from a NULL file\n");
    printf("The pointer to the project file was passed as NULL to read_design()\n");
    g_object_unref (cell) ;
    return NULL;
    }

  // -- read the first line in the file to the buffer -- //
  buffer = ReadLine (stream, 0, FALSE) ;

  // -- check whether that was the end of the file -- //
  if(feof(stream))
    {
    g_object_unref (cell) ;
    if (NULL != buffer) g_free (buffer) ;
    return NULL;
    }

  while (strcmp (buffer, "[QCELL]") != 0)
    {
    g_free (buffer) ;
    buffer = ReadLine (stream, 0, FALSE) ;

    if(feof(stream))
      {
      fprintf (stderr, "Premature end of file reached your design may not have been loaded\n");
      fprintf (stderr, "End of file reached while searching for the opening [QCELL] tag\n");
      if (NULL != buffer) g_free (buffer) ;
      g_object_unref (cell) ;
      return NULL;
      }
    }//while ![QCELL]

  if (NULL != cell->cell_dots)
    g_free (cell->cell_dots) ;
  cell->number_of_dots = 0 ;
  cell->cell_dots = NULL;

  // -- read in the first option --//
  buffer = ReadLine (stream, 0, FALSE) ;

  // -- keep reading in lines of the file until the end tag is reached -- //
  while (strcmp (buffer, "[#QCELL]") != 0)
    {
    tokenize_line (pszName = buffer, strlen (buffer), &pszValue, '=') ;

    // -- Find and set the appropriate property of the cell from the buffer -- //
    if (!strncmp (pszName, "x", sizeof ("x")))
      obj->x = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszName, "y", sizeof ("y")))
      obj->y = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszName, "top_x", sizeof ("top_x")))
      obj->bounding_box.xWorld = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszName, "top_y", sizeof ("top_y")))
      obj->bounding_box.yWorld = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszName, "bot_x", sizeof ("bot_x")))
      obj->bounding_box.cxWorld = g_ascii_strtod (pszValue, NULL) - obj->bounding_box.xWorld ;
    else
    if (!strncmp (pszName, "bot_y", sizeof("bot_y")))
      obj->bounding_box.cyWorld = g_ascii_strtod (pszValue, NULL) - obj->bounding_box.yWorld ;
    else
    if (!strncmp (pszName, "cell_width", sizeof ("cell_width")))
      cell->cell_options.cxCell = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszName, "cell_height", sizeof ("cell_height")))
      cell->cell_options.cyCell = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszName, "clock", sizeof ("clock")))
      clock =
      cell->cell_options.clock = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszName, "is_input", sizeof ("is_input")))
      bInput = atoi (pszValue);
    else
    if (!strncmp (pszName, "is_output", sizeof ("is_output")))
      bOutput = atoi(pszValue);
    else
    if (!strncmp (pszName, "is_fixed", sizeof ("is_fixed")))
      bFixed = atoi(pszValue);
    else
    if (!strncmp (pszName, "number_of_dots", sizeof ("number_of_dots")))
      cell->number_of_dots = atoi (pszValue);
    else
    if (!strncmp (pszName, "label", sizeof ("label")))
      pszLabel = g_strdup (pszValue) ;
    // -- else check if this the opening tag for a QDOT -- //
    else
    if (strcmp (buffer, "[QDOT]") == 0)
      {
      // -- allocate the memory for the cell dots -- //
      if(cell->cell_dots == NULL)
        {
        if(cell->number_of_dots <= 0 )
          {
	        printf("Error attempting to load the dots into a cell: number_of_dots <=0\n");
	        printf("Possibly due to having [QDOT] definition before number_of_dots definition\n");
	        printf("The file has failed to load\n");
	        g_object_unref (cell) ;
	        return NULL;
          }
	      cell->cell_dots = g_malloc0 (sizeof(QCADCellDot) * cell->number_of_dots);
	      current_dot = 0;
        }

      g_free (buffer) ;
      buffer = ReadLine (stream, 0, FALSE) ;

      // -- extract all the data within the [QDOT] tags -- //
      while(strcmp(buffer, "[#QDOT]") != 0)
        {
        tokenize_line (pszName = buffer, strlen (buffer), &pszValue, '=') ;

	      // -- Find and set the appropriate property of the cell from the buffer -- //
	      if (!strncmp (pszName, "x", sizeof ("x")))
		      cell->cell_dots[current_dot].x = g_ascii_strtod (pszValue, NULL) ;
	      else
        if (!strncmp (pszName, "y", sizeof ("y")))
		      cell->cell_dots[current_dot].y = g_ascii_strtod (pszValue, NULL) ;
	      else
        if (!strncmp (pszName, "diameter", sizeof ("diameter")))
		      cell->cell_dots[current_dot].diameter = g_ascii_strtod (pszValue, NULL) ;
	      else
        if(!strncmp (pszName, "charge", sizeof ("charge")))
		      cell->cell_dots[current_dot].charge = g_ascii_strtod (pszValue, NULL) ;
	      else
        if (!strncmp (pszName, "spin", sizeof ("spin")))
		      cell->cell_dots[current_dot].spin = g_ascii_strtod (pszValue, NULL) ;
	      else
        if(!strncmp (pszName, "potential", sizeof ("potenital")))
		      cell->cell_dots[current_dot].potential = g_ascii_strtod (pszValue, NULL) ;

	      if (feof (stream))
          {
	        printf("Premature end of file reached while trying to locate the [#QDOT] tag\n");
          g_object_unref (cell) ;
	        return NULL;
	        }

        g_free (buffer) ;
        buffer = ReadLine (stream, 0, FALSE) ;
        }//while not [#QDOT]

      // -- Increment the dot counter and check if it is out of the bounds set by number_of_dots -- //
      if(++current_dot > cell->number_of_dots)
        {
	      fprintf (stderr, "There appear to be more [QDOTS] then the set number_of_dots in one of the cells\n");
	      fprintf (stderr, "The file has failed to load\n");
	      g_object_unref (cell) ;
        g_free (buffer) ;
	      return NULL;
	      }
      }

    // -- Make sure that the terminating [#QCELL] tag was found prior to the end of the file -- //
    if (feof (stream))
      {
      printf("Premature end of file reached while trying to locate the [#QCELL] tag\n");
      g_object_unref (cell) ;
      g_free (buffer) ;
      return NULL;
      }
    g_free (buffer) ;
    buffer = ReadLine (stream, 0, FALSE) ;
    }

  // -- Make sure that the dots within the cell have been initalized -- //
  if (NULL == cell->cell_dots)
    {
    fprintf (stderr, "QCELL appears to have been loaded without any dots\n");
    fprintf (stderr, "Edit the project file and check for [QDOT] tags\n");
    g_object_unref (cell) ;
    g_free (buffer) ;
    return NULL;
    }

  dPolarization = qcad_cell_calculate_polarization (cell) ;

  cell_function = bInput  ? QCAD_CELL_INPUT  :
                  bOutput ? QCAD_CELL_OUTPUT :
                  bFixed  ? QCAD_CELL_FIXED  : QCAD_CELL_NORMAL ;

  g_object_set (G_OBJECT (cell),
    "function",     cell_function,
    "polarization", dPolarization,
    "clock",        clock,
    NULL) ;
  if (QCAD_CELL_INPUT == cell_function || QCAD_CELL_OUTPUT == cell_function)
    g_object_set (G_OBJECT (cell),
      "label", pszLabel,
      NULL) ;

  g_free (pszLabel) ;
  g_free (buffer) ;
  return obj;
  }//read_design

void create_simulation_output_file (char *pszFName, SIMULATION_OUTPUT *sim_output)
  {
  FILE *fp = NULL ;

  if (NULL == (fp = fopen (pszFName, "w"))) return ;
  else create_simulation_output_file_fp (fp, sim_output) ;
  fclose (fp) ;
  }

void create_simulation_output_file_fp (FILE *pfile, SIMULATION_OUTPUT *sim_output)
  {
  fprintf (pfile, "[SIMULATION_OUTPUT]\n") ;
  if (NULL != sim_output->sim_data)
    simulation_data_serialize (pfile, sim_output->sim_data) ;
  if (NULL != sim_output->bus_layout)
    design_bus_layout_serialize (sim_output->bus_layout, pfile) ;
  fprintf (pfile, "[#SIMULATION_OUTPUT]\n") ;
  }

void simulation_data_serialize (FILE *pfile, simulation_data *sim_data)
  {
  int Nix ;

  fprintf (pfile, "[SIMULATION_DATA]\n") ;
  fprintf (pfile, "number_samples=%d\n", sim_data->number_samples) ;
  fprintf (pfile, "number_of_traces=%d\n", sim_data->number_of_traces) ;
  fprintf (pfile, "[TRACES]\n") ;
  for (Nix = 0 ; Nix < sim_data->number_of_traces ; Nix++)
    serialize_trace (pfile, &(sim_data->trace[Nix]), sim_data->number_samples) ;
  fprintf (pfile, "[#TRACES]\n[CLOCKS]\n") ;
  for (Nix = 0 ; Nix < 4 ; Nix++)
    serialize_trace (pfile, &(sim_data->clock_data[Nix]), sim_data->number_samples) ;
  fprintf (pfile, "[#CLOCKS]\n") ;
  fprintf (pfile, "[#SIMULATION_DATA]\n") ;
  }

static void serialize_trace (FILE *pfile, struct TRACEDATA *trace, int icSamples)
  {
  int Nix ;
  int idx = 0 ;
  int sample_idx = 0 ;

  fprintf (pfile, "[TRACE]\n") ;
  fprintf (pfile, "data_labels=%s\n", trace->data_labels) ;
  fprintf (pfile, "trace_function=%d\n", trace->trace_function) ;
  fprintf (pfile, "drawtrace=%s\n", trace->drawtrace ? "TRUE" : "FALSE") ;
  fprintf (pfile, "[TRACE_DATA]\n") ;
  for (Nix = 0 ; Nix < icSamples - 1 ; Nix += FLOATS_PER_LINE)
    {
    for (idx = 0 ; idx < FLOATS_PER_LINE ; idx++)
      if ((sample_idx = Nix + idx) < icSamples - 1)
        fprintf (pfile, "%e ", trace->data[sample_idx]) ;
      else
        break ;
    if (sample_idx < icSamples - 1)
      fprintf (pfile, "\n") ;
    }

  fprintf (pfile, "%e\n", trace->data[icSamples - 1]) ;

  fprintf (pfile, "[#TRACE_DATA]\n") ;
  fprintf (pfile, "[#TRACE]\n") ;
  }

SIMULATION_OUTPUT *open_simulation_output_file (char *pszFName)
  {
  FILE *pfile = NULL ;
  SIMULATION_OUTPUT *sim_output = NULL ;

  if (NULL == (pfile = file_open_and_buffer (pszFName)))
    return NULL ;

  set_progress_bar_visible (TRUE) ;
  set_progress_bar_label (_("Opening File:")) ;
  set_progress_bar_fraction (0.0) ;

  sim_output = open_simulation_output_file_fp (pfile) ;

  file_close_and_unbuffer (pfile) ;

  set_progress_bar_visible (FALSE) ;

  return sim_output ;
  }

SIMULATION_OUTPUT *open_simulation_output_file_fp (FILE *pfile)
  {
  char *pszLine = NULL ;
  SIMULATION_OUTPUT *sim_output = NULL ;

  if (!SkipPast (pfile, '\0', "[SIMULATION_OUTPUT]", NULL))
    return NULL ;

  DBG_OPEN (fprintf (stderr, "open_simulation_output_file_fp: Skipped past the opening tag\n")) ;

  sim_output = g_malloc0 (sizeof (SIMULATION_OUTPUT)) ;

  sim_output->bFakeIOLists = TRUE ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (pfile, '\0', TRUE))) break ;
    DBG_OPEN (fprintf (stderr, "open_simulation_output_file_fp: Peeked line \"%s\"\n", pszLine)) ;

    if (!strcmp ("[#SIMULATION_OUTPUT]", pszLine))
      {
      g_free (pszLine) ;
      break ;
      }
    set_progress_bar_fraction (get_file_percent (pfile)) ;
    if (!strcmp (pszLine, "[SIMULATION_DATA]"))
      sim_output->sim_data = simulation_data_unserialize (pfile) ;
    else
    if (!strcmp (pszLine, "[TYPE:BUS_LAYOUT]"))
      {
      sim_output->bus_layout = design_bus_layout_unserialize (pfile) ;
      DBG_OPEN (fprintf (stderr, "open_simulation_output_file_fp: Unserialized the following bus_layout:\n")) ;
      DBG_OPEN (design_bus_layout_dump (sim_output->bus_layout, stderr)) ;
      }

    g_free (pszLine) ;
    g_free (ReadLine (pfile, '\0', FALSE)) ;
    }

  if (NULL == sim_output->sim_data)
    {
    if (NULL != sim_output->bus_layout)
      design_bus_layout_free (sim_output->bus_layout) ;
    g_free (sim_output) ;
    sim_output = NULL ;
    }

  if (NULL == sim_output->bus_layout)
    sim_output->bus_layout = design_bus_layout_new () ;
  if (NULL != sim_output->bus_layout)
    build_io_tables (sim_output->sim_data, sim_output->bus_layout) ;

  return sim_output ;
  }

simulation_data *simulation_data_unserialize (FILE *pfile)
  {
  simulation_data *sim_data = NULL ;
  char *pszLine = NULL, *pszValue = NULL ;
  int idxTrace = 0 ;

  if (!SkipPast (pfile, '\0', "[SIMULATION_DATA]", NULL))
    return NULL ;

  sim_data = g_malloc0 (sizeof (simulation_data)) ;
  sim_data->number_samples = -1 ;
  sim_data->number_of_traces = -1 ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (pfile, '\0', TRUE))) break ;

    if (!strcmp ("[#SIMULATION_DATA]", pszLine))
      {
      g_free (pszLine) ;
      break ;
      }

    tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;
    set_progress_bar_fraction (get_file_percent (pfile)) ;

    if (!strcmp (pszLine, "number_samples"))
      sim_data->number_samples = atoi (pszValue) ;
    else
    if (!strcmp (pszLine, "number_of_traces"))
      sim_data->number_of_traces = atoi (pszValue) ;
    else
    if (!strncmp (pszLine, "[TRACES]", 8))
      {
      // By now, we must have read the number of traces and samples.
      if (-1 == sim_data->number_of_traces || -1 == sim_data->number_samples)
        {
        g_free (pszLine) ;
        simulation_data_destroy (sim_data) ;
        return NULL ;
        }
      else
        {
        sim_data->trace = g_malloc0 (sim_data->number_of_traces * sizeof (struct TRACEDATA)) ;
        for (idxTrace = 0 ; idxTrace < sim_data->number_of_traces ; idxTrace++)
          unserialize_trace (pfile, &(sim_data->trace[idxTrace]), sim_data->number_samples) ;
        }
      }
    else
    if (!strncmp (pszLine, "[CLOCKS]", 8))
      {
      if (-1 == sim_data->number_samples)
        {
        g_free (pszLine) ;
        simulation_data_destroy (sim_data) ;
        return NULL ;
        }
      else
        {
        sim_data->clock_data = g_malloc0 (4 * sizeof (struct TRACEDATA)) ;
        for (idxTrace = 0 ; idxTrace < 4 ; idxTrace++)
          unserialize_trace (pfile, &(sim_data->clock_data[idxTrace]), sim_data->number_samples) ;
        }
      }
    g_free (pszLine) ;
    g_free (ReadLine (pfile, '\0', FALSE)) ;
    }
  return sim_data ;
  }

coherence_OP *open_coherence_options_file (char *pszFName)
  {
  coherence_OP *coherence_options = NULL ;
  FILE *fp = NULL ;

  if (NULL == (fp = file_open_and_buffer (pszFName)))
    return NULL ;

  coherence_options = open_coherence_options_file_fp (fp) ;

  file_close_and_unbuffer (fp) ;

  return coherence_options ;
  }
  
coherence_energy_OP *open_coherence_energy_options_file (char *pszFName)
  {
  coherence_energy_OP *coherence_energy_options = NULL ;
  FILE *fp = NULL ;

  if (NULL == (fp = file_open_and_buffer (pszFName)))
    return NULL ;

  coherence_energy_options = open_coherence_energy_options_file_fp (fp) ;

  file_close_and_unbuffer (fp) ;

  return coherence_energy_options ;
  }

static coherence_OP *open_coherence_options_file_fp (FILE *pfile)
  {
  coherence_OP *coherence_options = NULL ;
  char *pszLine = NULL, *pszValue = NULL ;

  if (!SkipPast (pfile, '\0', "[COHERENCE_OPTIONS]", NULL))
    return NULL ;

  coherence_options = g_malloc0 (sizeof (coherence_OP)) ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (pfile, '\0', TRUE))) break ;

    if (!strncmp (pszLine, "[#COHERENCE_OPTIONS]", sizeof ("[#COHERENCE_OPTIONS]") - 1))
      {
      g_free (pszLine) ;
      break ;
      }

    tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

    if (!strncmp (pszLine, "T", sizeof ("T") - 1))
      coherence_options->T = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "relaxation", sizeof ("relaxation") - 1))
      coherence_options->relaxation = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "time_step", sizeof ("time_step") - 1))
      coherence_options->time_step = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "relaxation", sizeof ("relaxation") - 1))
      coherence_options->relaxation = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "duration", sizeof ("duration") - 1))
      coherence_options->duration = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "clock_high", sizeof ("clock_high") - 1))
      coherence_options->clock_high = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "clock_low", sizeof ("clock_low") - 1))
      coherence_options->clock_low = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "clock_shift", sizeof ("clock_shift") - 1))
      coherence_options->clock_shift = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "clock_amplitude_factor", sizeof ("clock_amplitude_factor") - 1))
      coherence_options->clock_amplitude_factor = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "radius_of_effect", sizeof ("radius_of_effect") - 1))
      coherence_options->radius_of_effect = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "epsilonR", sizeof ("epsilonR") - 1))
      coherence_options->epsilonR = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "layer_separation", sizeof ("layer_separation") - 1))
      coherence_options->layer_separation = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "algorithm", sizeof ("algorithm") - 1))
      coherence_options->algorithm = atoi (pszValue) ;
    else
    if (!strncmp (pszLine, "randomize_cells", sizeof ("randomize_cells") - 1))
      coherence_options->randomize_cells = !strncmp (pszValue, "TRUE", sizeof ("TRUE") - 1) ? TRUE : FALSE ;
    else
    if (!strncmp (pszLine, "animate_simulation", sizeof ("animate_simulation") - 1))
      coherence_options->animate_simulation = !strncmp (pszValue, "TRUE", sizeof ("TRUE") - 1) ? TRUE : FALSE ;

    g_free (pszLine) ;
    g_free (ReadLine (pfile, '\0', FALSE)) ;
    }

  return coherence_options ;
  }

static coherence_energy_OP *open_coherence_energy_options_file_fp (FILE *pfile)
  {
  coherence_energy_OP *coherence_energy_options = NULL ;
  char *pszLine = NULL, *pszValue = NULL ;

  if (!SkipPast (pfile, '\0', "[COHERENCE_ENERGY_OPTIONS]", NULL))
    return NULL ;

  coherence_energy_options = g_malloc0 (sizeof (coherence_energy_OP)) ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (pfile, '\0', TRUE))) break ;

    if (!strncmp (pszLine, "[#COHERENCE_ENERGY_OPTIONS]", sizeof ("[#COHERENCE_ENERGY_OPTIONS]") - 1))
      {
      g_free (pszLine) ;
      break ;
      }

    tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

    if (!strncmp (pszLine, "T", sizeof ("T") - 1))
      coherence_energy_options->T = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "relaxation", sizeof ("relaxation") - 1))
      coherence_energy_options->relaxation = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "time_step", sizeof ("time_step") - 1))
      coherence_energy_options->time_step = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "relaxation", sizeof ("relaxation") - 1))
      coherence_energy_options->relaxation = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "duration", sizeof ("duration") - 1))
      coherence_energy_options->duration = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "clock_high", sizeof ("clock_high") - 1))
      coherence_energy_options->clock_high = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "clock_low", sizeof ("clock_low") - 1))
      coherence_energy_options->clock_low = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "clock_shift", sizeof ("clock_shift") - 1))
      coherence_energy_options->clock_shift = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "radius_of_effect", sizeof ("radius_of_effect") - 1))
      coherence_energy_options->radius_of_effect = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "epsilonR", sizeof ("epsilonR") - 1))
      coherence_energy_options->epsilonR = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "layer_separation", sizeof ("layer_separation") - 1))
      coherence_energy_options->layer_separation = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "algorithm", sizeof ("algorithm") - 1))
      coherence_energy_options->algorithm = atoi (pszValue) ;
    else
    if (!strncmp (pszLine, "randomize_cells", sizeof ("randomize_cells") - 1))
      coherence_energy_options->randomize_cells = !strncmp (pszValue, "TRUE", sizeof ("TRUE") - 1) ? TRUE : FALSE ;
    else
    if (!strncmp (pszLine, "animate_simulation", sizeof ("animate_simulation") - 1))
      coherence_energy_options->animate_simulation = !strncmp (pszValue, "TRUE", sizeof ("TRUE") - 1) ? TRUE : FALSE ;

    g_free (pszLine) ;
    g_free (ReadLine (pfile, '\0', FALSE)) ;
    }

  return coherence_energy_options ;
  }

bistable_OP *open_bistable_options_file (char *pszFName)
  {
  bistable_OP *bistable_options = NULL ;
  FILE *fp = NULL ;

  if (NULL == (fp = file_open_and_buffer (pszFName)))
    return NULL ;

  bistable_options = open_bistable_options_file_fp (fp) ;

  file_close_and_unbuffer (fp) ;

  return bistable_options ;
  }

static bistable_OP *open_bistable_options_file_fp (FILE *pfile)
  {
  bistable_OP *bistable_options = NULL ;
  char *pszLine = NULL, *pszValue = NULL ;

  if (!SkipPast (pfile, '\0', "[BISTABLE_OPTIONS]", NULL))
    return NULL ;

  bistable_options = g_malloc0 (sizeof (bistable_OP)) ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (pfile, '\0', TRUE))) break ;

    if (!strncmp (pszLine, "[#BISTABLE_OPTIONS]", sizeof ("[#BISTABLE_OPTIONS]") - 1))
      {
      g_free (pszLine) ;
      break ;
      }

    tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

    if (!strncmp (pszLine, "number_of_samples", sizeof ("number_of_samples") - 1))
      bistable_options->number_of_samples = atoi (pszValue) ;
    else
    if (!strncmp (pszLine, "animate_simulation", sizeof ("animate_simulation") - 1))
      bistable_options->animate_simulation = !strncmp (pszValue, "TRUE", sizeof ("TRUE") - 1) ? TRUE : FALSE ;
    else
    if (!strncmp (pszLine, "convergence_tolerance", sizeof ("convergence_tolerance") - 1))
      bistable_options->convergence_tolerance = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "radius_of_effect", sizeof ("radius_of_effect") - 1))
      bistable_options->radius_of_effect = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "epsilonR", sizeof ("epsilonR") - 1))
      bistable_options->epsilonR = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "clock_high", sizeof ("clock_high") - 1))
      bistable_options->clock_high = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "clock_low", sizeof ("clock_low") - 1))
      bistable_options->clock_low = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "clock_shift", sizeof ("clock_shift") - 1))
      bistable_options->clock_shift = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "clock_amplitude_factor", sizeof ("clock_amplitude_factor") - 1))
      bistable_options->clock_amplitude_factor = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "max_iterations_per_sample", sizeof ("max_iterations_per_sample") - 1))
      bistable_options->max_iterations_per_sample = atoi (pszValue) ;
    else
    if (!strncmp (pszLine, "layer_separation", sizeof ("layer_separation") - 1))
      bistable_options->layer_separation = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strncmp (pszLine, "randomize_cells", sizeof ("randomize_cells") - 1))
      bistable_options->randomize_cells = !strncmp (pszValue, "TRUE", sizeof ("TRUE") - 1) ? TRUE : FALSE ;

    g_free (pszLine) ;
    g_free (ReadLine (pfile, '\0', FALSE)) ;
    }

  return bistable_options ;
  }

// Best effort trace unserialization - in the worst case, it'll be a flatline.
static void unserialize_trace (FILE *pfile, struct TRACEDATA *trace, int icSamples)
  {
  char *pszLine = NULL, *pszValue = NULL ;

  trace->data = g_malloc0 (icSamples * sizeof (double)) ;
  trace->data_labels = NULL ;
  trace->trace_function = QCAD_CELL_NORMAL ;
  trace->drawtrace = TRUE ;

  set_progress_bar_fraction (get_file_percent (pfile)) ;

  if (!SkipPast (pfile, '\0', "[TRACE]", NULL))
    return ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (pfile, '\0', TRUE))) break ;

    if (!strcmp (pszLine, "[#TRACE]"))
      {
      g_free (pszLine) ;
      break ;
      }

    tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

    if (!strcmp (pszLine, "data_labels"))
      trace->data_labels = g_strdup (pszValue) ;
    else
    if (!strcmp (pszLine, "trace_function"))
      trace->trace_function = atoi (pszValue) ;
    else
    if (!strcmp (pszLine, "drawtrace"))
      trace->drawtrace = strcmp (pszValue, "FALSE") ? TRUE : FALSE ;
    else
    if (!strncmp (pszLine, "[TRACE_DATA]", 12))
      unserialize_trace_data (pfile, trace, icSamples) ;
    g_free (pszLine) ;
    g_free (ReadLine (pfile, '\0', FALSE)) ;
    }
  }

static void unserialize_trace_data (FILE *pfile, struct TRACEDATA *trace, int icSamples)
  {
  char *pszLine = NULL ;
  int idx = 0 ;
  char *pszValue = NULL, *pszItr = NULL ;

  if (!SkipPast (pfile, '\0', "[TRACE_DATA]", NULL))
    return ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (pfile, '\0', TRUE))) break ;

    if (!strcmp (pszLine, "[#TRACE_DATA]"))
      {
      g_free (pszLine) ;
      break ;
      }

    pszItr = pszLine ;
    while (NULL != (pszValue = next_space_separated_value (&pszItr)) && idx < icSamples)
      {
      trace->data[idx++] = g_ascii_strtod (pszValue, NULL) ;
      if (0 == idx % 1000)
        set_progress_bar_fraction (get_file_percent (pfile)) ;
      }

    g_free (pszLine) ;
    g_free (ReadLine (pfile, '\0', FALSE)) ;
    }
  }

static void build_io_tables (simulation_data *sim_data, BUS_LAYOUT *bus_layout)
  {
  int Nix, Nix1 ;
  EXP_ARRAY *cell_list = NULL ;
  BUS_LAYOUT_CELL blcell = {NULL, FALSE} ;
  BUS *bus = NULL ;

  if (NULL == bus_layout) return ;

  if (NULL == sim_data) return ;

  for (Nix = 0 ; Nix < sim_data->number_of_traces ; Nix++)
    {
    blcell.cell = QCAD_CELL (qcad_cell_new_with_function ((sim_data->trace)[Nix].trace_function, (sim_data->trace)[Nix].data_labels)) ;
    cell_list = (QCAD_CELL_INPUT == blcell.cell->cell_function) ? bus_layout->inputs : bus_layout->outputs ;
    exp_array_insert_vals (cell_list, &blcell, 1, 1, -1) ;
    }

  for (Nix = 0 ; Nix < bus_layout->buses->icUsed ; Nix++)
    {
    bus = &(exp_array_index_1d (bus_layout->buses, BUS, Nix)) ;
    for (Nix1 = 0 ; Nix1 < bus->cell_indices->icUsed ; Nix1++)
      exp_array_index_1d (
        (QCAD_CELL_INPUT == bus->bus_function) ?
          bus_layout->inputs : bus_layout->outputs,
        BUS_LAYOUT_CELL,
        exp_array_index_1d (bus->cell_indices, int, Nix1)).bIsInBus = TRUE ;
    }
  }
