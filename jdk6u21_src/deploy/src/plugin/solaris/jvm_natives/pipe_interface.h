/*
 * @(#)pipe_interface.h	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/* Interface to reading from the pipe mechanism on the VM side */

/* Initialize the pipe storage interface */
int init_pipe_interface(int pipe, void* rlock, void* wlock);

/* Read a message from the pipe. Should consume the "size" portion
   of the message at least, and may buffer the rest of the message
   as well */
int read_message(int pipe);

/* Retrieve the message and the length of the message, This call is always
   followed by read_message. Appropriate lock is required to make sure
   these 2 calls are executed one after another. 
*/
char* get_message(int pipe, int* length);

/* Read the next nbytes from the pipe */
void get_bytes(int pipe, void* into, int nbytes);


/* Get the next word from the pipe */
int get_bits32(int pipe);

/* Read and allocate a string from the pipe */
char* get_string(int pipe);

/* isRead = 1, return readlock,
   otherwise, return writelock;
*/
void* get_pipelock(int pipe, int isRead);
