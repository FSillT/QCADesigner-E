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
// Header for GTK utility functions, including pixmap   //
// loading and pixmap data directory maintenance, as    //
// well as definitions for string internationalization. //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _SUPPORT_H_
#define _SUPPORT_H_

//#ifdef WIN32
//  // For now, until I figure out this whole libintl business
//  #undef ENABLE_NLS
//#endif

#ifdef GTK_GUI
  #include <gtk/gtk.h>
#endif /* def GTK_GUI */

// Standard gettext macros.
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

// Public Functions.

// This function returns a widget in a component created by Glade.
// Call it with the toplevel widget in the component (i.e. a window/dialog),
// or alternatively any widget in the component, and the name of the widget
// you want returned.
#ifdef GTK_GUI
GtkWidget *lookup_widget (GtkWidget *widget, const gchar *widget_name);

// This function returns the fully qualified path for a filename iff it
// finds it among the pixmap directories
gchar *find_pixmap_file (const gchar *filename) ;

// Use this function to set the directory containing installed pixmaps.
void add_pixmap_directory (const gchar *directory);


// Private Functions.

// This is used to create the pixmaps used in the interface.
GtkWidget *create_pixmap (GtkWidget *widget, const gchar *filename);

// Add a stock icon to the list of stock icons
void add_stock_icon (const gchar *filename, const gchar *stock_id) ;

// This is used to create the pixbufs used in the interface.
GdkPixbuf *create_pixbuf (const gchar *filename);

// This is used to set ATK action descriptions.
void glade_set_atk_action_description (AtkAction *action, const gchar *action_name, const gchar *description);
#endif /* def GTK_GUI */
#endif /* _SUPPORT_H_ */
