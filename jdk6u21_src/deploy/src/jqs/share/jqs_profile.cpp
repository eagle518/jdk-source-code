/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <sstream>
#include <iomanip>

#include "jqs_profile.hpp"
#include "print.hpp"
#include "utils.hpp"
#include "parse.hpp"
#include "os_utils.hpp"

using namespace std;

/*
 * Header for the configuration file template generated.
 */
static const char* CONFIG_FILE_HEADER = COMMENT_S " Config file template, created by JQS profiler";


/*
 * Creates the JQS profile writer object. The outConfig parameter can be NULL.
 */
JQSProfileWriter::JQSProfileWriter (FILE* outProfile_, FILE* outConfig_)
    : outProfile(outProfile_)
    , outConfig(outConfig_)
    , lastRegionEnd(0)
    , fileSectionOpened(false)
{
    assert(outProfile);
    if (outConfig) {
        fprintf (outConfig, "%s\n\n", CONFIG_FILE_HEADER);
        fprintf (outConfig, COMMENT_S " " CMD_SET " " JAVA_HOME "=\n\n");
    }
}

/*
 * Opens new profile block for the file. Writes the config string to the 
 * configuration file template if the latter is specified.
 */
void JQSProfileWriter::openFileSection(const char* file, const char* config) {
    fprintf (outProfile,  "[%s]\n", file);
    fprintf (outProfile,  CONFIG_ENTRY " %s\n", config);
    if (outConfig) {
        fprintf (outConfig, "%s\n", config);
    }
    lastRegionEnd = 0;
    fileSectionOpened = true;
}

/*
 * Writes region to opened file section.
 */
void JQSProfileWriter::writeRegion (uint64_t start, uint64_t end) {
    assert (fileSectionOpened);
    assert (start <= end);
    assert (lastRegionEnd <= start);

    fprintf (outProfile, "%08" FORMAT64_MODIFIER "x" 
                         "-"
                         "%08" FORMAT64_MODIFIER "x"
                         "\n", start, end);
    lastRegionEnd = end;
}

/*
 * Closes file section in the profile.
 */
void JQSProfileWriter::closeFileSection() {
    fprintf (outProfile,  "\n");
    fileSectionOpened = false;
}

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
                     FILE* file, 
                     size_t lineno)
{
    assert(contentHandler);

    if (!file) {
        file = fopen (fileName, "rt");
        if (!file) {
            jqs_info (3, "Unable to open profile %s: %s\n", fileName, strerror(errno));
            return false;
        }
    }

    string line;
    bool sectionOpened = false;
    uint64_t lastRegionEnd = 0;

    static const string ConfigEntry(CONFIG_ENTRY);

    while (getline (file, line)) {
        lineno++;
        line = trim(line);
        if (line.empty () || (line[0] == '#')) {
            continue;
        }

        if ((line [0] == '[') && (line[line.length () - 1] == ']')) {
            // observing new file entry
            string filename = line.substr (1, line.length() - 2);
            if (sectionOpened) {
                contentHandler->closeFileSection();
            }
            contentHandler->openFileSection(filename);
            sectionOpened = true;
            lastRegionEnd = 0;

        } else if (line.find(ConfigEntry) == 0) {
            string config = trim(line.substr(ConfigEntry.length()));
            contentHandler->setConfig(config);
        } else {
            uint64_t begin = 0, end = 0;
            char ch = 0;

            istringstream istr (line);
            istr >> hex >> begin >> ch >> end;
            if ((ch != '-') || !istr.eof () || (begin >= end)) {
                jqs_error ("JQS profile file %s: error at line %d: incorrect region definition \"%s\"\n", fileName, lineno, line.c_str());
                return false;
            }
            if (!sectionOpened) {
                jqs_error ("JQS profile file %s: error at line %d: file section is not opened\n", fileName, lineno);
                return false;
            }
             
            // require ordered regions
            if (lastRegionEnd > begin) {
                jqs_error ("JQS profile file %s contains unordered region at line %d\n", fileName, lineno);
                return false;
            }

            lastRegionEnd = end;
            contentHandler->addRegion (begin, end);
        }
    }

    if (sectionOpened) {
        contentHandler->closeFileSection();
    }

    return true;
}
