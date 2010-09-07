/*
 * @(#)ProcessImpl_md.c	1.21 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <assert.h>
#include "java_lang_ProcessImpl.h"

#include "jni.h"
#include "jvm.h"
#include "jni_util.h"
#include "io_util.h"
#include <windows.h>

#define PIPE_SIZE 512

char *
extractExecutablePath(JNIEnv *env, char *source)
{
    char *p, *r;

    /* If no spaces, then use entire thing */
    if ((p = strchr(source, ' ')) == NULL)
        return source;

    /* If no quotes, or quotes after space, return up to space */
    if (((r = strchr(source, '"')) == NULL) || (r > p)) {
        *p = 0;
        return source;
    }

    /* Quotes before space, return up to space after next quotes */
    p = strchr(r, '"');
    if ((p = strchr(p, ' ')) == NULL)
        return source;
    *p = 0;
    return source;
}

DWORD
selectProcessFlag(JNIEnv *env, jstring cmd0)
{
    char buf[MAX_PATH];
    DWORD newFlag = 0;
    DWORD *type;
    char *exe, *p, *name;
    unsigned char buffer[2];
    long headerLoc = 0;
    int fd = 0;

    exe = (char *)JNU_GetStringPlatformChars(env, cmd0, 0);
    exe = extractExecutablePath(env, exe);

    if (exe != NULL) {
        if ((p = strchr(exe, '\\')) == NULL) {
            SearchPath(NULL, exe, ".exe", MAX_PATH, buf, &name);
        } else {
            p = strrchr(exe, '\\');
            *p = 0;
            p++;
            SearchPath(exe, p, ".exe", MAX_PATH, buf, &name);
        }
    }

    fd = _open(buf, _O_RDONLY);
    if (fd > 0) {
        _read(fd, buffer, 2);
        if (buffer[0] == 'M' && buffer[1] == 'Z') {
            _lseek(fd, 60L, SEEK_SET);
            _read(fd, buffer, 2);
            headerLoc = (long)buffer[1] << 8 | (long)buffer[0];
            _lseek(fd, headerLoc, SEEK_SET);
            _read(fd, buffer, 2);
            if (buffer[0] == 'P' && buffer[1] == 'E') {
                newFlag = DETACHED_PROCESS;
            }
        }
        _close(fd);
    }
    JNU_ReleaseStringPlatformChars(env, cmd0, exe);
    return newFlag;
}

