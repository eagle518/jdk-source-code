/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include "jqs.hpp"
#include "timer.hpp"
#include "utils.hpp"
#include "os_defs.hpp"
#include "os_utils.hpp"
#include "prefetch.hpp"

using namespace std;

/*
 * JQS options read from command line and configuration file.
 */

/*
 * Configuration file name.
 */
string configFileName;
/*
 * Profile file name, if the name is not specified the string is empty.
 */
string profileFileName;
/*
 * Log file name, if the name is not specified the string is empty.
 */
string logFileName;
/*
 * JQS performs periodic prefetch cycles in the specified number of seconds.
 */
unsigned int interval = DEFAULT_INTERVAL;
/*
 * Low memory threshold in bytes. If the OS does not support low memory 
 * notifications, JQS does not perform a prefetch cycles if the amount of 
 * available physical memory is less than the value.
 */
size_t memoryLimit = DEFAULT_MEMORY_LIMIT;

/*
 * Threshold percentage values for the "boot completed" check. The first 
 * prefetch cycle is not started until the CPU and disk I/O usage in the 
 * system goes below these values.
 */
PerfCountersThreshold bootThresholds = { DEFAULT_DISK_TIME_THRESHOLD, DEFAULT_PROCESSOR_TIME_THRESHOLD};
/*
 * Threshold percentage value for the "system idle time" check. The first 
 * prefetch cycle is not started until the CPU and disk I/O usage in the 
 * system goes below these values.
 */
PerfCountersThreshold refreshThresholds = { DEFAULT_DISK_TIME_THRESHOLD, DEFAULT_PROCESSOR_TIME_THRESHOLD};


#define WHITE_SPACE " \t"


/*
 * Round given value up or down to conform to given alignment.
 */
size_t roundUp(size_t val, size_t align) {
    return ((val + align - 1) / align) * align; 
}

uint64_t roundUp64(uint64_t val, size_t align) {
    return ((val + align - 1) / align) * align; 
}

uint64_t roundDown64(uint64_t val, size_t align) {
    return (val / align) * align; 
}

/*
 * Removes leading and trailing spaces from the string.
 */
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(WHITE_SPACE);
    if (start == string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(WHITE_SPACE);
    assert (end != string::npos);
    return str.substr(start, end + 1);
}

/*
 * Converts string to a lower case.
 */
std::string tolowercase(const std::string& str) {
    string s(str);
    for (size_t i = 0; i < s.length (); i++) {
        s[i] = tolower(s[i]);
    }
    return s;
}

/*
 * Reads one line from the file, returns true on success.
 * Similar to std::getline (std::istream&, std::string).
 */
bool getline (FILE* file, std::string& line) {
    const size_t BUF_SIZE = 1024;
    char buf [BUF_SIZE];

    line.clear ();

    for (;;) {
        buf [0] = 0;
        fgets (&buf[0], BUF_SIZE, file);
        if (buf[0]) { // something was read to buffer
            line += &buf[0];
            size_t len = line.length ();
            if (line[len - 1] == '\n') {
                // end of line found
                line.erase (len - 1);
                return true;
            }
            if (feof (file)) {
                // eof found - return the rest of the string
                return true;
            }
        } else {
            // eof or error
            return false;
        }
    }
}
