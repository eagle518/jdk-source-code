/*
 * @(#)version_comp.h	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef VERSION_COMP_H
#define VERSION_COMP_H

/*
 * Function prototypes.
 */
int exact_version_id(char *id1, char *id2);
int prefix_version_id(char *id1, char *id2);
int acceptable_release(char *release, char *version_string);
int valid_version_string(char *version_string);

#endif /* VERSION_COMP_H */
