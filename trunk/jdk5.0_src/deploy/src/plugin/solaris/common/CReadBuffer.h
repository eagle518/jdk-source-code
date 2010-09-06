/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__CReadBuffer__)
#define __CReadBuffer__

class CReadBuffer {
public:

    CReadBuffer(int);
    int getInt(int *);
    int getShort(short *);
    int getString(char **);
    int getByte(char *);
    static void free(char *);

private:
    int getIt(char *, int);
    int m_fd;
};

#endif
