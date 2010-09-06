/*
 * @(#)pipe_interface.h	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/* Interface to reading from the pipe mechanism on the VM side */



void init_pipe_interface();

/* Read a message from the pipe. Should consume the "size" portion
   of the message at least, and may buffer the rest of the message
   as well */
void read_message(int pipe);

/* Read the next nbytes from the pipe */
void get_bytes(int pipe, void* into, int nbytes);


/* Get the next word from the pipe */
int get_bits32(int pipe);

/* Read and allocate a string from the pipe */
char* get_string(int pipe);
