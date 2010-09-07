/*
 * @(#)zip_util.c	1.75 04/01/28
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Support for reading ZIP/JAR files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>

#include "jni.h"
#include "jlong.h"
#include "jvm.h"
#include "zip_util.h"
#include "zlib.h"

#ifdef USE_MMAP
#include <sys/mman.h>
#endif

#define MAXREFS 0xFFFF	/* max number of open zip file references */
#define MAXSIZE ((jlong) 0xFFFFFFFF) /* max size of zip file or zip entry */

#define MCREATE()      JVM_RawMonitorCreate()
#define MLOCK(lock)    JVM_RawMonitorEnter(lock)
#define MUNLOCK(lock)  JVM_RawMonitorExit(lock)
#define MDESTROY(lock) JVM_RawMonitorDestroy(lock)

static jzfile *zfiles = 0;	/* currently open zip files */
static void *zfiles_lock = 0;

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

/*
 * Initialize zip file support. Return 0 if successful otherwise -1
 * if could not be initialized.
 */
jint InitializeZip()
{
    extern void out_of_memory(void);
    static jboolean inited = JNI_FALSE;
    if (inited)
        return 0;
    zfiles_lock = MCREATE();
    if (zfiles_lock == 0) {
	return -1;
    }
    inited = JNI_TRUE;

    return 0;
}

/*
 * Reads len bytes of data into buf. Returns 0 if all bytes could be read,
 * otherwise returns -1.
 */
static int readFully(int fd, void *buf, jlong len)
{
    char *bp = (char *) buf;

    while (len > 0) {
	jlong limit = ((((jlong) 1) << 31) - 1);
	jint count = (len < limit) ?
	    (jint) len :
	    (jint) limit;
	jint n = JVM_Read(fd, bp, count);
	if (n > 0) {
	    bp += n;
	    len -= n;
	} else if (n == JVM_IO_ERR && errno == EINTR) {
	    continue;	/* Retry after EINTR */
	} else {	/* EOF or IO error */
	    return -1;
	}
    }
    return 0;
}

/*
 * Allocates a new zip file object for the specified file name.
 * Returns the zip file object or NULL if not enough memory.
 */
jzfile *allocZip(const char *name)
{
    jzfile *zip = calloc(1, sizeof(jzfile));

    if (zip == 0) {
	return 0;
    }
    zip->name = strdup(name);
    if (zip->name == 0) {
	free(zip);
	return 0;
    }
    zip->lock = MCREATE();
    if (zip->lock == 0) {
	free(zip->name);
	free(zip);
	return 0;
    }
    return zip;
}

/*
 * Frees the specified zip file object.
 */
static void freeZip(jzfile *zip)
{
    /* First free any cached jzentry */
    ZIP_FreeEntry(zip,0);
    if (zip->name != 0) {
	free(zip->name);
    }
    if (zip->lock != 0) {
	MDESTROY(zip->lock);
    }
    if (zip->comment != 0) {
	free(zip->comment);
    }
    if (zip->entries != 0) {
	free(zip->entries);
    }
    if (zip->table != 0) {
	free(zip->table);
    }
    if (zip->metanames != 0) {
	jint i;
	for (i = 0; i < zip->metacount; i++) {
	    if (zip->metanames[i]) {
	        free(zip->metanames[i]);
	    }
	}
	free(zip->metanames);
    }
    if (zip->comments != 0) {
	jint i;
	for (i = 0; i < zip->total; i++) {
	    if (zip->comments[i]) {
	        free(zip->comments[i]);
	    }
	}
	free(zip->comments);
    }
    free(zip);
}

/* The END header is followed by a variable length comment of size < 64k. */
static const jlong END_MAXLEN = 0xFFFF + ENDHDR;

#ifdef USE_MMAP
/*
 * Searches for end of central directory (END) header. An offset to
 * the END header will be returned. 
 */
static jlong findEND(jzfile *zip, unsigned char **endbuf)
{
    unsigned char *mark;
    unsigned char *endaddr = zip->maddr + zip->len;
    unsigned char *cp;

    /*
     * Find end of central directory (END) header in zip file and set the file
     * pointer to start of header.
     * Set limit on how far back we need to search. The END header must be
     * located within the last END_MAXLEN bytes of the file.
     */
    if (zip->len >= END_MAXLEN)
	mark = endaddr - END_MAXLEN;
    else
	mark = zip->maddr;

    /*
     * Search backwards from the end of the file stopping when the END header
     * signature has been found.
     */
    for (cp = endaddr - ENDHDR; cp >= mark; cp--) {
	if ((cp[0] == (ENDSIG & 0xFF))
	    && (GETSIG(cp) == ENDSIG)
	    && (cp + ENDHDR + ENDCOM(cp) == endaddr))
	{
	    *endbuf = cp;
	    return cp - zip->maddr;
	}
    }
    return 0;
}
#else
/*
 * Searches for end of central directory (END) header. The contents of
 * the END header will be read and placed in endbuf. Returns the file
 * position of the END header, otherwise returns 0 if the END header
 * was not found or -1 if an error occurred.
 */
