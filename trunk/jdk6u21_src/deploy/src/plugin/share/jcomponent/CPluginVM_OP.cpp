/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "CPluginVM_OP.h"
#include "CReadBuffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <ulimit.h>
#include <limits.h>
#include <dlfcn.h>

void workPipeEntry(void * data) {

    fprintf(stderr,"In workPipeEntry\n");
    CPluginVM_OP * vm = (CPluginVM_OP *) data;

    vm->workPipeHandler();

}

CPluginVM_OP::PairsTable::InstanceCBPair::InstanceCBPair(JavaID i, 
                                                         IPluginVMCB * lcb) {
    instance = i;
    cb = lcb;
    next = NULL;
}

CPluginVM_OP::PairsTable::InstanceCBPair::~InstanceCBPair() {
}

CPluginVM_OP::PairsTable::PairsTable() {
    int x;
    for(x=0;x<table_size;x++) {
        m_table[x] = NULL;
    }
}

void CPluginVM_OP::PairsTable::add(JavaID jid, IPluginVMCB * cb) {
    
    InstanceCBPair * pair = new InstanceCBPair(jid,cb);
    int location = (table_size - 1) & jid;
    pair->next = m_table[location];
    m_table[location] = pair;
}

void CPluginVM_OP::PairsTable::remove(JavaID jid) {

    int location = (table_size - 1) & jid;
    InstanceCBPair * pair = m_table[location];
    InstanceCBPair * previous = NULL;
    while( pair != NULL ) {
        if (pair->instance == jid) {
            if (previous != NULL) {
                previous->next = pair->next;
            } else {
                m_table[location] = pair->next;
            }
            delete pair;
            break;
        }
        previous = pair;
        pair = pair->next;
    }
}

IPluginVMCB * CPluginVM_OP::PairsTable::find(JavaID jid) {

    if ( -1 == jid) {
        // Just return a random instance
        int i;
        for(i=0;i<table_size;i++) {
            if(m_table[i] != NULL) {
                return m_table[i]->cb;
            }
        }
    } else {
        int location = (table_size - 1) & jid;
        InstanceCBPair * pair = m_table[location];
        InstanceCBPair * previous = NULL;
        while( pair != NULL ) {
            if (pair->instance == jid) {
                return pair->cb;
            }
            previous = pair;
            pair = pair->next;
        }
    }
    
    return NULL;
}

CPluginVM_OP::CPluginVM_OP(IFDMonitor * fdmon,const char * userAgent) {

    fprintf(stderr,"In CPluginVM_OP::CPluginVM_OPv2\n");
    m_FDMonitor = fdmon;
    m_userAgent = userAgent;
    m_ltstate = new LongTermState();
    m_ltstate->vmstatus = LongTermState::nONE;
    m_ltstate->instance_count = 0;

    findContainingJRE();
}

CPluginVM_OP::~CPluginVM_OP() {
   fprintf(stderr,"In ~CPluginVM_OP()\n");
}

void CPluginVM_OP::connectPipeMonitor() {

    m_FDMonitor->connectFD(m_ltstate->work_pipe,workPipeEntry,this);
}

bool CPluginVM_OP::startVM() {
    bool ret;
    switch (m_ltstate->vmstatus) {
    case LongTermState::rUNNING:
        ret = true;
        break;
    case LongTermState::fAILED:
        ret = false;
        break;
    case LongTermState::nONE:
        ret = forkAndExecVM();
        break;
    default:
        break;
    }

    return ret;
}

