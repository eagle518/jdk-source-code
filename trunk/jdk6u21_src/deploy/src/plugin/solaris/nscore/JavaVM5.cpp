/*
 * @(#)JavaVM5.cpp	1.117 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*  
 * The JavaVM5 class represents our connect to the actual VM.  It is
 * created, intialized and then started, which spawns off the VM
 * process. The basic execution structure is the same as that in the
 * original plugin 1.1.  BG Sept 98
 *                                      
 */

#include "commonhdr.h"
#include <sys/sem.h>
#include <sys/ipc.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stropts.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stropts.h>
#include <dlfcn.h>
#include <limits.h>
#include <errno.h>
#include "jni.h"
#include "remotejni.h"
#include "IUnixService.h"
#include "CReadBuffer.h"
#include "CWriteBuffer.h"
#include "JSObject.h"
/* For htonl and htons */
#include <arpa/inet.h>

#ifdef LINUX
#include <linux/limits.h>
#include <linux/types.h>
#include <linux/dirent.h>
#define _DIRENTRY_H
#else
#include <ulimit.h>
#include <dirent.h>
#include <sys/systeminfo.h>       /* For os_arch */
#endif
#include <sys/param.h>          // For MAXPATHLEN

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Vendor.h>
#include <X11/Shell.h>

#include "IJVMManager.h"
#include "IPluginStreamListener.h"
#include "IPluginManager.h"
#include "protocol.h"
#include "commandprotocol.h"
#include "workerprotocol.h"
#include "JavaVM5.h"
#include "JavaPluginFactory5.h"
#include "JavaPluginInstance5.h"
#include "pluginversion.h"
#include "CookieSupport.h"
#include "CSecureJNIEnv.h"
#include "QueueRunnable.h"

#include <sys/utsname.h>	/* For os_name */

#define FILE_PERM S_IRWXU                 /*rwx for user only */
extern IUnixService* g_unixService; 

extern "C" void socket_cleanup()
{
    char path[PATH_MAX];
    bzero(path,PATH_MAX);
    pid_t pid = getpid();
    sprintf(path,"%s.%s.%d",JAVA_PLUGIN_SOCKFILE,PLUGIN_NODOTVERSION,(int) pid);
    unlink(path);
}

static JD_DEFINE_IID(jISupportsIID, ISUPPORTS_IID);

void spont_queue_processor(void * data) {

        trace("JavaVM5:In spont event handler");
        if(data != NULL)  {
                JavaVM5 *vm = (JavaVM5*) data;
                vm->ProcessSpontaneousQueue();
        }
}

void worker_queue_processor(void * data) {

        trace("JavaVM5:In worker event handler");
        if(data != NULL)  {
                JavaVM5 *vm = (JavaVM5*) data;
                vm->ProcessWorkQueue();
        }
}

/* The following portion of codes contains Xt dependencies and is used
 * only for NS4.x purpose, hence should not be
 * compiled for linux platform to avoid Xt/Xm link problem at runtime.
 */
#if !defined(LINUX)
static void
child_event_handler(XtPointer data, int *fid, XtInputId *id) 
{
    worker_queue_processor((void*)data);
}
#endif	/* !defined(LINUX) */

// Initialize socket for later use by attachThread (only for NS6+/Mozilla)
static void InitializeSocket(LongTermState* state)
{

#if defined(USE_TCP_SOCKET)
    void* server_socket = g_unixService->JD_NewTCPSocket();
#else
    void* server_socket = g_unixService->JD_Socket(AF_UNIX, SOCK_STREAM, 0);
#endif
    JDNetAddr net_address;

    int try_port = INITIAL_ATHREAD_PORT;
#if defined(USE_TCP_SOCKET)
    net_address.inet.family = AF_INET;
    net_address.inet.port = htons(try_port);
    net_address.inet.ip = htonl(INADDR_ANY);
    while (g_unixService->JD_Bind(server_socket, &net_address) != JD_SUCCESS) {
        /* Try higher number addresses */
        trace("JavaVM5::Binding of server socket failed at %d", try_port);
        try_port++;
        net_address.inet.port = htons(try_port);
    }
#else
    pid_t spid = getpid();
    net_address.local.family = AF_UNIX;
    bzero(net_address.local.path,sizeof(net_address.local.path));
    sprintf(net_address.local.path,"%s.%s.%d",JAVA_PLUGIN_SOCKFILE,PLUGIN_NODOTVERSION,(int) spid);
    unlink(net_address.local.path);
    try_port = htonl(spid);
    if (g_unixService->JD_Bind(server_socket, &net_address) != JD_SUCCESS) {
      trace("JavaVM5::Binding of server socket failed at %d", try_port);
    }
    /*
     * Change the permission to the socket file for rwx by user only, however,
     * we still can not prevent the user from removing the file during plugin
     * running. This can be viewed as a hole in BSD & System V TLI. But we
     * have to live with it for now.
     */ 
    if (chmod(net_address.local.path, FILE_PERM) < 0) {
      trace("JavaVM5: Unable to change %s's mode\n",  net_address.local.path);
    }
    atexit(socket_cleanup);
#endif
    
    if (g_unixService->JD_Listen(server_socket, 3) != JD_SUCCESS)
        plugin_error("Listen on server socket failed");

    state->server_socket = server_socket;
    state->port = try_port;
}

/* The following portion of codes contains Xt dependencies and is used
 * only for NS4.x purpose, hence should not be
 * compiled for linux platform to avoid Xt/Xm link problem at runtime.
 */
#if !defined(LINUX)
void JavaVM5::ConnectPipeEvent(void* pipe) {
  int fd = (int)pipe;
    Display *display = (Display *) NULL;

    trace("JavaVM5::ConnectPipeEvent\n");

    m_pPluginFactory->GetPluginManager()->GetValue(JDPluginManagerVariable_XDisplay,
					           &display);

    if (display == NULL)
      plugin_error("Could not open display!");

    XtAppContext apctx = XtDisplayToApplicationContext(display);
    if (apctx == NULL)
      plugin_error("Could not obtain application context!");

    state->inputid = XtAppAddInput(apctx,fd,
                                   (XtPointer) XtInputReadMask,
                                   child_event_handler,this); 
}
#endif	/* !defined(LINUX) */

/*
 * Process work from the worker queue, coming from the out-of-proc
 * VM. Basically wait in an infinite loop on a timed select.
 * If we time out, return, since this may be called in the main
 * thread, and it is important that other work also get done
 * in the browser!
 */
