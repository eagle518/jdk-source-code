/*
 * Copyright (c) 2000, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

#ifndef _REAPER_
#define _REAPER_

#include <vector>
#include <windows.h>

typedef void ReaperCB(void* userData);

/** A Reaper maintains a thread which waits for child processes to
    terminate; upon termination it calls a user-specified ReaperCB to
    clean up resources associated with those child processes. */

class Reaper {
private:
  Reaper& operator=(const Reaper&);
  Reaper(const Reaper&);

public:
  Reaper(ReaperCB*);
  ~Reaper();

  // Start the reaper thread.
  bool start();

  // Stop the reaper thread. This is called automatically in the
  // reaper's destructor. It is not thread safe and should be called
  // by at most one thread at a time.
  bool stop();

  // Register a given child process with the reaper. This should be
  // called by the application's main thread. When that process
  // terminates, the cleanup callback will be called with the
  // specified userData in the context of the reaper thread. Callbacks
  // are guaranteed to be called serially, so they can safely refer to
  // static data as well as the given user data.
  void registerProcess(HANDLE processHandle, void* userData);

private:
  // For thread safety of register()
  CRITICAL_SECTION crit;

  ReaperCB* cb;

  // State variables
  volatile bool active;
  volatile bool shouldShutDown;

  struct ProcessInfo {
    HANDLE handle;
    void* userData;
  };

  // Bookkeeping
  std::vector<ProcessInfo> procInfo;

  // Synchronization between application thread and reaper thread
  HANDLE event;

  // Entry point for reaper thread
  void reaperThread();

  // Static function which is actual thread entry point
  static DWORD WINAPI reaperThreadEntry(LPVOID data);
};

#endif  // #defined _REAPER_
