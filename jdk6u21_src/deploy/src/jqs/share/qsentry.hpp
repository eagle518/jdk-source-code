/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef QSENTRY_HPP
#define QSENTRY_HPP

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string>
#include <vector>

#include "os_defs.hpp"
#include "os_utils.hpp"
#include "os_qsentry.hpp"


/*
 * The quick starter entry kind.
 */
enum QSCmd {
    QS_NONE,
    QS_LOAD,
    QS_REFRESH,
    QS_LOADLIB,
    QS_REFRESHLIB,
    QS_REFRESHDIR,
};

/*
 * Profile information item. 
 */
struct Region {
    uint64_t start;
    uint64_t end;
};

/*
 * Profile information for the quick starter entry.
 */
typedef std::vector<Region> Regions;


class QSEntry;
typedef std::vector<QSEntry*> QSEntries;


/*
 * The quick starter entry with all its properties, including kind, file name, 
 * profile and options.
 */
class QSEntry {
    /*
     * Quick starter entry kind.
     */
    QSCmd cmd;
    std::string fileName;

    /*
     * Platform specific extension, contains OS handle to the file.
     */
    QSEntryOS os;

    /*
     * "pagesize" and "bufsize" options' values respectively.
     */
    size_t pageSize;
    size_t bufferSize;

    /*
     * "locked" flag, makes sense only for QS_LOAD, QS_LOADLIB.
     */
    bool locked;

    /*
     * "mapped" flag, makes sense only for QS_REFRESH.
     */
    bool mapped;

    /*
     * "libpath" value and a list of dependent quick starter entries created
     * according to "libdepend" value. Make sense only for QS_LOADLIB, QS_REFRESHLIB.
     */
    std::string libPath;
    QSEntries dependentLibs;

    /*
     * Profile information associated with the entry.
     */
    Regions regions;

    /*
     * "recursive" flag, makes sense only for QS_REFRESHDIR.
     */
    bool recursive;

    /*
     * Time of last modification of the file.
     */
    time_t lastModified;

    /*
     * Size of the file.
     */
    uint64_t fileSize;

    /*
     * Address and size of file mapping. 
     */
    void* mapaddr;
    size_t mapsize;

public:
    QSEntry (QSCmd cmd_);
    ~QSEntry();

    /*
     * Returns quick starter entry kind.
     */
    QSCmd getCmd() const {
        return cmd;
    }

    /*
     * Returns entry file name.
     */
    const char* getFileName () const {
        return fileName.c_str();
    }

    /*
     * Sets entry file name.
     */
    void setFileName (const std::string& fileName_) {
        fileName = fileName_;
    }

    /*
     * Returns platform specific extension.
     */
    QSEntryOS& getOS() {
        return os;
    }

    /*
     * Get/set "pagesize" option value.
     */
    size_t getPageSize() const {
        return pageSize;
    }
    void setPageSize(size_t pageSize_) {
        pageSize = pageSize_;
    }

    /*
     * Get/set "bufsize" option value.
     */
    size_t getBufferSize() const {
        return bufferSize;
    }
    void setBufferSize(size_t bufferSize_) {
        bufferSize = bufferSize_;
    }

    /*
     * Get/set "locked" flag.
     */
    bool isLocked() const {
        return locked;
    }
    void setLocked(bool locked_) {
        assert ((cmd == QS_LOAD) || (cmd == QS_LOADLIB));
        locked = locked_;
    }

    /*
     * Get/set "mapped" flag.
     */
    bool isMapped() const {
        assert (cmd == QS_REFRESH);
        return mapped;
    }
    void setMapped(bool mapped_) {
        assert (cmd == QS_REFRESH);
        mapped = mapped_;
    }

    /*
     * Get/set "recursive" flag.
     */
    bool isRecursive() const {
        assert (cmd == QS_REFRESHDIR);
        return recursive;
    }
    void setRecursive(bool recursive_) {
        assert (cmd == QS_REFRESHDIR);
        recursive = recursive_;
    }

    /*
     * Get/set "libpath" option value.
     */
    const std::string& getLibPath() const {
        assert ((cmd == QS_LOADLIB) || (cmd == QS_REFRESHLIB));
        return libPath;
    }
    void setLibPath(const std::string& path) {
        assert ((cmd == QS_LOADLIB) || (cmd == QS_REFRESHLIB));
        libPath = path;
    }

    /*
     * Returns the list of dependent quick starter entries.
     */
    const QSEntries& getDependentLibs() const {
        return dependentLibs;
    }
    /*
     * Adds entry to the list of dependent entries.
     */
    void addDependentLib(QSEntry* libEntry) {
        assert (cmd == QS_REFRESHLIB);
        dependentLibs.push_back(libEntry);
    }

    /*
     * Returns the profile information associated with the entry.
     */
    const Regions& getRegions () const {
        return regions;
    }
    /*
     * Adds new item to the profile information.
     */
    void addRegion (uint64_t start, uint64_t end);

    /*
     * Get/set time of a last modification of the file.
     */
    time_t getLastModified() const {
        return lastModified;
    }
    void setLastModified(time_t lastModified_) {
        lastModified = lastModified_;
    }

    /*
     * Get/set file size.
     */
    uint64_t getFileSize() const {
        return fileSize;
    }
    void setFileSize(uint64_t fileSize_) {
        fileSize = fileSize_;
    }

    /*
     * Returns true if the entry is already loaded.
     */
    bool isLoaded() const {
        return (mapaddr != NULL);
    }

    /*
     * Get/set file mapping information.
     */
    void* getMapAddr() const {
        return mapaddr;
    }
    size_t getMapSize() const {
        return mapsize;
    }
    void setMapping(void* mapaddr_, size_t mapsize_) {
        mapaddr = mapaddr_;
        mapsize = mapsize_;
    }

    /*
     * Dumps quick starter entry configuration to the file.
     */
    void dumpConfig(FILE* out) const;

};

/*
 * A list of quick starter entries created according to configuration file.
 */
extern QSEntries g_QSEntries;

#endif