static jlong findEND(jzfile *zip, void *endbuf)
{
    unsigned char buf[ENDHDR * 2];
    jlong len, pos;
    int fd = zip->fd;

    /* Get the length of the zip file */
    len = JVM_Lseek(fd, 0, SEEK_END);
    if (len == -1) {
	return -1;
    }
    /*
     * Search backwards ENDHDR bytes at a time from end of file stopping
     * when the END header has been found. We need to make sure that we
     * handle the case where the END header may straddle a block boundary.
     */

    memset(buf, '\0', ENDHDR);
    
    for (pos = len - ENDHDR;
	 (pos + ENDHDR + END_MAXLEN >= len) && (pos >= 0);
	 pos -= ENDHDR) {
	unsigned char *bp;

	/* Shift previous block */
	memcpy(buf + ENDHDR, buf, ENDHDR);

	if (JVM_Lseek(fd, pos, SEEK_SET) == -1)
	    return -1;
	if (readFully(fd, buf, ENDHDR) == -1)
	    return -1;

	/* Now scan the block for END header signature */
	for (bp = buf; bp < buf + ENDHDR; bp++) {
	    if (GETSIG(bp) == ENDSIG) {
		/* Check for possible END header */
		jlong endpos = pos + (bp - buf);
		jint clen = ENDCOM(bp);
		if (endpos + ENDHDR + clen == len) {
		    /* Found END header */
		    memcpy(endbuf, bp, ENDHDR);
		    if (JVM_Lseek(fd, endpos + ENDHDR, SEEK_SET) == -1) {
			return -1;
		    }
		    if (clen > 0) {
			zip->comment = malloc(clen + 1);
			if (zip->comment == 0) {
			    return -1;
			}
			if (readFully(zip->fd, zip->comment, clen) == -1) {
			    free(zip->comment);
			    zip->comment = 0;
			    return -1;
			}
			zip->comment[clen] = '\0';
		    }
		    return endpos;
		}
	    }
	}
    }
    return 0; /* END header not found */
}
#endif

/*
 * Returns a hash code value for the specified string.
 */
static unsigned int
hash(const char *s)
{
    int h = 0;
    while (*s != '\0') {
	h = 31*h + *s++;
    }
    return h;
}

static unsigned int
hash_append(unsigned int hash, char c)
{
    return ((int)hash)*31 + c;
}


/*
 * Returns true if the specified entry's name begins with the string
 * "META-INF/" irrespect of case.
 */
static int
isMetaName(char *name)
{
#define META_INF "META-INF/"
    char *s = META_INF, *t = name;
    while (*s != '\0') {
	if (*s++ != (char)toupper(*t++)) {
	    return 0;
	}
    }
    return 1;
}

static void
addMetaName(jzfile *zip, char *name)
{
    int i;
    if (zip->metanames == 0) {
	zip->metacount = 2;
	zip->metanames = calloc(zip->metacount, sizeof(char *));
    }
    for (i = 0; i < zip->metacount; i++) {
	if (zip->metanames[i] == 0) {
	    zip->metanames[i] = strdup(name);
	    break;
	}
    }
    /* If necessary, grow the metanames array */
    if (i >= zip->metacount) {
	int new_count = 2 * zip->metacount;
	char **tmp = calloc(new_count, sizeof(char *));
	for (i = 0; i < zip->metacount; i++) {
	    tmp[i] = zip->metanames[i];
	}
	tmp[i] = strdup(name);
	free(zip->metanames);
	zip->metanames = tmp;
	zip->metacount = new_count;
    }
}

static void
addEntryComment(jzfile *zip, int index, char *comment)
{
    if (zip->comments == NULL) {
	zip->comments = calloc(zip->total, sizeof(char *));
    }
    zip->comments[index] = comment;
}

/*
 * Reads zip file central directory. Returns the file position of first
 * CEN header, otherwise returns 0 if central directory not found or -1
 * if an error occurred. If zip->msg != NULL then the error was a zip
 * format error and zip->msg has the error text.
 */
