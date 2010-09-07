/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JQS_TIMER_HPP
#define JQS_TIMER_HPP

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <string>
#include <sstream>
#include <iomanip>

#include "os_defs.hpp"
#include "print.hpp"

/*
 * Defines verbosity level of the performance measurement logic.
 */
extern unsigned int print_times;

/*
 * Initializes timers. One must call this function before using the timers
 * for the first time.
 */
extern void initTimer ();

/*
 * Returns current value of a system performance counter.
 */
extern uint64_t os_elapsed_counter();
/*
 * Returns the frequency of the performance counter.
 */
extern uint64_t os_elapsed_frequency();


/*
 * Simple timer implementation, using system performance counters.
 * Before using the class for the first time the initTimer() function 
 * should be called.
 */
class ElapsedTimer {
    uint64_t counter;
    uint64_t start_counter;
    bool active;

public:
    ElapsedTimer()
        : active(false)
        , start_counter(0)
    {
        reset();
    }

    void reset() { 
        counter = 0; 
    }
    
    void start() {
        if (!active) {
            active = true;
            start_counter = os_elapsed_counter();
        }
    }

    void stop() {
        if (active) {
            counter += os_elapsed_counter() - start_counter;
            active = false;
        }
    }

    double seconds() const {
        double count = (double)counter;
        double freq = (double)os_elapsed_frequency();
        return count/freq;
    }
};

/*
 * Measures time elapsed since object construction till destruction, 
 * and prints an information message with the results.
 */
class TraceTime {
    bool active;
    ElapsedTimer timer;
    std::string label;
    int level;

public:
    TraceTime(const std::string& label_, int level_ = 0, bool active_ = false)
        : level(level_)
        , active(active_)
        , label(label_)
    {
        if (active) {
            timer.start();
        }
    }
    ~TraceTime() {
        if (active) {
            timer.stop();

            std::ostringstream strm;
            if (level > 0) {
                for (int i = 0 ; i < level ; i++) {
                    strm << "  ";
                }
            }

            strm << "[" << label << ", "
                 << std::setiosflags(std::ios::fixed)
                 << std::setw(10)
                 << std::setprecision(7)
                 << timer.seconds()
                 << " secs]\n";

            jqs_info (0, strm.str().c_str());
        }
    }

    void suspend() { 
        if (active) { 
            timer.stop();  
        }
    }

    void resume() { 
        if (active) {
            timer.start(); 
        }
    }
};

#endif
