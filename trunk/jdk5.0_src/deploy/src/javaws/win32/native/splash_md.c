/*
 * @(#)splash_md.c	1.7 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Java Web Start Splash Screen "Server"
 * 
 * The code for loading a JPEG file was lifted from example.c and
 * from jdatasrc.c, look there and in libjpeg.doc for more information
 * about how libjpeg is supposed to be used.
 */


#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"

#include "system.h"
#include "util.h"
#include <setjmp.h>
#include <locale.h>


/* This is the socket we listen on.
 */   
static SOCKET server;

/* This is the message that's sent when there's a pending connection
 * on our server socket.  See the call.
 */
#define WM_CONNECT WM_USER + 1


/* Number of seconds to wait before taking the splash screen down.
 * Normally we'll get a "Z" message before the timeout occurs however
 * if something's gone wrong or the VM is REALLY slow coming up we
 * exit after this many seconds.
 */
#define TIMEOUT 20


/* 
 * Number of colormap entries to use, if we're running 
 * on a 8 bit display.
 */
#define NCOLORS 32


/* This macro forces it's value to be a multiple of 4.  It's used
 * when create a DIB to ensure that the rows end on word boundary.
 */
#define ALIGN32(width) ((int)(~3UL & (unsigned long)((width) +3)))


/* pointers (may be NULL) to the two halves of the actual image data
 */
unsigned char * mainJPEGImageData;
unsigned char * localeJPEGImageData;
size_t localeSize, mainSize;

/* 
 * Clean up any resources that will not be cleaned up properly as 
 * a side-effect of just exiting, and then exit.
 */
void splashExit() {
    exit(0);
}


/* 
 * Print an (obscure) error message and exit.  For an explanation of the
 * WinSock error codes, see:
 *   metalab.unc.edu/pub/micro/pc-stuff/ms-windows/winsock/winsock-1.1/winsock.html#ErrorCodes
 */
void errorExit(char *msg) {
    fprintf(stderr, getMsgString(MSG_SPLASH_EXIT) );
    if (errno != 0) {
	perror(msg);
    }
    else {
	fprintf(stderr, "\t%s\n", msg);
    }
    fprintf(stderr, "%s %d\n", getMsgString(MSG_WSA_ERROR), WSAGetLastError());
}



/* 
 * The following struct and five functions define an in-memory
 * source manager for the jpeg library.  It's used to pull
 * input from the JPEGData arrays which contains the raw
 * splash screen image in JPEG format.   Take a look at 
 * jdatasrc.c (in the jpeg library sources) to see a well
 * commented example of a source manager.
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
 * a custom error handler for the jpeg library.  Since we're pulling
 * data from an array of bytes that's read in from the jpg files
 * into this application - we don't really expect any errors.  If they
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



HBITMAP createPseudoColorSplashImage(
    HWND hWnd, 
    HDC hDC, 
    struct jpeg_decompress_struct *cinfo, 
    int *width, 
    int *height) 
{
    HBITMAP image;
    BITMAPINFO *bi;
    int imageWidth, imageHeight, rowWidth, nColors;
    char *imageData = 0;

    cinfo->quantize_colors = TRUE;
    cinfo->desired_number_of_colors = NCOLORS;
    jpeg_start_decompress(cinfo);

    imageWidth = ALIGN32(cinfo->output_width);
    imageHeight = cinfo->output_height;
    rowWidth = cinfo->output_width * cinfo->output_components;
    nColors = cinfo->actual_number_of_colors;

    bi = (BITMAPINFO *)calloc(1, sizeof(BITMAPINFOHEADER) + (nColors * sizeof(RGBQUAD)));
    bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi->bmiHeader.biWidth = imageWidth;
    bi->bmiHeader.biHeight = imageHeight;
    bi->bmiHeader.biPlanes = 1;
    bi->bmiHeader.biBitCount = 8;
    bi->bmiHeader.biCompression = BI_RGB;
    bi->bmiHeader.biSizeImage = imageWidth * imageHeight;
    bi->bmiHeader.biClrUsed = nColors;
    bi->bmiHeader.biClrImportant = nColors;

    /* The jpeg decoder has compressed the colormap for the 
     * image data down to NCOLORS colors (or less).  Copy the
     * jpeg colormap into bi->bmiColors.
     */
    {
	JSAMPROW r = cinfo->colormap[0];
	JSAMPROW g = cinfo->colormap[1];
	JSAMPROW b = cinfo->colormap[2];
	int i;
	for(i = 0; i < nColors; i++) {
	    bi->bmiColors[i].rgbRed = GETJSAMPLE(r[i]);
	    bi->bmiColors[i].rgbGreen = GETJSAMPLE(g[i]);
	    bi->bmiColors[i].rgbBlue = GETJSAMPLE(b[i]);
	}
    }

  image = CreateDIBSection(hDC, bi, DIB_RGB_COLORS, (void **)&imageData, NULL, 0);

  /* Copy the uncompressed image into the image data array.
   */
  {
      JSAMPARRAY buffer = (*cinfo->mem->alloc_sarray)((j_common_ptr)cinfo, JPOOL_IMAGE, rowWidth, 1);
      while (cinfo->output_scanline < cinfo->output_height) {
	  int i, invRow = (imageHeight - 1) - cinfo->output_scanline;
	  char *imageRow = imageData + (invRow * imageWidth);
	  jpeg_read_scanlines(cinfo, buffer, 1);
	  for(i = 0; i < rowWidth; i++) {
	      *imageRow++ = buffer[0][i];
	  }
      }
  }

  *width = imageWidth;
  *height = imageHeight;
  return image;
}