void JavaVM5::ProcessWorkQueue(void) {    
    trace("JavaVM5:ProcessWorkQueue");
    int wp = g_unixService->JDFileDesc_To_FD(state->work_pipe);
    if (wp < 0) {
        trace("JavaVM5:work pipe is dead");
        return;
    }
	int rc;
	struct pollfd fds[1];
    fds[0].fd = wp;
    fds[0].events = POLLRDNORM;

    for (;;) {
        rc = poll(fds, 1, 0);
        if (rc < 1) {
            trace("JavaVM5:No work on work pipe");
            g_unixService->JD_EnterMonitor(workMonitor);
            workPipeClean = true;
            g_unixService->JD_NotifyAll(workMonitor);
            g_unixService->JD_ExitMonitor(workMonitor);
            break;
        }
        /* We have work in the pipe */
        DoWork();
    }
    trace("JavaVM5:Done with processing work queue");
}


/*  
 * Create a new JavaVM, associated with the plugin factory 'plugin'
 * (an instance of NPIPlugin).  Also determine whether the VM should
 * use green or native threads based on the THREADS_FLAG.  Initialize
 * and then StartVM must be called.
 */
JavaVM5::JavaVM5(JavaPluginFactory5 *plugin) {
    trace("JavaVM5:Creating JavaVM5");

    state = new LongTermState();
    sprintf(stateENVName,"%s%s","JAVA_PLUGIN_STATE",PLUGIN_NODOTVERSION);
    m_pPluginFactory = plugin;
    memset(state, 0, sizeof(struct LongTermState));
    jvm_status = JVMStatus_Enabled;
    workPipeClean = true;
    spontPipeClean = true;
    workMonitor = g_unixService->JD_NewMonitor();
    spontMonitor = g_unixService->JD_NewMonitor();
}


/*
 * Destroy the VM. In mozilla 5, since the factory is never deleted,
 * this should never be called
 */
JavaVM5::~JavaVM5() {
    trace("JavaVM5:Destroying JavaVM");
    if(workMonitor != NULL) {
        g_unixService->JD_DestroyMonitor(workMonitor);
    }
    if(spontMonitor != NULL) {
        g_unixService->JD_DestroyMonitor(spontMonitor);
    }
}

/* Return whether the browser has enabled the  VM  (it may
 * be disabled even if it is running.
 */
JDBool JavaVM5::GetJVMEnabled(void) {
    trace("JavaVM5:GetJVMEnabled");

    return JD_TRUE;
}


/*
 * Return the plugin factory associated with this VM
 */
JavaPluginFactory5* JavaVM5::GetPluginFactory(void) {
    trace("JavaVM5:GetPluginFactory");
    return m_pPluginFactory;
}


/* Dup the fdarr to safety range */
void
dup2_to_safety(int nfds, int fdarr[], 
               int safe_range_start, int safe_range_end) {

    int i;

    /* Find the max fd to identify a fd range to store fds
       temporarily during duping */
    int tempfd = safe_range_end + 1;
    for(i = 0; i < nfds ; i++)
        tempfd = (fdarr[i] >= tempfd) ? fdarr[i] + 1 : tempfd;
    
    /* Dup the FDs out of the safe range */
    for(i = 0; i < nfds ; i++){
        int curfd = fdarr[i];
        /* If the FD is already in the safe range, dup it temporarily
           in the temporary range */
        if (curfd >= safe_range_start && curfd <= safe_range_end) {
            trace("JavaVM5::Conflict %d", i);
            trace("JavaVM5::fd %d", fdarr[i]);
            trace("JavaVM5::fd target %d", tempfd);
            fdarr[i] = wrap_dup2(curfd, tempfd);
            tempfd++;
        }
    }

    /* assert fdarr[i] not in safe_range */
    /* Dup the FDs into the safe range */
    int j;
    for(i = 0, j=safe_range_start; i < nfds; i++, j++) {
        fdarr[i] = wrap_dup2(fdarr[i], j);
    }
}


/*
 * Start the actual VM, using the class path 'addClasspath'
 * in addition to anything else determined to be in the class path.
 * Determine the location of the JRE, determine the child's
 * environment (set up the env_* strings) and then do the
 * fork1. Then putenv the env_* strings and do an exec of
 * java_vm or java_vm_native
 */
