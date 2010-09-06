/*
 * @(#)zip.cpp	1.5 03/12/19 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
/**
 * Note: Lifted from uncrunch.c from jdk sources
 */ 
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/stat.h>

#include <direct.H>

#include <stdlib.h>

#ifndef _MSC_VER 
#include <strings.h>
#else

#endif

#include "zip.h"



// Data for ZIP central directory.
uchar central_directory[1024*1024];
uchar *central_directory_limit = central_directory + sizeof(central_directory);
uchar *central_directory_ptr = central_directory;
ushort central_directory_count;

// Globals
FILE *jarfp;
static char *tfile;

void l_abort(char *);

static uint output_file_offset = 0;

static char message[MAX_PATH*2];

static char ziptmpfile[MAX_PATH+1];

// Write data to the ZIP output stream.
void
write_data(void *buff, int len) 
{
    while (len > 0) {
	int rc = fwrite(buff, 1, len, jarfp);
	if (rc <= 0) {
	    sprintf(message, "Write on output file failed err=%d\n",errno);
	    l_abort(message);
	}
	output_file_offset += rc;
	buff = ((char *)buff) + rc;
	len -= rc;
    }
}

static void
add_to_zip_directory(char *fname, int len){
    uint fname_length = strlen(fname);
    ushort header[500];

    if ((central_directory_ptr + 400) > central_directory_limit) {
	sprintf(message, "Ran out of ZIP central directory space\n \
                         after creating %d entries.\n", central_directory_count);
	l_abort(message);
    }
    
    header[0] = SWAP_BYTES(0x4B50);
    header[1] = SWAP_BYTES(0x0201);
    header[2] = SWAP_BYTES(0xA);
    // required version
    header[3] = SWAP_BYTES(0xA);
    // flags => not compressed
    header[4] = 0;
    // Compression method => none
    header[5] = 0;
    // Last modified date and time.
    header[6] = 0;
    header[7] = 0;
    // CRC
    header[8] = 0;
    header[9] = 0;

    // Compressed length:
    header[10] = SWAP_BYTES(len & 0xFFFF);
    header[11] = SWAP_BYTES((len >> 16) & 0xFFFF);
    // Uncompressed length.  Same as compressed length.,
    header[12] = SWAP_BYTES(len & 0xFFFF);
    header[13] = SWAP_BYTES((len >> 16) & 0xFFFF);

    // Filename length
    header[14] = SWAP_BYTES(fname_length);
    // So called "extra field" length.
    header[15] = 0;
    // So called "comment" length.
    header[16] = 0;
    // Disk number start
    header[17] = 0;
    // File flags => binary
    header[18] = 0;
    // More file flags
    header[19] = 0;
    header[20] = 0;
    // Offset within ZIP file.
    header[21] = SWAP_BYTES(output_file_offset & 0xFFFF);
    header[22] = SWAP_BYTES((output_file_offset >> 16) & 0xFFFF);
    // Copy the fname to the header.
    memcpy((char *)(header+23), fname, fname_length);

    // Copy the whole thing into the central directory.
    memcpy((char *)central_directory_ptr, (char *)header, 46+fname_length);
    central_directory_ptr += (46+fname_length);

    central_directory_count++;
}

static void
write_zip_header(char *fname, int len) {
    uint fname_length = strlen(fname);
    ushort header[500];

    // ZIP LOC magic.
    header[0] = SWAP_BYTES(0x4B50);
    header[1] = SWAP_BYTES(0x0403);
    
    // Version
    header[2] = SWAP_BYTES(0xA);
    // flags => not compressed
    header[3] = 0;
    // Compression method => none
    header[4] = 0;
    // Last modified date and time.
    header[5] = 0;
    header[6] = 0;
    // CRC
    header[7] = 0;
    header[8] = 0;
    // Compressed length:
    header[9] = SWAP_BYTES(len & 0xFFFF);
    header[10] = SWAP_BYTES((len >> 16) & 0xFFFF);
    // Uncompressed length.  Same as compressed length.,
    header[11] = SWAP_BYTES(len & 0xFFFF);
    header[12] = SWAP_BYTES((len >> 16) & 0xFFFF);
    // Filename length
    header[13] = SWAP_BYTES(fname_length);
    // So called "extra field" length.
    header[14] = 0;
    // Copy the fname to the header.
    memcpy((char *)(header+15), fname, fname_length);

    // Write the LOC header to the output file.
    write_data(header, 30 + fname_length);
}

