/*
 * @(#)main.cpp	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <stdlib.h>



#include "zip.h"

#define CLASS_NAME "zipper-upper"

static char *log_file_name;
static char *cmdline;

int syntax(char *);
int run(int argc, char **argv);

int syntax(char *program) {
   fprintf(stderr, "Usage: %s -r file.zip outputdir [logfilename]\n",program);
   fprintf(stderr, "       -r deletes file.zip\n");
   fprintf(stderr, "       optionally the last argument is a log file name\n");
   fprintf(stderr, "       %s -w files.list file.zip [logfilename]\n",program); 
   fprintf(stderr, "       -w files.list create file.zip\n");
   fprintf(stderr, "       optionally the last argument is a log file name\n");
   fprintf(stderr, "       input.zip outputdir [logfilename]\n",program);
   fprintf(stderr, "       default read from file.zip extracting the\n"); 
   fprintf(stderr, "       files to the specified dir[must contain trailing\\ \n");
   fprintf(stderr, "       optionally the last argument is a log file name\n");
   exit(-1);
}

// Redirect all stdout and stderr mesages to a log file.
void redirect_stdio(const char *logname) {
#ifdef WIN32
  if ( logname == NULL || (*logname != '\0' && freopen(logname, "a+", stderr) == NULL) ) { 
    log_file_name = (char *)malloc(MAX_PATH+1);
    char tmpdir[MAX_PATH+1];
    int n = GetTempPath(MAX_PATH,tmpdir); //API returns with trailing '\'
    if ( n < 1  || n > MAX_PATH) {
        sprintf(tmpdir,"C:\\");
    }
    sprintf(log_file_name, "%s\\zipper.log",tmpdir);

    if (freopen(log_file_name, "a+", stderr) == NULL) {
        //Try with unique temporary file, guaranteed to exist by MS!
        GetTempFileName(tmpdir,"#zpr",0,log_file_name);
        freopen(log_file_name, "a+", stderr);
    }
  } 
   _dup2(2,1);
   setbuf(stderr, 0);
   setbuf(stdout, 0);
   fprintf(stderr,"cmdline = %s\n",cmdline);
#endif
}


HINSTANCE ourInstance;


// This function is called to process window messages
LRESULT CALLBACK window_proc(
	HWND window,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam)
{
    // fprintf(stderr, "window_proc: %d\n", msg);
     return 0;
}


// This function runs as a separate thread to manage our
// window state.
// We crate an invisible window simply so that InstallShield
// can detect when we exit.  Sigh.
// Lifted from uncrunch code.
DWORD WINAPI
window_thread(LPVOID arg) 
{
    WNDCLASSEX wex;
    ATOM our_class;
    HWND window;
    MSG msg;

    // fprintf(stderr, "Window thread started\n");

    // Initialize our window state.
    wex.cbSize = sizeof(wex);
    wex.style = CS_NOCLOSE;
    wex.lpfnWndProc = window_proc;
    wex.cbClsExtra = 0;
    wex.cbWndExtra = 0;
    wex.hInstance = ourInstance;
    wex.hIcon = NULL;
    wex.hCursor = NULL;
    wex.hbrBackground = NULL;
    wex.lpszMenuName = NULL;
    wex.lpszClassName = CLASS_NAME;
    wex.hIconSm = NULL;

    our_class = RegisterClassEx(&wex);
    
    if (our_class == 0) {
	fprintf(stderr, "RegisterClassEx error %d\n", GetLastError());
    }

    window = CreateWindow(CLASS_NAME,
		"Unpack.exe",
		WS_DLGFRAME,
		400, 300,	// Position
		300, 200,	//Sixe
		NULL,
		NULL,
		ourInstance,
		0);

    if (window == 0) {
	fprintf(stderr, "CreateWindow error %d\n", GetLastError());
    }

    // Run the window message loop.
    while (GetMessage(&msg, NULL, 0, 0)) { 
        DispatchMessage(&msg);
    } 
    return 0;
}

struct strargs {
  int   argc   ;
  char  *argv[1024] ;
} st ;


void getArgs(char *s) {
  char *delim = {"\n\t "};
  char *p;
  char *str = strdup(s);

  st.argc = 0 ;

  // Parameters could be quoted for FileNames with spaces.
  for (p = strtok(str, delim) ; p != NULL ;) {
    if (*p == '"') { 
      char *q = new char[MAX_PATH];
      strcpy(q, ++p) ; 
      if (q[strlen(p) - 1] == '"') { //Get rid of the last quote
        q[strlen(p) -1 ] = '\0';
      } else {  // If we dont have a quote look for more
        while ( (p = strtok(NULL, delim)) != NULL) {
          strcat(q, " ");
	  strcat(q, p);
	  char *lastq = strrchr(q,'"');
	  if (lastq != NULL) {
	    *lastq = '\0';
	    break;
	  }
        }
      }
      p = q ;
    }

    // WinMain will not set argv[0] to the program name
    // MSVC debugger will set it.
    if (st.argc == 0 ) {
      char *last_dot = strrchr(p,'.');
      if ( (*p == '-') || ((last_dot != NULL) && (strcasecmp(last_dot, ".exe") != 0)) ) {
       st.argv[st.argc] = strdup("unpack.exe");
       st.argc++;
      }
    }

    st.argv[st.argc] = strdup(p) ;
    p = strtok(NULL, delim);
    st.argc++;
  }
}


// Main entry point.
int APIENTRY
WinMain(HINSTANCE hInstance, 
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
    cmdline = lpCmdLine; // so we can print it out later
    ourInstance = hInstance;
    getArgs(lpCmdLine); 
    return run(st.argc, st.argv);
}

int run(int argc, char **argv) 
{
  if (argc < 2) {
    return syntax(argv[0]);
  }
  char opt = (argv[1][0] == '-') ? argv[1][1] : '\0';
  switch (opt) {
    case 'r':
      set_remove_input_file();
      if (argv[2]) {
        redirect_stdio(argv[4]);  
	do_read(argv[2], argv[3]);
      } else syntax(argv[0]);
      break ;
    case 'w':
      redirect_stdio(argv[4]);       
      do_write(argv[2], argv[3]);
      break ;
    default:
      redirect_stdio(argv[3]);     
      do_read(argv[1], argv[2]);
  }
  return 0; // "Alls well that ends well"
}
