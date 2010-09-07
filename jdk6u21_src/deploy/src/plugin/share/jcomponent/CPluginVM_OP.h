/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__CPluginVM_OP__)
#define __CPluginVM_OP__

#include <unistd.h>

#include "IPluginVM.h"
#include "IPluginVMCB.h"
#include "CJavaService.h"
#include "CWriteBuffer.h"
#include "IFDMonitor.h"

extern void workPipeEntry(void *); 

class CPluginVM_OP: public IPluginVM {
public:
    DECL_IEGO

    void newx(JavaType, int, const char**, const char**, IPluginVMCB *, JavaID *);
    void start(JavaID);
    void stop(JavaID);
    void destroy(JavaID);
    void window(JavaID, int, int, int, int, int); 
    void print(JavaID);
    void docbase(JavaID, const char *);
    void proxyMapping(char *, char *);
    void cookie(JavaID, char *);
    void javaScriptReply(JavaID, const char *);
    void javaScriptEnd(JavaID);
    void attachThread();
    void getInstanceJavaObject();
    void consoleShow();
    void consoleHide();

    void invalidateCB(JavaID);

    friend class CJavaService;

    ~CPluginVM_OP();

private:
    enum CommandPipeProtocol {
        JAVA_PLUGIN_NEW                         = 0xFA0001,
        JAVA_PLUGIN_DESTROY                     = 0xFA0002,
        JAVA_PLUGIN_WINDOW                      = 0xFA0003,
        JAVA_PLUGIN_SHUTDOWN                    = 0xFA0004,
        JAVA_PLUGIN_DOCBASE                     = 0xFA0005,
        JAVA_PLUGIN_PROXY_MAPPING               = 0xFA0007,
        JAVA_PLUGIN_COOKIE                      = 0xFA0008,
        JAVA_PLUGIN_JAVASCRIPT_REPLY            = 0xFA000A,
        JAVA_PLUGIN_JAVASCRIPT_END              = 0xFA000B,
        JAVA_PLUGIN_START                       = 0xFA0011,
        JAVA_PLUGIN_STOP                        = 0xFA0012,
        JAVA_PLUGIN_ATTACH_THREAD               = 0xFA0013,
        JAVA_PLUGIN_REQUEST_ABRUPTLY_TERMINATED = 0xFA0014,
        JAVA_PLUGIN_GET_INSTANCE_JAVA_OBJECT    = 0xFA0015,
        JAVA_PLUGIN_PRINT                       = 0xFA0016,
        JAVA_PLUGIN_CONSOLE_SHOW                = 0xFA0019,
        JAVA_PLUGIN_CONSOLE_HIDE                = 0xFA001A,
        JAVA_PLUGIN_OK                          = 0xFB0001
    };

    enum WorkerPipeProtocol {
        JAVA_PLUGIN_SHOW_STATUS                 = 0xF60001,
        JAVA_PLUGIN_SHOW_DOCUMENT               = 0xF60002,
        JAVA_PLUGIN_FIND_PROXY                  = 0xF60003,
        JAVA_PLUGIN_FIND_COOKIE                 = 0xF60004,
        JAVA_PLUGIN_JAVASCRIPT_REQUEST          = 0xF60006,
        JAVA_PLUGIN_SET_COOKIE                  = 0xF60009
    };

    enum PipeNumbers {
        STARTSAFEFD = 10,
        ENDSAFEFD   = 16,
        SPONT_FD    = 10,
        COMMAND_FD  = 11,
        WORK_FD     = 12,
        PRINT_FD    = 13
    };

    CPluginVM_OP(IFDMonitor *, const char *);
    IFDMonitor * m_FDMonitor;
    void requestAbruptlyTerminated();
    void connectPipeMonitor();
    void workPipeHandler();
    friend void workPipeEntry(void * data);
    class PairsTable {
            const static int table_size = 0x0010;
        class InstanceCBPair {
            JavaID instance;
            IPluginVMCB * cb;
            InstanceCBPair * next;
        public:
            InstanceCBPair(JavaID, IPluginVMCB *);
            ~InstanceCBPair();
            friend class PairsTable;
        };
        InstanceCBPair * m_table[table_size];
    public:
       PairsTable();
       void add(JavaID, IPluginVMCB *);
       void remove(JavaID); 
       IPluginVMCB * find(JavaID);
    };

    PairsTable m_pairsTable;

    struct LongTermState {
       enum VMstatus { nONE,rUNNING,fAILED };
       int command_pipe;
       int work_pipe;
       int print_pipe;
       char *java_dir;
       pid_t child_pid;
       JavaID instance_count;
       VMstatus vmstatus; 
    } * m_ltstate;
    bool startVM();
    bool forkAndExecVM();
    void findContainingJRE();
    void setupChildENV();
    int createPipe(int fd[2]);
    void dup2ToSafety(int nfds, int fdarr[],
                      int safe_range_start, int safe_range_end);
    void protectedWrite(CWriteBuffer);
    const char * m_userAgent;
};

#endif
