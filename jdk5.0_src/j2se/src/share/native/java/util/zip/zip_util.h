/*
 * @(#)zip_util.h	1.29 03/06/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Prototypes for zip file support
 */

#ifndef _ZIP_H_
#define _ZIP_H_

/*
 * Header signatures
 */
#define LOCSIG 0x04034b50L	    /* "PK\003\004" */
#define EXTSIG 0x08074b50L	    /* "PK\007\008" */
#define CENSIG 0x02014b50L	    /* "PK\001\002" */
#define ENDSIG 0x06054b50L	    /* "PK\005\006" */

/*
 * Header sizes including signatures
 */
#ifdef USE_MMAP
#define SIGSIZ  4
#endif
#define LOCHDR 30
#define EXTHDR 16
#define CENHDR 46
#define ENDHDR 22

/*
 * Header field access macros
 */
#define CH(b, n) (((unsigned char *)(b))[n])
#define SH(b, n) (CH(b, n) | (CH(b, n+1) << 8))
#define LG(b, n) (SH(b, n) | (SH(b, n+2) << 16))
#define GETSIG(b) LG(b, 0)

/*
 * Macros for getting local file (LOC) header fields
 */
#define LOCVER(b) SH(b, 4)	    /* version needed to extract */
#define LOCFLG(b) SH(b, 6)	    /* general purpose bit flags */
#define LOCHOW(b) SH(b, 8)	    /* compression method */
#define LOCTIM(b) LG(b, 10)	    /* modification time */
#define LOCCRC(b) LG(b, 14)	    /* crc of uncompressed data */
#define LOCSIZ(b) LG(b, 18)	    /* compressed data size */
#define LOCLEN(b) LG(b, 22)	    /* uncompressed data size */
#define LOCNAM(b) SH(b, 26)	    /* filename length */
#define LOCEXT(b) SH(b, 28)	    /* extra field length */

/*
 * Macros for getting extra local (EXT) header fields
 */
#define EXTCRC(b) LG(b, 4)	    /* crc of uncompressed data */
#define EXTSIZ(b) LG(b, 8)	    /* compressed size */
#define EXTLEN(b) LG(b, 12)         /* uncompressed size */

/*
 * Macros for getting central directory header (CEN) fields
 */
#define CENVEM(b) SH(b, 4)	    /* version made by */
#define CENVER(b) SH(b, 6)	    /* version needed to extract */
#define CENFLG(b) SH(b, 8)	    /* general purpose bit flags */
#define CENHOW(b) SH(b, 10)	    /* compression method */
#define CENTIM(b) LG(b, 12)	    /* modification time */
#define CENCRC(b) LG(b, 16)	    /* crc of uncompressed data */
#define CENSIZ(b) LG(b, 20)	    /* compressed size */
#define CENLEN(b) LG(b, 24)	    /* uncompressed size */
#define CENNAM(b) SH(b, 28)	    /* length of filename */
#define CENEXT(b) SH(b, 30)	    /* length of extra field */
#define CENCOM(b) SH(b, 32)	    /* file comment length */
#define CENDSK(b) SH(b, 34)	    /* disk number start */
#define CENATT(b) SH(b, 36)	    /* internal file attributes */
#define CENATX(b) LG(b, 38)	    /* external file attributes */
#define CENOFF(b) LG(b, 42)	    /* offset of local header */

/*
 * Macros for getting end of central directory header (END) fields
 */
#define ENDSUB(b) SH(b, 8)	    /* number of entries on this disk */
#define ENDTOT(b) SH(b, 10)	    /* total number of entries */
#define ENDSIZ(b) LG(b, 12)	    /* central directory size */
#define ENDOFF(b) LG(b, 16)	    /* central directory offset */
#define ENDCOM(b) SH(b, 20)	    /* size of zip file comment */

/*
 * Supported compression methods
 */
#define STORED	    0
#define DEFLATED    8

