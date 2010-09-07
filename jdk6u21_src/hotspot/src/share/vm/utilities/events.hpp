/*
 * Copyright (c) 1997, 2003, Oracle and/or its affiliates. All rights reserved.
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

// Events and EventMark provide interfaces to log events taking place in the vm.
// This facility is extremly useful for post-mortem debugging. The eventlog
// often provides crucial information about events leading up to the crash.
//
// All arguments past the format string must be passed as an intptr_t.
//
// To log a single event use:
//    Events::log("New nmethod has been created " INTPTR_FORMAT, nm);
//
// To log a block of events use:
//    EventMark m("GarbageCollecting %d", (intptr_t)gc_number);
//
// The constructor to eventlog indents the eventlog until the
// destructor has been executed.
//
// IMPLEMENTATION RESTRICTION:
//   Max 3 arguments are saved for each logged event.
//

class Events : AllStatic {
 public:
  // Logs an event, format as printf
  static void log(const char* format, ...) PRODUCT_RETURN;

  // Prints all events in the buffer
  static void print_all(outputStream* st) PRODUCT_RETURN;

  // Prints last number events from the event buffer
  static void print_last(outputStream *st, int number) PRODUCT_RETURN;
};

class EventMark : public StackObj {
 public:
  // log a begin event, format as printf
  EventMark(const char* format, ...) PRODUCT_RETURN;
  // log an end event
  ~EventMark() PRODUCT_RETURN;
};

int print_all_events(outputStream *st);
