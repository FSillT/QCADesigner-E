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
// Header file for various file I/O and filesystem-     //
// related helpers, including an implementation of a    //
// line-sized "peek" buffer for a given FILE *. That    //
// is, opening the FILE * using the provided wrappers,  //
// one can use the provided ReadLine function to not    //
// only read a line from the file, but also peek the    //
// line so that you can retrieve it repeatedly until    //
// the caller is ready for the next line.               //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _FILEIO_HELPERS_H_
#define _FILEIO_HELPERS_H_

#include <stdio.h>
#include <glib.h>

FILE *file_open_and_buffer (char *pszFName) ;
void file_close_and_unbuffer (FILE *pfile) ;
char *ReadLine (FILE *pfile, char cComment, gboolean bPeek) ;
char *ReadPast (FILE *pfile, char *pattern, char cComment) ;
char *base_name (char *pszFile) ;
char *CreateUserFName (char *pszBaseName) ;
void tokenize_line (char *pszLine, int length, char **pszValue, char delim) ;
void tokenize_line_type (char *pszLine, int length, char **pszValue, char delim) ;
gboolean SkipPast (FILE *pfile, char cComment, ...) ;
int file_age_compare (char *pszFName1, char *pszFName2) ;
char *absolute_path (char *pszFName) ;
char *next_space_separated_value (char **ppsz) ;
double get_file_percent (FILE *pfile) ;

#endif /* _FILEIO_HELPERS_H_ */