bool CPluginVM_OP::forkAndExecVM() {
    pid_t fork_return;
    int command_fd[2];
    int worker_fd[2];
    int spont_fd[2];
    int print_fd[2];

    if (createPipe(spont_fd) != 0) {
        m_ltstate->vmstatus = LongTermState::fAILED;
        return false;
    } 
    if (createPipe(command_fd) != 0) {
        m_ltstate->vmstatus = LongTermState::fAILED;
        return false;
    }

    if (createPipe(worker_fd) != 0) {
        m_ltstate->vmstatus = LongTermState::fAILED;
        return false;
    } 

    if (createPipe(print_fd) != 0) {
        m_ltstate->vmstatus = LongTermState::fAILED;
        return false;
    } 
    
    int nfds = 4;
    int fdarr[] = {spont_fd[1],command_fd[1],worker_fd[1],print_fd[1]};

#ifdef __linux__
    fork_return = fork();
#else
    fork_return = fork1();
#endif

    if (fork_return == -1) {
        return false;
    } else {
        if (fork_return == 0) {
            // Child Process
            int i;
            // Don't close stdin, stdout, sdterr on exec 
            for (i = 0; i < 3; i++) {
                fcntl(i, F_SETFD, 0);
            }
            dup2ToSafety(nfds, fdarr, STARTSAFEFD, ENDSAFEFD);
    
            // Don't close our pipes on exec
            for(i = 0; i < nfds; i++) {
                fcntl(fdarr[i], F_SETFD, 0);
            }
            
            // Close everyone else
#ifndef __linux__
            int max = ulimit(UL_GDESLIM);
#else
            int max = sysconf(_SC_OPEN_MAX);
#endif
            for (i = 3; i <= max; i++) {
                int do_close = 1;
                for(int j = 0; j < nfds; j++)
                    if (i == fdarr[j] ) do_close = 0;
                if (do_close) {
                    close(i);
                }
            }
    
            setupChildENV();
            char pathToExec[PATH_MAX];
            char * command = "java_vm";

            sprintf(pathToExec,"%s/bin/%s",m_ltstate->java_dir,command);
            if (getenv("JAVA_PLUGIN_TRACE") != NULL) {
                execlp(pathToExec,command,"-t",NULL);
            } else {
                execlp(pathToExec,command,"",NULL);
            }
    
        }
    }
    // Parent Process
    m_ltstate->child_pid = fork_return;
    m_ltstate->command_pipe = command_fd[0];
    m_ltstate->work_pipe = worker_fd[0];
    int i;
    for(i = 0;i < nfds; i++ ) {
        close(fdarr[i]);
    }
    // Child VM will put byte 17 on the command pipe
    // to test if everything is ok
    char ack_byte;
    CReadBuffer rb(m_ltstate->command_pipe);
    rb.getByte(&ack_byte);
    
    connectPipeMonitor();
    m_ltstate->vmstatus = LongTermState::rUNNING;
    return true;
}

int CPluginVM_OP::createPipe(int fds[2]) {
    int res = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    return res;
}

void CPluginVM_OP::dup2ToSafety(int nfds, int fdarr[],
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
            fdarr[i] = dup2(curfd, tempfd);
            tempfd++;
        }
    }

    /* assert fdarr[i] not in safe_range */
    /* Dup the FDs into the safe range */
    int j;
    for(i = 0, j=safe_range_start; i < nfds; i++, j++) {
        fdarr[i] = dup2(fdarr[i], j);
    }
}

void CPluginVM_OP::findContainingJRE() {
    Dl_info dlinfo;
    static int dummy;
    char buf[PATH_MAX];
    char real[PATH_MAX];

    m_ltstate->java_dir = NULL;
    dladdr((void *)&dummy, &dlinfo);
    strcpy(buf, dlinfo.dli_fname);

    if (realpath(buf, real) == NULL) {
        fprintf(stderr, "Error: realpath(`%s') failed.\n", buf);
        return;
    }

    *(strrchr(real, '/')) = '\0';  /* library file         */
    *(strrchr(real, '/')) = '\0';  /* $(ARCH) subdirectory */
    *(strrchr(real, '/')) = '\0';  /* lib subdirectory     */

    m_ltstate->java_dir = strdup(real);
}

