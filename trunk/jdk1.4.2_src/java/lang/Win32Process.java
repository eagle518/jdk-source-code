/*
 * @(#)Win32Process.java	1.26 03/02/20
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package java.lang;

import java.io.*; 

class Win32Process extends Process {
    private long handle = 0;
    private FileDescriptor stdin_fd;
    private FileDescriptor stdout_fd;
    private FileDescriptor stderr_fd;
    private OutputStream stdin_stream;
    private InputStream stdout_stream;
    private InputStream stderr_stream;

    Win32Process(String cmd[], String env[]) throws Exception {
        this(cmd, env, null);
    }
    
    Win32Process(String cmd[], String env[], String path) throws Exception {
	StringBuffer cmdbuf = new StringBuffer(80);
	for (int i = 0; i < cmd.length; i++) {
            if (i > 0) {
                cmdbuf.append(' ');
            }
	    String s = cmd[i];
	    if (s.indexOf(' ') >= 0 || s.indexOf('\t') >= 0) {
	        if (s.charAt(0) != '"') {
		    cmdbuf.append('"');
		    cmdbuf.append(s);
		    if (s.endsWith("\\")) {
			cmdbuf.append("\\");
		    }
		    cmdbuf.append('"');
                } else if (s.endsWith("\"")) {
		    /* The argument has already been quoted. */
		    cmdbuf.append(s);
		} else {
		    /* Unmatched quote for the argument. */
		    throw new IllegalArgumentException();
		}
	    } else {
	        cmdbuf.append(s);
	    }
	}
	String cmdstr = cmdbuf.toString();

        String envstr = null;
        if (env != null) {
            StringBuffer envbuf = new StringBuffer(256);
            for (int i = 0; i < env.length; i++) {
                envbuf.append(env[i]).append('\0');
            }
            envstr = envbuf.toString();
        }

	stdin_fd = new FileDescriptor();
	stdout_fd = new FileDescriptor();
	stderr_fd = new FileDescriptor();
	
	handle = create(cmdstr, envstr, path, stdin_fd, stdout_fd, stderr_fd);
	java.security.AccessController.doPrivileged(
				    new java.security.PrivilegedAction() {
	    public Object run() {
		stdin_stream = 
		    new BufferedOutputStream(new FileOutputStream(stdin_fd));
		stdout_stream =
		    new BufferedInputStream(new FileInputStream(stdout_fd));
		stderr_stream = 
		    new FileInputStream(stderr_fd);
		return null;
	    }
	});
    }

    public OutputStream getOutputStream() {
	return stdin_stream;
    }

    public InputStream getInputStream() {
	return stdout_stream;
    }

    public InputStream getErrorStream() {
	return stderr_stream;
    }

    public void finalize() {
	close();
    }

    public native int exitValue();
    public native int waitFor();
    public native void destroy();

    private native long create(String cmdstr, String envstr, String path,
			      FileDescriptor in_fd,
			      FileDescriptor out_fd,
			      FileDescriptor err_fd);
    private native void close();
}
