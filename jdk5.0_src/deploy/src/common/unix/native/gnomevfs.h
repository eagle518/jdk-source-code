/*
 * @(#)gnomevfs.h	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

typedef int    gint;
typedef gint   gboolean;
typedef unsigned int    guint;

typedef struct _GList		GList;

typedef void* gpointer;

struct _GList
{
  gpointer data;
  GList *next;
  GList *prev;
};

typedef enum {
	GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS,
	GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_PATHS,
	GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS_FOR_NON_FILES
} GnomeVFSMimeApplicationArgumentType;

typedef struct {
	char *id;
	char *name;
	char *command;
	gboolean can_open_multiple_files;
	GnomeVFSMimeApplicationArgumentType expects_uris;
	GList *supported_uri_schemes;
	gboolean requires_terminal;

	/* Padded to avoid future breaks in ABI compatibility */
	void *reserved1;
	void *reserved2;

} GnomeVFSMimeApplication;

typedef const void *gconstpointer;
typedef guint           (*GHashFunc)            (gconstpointer  key);

static void *pLibGNOME = NULL;


gboolean (*jws_gnome_vfs_init)(void);
char* (*jws_gnome_vfs_get_mime_type)(const char  *text_uri);
const char* (*jws_gnome_vfs_mime_get_value)(const char *mime_type, const char *key);
const char* (*jws_gnome_vfs_mime_get_description)(const char* mime_type);
const char* (*jws_gnome_vfs_mime_get_icon)(const char* mime_type);
GList* (*jws_gnome_vfs_mime_get_key_list)(const char* mime_type);
GnomeVFSMimeApplication* (*jws_gnome_vfs_mime_get_default_application)(const char *mime_type);
GList* (*jws_gnome_vfs_get_registered_mime_types)(void);
GList* (*jws_gnome_vfs_mime_get_extensions_list)(const char *mime_type);
gpointer (*jws_g_list_nth_data)(GList *list, guint n);
guint    (*jws_g_list_length)(GList *list);