void CPluginVM_OP::setupChildENV() {
    
    const char * ld_lib_str = "LD_LIBRARY_PATH=";
    char our_ld_paths[2*PATH_MAX + 2];
    const char * ld_lib_path = getenv("LD_LIBRARY_PATH");

    char * jre = m_ltstate->java_dir;
    
    // ARCH is passed in on the compile line
    sprintf(our_ld_paths,"%s/lib/%s/client:%s/lib/%s",jre,ARCH,jre,ARCH);

    char * new_ld_lib_path;

    if (ld_lib_path != NULL) {
        new_ld_lib_path = (char *) malloc(strlen(ld_lib_str) + 
                                          strlen(our_ld_paths) +
                                          strlen(":") + 
                                          strlen(ld_lib_path) + 1);
        sprintf(new_ld_lib_path,"%s%s:%s",ld_lib_str,our_ld_paths,ld_lib_path);
    } else {
        new_ld_lib_path = (char *) malloc(strlen(ld_lib_str) + 
                                          strlen(our_ld_paths) + 1);
        sprintf(new_ld_lib_path,"%s%s",ld_lib_str,our_ld_paths);
    }

    putenv(new_ld_lib_path);
    
    // This is the location of the JRE the user want us to use
    // Possible set via the ControlPanel
    const char * java_home_str = "JAVA_HOME=";
    char * java_home = (char *) malloc(strlen(java_home_str) + strlen(jre) + 1); 
    sprintf(java_home,"%s%s",java_home_str,jre);
    putenv(java_home);

    // This is the location of the JRE we came with
    const char * plugin_home_str = "PLUGIN_HOME=";
    char * plugin_home = (char *) malloc(strlen(plugin_home_str) +
                                         strlen(m_ltstate->java_dir) + 1); 
    sprintf(plugin_home,"%s%s",plugin_home_str,m_ltstate->java_dir);
    putenv(plugin_home);

    char * jpe = (char *) malloc(strlen("JAVA_PLUGIN_AGENT=") +
                                 strlen(m_userAgent) + 1);
    sprintf(jpe,"JAVA_PLUGIN_AGENT=%s",m_userAgent);
    putenv(jpe);
}

void CPluginVM_OP::workPipeHandler() {
   
fprintf(stderr,"In workpipehandler\n");
    CReadBuffer rb(m_ltstate->work_pipe);

    int code;
    short instance;

    rb.getInt(&code); 
    rb.getShort(&instance);
    IPluginVMCB * cb = m_pairsTable.find(instance);
    fprintf(stderr,"Code = %X CB=%X instance=%d\n",code,cb,instance);
    switch(code) {
    case JAVA_PLUGIN_SHOW_STATUS:
        char * mess;
        rb.getString(&mess);
	if (cb != NULL)
            cb->showStatus(mess);
        rb.free(mess);
        break;
    case JAVA_PLUGIN_SHOW_DOCUMENT:
        char * url;
        char * target;
        rb.getString(&url);
        rb.getString(&target);
        fprintf(stderr,"CPluginVM SHOW_DOCUMENT\n");
	if (cb != NULL)
            cb->showDocument(url,target);
        rb.free(url);
        rb.free(target);
        break;
    case JAVA_PLUGIN_FIND_PROXY:
        char * host;
        char * proxy;
        rb.getString(&url);
        rb.getString(&host);
        if ((cb == NULL) || cb->findProxy(url, &proxy)) {
            requestAbruptlyTerminated();
        } else {
        fprintf(stderr,"CPluginVM proxy is: %s\n",proxy);
            proxyMapping(url,proxy);
            free(proxy);
        }
        rb.free(url);
        rb.free(host);
        break;
    case JAVA_PLUGIN_FIND_COOKIE:
        char * ck;
        rb.getString(&url);
	if (cb != NULL) {
            cb->findCookie(url,&ck);
fprintf(stderr,"CPluginVM  cookie=%s\n");
            cookie(instance,ck);
        }
        rb.free(url);
        break;
    case JAVA_PLUGIN_JAVASCRIPT_REQUEST:
        short type;
        rb.getShort(&type);
        rb.getString(&mess);
        if (cb == NULL || cb->javascriptRequest(mess)) {
            requestAbruptlyTerminated();
        }
        rb.free(mess);
        break;
    case JAVA_PLUGIN_SET_COOKIE:
        rb.getString(&url);
        rb.getString(&ck);
	if (cb != NULL)
            cb->setCookie(url,ck);
        rb.free(url);
        rb.free(ck);
        break;
    default:
        break;
    }

fprintf(stderr,"Leaving workpipehandler\n");
}

void CPluginVM_OP::requestAbruptlyTerminated() {
    CWriteBuffer wb;
    wb.putInt(JAVA_PLUGIN_REQUEST_ABRUPTLY_TERMINATED);
    wb.send(m_ltstate->command_pipe);
}
