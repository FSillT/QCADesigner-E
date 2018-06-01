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
// A PostScript printer for graph traces.               //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "design.h"
#include "print.h"
#include "support.h"
#include "print_util.h"
#include "generic_utils.h"
#include "simulation.h"
#include "graph_dialog_widget_data.h"
#include "global_consts.h"

#define GAP_HEIGHT 10
#define TRACE_GAP_HEIGHT 5
#define TRACE_HEADER_WIDTH 120.0

#define GAP_PERCENT 0.01
#define TRACE_GAP_PERCENT 0.005
#define FONT_SIZE 12 /* points */

#define RULER_HEIGHT 12 /* points */
#define RULER_GRAD_HEIGHT 4 /* points */
#define RULER_SMALL_GRAD_HEIGHT 2 /* points */
#define RULER_FONT_SIZE 8 /* points */

#define OBJ_TYPE_PATH 0

#define DBG_PG(s)

typedef struct
  {
  double x ;
  double y ;
  } DPOINT ;

typedef struct
  {
  int idxX ;
  int idxY ;
  } PAGE_IDX_2D ;

typedef struct
  {
  EXP_ARRAY *strings ;
  EXP_ARRAY *pages ;
  } PAGES ;

static void SimDataToPageData       (print_graph_OP *pPO, PAGES *pPages, PRINT_GRAPH_DATA *print_graph_data) ;
static void PlaceSingleTrace        (print_graph_OP *pPO, PAGES *pPages, struct TRACEDATA *ptd,   int idx, int icVisibleTraces, double dcxEffective, double dcyEffective, double dcyTrace, int icSamples, double dTraceMin, double dTraceMax) ;
static void PlaceSingleWaveform     (print_graph_OP *pPO, PAGES *pPages, struct TRACEDATA *ptd, double dxMin, double dyMin, double dxMax, double dyMax, int icSamples, double dTraceMin, double dTraceMax) ;
static void PlaceSingleBusTrace     (print_graph_OP *pPO, PAGES *pPages, HONEYCOMB_DATA *hc_data, BUS *bus, int idx, int icVisibleTraces, double dcxEffective, double dcyEffective, double dcyTrace, int icSamples, int base) ;
static void PlaceSingleTraceBox     (print_graph_OP *pPO, PAGES *pPages, double dyTraceTop, double dcxTraceHeader, double dcxEffective, double dcyEffective, double dcyTrace, gboolean bDrawMidLine) ;
static void PlaceSingleBox          (print_graph_OP *pPO, PAGES *pPages, double dxMin, double dyMin, double dxMax, double dyMax, double dcxEffective, double dcyEffective) ;
static void PlaceSingleString       (print_graph_OP *pPO, PAGES *pPages, double dcxEffective, double dcyEffective, double dx, double dy, char *pszClr, char *pszClrType, char *pszString, int font_size, char *pszStringPlacement) ;
static void PlaceSingleLine         (print_graph_OP *pPO, PAGES *pPages, double dxMin, double dyMin, double dxMax, double dyMax, double dcxEffective, double dcyEffective, char *pszClr, char *pszClrType, char *pszDash) ;
static void PlaceSingleHoneycomb    (print_graph_OP *pPO, PAGES *pPages, double dxMin, double dxL, double dxR, double dxMax, double dyMin, double dyMax, char *pszLabel, double dcxEffective, double dcyEffective, char *pszClr, char *pszClrType) ;
static void PlaceRuler              (print_graph_OP *pPO, PAGES *pPages, simulation_data *sim_data, int idx, int icVisibleTraces, double dcxEffective, double dcyEffective, double dcyRuler) ;
static int GetPageIndex (double dCoord, int icPages, double dEffectiveLength) ;
static void PrintSingleGraphPage (print_graph_OP *pPO, FILE *pfile, EXP_ARRAY *page, int idx) ;
static EXP_ARRAY *get_polyline_page_indices (print_graph_OP *pPO, DPOINT *pts, int icPts, gboolean bClose, double dcxEffective, double dcyEffective) ;

