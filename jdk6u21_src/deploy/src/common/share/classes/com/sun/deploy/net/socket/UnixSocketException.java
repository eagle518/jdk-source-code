/*
 * @(#)UnixSocketException.java	1.3 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net.socket;

import java.net.SocketException;

public class UnixSocketException extends SocketException {
    public static int EANY              = -1  /* unspecified error */;

    public static int ENOENT            =  2  /* The file does not exist. */;
    public static int ENOMEM            = 12  /* Insufficient kernel memory was available. */;
    public static int EACCES            = 13  /* Search permission is denied on a component of the path prefix */;
    public static int EFAULT            = 14  /* my_addr points outside the user's accessible address space. */;
    public static int ENOTDIR           = 20  /* A component of the path prefix is not a directory.*/;
    public static int EINVAL            = 22  /* The addrlen is wrong, or the socket was not in the AF_UNIX family. */;
    public static int ENFILE            = 23  /* File table overflow */;
    public static int EMFILE            = 24  /* Too many open files */;
    public static int EROFS             = 30  /* The socket inode would reside on a read-only file system. */;
    public static int ELOOP             = 40  /* Too many symbolic links were encountered in resolving my_addr. */;
    public static int ENAMETOOLONG      = 36  /* my_addr is too long */;
    public static int EBADFD            = 77  /* File descriptor in bad state */;
    public static int ENOTSOCK          = 88  /* Socket operation on non-socket */;
    public static int EAFNOSUPPORT      = 97  /* Address family not supported by protocol */;
    public static int EPROTONOSUPPORT   = 93  /* Protocol not supported */;
    public static int ESOCKTNOSUPPORT   = 94  /* Socket type not supported */;
    public static int EOPNOTSUPP        = 95  /* Operation not supported on transport endpoint */;
    public static int EADDRINUSE        = 98  /* Address already in use */;
    public static int EADDRNOTAVAIL     = 99  /* Cannot assign requested address */;
    public static int ENOBUFS           =105  /* No buffer space available */;

    public static UnixSocketException createUnixSocketException(String msg, int errno) {
        return new UnixSocketException(msg, errno);
    }

    /**
     * Constructs a new UnixSocketException with the specified detail
     * message as to why the UnixSocketException error occurred.
     * A detail message is a String that gives a specific
     * description of this error.
     * @param msg the detail message
     * @param errno the detail errno
     */
    public UnixSocketException(String msg, int errno) {
        super(msg);
        this.errno = errno;
    }

    /**
     * Constructs a new UnixSocketException with the specified detail
     * message as to why the UnixSocketException error occurred.
     * A detail message is a String that gives a specific
     * description of this error.
     * @param msg the detail message
     */
    public UnixSocketException(String msg) {
        super(msg);
        this.errno = EANY;
    }

    /**
     * Construct a new UnixSocketException with no detailed message.
     */
    public UnixSocketException() {
        this.errno = EANY;
    }

    public String getMessage() {
        return "errno "+errno+": "+super.getMessage();
    }

    public int errno() {
        return errno;
    }
    
    private int errno;
}

