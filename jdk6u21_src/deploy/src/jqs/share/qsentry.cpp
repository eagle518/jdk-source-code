/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "os_defs.hpp"
#include "qsentry.hpp"
#include "prefetch.hpp"
#include "utils.hpp"
#include "parse.hpp"


/*
 * If the distance between adjacent file regions is less than this 
 * constant the regions are merged together.
 */
#define REGION_MERGE_DISTANCE           (4*4096)

/*
 * Defines default buffer size for refresh directive in pages. 
 * Experiments showed better efficiency of 16-pages (64K) buffers.
 */
#define DEFAULT_BUFFER_SIZE_IN_PAGES    16


/*
 * A list of quick starter entries created according to configuration file.
 */
QSEntries g_QSEntries;


QSEntry::QSEntry (QSCmd cmd_)
    : cmd (cmd_)
    , pageSize(getDefaultPageSize())
    , bufferSize(getDefaultPageSize() * DEFAULT_BUFFER_SIZE_IN_PAGES)
    , locked(false)
    , mapped(false)
    , recursive(false)
    , mapaddr(NULL)
    , mapsize(0)
    , lastModified(0)
    , fileSize(0)
{
}

QSEntry::~QSEntry() {
    for (size_t i = 0; i < dependentLibs.size (); i++) {
        delete dependentLibs[i];
    }
}

/*
 * Adds new item to the profile information.
 */
void QSEntry::addRegion (uint64_t start, uint64_t end) {
    assert (start < end);

    start = roundDown64(start, pageSize);
    end = roundUp64(end, pageSize);

    if (!regions.empty()) {
        // merge regions if necessary
        Region& lastRegion = regions.back();
        assert (lastRegion.end <= start + pageSize); // (end <= start) assertion not always hold due to rounding
        assert (lastRegion.end <= end);

        if ((start <= lastRegion.end) || ((start - lastRegion.end) <= REGION_MERGE_DISTANCE)) {
            lastRegion.end = end;
            return;
        }
    }
    Region region;
    region.start = start;
    region.end   = end;
    regions.push_back(region);
} 

/*
 * Dumps quick starter entry configuration to the file.
 */
void QSEntry::dumpConfig(FILE* out) const {
    switch (cmd) {
        case QS_LOAD:
            fprintf (out, CMD_LOAD);
            break;

        case QS_REFRESH:
            fprintf (out, CMD_REFRESH);
            break;

        case QS_LOADLIB:
            fprintf (out, CMD_LOADLIB);
            break;

        case QS_REFRESHLIB:
            fprintf (out, CMD_REFRESHLIB);
            break;

        case QS_REFRESHDIR:
            fprintf (out, CMD_REFRESHDIR);
            break;

        default:
            assert(false);
    }

    fprintf (out, " \"%s\"", getFileName());

    if (isLocked()) {
        fprintf (out, " " OPTION_LOCKED);
    }
    if ((cmd == QS_REFRESH) && isMapped()) {
        fprintf (out, " " OPTION_MAPPED);
    }
    if ((cmd == QS_REFRESHDIR) && isRecursive()) {
        fprintf (out, " " OPTION_RECURSIVE);
    }
    if ((cmd == QS_REFRESH) && !isMapped()) {
        fprintf (out, " " OPTION_BUFSIZE "=" SIZET_FORMAT, getBufferSize());
    } else {
        fprintf (out, " " OPTION_PAGESIZE "=" SIZET_FORMAT, getPageSize());
    }
    if ((cmd == QS_LOADLIB) || (cmd == QS_REFRESHLIB)) {
        if (!libPath.empty()) {
            fprintf (out, " " OPTION_LIBPATH "=\"%s\"", libPath.c_str());
        }
        if (!dependentLibs.empty()) {
            fprintf (out, " " OPTION_LIBDEPEND "=\"");
            for (size_t i = 0; i < dependentLibs.size(); i++) {
                if (i != 0) {
                    fprintf (out, ";");
                }
                fprintf (out, "%s", dependentLibs[i]->getFileName());
            }
            fprintf (out, "\"");
        }
    }
    fprintf (out, "\n");
}
