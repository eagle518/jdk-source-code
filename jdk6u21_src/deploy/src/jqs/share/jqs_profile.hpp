/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JQS_PROFILE_HPP
#define JQS_PROFILE_HPP

#include <string>
#include <stdio.h>

#include "os_defs.hpp"


#define CONFIG_ENTRY    "config:"

/*
 * The JQS profile writer with the ability to write template of the JQS 
 * configuration file.
 */
class JQSProfileWriter {
    FILE* outProfile;
    FILE* outConfig;
    uint64_t lastRegionEnd;
    bool fileSectionOpened;

public:
    JQSProfileWriter(FILE* outProfile, FILE* outConfig);

    /*
     * Opens new profile block for the file. Writes the config string to the 
     * configuration file template if the latter is specified.
     */
    void openFileSection(const char* file, const char* config);
    /*
     * Writes region to opened file section.
     */
    void writeRegion (uint64_t start, uint64_t end);
    /*
     * Closes file section in the profile.
     */
    void closeFileSection();
};

/*
 * The content handler interface. The successors are intended to be passed to
 * ReadJQSProfile function.
 */
class JQSProfileContentHandler {
public:
    /*
     * Called for each profile information block associated with a file.
     */
    virtual void openFileSection(const std::string& file) = 0;
    /*
     * Sets configuration information for current file.
     */
    virtual void setConfig(const std::string& config) = 0;
    /*
     * Called for each region in the profile information block.
     */
    virtual void addRegion (uint64_t start, uint64_t end) = 0;
    /*
     * Called when the profile block is finished.
     */
    virtual void closeFileSection() = 0;
};

/*
 * Parses profile using given content handler object.
 * If the file is not specified, then the file with given file name is opened
 * for parsing. Otherwise, it parses given file and use fileName and lineno
 * for error reporting. The former mode is used to parse a separate profile file 
 * and the latter one is used to parse the profile section of the configuration 
 * file.
 */
bool ReadJQSProfile (const char* fileName, 
                     JQSProfileContentHandler* contentHandler,
                     FILE* file = NULL, 
                     size_t lineno = 0);

#endif