int JavaVM5::StartJavaVM(const char *addClasspath)
{
    int rc;
    char *buff;
    char *env_JAVA_PLUGIN_AGENT;
    const char *progname;
    const char *agent = "No agent";
    JDresult res;
    bool    isNS4 = false;

    UNUSED(addClasspath);

    trace("********************* StartJavaVM ***************************");

    res = m_pPluginFactory->GetPluginManager()->UserAgent(&agent);
    if (JD_OK != res) 
        return res;
    env_JAVA_PLUGIN_AGENT = (char *)checked_malloc(40 + slen(agent));
    sprintf(env_JAVA_PLUGIN_AGENT, "JAVA_PLUGIN_AGENT=%s", agent);

    // Detect whether the browser is Netscape 4 by peeking agent string
    isNS4 = strstr(agent, "Gecko") != NULL ? false : true;

     /*
     * Check if we already have an Java child process.
     * We use the Unix environment to locate our saved state
     */
	
    LongTermState *pstateTmp;
    buff = getenv(stateENVName);
    if (buff != NULL) {
       //rc = sscanf(buff, "%X", &pstateTmp);
       rc = sscanf(buff, "%p", (void **) &pstateTmp);
       if (rc == 1 && pstateTmp != 0) {
           /* We already have an active connection. */
                 free(env_JAVA_PLUGIN_AGENT);
                 delete state;
	         state = pstateTmp;
                 trace("JavaVM5:reusing child process");
#if !defined(LINUX)
		 if (isNS4)
		   ConnectPipeEvent(state->work_pipe);
#endif	/* !defined(LINUX) */
                 jvm_status = JVMStatus_Running;
                 return JD_OK;
       }
    } 

    /* Get the browser's name (Mozilla-5) to store in the environment
     *  variable JAVA_PLUGIN_AGENT for the child to read to determine
     *  its version. 
     */

    /* Find directory where plugin JRE is installed */
    FindJavaDir();
    if (state->java_dir == NULL) {
        free(env_JAVA_PLUGIN_AGENT);
        plugin_formal_error("Plugin: can't find plugins/../java directory");
        return JD_ERROR_FAILURE;
    }

    /* Set up the child environment variables. Wait till after the
     * fork to actually put them into the environment.
     */
    SetupChildEnvironment();

	progname = "java_vm";

    /* Initialize the server socket for later use with attach_thread */
    if (!isNS4)
        InitializeSocket(state);    

    int JVMWorkFD = 0, JVMCommandFD = 0, JVMSpontaneousFD = 0, JVMPrintFD = 0;
    void* Work[2];
    void* Command[2];
    void* Spontaneous[2]; 
    void* Printing[2];

    /* For the worker pipe communication */
    wrap_JD_CreateSocketPair("work", Work);
    JVMWorkFD = g_unixService->JDFileDesc_To_FD(Work[1]);

    /* For the command pipe communication */
    wrap_JD_CreateSocketPair("command", Command);
    JVMCommandFD = g_unixService->JDFileDesc_To_FD(Command[1]);

    /* For the other JNI/JS communication from JVM */
    if (!isNS4) {
        wrap_JD_CreateSocketPair("spontaneous", Spontaneous);
        JVMSpontaneousFD = g_unixService->JDFileDesc_To_FD(Spontaneous[1]);
    }
    else {
        Spontaneous[0] = Spontaneous[1] = NULL;
    }
    /* For the Print communication */
    wrap_JD_CreateSocketPair("print", Printing);
    JVMPrintFD = g_unixService->JDFileDesc_To_FD(Printing[1]);

    /* Determine the full executable path name */
    char *fullprogname = (char *) checked_malloc(100 + slen(state->java_dir));
    sprintf(fullprogname, "%s/bin/%s", state->java_dir, progname);

    int nfds = 4;
    int fdarr[4] = {JVMSpontaneousFD, 
                    JVMCommandFD, JVMWorkFD, JVMPrintFD};


    /* If the vm uses native threads make these fds be blocking.
       The FDs that mozilla returns are non-blocking, at
       least when created with a "local threads" nspr.
       If the VM uses green threads it requires blocking FDs */
    if (!isNS4) {
        for(int i=0; i < nfds; i++) {
            int origflags = fcntl(fdarr[i], F_GETFL);
            int newflags = origflags & (~O_NONBLOCK);
            fcntl(fdarr[i], F_SETFL, newflags);
            trace("native vm:%d Orig flags = %X New flags = %X \n", fdarr[i],
                  origflags, newflags);
        }
    }


    char env_MOZILLA_WORKAROUND[1024];
    sprintf(env_MOZILLA_WORKAROUND, "MOZILLA_WORKAROUND=true");

    trace("JavaVM5:Ready to fork");
  
    /* Create our child java process */
#if defined(__linux__) 
    rc = fork();
#else
    rc = fork1();
#endif

    if (rc == -1) {
        free(env_JAVA_PLUGIN_AGENT);
        free(fullprogname);
        plugin_error("Fork failed: %d\n", errno);
        return JD_ERROR_OUT_OF_MEMORY;
    }

    if (rc == 0) {
        /* Child process.  Prepare to exec. */
        int i, max;

        /* Must include javaplugin.jar */
        putenv(env_LD_LIBRARY_PATH);
        /* Used in call to sun.plugin.JavaRunTime.initEnvironment */
        putenv(env_JAVA_HOME);
        /* To derive the version of mozilla calling the plugin */
        putenv(env_JAVA_PLUGIN_AGENT);
	/* Workaround Mozilla bug */
	putenv(env_MOZILLA_WORKAROUND);

	putenv(env_PLUGIN_HOME);

        if (tracing) {
            /* If PLUGIN_LD_DEBUG is set to anything pass it as LD_DEBUG*/
            char* ld_flags = getenv("JAVA_PLUGIN_LD_DEBUG");
            if (ld_flags != NULL) {
                char* ldbuf = (char *) checked_malloc(strlen(ld_flags) + 11);
                sprintf(ldbuf, "LD_DEBUG=%s", ld_flags);
                putenv(ldbuf);
            }
        }
        /* Clear the close-on-exec flags for stdio and our pipe. */
	for (i = 0; i < 3; i++) {
          fcntl(i, F_SETFD, 0);
	}
        

        /* We need to dup the FDs into a lower range since under green
           threads monitors for only the first 16 FDs are
           pre-allocated. 
           */
        int safe_range_start = MOZ5_STARTSAFEFD;
        int safe_range_end   = MOZ5_ENDSAFEFD;
        dup2_to_safety(nfds, fdarr, safe_range_start, safe_range_end);

        /* Don't close on exec for our pipes */
        for(i = 0; i < nfds; i++)
            fcntl(fdarr[i], F_SETFD, 0);
        

        /* Close all other file descriptors. */
#ifndef __linux__
        max = ulimit(UL_GDESLIM);
#else
        max = sysconf(_SC_OPEN_MAX);
#endif       

        for (i = 3; i <= max; i++) {
            int do_close = 1;
            for(int j = 0; j < nfds; j++)
                if (i == fdarr[j] ) do_close = 0;
            if (do_close) close(i);
        }

        if (tracing) {
            execlp(fullprogname, progname, "-t", 0);
        } else {
            execlp(fullprogname, progname, "", 0);
        }

        char *tmpbuff = (char *)malloc(40 + slen(fullprogname) + slen(strerror(errno)));
        sprintf(tmpbuff, "Exec of \"%s\" failed: %s\n", fullprogname, strerror(errno));
	plugin_formal_error(tmpbuff);
	/* free allocated memory to keep consistency though it need not */
	free(env_JAVA_PLUGIN_AGENT);
	free(fullprogname);
	free(tmpbuff);
        exit(6);
    }

    free(env_JAVA_PLUGIN_AGENT);
    free(fullprogname);      
    trace("JavaVM5:Continuing in parent process....");
    state->child_pid = rc;
    state->command_pipe = Command[0];
    state->work_pipe = Work[0];
    state->spont_pipe = Spontaneous[0];
    state->print_pipe = Printing[0];

    g_unixService->JD_Close(Command[1]);
    g_unixService->JD_Close(Work[1]);
    g_unixService->JD_Close(Spontaneous[1]);
    g_unixService->JD_Close(Printing[1]);

    char str[2];
    int resack = g_unixService->JD_Read(Command[0], (void*)str, 1);
    if (resack != 1) {
        plugin_formal_error("Could not read ack from child process");
	Cleanup();
	return JD_ERROR_FAILURE;
    }
      
    jvm_status = JVMStatus_Running;

    /* Allocate and record a state struct.  This is used to retrieve
     * our state from the environment if the plugin is restarted.
     * Do it here only after JVM has been successfully started. 
     */
    buff = (char* )checked_malloc(140);
    sprintf(buff, "%s=%p", stateENVName,(void *) state);
    putenv(buff);

#if !defined(LINUX)
    if (isNS4) {
        ConnectPipeEvent(Work[0]);
        return 0;
    }
#endif	/* !defined(LINUX) */

    {
        /* this is here because the PR_ stuff creates NONBlocking
           pipes and CReadbuffer doesn't like that.
        */
        int change_fd = g_unixService->JDFileDesc_To_FD(Command[0]);
     	int flags = fcntl(change_fd, F_GETFL);
	int newflags = flags & ~O_NONBLOCK;
	int res = fcntl(change_fd, F_SETFL, newflags);
	if (res == -1)
            trace("JavaVM5:\n***CHANGING FLAGS DID NOT WORK ON BROWSER\n");
	flags = fcntl(change_fd, F_GETFL);
	trace("JavaVM5::browser flags = %d\n", flags);
    }

    trace("JavaVM5::Parent OK, child pid %d\n", state->child_pid);

    spont_remotejni            = create_RemoteJNIEnv();

    CSecureJNIEnv* secjni        = new CSecureJNIEnv(NULL, spont_remotejni);

    JNIEnv* proxyJNI           = NULL;

    if (JD_SUCCEEDED((m_pPluginFactory->GetJVMManager())->CreateProxyJNI(secjni,&proxyJNI))){
        //fprintf(stderr, " Succeeding in creating proxy\n");
    }

    /* Register the mapping between proxy and remote jni */
    int spont_env_index =  m_pPluginFactory
        ->RegisterRemoteEnv(spont_remotejni, proxyJNI);

    /* Initialize the remote JNI Env with its pipe etc */
    init_RemoteJNIEnv(spont_remotejni, spont_env_index, state->spont_pipe);


    void * tid = g_unixService->JD_GetCurrentThread();

    IThreadManager *tm = NULL;

    m_pPluginFactory->GetServiceProvider()->QueryService(
                                      IJVMManager::GetCID(),
                                      JD_GET_IID(IThreadManager),
                                      (ISupports**)&tm);

    QueueRunnable * wqr = new QueueRunnable(this,
                                           worker_queue_processor,
                                           g_unixService->JDFileDesc_To_FD(state->work_pipe),
                                           (JDUint32)tid, 
                                           workMonitor, &workPipeClean,
                                           tm);

    QueueRunnable * sqr = new QueueRunnable(this,
                                           spont_queue_processor,
                                           g_unixService->JDFileDesc_To_FD(state->spont_pipe),
                                           (JDUint32)tid,
                                           spontMonitor,&spontPipeClean,
                                           tm);

    m_pPluginFactory->GetServiceProvider()->ReleaseService(
                                        IJVMManager::GetCID(),
                                        tm);

    g_unixService->JD_CreateThread( JD_USER_THREAD, QueueRunnable::threadEntry, 
                                  (void*) wqr, JD_PRIORITY_HIGH, 
                                  JD_GLOBAL_THREAD, JD_JOINABLE_THREAD, 0);


    g_unixService->JD_CreateThread( JD_USER_THREAD, QueueRunnable::threadEntry, 
                                  (void*) sqr, JD_PRIORITY_HIGH, 
                                  JD_GLOBAL_THREAD, JD_JOINABLE_THREAD, 0);

    return JD_OK;
}