JNIEXPORT jlong JNICALL
Java_java_lang_ProcessImpl_create(JNIEnv *env, jobject process,
				  jstring cmd,
				  jstring envBlock,
				  jstring dir,
				  jboolean redirectErrorStream,
				  jobject in_fd,
				  jobject out_fd,
				  jobject err_fd)
{
    HANDLE inRead   = 0;
    HANDLE inWrite  = 0;
    HANDLE outRead  = 0;
    HANDLE outWrite = 0;
    HANDLE errRead  = 0;
    HANDLE errWrite = 0;
    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    LPTSTR  pcmd      = NULL;
    LPCTSTR pdir      = NULL;
    LPVOID  penvBlock = NULL;
    jlong ret = 0;
    OSVERSIONINFO ver;
    jboolean onNT = JNI_FALSE;
    DWORD processFlag;

    ver.dwOSVersionInfoSize = sizeof(ver);
    GetVersionEx(&ver);
    if (ver.dwPlatformId == VER_PLATFORM_WIN32_NT)
        onNT = JNI_TRUE;

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = 0;
    sa.bInheritHandle = TRUE;

    if (!(CreatePipe(&inRead,  &inWrite,  &sa, PIPE_SIZE) &&
          CreatePipe(&outRead, &outWrite, &sa, PIPE_SIZE) &&
	  CreatePipe(&errRead, &errWrite, &sa, PIPE_SIZE))) {
	JNU_ThrowByName(env, "java/io/IOException", "CreatePipe");
	goto failure;
    }

    assert(cmd != NULL);
    pcmd = (LPTSTR) JNU_GetStringPlatformChars(env, cmd, NULL);
    if (pcmd == NULL) goto failure;

    if (dir != 0) {
	pdir = (LPCTSTR) JNU_GetStringPlatformChars(env, dir, NULL);
	if (pdir == NULL) goto failure;
        pdir = (LPCTSTR) JVM_NativePath((char *)pdir);
    }

    if (envBlock != NULL) {
	penvBlock = onNT
	    ? (LPVOID) ((*env)->GetStringChars(env, envBlock, NULL))
	    : (LPVOID) JNU_GetStringPlatformChars(env, envBlock, NULL);
	if (penvBlock == NULL) goto failure;
    }

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput  = inRead;
    si.hStdOutput = outWrite;
    si.hStdError  = redirectErrorStream ? outWrite : errWrite;

    SetHandleInformation(inWrite, HANDLE_FLAG_INHERIT, FALSE);
    SetHandleInformation(outRead, HANDLE_FLAG_INHERIT, FALSE);
    SetHandleInformation(errRead, HANDLE_FLAG_INHERIT, FALSE);

    if (redirectErrorStream)
	SetHandleInformation(errWrite, HANDLE_FLAG_INHERIT, FALSE);

    if (onNT)
	processFlag = CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT;
    else
        processFlag = selectProcessFlag(env, cmd);

    /* Java and Windows are both pure Unicode systems at heart.
     * Windows has both a legacy byte-based API and a 16-bit Unicode
     * "W" API.  The Right Thing here is to call CreateProcessW, since
     * that will allow all process-related information like command
     * line arguments to be passed properly to the child.  We don't do
     * that currently, since we would first have to have "W" versions
     * of JVM_NativePath and perhaps other functions.  In the
     * meantime, we can call CreateProcess with the magic flag
     * CREATE_UNICODE_ENVIRONMENT, which passes only the environment
     * in "W" mode.  We will fix this later. */

    ret = CreateProcess(0,           /* executable name */
                        pcmd,        /* command line */
                        0,           /* process security attribute */
                        0,           /* thread security attribute */
                        TRUE,        /* inherits system handles */
                        processFlag, /* selected based on exe type */
                        penvBlock,   /* environment block */
                        pdir,        /* change to the new current directory */
                        &si,         /* (in)  startup information */
                        &pi);        /* (out) process information */

    if (!ret) {
	char msg[1024];
	jio_snprintf(msg, 1024,
		     "CreateProcess: %s error=%d", pcmd, GetLastError());
	JNU_ThrowByName(env, "java/io/IOException", msg);
	goto failure;
    }

    CloseHandle(pi.hThread);
    ret = (jlong)pi.hProcess;
    (*env)->SetLongField(env, in_fd,  IO_handle_fdID, (jlong)inWrite);
    (*env)->SetLongField(env, out_fd, IO_handle_fdID, (jlong)outRead);
    (*env)->SetLongField(env, err_fd, IO_handle_fdID, (jlong)errRead);

    goto success;

 failure:
    /* Clean up the parent's side of the pipes in case of failure only */
    if (inWrite) CloseHandle(inWrite);
    if (outRead) CloseHandle(outRead);
    if (errRead) CloseHandle(errRead);

 success:
    /* Always clean up the child's side of the pipes */
    if (inRead)   CloseHandle(inRead);
    if (outWrite) CloseHandle(outWrite);
    if (errWrite) CloseHandle(errWrite);

    if (pcmd != NULL)
	JNU_ReleaseStringPlatformChars(env, cmd, (char *) pcmd);
    if (pdir != NULL)
	JNU_ReleaseStringPlatformChars(env, dir, (char *) pdir);
    if (penvBlock != NULL) {
	if (onNT)
	    (*env)->ReleaseStringChars(env, envBlock, (jchar *) penvBlock);
	else
	    JNU_ReleaseStringPlatformChars(env, dir, (char *) penvBlock);
    }
    return ret;
}

JNIEXPORT jint JNICALL
Java_java_lang_ProcessImpl_exitValue(JNIEnv *env, jobject process)
{
    jint exit_code;
    jboolean exc;
    jlong handle = JNU_GetFieldByName(env, &exc, process, "handle", "J").j;
    if (exc) {
        return 0;
    }

    GetExitCodeProcess((void *)handle, &exit_code);
    if (exit_code == STILL_ACTIVE) {
	JNU_ThrowByName(env, "java/lang/IllegalThreadStateException",
			"process has not exited");
	return -1;
    }
    return exit_code;
}

#define PROCESS_EVENT 0
#define INTERRUPT_EVENT 1

JNIEXPORT jint JNICALL
Java_java_lang_ProcessImpl_waitFor(JNIEnv *env, jobject process)
{
    long exit_code;
    int which;
    HANDLE events[2];
    jboolean exc;
    jlong handle = JNU_GetFieldByName(env, &exc, process, "handle", "J").j;
    if (exc) {
        return 0;
    }

    events[PROCESS_EVENT] = (void *)handle;
    events[INTERRUPT_EVENT] = JVM_GetThreadInterruptEvent();

    which = WaitForMultipleObjects(2, events, FALSE, INFINITE);
    if (which == PROCESS_EVENT) {
	GetExitCodeProcess((void *)handle, &exit_code);
    } else {
	JNU_ThrowByName(env, "java/lang/InterruptedException", 0);
    }

    return exit_code;
}

#undef PROCESS_EVENT
#undef INTERRUPT_EVENT

JNIEXPORT void JNICALL
Java_java_lang_ProcessImpl_destroy(JNIEnv *env, jobject process)
{
    jboolean exc;
    jlong handle = JNU_GetFieldByName(env, &exc, process, "handle", "J").j;
    if (exc) {
        return;
    }
    TerminateProcess((void *)handle, 1);
}

JNIEXPORT void JNICALL
Java_java_lang_ProcessImpl_close(JNIEnv *env, jobject process)
{
    jboolean exc;
    jlong handle = JNU_GetFieldByName(env, &exc, process, "handle", "J").j;
    if (exc) {
        return;
    }
    CloseHandle((void *)handle);
}
