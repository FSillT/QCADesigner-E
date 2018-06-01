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
// GTK utility functions, including pixmap loading and  //
// pixmap data directory maintenance.                   //
//                                                      //
//////////////////////////////////////////////////////////

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "fileio_helpers.h"
#include "support.h"

#define DBG_SUP(s)

GtkWidget *lookup_widget (GtkWidget *widget, const gchar *widget_name)
  {
  GtkWidget *parent, *found_widget;

  for (;;)
    {
    if (GTK_IS_MENU (widget))
      parent = gtk_menu_get_attach_widget (GTK_MENU (widget));
    else
      parent = widget->parent;
    if (!parent)
      parent = g_object_get_data (G_OBJECT (widget), "GladeParentKey");
    if (parent == NULL)
      break;
    widget = parent;
    }

  found_widget = (GtkWidget*) g_object_get_data (G_OBJECT (widget), widget_name);
  if (!found_widget) g_warning ("Widget not found: %s", widget_name);
  return found_widget;
  }

static GList *pixmaps_directories = NULL;

// Use this function to set the directory containing installed pixmaps.
void add_pixmap_directory (const gchar *directory)
  {pixmaps_directories = g_list_prepend (pixmaps_directories, g_strdup (directory));}

// This is an internally used function to find pixmap files.
gchar *find_pixmap_file (const gchar *filename)
  {
  GList *elem;

  // We step through each of the pixmaps directory to find it.
  elem = pixmaps_directories;
  while (elem)
    {
    gchar *pathname = g_strdup_printf ("%s%s%s", (gchar*)elem->data, G_DIR_SEPARATOR_S, filename);

    if (g_file_test (pathname, G_FILE_TEST_EXISTS)) return pathname;
    g_free (pathname);
    elem = elem->next;
    }
  return NULL;
  }

// This is an internally used function to create pixmaps.
GtkWidget *create_pixmap (GtkWidget *widget, const gchar *filename)
  {
  gchar *pathname = NULL;
  GtkWidget *pixmap;

  if (!filename || !filename[0])
    return gtk_image_new ();

  pathname = find_pixmap_file (filename);

  DBG_SUP (fprintf (stderr, "Found '%s' here: '%s'\n", filename, pathname)) ;

  if (!pathname)
    {
      g_warning (_("Couldn't find pixmap file: %s"), filename);
      return gtk_image_new ();
    }

  pixmap = gtk_image_new_from_file (pathname);
  g_free (pathname);
  return pixmap;
  }

// This is an internally used function to create pixmaps.
GdkPixbuf *create_pixbuf (const gchar *filename)
  {
  gchar *pathname = NULL;
  GdkPixbuf *pixbuf;
  GError *error = NULL;

  if (!filename || !filename[0])
    return NULL;

  pathname = find_pixmap_file (filename);

  if (!pathname)
    {
    g_warning (_("Couldn't find pixmap file: %s"), filename);
    return NULL;
    }

  if (!(pixbuf = gdk_pixbuf_new_from_file (pathname, &error)))
    {
    fprintf (stderr, "Failed to load pixbuf file: %s: %s\n", pathname, error->message);
    g_error_free (error);
    }
  g_free (pathname);
  return pixbuf;
  }

// This is used to set ATK action descriptions.
void glade_set_atk_action_description (AtkAction *action, const gchar *action_name, const gchar *description)
  {
  gint n_actions, i;

  n_actions = atk_action_get_n_actions (action);
  for (i = 0; i < n_actions; i++)
    if (!strcmp (atk_action_get_name (action, i), action_name))
      atk_action_set_description (action, i, description);
  }

void add_stock_icon (const gchar *filename, const gchar *stock_id)
  {
  GtkIconSource *icon_source = NULL ;
  GtkIconSet *icon_set = NULL ;
  static GtkIconFactory *icon_factory = NULL ;
  char *psz = NULL ;

  if (NULL == icon_factory)
    gtk_icon_factory_add_default (icon_factory = gtk_icon_factory_new ()) ;

  if (NULL != (icon_source = gtk_icon_source_new ()))
    {
    gtk_icon_source_set_filename (icon_source, psz = find_pixmap_file (filename)) ;
    if (NULL == (icon_set = gtk_icon_factory_lookup (icon_factory, stock_id)))
      {
      icon_set = gtk_icon_set_new () ;
      gtk_icon_factory_add (icon_factory, stock_id, icon_set) ;
      }
    gtk_icon_set_add_source (icon_set, icon_source) ;
    }
  }