/*
 * Support for reading ZIP/JAR files. Some things worth noting:
 *
 * - Zip files larger than 2**32 bytes are not supported.
 * - Zip file entries larger than 2**32 bytes are not supported.
 * - Maximum number of entries is 2**16-2.
 * - jzentry time and crc fields are signed even though they really
 *   represent unsigned quantities.
 * - If csize is zero then entry is uncompressed.
 * - If extra != 0 then the first two bytes are the length of the extra
 *   data in intel byte order.
 * - If pos is negative then is position of entry LOC header. It is set
 *   to position of entry data once it is first read.
 */

typedef struct jzentry {  /* Zip file entry */
    char *name;		  /* entry name */
    jlong time;		  /* modification time */
    jlong size;		  /* size of uncompressed data */
    jlong csize;	  /* size of compressed data (zero if uncompressed) */
    jint crc;		  /* crc of uncompressed data */
    char *comment;	  /* optional zip file comment */
    jbyte *extra;	  /* optional extra data */
    jlong pos;		  /* position of LOC header (if negative) or data */
} jzentry;

/*
 * In-memory hash table cell.
 * In a typical system we have a *lot* of these, as we have one for
 * every entry in every active JAR.
 * Note that in order to save space we don't keep the name in memory,
 * but merely remember a 32 bit hash.
 */
typedef struct jzcell {
    unsigned int pos;          	/* Offset of LOC within ZIP file */
    unsigned int hash;		/* 32 bit hashcode on name */
    unsigned short nelen;       /* length of name and extra data */
    unsigned short next;      	/* hash chain: index into jzfile->entries */
    unsigned int size;		/* Uncompressed size */
    unsigned int csize;		/* Compressed size */
    jint crc;
    unsigned short elen;        /* length of extra data in CEN */
    unsigned int cenpos;        /* Offset of file headers in CEN */
} jzcell;

/*
 * Descriptor for a ZIP file.
 */
typedef struct jzfile {   /* Zip file */
    char *name;	  	  /* zip file name */
    jint refs;		  /* number of active references */
#ifdef USE_MMAP
    unsigned char *maddr; /* beginning address of mapped file */
    jlong len;		  /* length (in bytes) of mapped file */
#else
    int fd;		  /* open file descriptor */
#endif
    void *lock;		  /* read lock */
    char *comment; 	  /* zip file comment */
    char *msg;		  /* zip error message */
    jzcell *entries;      /* array of hash cells */
    jint total;	  	  /* total number of entries */
    unsigned short *table;    /* Hash chain heads: indexes into entries */
    jint tablelen;	  /* number of hash heads */
    struct jzfile *next;  /* next zip file in search list */
    jzentry *cache;       /* we cache the most recently freed jzentry */
    /* Information on metadata names in META-INF directory */
    char **metanames;     /* array of meta names (may have null names) */
    jint metacount;	  /* number of slots in metanames array */
    /* If there are any per-entry comments, they are in the comments array */
    char **comments;
    jlong lastModified;   /* last modified time */
} jzfile;

/*
 * We impose arbitrary but reasonable limits on ZIP files.
 */
#define ZIP_MAXENTRIES (0x10000 - 2)

/* 
 * Typical size of entry name 
 */
#define ZIP_TYPNAMELEN 512 


/*
 * Index representing end of hash chain
 */
#define ZIP_ENDCHAIN 0xFFFF

jzentry * JNICALL
ZIP_FindEntry(jzfile *zip, char *name, jint *sizeP, jint *nameLenP);

jboolean JNICALL
ZIP_ReadEntry(jzfile *zip, jzentry *entry, unsigned char *buf, char *entrynm);

jzentry * JNICALL
ZIP_GetNextEntry(jzfile *zip, jint n);

jzfile * JNICALL
ZIP_Open(const char *name, char **pmsg);

jzfile *
ZIP_Open_Generic(const char *name, char **pmsg, int mode, jlong lastModified);

jzentry * ZIP_GetEntry(jzfile *zip, char *name, jint ulen); 
void ZIP_Close(jzfile *zip);
void ZIP_Lock(jzfile *zip);
void ZIP_Unlock(jzfile *zip);
jint ZIP_Read(jzfile *zip, jzentry *entry, jlong pos, void *buf, jint len);
void ZIP_FreeEntry(jzfile *zip, jzentry *ze);
#endif /* !_ZIP_H_ */ 
