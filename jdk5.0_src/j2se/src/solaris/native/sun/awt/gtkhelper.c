/*
 * @(#)gtkhelper.c	1.2 04/07/26
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
  gcc -ldl -o gtkhelper -g gtkhelper.c
*/


#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#define GTK_LIB "libgtk-x11-2.0.so"

/* From enum GtkTextDirection */
#define GTK_TEXT_DIR_LTR 1
#define GTK_TEXT_DIR_RTL 2



/* We link GTK dynamically using dlopen(). The variables below are        */
/* pointers to functions we'll call.                                      */

static void (*gtk_init)(int *argc, char ***argv);
static void* (*gtk_image_new)(const char *str);
static void (*gtk_widget_set_direction)(void *widget, int dir);
static void* (*gtk_widget_render_icon)
     (void *widget, const char *stock_id, int size, const char *detail);

static int (*gdk_pixbuf_get_width)(const void *pixbuf);
static int (*gdk_pixbuf_get_height)(const void *pixbuf);
static int (*gdk_pixbuf_get_bits_per_sample)(const void *pixbuf);
static int (*gdk_pixbuf_get_rowstride)(const void *pixbuf);
static char* (*gdk_pixbuf_get_pixels)(const void *pixbuf);
static void (*g_object_unref)(void *object);

static void *libhandle;
static void *widget;


static int load_function(const char *name, const void **var)
{
  *var = dlsym(libhandle, name);
  if (*var == NULL) {
    fprintf(stderr, "Error resolving GTK symbol %s\n", name);
    return 0;
  } else {
    return 1;
  }
}


static int load_gtk()
{
  libhandle = dlopen(GTK_LIB, RTLD_LAZY | RTLD_GLOBAL);
  if (libhandle == NULL) {
    fprintf(stderr, "Error linking GTK: %s\n", dlerror());
    return 0;
  }

  if (!load_function("gtk_init", &gtk_init) ||
      !load_function("gtk_image_new", &gtk_image_new) ||
      !load_function("gtk_widget_set_direction", &gtk_widget_set_direction) ||
      !load_function("gtk_widget_render_icon", &gtk_widget_render_icon) ||
      !load_function("gdk_pixbuf_get_width", &gdk_pixbuf_get_width) ||
      !load_function("gdk_pixbuf_get_height", &gdk_pixbuf_get_height) ||
      !load_function("gdk_pixbuf_get_bits_per_sample",
                     &gdk_pixbuf_get_bits_per_sample) ||
      !load_function("gdk_pixbuf_get_rowstride", &gdk_pixbuf_get_rowstride) ||
      !load_function("gdk_pixbuf_get_pixels", &gdk_pixbuf_get_pixels) ||
      !load_function("g_object_unref", &g_object_unref)) {
    return 0;
  }
  return 1;
}


static int write_icon(const char *id, int size, char *orientation)
{
  int orn;
  void *icon;
  unsigned char id_len, size8;
  unsigned char w, h, rowstride8;
  unsigned char *pixels;
  int rowstride, bps;

  /* Set orientation on widget */
  orn = *orientation == 'r' ? GTK_TEXT_DIR_RTL : GTK_TEXT_DIR_LTR;
  (*gtk_widget_set_direction)(widget, orn);

  /* Render the icon */
  icon = (*gtk_widget_render_icon)(widget, id, size, NULL);
  if (icon == NULL) {
    return 0;
  }

  /* Check assumptions */
  rowstride = (*gdk_pixbuf_get_rowstride)(icon);
  bps = (*gdk_pixbuf_get_bits_per_sample)(icon);
  if (rowstride >= 256 || bps != 8) {
    return 0;
  }

  /* Write out icon data */
  size8 = (unsigned char) size;
  id_len = (unsigned char) strlen(id);
  w = (unsigned char) (*gdk_pixbuf_get_width)(icon);
  h = (unsigned char) (*gdk_pixbuf_get_height)(icon);
  rowstride8 = (unsigned char) rowstride;
  pixels = (*gdk_pixbuf_get_pixels)(icon);

  write(1, &id_len, 1);
  write(1, id, id_len);
  write(1, &size8, 1);
  write(1, &w, 1);
  write(1, &h, 1);
  write(1, &rowstride8, 1);
  write(1, pixels, rowstride * h);

  (*g_object_unref)(icon);
  return 1;
}


int main(int argc, char **argv)
{
  int i;

  if (argc < 3) {
    fprintf(stderr, "Usage: %s stock_id size orientation [...]'\n", argv[0]);
    return 1;
  }

  if (!load_gtk()) {
    return -1;
  }

  (*gtk_init)(&argc, &argv);
  widget = (*gtk_image_new)(NULL);
  for (i=1; i<argc; i+=3) {
    write_icon(argv[i], *argv[i+1] - '0', argv[i+2]);
  }

  dlclose(libhandle);
  close(1);
  return 0;
}