static jlong readCEN(jzfile *zip)
{
    /* Following are unsigned 32-bit */
    jlong endpos, locpos, cenpos, cenoff, cenlen;
    /* Following are unsigned 16-bit */
    jint total, count, tablelen, i, tmplen;
#ifdef USE_MMAP
    unsigned char *endbuf, *cenbuf, *cp;
#else
    unsigned char endbuf[ENDHDR], *cenbuf, *cp;
#endif
    jzcell *entries;
    unsigned short *table;
    char namebuf[ZIP_TYPNAMELEN + 1];
    char* name = namebuf;
    int namelen = ZIP_TYPNAMELEN + 1;


    /* Clear previous zip error */
    zip->msg = 0;
    /* Get position of END header */
#ifdef USE_MMAP
    endpos = findEND(zip, &endbuf);
#else
    endpos = findEND(zip, endbuf);
#endif
    if (endpos == 0) {
	return 0;  /* END header not found */
    }
#ifndef USE_MMAP
    if (endpos == -1) {
	return -1; /* system error */
    }
#endif
    /* Get position and length of central directory */
    cenlen = ENDSIZ(endbuf);
    if (cenlen < 0 || cenlen > endpos) {
	zip->msg = "invalid END header (bad central directory size)";
	return -1;
    }
    cenpos = endpos - cenlen;
    /*
     * Get position of first local file (LOC) header, taking into
     * account that there may be a stub prefixed to the zip file.
     */ 
    cenoff = ENDOFF(endbuf);
    if (cenoff < 0 || cenoff > cenpos) {
	zip->msg = "invalid END header (bad central directory offset)";
	return -1;
    }
    locpos = cenpos - cenoff;
    /* Get total number of central directory entries */
    total = zip->total = ENDTOT(endbuf);
    if (total < 0 || total * CENHDR > cenlen) {
	zip->msg = "invalid END header (bad entry count)";
	return -1;
    }
    if (total > ZIP_MAXENTRIES) {
	zip->msg = "too many entries in ZIP file";
	return -1;
    }
#ifdef USE_MMAP
    cenbuf = zip->maddr + cenpos;
#else
    /* Seek to first CEN header */
    if (JVM_Lseek(zip->fd, cenpos, SEEK_SET) == -1) {
	return -1;
    }

    /* Allocate temporary buffer for central directory bytes */
    cenbuf = malloc((size_t) cenlen);
    if (cenbuf == 0) {
	return -1;
    }
    /* Read central directory */
    if (readFully(zip->fd, cenbuf, cenlen) == -1) {
	free(cenbuf);
	return -1;
    }
#endif
    /* Allocate array for item descriptors */
    entries = zip->entries = calloc(total, sizeof(jzcell));
    if (entries == 0) {
#ifndef USE_MMAP
	free(cenbuf);
#endif
	return -1;
    }
    /* Allocate hash table */
    tmplen = total/2;
    tablelen = zip->tablelen = (tmplen > 0 ? tmplen : 1);
    table = zip->table = calloc(tablelen, sizeof(unsigned short));
    if (table == 0) {
#ifndef USE_MMAP
	free(cenbuf);
#endif
	free(entries);
	zip->entries = 0;
	return -1;
    }
    for (i = 0; i < tablelen; i++) {
	table[i] = ZIP_ENDCHAIN;
    }

    /* Now read the zip file entries */
    for (count = 0, cp = cenbuf; count < total; count++) {
	jzcell *zc = &entries[count];
	/* Following are unsigned 16-bit */
	jint method, nlen, clen, elen;
	unsigned int hsh;

	/* Check CEN header looks OK */
	if ((cp - cenbuf) + CENHDR > cenlen) {
	    zip->msg = "invalid CEN header (bad header size)";
	    break;
	}
	/* Verify CEN header signature */
	if (GETSIG(cp) != CENSIG) {
	    zip->msg = "invalid CEN header (bad signature)";
	    break;
	}
	/* Check if entry is encrypted */
	if ((CENVER(cp) & 1) == 1) {
	    zip->msg = "invalid CEN header (encrypted entry)";
	    break;
	}
	method = CENHOW(cp);
	if (method != STORED && method != DEFLATED) {
	    zip->msg = "invalid CEN header (bad compression method)";
	    break;
	}

	/* Get header field lengths */
	nlen         = CENNAM(cp);
	elen         = CENEXT(cp);
	clen         = CENCOM(cp);
	if ((cp - cenbuf) + CENHDR + nlen + clen + elen > cenlen) {
	    zip->msg = "invalid CEN header (bad header size)";
	    break;
	}

	zc->size     = CENLEN(cp);
	zc->csize    = CENSIZ(cp);
        zc->crc      = CENCRC(cp);
	/* Set compressed size to zero if entry uncompressed */
	if (method == STORED) {
	    zc->csize = 0;
	}

	/*
         * Copy the name into a temporary location so we can null
         * terminate it (sigh) as various functions expect this.
         */
        if (namelen < nlen + 1) { /* grow temp buffer */
            do  
                namelen = namelen * 2;
            while (namelen < nlen + 1);
            if (name != namebuf) /* free malloc()ated buffer */
                free(name);
            name = malloc(namelen);
        } 
	memcpy(name, cp+CENHDR, nlen);
        name[nlen] = 0;

	/*
         * Record the LOC offset and the name hash in our hash cell.
         */
	zc->pos = CENOFF(cp) + locpos;
	zc->nelen = nlen + elen;
	zc->hash = hash(name);
     	zc->cenpos = cenpos + (cp - cenbuf);
	zc->elen = elen;
	/*
	 * if the entry is metadata add it to our metadata names
         */
	if (isMetaName(name)) {
	    addMetaName(zip, name);
	}

	/*
         * If there is a comment add it to our comments array.
         */
	if (clen > 0) {
	    char *comment = malloc(clen+1);
	    memcpy(comment, cp+CENHDR+nlen+elen, clen);
            comment[clen] = 0;
	    addEntryComment(zip, count, comment);
 	}

	/*
         * Finally we can add the entry to the hash table
         */
	hsh = zc->hash % tablelen;
	zc->next = table[hsh];
	table[hsh] = count;

	cp += (CENHDR + nlen + elen + clen);
    }
    /* Free up temporary buffers */
#ifndef USE_MMAP
    free(cenbuf);
#endif
    if (name != namebuf)
        free(name);

    /* Check for error */
    if (count != total) {
	/* Central directory was invalid, so free up entries and return */
	free(entries);
	zip->entries = 0;
	free(table);
	zip->table = 0;
	return -1;
    }
    return cenpos;
}

