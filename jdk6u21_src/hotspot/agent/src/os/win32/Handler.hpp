/*
 * Copyright (c) 2000, 2001, Oracle and/or its affiliates. All rights reserved.
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

#ifndef _HANDLER_
#define _HANDLER_

/** An abstract base class encapsulating the handlers for all commands
    understood by the system. */
class Handler {
public:
  virtual void ascii(char* arg)         = 0;
  virtual void unicode(char* arg)       = 0;
  virtual void procList(char* arg)      = 0;
  virtual void attach(char* arg)        = 0;
  virtual void detach(char* arg)        = 0;
  virtual void libInfo(char* arg)       = 0;
  virtual void peek(char* arg)          = 0;
  virtual void poke(char* arg)          = 0;
  virtual void threadList(char* arg)    = 0;
  virtual void dupHandle(char* arg)     = 0;
  virtual void closeHandle(char* arg)   = 0;
  virtual void getContext(char* arg)    = 0;
  virtual void setContext(char* arg)    = 0;
  virtual void selectorEntry(char* arg) = 0;
  virtual void suspend(char* arg)       = 0;
  virtual void resume(char* arg)        = 0;
  virtual void pollEvent(char* arg)     = 0;
  virtual void continueEvent(char* arg) = 0;
  virtual void exit(char* arg)          = 0;
};

#endif  // #defined _HANDLER_