/*
 * ReceivePrint polls the printpipe until it receives ok from
 * plugin
 */
void JavaVM5::ReceivePrinting(FILE *fp) {
    struct JDPollDesc pollDesc[2];
    JDIntervalTime  timeout=JD_INTERVAL_NO_TIMEOUT;
    JDIntn npds= 2;
    int finished = 0;
    char buf[1024];
    int ok;
    int n;
    int oldsize=0;

    trace("JavaVM5:Receiving printing ");
    pollDesc[0].fd = state->command_pipe;
    pollDesc[1].fd = state->print_pipe;
    pollDesc[0].in_flags = pollDesc[1].in_flags=JD_POLL_READ;
    int fd1 = g_unixService->JDFileDesc_To_FD(pollDesc[1].fd);
    
    for (;;) {
        if (g_unixService->JD_Poll(pollDesc, npds, timeout ) < 0 ) {
            continue;
        }
	if (pollDesc[1].out_flags & JD_POLL_READ) {
	  /* Currently the printing only works for Netscape 4.x on Soalris
	     using JD_Read will block the call forever, so we use read explicitly
	  */
	    n = read(fd1, buf, sizeof(buf));
	  if ( n > 0 ) {
	      fwrite(buf, n, 1, fp);
	  } else if ( n < 0 ) {
	    fprintf(stderr, "Error reading print pipe %d\n", g_unixService->JD_GetError());
	    break;
	  }
        } 
       
	if (finished) break;

        if (pollDesc[0].out_flags & JD_POLL_READ) {
            CReadBuffer rb(g_unixService->JDFileDesc_To_FD(pollDesc[0].fd));
            if ((rb.getInt(&ok) > 0) && (JAVA_PLUGIN_OK == ok)) {
                finished = 1;
                timeout = 0;
            }
        }
    }
   
}

jobject
JavaVM5::GetJavaObjectForInstance(int plugin_number) {
  /* This method is provided for Javascript to Java communication
     For Usual Javascript to Java call, we need to wait for
     applet start method is called. The only exception for
     this is when the Javascript call is triggered by the
     Java call. Since we need to support Java to Javascript call
     during applet init method, we can't wait to the applet start method
     to be called. In that case, when applet is fully loaded, we can
     safely return the applet object to the browser. See detail
     from bug 4821301 & 4920552.*/
    
  JavaPluginInstance5* pluginInstance;
  pluginInstance = m_pPluginFactory->GetInstance(plugin_number);
  if (pluginInstance == NULL)
    return NULL;

  /* If 'spontPipeClean' is true, meaning the javascript call is
	 not triggered by Java call, so we need to wait until the
	 applet start method gets called, 
	 Otherwise, we don't block since javascript call is triggered 
	 by the java call.
  */
  while (spontPipeClean && (pluginInstance->GetStatus()) < APPLET_START) {
    /* Wait for the applet notify us that APPLET IS STARTED
       while waiting, we need to poll the worker queue since
       there maybe a request from java side such as cookie/proxy
       request. However, don't call to ProcessWorkQueue to handle work 
       queue messages forever, but only process one message a time
       to allow break out of this loop when applet is
       destroyed before it gets started.
     */
    DoWork();
  }

  if (pluginInstance->GetStatus() >= APPLET_DESTROY)
    return NULL;

  CWriteBuffer wb;
  int res;
  wb.putInt(JAVA_PLUGIN_GET_INSTANCE_JAVA_OBJECT);
  wb.putInt(plugin_number);
  SendRequest(wb, TRUE, TRUE, &res);
  return (jobject) res;
}

