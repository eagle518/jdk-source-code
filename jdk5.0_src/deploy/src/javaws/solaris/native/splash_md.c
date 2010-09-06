/*
 * @(#)splash_md.c	1.7 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * The Java Web Start splash screen "server" for X11. 
 */

/*
 * The code for loading a JPEG file was lifted from example.c and
 * from jdatasrc.c, look there and in libjpeg.doc for more information
 * about how libjpeg is supposed to be used.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <setjmp.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <libintl.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"

#include "system.h"

/* 
 * Number of (shared) colormap colors to allocate, if we're running 
 * on a PsuedoColor Visual.
 */
#define NCOLORS 32



/* Number of seconds to wait before taking the splash screen down.
 * Normally we'll get a "Z" message before the timeout occurs however
 * if something's gone wrong or the VM is REALLY slow coming up we
 * exit after this many seconds.
 */
#define TIMEOUT 20


struct CloseColor {
    XColor cell;
    int distance;
};

int compareCloseColors(const void *a, const void *b)
{
    struct CloseColor *c1 = (struct CloseColor *)a, *c2 = (struct CloseColor *)b;
    return c1->distance - c2->distance;
}


/* 
 * Clean up any resources that will not be cleaned up properly as 
 * a side-effect of just exiting, and then exit.
 */
void splashExit() {
    exit(0);
}


/* 
 * Print an (obscure) error message and exit.  
 */
void errorExit(char *msg) {
    char *msg1 = getMsgString(MSG_SPLASH_EXIT);
    fprintf(stderr, msg1);
    if (errno != 0) {
	perror(msg);
    }
    else {
	fprintf(stderr, "\t%s\n", msg);
    }
    splashExit();
}


/* 
 * The following struct and five functions define an in-memory
 * source manager for the jpeg library.  It's used to pull
 * input from the splashJPEGData array (read in from jpeg file
 * which contains the raw splash screen image in JPEG format.   
 * Take a look at jdatasrc.c (in the jpeg library sources) to 
 * see a well commented example of a source manager.
 */

struct mem_source_mgr {
  struct jpeg_source_mgr pub;
};

void init_source (j_decompress_ptr cinfo) {
}

boolean fill_input_buffer (j_decompress_ptr cinfo) {
  return TRUE;
}

void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  struct mem_source_mgr *src = (struct mem_source_mgr *)cinfo->src;
  if (num_bytes > 0) {
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
  }
}

void term_source (j_decompress_ptr cinfo) {
}

void jpeg_mem_src (j_decompress_ptr cinfo, char *data, size_t nbytes)
{
  struct mem_source_mgr *src;

  cinfo->src = (struct jpeg_source_mgr *)
    (*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_IMAGE, SIZEOF(struct mem_source_mgr));

  src = (struct mem_source_mgr *)cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; 
  src->pub.term_source = term_source;
  src->pub.bytes_in_buffer = nbytes;
  src->pub.next_input_byte = (JOCTET *)data;
}



/* 
 * The following struct and the splash_error_exit function define
 * a custom error handler for the jpeg library.  If errors
 * do occur a cryptic message will be printed to stderr.
 */

struct splash_error_mgr {
  struct jpeg_error_mgr pub;	
  jmp_buf setjmp_buffer;	
};

void splash_error_exit (j_common_ptr cinfo)
{
  struct splash_error_mgr *err = (struct splash_error_mgr *)cinfo->err;
  (*cinfo->err->output_message)(cinfo);
  errorExit(getMsgString(MSG_SPLASH_NOIMAGE));
  /* longjmp(err->setjmp_buffer, 1); */
}


XImage *createPseudoColorSplashImage(
  Display *display, 
  Screen *screen, 
  struct jpeg_decompress_struct *cinfo) 
{
  XImage *image;
  XColor xcolors[NCOLORS];
  int nXColors = 0;
  int imageWidth, imageHeight;
  char *imageData;

  cinfo->quantize_colors = TRUE;
  cinfo->desired_number_of_colors = NCOLORS;
  jpeg_start_decompress(cinfo);