void print_graphs (print_graph_OP *pPrintOpts, PRINT_GRAPH_DATA *print_graph_data)
  {
  int Nix ;
  FILE *pfile = NULL ;
  PAGES pg = {NULL, NULL} ;
  char *pszClr = pPrintOpts->bPrintClr ? PS_GREEN : "0.00",
       *pszClrHCFill = pPrintOpts->bPrintClr ? PS_HCFILL : "0.90",
       *pszClrType = pPrintOpts->bPrintClr ? "setrgbcolor" : "setgray" ;


  if (NULL == (pfile = OpenPrintStream ((print_OP *)pPrintOpts)))
    {
    fprintf (stderr, "Failed to open print stream.\n") ;
    return ;
    }

  SimDataToPageData (pPrintOpts, &pg, print_graph_data) ;

  if (NULL == pg.pages)
    {
    fprintf (stderr, "Failed to create pages.\n") ;
    ClosePrintStream (pfile, (print_OP *)pPrintOpts) ;
    return ;
    }

  fprintf (pfile,
    "%%!PS-Adobe 3.0\n"
    "%%%%Pages: (atend)\n"
    "%%%%Orientation: %s\n"
    "%%%%BoundingBox: 0 0 %d %d\n"
    "%%%%HiResBoundingBox: %f %f %f %f\n"
    "%%........................................................\n"
    "%%%%Creator: QCADesigner\n"
    "%%%%EndComments\n",
    pPrintOpts->po.bPortrait ? "Portrait" : "Landscape",
    (int)(pPrintOpts->po.dPaperCX), (int)(pPrintOpts->po.dPaperCY),
    0.0, 0.0, pPrintOpts->po.dPaperCX, pPrintOpts->po.dPaperCY) ;

  // Print prolog at this point, if any
  fprintf (pfile,
    "%%%%BeginProlog\n"
    "/labelfontsize %d def\n"
    "/txtlt { /font_size exch def gsave dup 0 -1 font_size mul rmoveto show grestore } def\n"
    "/txtlm { /font_size exch def gsave dup 0 font_size 2 div -1 mul rmoveto show grestore } def\n"
    "/txtlb { /font_size exch def gsave dup 0 0 rmoveto show grestore } def\n"
    "/txtct { /font_size exch def gsave dup stringwidth exch 2 div -1 mul exch pop font_size -1 mul rmoveto show grestore } def\n"
    "/txtcm { /font_size exch def gsave dup stringwidth exch 2 div -1 mul exch pop font_size 2 div -1 mul rmoveto show grestore } def\n"
    "/txtcb { /font_size exch def gsave dup stringwidth pop 2 div -1 mul 0 rmoveto show grestore } def\n"
    "/txtrt { /font_size exch def gsave dup stringwidth exch -1 mul exch pop font_size -1 mul rmoveto show grestore } def\n"
    "/txtrm { /font_size exch def gsave dup stringwidth exch -1 mul exch pop font_size 2 div -1 mul rmoveto show grestore } def\n"
    "/txtrb { /font_size exch def gsave dup stringwidth exch -1 mul exch pop 0 rmoveto show grestore } def\n"
    "/point {} def\n"
    "%%/point { -3 -3 rmoveto 6 6 rlineto -6 0 rmoveto 6 -6 rlineto -3 3 rmoveto } def\n"
    "\n"
    "%%xTop yTop xBot yBot tracebox\n"
    "/tracebox\n"
    "  {\n"
    "  /yBot exch def\n"
    "  /xBot exch def\n"
    "  /yTop exch def\n"
    "  /xTop exch def\n"
    "\n"
    "  gsave\n"
    "  %s %s\n"
    "  newpath\n"
    "  xTop yTop moveto\n"
    "  xBot yTop lineto\n"
    "  xBot yBot lineto\n"
    "  xTop yBot lineto\n"
    "  closepath stroke\n"
    "  grestore\n"
    "  } def\n"
    "\n"
    "%%xMin xL xR xMax yTop yBot (label) honeycomb\n"
    "/honeycomb\n"
    "  {\n"
    "  /label exch def\n"
    "  /yBot exch def\n"
    "  /yTop exch def\n"
    "  /xMax exch def\n"
    "  /xR exch def\n"
    "  /xL exch def\n"
    "  /xMin exch def\n"
    "\n"
    "  /yMid yBot yTop add 2 div def\n"
    "\n"
    "  gsave\n"
    "  newpath\n"
    "  %s %s\n"
    "  xMin yMid moveto xL yTop lineto xR yTop lineto xMax yMid lineto xR yBot lineto xL yBot lineto\n"
    "  closepath\n"
    "  fill\n"
    "  grestore\n"
    "\n"
    "  gsave\n"
    "  newpath\n"
    "  xMin yMid moveto xL yTop lineto xR yTop lineto xMax yMid lineto xR yBot lineto xL yBot lineto\n"
    "  closepath\n"
    "  stroke\n"
    "  newpath\n"
    "  xL xR add 2 div yMid moveto\n"
    "  (" PS_FONT ") findfont labelfontsize scalefont setfont\n"
    "  label labelfontsize txtcm\n"
    "  stroke\n"
    "  grestore\n"
    "  } def\n"
    "\n"
    "2 setlinejoin 2 setlinecap\n"
    "%%%%EndProlog\n", FONT_SIZE,
    pszClr, pszClrType,
    pszClrHCFill, pszClrType) ;

  for (Nix = 0 ; Nix < pg.pages->icUsed ; Nix++)
    PrintSingleGraphPage (pPrintOpts, pfile, exp_array_index_1d (pg.pages, EXP_ARRAY *, Nix), Nix) ;

  fprintf (pfile,
    "%%%%Trailer\n"
    "%%%%Pages: %d\n"
    "%%%%EOF\n",
    pg.pages->icUsed) ;

  for (Nix = 0 ; Nix < pg.strings->icUsed ; Nix++)
    g_free (exp_array_index_1d (pg.strings, char *, Nix)) ;

  exp_array_free (pg.strings) ;
  exp_array_free (pg.pages) ;

  ClosePrintStream (pfile, (print_OP *)pPrintOpts) ;
  }

static void PrintSingleGraphPage (print_graph_OP *pPO, FILE *pfile, EXP_ARRAY *page, int idx)
  {
  int Nix ;

  fprintf (pfile, "%%%%Page: %d %d\n", idx + 1, idx + 1) ;

  fprintf (pfile,
    "%% margins\n"
    "gsave\n"
    "newpath\n" // The margins
    "%f %f moveto\n"
    "%f %f lineto\n"
    "%f %f lineto\n"
    "%f %f lineto\n"
    "closepath eoclip\n\n", // Need eoclip here
    pPO->po.dLMargin, pPO->po.dBMargin,
    pPO->po.dLMargin, pPO->po.dPaperCY - pPO->po.dTMargin,
    pPO->po.dPaperCX - pPO->po.dRMargin, pPO->po.dPaperCY - pPO->po.dTMargin,
    pPO->po.dPaperCX - pPO->po.dRMargin, pPO->po.dBMargin) ;

  DBG_PG (fprintf (stderr, "There are %d objects on page %d\n", pPage->icObjects, idx)) ;

  for (Nix = 0 ; Nix < page->icUsed ; Nix++)
    fprintf (pfile, "%s", exp_array_index_1d (page, char *, Nix)) ;

  fprintf (pfile,
    "grestore\n"
    "showpage\n"
    "%%%%PageTrailer\n") ;
  }

