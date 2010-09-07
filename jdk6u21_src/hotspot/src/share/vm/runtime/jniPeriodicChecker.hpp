/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
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

class JniPeriodicCheckerTask;

/*
 * This gets activated under Xcheck:jni (CheckJNICalls), and is typically
 * to detect any errors caused by JNI applications, such as signal handler,
 * hijacking, va 0x0 hijacking either by mmap or an OS error.
 */


class JniPeriodicChecker : AllStatic {

  friend class JniPeriodicCheckerTask;

  private:
    static JniPeriodicCheckerTask* _task;

  public:
    // Start/stop task
    static void engage();
    static void disengage();

    static bool is_active() { return _task != NULL; }

    static void initialize();
    static void destroy();
};

void jniPeriodicChecker_exit();