  /* The jpeg decoder has compressed the colormap for the 
   * image data down to NCOLORS colors (or less).  For 
   * each color in the compressed JPEG colormap we allocate
   * one shared color in the X11 default colormap. The allocated
   * XColors are are stored in the xcolors array which maps from
   * raw (JPEG) pixel value to the allocated XColor.
   */
  {
    Colormap colormap = XDefaultColormapOfScreen(screen);
    int nColors = cinfo->actual_number_of_colors;
    JSAMPROW r = cinfo->colormap[0];
    JSAMPROW g = cinfo->colormap[1];
    JSAMPROW b = cinfo->colormap[2];
    int i;
    for(i = 0; i < nColors; i++) {
      xcolors[i].red = GETJSAMPLE(r[i]) << 8;
      xcolors[i].green = GETJSAMPLE(g[i]) << 8;
      xcolors[i].blue = GETJSAMPLE(b[i]) << 8;
      if (XAllocColor(display, colormap, &(xcolors[i]))) {
	nXColors += 1;
      }
    }
  }
  
  /* We weren't able to allocate the colors we needed.  This is usually because
   * some other application, like netscape, has consumed all of the available 
   * colormap entries in the default colormap.  
   */
  if (nXColors != NCOLORS) {
      Colormap colormap = XDefaultColormapOfScreen(screen);
      int nColors = cinfo->actual_number_of_colors;
      JSAMPROW rv = cinfo->colormap[0];
      JSAMPROW gv = cinfo->colormap[1];
      JSAMPROW bv = cinfo->colormap[2];
      struct CloseColor *colors;
      Visual *visual = XDefaultVisualOfScreen(screen);
      int i, j, ncells = visual->map_entries;

      /* Load the contents of the current colormap into an array 
       * of CloseColor structs.
       */
      {
	  XColor *cells = (XColor *)calloc(ncells, sizeof(XColor));
	  for(i = 0; i < ncells; i++) {
	      cells[i].pixel = i;
	  }
	  XQueryColors(display, colormap, cells, ncells);
	  colors = (struct CloseColor *)calloc(ncells, sizeof(struct CloseColor));
	  for(i = 0; i < ncells; i++) {
	      colors[i].cell = cells[i];
	  }
	  free(cells);
      }

      nXColors = 0;

      /* For each color we want, find and allocate the closest available colormap
       * color.
       */
      for(i = 0; i < nColors; i++) {
	  int r = GETJSAMPLE(rv[i]) << 8;  /* r,g,b components of the color we want */
	  int g = GETJSAMPLE(gv[i]) << 8;
	  int b = GETJSAMPLE(bv[i]) << 8;

	  for(j = 0; j < ncells; j++) {
	      XColor *cell = &(colors[j].cell);
	      int cr = cell->red;
	      int cg = cell->green;
	      int cb = cell->blue;
	      colors[j].distance = abs(r-cr) + abs(b-cb) + abs(g-cg);
	  }
	  qsort(colors, ncells, sizeof(struct CloseColor), compareCloseColors);

	  /* The colors array contains all of the colormap cells sorted by
	   * how close they are to the r,g,b color we want.  Typically we'll
	   * be able to allocate the colors[0].cell, i.e. the best match available,
	   * however sometimes this cell is read/write or another client has 
	   * managed to free and reuse it, and XAllocColor fails.  In that case we
	   * use colors[1].cell and so on until we find a color that can be allocated.
	   */
	  for(j = 0; j < ncells; j++) {
	      xcolors[i] = colors[j].cell;
	      if (XAllocColor(display, colormap, &(xcolors[i]))) {
		  nXColors += 1;
		  break;
	      }
	  }
      }
  }

  imageWidth = cinfo->output_width;
  imageHeight = cinfo->output_height;
  imageData = (char *)(malloc(imageWidth * imageHeight));

  image = XCreateImage(
      display, 
      XDefaultVisualOfScreen(screen), 
      XDefaultDepthOfScreen(screen), 
      ZPixmap, 
      0, 
      imageData, 
      imageWidth, 
      imageHeight,
      8, 
      imageWidth);

  /* Copy the uncompressed image into the XImage data array by mapping 
   * from raw pixel values to the corresponding colormap entries.
   */
  {
    JSAMPARRAY buffer = (*cinfo->mem->alloc_sarray)((j_common_ptr)cinfo, JPOOL_IMAGE, imageWidth, 1);
    while (cinfo->output_scanline < cinfo->output_height) {
      int i;
      jpeg_read_scanlines(cinfo, buffer, 1);
      for(i = 0; i < imageWidth; i++) {
	*imageData++ = xcolors[buffer[0][i]].pixel;
      }
    }
  }

  return image;
}

typedef struct {
  int red_left_shift;
  int green_left_shift;
  int blue_left_shift;
} shift_info;