static void SimDataToPageData (print_graph_OP *pPO, PAGES *pPages, PRINT_GRAPH_DATA *print_graph_data)
  {
  int idxTraceOnPg = -1 ;
  int icVisibleTraces = 0 ;
  int Nix, Nix1, xIdxTitle, yIdxTitle, idxPg ;
  BUS *bus = NULL ;
  HONEYCOMB_DATA *hc_data = NULL ;
  struct TRACEDATA *trace = NULL ;
  double
    dcxEffective = 0,
    dcyEffective = 0,
    dcyTrace = 0,
    dxTitle, dyTitle, dTraceMinVal = -1.0, dTraceMaxVal = 0.0 ;
  char *psz = NULL ;

  if (NULL == pPages || NULL == pPO || NULL == print_graph_data) return ;

  // Count traces
  // Count buses
  for (Nix = 0 ; Nix < print_graph_data->bus_layout->buses->icUsed ; Nix++)
    {
    if (exp_array_index_1d (print_graph_data->bus_traces, GRAPH_DATA *, Nix)->bVisible)
      icVisibleTraces++ ;
    bus = &exp_array_index_1d (print_graph_data->bus_layout->buses, BUS, Nix) ;
    for (Nix1 = 0 ; Nix1 < bus->cell_indices->icUsed ; Nix1++)
      if (print_graph_data->sim_data->trace[exp_array_index_1d (bus->cell_indices, int, Nix1) + (QCAD_CELL_INPUT == bus->bus_function ? 0 : print_graph_data->bus_layout->inputs->icUsed)].drawtrace)
        icVisibleTraces++ ;
    }
  // Count remaining loose inputs
  for (Nix = 0 ; Nix < print_graph_data->bus_layout->inputs->icUsed ; Nix++)
    if (!exp_array_index_1d (print_graph_data->bus_layout->inputs, BUS_LAYOUT_CELL, Nix).bIsInBus)
      if (print_graph_data->sim_data->trace[Nix].drawtrace)
        icVisibleTraces++ ;
  // Count remaining loose outputs
  for (Nix = 0 ; Nix < print_graph_data->bus_layout->outputs->icUsed ; Nix++)
    if (!exp_array_index_1d (print_graph_data->bus_layout->outputs, BUS_LAYOUT_CELL, Nix).bIsInBus)
      if (print_graph_data->sim_data->trace[Nix + print_graph_data->bus_layout->inputs->icUsed].drawtrace)
        icVisibleTraces++ ;
  // Count clocks
  for (Nix = 0 ; Nix < 4 ; Nix++)
    if (print_graph_data->sim_data->clock_data[Nix].drawtrace)
      icVisibleTraces++ ;

  if (!icVisibleTraces) return ;

  // Precalculate some parameters for each trace
  dcxEffective = pPO->iCXPages * (pPO->po.dPaperCX - pPO->po.dLMargin - pPO->po.dRMargin) ;
  dcyEffective = pPO->iCYPages * (pPO->po.dPaperCY - pPO->po.dTMargin - pPO->po.dBMargin) ;
  dcyTrace = (((dcyEffective - (FONT_SIZE + GAP_HEIGHT)) - (GAP_HEIGHT * icVisibleTraces)) - RULER_HEIGHT) / icVisibleTraces ;

  pPages->strings = exp_array_new (sizeof (char *), 1) ;
  pPages->pages = exp_array_new (sizeof (char *), 2) ;

  // Place objects
  // Place buses
  for (Nix = 0 ; Nix < print_graph_data->bus_layout->buses->icUsed ; Nix++)
    {
    if ((hc_data = exp_array_index_1d (print_graph_data->bus_traces, HONEYCOMB_DATA *, Nix))->graph_data.bVisible)
      PlaceSingleBusTrace (pPO, pPages, hc_data, &(exp_array_index_1d (print_graph_data->bus_layout->buses, BUS, Nix)), ++idxTraceOnPg, icVisibleTraces, dcxEffective, dcyEffective, dcyTrace, print_graph_data->sim_data->number_samples, print_graph_data->honeycomb_base) ;
    bus = &(exp_array_index_1d (print_graph_data->bus_layout->buses, BUS, Nix)) ;
    for (Nix1 = 0 ; Nix1 < bus->cell_indices->icUsed ; Nix1++)
      if ((trace = &(print_graph_data->sim_data->trace[exp_array_index_1d (bus->cell_indices, int, Nix1) + (QCAD_CELL_INPUT == bus->bus_function ? 0 : print_graph_data->bus_layout->inputs->icUsed)]))->drawtrace)
        PlaceSingleTrace (pPO, pPages, trace, ++idxTraceOnPg, icVisibleTraces, dcxEffective, dcyEffective, dcyTrace, print_graph_data->sim_data->number_samples, -1.0, 1.0) ;
    }
  // Place remaining loose inputs
  for (Nix = 0 ; Nix < print_graph_data->bus_layout->inputs->icUsed ; Nix++)
    if (!exp_array_index_1d (print_graph_data->bus_layout->inputs, BUS_LAYOUT_CELL, Nix).bIsInBus)
      if ((trace = &(print_graph_data->sim_data->trace[Nix]))->drawtrace)
        PlaceSingleTrace (pPO, pPages, trace, ++idxTraceOnPg, icVisibleTraces, dcxEffective, dcyEffective, dcyTrace, print_graph_data->sim_data->number_samples, -1.0, 1.0) ;
  // Place remaining loose outputs
  for (Nix = 0 ; Nix < print_graph_data->bus_layout->outputs->icUsed ; Nix++)
    if (!exp_array_index_1d (print_graph_data->bus_layout->outputs, BUS_LAYOUT_CELL, Nix).bIsInBus)
      if ((trace = &(print_graph_data->sim_data->trace[Nix + print_graph_data->bus_layout->inputs->icUsed]))->drawtrace)
        PlaceSingleTrace (pPO, pPages, trace, ++idxTraceOnPg, icVisibleTraces, dcxEffective, dcyEffective, dcyTrace, print_graph_data->sim_data->number_samples, -1.0, 1.0) ;
  // Place clocks
  for (Nix = 0 ; Nix < 4 ; Nix++)
    if ((trace = &print_graph_data->sim_data->clock_data[Nix])->drawtrace)
      {
      tracedata_get_min_max (trace, 0, print_graph_data->sim_data->number_samples - 1, &dTraceMinVal, &dTraceMaxVal) ;
      PlaceSingleTrace (pPO, pPages, &(print_graph_data->sim_data->clock_data[Nix]), ++idxTraceOnPg, icVisibleTraces, dcxEffective, dcyEffective, dcyTrace, print_graph_data->sim_data->number_samples, dTraceMinVal, dTraceMaxVal) ;
      }

  PlaceRuler (pPO, pPages, print_graph_data->sim_data, idxTraceOnPg, icVisibleTraces, dcxEffective, dcyEffective, RULER_HEIGHT) ;

  // Graph title ("Simulation Results")
  dxTitle = dcxEffective / 2 ;
  dyTitle = 0 ;
  xIdxTitle = GetPageIndex (dxTitle, pPO->iCXPages, dcxEffective) ;
  yIdxTitle = GetPageIndex (dyTitle, pPO->iCYPages, dcyEffective) ;
  idxPg = pPO->bPrintOrderOver ? yIdxTitle * pPO->iCXPages + xIdxTitle : xIdxTitle * pPO->iCYPages + yIdxTitle ;

  psz = g_strdup_printf (
    "gsave\n"
    "newpath\n"
    "(" PS_FONT ") findfont labelfontsize scalefont setfont\n"
    "%lf %lf moveto\n"
    "(%s) labelfontsize txtct\n"
    "stroke\n"
    "grestore\n",
    dxTitle - xIdxTitle * (dcxEffective / pPO->iCXPages) + pPO->po.dLMargin,
    pPO->po.dPaperCY -
   (dyTitle - yIdxTitle * (dcyEffective / pPO->iCYPages) + pPO->po.dTMargin),
   _("Simulation Results")) ;

  exp_array_insert_vals (pPages->strings, &psz, 1, 1, -1) ;
  exp_array_insert_vals (pPages->pages, &psz, 1, 1, idxPg, -1) ;
  }