/*
 * Map a file read-only.
 */
#ifdef USE_MMAP
static void *
mapFdReadOnly(size_t requestedSize, int Fd)
{
    void * mappedAddr;

    if ((mappedAddr = mmap(0, requestedSize, PROT_READ,
                           MAP_SHARED, Fd, 0)) == MAP_FAILED) {
        return 0;
    }

    return mappedAddr;
}
#endif

/*
 * Opens a zip file with the specified mode. Returns the jzfile object 
 * or NULL if an error occurred. If a zip error occurred then *pmsg will 
 * be set to the error message text if pmsg != 0. Otherwise, *pmsg will be
 * set to NULL.
 */
jzfile *
ZIP_Open_Generic(const char *name, char **pmsg, int mode, jlong lastModified)
{
    static char errbuf[256];
    char buf[PATH_MAX];
    jzfile *zip;

    if (InitializeZip()) {
        return NULL;
    }

    /* Clear zip error message */
    if (pmsg != 0) {
	*pmsg = NULL;
    }

    if (strlen(name) >= PATH_MAX) {
        if (pmsg) {
            *pmsg = "zip file name too long";
        }
        return NULL;
    }
    strcpy(buf, name);
    JVM_NativePath(buf);
    name = buf;

    MLOCK(zfiles_lock);
    for (zip = zfiles; zip != 0; zip = zip->next) {
        if (strcmp(name, zip->name) == 0
            && (zip->lastModified == lastModified || zip->lastModified == 0)
            && zip->refs < MAXREFS) {
	    zip->refs++;
	    break;
	}
    }
    MUNLOCK(zfiles_lock);
    if (zip == 0) {
	jlong len;
#ifdef USE_MMAP
	int fd;
#endif
	/* If not found then allocate a new zip object */
	zip = allocZip(name);
	if (zip == 0) {
	    return NULL;
	}
	zip->refs = 1;
        zip->lastModified = lastModified;
#ifdef USE_MMAP
	fd = JVM_Open(name, mode, 0);
	if (fd == -1) {
            if (pmsg != 0 && JVM_GetLastErrorString(errbuf, 256) > 0)
		*pmsg = errbuf;
	    freeZip(zip);
	    return NULL;
	}

	len = JVM_Lseek(fd, 0, SEEK_END);
	if (len == -1) {
            JVM_Close(fd);
	    freeZip(zip);
	    return NULL;
	}
	zip->len = len;
	zip->maddr = mapFdReadOnly(len, fd);
	if (zip->maddr == 0) {
#ifdef DEBUG
	    jio_fprintf(stderr, "Zip Error: %s: %s\n", name, strerror(errno));
#endif
            JVM_Close(fd);
            freeZip(zip);
	    return NULL;
	}
	if (zip->len > MAXSIZE) {
	    if (pmsg != 0) {
		*pmsg = "zip file too large";
	    }
            munmap((char *)zip->maddr, zip->len);
            JVM_Close(fd);
            freeZip(zip);
	    return NULL;
	}
	JVM_Close(fd);
#else
	zip->fd = JVM_Open(name, mode, 0);
	if (zip->fd == -1) {
            if (pmsg != 0 && JVM_GetLastErrorString(errbuf, 256) > 0)
		*pmsg = errbuf;
	    freeZip(zip);
	    return NULL;
	}
	len = JVM_Lseek(zip->fd, 0, SEEK_END);
	if (len == -1) {
            if (pmsg != 0) {
                char* buf = errbuf;
                if (JVM_GetLastErrorString(buf, 256) > 0) {
                    *pmsg = buf;
                }
            }
            JVM_Close(zip->fd);
	    freeZip(zip);
	    return NULL;
	}
	if (len > MAXSIZE) {
	    if (pmsg != 0) {
		*pmsg = "zip file too large";
	    }
            JVM_Close(zip->fd);
	    freeZip(zip);
	    return NULL;
	}
#endif
	if (readCEN(zip) <= 0) {
	    /* An error occurred while trying to read the zip file */
	    if (pmsg != 0) {
		/* Set the zip error message */
		*pmsg = zip->msg;
	    }
#ifdef USE_MMAP
    munmap((char *)zip->maddr, zip->len);
#else
    JVM_Close(zip->fd);
#endif
	    freeZip(zip);
	    return NULL;
	}
	MLOCK(zfiles_lock);
	zip->next = zfiles;
	zfiles = zip;
	MUNLOCK(zfiles_lock);
    }
    return zip;
}