void getShiftInfo(unsigned long red_mask,
		  unsigned long green_mask,
		  unsigned long blue_mask,
		  shift_info* _shift_info) {
  unsigned long high_bit = ~0UL;
  int startBitPos = 8 * (sizeof(unsigned long) - 1);
  int i = 0;

  high_bit = ~(high_bit >> 1);

  while ((red_mask & high_bit) == 0 &&
	 (i != startBitPos)) {
    red_mask <<= 1;
    i++;
  }
  _shift_info->red_left_shift = startBitPos - i;
  i = 0;
  while ((green_mask & high_bit) == 0 &&
	 (i != 128)) {
    green_mask <<= 1;
    i++;
  }
  _shift_info->green_left_shift = startBitPos - i;
  i = 0;
  while ((blue_mask & high_bit) == 0 &&
	 (i != 128)) {
    blue_mask <<= 1;
    i++;
  }
  _shift_info->blue_left_shift = startBitPos - i;
  i = 0;
}

unsigned long packPixel(shift_info* _shift_info,
			unsigned long red_mask,
			unsigned long green_mask,
			unsigned long blue_mask,
			unsigned char red,
			unsigned char green,
			unsigned char blue) {
  unsigned long longRed = red;
  unsigned long longGreen = green;
  unsigned long longBlue = blue;

  if (_shift_info->red_left_shift > 0) {
    longRed <<= _shift_info->red_left_shift;
  } else {
    longRed >>= (-1 * _shift_info->red_left_shift);
  }
  if (_shift_info->green_left_shift > 0) {
    longGreen <<= _shift_info->green_left_shift;
  } else {
    longGreen >>= (-1 * _shift_info->green_left_shift);
  }
  if (_shift_info->blue_left_shift > 0) {
    longBlue <<= _shift_info->blue_left_shift;
  } else {
    longBlue >>= (-1 * _shift_info->blue_left_shift);
  }
  
  longRed &= red_mask;
  longGreen &= green_mask;
  longBlue &= blue_mask;
  
  return longRed | longGreen | longBlue;
}	       
	       
void putPackedPixel(unsigned char* cur_ptr, int bits_per_pixel, int image_byte_order, unsigned long pixel) {
  /* FIXME: assumes little-endian byte ordering */
  int i = bits_per_pixel;
  int bytes_per_pixel = i >> 3;  /* i / 8 */
  int direction;

  if (bytes_per_pixel == 1) {
    bytes_per_pixel = 2; /* Will this handle 15-bit color? */
  }
  
  if (image_byte_order == MSBFirst) {
    direction = -1;
    cur_ptr += bytes_per_pixel - 1;
  } else {
    direction = 1;
  }

  while (bytes_per_pixel != 0) {
    unsigned char component = pixel & 0xFF;
    *cur_ptr = component;
    cur_ptr += direction;
    --bytes_per_pixel;
    pixel >>= 8;
  }
}

XImage *createTrueColorSplashImage(
  Display *display, 
  Screen *screen, 
  struct jpeg_decompress_struct *cinfo) 
{
  XImage *image;
  int imageWidth, imageHeight, rowWidth;
  int bytesPerPixel;
  char *imageData;
  shift_info _shift_info;

  jpeg_start_decompress(cinfo);

  imageWidth = cinfo->output_width;
  imageHeight = cinfo->output_height;
  rowWidth = cinfo->output_width * cinfo->output_components;
  imageData = (char *)(malloc(4 * imageWidth * imageHeight));

  image = XCreateImage(
      display, 
      XDefaultVisualOfScreen(screen), 
      XDefaultDepthOfScreen(screen),
      ZPixmap, 
      0, 
      imageData, 
      imageWidth, 
      imageHeight,
      8, 
      0);

  getShiftInfo(image->red_mask, image->green_mask, image->blue_mask,
	       &_shift_info);
  bytesPerPixel = image->bits_per_pixel >> 3;
  if (bytesPerPixel == 1) {
    bytesPerPixel = 2; /* Will this handle 15-bit color? */
  }

