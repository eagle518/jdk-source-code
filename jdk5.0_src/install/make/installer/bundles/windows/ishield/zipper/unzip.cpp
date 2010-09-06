/*
 * @(#)unzip.cpp	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// Author: Kumar Srinivasan
 
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <stdlib.h>


#ifndef _MSC_VER 
#include <strings.h>
#endif

#include "zip.h"

FILE *zipfp;

void l_abort(char *);

static int remove_input_zip_file = FALSE; //FALSE

static char message[MAX_PATH*2];

static char 
*dirname(char *path) {
   char *dstr = strdup(path);

   char *p = dstr ;
   while ( (p = strchr(p,'\\')) != NULL) { 
     *p = '/';
      p++;
   }

  char *lastslash = strrchr(dstr, '/');
  if (lastslash != NULL) {
    *lastslash = '\0';
  } else {
    free(dstr) ;
    dstr = NULL ;
  }
  return dstr ;
}

static void 
mkdirs(char* path) {

  if (strlen(path) <= 0)  return;
  char dir[MAX_PATH];

  strcpy(dir, path);
  char* slash = strrchr(dir, '/');
  if (slash == 0)  return;
  *slash = 0;
  mkdirs(dir);
  mkdir(dir);
}


void l_abort(char *msg) {
  fprintf(stderr,"%s\n",msg);
#ifdef _DEBUG
  abort();
#else
  exit(-1);
#endif
}


/* For documentation purposes.
struct zip_header {
  ushort  magic0 ; 2 2
  ushort  magic1 ; 2 4
  ushort  vers; 2 6
  ushort  flags; 2 8
  ushort  compression; 2 10
  ushort  modtime;2 12
  ushort  moddate;2 14
  int     crc; 4 18
  int     clen;4  22
  int     ulen; 4 26 
  ushort  filenamelen; 2 28
  ushort  extrafieldlen; 2 30
};
*/

static void readAndWrite() {
  char filename[MAX_PATH];
  int rc;

  //Note: We have already read the magic number of 4 bytes.

  //Skip to compression if the file is compressed then abort.
  fseek(zipfp,4,SEEK_CUR);
  ushort compression;
  rc = fread(&compression,1,sizeof(ushort),zipfp);
  if (compression) l_abort("Error: Cannot unzip deflated file entries.\n");

  // Skip over to file length
  fseek(zipfp,12,SEEK_CUR); 
  int file_len;
  rc = fread(&file_len,1,sizeof(int),zipfp);

  ushort filenamelen;
  rc = fread(&filenamelen,1,sizeof(ushort),zipfp);

  ushort extrafieldlen;
  rc = fread(&extrafieldlen,1, sizeof(ushort), zipfp);
  
  rc = fread(filename,filenamelen,sizeof(char),zipfp);
  filename[filenamelen]='\0';
  if (rc <=0) {
    sprintf(message,"Error: Could not read filename <%s>  from zip header\n", filename);
    l_abort(message);
  }

  bool isDir = (filename[filenamelen-1] == '/') ? true : false;

  char *pathname = dirname(filename);
  if (pathname != NULL) {
    mkdirs(filename);
    free(pathname);
  }

  fprintf(stderr,"  extracting: %s",filename);
  
  FILE *filefp=NULL;

  if (!isDir) {
    filefp = fopen(filename,"wb+");
    if (filefp == NULL) {
      sprintf(message,"Error:fopen: while opening file <%s>\n",filename);
      l_abort(message);
    }
  }

  if (extrafieldlen > 0) {
    fseek(zipfp,extrafieldlen,SEEK_CUR);
  }

  if (!isDir) {
    //Read and write our file entry
    rc = 1;
    {
      char *buffer = (char *) malloc(file_len+1);
      rc = fread(buffer, 1, file_len, zipfp);
      int rc2 = fwrite(buffer,1,rc,filefp);
      if (buffer) free(buffer);
    }
    fclose(filefp);
  }
  fprintf(stderr,"\n");
}


bool isNext() {
  ushort magic[2];
  int rc;
  rc = fread(&magic[0],1,2,zipfp);
  rc = fread(&magic[1],1,2,zipfp);

  return ( (magic[0] == SWAP_BYTES(0x4B50)) && 
           (magic[1] == SWAP_BYTES(0x0403)) );
}



// Public API
//Open a Zip file and initialize.
void
openZipFileReader(char *fname) {
    if (!zipfp) {
    	zipfp = fopen(fname, "rb");
    	if (!zipfp) {
    	    perror("fopen");
            sprintf(message, "Could not open input zip file %s\n",fname);
    	    l_abort(message);
    	}
    }
}

//Close a Zip File Reader
void closeZipFileReader() {
    if (zipfp) {
    	fflush(zipfp);
	fclose(zipfp);
	zipfp=NULL;
    }
}

//Read the file and extract its contents to the
//to the directory of the zipfile.
void do_read(char *inputzip, char* outputdir)
{
  char *extract_dir = NULL;

  if (outputdir != NULL)
      extract_dir = dirname(outputdir);
  else
      extract_dir = dirname(inputzip);

  if ( (extract_dir != NULL) && (strlen(extract_dir) > 0) ) 
  {
     // ensure directory is created first
     char *dir = (char *) malloc(strlen(extract_dir) + 2);
     sprintf(dir, "%s/", extract_dir);
     mkdirs(dir);
     free(dir);

     chdir(extract_dir);
  }
  openZipFileReader(inputzip);
  fprintf(stderr,"Archive: %s\n",inputzip);

  bool next = isNext();

  // We must have atleast one entry
  if (!next) {  
    sprintf(message,"Error: no entries in zip file.\n");
    l_abort(message);
  }

  while (next) {
    readAndWrite();
    next = isNext();
  }

  closeZipFileReader();
  if (remove_input_zip_file) remove(inputzip);
}

//sets remove input file after completion
void set_remove_input_file() {
  remove_input_zip_file=1;
}
