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
// Various file I/O and filesystem-related helpers, in- //
// cluding an implementation of a line-sized "peek"     //
// buffer for a given FILE *. That is, opening the      //
// FILE * using the provided wrappers, one can use the  //
// provided ReadLine function to not only read a line   //
// from the file, but also peek the line so that you    //
// can retrieve it repeatedly until the caller is ready //
// for the next line.                                   //
//                                                      //
//////////////////////////////////////////////////////////

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef WIN32
  #include <windows.h>
#endif /* def WIN32 */
#include "exp_array.h"
#include "fileio_helpers.h"

#define DBG_FIOH(s)
#define DBG_FIOH_LOOP(s)
//#define DBG_FIOH_PATTERNS
#ifdef DBG_FIOH_PATTERNS
  #define DBG_FIOH_SKIP_LOOP(s)
#else /* not def DBG_FIOH_PATTERNS */
  #define DBG_FIOH_SKIP_LOOP(s)
#endif /* def DBG_FIOH_PATTERNS */
#define DBG_FIOH_READLINE_OUT(s)

typedef struct
  {
  EXP_ARRAY *buffer ;
  gboolean bWasPeek ;
  int file_size ;
  } BUFFER_INFO ;

static GHashTable *hash_table = NULL ;

FILE *file_open_and_buffer (char *pszFName)
  {
  FILE *pfile = NULL ;
  BUFFER_INFO *pbi = NULL ;
  int file_size = 0 ;

  if (NULL == (pfile = fopen (pszFName, "r"))) return NULL ;

  fseek (pfile, 0, SEEK_END) ;
  file_size = ftell (pfile) ;
  fclose (pfile) ;

  if (NULL == (pfile = fopen (pszFName, "r"))) return NULL ;

  if (NULL == hash_table)
    if (NULL == (hash_table = g_hash_table_new (NULL, NULL)))
      {
      fprintf (stderr, "Failed to create file peek buffer hash table\n") ;
      exit (127) ;
      }

  if (NULL == (g_hash_table_lookup (hash_table, (gconstpointer)pfile)))
    {
    pbi = g_malloc0 (sizeof (BUFFER_INFO)) ;
    pbi->buffer = exp_array_new (sizeof (char), 1) ;
    pbi->bWasPeek = FALSE ;
    pbi->file_size = file_size ;
    g_hash_table_insert (hash_table, (gpointer)pfile, (gpointer)pbi) ;
    }

  return pfile ;
  }

// Keep a buffer to hold a line of string, stripping comments. Return the buffer
char *ReadLine (FILE *pfile, char cComment, gboolean bPeek)
  {
  BUFFER_INFO bi = {NULL, FALSE} ;
  BUFFER_INFO *pbi = NULL ;
  gboolean bFoundHash = FALSE ;
  char *pszRet = NULL ;
  char cZero = 0 ;
  int Nix ;

  if (NULL == hash_table)
    {
    pbi = &bi ;
    pbi->buffer = exp_array_new (sizeof (char), 1) ;
    }
  else
  if (!(bFoundHash = (NULL != (pbi = g_hash_table_lookup (hash_table, pfile)))))
    {
    pbi = &bi ;
    pbi->buffer = exp_array_new (sizeof (char), 1) ;
    }

  if (!(pbi->bWasPeek))
    {
    char pszBuffer[81] = {0} ;
    int cb = 0 ;
    exp_array_remove_vals (pbi->buffer, 1, 0, pbi->buffer->icUsed) ;

    while (!feof (pfile))
      {
      fgets (pszBuffer, 81, pfile) ;

      if ((cb = strlen (pszBuffer)) > 0)
        exp_array_insert_vals (pbi->buffer, pszBuffer, cb, 1, -1) ;
      else
        break ;

      if ('\n' == exp_array_index_1d (pbi->buffer, char, pbi->buffer->icUsed - 1)) 
        {
        exp_array_index_1d (pbi->buffer, char, pbi->buffer->icUsed - 1) = 0 ;
        break ;
        }
      }

    if (pbi->buffer->icUsed > 0)
      {
      if (exp_array_index_1d (pbi->buffer, char, pbi->buffer->icUsed - 1) != 0)
        exp_array_insert_vals (pbi->buffer, &cZero, 1, 1, -1) ;
      if (pbi->buffer->icUsed > 1)
        {
        for (Nix = pbi->buffer->icUsed - 2 ; Nix > -1 ; Nix--)
          {
          if ('\r' == exp_array_index_1d (pbi->buffer, char, Nix) ||
              cComment == exp_array_index_1d (pbi->buffer, char, Nix))
            exp_array_index_1d (pbi->buffer, char, Nix) = 0 ;
          }
        }
      }
    }

  pbi->bWasPeek = bPeek ;

  if (pbi->buffer->icUsed > 0)
    pszRet = g_strdup (&(exp_array_index_1d (pbi->buffer, char, 0))) ;

  if (!bFoundHash)
    exp_array_free (pbi->buffer) ;

  DBG_FIOH_LOOP (fprintf (stderr, "ReadLine: Before returning:\n")) ;
  DBG_FIOH_LOOP (exp_array_dump (pbi->buffer, stderr, 0)) ;
  DBG_FIOH (fprintf (stderr, "ReadLine: Final return value is \"%s\"\n", pszRet)) ;

  DBG_FIOH_READLINE_OUT (fprintf (stderr, "ReadLine:pszRet = %s\n", pszRet)) ;
  DBG_FIOH_READLINE_OUT (fflush (stderr)) ;

  return pszRet ;
  }