static void
write_central_directory(){
    ushort header[25];
    uint directory_len = central_directory_ptr - central_directory;

    // Create the End of Central Directory structure.
    header[0] = SWAP_BYTES(0x4B50);
    header[1] = SWAP_BYTES(0x0605);
    // disk numbers
    header[2] = 0;
    header[3] = 0;
    // Number of entries in central directory.
    header[4] = SWAP_BYTES(central_directory_count);
    header[5] = SWAP_BYTES(central_directory_count);
    // Size of the central directory}
    header[6] = SWAP_BYTES(directory_len & 0xFFFF);
    header[7] = SWAP_BYTES((directory_len >> 16) & 0xFFFF);
    // Offset of central directory within disk. 
    header[8] = SWAP_BYTES(output_file_offset & 0xFFFF); 
    header[9] = SWAP_BYTES((output_file_offset >> 16) & 0xFFFF); 
    // zipfile comment length;
    header [10] = 0;
    

    // Write the central directory.
    //fprintf(stderr, "Central directory at %d\n", output_file_offset);
    write_data(central_directory, directory_len);

    // Write the End of Central Directory structure.
    //fprintf(stderr, "end-of-directory at %d\n", output_file_offset);
    write_data(header, 22);
}

// Public API

//Open a Temporary Jar file and initialize.
void
openJarFileWriter(char *fname) {
    if (!jarfp) {
      sprintf(ziptmpfile,"%s#ziptmp",fname);
    	jarfp = fopen(ziptmpfile, "wb");
    	if (!jarfp) {
            sprintf(message,"ERROR: Could not open jar file <%s>\n",fname);
    	    l_abort(message);
    	}
    }
}

// Add a ZIP entry and copy the file data

void
addJarEntry(char *fname, int len, char *data) {
  add_to_zip_directory(fname, len);
  write_zip_header(fname, len);
  write_data(data,len);

}

// Add a ZIP entry for a directory name with no data
void
addDirectoryToJarFile(char *dir_name) {
    add_to_zip_directory((char *)dir_name, 0);
    write_zip_header((char *)dir_name, 0);
}

// Write out the central directory and close the jar file.
void closeJarFileWriter(char *fname) {
    if (jarfp) {
      if(central_directory_count > 0) {
    	fflush(jarfp);
	write_central_directory();
	fflush(jarfp);
	fclose(jarfp);
	jarfp=NULL;
        remove(fname);
        rename(ziptmpfile,fname);
      } else {
        fclose(jarfp);
        fprintf(stderr,"Error: Archive not created. No files found, atleast one file must be specified\n");
        remove(ziptmpfile);
      }
    }
}

//Normalize the file name ie. replace back-slash with fwd-slash
//and remove the drive:\ specification so that its a relative path.
static char 
*xlate(char *fname, char *out) {

   char *s = out;

   char *p = strchr(fname,':');
   if (p != NULL) {
     p++; //skip over the :
     p++; //Skip over the next slash if any
   } else {
     p = fname;
   }
 
   while (*p != NULL) {
     
     *s = (*p == '\\') ? '/' : *p ;
      s++ ; p++;
   }
   *s = '\0';
   return out;
}

void do_write(char *flist, char *zipname) {
  openJarFileWriter(zipname);
  char fname[MAX_PATH+1];
  char zname[MAX_PATH+1];

  FILE *fp = fopen(flist,"r");
  if (fp == NULL) {
    sprintf(message,"Error:fopen: Could not open list file <%s>\n",flist); 
    l_abort(message);
  }

  while (fgets(fname,MAX_PATH+1,fp) > 0) {
    char *nl = strchr(fname,'\n');
    *nl='\0';
    struct stat st;
    if (stat(fname,&st) < 0) {
      perror("stat");
      sprintf(message,"Could not stat file <%s>\n",fname);
      l_abort(message);
    }
    char *z = xlate(fname, zname);
    if (st.st_mode & _S_IFREG) {
      int len = st.st_size;
      FILE *filefp = fopen(fname,"r");
      if (filefp == NULL) {
        perror("fopen");
        sprintf(message,"Error:Could not open input file <%s>\n",fname);
        l_abort(message);
      }
      char *fbuf = (char *)malloc(len+1);
      fread(fbuf,1,len,filefp);
      addJarEntry(zname, len, fbuf);
      if (fbuf) free(fbuf);
    } 
  }
  closeJarFileWriter(zipname);
}
