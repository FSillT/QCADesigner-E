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
// Various constants, including physical constants, and //
// other enumerations.                                  //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _GLOBAL_CONSTS_H_
#define _GLOBAL_CONSTS_H_

#ifdef WIN32
  #include <windows.h>
#endif

///////////////////////////////////////////////////////////////////
///////////////////////// DEFINES /////////////////////////////////
///////////////////////////////////////////////////////////////////

//!File OPEN/SAVE actions used by file save dialog to determing whether to open or save
//!the argument file.
#ifdef STDIO_FILEIO
#define FILEOP_NO_ACTION 0
#define FILEOP_OPEN 1
#define FILEOP_SAVE_AS 2
#define FILEOP_EXPORT 3
#define FILEOP_IMPORT 4
#define FILEOP_OPEN_RECENT 5
#define FILEOP_SAVE 7
#define FILEOP_CMDLINE 9
#define FILEOP_AUTOSAVE 10
#define FILEOP_AUTOLOAD 11
#endif /* def STDIO_FILEIO */
#define FILEOP_NEW 6
#define FILEOP_CLOSE 8

#define PRINTED_OBJECTS_DIE     0
#define PRINTED_OBJECTS_CELLS   1
#define PRINTED_OBJECTS_COLOURS 2

//!Color defines
#define GREEN 0
#define GREEN1 5
#define GREEN2 6
#define GREEN3 7
#define RED 1
#define BLUE 2
#define YELLOW 3
#define WHITE 4
#define ORANGE 8
#define BLACK 9
#define HONEYCOMB_BACKGROUND 10
#define PURPLE

//!Simulation Engines
#define BISTABLE 1
#define DIGITAL_SIM 2
#define SCQCA 3
#define COHERENCE_VECTOR 4
#define COHERENCE_VECTOR_ENERGY 5


//!Simulation Types
#define EXHAUSTIVE_VERIFICATION 0
#define VECTOR_TABLE 1

//!Simulation Algorithms
#define RUNGE_KUTTA 1
#define EULER_METHOD 2

//!Some useful physical constants
#define QCHARGE_SQUAR_OVER_FOUR 6.417423538e-39
#define QCHARGE 1.602176462e-19
#define HALF_QCHARGE 0.801088231e-19
#define OVER_QCHARGE 6.241509745e18
#define ONE_OVER_FOUR_HALF_QCHARGE 3.12109e18
#define EPSILON 8.8541878e-12
#define PI (double)(3.1415926535897932384626433832795)
#define FOUR_PI 12.56637061
#define FOUR_PI_EPSILON 1.112650056e-10
#define HBAR 1.0545887e-34
#define PRECISION 1e-5

//!Clock types
#define COS 1
#define RAMP 2
#define GAUSS 3
  
// Maximum length of a file system path
#ifdef MAX_PATH
  // The Win32 headers have a similar variable already defined, so let's use it
  #define PATH_LENGTH MAX_PATH
#else
  #define PATH_LENGTH 1024
#endif

// Font to use
#ifndef WIN32
  #define QCAD_GDKFONT "-adobe-courier-medium-r-normal--12-*-*-*-*-*-*"
#else
  #define QCAD_GDKFONT "Courier"
#endif
#define PS_FONT "Helvetica"

#endif /* _GLOBAL_CONSTS_H_ */
