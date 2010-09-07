/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <stdio.h>

#include "jqs.hpp"

// provided in main.cpp
extern void unload_all();
extern void do_jqs(unsigned int interval);

/*
 * Round given value up or down to conform to given alignment.
 */
extern size_t roundUp(size_t val, size_t align);
extern uint64_t roundUp64(uint64_t val, size_t align);
extern uint64_t roundDown64(uint64_t val, size_t align);

/*
 * Removes leading and trailing spaces from the string.
 */
extern std::string trim(const std::string& str);

/*
 * Converts string to a lower case.
 */
extern std::string tolowercase(const std::string& str);

/*
 * Reads one line from the file, returns true on success.
 * Similar to std::getline (std::istream&, std::string).
 */
extern bool getline (FILE* file, std::string& line);



/*
 * JQS options read from command line and configuration file.
 */

/*
 * Configuration file name.
 */
extern std::string configFileName;
/*
 * Profile file name, if the name is not specified the string is empty.
 */
extern std::string profileFileName;
/*
 * Log file name, if the name is not specified the string is empty.
 */
extern std::string logFileName;
/*
 * JQS performs periodic prefetch cycles in the specified number of seconds.
 */
extern unsigned int interval;
/*
 * Low memory threshold in bytes. If the OS does not support low memory 
 * notifications, JQS does not perform a prefetch cycles if the amount of 
 * available physical memory is less than the value.
 */
extern size_t memoryLimit;

/*
 * Threshold percentage values for the "boot completed" check. The first 
 * prefetch cycle is not started until the CPU and disk I/O usage in the 
 * system goes below these values.
 */
extern PerfCountersThreshold bootThresholds;
/*
 * Threshold percentage value for the "system idle time" check. The first 
 * prefetch cycle is not started until the CPU and disk I/O usage in the 
 * system goes below these values.
 */
extern PerfCountersThreshold refreshThresholds;


#endif
