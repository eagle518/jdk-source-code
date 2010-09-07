/*
 * @(#)util.c	1.22 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <sys/stat.h>
#include "util.h"
#include "system.h"
#include <fcntl.h>
#include "msgString.h"

static void ShowMessage(char* kind, char *msg) {
    char buffer[1024];
    strcpy(buffer, kind);
    strcat(buffer, msg);
    sysMessage(buffer);
}


/* Utility method that reads the contents of a file into a memory buffer. */
int ReadFileToBuffer(char* filename, char **buffer) {
    long size, nread;
    char *buf;
    FILE *fp;
 
    /* Find size of file */
    struct stat statBuf;
    
    /* stat and fopen must succeed */
    if (stat(filename,  &statBuf) != 0 || 
	(fp = fopen(filename, "rb")) == NULL) {
      *buffer = NULL;
      return 0;
    }

    size = statBuf.st_size;
    
    /* Allocate memory for contents */
    buf = (char *)malloc(size+1);

    nread = 0;
    nread = fread((void *)buf, 1, size, fp);        

    assert(nread <= size, getMsgString(MSG_READ_ERROR));

    fclose(fp);
    if (nread != size) {
        free(buf);
        *buffer = NULL;
        return 0;
    }
    
    buf[nread] = '\0';
    *buffer = buf;
    return nread;
}

/* Utility method that saves the contents of a memory buffer to file 
   Returns TRUE if succedded, otherwise FALSE
*/
int SaveBufferToFile(char* filename, char* s, int size) {    
    int r;
    FILE *fp;

    /* Open output file */
    fp = fopen(filename, "wb+");
    if (fp == NULL) return FALSE;
    
    r = fwrite((void *)s, 1, size, fp);
    fclose(fp);

    /* fix for 4685680 */
    if (r != size) {
        return FALSE;
    }
    
    return TRUE;                         
}

/* ------------------------------------------------------------------------- */
/* Error reporting methods */

void Abort(char *msg) {
    sysErrorExit(msg);
}

void Message(char *msg) {
    sysMessage(msg);
}

int isUTF8 (char *buffer, int size)
{
        unsigned char *buf = (unsigned char *) buffer;
        char encoding[25];

        /* See if we can figure out the character encoding used
        * in this file by peeking at the first few bytes.
        */
	switch (buf [0] & 0x0ff) {
            case 0:
              /* 00 3c 00 3f == illegal UTF-16 big-endian */
              if (buf [1] == 0x3c && buf [2] == 0x00 && buf [3] == 0x3f) {
		  strcpy (encoding, "UnicodeBig");
                  return 0;
              }
	      /* else it's probably UCS-4 */
	      break;

            case '<':      /* 0x3c: the most common cases! */
              switch (buf [1] & 0x0ff) {
                /* First character is '<'; could be XML without
		* an XML directive such as "<hello>", "<!-- ...",
		* and so on.
                */
                default:
                  break;

                /* 3c 00 3f 00 == illegal UTF-16 little endian */
                case 0x00:
                  if (buf [2] == 0x3f && buf [3] == 0x00) {
		      strcpy (encoding, "UnicodeLittle");
		      return 0;
                  }
		  /* else probably UCS-4 */
		  break;

                /* 3c 3f 78 6d == ASCII and supersets '<?xm' */
                case '?': 
                  if (buf [2] != 'x' || buf [3] != 'm')
		      break;
		  /*
		  * One of several encodings could be used:
                  * Shift-JIS, ASCII, UTF-8, ISO-8859-*, etc
		  */
		  return useEncodingDecl ((char *)buf, "UTF8", size, encoding, sizeof(encoding));
              }
	      break;

            /* 4c 6f a7 94 ... some EBCDIC code page */
            case 0x4c:
              if (buf [1] == 0x6f
		    && (0x0ff & buf [2]) == 0x0a7
		    && (0x0ff & buf [3]) == 0x094) {
		  return useEncodingDecl ((char *)buf, "CP037", size,  encoding, sizeof(encoding));
	      }
	      /* whoops, treat as UTF-8 */
	      break;

            /* UTF-16 big-endian */
            case 0xfe:
              if ((buf [1] & 0x0ff) != 0xff)
                  break;
	      strcpy (encoding, "UTF-16");
              return 0;

            /* UTF-16 little-endian */
            case 0xff:
              if ((buf [1] & 0x0ff) != 0xfe)
                  break;
	      strcpy (encoding, "UTF-16");
	      return 0;

            /* default ... no XML declaration */
            default:
              break;
        }

	/*
        * If all else fails, assume XML without a declaration, and
        * using UTF-8 encoding.
	*/
	strcpy (encoding, "UTF-8");
        return 1;
}

    /*
     * Read the encoding decl on the stream, knowing that it should
     * be readable using the specified encoding (basically, ASCII or
     * EBCDIC).  The body of the document may use a wider range of
     * characters than the XML/Text decl itself, so we switch to use
     * the specified encoding as soon as we can.  (ASCII is a subset
     * of UTF-8, ISO-8859-*, ISO-2022-JP, EUC-JP, and more; EBCDIC
     * has a variety of "code pages" that have these characters as
     * a common subset.)
     */
