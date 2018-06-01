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
// This file implements a PostScript printer for        //
// QCADesigner designs.                                 //
// Completion Date: June 2003                           //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "print.h"
#include "design.h"
#include "print_util.h"
#include "objects/QCADLayer.h"

#define DBG_P(s)

static GList **PlaceObjectsOnPages (DESIGN *design, print_design_OP *pPO, double cxPageNm, double cyPageNm, double dxMinNm, double dyMinNm) ;

void PrintPages (FILE *pfile, print_design_OP *pPO, GList **lstPages, double dEffPageCYPts, double dxMinNm, double dyMinNm, double dxDiffMinNm, double dyDiffMinNm) ;

static void PrintObjectPreamble (FILE *pfile, GList **lstPages, int icPages) ;

void print_world (print_design_OP *pPO, DESIGN *design)
  {
  FILE *pfile = NULL ;
  int Nix ;
  GList
    *lstItr = NULL,
    **lstPages = NULL ; // 2D array of GList * (cxPages wide and cyPages long)
  gboolean bPrintThisLayer = FALSE, bFirstLayer = TRUE ;
  double dxMinNm = 0, dyMinNm = 0, dxMaxNm = 0, dyMaxNm = 0, cxNm = 0, cyNm = 0,
    dxDiffMinNm = 0, dyDiffMinNm = 0,
    dEffPageCXPts = pPO->po.dPaperCX - pPO->po.dLMargin - pPO->po.dRMargin,
    dEffPageCYPts = pPO->po.dPaperCY - pPO->po.dTMargin - pPO->po.dBMargin ;
  WorldRectangle extentsNm = {0.0} ;

  DBG_P (fprintf (stderr, "print_world:Entering\n")) ;

  // Get the size of the world in nanos
  for (Nix = 0, lstItr = design->lstLayers ; lstItr != NULL ; Nix++, lstItr = lstItr->next)
    {
    if (Nix < pPO->icPrintedObjs)
      bPrintThisLayer = pPO->pbPrintedObjs[Nix] ;
    else
      bPrintThisLayer = TRUE ;

    if (bPrintThisLayer)
      {
      qcad_layer_get_extents ((QCAD_LAYER (lstItr->data)), &extentsNm, FALSE) ;
      if (bFirstLayer)
        {
        dxMinNm = extentsNm.xWorld ;
        dyMinNm = extentsNm.yWorld ;
        dxMaxNm = extentsNm.cxWorld + extentsNm.xWorld ;
        dyMaxNm = extentsNm.cyWorld + extentsNm.yWorld ;
        bFirstLayer = FALSE ;
        }
      else
        {
        dxMinNm = MIN (extentsNm.xWorld, dxMinNm) ;
        dyMinNm = MIN (extentsNm.yWorld, dyMinNm) ;
        dxMaxNm = MAX (extentsNm.xWorld + extentsNm.cxWorld, dxMaxNm) ;
        dyMaxNm = MAX (extentsNm.yWorld + extentsNm.cyWorld, dyMaxNm) ;
        }
      DBG_P (fprintf (stderr, "Layer \"%s\" extents:(%lf,%lf)<->(%lf,%lf)=<%lfx%lf>\n", ((LAYER *)(lstItr->data))->pszDescription,
        dxTopNm, dyTopNm, dxBotNm, dyBotNm, dxBotNm - dxTopNm, dyBotNm - dyTopNm)) ;
      }
    }

  cxNm = dxMaxNm - dxMinNm ;
  cyNm = dyMaxNm - dyMinNm ;

  DBG_P (fprintf (stderr, "Design extents: (%lf,%lf)<%lfx%lf>\n", dxMinNm, dyMinNm, cxNm, cyNm)) ;

  // If we are to fit to n x m pages instead of using a fixed scale, let's re-calculate the scale here,
  // since the GtkAdjustment approximates the scale to as many decimals as specified
  if (pPO->bFit)
    pPO->dPointsPerNano = MIN (dEffPageCXPts * pPO->iCXPages / cxNm,
                               dEffPageCYPts * pPO->iCYPages / cyNm) ;

  if (pPO->bCenter)
    {
    double dWidthPoints = cxNm * pPO->dPointsPerNano,
           dHeightPoints = cyNm * pPO->dPointsPerNano ;
    dxDiffMinNm = (((dEffPageCXPts * pPO->iCXPages) - dWidthPoints) / 2) / pPO->dPointsPerNano ;
    dyDiffMinNm = (((dEffPageCYPts * pPO->iCYPages) - dHeightPoints) / 2) / pPO->dPointsPerNano ;
    }

  lstPages = PlaceObjectsOnPages (design, pPO,
    dEffPageCXPts / pPO->dPointsPerNano, dEffPageCYPts / pPO->dPointsPerNano,
    dxMinNm - dxDiffMinNm, dyMinNm - dyDiffMinNm) ;

  if (NULL == (pfile = OpenPrintStream ((print_OP *)pPO)))
    {
    fprintf (stderr, "Failed to open print stream \"%s\"\n", pPO->po.pszPrintString) ;
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
    "%%%%EndComments\n"
    "%%%%BeginProlog\n"
    "/epsilon 0.1 def\n"
    "/nm { %f mul } def\n"
    "/labelfontsize 12 nm def\n"
    "\n"
    "%% a b min\n"
    "/min\n"
    "  {\n"
    "  /b exch def\n"
    "  /a exch def\n"
    "\n"
    "  a b lt { a } { b } ifelse\n"
    "  } def\n"
    "\n"
    "%% a b max\n"
    "/max\n"
    "  {\n"
    "  /b exch def\n"
    "  /a exch def\n"
    "\n"
    "  a b gt { a } { b } ifelse\n"
    "  } def\n"
    "\n"
    "/linewidth 0.1 nm 1 min def\n"
    "\n"
    "%% (string) txt??\n"
    "/txtlt { gsave dup 0 -1 labelfontsize mul rmoveto show grestore } def\n"
    "/txtlm { gsave dup 0 labelfontsize 2 div -1 mul rmoveto show grestore } def\n"
    "/txtlb { gsave dup 0 0 rmoveto show grestore } def\n"
    "/txtct { gsave dup stringwidth exch 2 div -1 mul exch pop labelfontsize -1 mul rmoveto show grestore } def\n"
    "/txtcm { gsave dup stringwidth exch 2 div -1 mul exch pop labelfontsize 2 div -1 mul rmoveto show grestore } def\n"
    "/txtcb { gsave dup stringwidth pop 2 div -1 mul 0 rmoveto show grestore } def\n"
    "/txtrt { gsave dup stringwidth exch -1 mul exch pop labelfontsize -1 mul rmoveto show grestore } def\n"
    "/txtrm { gsave dup stringwidth exch -1 mul exch pop labelfontsize 2 div -1 mul rmoveto show grestore } def\n"
    "/txtrb { gsave dup stringwidth exch -1 mul exch pop 0 rmoveto show grestore } def\n"
    "1 setlinejoin\n"
    "1 setlinecap\n"
    "linewidth setlinewidth\n",
    pPO->po.bPortrait ? "Portrait" : "Landscape",
    (int)(pPO->po.dPaperCX), (int)(pPO->po.dPaperCY),
    0.0, 0.0, pPO->po.dPaperCX, pPO->po.dPaperCY, pPO->dPointsPerNano) ;

  PrintObjectPreamble (pfile, lstPages, pPO->iCXPages * pPO->iCYPages) ;

  fprintf (pfile, "%%%%EndProlog\n") ;

  PrintPages (pfile, pPO, lstPages, dEffPageCYPts, dxMinNm, dyMinNm, dxDiffMinNm, dyDiffMinNm) ;

  fprintf (pfile,
    "%%%%Trailer\n"
    "%%%%Pages: %d\n"
    "%%%%EOF\n",
    pPO->iCXPages * pPO->iCYPages) ;

  ClosePrintStream (pfile, (print_OP *)pPO) ;
  }