/*
 * This is called when Netscape is about to unload the library.
 * All our plugin instances should be closed down at this point.
 * We keep our Java child process alive for reuse later.
 */
void
JavaVM5::ShutdownJavaVM(int RealShutdown)
{
    CWriteBuffer wb;
    trace("JavaVM5:Shutdown");

    /* This is just advice, the java_vm process will actually stick around */
    wb.putInt(JAVA_PLUGIN_SHUTDOWN);
    SendRequest(wb, FALSE);

    // Clean off our work queue
    ProcessWorkQueue();

    /* free memory allocated to environment variable */
    free(env_LD_LIBRARY_PATH);
    free(env_JAVA_HOME);
    free(env_PLUGIN_HOME);
    if (RealShutdown) {
//        putenv((char *) "JAVA_PLUGIN_STATE=0x0");
//        close(state->command_pipe);
//        close(state->work_pipe);
//        free(state);
//        state = NULL;
    }

#if !defined(LINUX)
    if (state->inputid)
        XtRemoveInput(state->inputid);
#endif	/* !defined(LINUX) */
}

/* Handle requests on the spontaneous pipe. These requests are handled
   similarly to requests on */
void
JavaVM5::ProcessSpontaneousQueue() {

    int sp = g_unixService->JDFileDesc_To_FD(state->spont_pipe);
    if (sp < 0) {
        trace("JavaVM5:spont pipe is dead");
        return;
    }
	 
	int rc;
	struct pollfd fds[1];
    fds[0].fd = sp;
    fds[0].events = POLLRDNORM;
    for (;;) {
        int rc;
        rc = poll(fds, 1, 0);
        if (rc < 1) {
            trace("JavaVM5:No work on spont pipe");
            g_unixService->JD_EnterMonitor(spontMonitor);
            spontPipeClean = true;
            g_unixService->JD_NotifyAll(spontMonitor);
            g_unixService->JD_ExitMonitor(spontMonitor);

            break;
        }
        /* We have work in the pipe */
        int request_code = 0;

        trace("JavaVM5:Spontaneous thread waiting for next request...");

        read_JD_fully("Spont Req", state->spont_pipe, (char *)&request_code, 4);

        trace("Received request code:%d\n", request_code);

        switch (request_code) {
        case JAVA_PLUGIN_REQUEST: 
            {
                /* This is a JS call from a spontaneous thread */
                trace("JavaVM5:SPONTANEOUS CALL!!! (JAVA_PLUGIN_REQUEST)");
				spontPipeClean = false;
                JSHandler(spont_remotejni);
                break;
            }
        default:
            plugin_error("Did not understand spontaneous code %X\n", 
                         request_code);
        }
    }
}



/* 
 * Inform plugin to terminate current request without a reply
 */
void JavaVM5::TerminateRequestAbruptly(const char* infoMsg) {
    trace("JavaVM5::Abruptly terminating request %s", infoMsg);
   
    CWriteBuffer wb;
    wb.putInt(JAVA_PLUGIN_REQUEST_ABRUPTLY_TERMINATED);
    SendRequest(wb, FALSE);

}


/*
 * This method is called when we detect that our child process has 
 * sent us a work request that we ought to process.
 */
void JavaVM5::DoWork()
{
    int code; 
    short ix;
    JavaPluginInstance5* pluginInstance;

    if (state->work_pipe < 0) {
        plugin_error("JavaVM5::work pipe is dead");
        m_pPluginFactory->ExitMonitor("ProcessWorkQueue-1");
        return;
    }

    CReadBuffer rb(g_unixService->JDFileDesc_To_FD(state->work_pipe));
    
    // First read the request code. The special value of 0 is used
    // to indicate that the child is ready and is otherwise ignored.
    rb.getInt(&code);
    
    trace("JavaVM5::Obtained next work code %d\n", code);

    rb.getShort(&ix);
    pluginInstance = m_pPluginFactory->GetInstance(ix);

    /* Now handle the normal cases */
    if (code == JAVA_PLUGIN_SHOW_STATUS) {
        char *mess;
        int rc = rb.getString(&mess);
        if (rc < 0)
	    mess=strdup(" ");
        if (pluginInstance != NULL) {
            IPluginInstancePeer *peer = NULL;
            pluginInstance->GetPeer(&peer);
            if (peer != NULL) {
                peer->ShowStatus(mess);
                peer->Release();
            } else {
                plugin_error("No peer found for show status!");
            }
        }
        if (rc < 0) 
            free(mess);
        else 
            rb.free(mess);
    } else if (code == JAVA_PLUGIN_SHOW_DOCUMENT) {
        char *url;
        char *target;
         rb.getString(&url);
         rb.getString(&target);
        if (url == NULL || target == NULL) {
            WorkError(4);
            return;
        }
        trace("JavaVM5::Show document URL %s\n", url);
        trace("JavaVM5::Show document target %s\n", target);
        if (pluginInstance != NULL) {
            IPluginManager *mgr = m_pPluginFactory->GetPluginManager();
            JDresult rv  = mgr->GetURL((IJVMPluginInstance*)pluginInstance, url, 
							    target, NULL);
	    if (JD_SUCCEEDED(rv))
	      trace("JavaVM5:Return from GetURL OK");
	    else
	      trace("JavaVM5:Return from GetURL FAIL");
        }
        rb.free(url);
        rb.free(target);
    } else if (code == JAVA_PLUGIN_FIND_PROXY) {
        char *url;
        char *host;
        rb.getString(&url);
        rb.getString(&host);
	if (url == NULL || host == NULL) {
	  WorkError(5);
	  return;
	}

	if (pluginInstance != NULL) {
	  m_pPluginFactory->GetProxySupport()->ProxmapFindProxy((IPluginInstance*) pluginInstance, url, host);
	} else {
	  TerminateRequestAbruptly("FindProxy");
	}
        rb.free(url);
        rb.free(host);
    } else if (code == JAVA_PLUGIN_FIND_COOKIE) {
        char *url;
        rb.getString(&url);
	if (pluginInstance != NULL) {
	    m_pPluginFactory->GetCookieSupport()->FindCookieForURL(pluginInstance,url);
	}
	else {
	   TerminateRequestAbruptly("JavaScriptRequest");
        }
        rb.free(url);
    } else if (code == JAVA_PLUGIN_SET_COOKIE) {
        char *url;
        char *cookie;
        rb.getString(&url);
        rb.getString(&cookie);
	if (pluginInstance != NULL) {
	    m_pPluginFactory->GetCookieSupport()->SetCookieForURL(url, cookie);
	}
	else {
	  TerminateRequestAbruptly("JavaScriptRequest");
        }
        rb.free(url);
        rb.free(cookie);
    } // This is for Netscape 4.x browser only
    else if (code == JAVA_PLUGIN_JAVASCRIPT_REQUEST) {
        short flag;  // outdated see comment in Worker.java:sendJSRequest
        rb.getShort(&flag);
        char* url;
        rb.getString(&url);
        if (url == NULL) {
            WorkError(8);
            return;
        }
        if (pluginInstance != NULL) {
            m_pPluginFactory->GetPluginManager()->GetURL(
                (IPluginInstance*)pluginInstance, url, NULL,
                (IPluginStreamListener*)JAVA_PLUGIN_JAVASCRIPT_REQUEST);
        }
        else {
            TerminateRequestAbruptly("JavaScriptRequest");
        }
        free(url);
    }
	else if (code == JAVA_PLUGIN_STATUS_CHANGE) {
	    short status;
	    rb.getShort(&status);
	    if (pluginInstance != NULL) {
                pluginInstance->SetStatus(status);
        }
	    // If we receive notification from the java_vm process
	    // that we're destroying the applet, unlock the plugin
	    // instance index associated with that applet for later
	    // reuse
	    if (status == APPLET_DESTROY) {
                m_pPluginFactory->UnlockInstanceIndex(ix);
	    }
	}
    else {
        plugin_formal_error("Plugin: unexpected work request from child");
        plugin_error("Code = %0x", code);
    }
}

