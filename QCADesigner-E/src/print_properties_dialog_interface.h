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
// Header for the interface for the print properties    //
// dialog.                                              //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _PRINT_PROPERTIES_DIALOG_INTERFACE_H_
#define _PRINT_PROPERTIES_DIALOG_INTERFACE_H_

#include <gtk/gtk.h>

typedef struct
  {
  GtkWidget     *dlgPrintProps ;
  GtkAdjustment *adjCXPages ;
  GtkAdjustment *adjCYPages ;
  GtkAdjustment *adjNanoToUnits ;
  GtkWidget     *fmFit ;
  GtkWidget     *fmPrintOrder ;
  GtkWidget     *fmScale ;
  GSList        *grpScaleOpts ;
  GtkWidget     *lblNanoIs ;
  GtkWidget     *lblPgsTall ;
  GtkWidget     *lblPgsWide ;
  GtkWidget     *lblPrintOrder ;
  GtkWidget     *lblScale ;
  GtkWidget     *lstPrintedObjs ;
  GtkWidget     *rbFitPages ;
  GtkWidget     *rbFixedScale ;
  GtkWidget     *scrwPrintedObjs ;
  GtkWidget     *spnCXPages ;
  GtkWidget     *spnCYPages ;
  GtkWidget     *spnNanoToUnits ;
  GtkWidget     *table2 ;
  GtkWidget     *table3 ;
  GtkWidget     *table4 ;
  GtkWidget     *tblScale ;
  GtkWidget     *tbtnPrintOrder ;
  GtkWidget     *vbPrintedObjs ;
  GtkWidget     *vpPrintedObjs ;
  GtkWidget     *vbScale ;
  GtkWidget     *tbtnCenter ;
  GtkWidget     *tblCenter ;
  GtkWidget     *fmCenter ;
  GtkWidget     *lblCenter ;
  GtkWidget     *chkColour ;
  GtkWidget     *chkPrintDots ;

  GtkWidget     **ppPrintedObjs ;
  int           icPrintedObjs ;
  } print_properties_D ;

void create_print_design_properties_dialog (print_properties_D *dialog, print_design_OP *pPO) ;

#endif /* _PRINT_PROPERTIES_DIALOG_INTERFACE_H_ */