HBITMAP createTrueColorSplashImage(
    HWND hWnd, 
    HDC hDC, 
    struct jpeg_decompress_struct *cinfo, 
    int *width, 
    int *height) 
{
    HBITMAP image;
    BITMAPINFO bi = {0};
    int imageWidth, imageHeight, rowWidth;
    char *imageData = 0;

    jpeg_start_decompress(cinfo);

    imageWidth = ALIGN32(cinfo->output_width);
    imageHeight = cinfo->output_height;
    rowWidth = cinfo->output_width * cinfo->output_components;

    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = imageWidth;
    bi.bmiHeader.biHeight = imageHeight;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biSizeImage = imageWidth * imageHeight * 3;

    image = CreateDIBSection(hDC, &bi, DIB_RGB_COLORS, (void **)&imageData, NULL, 0);

    /* Copy the uncompressed JPEG image into the imageData array.  The
     * format of the JPEG pixels is RGB RGB RGB ....  The format of the
     * windows pixels is BGR BGR and the first pixel corresponds to 
     * the last pixel in imageData.  To complicate matters further, the width
     * of the DIB must be (32 bit) word aligned; the uncompressed jpeg data
     * isn't aligned.
     */
    {
	JSAMPARRAY buffer = 
	    (*cinfo->mem->alloc_sarray)((j_common_ptr)cinfo, JPOOL_IMAGE, rowWidth, 1);

	while (cinfo->output_scanline < cinfo->output_height) {
	    int i, invRow = (imageHeight - 1) - cinfo->output_scanline;
	    char *imageRow = imageData + (((invRow * imageWidth * 3) + rowWidth) - 1);
	    jpeg_read_scanlines(cinfo, buffer, 1);
	    for(i = rowWidth - 1; i >= 0; i -= 3) {
		*imageRow-- = buffer[0][i-2];
		*imageRow-- = buffer[0][i-1];
		*imageRow-- = buffer[0][i];
	    }
	}
    }

    *width = imageWidth;
    *height = imageHeight;
    return image;
}



HBITMAP createSplashImage(HWND hWnd, unsigned char *data, size_t size,
				     int *width, int *height) 
{
  struct jpeg_decompress_struct cinfo;
  struct splash_error_mgr jerr;
  HBITMAP image;
  HDC hDC;


  if ((size == 0) || (data == NULL)) {
	return (HBITMAP) 0;
  }

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = splash_error_exit;
  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    return NULL;
  }


  jpeg_create_decompress(&cinfo);
  jpeg_mem_src(&cinfo, (char *)data, size);
  jpeg_read_header(&cinfo, TRUE);
  
  hDC = GetDC(hWnd);
  if (GetDeviceCaps(hDC, NUMCOLORS) <= 256) {
      image = createPseudoColorSplashImage(hWnd, hDC, &cinfo, width, height);
  }
  else {
      image = createTrueColorSplashImage(hWnd, hDC, &cinfo, width, height);
  }
  ReleaseDC(hWnd, hDC);
      
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);


  return image;
}



