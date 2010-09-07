/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "os_layer.hpp"

#include "timer.hpp"

unsigned int print_times = 0;

static bool has_performance_counter = false;
static uint64_t counter_frequency = 0;

/*
 * function to convert a LARGE_INTERGER type into a 64-bit
 * long type.
 */
uint64_t as_long2(int high, int low) {
    return ((uint64_t)high << 32) | ((uint64_t)low & 0xffffffff);
}

uint64_t as_long(LARGE_INTEGER x) {
    return as_long2(x.HighPart, x.LowPart);
}

/*
 * function to get the high resolution time value from the
 * PerformanceCounter system.
 */
uint64_t os_elapsed_counter() {
    LARGE_INTEGER value;
    FILETIME wt;

    if (has_performance_counter) {
        QueryPerformanceCounter(&value);
        return as_long(value);

    } else {
        GetSystemTimeAsFileTime(&wt);
        return as_long2(wt.dwHighDateTime, wt.dwLowDateTime);
    }
}

/*
 * function to provide the frequency of the high resolution timer
 */
uint64_t os_elapsed_frequency() {
    if (has_performance_counter) {
        return counter_frequency;
    } else {
        return 10000000;
    }
}

/*
 * Initializes timers. One must call this function before using the timers
 * for the first time.
 */
void initTimer () {
    LARGE_INTEGER count;

    if (QueryPerformanceFrequency(&count)) {
        has_performance_counter = true;
        counter_frequency = as_long(count);
    }
}