/*
 * Clean up the java process since an error has occurred 
 */
void JavaVM5::Cleanup(void) {

    plugin_formal_error("Plugin: Java VM process has died.");
        
    jvm_status = JVMStatus_Failed;
    /* Wipe out the JAVA_PLUGIN_STATE environment variable, close pipes */
    if (state->command_pipe >= 0) {
        g_unixService->JD_Close(state->command_pipe);
        state->command_pipe = 0;
    }
    if (state->work_pipe >= 0) {
        g_unixService->JD_Close(state->work_pipe);
        state->work_pipe = 0;
    }
    if (state->spont_pipe >= 0) {
        g_unixService->JD_Close(state->spont_pipe);
        state->spont_pipe = 0;
    }
    if (state->print_pipe >= 0) {
        g_unixService->JD_Close(state->print_pipe);
        state->print_pipe = 0;
    }

//        char *buff;
//        free(state);
//        state = NULL;
//       buff = (char *)checked_malloc(40);
//        sprintf(buff, "JAVA_PLUGIN_STATE=XXX");
//        putenv(buff);

    trace("JavaVM5:Cleaned up child state");
    trace("JavaVM5:Waiting for child process to termiante ");
    /* Wait for the child process to terminate */
    if (state->child_pid != 0) {
        int stat = 0;
        int rc = waitpid(state->child_pid, &stat, WNOHANG);
        if (rc > 0) {
            if (WIFEXITED(stat)) {
                fprintf(stderr, "%s %d\n",
                        DGETTEXT(JAVA_PLUGIN_DOMAIN, 
                                 "plugin: java process exited with status"), 
                        WEXITSTATUS(stat));
            } else if (WIFSIGNALED(stat)) {
                fprintf(stderr,"%s %d\n", 
                        DGETTEXT(JAVA_PLUGIN_DOMAIN, 
                                 "plugin: java process died due to signal"), 
                        WTERMSIG(stat));
                if (WCOREDUMP(stat)) {
                    fprintf(stderr, "%s\n", 
                            DGETTEXT(JAVA_PLUGIN_DOMAIN, 
                                     "  a core file was generated"));
                }
            }
        }
        state->child_pid = 0; 
    }
}



/* Send the message to create a new applet */
void JavaVM5::CreateApplet(const char *appletType, int appletNumber, int argc,
                           char **argn, char **argv) {
    CWriteBuffer wb;
    int i;
    
    trace("JavaVM JAVA_PLUGIN_NEW creating applet %d\n", appletNumber);
    wb.putInt(JAVA_PLUGIN_NEW);
    wb.putInt(appletNumber);

    if (strstr(appletType, "bean")) {
        /* It's a bean */
        wb.putInt(1);
    } else {
        /* Its a regular applet */
        wb.putInt(0);
    }

    wb.putInt(argc);
    for (i = 0; i < argc; i++) {
        const char *s;
        wb.putString(argn[i]);

        s = argv[i];
	
	if(strcasecmp("mayscript",argn[i]) == 0) {
		if(s == NULL || slen(s) < 1 || s[0] == ' ') {
			s = "true";
		}
	}
	
        wb.putString(s);
    }
    SendRequest(wb, TRUE);
}

/*
    During the period of waiting for the reply,
    we need to poll the spontaneous pipe as well,
    since Java side may waiting for the java callback to finish
 */
JD_METHOD JavaVM5::WaitingForReply(int commandPipe, int& reply)
{
  if (commandPipe == 0)
	return JD_ERROR_ILLEGAL_VALUE;

  JDresult res;

  if (state->spont_pipe != 0) {
	struct pollfd fds[2];
	fds[0].fd = g_unixService->JDFileDesc_To_FD(state->spont_pipe);
	fds[0].events = POLLRDNORM;
	fds[1].fd = commandPipe;
	fds[1].events = POLLRDNORM;
	int rv;

	if (fds[0].fd < 0) {
	  trace("JavaVM5:spont pipe is dead");
	  return JD_ERROR_FAILURE;
	}
	for (;;) {
	  fds[0].revents = 0;
	  fds[1].revents = 0;

	  rv = poll(fds, 2, -1);
	  if (rv == -1) {
		if (errno != EINTR)
		  return JD_ERROR_FAILURE;
	  }
	  else {
		if (fds[1].revents & POLLRDNORM) {
		  break;
		}

		if (fds[0].revents & POLLRDNORM) {
		  ProcessSpontaneousQueue();
		}
	  }
	}
  }

  CReadBuffer rb(commandPipe);
  if (rb.getInt(&reply) <= 0)
	res = JD_ERROR_FAILURE;
  else
	res = JD_OK;

  return res;
}

