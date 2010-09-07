/*
 * @(#)gnomevfs.h	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

typedef int    gint;
typedef gint   gboolean;
typedef unsigned int    guint;

typedef struct _GList		GList;

typedef void* gpointer;

typedef gpointer ghandle;

typedef unsigned long long GnomeVFSFileSize;

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

typedef enum {
        GNOME_VFS_OK
} GnomeVFSResult;

typedef enum {
        GNOME_VFS_OPEN_NONE = 0,
        GNOME_VFS_OPEN_READ = 1 << 0,
        GNOME_VFS_OPEN_WRITE = 1 << 1,
        GNOME_VFS_OPEN_RANDOM = 1 << 2
} GnomeVFSOpenMode;

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

guint (*jws_gnome_vfs_get_file_info)(const char  *text_uri, void * info, gint n);
guint (*jws_gnome_vfs_unlink)(const char *text_uri);
guint (*jws_gnome_vfs_remove_directory)(const char *text_uri);
guint (*jws_gnome_vfs_read)(ghandle *handle, gconstpointer buffer, GnomeVFSFileSize buf_size, GnomeVFSFileSize * read_size);
guint (*jws_gnome_vfs_write)(ghandle *handle, gconstpointer buffer, GnomeVFSFileSize buf_size, GnomeVFSFileSize * write_size);
guint (*jws_gnome_vfs_mkdir)(const char *text_uri, guint perm);
guint (*jws_gnome_vfs_open)(ghandle *handle, const char *text_uri, guint open_mode);
guint (*jws_gnome_vfs_directory_open)(ghandle *handle, const char *text_uri, guint open_mode);
guint (*jws_gnome_vfs_directory_close)(ghandle *handle);
guint (*jws_gnome_vfs_close)(ghandle *handle);
guint (*jws_gnome_vfs_create)(ghandle *handle, const char *text_uri, guint open_mode, gboolean exclusive, guint perm);
gpointer (*jws_gnome_vfs_file_info_new)();
void (*jws_gnome_vfs_file_info_unref)(gpointer info);

const char * (*jws_gnome_vfs_result_to_string)(guint n);

void jws_throw_by_name(JNIEnv *env, const char *name, const char *msg);