static void PlaceSingleTrace (print_graph_OP *pPO, PAGES *pPages, struct TRACEDATA *ptd, int idx, int icVisibleTraces, double dcxEffective, double dcyEffective, double dcyTrace, int icSamples, double dMinTrace, double dMaxTrace)
  {
  char *psz = NULL ;
  double
    dcxTraceHeader = TRACE_HEADER_WIDTH, // points
    dyTraceTop = idx * (GAP_HEIGHT + dcyTrace) + (FONT_SIZE + GAP_HEIGHT),
    dTraceMaxVal = 0, dTraceMinVal = 0 ;
  char *pszClr =
    QCAD_CELL_FIXED  == ptd->trace_function ? PS_RED    :
    QCAD_CELL_OUTPUT == ptd->trace_function ? PS_YELLOW :
    QCAD_CELL_INPUT  == ptd->trace_function ? PS_BLUE   : PS_BLACK ;

  DBG_PG (fprintf (stderr, "Entering PlaceSingleTrace\n")) ;

  PlaceSingleTraceBox (pPO, pPages, dyTraceTop, dcxTraceHeader, dcxEffective, dcyEffective, dcyTrace, TRUE) ;

  PlaceSingleWaveform (pPO, pPages, ptd,
    dcxTraceHeader + GAP_HEIGHT + TRACE_GAP_HEIGHT,
    dyTraceTop + dcyTrace - TRACE_GAP_HEIGHT,
    dcxEffective - TRACE_GAP_HEIGHT,
    dyTraceTop + TRACE_GAP_HEIGHT, icSamples, dMinTrace, dMaxTrace) ;

  // Filling in the trace header
  tracedata_get_min_max (ptd, 0, icSamples -1, &dTraceMinVal, &dTraceMaxVal) ;

  PlaceSingleString (pPO, pPages, dcxEffective, dcyEffective, TRACE_GAP_HEIGHT, dyTraceTop + TRACE_GAP_HEIGHT,
    pPO->bPrintClr ? pszClr : "0.00",
    pPO->bPrintClr ? "setrgbcolor" : "setgray",
    psz = g_strdup_printf ("max: %9.2e", dTraceMaxVal), FONT_SIZE, "txtlt") ;
  g_free (psz) ;

  PlaceSingleString (pPO, pPages, dcxEffective, dcyEffective, TRACE_GAP_HEIGHT, dyTraceTop + dcyTrace - TRACE_GAP_HEIGHT,
    pPO->bPrintClr ? pszClr : "0.00",
    pPO->bPrintClr ? "setrgbcolor" : "setgray",
    psz = g_strdup_printf ("min: %9.2e", dTraceMinVal), FONT_SIZE, "txtlb") ;
  g_free (psz) ;

  PlaceSingleString (pPO, pPages, dcxEffective, dcyEffective, TRACE_GAP_HEIGHT, dyTraceTop +  dcyTrace / 2,
    pPO->bPrintClr ? pszClr : "0.00",
    pPO->bPrintClr ? "setrgbcolor" : "setgray",
    ptd->data_labels, FONT_SIZE, "txtlm") ;
  }

static void PlaceRuler (print_graph_OP *pPO, PAGES *pPages, simulation_data *sim_data, int idx, int icVisibleTraces, double dcxEffective, double dcyEffective, double dcyRuler)
  {
  double dMaxDigits = 0 ;
  double dxCurrentGrad = 0.0 ;
  double dxSmallGrad = 0.0 ;
  int Nix ;
  int icAvailTics = 0 ;
  int icSamplesPerTic = 0 ;
  int current_sample = 0 ;
  int magnitude = 0 ;
  char *psz = NULL ;
  double 
    dxRulerTop = TRACE_HEADER_WIDTH + GAP_HEIGHT + TRACE_GAP_HEIGHT,
    dyRulerTop = dcyEffective - RULER_HEIGHT,
    dxRulerBot = dcxEffective - TRACE_GAP_HEIGHT,
    dcxRuler = dxRulerBot - dxRulerTop ;

  // Main ruler line
  PlaceSingleLine (pPO, pPages, dxRulerTop, dyRulerTop, dxRulerBot, dyRulerTop, dcxEffective, dcyEffective, PS_BLACK, "setrgbcolor", NULL) ;

  // Right Graduation
  PlaceSingleLine (pPO, pPages, dxRulerBot, dyRulerTop, dxRulerBot, dyRulerTop + RULER_GRAD_HEIGHT, dcxEffective, dcyEffective, PS_BLACK, "setrgbcolor", NULL) ;

  dMaxDigits = ceil (log (sim_data->number_samples) / log (10)) ;

  icAvailTics = 1 + floor (dcxRuler / (dMaxDigits * RULER_FONT_SIZE + 2 * TRACE_GAP_HEIGHT)) ;

  // Samples between the right edge of the "0" tic and the left edge of the "max" tic
  icSamplesPerTic = (int)ceil (((double)(sim_data->number_samples)) / ((double)icAvailTics)) ;

  fprintf (stderr, "Raw samples per tic are %d\n", icSamplesPerTic) ;
  magnitude = pow (10, (int)(log10 (icSamplesPerTic))) ;

  fprintf (stderr, "magnitude = %d\n", magnitude) ;

  icSamplesPerTic = (int)(((int)(((double)icSamplesPerTic)/(magnitude))) * magnitude) ;
  fprintf (stderr, "Rounded samples per tic are %d\n", icSamplesPerTic) ;

  if (magnitude >= icSamplesPerTic)
    icSamplesPerTic = magnitude ;
  else
  if (magnitude * 2 >= icSamplesPerTic)
    icSamplesPerTic = 2 * magnitude ;
  else
  if (magnitude * 5 >= icSamplesPerTic)
    icSamplesPerTic = 5 * magnitude ;
  else
    icSamplesPerTic = 10 * magnitude ;
  fprintf (stderr, "Final samples per tic are %d\n", icSamplesPerTic) ;

  for (current_sample = 0 ; current_sample < sim_data->number_samples ; current_sample += icSamplesPerTic)
    {
    // Graduation
    PlaceSingleLine (pPO, pPages, 
      dxCurrentGrad = 
      dxRulerTop + (current_sample * dcxRuler) / (double)(sim_data->number_samples), dyRulerTop, 
      dxRulerTop + (current_sample * dcxRuler) / (double)(sim_data->number_samples), dyRulerTop + RULER_GRAD_HEIGHT, 
      dcxEffective, dcyEffective, PS_BLACK, "setrgbcolor", NULL) ;

    // Label
    PlaceSingleString (pPO, pPages, dcxEffective, dcyEffective, 
      dxRulerTop + (current_sample * dcxRuler) / (double)(sim_data->number_samples), 
      dyRulerTop + RULER_GRAD_HEIGHT, PS_BLACK, "setrgbcolor", psz = g_strdup_printf ("%d", current_sample), RULER_FONT_SIZE, "txtct") ;
    g_free (psz) ;

    for (Nix = 0 ; Nix < 10 ; Nix++)
      if ((dxSmallGrad = dxCurrentGrad + (icSamplesPerTic * dcxRuler * Nix * 0.1) / (double)(sim_data->number_samples)) < dxRulerBot)
        PlaceSingleLine (pPO, pPages, dxSmallGrad, dyRulerTop, dxSmallGrad, dyRulerTop + RULER_SMALL_GRAD_HEIGHT + ((5 == Nix) ? 1 : 0), 
          dcxEffective, dcyEffective, PS_BLACK, "setrgbcolor", NULL) ;
      else
        break ;
    }
  }

