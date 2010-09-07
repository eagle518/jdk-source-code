/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