/*
 * Opens a zip file for reading. Returns the jzfile object or NULL
 * if an error occurred. If a zip error occurred then *msg will be
 * set to the error message text if msg != 0. Otherwise, *msg will be
 * set to NULL.
 */
jzfile * JNICALL
ZIP_Open(const char *name, char **pmsg)
{
    return ZIP_Open_Generic(name, pmsg, O_RDONLY, 0);
}

/*
 * Closes the specified zip file object.
 */
void ZIP_Close(jzfile *zip)
{
    MLOCK(zfiles_lock);
    if (--zip->refs > 0) {
	/* Still more references so just return */
	MUNLOCK(zfiles_lock);
	return;
    }
    /* No other references so close the file and remove from list */
    if (zfiles == zip) {
	zfiles = zfiles->next;
    } else {
	jzfile *zp;
	for (zp = zfiles; zp->next != 0; zp = zp->next) {
	    if (zp->next == zip) {
		zp->next = zip->next;
		break;
	    }
	}
    }
    MUNLOCK(zfiles_lock);
#ifdef USE_MMAP
    munmap((char *)zip->maddr, zip->len);
    zip->maddr = 0;
#else
    JVM_Close(zip->fd);
#endif
    freeZip(zip);
    return;
}

/*
 * Read a LOC corresponding to a given hash cell and
 * create a corresponding jzentry entry descriptor
 * The ZIP lock should be held here.
 */