  /* Copy the uncompressed JPEG image into the imageData array.  The
   * format of the JPEG pixels is RGB RGB RGB ....  The format of the
   * XImage pixels is similar except, depending on the value of 
   * XImage.bits_per_pixel, they're packed into 24 or 32 bit values.
   */
  {
    JSAMPARRAY buffer = 
      (*cinfo->mem->alloc_sarray)((j_common_ptr)cinfo, JPOOL_IMAGE, rowWidth, 1);

    while (cinfo->output_scanline < cinfo->output_height) {
      int i;
      unsigned long packed_pixel;

      jpeg_read_scanlines(cinfo, buffer, 1);
      for(i = 0; i < rowWidth; i += 3) {
	putPackedPixel((unsigned char*)imageData, image->bits_per_pixel, image->byte_order,
		       packPixel(&_shift_info, image->red_mask, image->green_mask,
				 image->blue_mask,
				 buffer[0][i], buffer[0][i+1], buffer[0][i+2]));
	imageData += bytesPerPixel;
      }
    }
  }

  return image;
}


XImage *createSplashImage(Display *display, Screen *screen, 
			  unsigned char *data, size_t length)
{
  struct jpeg_decompress_struct cinfo;
  struct splash_error_mgr jerr;
  XImage *image;

  if (data == NULL) { 
    return NULL;
  }

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = splash_error_exit;
  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    return NULL;
  }

  jpeg_create_decompress(&cinfo);
  jpeg_mem_src(&cinfo, (char *)data, length);
  jpeg_read_header(&cinfo, TRUE);
  

  {
    Visual *visual = XDefaultVisualOfScreen(screen);
    if (visual->class == PseudoColor) {
      image = createPseudoColorSplashImage(display, screen, &cinfo);
    }
    else if (visual->class == TrueColor) {
      image = createTrueColorSplashImage(display, screen, &cinfo);
    }
    else {
      image = NULL;
    }
  }
      
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  return image;
}


Window createSplashWindow(Display *display, Screen *screen, 
                          unsigned char *mainData, size_t mainSize,
                          unsigned char *localeData, size_t localeSize)
{
  XSetWindowAttributes windowAttributes;
  unsigned short width, height;
  int depth;
  Pixmap pixmap;
  int localeOffset = 0;

  XImage *mainImage = createSplashImage(display, screen, mainData, mainSize);
  XImage *localeImage = 
		createSplashImage(display, screen, localeData, localeSize);

  if (mainImage != NULL) {
      width = mainImage->width;
      height = mainImage->height;
      depth = mainImage->depth;
      if (localeImage != NULL) {
          height += localeImage->height;
      }
  } else if (localeImage != NULL) {
      width = localeImage->width;
      height = localeImage->height;
      depth = localeImage->depth;
  } else {
    return (Window) NULL;
  }
      
  pixmap = XCreatePixmap(
    display,
    XRootWindowOfScreen(screen),
    width,
    height,
    depth);

  if (mainImage != NULL) {
    XPutImage(
      display,
      pixmap, 
      XDefaultGCOfScreen(screen),
      mainImage,
      0, 0, 0, 0,
      mainImage->width,
      mainImage->height);
    localeOffset = mainImage->height;
  }

  if (localeImage != NULL) {
    XPutImage(
      display,
      pixmap, 
      XDefaultGCOfScreen(screen),
      localeImage,
      0, 0, 0, localeOffset,
      localeImage->width,
      localeImage->height);
  }     
     
  windowAttributes.background_pixmap = pixmap;
  windowAttributes.colormap = XDefaultColormapOfScreen(screen);
  windowAttributes.backing_store = NotUseful;
  windowAttributes.save_under = True;
  windowAttributes.override_redirect = True;

  return XCreateWindow(
     display,		       
     XRootWindowOfScreen(screen),                       
     (XWidthOfScreen(screen) / 2) - (width / 2),
     (XHeightOfScreen(screen) / 2) - (height / 2),
     width, 
     height, 
     0,
     0, 
     InputOutput, 
     CopyFromParent, 
     CWBackPixmap | CWColormap | CWBackingStore | CWSaveUnder | CWOverrideRedirect, 
     &windowAttributes
   );
}


