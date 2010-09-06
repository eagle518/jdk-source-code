/*
 * @(#)IOVecWrapper.java	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import sun.misc.*;


/**
 * Manipulates a native array of iovec structs on Solaris:
 *
 * typedef struct iovec {
 *    caddr_t  iov_base;
      int      iov_len;
 * } iovec_t;
 *
 * @author Mike McCloskey
 * @version 1.13 03/12/19
 * @since 1.4
 */

class IOVecWrapper {

    // Miscellaneous constants
    static int BASE_OFFSET = 0;
    static int LEN_OFFSET;
    static int SIZE_IOVEC;

    // The iovec array
    private AllocatedNativeObject vecArray;

    // Base address of this array
    long address;

    // Address size in bytes
    static int addressSize;

    IOVecWrapper(int newSize) {
        newSize = (newSize + 1) * SIZE_IOVEC;
        vecArray = new AllocatedNativeObject(newSize, false);
        address = vecArray.address();
    }

    void putBase(int i, long base) {
        int offset = SIZE_IOVEC * i + BASE_OFFSET;
        if (addressSize == 4)
            vecArray.putInt(offset, (int)base);
        else
            vecArray.putLong(offset, base);
    }

    void putLen(int i, long len) {
        int offset = SIZE_IOVEC * i + LEN_OFFSET;
        if (addressSize == 4)
            vecArray.putInt(offset, (int)len);
        else
            vecArray.putLong(offset, len);
    }

    void free() {
        vecArray.free();
    }

    static {
        addressSize = Util.unsafe().addressSize();
        LEN_OFFSET = addressSize;
        SIZE_IOVEC = (short) (addressSize * 2);
    }
}