static jzentry *
readLOC(jzfile *zip, jzcell *zc)
{
    jint nlen, elen;  /* unsigned 16-bit */
    jzentry *ze = NULL;
    jlong start, end; /* unsigned 32-bit */

#ifdef USE_MMAP
    unsigned char *locbuf = zip->maddr + zc->pos;
#else
    unsigned char locbuf[LOCHDR];

    /* Seek to beginning of LOC header */
    if (JVM_Lseek(zip->fd, zc->pos, SEEK_SET) == -1) {
	zip->msg = "seek failed";
	return NULL;
    }

    /* Try to read in the LOC header */
    if (readFully(zip->fd, locbuf, LOCHDR) == -1) {
	zip->msg = "couldn't read LOC header";
        goto FREE_AND_RETURN_NULL;
    }
#endif

    /* Verify signature */
    if (GETSIG(locbuf) != LOCSIG) {
	zip->msg = "invalid LOC header (bad signature)";
        goto FREE_AND_RETURN_NULL;
    }

    /* nlen is name length in LOC */
    nlen = LOCNAM(locbuf);
    if (nlen < 0) {
        zip->msg = "invalid LOC header (bad name length)";
        goto FREE_AND_RETURN_NULL;
    }

    /* elen is extra data length in LOC */
    elen = LOCEXT(locbuf);
    if (elen < 0) {
        zip->msg = "invalid LOC header (bad extra data length)";
        goto FREE_AND_RETURN_NULL;
    }

    /* Allocate the entry to return */
    ze = calloc(1, sizeof(jzentry));
    if (ze == NULL) {
        zip->msg = "out of memory";
        goto FREE_AND_RETURN_NULL;
    }
    /* Allocate the entry name field */
    ze->name = malloc(nlen + 1);
    if (ze->name == NULL) {
        zip->msg = "out of memory";
        goto FREE_AND_RETURN_NULL;
    }

#ifdef USE_MMAP
    start = zc->pos + LOCHDR;
    end = start + nlen;
    if (start < 0 || start > end || end > zip->len) {
       zip->msg = "Corrupt zip file: invalid LOC headers";
       goto FREE_AND_RETURN_NULL;
    }
    memcpy(ze->name, zip->maddr + start, nlen);
#else
    /* Read in the entry name and zero terminate it */
    if (readFully(zip->fd, ze->name, nlen) == -1) {
	zip->msg = "couldn't read name";
        goto FREE_AND_RETURN_NULL;
    }
#endif
    ze->name[nlen] = 0;

    /* If extra in CEN, use it instead of extra in LOC */
    if (zc->elen > 0) {
        jlong off = CENHDR + zc->nelen - zc->elen + zc->cenpos;
        elen = zc->elen;
	ze->extra = malloc(elen+2);
        if (ze->extra == NULL) {
            zip->msg = "out of memory";
            goto FREE_AND_RETURN_NULL;
        }
	ze->extra[0] = (unsigned char)elen;
	ze->extra[1] = (unsigned char)(elen >> 8);

#ifdef USE_MMAP
        start = off;
        end = start + elen;
        if (start < 0 || start > end || end > zip->len) {
            zip->msg = "Corrupt zip file: invalid LOC headers";
            goto FREE_AND_RETURN_NULL;
        }
        memcpy(&ze->extra[2], zip->maddr + start, elen);
#else 
	/* Seek to begin of CEN header extra field */
	if (JVM_Lseek(zip->fd, off, SEEK_SET) == -1) {
	    zip->msg = "seek failed";
            goto FREE_AND_RETURN_NULL;
	}
	/* Try to read in the CEN Extra */
	if (readFully(zip->fd, &ze->extra[2], elen) == -1) {
	    zip->msg = "couldn't read CEN extra";
            goto FREE_AND_RETURN_NULL;
	}
#endif 
    }

    else if (LOCEXT(locbuf) != 0) {
        /* Allocate space for extra data plus two bytes for length */
	ze->extra = malloc(elen + 2);
        if (ze->extra == NULL) {
            zip->msg = "out of memory";
            goto FREE_AND_RETURN_NULL;
        }
	/* Store the extra data size in the first two bytes */
	ze->extra[0] = (unsigned char)elen;
	ze->extra[1] = (unsigned char)(elen >> 8);

#ifdef USE_MMAP
        start = zc->pos + LOCHDR + nlen;
        end = start + elen;
        if (start < 0 || start > end || end > zip->len) {
            zip->msg = "Corrupt zip file: invalid LOC headers";
            goto FREE_AND_RETURN_NULL;
        }
        memcpy(&ze->extra[2], zip->maddr + start, elen);
#else 
       	/* Try to read in the extra data */
	if (readFully(zip->fd, &ze->extra[2], elen) == -1) {
	    zip->msg = "couldn't read extra";
            goto FREE_AND_RETURN_NULL;
	}
#endif 
    }

    /*
     * Process any comment (this should be very rare)
     */
    if (zip->comments) {
	ze->comment = zip->comments[zc - zip->entries];
    }
    /*
     * We'd like to initialize the sizes from the LOC, but unfortunately
     * some ZIPs, including the jar command, don't put them there.
     * So we have to store them in the jzcell.
     */
    ze->size = zc->size;
    ze->csize = zc->csize;
    ze->crc = zc->crc;

    /* Fill in the rest of the entry fields from the LOC */
    ze->time = LOCTIM(locbuf);
    ze->pos = zc->pos + LOCHDR + LOCNAM(locbuf) + LOCEXT(locbuf);

    return ze;

 FREE_AND_RETURN_NULL:
    if (ze != NULL) {
	if (ze->extra != NULL)
	    free(ze->extra);
	if (ze->name != NULL)
	    free(ze->name);
        free(ze);
    }
    return NULL;
}

/*
 * Free the given jzentry.
 * In fact we maintain a one-entry cache of the most recently used
 * jzentry for each zip.  This optimizes a common access pattern.
 */

void
ZIP_FreeEntry(jzfile *jz, jzentry *ze)
{
    jzentry *last;
    ZIP_Lock(jz);
    last = jz->cache;
    jz->cache = ze;
    if (last != NULL) {
        /* Free the previously cached jzentry */
        if (last->extra) {
	    free(last->extra);
        }
        if (last->name) {
	    free(last->name);
        }
        free(last);
    }
    ZIP_Unlock(jz);
}

/*
 * Returns the zip entry corresponding to the specified name, or
 * NULL if not found.
 */