static GList **PlaceObjectsOnPages (DESIGN *design, print_design_OP *pPO, double cxPageNm, double cyPageNm, double dxMinNm, double dyMinNm)
  {
  gboolean bPrintThisLayer = FALSE ;
  GList
    **lstRet = g_malloc0 (pPO->iCXPages * pPO->iCYPages * sizeof (GList *)),
    *lstItr = NULL,
    *lstItrObjs = NULL ;
  int Nix, idxX1 = -1, idxX2 = -1, idxY1 = -1, idxY2 = -1, Nix1, Nix2, idx = -1 ;
  WorldRectangle rcWorld ;

  DBG_P (fprintf (stderr, "PlaceObjectsOnPage:Entering\n")) ;

  for (Nix = 0, lstItr = design->lstLayers ; lstItr != NULL ; Nix++, lstItr = lstItr->next)
    {
    DBG_P (fprintf (stderr, "PlaceObjectsOnPage:Checking whether to examine layer \"%s\"...", ((LAYER *)(lstItr->data))->pszDescription)) ;
    if (Nix < pPO->icPrintedObjs)
      bPrintThisLayer = pPO->pbPrintedObjs[Nix] ;
    else
      bPrintThisLayer = TRUE ;

    DBG_P (fprintf (stderr, "%s\n", bPrintThisLayer ? "yes" : "no")) ;

    if (bPrintThisLayer)
      {
      for (lstItrObjs = (QCAD_LAYER (lstItr->data))->lstObjs ; lstItrObjs != NULL ; lstItrObjs = lstItrObjs->next)
        if (NULL != lstItrObjs->data)
          {
          qcad_design_object_get_bounds_box (QCAD_DESIGN_OBJECT (lstItrObjs->data), &rcWorld) ;

          DBG_P (fprintf (stderr, "Found bounds box:(%lf,%lf)<%lfx%lf>\n", x, y, cx, cy)) ;

          idxX1 = (int) ((rcWorld.xWorld - dxMinNm) / cxPageNm) ;
          idxX1 = idxX1 < 0 ? 0 : idxX1 >= pPO->iCXPages ? pPO->iCXPages - 1 : idxX1 ; // Paranoia check
          idxY1 = (int) ((rcWorld.yWorld - dyMinNm) / cyPageNm) ;
          idxY1 = idxY1 < 0 ? 0 : idxY1 >= pPO->iCYPages ? pPO->iCYPages - 1 : idxY1 ; // Paranoia check
          idxX2 = (int) ((rcWorld.xWorld + rcWorld.cxWorld - dxMinNm) / cxPageNm) ;
          idxX2 = idxX2 < 0 ? 0 : idxX2 >= pPO->iCXPages ? pPO->iCXPages - 1 : idxX2 ; // Paranoia check
          idxY2 = (int) ((rcWorld.yWorld + rcWorld.cyWorld - dyMinNm) / cyPageNm) ;
          idxY2 = idxY2 < 0 ? 0 : idxY2 >= pPO->iCYPages ? pPO->iCYPages - 1 : idxY2 ; // Paranoia check

          for (Nix1 = idxX1 ; Nix1 <= idxX2 ; Nix1++)
            for (Nix2 = idxY1 ; Nix2 <= idxY2 ; Nix2++)
              {
              DBG_P (fprintf (stderr, "Adding an object of type %s to page (%d,%d)\n",
                g_type_name (G_TYPE_FROM_INSTANCE (lstItrObjs->data)), Nix1, Nix2)) ;
              idx = pPO->bPrintOrderOver ? Nix2 * pPO->iCXPages + Nix1 : Nix1 * pPO->iCYPages + Nix2 ;
              lstRet[idx] = g_list_prepend (lstRet[idx], lstItrObjs->data) ;
              }
          }
      }
    }

  return lstRet ;
  }