/*
     * Send a request to the VM on the command pipe. Depending on
     * wait_for_reply either do or do not wait for an acknowledgement.
     */
void JavaVM5::SendRequest(const CWriteBuffer& wb, int wait_for_reply, bool wait_for_return, int *result)
{
    int rc;
    int reply;
    static int request_id = 0;  // To identify requests

    /* Pipe monitor must be held */
    m_pPluginFactory->EnterMonitor("SendRequest");

    request_id++;
    trace("JavaVM5:Sending command ");

    /* Check that the command pipe is alive */
    if (state->command_pipe <= 0) {
        plugin_error("SendRequest: Bad pipe. Dead VM?");
        m_pPluginFactory->ExitMonitor("SendRequest-badpipe");
        return;
    }

    if (!wb.send(g_unixService->JDFileDesc_To_FD(state->command_pipe))) {
	/* Write the message to the command pipe */
	plugin_error("SendRequest: write failed. Dead VM? %d\n",
			errno);
	m_pPluginFactory->ExitMonitor("SendRequest-failedread");
	Cleanup();
	return;
    }
    rc = g_unixService->JD_Sync(state->command_pipe);

    /* See if we need an ACK from the VM. Return, if not */
    if (wait_for_reply == 0) {
        trace("JavaVM5:SendRequest: Wrote request. No reply needed.");
        m_pPluginFactory->ExitMonitor("SendRequest-noreply");
        return;
    }
    
    /* Get the ACK */
    trace("JavaVM5:SendRequest: Wrote request. Waiting for ack.");
    int cmdPipe = g_unixService->JDFileDesc_To_FD(state->command_pipe);
    if (JD_FAILED(WaitingForReply(cmdPipe, reply))) {
      plugin_error("SendRequest: Read of ack failed: %d\n", errno);
      m_pPluginFactory->ExitMonitor("SendRequest-replynotread");
      Cleanup();
      return;
    }

    /* Verify that the ack is OK */
    if (reply == JAVA_PLUGIN_OK) {
      if (wait_for_return == TRUE) {
	CReadBuffer rb(cmdPipe);
	rb.getInt(result);
      }
      trace("JavaVM5::SendRequest: Read OK acknowledgement %d\n", request_id);
    } else {
      Cleanup();
      plugin_formal_error("Java Plug-in ERROR");
      plugin_error("JavaVM5::SendRequest: Got an erroneous ack. %d %d\n", request_id,
		   reply);
    }

    m_pPluginFactory->ExitMonitor("SendRequest-ok");
    return;
}

/* Create a new remote JNI Env for a given secure jni env  */
RemoteJNIEnv* JavaVM5::CreateRemoteJNIEnv(JNIEnv *proxyenv) {
    m_pPluginFactory->EnterMonitor("createenv");

    RemoteJNIEnv *env =  create_RemoteJNIEnv();
    
    int env_index = m_pPluginFactory->RegisterRemoteEnv(env, proxyenv);

    CWriteBuffer wb;
    wb.putInt(JAVA_PLUGIN_ATTACH_THREAD);
    wb.send(g_unixService->JDFileDesc_To_FD(state->command_pipe));

    write_JD_fully("SendingAttachPort", state->command_pipe, 
                   (char*)&(state->port), 4);

    JDNetAddr result_addr;

    void* fd = g_unixService->JD_Accept(state->server_socket, &result_addr, 1000000);
    
    if (fd == NULL) {
        plugin_error("Did not accept a connection");
    }

    int junk_ack;
    g_unixService->JD_Read(fd, &junk_ack, 4);

    trace("JavaVM5:Read the initial ack");

    if (junk_ack != 5050) 
        plugin_error("Could not read initial ack over the new fd");

    junk_ack++;

    g_unixService->JD_Write(fd, &junk_ack, 4);

    trace("JavaVM5:Wrote the initial ack");

    int change_fd = g_unixService->JDFileDesc_To_FD(fd);

    {
        int flags = fcntl(change_fd, F_GETFL);
        /*      int newflags = flags  | O_NONBLOCK; */
        int newflags = flags & ~O_NONBLOCK;
        int res = fcntl(change_fd, F_SETFL, newflags);
        if (res == -1)
            fprintf(stderr, "\n***CHANGING FLAGS DID NOT WORK ON BROWSER\n");
        flags = fcntl(change_fd, F_GETFL);
        trace("JavaVM5::browser flags = %d\n", flags);
    }

    m_pPluginFactory->ExitMonitor("createenv");

    init_RemoteJNIEnv(env, env_index, fd);

    {      
        for(int i = 0; i < 2; i++) {
            env->FindClass("java/lang/System");
            env->ExceptionClear();
            env->ExceptionOccurred();
        }
    }
    return env;

}

/*
 * Define the variables needed by the child process. This merely initializes
 * the env_ variables. They are later putenv'd after the fork.
 */
void JavaVM5::SetupChildEnvironment(void) {
  trace("JavaVM5::SetupChildEnvironment");

  // Figure out which JRE to use.
  char *jre = FindJRE();
  trace("JavaVM5::Using JRE from %s", jre);

  char *ld_path = getenv("LD_LIBRARY_PATH");
  char *buff = (char *)malloc(300 + slen(state->java_dir) + 5*slen(jre)
                              + 5*slen(LIBARCH) + slen(ld_path));

  // The ARCH macro is defined in the makefile.  E.g. -DARCH="sparc"
  // Be careful of the LD_LIBRARY_PATH - beware the dummy libhpi
  // and libjvm.so
  //   In 1.2 the non-dummy versions are in
  //        <jre>/lib/sparc/<thread_type>/libhpi.so
  //  If jre is the same as state->javadir (typically ~/.netscape/java)
  //  then if jre/lib is too early it ends up pointing to
  // .netscape/java/lib
  //  and picks up the (wrong, dummy) version of libhpi etc which are
  //   only meant for 1.1. So put the thread specific directory first
  //   Also: The 1.2fcs build places libjvm in
  //          jre/lib/<arch>/classic/libjvm.so
  // (to permit hotspot later).  So this path is only needed for 1.2

      sprintf(buff, "LD_LIBRARY_PATH=%s/lib/%s/client:%s/lib/%s",
              jre, LIBARCH,                // libjvm
              jre, LIBARCH);               // libjava, libawt & friends

  if (slen(ld_path) > 0) {
    // Append the user's LD_LIBRARY_PATH
    strcat(buff, ":");
    strcat(buff, ld_path);
  }
  trace("JavaVM5::Library path is %s\n", buff);
  env_LD_LIBRARY_PATH = buff;

  buff =(char *) malloc(100 + slen(jre));
  sprintf(buff, "JAVA_HOME=%s", jre);
  free(jre);

  trace("JavaVM5::JAVA_HOME is %s\n", buff);
  env_JAVA_HOME = buff;

  buff =(char *) malloc(100 + slen(state->java_dir));
  sprintf(buff, "PLUGIN_HOME=%s", state->java_dir);

  trace("JavaVM5::PLUGIN_HOME is %s\n", state->java_dir);
  env_PLUGIN_HOME = buff;
}