int  useEncodingDecl (char *buffer, char *encoding, int size, char *enc, int encSize)
{
        int i, j;
	char	tempbuf[64] ;
        char    keystr[64];
	char	*keyBuf = NULL;
	char	*key = NULL;
	char	quoteChar = 0;
	int	sawEq = 0;
	int	sawQuestion = 0;
        int     tempbuflen = 0;

	/*
	* Buffer up a bunch of input, and set up to read it in
	* the specified encoding ... we can skip the first four
	* bytes since we know that "<?xm" was read to determine
	* what encoding to use!
	*
	*
	* Next must be "l" (and whitespace) else we conclude
	* error and choose UTF-8.
	*/
	if (buffer[4] != 'l') {
	    strcpy (enc, "UTF-8");
	    return 1;
	}

	/*
	* Then, we'll skip any
	* 	S version="..." 	[or single quotes]
	* bit and get any subsequent 
	* 	S encoding="..." 	[or single quotes]
	*
	*/

        tempbuf[0] = '\0';
        keystr[0] = '\0';

	for (i = 5; i < size ; ++i) {
	    /* ignore whitespace before/between "key = 'value'" */
	    if (isWhitespace(buffer[i]))
		continue;

	    /* ... but require at least a little! */
	    if (i == 5)
		break;
	    
	    /* terminate the loop ASAP */
	    if (buffer[i] == '?')
		sawQuestion = 1;
	    else if (sawQuestion) {
		if (buffer[i] == '>')
		    break;
		sawQuestion = 0;
	    }
	    /* did we get the "key =" bit yet? */
	    if (key == NULL || !sawEq) {
		if (keyBuf == NULL) {
	            if (isWhitespace(buffer[i]))
			continue;
		    keyBuf = tempbuf;
		    tempbuf[0] =  buffer[i];
		    tempbuf[1] = '\0';
                    tempbuflen = 1;
		    sawEq = 0;
		} else if (isWhitespace (buffer[i])) {
                    keystr[0] = '\0';
                    strcpy(keystr, keyBuf);
		    key = keystr;
		} else if (buffer[i] == '=') {
		    if (key == NULL) {
                        strcpy(keystr, keyBuf);
		        key = keystr;
                    }
		    sawEq = 1;
		    keyBuf = NULL;
		    quoteChar = 0;
		} else {
		    if (tempbuflen < (sizeof(tempbuf)-1)) {
		        tempbuf[tempbuflen++] = buffer[i];
                    	tempbuf[tempbuflen] = '\0';
		    }
		    else {
                    	tempbuf[tempbuflen] = '\0';
			break;
		    }
	        }

		continue;
	    }

	    /* space before quoted value */
	    if (isWhitespace(buffer[i]))
		continue;
	    if (buffer[i] == '"' || buffer[i] == '\'') {
		if (quoteChar == 0) {
		    quoteChar = (char) buffer[i];
                    tempbuflen = 0;
                    tempbuf[tempbuflen] = '\0';
		    continue;
		} else if (buffer[i] == quoteChar) {
		    if ((strcmp(key, "encoding")) == 0) {

			/* [81] Encname ::= [A-Za-z] ([A-Za-z0-9._]|'-')* */
			for (j = 0; j < tempbuflen; j++) {
			    if ((tempbuf[j] >= 'A' && tempbuf[j] <= 'Z')
				    || (tempbuf[j] >= 'a' && tempbuf[j] <= 'z'))
				continue;
			    if (j == 0) {
	                        strcpy (enc, "UTF-8");
                                return 1;
                            }
			    if (j > 0 && (tempbuf[j] == '-'
				    || (tempbuf[j] >= '0' && tempbuf[j] <= '9')
				    || tempbuf[j] == '.' || tempbuf[j] == '_'))
				continue;
			    /* map illegal names to UTF-8 default */
	                    strcpy (enc, "UTF-8");
                            return 1;
			}

			if (strlen(tempbuf) >= encSize) {
			    return 0;
			}
			else {
			    strcpy (enc, tempbuf);
			}
			if ((strcmp(enc, "utf-8") == 0) || (strcmp(enc, "UTF-8")==0))
                            return 1;
                        else
                            return 0;

		    } else {
			key = NULL;
                        keystr[0] = '\0';
			continue;
		    }
		}
	    }
	    if (tempbuflen < (sizeof(tempbuf)-1)) {
	        tempbuf[tempbuflen++] = buffer[i];
                tempbuf[tempbuflen] = '\0';
	    }
	    else {
                tempbuf[tempbuflen] = '\0';
		break;
	    }
	}
	strcpy (enc, "UTF-8");
        return 1;
}
