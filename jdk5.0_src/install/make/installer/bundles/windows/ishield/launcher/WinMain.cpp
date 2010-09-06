/*
 * @(#)WinMain.cpp	1.1 04/02/10 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// General purpose launcher to suppress dos windows
// during installer launches.
#include "windows.h"
#include <stdio.h>

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow) {
  STARTUPINFO si;
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);

  PROCESS_INFORMATION pi;
  memset(&pi, 0, sizeof(pi));

  //Test
  //lpCmdLine = "c:/build/ws-tiger/windows-i586/bin/unpack200 -l c:/Temp/log c:/Temp/rt.pack c:/Temp/rt.jar";
  int ret = CreateProcess(NULL,			/* Exec. name */
			  lpCmdLine,		/* cmd line */
			  NULL,			/* proc. sec. attr. */
			  NULL,			/* thread sec. attr */
			  TRUE,			/* inherit file handle */
			  CREATE_NO_WINDOW | DETACHED_PROCESS, /* detach the process/suppress console */
			  NULL,			/* env block */
			  NULL,			/* inherit cwd */
			  &si,			/* startup info */
			  &pi);			/* process info */
  if ( ret == 0) ExitProcess(255);

  // Wait until child process exits.
  WaitForSingleObject( pi.hProcess, INFINITE );
  DWORD exit_val;

  // Be conservative and return
  if (GetExitCodeProcess(pi.hProcess, &exit_val) == 0) ExitProcess(255);
  ExitProcess(exit_val); // Return the error code of the child process

  return -1;
}
