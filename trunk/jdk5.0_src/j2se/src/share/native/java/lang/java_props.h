/*
 * @(#)java_props.h	1.17 04/06/13
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _JAVA_PROPS_H
#define _JAVA_PROPS_H

#include <jni_util.h>

typedef struct {
    char *os_name;
    char *os_version;
    char *os_arch;

    char *tmp_dir;
    char *font_dir;
    char *user_dir;

    char *file_separator;
    char *path_separator;
    char *line_separator;

    char *user_name;
    char *user_home;

    char *language;
    char *country;
    char *variant;
    char *encoding;
    char *sun_jnu_encoding;
    char *timezone;

    char *printerJob;
    char *graphics_env;
    char *awt_toolkit;

    char *unicode_encoding;	/* The default endianness of unicode
				    i.e. UnicodeBig or UnicodeLittle   */

    const char *cpu_isalist;	/* list of supported instruction sets */

    char *cpu_endian;           /* endianness of platform */
    
    char *data_model;           /* 32 or 64 bit data model */
 
    char *patch_level;          /* patches/service packs installed */

    char *desktop;              /* Desktop name. */

} java_props_t;

java_props_t *GetJavaProperties(JNIEnv *env);

#endif /* _JAVA_PROPS_H */