void initSplashWindow(HWND hWnd, int width, int height)
{
    HWND hDesktopWnd;
    RECT screenRect;
    int x,y;

    hDesktopWnd = GetDesktopWindow();
    GetClientRect(hDesktopWnd, &screenRect);
    x = ((screenRect.right - screenRect.left) - width) / 2;
    y = ((screenRect.bottom - screenRect.top) - height) / 2;
    SetWindowPos(hWnd, HWND_TOPMOST, x, y, width, height, SWP_SHOWWINDOW);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HBITMAP mainImage, localeImage;
    static int width1 = 0, height1 = 0;
    static int width2 = 0, height2 = 0;
    static int width = 0, height = 0; 
    PAINTSTRUCT ps;
    HDC hDC, hMemDC;

    switch (message) {

    case WM_CREATE:
	mainImage = createSplashImage(hWnd, mainJPEGImageData, mainSize,
					    &width1, &height1);
	localeImage = createSplashImage(hWnd, localeJPEGImageData, localeSize,
					      &width2, &height2);
        width = (width1 > width2) ? width1 : width2;
	height = height1 + height2;
	initSplashWindow(hWnd, width, height);

	if (mainJPEGImageData != NULL) {
	    free(mainJPEGImageData);
	    mainJPEGImageData = NULL;
        }

	if (localeJPEGImageData != NULL) {
	    free(localeJPEGImageData);
	    localeJPEGImageData = NULL;
        }

	break;

    case WM_PAINT:
	hDC = BeginPaint(hWnd, &ps);
	hMemDC = CreateCompatibleDC(hDC);
        if (mainImage != NULL) {
	    SelectObject(hMemDC, mainImage);
	    BitBlt(hDC, 0, 0, width1, height1, hMemDC, 0, 0, SRCCOPY);
        }
        if (localeImage != NULL) {
	    SelectObject(hMemDC, localeImage);
	    BitBlt(hDC, 0, height1, width2, height2, hMemDC, 0, 0, SRCCOPY);
        }
	DeleteDC(hMemDC);
	EndPaint(hWnd, &ps);
	break;

    case WM_CONNECT:
    {
	SOCKADDR_IN iname;
	int length = sizeof(iname);
	SOCKET client;
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
    case WM_LBUTTONDOWN:
	splashExit();

    case WM_DESTROY:
	PostQuitMessage(0);
	break;

    default:
	return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

VOID CALLBACK timeoutProc(HWND hWnd, UINT uMsg, UINT uTimerID, DWORD dwTime)
{
    splashExit();
}

#ifdef DEBUG
static char debug_string[512];
static FILE *myfile = NULL;
void debug_it() {

    if (myfile == NULL) {
        myfile = fopen("myfile", "wb");
    }
    if (myfile != NULL) {
        fprintf(myfile, debug_string);
        fflush(myfile);
    }
}
#endif



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
    WNDCLASS wce = {0};	
    HWND hWnd;
    MSG msg;
    int port;
    int ud = CW_USEDEFAULT;
    HINSTANCE hInstance = GetModuleHandle(NULL);

    mainJPEGImageData = NULL;
    localeJPEGImageData = NULL;

    if ((server = sysCreateListenerSocket(&port)) == INVALID_SOCKET) {
	errorExit(getMsgString(MSG_LISTENER_FAILED));
    }

    if (splashFile != NULL) {
        mainJPEGImageData = readJPEG(splashFile, &mainSize);
    }

    if (splashFile2 != NULL) {
        localeJPEGImageData = readJPEG(splashFile2, &localeSize);
    }

    /* Send our ephemeral port back to the parent process.  The parent 
     * process port number is splashPort.  We send the port number back
     * to the parent as a 6 character string.
     */
    {
	SOCKADDR_IN iname = {0};
	SOCKET parent;
	char data[6]; 

	if (splashPort <= 0) {
	    errorExit(getMsgString(MSG_SPLASH_PORT));
	}

	if ((parent = sysCreateClientSocket(splashPort)) == INVALID_SOCKET) {
	    errorExit(getMsgString(MSG_SPLASH_SOCKET));
	}

	sprintf(data, "%d", port);
	if (send(parent, data, 6, 0) != 6) {
	    errorExit(getMsgString(MSG_SPLASH_SEND));
	}

	sysCloseSocket(parent);
    }

    /* Check for NO Splash mode */
    if (mainJPEGImageData == NULL && localeJPEGImageData == NULL) return 0;


    /* Create the splash screen window.  It will be configured at
     * WM_CREATE time.
     */
    
    /* workaround for 4812026: J2D caused Java Web Start to crash Windows XP 
     *  HOME (no SP) with Intel(R) 82845G/GL/GE/PE/GV Graphics Controller
     */
    wce.style = 0;
    wce.lpfnWndProc = (WNDPROC)WndProc;
    wce.hInstance = hInstance;
    wce.hCursor	= LoadCursor(NULL, IDC_ARROW);
    wce.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wce.lpszClassName = "Win32Splash";
    RegisterClass(&wce);

    hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, "Win32Splash", NULL, WS_POPUP, ud,ud,ud,ud, NULL, NULL, hInstance, NULL);

    /* Set a timer that will exit this process after TIMEOUT seconds 
     */
    if (SetTimer(hWnd, 0, TIMEOUT * 1000, (TIMERPROC)timeoutProc) == 0) {
    	errorExit(getMsgString(MSG_SPLASH_TIMER));
    }

    /* Ask Win32 to notify us when there are pending connections on the
     * server socket.  WM_CONNECT is an "event" defined here.
     */
    WSAAsyncSelect(server, hWnd, WM_CONNECT, FD_ACCEPT);

    while (GetMessage(&msg, NULL, 0, 0)) {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }
    
    //return msg.wParam;
    return 0;
}