jzentry * ZIP_GetEntry(jzfile *zip, char *name, jint ulen) 
{
    unsigned int hsh = hash(name);
    int idx = zip->table[hsh % zip->tablelen];
    jzentry *ze;

    ZIP_Lock(zip);

    /*
     * This while loop is an optimization where a double lookup
     * for name and name+/ is being performed. The name char
     * array has enough room at the end to try again with a
     * slash appended if the first table lookup does not succeed.
     */
    while(1) {

        /* Check the cached entry first */
        ze = zip->cache;
        if (ze && strcmp(ze->name,name) == 0) {
            /* Cache hit!  Remove and return the cached entry. */
            zip->cache = 0;
            ZIP_Unlock(zip);
            return ze;
        }
        ze = 0;

        /*
         * Search down the target hash chain for a cell who's
         * 32 bit hash matches the hashed name.
         */
        while (idx != ZIP_ENDCHAIN) {
            jzcell *zc = &zip->entries[idx];

            if (zc->hash == hsh) {
                /*
                 * OK, we've found a ZIP entry whose 32 bit hashcode
                 * matches the name we're looking for.  Try to read its
                 * entry information from the LOC.
                 * If the LOC name matches the name we're looking,
                 * we're done.  
                 * If the names don't (which should be very rare) we
                 * keep searching.
                 */
                ze = readLOC(zip, zc);
                if (ze && strcmp(ze->name, name)==0) {
                    break;
                }
                if (ze != 0) {
                    /* We need to release the lock across the free call */
                    ZIP_Unlock(zip);
                    ZIP_FreeEntry(zip, ze);
                    ZIP_Lock(zip);
                }
                ze = 0;
            }
            idx = zc->next;
        }

        /* Entry found, return it */
        if (ze != 0) {
            break;
        }
 
        /* If no real length was passed in, we are done */
        if (ulen == 0) {
            break;
        }

        /* Slash is already there? */
        if (name[ulen-1] == '/') {
            break;
        }

        /* Add slash and try once more */
        name[ulen] = '/';
        name[ulen+1] = '\0';
        hsh = hash_append(hsh, '/');
        idx = zip->table[hsh % zip->tablelen];
        ulen = 0;
    }

    ZIP_Unlock(zip);
    return ze;
}

/*
 * Returns the n'th (starting at zero) zip file entry, or NULL if the
 * specified index was out of range.
 */
jzentry * JNICALL
ZIP_GetNextEntry(jzfile *zip, jint n)
{
    jzentry *result;
    if (n < 0 || n >= zip->total) {
	return 0;
    }
    ZIP_Lock(zip);
    result = readLOC(zip, &zip->entries[n]);
    ZIP_Unlock(zip);
    return result;
}

/*
 * Locks the specified zip file for reading.
 */
void ZIP_Lock(jzfile *zip)
{
    MLOCK(zip->lock);
}

/*
 * Unlocks the specified zip file.
 */
void ZIP_Unlock(jzfile *zip)
{
    MUNLOCK(zip->lock);
}


/*
 * Reads bytes from the specified zip entry. Assumes that the zip
 * file had been previously locked with ZIP_Lock(). Returns the
 * number of bytes read, or -1 if an error occurred. If zip->msg != 0
 * then a zip error occurred and zip->msg contains the error text.
 */
jint ZIP_Read(jzfile *zip, jzentry *entry, jlong pos, void *buf, jint len)
{
    jlong entry_size = (entry->csize != 0) ? entry->csize : entry->size;
    jlong start;

    /* Clear previous zip error */
    zip->msg = 0;

    /* Check specified position */
    if (pos < 0 || pos > entry_size - 1) {
	zip->msg = "ZIP_Read: specified offset out of range";
	return -1;
    }

    /* Check specified length */
    if (len <= 0)
	return 0;
    if (len > entry_size - pos)
	len = entry_size - pos;

    start = entry->pos + pos;
    assert(start >= 0);
    
#ifdef USE_MMAP
    if (start + len > zip->len) {
	zip->msg = "ZIP_Read: corrupt zip file: invalid entry size";
	return -1;
    }
    memcpy(buf, zip->maddr + start, len);
#else
    /* Seek to beginning of entry data and read bytes */
    if (JVM_Lseek(zip->fd, start, SEEK_SET) == JVM_IO_ERR) {
	zip->msg = "ZIP_Read: JVM_Lseek failed";
	return -1;
    }
    if (readFully(zip->fd, buf, len) == -1) {
	zip->msg = "ZIP_Read: error reading zip file";
	return -1;
    }
#endif

    return len;
}


/* The maximum size of a stack-allocated buffer.
 */
#define BUF_SIZE 4096