/* code copied from JRE 1.4 java_props_md.c, then
 * changed to prevent memory leak in caller */
char* sysGetOsName(void) {

   static struct utsname name = {{0}};
   if (!name.sysname[0])
	uname(&name);
   return name.sysname;
}

/* code copied from JRE 1.4 java_props_md.c */
char* sysGetOsArch(void) {

  char arch[12];

#ifdef LINUX
  return "i386";
#else
  sysinfo(SI_ARCHITECTURE, arch, sizeof(arch));
  if (strcmp(arch,"sparc") == 0 ) {
#ifdef _LP64
    return "sparcv9";
#else
    return "sparc";
#endif
  } else if (strcmp(arch,"i386") == 0 ) {
    /* use x86 to match the value received on win32 */
    return "x86";
  } else if (strcmp(arch,"ppc") == 0 ) {
    return "ppc";
#ifdef __linux__
  } else if (strcmp(arch,"m68k") == 0 ) {
    return "m68k";
#endif
  } else {
    return "Unknown";
  }
#endif
}


char *JavaVM5::FindJRE(void) {
    FILE *fin;
    char *home = NULL;
    char path[1024];
    char line[200];
    char jre[200];
    char jre_osname[200];
    char jre_osarch[200];
    int rc;

    home  = getenv("USER_JPI_PROFILE");
    if(home == NULL)
    {
	home  = getenv("HOME");
    }

    /* Look in the properties file for javaplugin.jre.path */
    sprintf(path, "%s/.java/deployment/deployment.properties", home); 

    fin = fopen(path, "r");
    if (fin == NULL) {
        return strdup(state->java_dir);
    }

    jre[0] = 0;
    jre_osname[0] = 0;
    jre_osarch[0] = 0;
	char JREPath[200];
	char JREOsName[200];
	char JREOsArch[200];
	sprintf(JREPath, "deployment.javapi.jre.%s.path", PLUGIN_VERSION);
        strcat(JREPath, "=%s");
	sprintf(JREOsName, "deployment.javapi.jre.%s.osname", PLUGIN_VERSION);
        strcat(JREOsName, "=%s");
	sprintf(JREOsArch, "deployment.javapi.jre.%s.osarch", PLUGIN_VERSION);
        strcat(JREOsArch, "=%s");
    while (fgets(line, sizeof(line), fin)) {
        trace_verbose("%s:%s\n", path,line);
        sscanf(line, JREPath, (char*) &jre);
	sscanf(line, JREOsName, (char*) &jre_osname);
	sscanf(line, JREOsArch, (char*) &jre_osarch);
    }
    fclose(fin);

    /* check if osarch and osname matches, if they exist */
    if ((jre_osname[0] == 0 && jre_osarch[0] == 0) ||
	(jre_osname[0] != 0 && !strcmp(jre_osname, sysGetOsName()) &&
	jre_osarch[0] != 0 && !strcmp(jre_osarch, sysGetOsArch()))) {

      /* If there is a javaplugin.jre.path defined, verify that it is ok */
      if (jre[0] != 0) {
	int len = slen(jre)+1;
        /* First check if the property is set to "Default" .
         */
        if (len > 4) {
	  const char* jrePrefix = "Default";
	  int match = TRUE;
	  for(int i = strlen(jrePrefix) - 1; i >= 0; i--) {
	    if (jre[i] != jrePrefix[i]) 
	      match = FALSE;
	  }
	  if (match) 
	    return strdup(state->java_dir);
        }
	
	/* Now check if that directory is reasonable */
        struct stat sbuf;
        sprintf(path, "%s/lib", jre);
        rc = stat(path, &sbuf);
        if (rc == 0) {
	  /* We've been given a plausible JRE string */
	  return strdup(jre);
        }
        plugin_formal_error("Java property javaplugin.jre.path defined as");
        plugin_raw_formal_error(jre);
        plugin_formal_error("But that directory does not exist."); 
        plugin_formal_error("Using JRE from");
        plugin_raw_formal_error(state->java_dir);
      }
    }
    return strdup(state->java_dir);
}


void JavaVM5::FindJavaDir(void) {
    Dl_info dlinfo;
    static int dummy;
    char buf[PATH_MAX];
    char real[PATH_MAX];

    // Now: Only use the JRE which came with the plugin.

    // Formerly:
    //   Use the NPX_PLUGIN_PATH to find the directories holding plugins
    //   Find the first of those that has a ../java directory.

    state->java_dir = NULL;
    dladdr((void *)&dummy, &dlinfo);
    strcpy(buf, dlinfo.dli_fname);

    if (realpath(buf, real) == NULL) {
        fprintf(stderr, "Error: realpath(`%s') failed.\n", buf);
        return;
    }

    *(strrchr(real, '/')) = '\0';  /*     executable file  */
    *(strrchr(real, '/')) = '\0';  /*     $(ARCH)          */
    *(strrchr(real, '/')) = '\0';  /*     $(LIB)           */

    state->java_dir = strdup(real);
}


/*
 * Report an error in performing work, and terminate the jvm
 */
void JavaVM5::WorkError(int x) {
    perror("Plugin worker error");
    fprintf(stderr, "%s (%d)\n",
            DGETTEXT(JAVA_PLUGIN_DOMAIN, 
                     "Plugin: trouble with work request from child"),
            x);
    Cleanup();
}

void* JavaVM5::GetWorkPipe() {
    return state->work_pipe;
}