void splashEventLoop(Display *display, Screen *screen, Window window, int server)
{
    struct pollfd fds[2];
    int nfds;

    fds[0].fd = XConnectionNumber(display);
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    fds[1].fd = server;
    fds[1].events = POLLIN;
    fds[1].revents = 0;

    XMapWindow(display, window);
    XFlush(display);

    while((nfds = poll(fds, 2, -1)) > 0) {

	/* If an error occurred on the X11 fd - we're done. */

	if (fds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) {
	    errorExit(getMsgString(MSG_SPLASH_X11_CONNECT));
	}
	
	/* If an error occurred on the connection socket - we're done. */

	if (fds[1].revents & (POLLERR | POLLHUP | POLLNVAL)) {
	    errorExit(getMsgString(MSG_SPLASH_SOCKET));
	}

	/* Data is available on the incoming connection socket: accept
	 * the connection, read one command, and then close the connection.
	 */
	if (fds[1].revents & POLLIN) {
	    SOCKADDR_IN iname = {0};
	    SOCKET client;
	    int length = sizeof(SOCKADDR_IN);
	    char cmd[1];

	    if ((client = accept(server, (SOCKADDR *)&iname, &length)) == -1) {
		errorExit(getMsgString(MSG_ACCEPT_FAILED));
	    }
	    if (recv(client, cmd, 1, 0) == 1) {
		switch(cmd[0]) {
		case 'Z': 
		    sysCloseSocket(client);
		    splashExit(); 
		    break;
		default:
		    errorExit(getMsgString(MSG_SPLASH_CMND));
		}
	    }
	}

	/* Data is available on the X11 fd - read all of the 
	 * pending events.
	 */
	if (fds[0].revents & POLLIN) {
	    int nEvents = XPending(display);
	    XEvent event;
	    while(nEvents-- > 0) {
		XNextEvent(display, &event);
                if (event.xany.type == ButtonPress) {
                    splashExit();
                }
	    }
	}
    }
}


void alarmHandler(int n) 
{
    if (n == SIGALRM) {
	splashExit();
    }
}

unsigned char *readJPEG(char *name, size_t *size)
{
FILE *fp;
unsigned char *buf = NULL;
size_t ret;


    fp = fopen(name, "rb");

    if (fp != NULL) {
        fseek(fp, 0, SEEK_END);
        *size = ftell(fp);
	if (*size == 0) {
            return NULL;
	}
        fseek(fp, 0, SEEK_SET);
        buf = malloc(*size);
        if ( buf == NULL) {
            return NULL;
        }
        ret = fread((void *)buf, 1, *size, fp);
        if (ret != *size) {
            free(buf);
            return NULL;
        }
    }
    return buf;
}

int sysSplash(int splashPort, char *splashFile, char *splashFile2)
{
    Display *display;
    Screen *screen;
    Window window;
    SOCKET server;
    int port;
    unsigned char* mainJPEGImageData;
    unsigned char* localeJPEGImageData;
    char *localeStr;
    char filename[MAXPATHLEN];
    int i;
    size_t localeSize, mainSize;

    if (splashPort <= 0) {
	errorExit(getMsgString(MSG_SPLASH_PORT));
    }

    mainJPEGImageData = NULL;
    localeJPEGImageData = NULL;

    if (splashFile != NULL) {
        mainJPEGImageData = readJPEG(splashFile, &mainSize);
    }

    if (splashFile2 != NULL) {
        localeJPEGImageData = readJPEG(splashFile2, &localeSize);
    }

    if ((server = sysCreateListenerSocket(&port)) == INVALID_SOCKET) {
	errorExit(getMsgString(MSG_LISTENER_FAILED));
    }

    /* Send our ephemeral port back to the parent process.  The parent 
     * process port number is argv[1].  We send the port number back
     * to the parent as a 6 character string.
     */
    {
	SOCKADDR_IN iname = {0};
	SOCKET parent;
	char data[6]; 

	parent = sysCreateClientSocket(splashPort);

	sprintf(data, "%d", port);
	if (send(parent, data, 6, 0) != 6) {
	    errorExit(getMsgString(MSG_SPLASH_SEND));
	}

	sysCloseSocket(parent);
    }

    /* Check for NO Splash mode */
    if (mainJPEGImageData == NULL && localeJPEGImageData == NULL) return;

    if ((display = XOpenDisplay(NULL)) == 0) {
	errorExit(getMsgString(MSG_SPLASH_X11_CONNECT));
    }

    screen = XDefaultScreenOfDisplay(display);
    window = createSplashWindow(display, screen, mainJPEGImageData, mainSize,
                                localeJPEGImageData, localeSize);

    if (mainJPEGImageData != NULL) {
	free(mainJPEGImageData);
    }

    if (localeJPEGImageData != NULL) {
	free(localeJPEGImageData);
    }

    XSelectInput(display, window, ButtonPressMask | ButtonReleaseMask |  PointerMotionMask);

    /* Set a timer that will exit this process after TIMEOUT seconds 
     */
    if (window != (Window)NULL) {
        alarm(TIMEOUT);

        splashEventLoop(display, screen, window, server);
    }
}