static void PrintObjectPreamble (FILE *pfile, GList **lstPages, int icPages)
  {
  int Nix ;
  GList *lstObjs = NULL, *lstItr = NULL ;

  // For each type of object appearing on the pages, add a reference to the linked list lstObjs
  for (Nix = 0 ; Nix < icPages ; Nix++)
    for (lstItr = lstPages[Nix] ; lstItr != NULL ; lstItr = lstItr->next)
      lstObjs = qcad_design_object_add_types (QCAD_DESIGN_OBJECT (lstItr->data), lstObjs) ;

  // For each link in lstObjs, print out the preamble
  for (lstItr = lstObjs ; lstItr != NULL ; lstItr = lstItr->next)
    fprintf (pfile, "\n%s", qcad_design_object_get_PostScript_preamble (QCAD_DESIGN_OBJECT (lstItr->data))) ;
  fprintf (pfile, "\n") ;
  }

void PrintPages (FILE *pfile, print_design_OP *pPO, GList **lstPages, double dEffPageCYPts, double dxMinNm, double dyMinNm, double dxDiffMinNm, double dyDiffMinNm)
  {
  int icPages = pPO->iCXPages * pPO->iCYPages, Nix ;
  GList *lstItr = NULL ;
  char *pszPSInstance = NULL ;
  double cxPageOffsetNm = 0, cyPageOffsetNm = 0 ;

  for (Nix = 0 ; Nix < icPages ; Nix++)
    {
    DBG_P (fprintf (stderr, "PrintPages:printing page %d: cxPageOffsetNm = %lf cyPageOffsetNm = %lf\n",
      Nix, cxPageOffsetNm, cyPageOffsetNm)) ;
//
//  +->-+ Margins are drawn like this (because the PostScript origin is bottom left)
//  |   |
//  ^   v
//  |   |
//  +-<-+
//  ^
//  |
//
    fprintf (pfile,
      "%%%%Page: %d %d\n"
      "%%%%BeginPageSetup\n"
      "/nmx { %f sub %f sub %f add %f mul %f add } def\n"
      "/nmy { %f exch %f sub %f sub %f add %f mul %f add sub } def\n"
      "%%%%EndPageSetup\n"
      "gsave\n"
      "%%The margin\n"
      "newpath\n"
      "%lf %lf moveto\n"
      "%lf %lf lineto\n"
      "%lf %lf lineto\n"
      "%lf %lf lineto\n"
      "closepath eoclip\n",
      Nix + 1, Nix + 1, // %%Page:
      dxMinNm, cxPageOffsetNm, dxDiffMinNm, pPO->dPointsPerNano, pPO->po.dLMargin, // nmx
      pPO->po.dPaperCY, dyMinNm, cyPageOffsetNm, dyDiffMinNm, pPO->dPointsPerNano, pPO->po.dTMargin, // nmy
      pPO->po.dLMargin, pPO->po.dBMargin, // moveto
      pPO->po.dLMargin, pPO->po.dPaperCY - pPO->po.dTMargin, // lineto
      pPO->po.dPaperCX - pPO->po.dRMargin, pPO->po.dPaperCY - pPO->po.dTMargin, // lineto
      pPO->po.dPaperCX - pPO->po.dRMargin,
      pPO->po.dBMargin // lineto
      ) ;

    if (pPO->bPrintOrderOver)
      cxPageOffsetNm += (pPO->po.dPaperCX - pPO->po.dLMargin - pPO->po.dRMargin) / pPO->dPointsPerNano ;
    else
      cyPageOffsetNm += (pPO->po.dPaperCY - pPO->po.dTMargin - pPO->po.dBMargin) / pPO->dPointsPerNano ;

    if (pPO->bPrintOrderOver && !((Nix + 1) % pPO->iCXPages))
      {
      DBG_P (fprintf (stderr,  "PrintPages:bPrintOrderOver:Increasing cy offset\n")) ;
      cyPageOffsetNm += (pPO->po.dPaperCY - pPO->po.dTMargin - pPO->po.dBMargin) / pPO->dPointsPerNano ;
      cxPageOffsetNm = 0 ;
      }
    else
    if (!pPO->bPrintOrderOver && !((Nix + 1) % pPO->iCYPages))
      {
      DBG_P (fprintf (stderr,  "PrintPages:!bPrintOrderOver:Increasing cx offset\n")) ;
      cxPageOffsetNm += (pPO->po.dPaperCX - pPO->po.dLMargin - pPO->po.dRMargin) / pPO->dPointsPerNano ;
      cyPageOffsetNm = 0 ;
      }

    for (lstItr = lstPages[Nix] ; lstItr != NULL ; lstItr = lstItr->next)
      {
      pszPSInstance = qcad_design_object_get_PostScript_instance (QCAD_DESIGN_OBJECT (lstItr->data), pPO->bColour) ;
      fprintf (pfile, "%s\n", pszPSInstance) ;
      g_free (pszPSInstance) ;
      }

    fprintf (pfile,
      "grestore\n"
      "showpage\n"
      "%%%%PageTrailer\n") ;
    }
  }