/*
 * This function is used by the runtime system to load compressed entries
 * from ZIP/JAR files specified in the class path. It is defined here
 * so that it can be dynamically loaded by the runtime if the zip library
 * is found.
 */
jboolean
InflateFully(jzfile *zip, jzentry *entry, void *buf, char **msg)
{
    z_stream strm;
    char tmp[BUF_SIZE];
    jlong pos = 0;
    jlong count = entry->csize;
    jboolean status;

    *msg = 0; /* Reset error message */

    if (count == 0) {
	*msg = "inflateFully: entry not compressed";
	return JNI_FALSE;
    }

    memset(&strm, 0, sizeof(z_stream));
    if (inflateInit2(&strm, -MAX_WBITS) != Z_OK) {
        *msg = strm.msg;
        return JNI_FALSE;
    }

    strm.next_out = buf;
    strm.avail_out = entry->size;

    while (count > 0) {
	jint n = count > (jlong)sizeof(tmp) ? (jint)sizeof(tmp) : count;
	ZIP_Lock(zip);
	n = ZIP_Read(zip, entry, pos, tmp, n);
	ZIP_Unlock(zip);
	if (n <= 0) {
            if (n == 0) {
                *msg = "inflateFully: Unexpected end of file";
            }
            inflateEnd(&strm);
	    return JNI_FALSE;
	}
	pos += n;
	count -= n;
	strm.next_in = (Bytef *)tmp;
	strm.avail_in = n;
	do {
	    switch (inflate(&strm, Z_PARTIAL_FLUSH)) {
	    case Z_OK:
		break;
	    case Z_STREAM_END:
		if (count != 0 || strm.total_out != entry->size) {
		    *msg = "inflateFully: Unexpected end of stream";
                    inflateEnd(&strm);
		    return JNI_FALSE;
		}
		break;
	    default:
		break;
	    }
	} while (strm.avail_in > 0);
    }
    inflateEnd(&strm);
    return JNI_TRUE;
}

jzentry * JNICALL
ZIP_FindEntry(jzfile *zip, char *name, jint *sizeP, jint *nameLenP)
{
    jzentry *entry = ZIP_GetEntry(zip, name, 0);
    if (entry) {
        *sizeP = entry->size;
	*nameLenP = strlen(entry->name);
    }
    return entry;
}

/*
 * Reads a zip file entry into the specified byte array
 * When the method completes, it releases the jzentry.
 * Note: this is called from the separately delivered VM (hotspot/classic)
 * so we have to be careful to maintain the expected behaviour.
 */
jboolean JNICALL
ZIP_ReadEntry(jzfile *zip, jzentry *entry, unsigned char *buf, char *entryname)
{
    char *msg;

    strcpy(entryname, entry->name);
    if (entry->csize == 0) {
	/* Entry is stored */
	jlong pos = 0;
	jlong size = entry->size;
	while (pos < size) {
	    jint n;
	    jlong limit = ((((jlong) 1) << 31) - 1);
	    jint count = (size - pos < limit) ?
		/* These casts suppress a VC++ Internal Compiler Error */
		(jint) (size - pos) :
		(jint) limit;
	    ZIP_Lock(zip);
	    n = ZIP_Read(zip, entry, pos, buf, count);
	    msg = zip->msg;
	    ZIP_Unlock(zip);
	    if (n == -1) {
		jio_fprintf(stderr, "%s: %s\n", zip->name,
			    msg != 0 ? msg : strerror(errno));
		return JNI_FALSE;
	    }
	    buf += n;
	    pos += n;
	}
    } else {
	/* Entry is compressed */
	int ok = InflateFully(zip, entry, buf, &msg);
	if (!ok) {
	    if ((msg == NULL) || (*msg == 0)) {
		msg = zip->msg;
	    }
	    jio_fprintf(stderr, "%s: %s\n", zip->name,
			msg != 0 ? msg : strerror(errno));
	    return JNI_FALSE;
	}
    }

    ZIP_FreeEntry(zip, entry);

    return JNI_TRUE;
}

#ifdef USE_MMAP
/* Returns a pointer to a byte in the mmaped space
   containing a zip file entry */
jboolean JNICALL
ZIP_ReadMappedEntry(jzfile *zip, jzentry *entry, unsigned char **buf,
		    char *entryname)
{
    char *msg;

    strcpy(entryname, entry->name);

    /* Return FALSE if entry is compressed, so caller can retry with ZIP_Read */
    if (entry->csize != 0)
	return JNI_FALSE;

    /* Clear previous zip error */
    zip->msg = 0;

    *buf = zip->maddr + entry->pos;

    ZIP_FreeEntry(zip, entry);

    return JNI_TRUE;
}
#endif
