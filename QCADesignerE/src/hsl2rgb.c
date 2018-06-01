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
// Convert a GdkColor structure containing an RGB value //
// to one containing an HSL value and vice versa. The   //
// members of the GdkColor structure (.red, .green,     //
// .blue) are abused to hold, H, S, and L,              //
// respectively.                                        //
//                                                      //
//////////////////////////////////////////////////////////

#include <math.h>
#include "gdk_structs.h"
#include "global_consts.h"
#include "hsl2rgb.h"

void HSLToRGB (GdkColor *clr)
  {
  float h, s, l, r, g, b ;

  h = (float)(clr->red) / 65535 ;
  s = (float)(clr->green) / 65535 ;
  l = (float)(clr->blue) / 65535 ;

  r = 0.0 + MAX (0.0, MIN (1.0, (0.5 + cos (PI / 180.0 * (00.0 + h * 360.0))))) ;
  g = 1.0 - MAX (0.0, MIN (1.0, (0.5 + cos (PI / 180.0 * (60.0 + h * 360.0))))) ;
  b = 1.0 - MAX (0.0, MIN (1.0, (0.5 + cos (PI / 180.0 * (60.0 - h * 360.0))))) ;

  r = l + (r - l) * s ;
  g = l + (g - l) * s ;
  b = l + (b - l) * s ;

  r += (l - 0.5f) * 2.0f * (l < 0.5f ? r : (1.0f - r)) ;
  g += (l - 0.5f) * 2.0f * (l < 0.5f ? g : (1.0f - g)) ;
  b += (l - 0.5f) * 2.0f * (l < 0.5f ? b : (1.0f - b)) ;

  clr->red = r * 65535 ;
  clr->green = g * 65535 ;
  clr->blue = b * 65535 ;
  }

void RGBToHSL (GdkColor *clr)
  {
  float h = 0 , s = 1.0f, l = 0.5f, r, g, b, r_dist, g_dist, b_dist, fMax, fMin ;

  r = (float)(clr->red) / 65535 ;
  g = (float)(clr->green) / 65535 ;
  b = (float)(clr->blue) / 65535 ;

  fMax = MAX (r, MAX (g, b)) ;
  fMin = MIN (r, MIN (g, b)) ;

  l = (fMax + fMin) / 2 ;
  if (fMax - fMin <= 0.00001)
    {
    h = 0 ;
    s = 0 ;
    }
  else
    {
    s = (fMax - fMin) / ((l < 0.5) ? (fMax + fMin) : (2 - fMax - fMin)) ;
    r_dist = (fMax - r) / (fMax - fMin) ;
    g_dist = (fMax - g) / (fMax - fMin) ;
    b_dist = (fMax - b) / (fMax - fMin) ;
    if (r == fMax) h = b_dist - g_dist ;
    else
    if (g == fMax) h = 2 + r_dist - b_dist ;
    else
    if (b == fMax) h = 4 + g_dist - r_dist ;
    h *= 60 ;
    if (h < 0) h += 360 ;
    }

  clr->red = (h * 65535.0 / 360.0) ;
  clr->green = (s * 65535) ;
  clr->blue = (l * 65535) ;
  }