static void PlaceSingleString (print_graph_OP *pPO, PAGES *pPages, double dcxEffective, double dcyEffective, double dx, double dy, char *pszClr, char *pszClrType, char *pszString, int font_size, char *pszStringPlacement)
  {
  char *psz = NULL ;
  int idxXPgMin = -1, idxYPgMin = -1, idxPg = -1 ;

  idxXPgMin = GetPageIndex (dx, pPO->iCXPages, dcxEffective) ;
  idxYPgMin = GetPageIndex (dy, pPO->iCYPages, dcyEffective) ;
  idxPg = pPO->bPrintOrderOver ? idxYPgMin * pPO->iCXPages + idxXPgMin : idxXPgMin * pPO->iCYPages + idxYPgMin ;

  psz = g_strdup_printf (
    "gsave\n"
    "%s %s\n"
    "newpath\n"
    "%lf %lf moveto\n"
    "(" PS_FONT ") findfont %d scalefont setfont\n"
    "(%s) %d %s\n"
    "stroke\n"
    "grestore\n",
    pszClr,
    pszClrType,
    dx - (idxXPgMin * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin,
    pPO->po.dPaperCY -
   (dy - (idxYPgMin * dcyEffective) / pPO->iCYPages + pPO->po.dTMargin), 
    font_size,
    pszString,
    font_size,
    pszStringPlacement) ;

  exp_array_insert_vals (pPages->strings, &psz, 1, 1, -1) ;
  exp_array_insert_vals (pPages->pages, &psz, 1, 1, idxPg, -1) ;
  }

static void PlaceSingleBusTrace (print_graph_OP *pPO, PAGES *pPages, HONEYCOMB_DATA *hc_data, BUS *bus, int idx, int icVisibleTraces, double dcxEffective, double dcyEffective, double dcyTrace, int icSamples, int base)
  {
  HONEYCOMB *hc = NULL ;
  double
    dcxTraceHeader = TRACE_HEADER_WIDTH, // points
    dyTraceTop = idx * (GAP_HEIGHT + dcyTrace) + (FONT_SIZE + GAP_HEIGHT),
    dxMin = dcxTraceHeader + GAP_HEIGHT + TRACE_GAP_HEIGHT,
    dyMin = dyTraceTop + dcyTrace - TRACE_GAP_HEIGHT,
    dxMax = dcxEffective - TRACE_GAP_HEIGHT,
    dyMax = dyTraceTop + TRACE_GAP_HEIGHT,
    dxInc = (dxMax - dxMin) / icSamples,
    dSlopeDistance = 0 ;
  char *pszClr = pPO->bPrintClr ? ((QCAD_CELL_INPUT == bus->bus_function) ? PS_BLUE : PS_YELLOW) : "0.00",
       *pszClrType = pPO->bPrintClr ? "setrgbcolor" : "setgray",
       *psz = NULL ;
  int Nix ;
  int idxCurrentSample = 0 ;

  PlaceSingleTraceBox (pPO, pPages, dyTraceTop, dcxTraceHeader, dcxEffective, dcyEffective, dcyTrace, FALSE) ;

  PlaceSingleString (pPO, pPages, dcxEffective, dcyEffective, TRACE_GAP_HEIGHT, dyTraceTop + TRACE_GAP_HEIGHT,
    pszClr, pszClrType,
    bus->pszName, FONT_SIZE, "txtlt") ;

  if (0 == hc_data->arHCs->icUsed)
    PlaceSingleLine (pPO, pPages,
      dxMin,                           dyTraceTop + dcyTrace / 2,
      dcxEffective - TRACE_GAP_HEIGHT, dyTraceTop + dcyTrace / 2,
      dcxEffective, dcyEffective,
      pszClr, pszClrType,
      NULL) ;
  else
    {
    for (Nix = 0 ; Nix < hc_data->arHCs->icUsed ; Nix++)
      {
      hc = &(exp_array_index_1d (hc_data->arHCs, HONEYCOMB, Nix)) ;
      if (idxCurrentSample < hc->idxBeg)
        PlaceSingleLine (pPO, pPages,
          dxMin + idxCurrentSample * dxInc, dyTraceTop + dcyTrace / 2,
          dxMin + hc->idxBeg * dxInc,       dyTraceTop + dcyTrace / 2,
          dcxEffective, dcyEffective,
          pszClr, pszClrType,
          NULL) ;

      dSlopeDistance = MIN ((((dyMin - dyMax) / 2.0) * cos (HONEYCOMB_ANGLE)), ((dxMin + hc->idxEnd * dxInc) - (dxMin + hc->idxBeg * dxInc)) / 2) ;

      PlaceSingleHoneycomb (pPO, pPages,
        dxMin + hc->idxBeg * dxInc,
        dxMin + hc->idxBeg * dxInc + dSlopeDistance,
        dxMin + hc->idxEnd * dxInc - dSlopeDistance,
        dxMin + hc->idxEnd * dxInc,
        dyMin, dyMax,
        psz = strdup_convert_to_base (hc->value, base),
        dcxEffective, dcyEffective, pszClr, pszClrType) ;
      g_free (psz) ;

      idxCurrentSample = hc->idxEnd ;
      }

    if (idxCurrentSample < icSamples - 1)
      PlaceSingleLine (pPO, pPages,
        dxMin + idxCurrentSample * dxInc, dyTraceTop + dcyTrace / 2,
        dxMin + (icSamples - 1) * dxInc,  dyTraceTop + dcyTrace / 2,
        dcxEffective, dcyEffective,
        pszClr,
        pszClrType,
        NULL) ;
    }
  }

static void PlaceSingleTraceBox (print_graph_OP *pPO, PAGES *pPages, double dyTraceTop, double dcxTraceHeader, double dcxEffective, double dcyEffective, double dcyTrace, gboolean bDrawMidLine)
  {
  double dxMin, dyMin, dxMax, dyMax ;
  char *pszClr = pPO->bPrintClr ? PS_GREEN : "0.00",
       *pszClrType = pPO->bPrintClr ? "setrgbcolor" : "setgray" ;

  // Trace header box
  dxMin = 0 ;
  dyMin = dyTraceTop ;
  dxMax = dcxTraceHeader ;
  dyMax = dyTraceTop + dcyTrace ;

  PlaceSingleBox (pPO, pPages, dxMin, dyMin, dxMax, dyMax, dcxEffective, dcyEffective) ;

  // Trace body box
  dxMin = dcxTraceHeader + GAP_HEIGHT ;
  dxMax = dcxEffective ;
  // dyMin = same as above
  // dyMax = same as above

  PlaceSingleBox (pPO, pPages, dxMin, dyMin, dxMax, dyMax, dcxEffective, dcyEffective) ;

  // Place trace max line onto every page
  PlaceSingleLine (pPO, pPages,
    dxMin + TRACE_GAP_HEIGHT,        dyTraceTop + TRACE_GAP_HEIGHT,
    dcxEffective - TRACE_GAP_HEIGHT, dyTraceTop + TRACE_GAP_HEIGHT,
    dcxEffective, dcyEffective,
      pszClr, pszClrType,
      "[4 4] 1 setdash") ;

  // Place trace min line onto every page
  PlaceSingleLine (pPO, pPages,
    dxMin + TRACE_GAP_HEIGHT,        dyTraceTop + dcyTrace - TRACE_GAP_HEIGHT,
    dcxEffective - TRACE_GAP_HEIGHT, dyTraceTop + dcyTrace - TRACE_GAP_HEIGHT,
    dcxEffective, dcyEffective,
      pszClr, pszClrType,
      "[4 4] 1 setdash") ;

  if (bDrawMidLine)
    // Place trace middle line onto every page
    PlaceSingleLine (pPO, pPages,
      dxMin + TRACE_GAP_HEIGHT,        dyTraceTop + dcyTrace / 2,
      dcxEffective - TRACE_GAP_HEIGHT, dyTraceTop + dcyTrace / 2,
      dcxEffective, dcyEffective,
      pszClr, pszClrType,
      "[4 4] 1 setdash") ;
  }

static void PlaceSingleBox (print_graph_OP *pPO, PAGES *pPages, double dxMin, double dyMin, double dxMax, double dyMax, double dcxEffective, double dcyEffective)
  {
  char *psz = NULL ;
  int Nix, Nix1 ;
  int
    idxXPgMin = -1, idxYPgMin = -1,
    idxXPgMax = -1, idxYPgMax = -1,
    idxPg = -1 ;
  double 
    dxMinPg = -1.0, dyMinPg = -1.0, 
    dxMaxPg = -1.0, dyMaxPg = -1.0 ;

  idxXPgMin = GetPageIndex (dxMin, pPO->iCXPages, dcxEffective) ;
  idxYPgMin = GetPageIndex (dyMin, pPO->iCYPages, dcyEffective) ;
  idxXPgMax = GetPageIndex (dxMax, pPO->iCXPages, dcxEffective) ;
  idxYPgMax = GetPageIndex (dyMax, pPO->iCYPages, dcyEffective) ;

  for (Nix = idxYPgMin ; Nix <= idxYPgMax ; Nix++)
    for (Nix1 = idxXPgMin ; Nix1 <= idxXPgMax ; Nix1++)
      {
      idxPg = pPO->bPrintOrderOver ? Nix * pPO->iCXPages + Nix1 : Nix1 * pPO->iCYPages + Nix ;

      dxMinPg = dxMin - (Nix1 * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin ;
      dyMinPg = pPO->po.dPaperCY -
               (dyMin - (Nix  * dcyEffective) / pPO->iCYPages + pPO->po.dTMargin)  ;
      dxMaxPg = dxMax - (Nix1 * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin ;
      dyMaxPg = pPO->po.dPaperCY -
               (dyMax - (Nix  * dcyEffective) / pPO->iCYPages + pPO->po.dTMargin)  ;

      psz = g_strdup_printf ("%lf %lf %lf %lf tracebox %%from PlaceSingleBox\n", dxMinPg, dyMinPg, dxMaxPg, dyMaxPg) ;

      exp_array_insert_vals (pPages->strings, &psz, 1, 1, -1) ;
      exp_array_insert_vals (pPages->pages, &psz, 1, 1, idxPg, -1) ;
      }
  }

static void PlaceSingleHoneycomb (print_graph_OP *pPO, PAGES *pPages, double dxMin, double dxL, double dxR, double dxMax, double dyMin, double dyMax, char *pszLabel, double dcxEffective, double dcyEffective, char *pszClr, char *pszClrType)
  {
  int Nix ;
  EXP_ARRAY *page_indices = NULL ;
  int idxPg = -1 ;
  PAGE_IDX_2D *existing_idx = NULL ;
  double dxMinPg, dxLPg, dxRPg, dxMaxPg, dyMinPg, dyMaxPg ;
  DPOINT pts[6] =
   {{dxMin, (dyMax + dyMin) / 2},
    {dxL, dyMin},
    {dxR, dyMin},
    {dxMax, (dyMax + dyMin) / 2},
    {dxR, dyMax},
    {dxL, dyMax}} ;
  char *psz = NULL ;

  if (NULL == pPO || NULL == pPages) return ;

  page_indices = get_polyline_page_indices (pPO, pts, 6, TRUE, dcxEffective, dcyEffective) ;

  for (Nix = 0 ; Nix < page_indices->icUsed ; Nix++)
    {
    existing_idx = &(exp_array_index_1d (page_indices, PAGE_IDX_2D, Nix)) ;

    idxPg = pPO->bPrintOrderOver
      ? existing_idx->idxY * pPO->iCXPages + existing_idx->idxX
      : existing_idx->idxX * pPO->iCYPages + existing_idx->idxY ;

    dxMinPg = dxMin - (existing_idx->idxX * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin ;
    dxLPg   = dxL   - (existing_idx->idxX * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin ;
    dxRPg   = dxR   - (existing_idx->idxX * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin ;
    dxMaxPg = dxMax - (existing_idx->idxX * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin ;
    dyMinPg = pPO->po.dPaperCY -
             (dyMin - (existing_idx->idxY * dcyEffective) / pPO->iCYPages + pPO->po.dTMargin) ;
    dyMaxPg = pPO->po.dPaperCY -
             (dyMax - (existing_idx->idxY * dcyEffective) / pPO->iCYPages + pPO->po.dTMargin) ;

    psz = g_strdup_printf (
      "gsave\n"
      "%s %s\n"
      "%lf %lf %lf %lf %lf %lf (%s) honeycomb\n"
      "grestore\n",
      pszClr, pszClrType, dxMinPg, dxLPg, dxRPg, dxMaxPg, dyMinPg, dyMaxPg, pszLabel) ;

    exp_array_insert_vals (pPages->strings, &psz, 1, 1, -1) ;
    exp_array_insert_vals (pPages->pages, &psz, 1, 1, idxPg, -1) ;
    }
  exp_array_free (page_indices) ;
  }

static EXP_ARRAY *get_polyline_page_indices (print_graph_OP *pPO, DPOINT *pts, int icPts, gboolean bClose, double dcxEffective, double dcyEffective)
  {
  int Nix, Nix1, Nix2, Nix3 ;
  PAGE_IDX_2D *existing_idx = NULL ;
  DPOINT ptCur = {0} ;
  EXP_ARRAY *page_indices = exp_array_new (sizeof (PAGE_IDX_2D), 1) ;
  gboolean bCircularIter = FALSE, bNeedInsert = TRUE ;
  int idxXPgBeg = -1, idxYPgBeg = -1, idxXPgEnd = -1, idxYPgEnd = -1,
      idxXPgMin = -1, idxYPgMin = -1, idxXPgMax = -1, idxYPgMax = -1 ;

  ptCur.x = pts[0].x ;
  ptCur.y = pts[0].y ;
  idxXPgBeg = GetPageIndex (ptCur.x, pPO->iCXPages, dcxEffective) ;
  idxYPgBeg = GetPageIndex (ptCur.y, pPO->iCYPages, dcyEffective) ;

  // Find out each page any of the line segments appear on
  for (Nix = 1 ; Nix < icPts ; Nix++)
    {
    if (!bCircularIter)
      {
      idxXPgEnd = GetPageIndex (pts[Nix].x, pPO->iCXPages, dcxEffective) ;
      idxYPgEnd = GetPageIndex (pts[Nix].y, pPO->iCYPages, dcyEffective) ;
      }
    else
      {
      idxXPgEnd = GetPageIndex (pts[0].x, pPO->iCXPages, dcxEffective) ;
      idxYPgEnd = GetPageIndex (pts[0].y, pPO->iCYPages, dcyEffective) ;
      }

    idxXPgMin = MIN (idxXPgBeg, idxXPgEnd) ;
    idxYPgMin = MIN (idxYPgBeg, idxYPgEnd) ;
    idxXPgMax = MAX (idxXPgBeg, idxXPgEnd) ;
    idxYPgMax = MAX (idxYPgBeg, idxYPgEnd) ;

    for (Nix1 = idxXPgMin ; Nix1 <= idxXPgMax ; Nix1++)
      for (Nix2 = idxXPgMin ; Nix2 <= idxXPgMax ; Nix2++)
        {
        bNeedInsert = TRUE ;
        for (Nix3 = 0 ; Nix3 < page_indices->icUsed ; Nix3++)
          {
          existing_idx = &(exp_array_index_1d (page_indices, PAGE_IDX_2D, Nix3)) ;
          if (existing_idx->idxX > Nix1)
            break ;
          else
          if (existing_idx->idxX == Nix1 && existing_idx->idxY > Nix2)
            break ;
          else
          if (existing_idx->idxX == Nix1 && existing_idx->idxY == Nix2)
            {
            bNeedInsert = FALSE ;
            break ;
            }
          }
        if (bNeedInsert)
          {
          exp_array_insert_vals (page_indices, NULL, 1, 1, Nix3) ;
          existing_idx = &(exp_array_index_1d (page_indices, PAGE_IDX_2D, Nix3)) ;
          existing_idx->idxX = Nix1 ;
          existing_idx->idxY = Nix2 ;
          }
        }

    if (bCircularIter) break ;

    ptCur.x = pts[Nix].x ;
    ptCur.y = pts[Nix].y ;
    idxXPgBeg = idxXPgEnd ;
    idxYPgBeg = idxYPgEnd ;
    if (bClose && (bCircularIter = (icPts - 1 == Nix))) Nix-- ;
    }

  return page_indices ;
  }

static void PlaceSingleLine (print_graph_OP *pPO, PAGES *pPages, double dxMin, double dyMin, double dxMax, double dyMax, double dcxEffective, double dcyEffective, char *pszClr, char *pszClrType, char *pszDash)
  {
  int Nix, Nix1 ;
  int idxXPgMin = -1, idxYPgMin = -1, idxXPgMax = -1, idxYPgMax = -1 ;
  int idxPg ;
  double dxMinPg, dyMinPg, dxMaxPg, dyMaxPg ;
  char *psz = NULL ;

  idxXPgMin = GetPageIndex (dxMin, pPO->iCXPages, dcxEffective) ;
  idxYPgMin = GetPageIndex (dyMin, pPO->iCYPages, dcyEffective) ;
  idxXPgMax = GetPageIndex (dxMax, pPO->iCXPages, dcxEffective) ;
  idxYPgMax = GetPageIndex (dyMax, pPO->iCYPages, dcyEffective) ;

  // Place trace mid line onto every page
  for (Nix = idxYPgMin ; Nix <= idxYPgMax ; Nix++)
    for (Nix1 = idxXPgMin ; Nix1 <= idxXPgMax ; Nix1++)
      {
      idxPg = pPO->bPrintOrderOver ? Nix * pPO->iCXPages + Nix1 : Nix1 * pPO->iCYPages + Nix ;

      dxMinPg = dxMin - (Nix1 * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin ;
      dyMinPg = pPO->po.dPaperCY -
               (dyMin - (Nix  * dcyEffective) / pPO->iCYPages + pPO->po.dTMargin)  ;
      dxMaxPg = dxMax - (Nix1 * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin ;
      dyMaxPg = pPO->po.dPaperCY -
               (dyMax - (Nix  * dcyEffective) / pPO->iCYPages + pPO->po.dTMargin)  ;

      psz = g_strdup_printf (
        "gsave\n"
        "newpath\n"
        "%s %s\n"
        "%s%s"
        "%lf %lf moveto\n"
        "%lf %lf lineto\n"
        "closepath\n"
        "stroke\n"
        "grestore\n",
        pszClr,
        pszClrType,
        NULL == pszDash ? "" : pszDash, NULL == pszDash ? "" : "\n",
        dxMinPg, dyMinPg, dxMaxPg, dyMaxPg) ;

      exp_array_insert_vals (pPages->strings, &psz, 1, 1, -1) ;
      exp_array_insert_vals (pPages->pages, &psz, 1, 1, idxPg, -1) ;
      }
  }

static int GetPageIndex (double dCoord, int icPages, double dEffectiveLength)
  {
  double dIdx = dCoord * icPages / dEffectiveLength ;
  if (fabs (((double)(int)dIdx) - dIdx) < 0.00001) dIdx-- ;
  return CLAMP ((int)dIdx, 0, icPages - 1) ;
  }

static void PlaceSingleWaveform (print_graph_OP *pPO, PAGES *pPages, struct TRACEDATA *ptd, double dxMin, double dyMin, double dxMax, double dyMax, int icSamples, double dTraceMin, double dTraceMax)
  {
//  The box the graph must fit into is given in effective
//  coordinates as follows:
//
//                                           (dxMax, dyMin)
//    +-------------------------------------------+
//    |        ___                       __       |
//    |       /   \__                   /  \      |
//    |  ____/       \                 /    \_....|
//    | /             \      _________/           |
//    |/               \____/                     |
//    +-------------------------------------------+
//  (dxMin, dyMax)
  double
    dcx = pPO->iCXPages * (pPO->po.dPaperCX - pPO->po.dLMargin - pPO->po.dRMargin),
    dcy = pPO->iCYPages * (pPO->po.dPaperCY - pPO->po.dTMargin - pPO->po.dBMargin) ;
  int
    xIdxMin = GetPageIndex (dxMin, pPO->iCXPages, dcx),
    yIdxMin = GetPageIndex (dyMin, pPO->iCYPages, dcy),
    yIdxMax = GetPageIndex (dyMax, pPO->iCYPages, dcy),
    Nix, Nix1, xIdx = xIdxMin, xIdxNew, idxPg ;
  double
    dYScale, dxInc, dx0, dy0, dx1, dy1, dx2, dy2 ; /* The 2 line segments (dx,dy)0 -> (dx,dy)1 -> (dx,dy)2 */
  gchar *psz = NULL, *pszNew = NULL, *pszClr = NULL ;

  pszClr =
    QCAD_CELL_FIXED  == ptd->trace_function ? PS_RED    :
    QCAD_CELL_OUTPUT == ptd->trace_function ? PS_YELLOW :
    QCAD_CELL_INPUT  == ptd->trace_function ? PS_BLUE   : PS_BLACK ;

  if (dTraceMax == dTraceMin) return ;
  if (icSamples < 2) return ;

  dxInc = (dxMax - dxMin) / icSamples ;

  DBG_PG (fprintf (stderr, "dxMax = %lf, dxMin = %lf, dxInc = %lf\n", dxMax, dxMin, dxInc)) ;

  dYScale = (dyMin - dyMax) / (dTraceMax - dTraceMin) ;

  DBG_PG (fprintf (stderr, "dyMax = %lf, dyMin = %lf, dTraceMax = %lf, dTraceMin = %lf, dYScale = %lf\n",
    dyMax, dyMin, dTraceMax, dTraceMin, dYScale)) ;

  for (Nix = yIdxMin ; Nix <= yIdxMax ; Nix++)
    {
    dx0 = dx1 = dx2 = dxMin ;
    dy0 = dy1 = dy2 = dyMin - (ptd->data[0] - dTraceMin) * dYScale ;
    psz = g_strdup_printf (
      "gsave\n"
      "%s %s\n"
      "newpath\n"
      "%lf %lf moveto point\n",
      pPO->bPrintClr ? pszClr : "0.00",
      pPO->bPrintClr ? "setrgbcolor" : "setgray",
      dx0 - (xIdx * dcx) / pPO->iCXPages + pPO->po.dLMargin,
      pPO->po.dPaperCY -
     (dy0 - (Nix  * dcy) / pPO->iCYPages + pPO->po.dTMargin)) ;
    for (Nix1 = 1 ; Nix1 < icSamples ; Nix1++)
      {
      dx2 = dx2 + dxInc ;
      dy2 = dyMin - (ptd->data[Nix1] - dTraceMin) * dYScale ;

      // If we cross over onto a new page, finish off one segment, change pages, and start another
      if (xIdx != (xIdxNew = GetPageIndex (dx2, pPO->iCXPages, dcx)))
        {
        pszNew = g_strdup_printf (
          "%s"
          "%lf %lf lineto point\n"
          "%lf %lf lineto point\n"
          "stroke\n"
          "grestore\n",
          psz,
          dx1 - (xIdx * dcx) / pPO->iCXPages + pPO->po.dLMargin,
          pPO->po.dPaperCY -
         (dy1 - (Nix  * dcy) / pPO->iCYPages + pPO->po.dTMargin),
          dx2 - (xIdx * dcx) / pPO->iCXPages + pPO->po.dLMargin,
          pPO->po.dPaperCY -
         (dy2 - (Nix  * dcy) / pPO->iCYPages + pPO->po.dTMargin)) ;
        g_free (psz) ;
        pszNew = psz ;

        idxPg = pPO->bPrintOrderOver ? Nix * pPO->iCXPages + xIdx : xIdx * pPO->iCYPages + Nix ;

        exp_array_insert_vals (pPages->strings, &psz, 1, 1, -1) ;
        exp_array_insert_vals (pPages->pages, &psz, 1, 1, idxPg, -1) ;

        xIdx = xIdxNew ;

        psz = g_strdup_printf (
          "gsave\n"
          "%s %s\n"
          "newpath\n"
          "%lf %lf moveto point\n"
          "%lf %lf lineto point\n",
          pPO->bPrintClr ? pszClr : "0.00",
          pPO->bPrintClr ? "setrgbcolor" : "setgray",
          dx1 - (xIdx * dcx) / pPO->iCXPages + pPO->po.dLMargin,
          pPO->po.dPaperCY -
         (dy1 - (Nix  * dcy) / pPO->iCYPages + pPO->po.dTMargin),
          dx2 - (xIdx * dcx) / pPO->iCXPages + pPO->po.dLMargin,
          pPO->po.dPaperCY -
         (dy2 - (Nix  * dcy) / pPO->iCYPages + pPO->po.dTMargin)) ;
        dx0 = dx1 = dx2 ;
        dy0 = dy1 = dy2 ;
        }

      if (LineSegmentCanBeSkipped (dx0, dy0, dx1, dy1, dx2, dy2, MAX_SLOPE_DIFF))
        {
        dx1 = dx2 ;
        dy1 = dy2 ;
        continue ;
        }
      pszNew = g_strdup_printf (
        "%s"
        "%lf %lf lineto point\n",
        psz,
      dx1 - (xIdx * dcx) / pPO->iCXPages + pPO->po.dLMargin,
      pPO->po.dPaperCY -
     (dy1 - (Nix  * dcy) / pPO->iCYPages + pPO->po.dTMargin)) ;

      g_free (psz) ;
      psz = pszNew ;

      dx0 = dx1 ;
      dy0 = dy1 ;
      dx1 = dx2 ;
      dy1 = dy2 ;
      }
    pszNew = g_strdup_printf (
      "%s"
      "%lf %lf lineto point\n"
      "%lf %lf lineto point\n"
      "stroke\n"
      "grestore\n",
      psz,
      dx1 - (xIdx * dcx) / pPO->iCXPages + pPO->po.dLMargin,
      pPO->po.dPaperCY -
     (dy1 - (Nix  * dcy) / pPO->iCYPages + pPO->po.dTMargin),
      dx2 - (xIdx * dcx) / pPO->iCXPages + pPO->po.dLMargin,
      pPO->po.dPaperCY -
     (dy2 - (Nix  * dcy) / pPO->iCYPages + pPO->po.dTMargin)) ;

    g_free (psz) ;
    psz = pszNew ;

    idxPg = pPO->bPrintOrderOver ? Nix * pPO->iCXPages + xIdx : xIdx * pPO->iCYPages + Nix ;

    exp_array_insert_vals (pPages->strings, &psz, 1, 1, -1) ;
    exp_array_insert_vals (pPages->pages, &psz, 1, 1, idxPg, -1) ;
    }
  }