void file_close_and_unbuffer (FILE *pfile)
  {
  BUFFER_INFO *pbi = NULL ;

  if (NULL != (pbi = g_hash_table_lookup (hash_table, pfile)))
    {
    g_hash_table_remove (hash_table, pfile);
    exp_array_free (pbi->buffer) ;
    g_free (pbi) ;
    }

  fclose (pfile) ;
  }

// Keep reading the file until the current line matches one of a NULL-terminated
// list of patter strings
gboolean SkipPast (FILE *pfile, char cComment, ...)
  {
  char *pszLine = NULL ;
  char *pszPattern = NULL ;
  va_list va ;

#ifdef DBG_FIOH_PATTERNS
  fprintf (stderr, "SkipPast: Entering with the following patterns:\n") ;
  va_start (va, cComment) ;
  while (NULL != (pszPattern = va_arg (va, char *)))
    fprintf (stderr, "  Pattern:|%s|\n", pszPattern) ;
  va_end (va) ;
#endif /* def DBG_FIOH_PATTERNS */

  while (!feof (pfile))
    {
    if (NULL == (pszLine = ReadLine (pfile, cComment, FALSE))) return FALSE ;
    DBG_FIOH_SKIP_LOOP (fprintf (stderr, "SkipPast:pszLine = \"%s\"\n", pszLine)) ;
    va_start (va, cComment) ;
    while (NULL != (pszPattern = va_arg (va, char *)))
      if (!strcmp (pszLine, pszPattern))
        {
        g_free (pszLine) ;
        va_end (va) ;
        DBG_FIOH_SKIP_LOOP (fprintf (stderr, "SkipPast: pszLine matches pattern |%s|, so exiting successfully\n", pszPattern)) ;
        return TRUE ;
        }
      va_end (va) ;
    g_free (pszLine) ;
    }
#ifdef DBG_FIOH_PATTERNS
  fprintf (stderr, "SkipPast: Exiting unsuccessfully\n") ;
#endif /* def DBG_FIOH_PATTERNS */
  return FALSE ;
  }

char *base_name (char *pszFile)
  {
  char *pszRet = &(pszFile[strlen (pszFile)]) ;
  while (--pszRet > pszFile)
    if (*pszRet == G_DIR_SEPARATOR)
      return pszRet + 1 ;
  return pszFile ;
  }

char *CreateUserFName (char *pszBaseName)
  {
  char *pszHome = getenv ("HOME"), *psz = NULL, *pszRet = NULL ;
  psz = g_strdup_printf ("%s%s.QCADesigner", pszHome,
    G_DIR_SEPARATOR == pszHome[strlen (pszHome) - 1] ? "" : G_DIR_SEPARATOR_S) ;
#ifndef WIN32
  mkdir (psz, 07777) ;
#else
  mkdir (psz) ;
#endif
  pszRet = g_strdup_printf ("%s%c%s", psz, G_DIR_SEPARATOR, pszBaseName) ;
  g_free (psz) ;
  return pszRet ;
  }

void tokenize_line (char *pszLine, int length, char **pszValue, char delim)
  {
  int idx = -1 ;

  for (idx = 0 ; idx < length ; idx++)
    if (delim == pszLine[idx])
      {
      pszLine[idx] = 0 ;
      (*pszValue) = &(pszLine[idx + 1]) ;
      return ;
      }
  }

void tokenize_line_type (char *pszLine, int length, char **pszValue, char delim)
  {
  char *pszEnd = NULL ;

  tokenize_line (pszLine, length, pszValue, delim) ;
  for (pszEnd = (*pszValue); (int)pszEnd < (int)pszLine + length ; pszEnd++)
    if (']' == *pszEnd)
      {
      *pszEnd = 0 ;
      return ;
      }
  }

// returns 1 if the "age" of pszFName1 is greater than the "age" of pszFName2, etc.
int file_age_compare (char *pszFName1, char *pszFName2)
  {
  struct stat s1, s2 ;

  stat (pszFName1, &s1) ;
  stat (pszFName2, &s2) ;

  return (s1.st_mtime  < s2.st_mtime ? 1 :
          s1.st_mtime == s2.st_mtime ? 0 : -1) ;
  }

char *absolute_path (char *pszFName)
  {
#ifdef WIN32
  int icNeeded = 0 ;
  char *pszFull = NULL ;
  char *pszShort = NULL ;
  char *pszFile = NULL ;
#else /* ndef WIN32 */
  char szRealPath[PATH_MAX] = "" ;
#endif

  if (NULL == pszFName) return NULL ;

#ifdef WIN32
  pszFull = g_malloc (1) ;
  pszFull[0] = 0 ;
  if (0 == (icNeeded = GetFullPathName (pszFName, 0, pszFull, &pszFile)))
    {
    g_free (pszFull) ;
    return g_strdup (pszFName) ;
    }
  pszFull = g_realloc (pszFull, icNeeded * sizeof (TCHAR)) ;
  pszFull[0] = 0 ;
  GetFullPathName (pszFName,icNeeded, pszFull, &pszFile) ;

  pszShort = g_malloc (1) ;
  pszShort[0] = 0 ;
  if (0 == (icNeeded = GetShortPathName (pszFull, pszShort, 0)))
    {
    g_free (pszFull) ;
    g_free (pszShort) ;
    return g_strdup (pszFName) ;
    }
  pszShort = g_realloc (pszShort, icNeeded * sizeof (TCHAR)) ;
  pszShort[0] = 0 ;
  GetShortPathName (pszFull, pszShort, icNeeded) ;

  g_free (pszFull) ;
  return pszShort ;
#else /* ndef WIN32 */
  if (NULL != realpath (pszFName, szRealPath))
    return g_strdup (szRealPath) ;
  else
    return g_strdup (pszFName) ;
#endif /* def WIN32 */
  }

// (*ppsz) is assumed not to start with any whitespace
char *next_space_separated_value (char **ppsz)
  {
  char *pszRet = NULL, *pszItr = NULL ;

  if (NULL == ppsz) return NULL ;
  if (NULL == (*ppsz)) return NULL ;

  // The string is empty, so indicate termination
  if ('\0' == ((*ppsz)[0]))
    return NULL ;
  else
    pszRet = (*ppsz) ;

  for (pszItr = pszRet ; !((*pszItr) == '\0' || (*pszItr) == ' ') ; pszItr++) ;

  if ((*pszItr) != '\0')
    {
    (*pszItr) = '\0' ;
    for (pszItr++ ; (' ' == (*pszItr) && '\0' != (*pszItr)) ; pszItr++) ;
    }

  (*ppsz) = pszItr ;

  return pszRet ;
  }

double get_file_percent (FILE *pfile)
  {
  long pos = 0, size = 0 ;
  BUFFER_INFO *pbi = NULL ;

  pos = ftell (pfile) ;

  if (-1 == pos) return 0.0 ;

  if (NULL == (pbi = g_hash_table_lookup (hash_table, (gconstpointer)pfile)))
    {
    fprintf (stderr, "get_file_percent:WARNING:Using fseek to determine percentage, because file isn't peekable. This doesn't work on Windoze.\n") ;
    if (-1 == fseek (pfile, 0, SEEK_END)) return 0.0 ;

    if (-1 == (size = ftell (pfile)))
      {
      fseek (pfile, pos, SEEK_SET) ;
      return 0.0 ;
      }

    fseek (pfile, pos, SEEK_SET) ;
    }
  else
    size = pbi->file_size ;

  return (((double)pos) / ((double)size)) ;
  }
