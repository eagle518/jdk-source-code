/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "os_layer.hpp"
#include <jvmti.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <set>

#include "jqs.hpp"
#include "print.hpp"
#include "os_utils.hpp"
#include "thread.hpp"
#include "timer.hpp"
#include "jqs_profile.hpp"
#include "parse.hpp"
#include "utils.hpp"

using namespace std;

/*
 * Defines JQS profiler verbosity levels.
 */
const unsigned int JQS_PROFILER_VERBOSE = 0;
const unsigned int JQS_PROFILER_TIMINGS = 0;

/*
 * The time interval in ms between two subsequent memory scans.
 */
const int PROFILER_INTERVAL = 10;

/*
 * The initial size for the buffer used for storing working set.
 */
const int INITIAL_WORKING_SET_BUFFER_SIZE = 1024;

/*
 * Page size constant.
 */
const int PAGE_SIZE = 4096;

/*
 * The profiler uses this ratio to calculate the working set size that it
 * should set in the following way: TotalPhysicalMemory * PHYS_MEMORY_OCCUPATION_RATIO.
 */
const double PHYS_MEMORY_OCCUPATION_RATIO = 0.70f;

/*
 * Handle to the JQS agent library instance.
 */
static HINSTANCE hAgentInstance = NULL;

/*
 * Handle to the kernel32.dll.
 */
static HMODULE Kernel32Handle = NULL;

//////////////////////////////////////////////////////////////////////////

static const char* SYSTEMROOT = "SystemRoot";
static const char* JVM_DLL = "jvm.dll";
static const char* JAVA_HOME_RELATIVE_TO_JVM_DLL = "..\\..";

/*
 * This class holds (var, path) pair and has a method which 
 * tries to find the path substring in given file name and to
 * replace it with $(var).
 */ 
class Substitution {
    string var;
    string path;

public:
    Substitution(const string& var_, const string& path_);
    /*
     * Searches for the path substring in the file name. On success replaces
     * it with the $(var) substring and returns true. Otherwise, returns false.
     */
    bool substitute (string& fileName) const;
};

Substitution::Substitution(const string& var_, const string& path_) {
    var = string (DOLLAR_SIGN_S OPEN_BRACE_S) + var_ + string(CLOSE_BRACE_S) + FILE_SEPARATOR_CHAR;
    path = tolowercase(path_) + FILE_SEPARATOR_CHAR;
}

/*
 * Searches for the path substring in the file name. On success replaces
 * it with the $(var) substring and returns true. Otherwise, returns false.
 */
bool Substitution::substitute (string& fileName) const {
    if ((fileName.length() > path.length()) &&
        !strncmp (fileName.c_str(), path.c_str(), path.length()))
    {
        fileName.replace(0, path.length(), var);
        return true;
    }
    return false;
}

/*
 * A set of Substitution objects.
 */
class Substitutions {
    vector<Substitution> subs;

    /*
     * Adds the following substitution for given environment variable
     * (envVar, getenv(envVar)).
     */
    void addEnv (const char* envVar);

    /*
     * Detects path to JRE and registers substitution for JAVA_HOME
     * variable.
     * To get the JRE path, this function locates jvm.dll loaded by JVM,
     * obtains the full path to jvm.dll and appends JAVA_HOME_RELATIVE_TO_JVM_DLL.
     */
    void addJavaHome ();

public:
    Substitutions();

    /*
     * Searches and applies the appropriate substitution for given file name.
     */
    string makeSubstitutions(const string& fileName) const;
};

Substitutions::Substitutions() {
    addEnv(SYSTEMROOT);
    addJavaHome();
}

/*
 * Adds the following substitution for given environment variable
 * (envVar, getenv(envVar)).
 */
void Substitutions::addEnv (const char* envVar) {
    const char* val = getenv(envVar);
    if (!val || !*val) {
        jqs_warn("Unable to get %s environment variable\n", envVar);
        return;
    }

    subs.push_back(Substitution(envVar, val));
}

/*
 * Detects path to JRE and registers substitution for JAVA_HOME
 * variable.
 * To get the JRE path, this function locates jvm.dll loaded by JVM,
 * obtains the full path to jvm.dll and appends JAVA_HOME_RELATIVE_TO_JVM_DLL.
 */
void Substitutions::addJavaHome () {
    // add JAVA_HOME variable, basing on the jvm.dll location

    HMODULE hJVM = GetModuleHandle(JVM_DLL);
    if (!hJVM) {
        jqs_warn("Unable to get %s handle: GetModuleHandle failed (error %d)\n", JVM_DLL, GetLastError());
        return;
    }

    string path = getSatelliteFileName(JAVA_HOME_RELATIVE_TO_JVM_DLL, hJVM);

    subs.push_back(Substitution(JAVA_HOME, getFullPath(path)));
}

/*
 * Searches and applies the appropriate substitution for given file name.
 */
string Substitutions::makeSubstitutions(const string& fileName) const {
    string fname = tolowercase(fileName);
    for (size_t i = 0; i < subs.size(); i++) {
        if (subs[i].substitute(fname)) {
            break;
        }
    }
    return fname;
}


//////////////////////////////////////////////////////////////////////////

/*
 * A representation of a file region, that was detected to be accessed.
 */
struct FileRegion {
    uint64_t start;
    uint64_t end;

    FileRegion (uint64_t start_, uint64_t end_)
        : start(start_)
        , end(end_)
    {}

    /*
     * Some order on file regions.
     */
    bool operator< (const FileRegion& reg) const {
        if (start != reg.start) {
            return start < reg.start;
        }
        return end < reg.end;
    }

    /*
     * Joins given region r2 to this one, returns true if 
     * regions are joined.
     */
    bool joinIfIntersects(const FileRegion& r2) {
        if (start < r2.start) {
            if (r2.start <= end) {
                end = max (end, r2.end);
                return true;
            }
        } else {
            if (start <= r2.end) {
                start = r2.start;
                end = max (end, r2.end);
                return true;
            }
        }
        return false;
    }
};

/*
 * The kind of the detected access to a file.
 */
enum FileAccessKind {
    AK_Unknown,
    AK_Read,
    AK_Mapped,
    AK_Library
};

//////////////////////////////////////////////////////////////////////////

/*
 * The object holds all necessary information about a file monitored by 
 * JQS agent. The information includes file name, kind of the access detected
 * and a set of file regions accessed.
 */
class MonitoredFile {
    string shortName;
    string fileName;
    string config;
    FileAccessKind accessKind;
    bool discarded;
    bool relocated;

    typedef set<FileRegion> FileRegions;
    FileRegions regions;

    /*
     * Prints regions, used for debugging.
     */
    void printRegions (const char* s);

public:
    MonitoredFile (const string& shortName_)
        : shortName(shortName_)
        , accessKind(AK_Unknown)
        , discarded(false)
        , relocated(false)
    {
    }

    /*
     * Return name of the file not including the path.
     */
    const string& getShortFileName() const {
        return shortName;
    }

    /*
     * Get/set methods for a full name property.
     */
    const string& getFileName() const {
        return fileName;
    }
    void setFileName(const string& fileName_) {
        fileName = fileName_;
    }

    /*
     * Marks the monitored file as discarded. JQS agent discards files when it
     * detects write access to a file. Discarded files are excluded from 
     * resulting profile.
     */
    void discard () {
        discarded = true;
    }
    /*
     * Returns true if the monitored file was discarded.
     */
    bool isDiscarded () const {
        return discarded;
    }

    /*
    * Marks the monitored file as relocated (placed at base address, different from default).
    */
    void setRelocated () {
        assert (accessKind == AK_Library);
        if (!relocated) {
            jqs_warn ("Library \"%s\" was relocated: its profile is inaccurate\n", getFileName().c_str());
            relocated = true;
        }
    }

    /*
     * Returns kind of access to the file, detected by JQS profiler.
     */
    FileAccessKind getAccessKind() const {
        return accessKind;
    }
    /*
     * Sets access kind attribute.
     */
    void setAccessKind(FileAccessKind ak) {
        accessKind = ak;
    }

    /*
     * Applies matching substitutions to the full file name and returns 
     * the JQS configuration file entry according to kind of access
     * to the file detected by the profiler.
     */
    const string& getConfig(const Substitutions* subs);
    /*
     * Sets JQS configuration file entry obtained from the profile file.
     */
    void setConfig(const string& conf) {
        config = conf;
    }

    /*
     * Adds new file region to the set and merges intersecting ones.
     */
    void touchRegion(uint64_t start, uint64_t end);

    /*
     * Writes all file regions detected by the profiler.
     */
    void save(JQSProfileWriter& writer);
};

/*
 * Prints regions, used for debugging.
 */
void MonitoredFile::printRegions (const char* s) {
    if (verbose < 7) {
        return;
    }

    jqs_info (7, "    printing regions: %s\n", s);
    for (FileRegions::const_iterator it = regions.begin (); it != regions.end (); ++it) {
        jqs_info (7, "    " UINT64_FORMAT "-" UINT64_FORMAT "\n", it->start, it->end);
    }
}

/*
 * Adds new file region to the set and merges intersecting ones.
 */
void MonitoredFile::touchRegion(uint64_t start, uint64_t end) {
    if (discarded) {
        return;
    }
    FileRegion region(start, end);
    jqs_info (5, "    touching file %s region " UINT64_FORMAT "-" UINT64_FORMAT "\n", 
              getFileName().c_str(), start, end);

    printRegions ("before insertion");
    if (regions.empty()) {
        regions.insert(region);
        printRegions ("after insertion");
        return;
    }
    FileRegions::iterator iter = regions.upper_bound(region);

    if (iter != regions.begin()) {
        --iter;
        assert (iter->start <= start);
        if (region.joinIfIntersects(*iter)) {

            iter = regions.erase (iter);
        } else {
            ++iter;
        }
    }

    while (iter != regions.end()) {
        if (!region.joinIfIntersects(*iter)) {
            break;
        }
        iter = regions.erase (iter);
    }
    if (iter != regions.begin()) {
        --iter;
    }
    regions.insert (iter, region);
    printRegions ("after insertion");
}

/*
 * Writes all file regions detected by the profiler.
 */
void MonitoredFile::save(JQSProfileWriter& writer) {
    assert (!discarded);
    for (FileRegions::const_iterator it = regions.begin (); it != regions.end (); ++it) {
        writer.writeRegion(it->start, it->end);
    }
}

/*
 * Applies matching substitutions to the full file name and returns 
 * the JQS configuration file entry according to kind of access
 * to the file detected by the profiler.
 */
const string& MonitoredFile::getConfig(const Substitutions* subs) {
    ostringstream os;

    string fname = subs->makeSubstitutions(fileName);

    switch (accessKind) {
        case AK_Unknown:
            break;

        case AK_Read:
            os << CMD_REFRESH << " \"" << fname << "\"";
            config = os.str();
            break;

        case AK_Mapped:
            os << CMD_REFRESH << " \"" << fname << "\" " << OPTION_MAPPED;
            config = os.str();
            break;

        case AK_Library:
            os << CMD_REFRESHLIB << " \"" << fname << "\"";
            config = os.str();
            break;
    }
    return config;
}


//////////////////////////////////////////////////////////////////////////

/*
 * The JQS profile implementation. Inherits JQSProfileContentHandler 
 * interface and therefore it is able to be passed to ReadJQSProfile
 * as content handler object. Upon JQS profile parsing, creates monitored 
 * file objects and fills the region information according to the profile.
 * It is also able to create a configuration file template.
 */
class JQSProfile : public JQSProfileContentHandler {
    string profileFileName;
    string configFileName;

    /*
     * Synchronizes accesses to the JQS profile.
     */
    CriticalSection profileLock;
    MonitoredFile* curMonitoredFile;

    /*
     * Mapping of a short file names to monitored file objects.
     */
    typedef map<string, MonitoredFile*> MonitoredFiles;
    MonitoredFiles monitoredFiles;

    /*
     * List of monitored files, in order of appearance in the profile file.
     */
    typedef vector<MonitoredFile*> MonitoredFilesList;
    MonitoredFilesList monitoredFilesList;

public:
    JQSProfile(const string& profileFileName, const string& configFileName);
    ~JQSProfile();

    /*
     * Returns the lock through which all accesses to the JQS profile should 
     * be synchronized.
     */
    CriticalSection& getLock () {
        return profileLock;
    }

    /*
     * Either finds existing monitored file by given name or creates a new monitored 
     * file object. Updates its access kind.
     */
    MonitoredFile* getMonitoredFile (const string& fileName, FileAccessKind ak);

    /*
     * Loads profile from the profile file specified in constructor.
     */
    void load();
    
    /*
     * Saves updated profile and configuration file template.
     */
    void save();

    /*
     * JQSProfileContentHandler interface implementation.
     */
    virtual void openFileSection(const std::string& file);
    virtual void setConfig(const std::string& config);
    virtual void addRegion (uint64_t start, uint64_t end);
    virtual void closeFileSection();
};


JQSProfile::JQSProfile(const string& profileFileName_, const string& configFileName_)
    : profileFileName(profileFileName_)
    , configFileName(configFileName_)
{
    load();
}

JQSProfile::~JQSProfile() {
    for (size_t i = 0; i < monitoredFilesList.size(); i++) {
        delete monitoredFilesList[i];
    }
}

/*
 * Either finds existing monitored file by given name or creates a new monitored 
 * file object. Updates its access kind.
 */
MonitoredFile* JQSProfile::getMonitoredFile (const string& fileName, FileAccessKind ak) {
    string shortName = tolowercase(getBaseName (fileName));
    MonitoredFile* file =  monitoredFiles[shortName];
    if (!file) {
        file = new MonitoredFile(shortName);
        monitoredFiles[shortName] = file;
        monitoredFilesList.push_back(file);
    }
    if (file->getAccessKind() < ak) {
        file->setFileName(fileName);
        file->setAccessKind(ak);
    }
    return file;
}

/*
 * JQSProfileContentHandler implementation
 */
void JQSProfile::openFileSection(const std::string& shortName) {
    curMonitoredFile = getMonitoredFile(shortName, AK_Unknown);
}

void JQSProfile::setConfig(const std::string& config) {
    assert (curMonitoredFile);
    curMonitoredFile->setConfig(config);
}

void JQSProfile::addRegion (uint64_t start, uint64_t end) {
    assert (curMonitoredFile);
    curMonitoredFile->touchRegion (start, end);
}

void JQSProfile::closeFileSection() {
    curMonitoredFile = NULL;
}

/*
 * Loads profile from the profile file specified in constructor.
 */
void JQSProfile::load() {
    CriticalSection::Lock lock(profileLock);

    ReadJQSProfile(profileFileName.c_str(), this);
    curMonitoredFile = NULL;
}

/*
 * Saves updated profile and configuration file template.
 */
void JQSProfile::save() {
    CriticalSection::Lock lock(profileLock);

    FILE* outProfile = fopen (profileFileName.c_str(), "w");
    if (!outProfile) {
        jqs_error ("Unable to write %s: %s\n", profileFileName.c_str(), strerror(errno));
        return;
    }

    FILE* outConfig = fopen(configFileName.c_str(), "w");
    if (!outConfig) {
        jqs_error ("Unable to write %s: %s\n", configFileName.c_str(), strerror(errno));
        fclose (outProfile);
        return;
    }

    JQSProfileWriter writer(outProfile, outConfig);
    Substitutions subs;

    for (size_t i = 0; i < monitoredFilesList.size(); i++) {
        MonitoredFile* file = monitoredFilesList[i];
        if (!file->isDiscarded()) {
            writer.openFileSection (file->getShortFileName().c_str (), 
                                    file->getConfig(&subs).c_str ());
            file->save(writer);
            writer.closeFileSection ();
        }
    }
//    fprintf (outConfig, "\n%s\n# append profile file here\n", CMD_PROFILE);
    fclose (outProfile);
    fclose (outConfig);
}


//////////////////////////////////////////////////////////////////////////

/*
 * Obtains the working set from the system, detects whether particular
 * page correspond to a memory mapped file, and updates the JQS profile.
 */
class WorkingSetProfiler  {
    vector<char> workingSetBuffer;

    typedef map<ULONG_PTR, uint64_t> MappingInfo;
    MappingInfo mappingInfo;
    CriticalSection mappingInfoLock;

    /*
     * The JQS profile object which is used to store the profile information
     * collected by the working set profiler object.
     */
    JQSProfile* profile;

    MEMORY_BASIC_INFORMATION cachedMemInfo;
    MonitoredFile* cachedMappedFile;
    uint64_t       cachedMappedFileOffset;

    /*
     * Clears cached memory info and cachedMappedFile objects.
     */
    void clearRegionInfo();

    /*
     * Gets information about a memory region containing given address, ensures
     * that the region belongs to a memory mapped file, detects the full 
     * name of the file, asks JQS profile object for a proper monitored file instance
     * and sets cachedMappedFile to the monitored file obtained.
     */
    void updateRegionInfo(ULONG_PTR addr);

    /*
     * Updates cachedMemInfo and cachedMappedFile objects according to given address.
     * If the page belongs to a memory mapped file, it calculates offset of the page 
     * in the file and updates profile information of the cachedMappedFile.
     */
    void addWSPage (ULONG_PTR addr);

    /*
     * Obtains working set, sorts pages and updates the JQS profile 
     * iterating pages from higher to lower addresses.
     */
    void scanWorkingSet();

public:
    WorkingSetProfiler(JQSProfile* profile);

    /*
     * Obtains working set, and updates the JQS profile.
     */
    void takeSnapshot();

    /*
     * Informs the profiler that file mapping with the specified base address 
     * was obtained from a file at given offset.
     */
    void addMappingInfo (ULONG_PTR baseAddress, uint64_t fileOffset);

    /*
     * Informs the profiler that file mapping with the specified base address 
     * was closed.
     */
    void removeMappingInfo (ULONG_PTR baseAddress);
};

/*
 * Working set profiler instance.
 */
static WorkingSetProfiler* workingSetProfiler = NULL;



WorkingSetProfiler::WorkingSetProfiler(JQSProfile* profile_)
    : profile(profile_)
    , workingSetBuffer(INITIAL_WORKING_SET_BUFFER_SIZE)
{
}

/*
 * Informs the profiler that file mapping with the specified base address 
 * was obtained from a file at given offset.
 */
void WorkingSetProfiler::addMappingInfo (ULONG_PTR baseAddress, uint64_t fileOffset) {
    CriticalSection::Lock lock(mappingInfoLock);

    mappingInfo[baseAddress] = fileOffset;
}

/*
 * Informs the profiler that file mapping with the specified base address 
 * was closed.
 */
void WorkingSetProfiler::removeMappingInfo (ULONG_PTR baseAddress) {
    CriticalSection::Lock lock(mappingInfoLock);

    mappingInfo.erase (baseAddress);
}

/*
 * Parses PE header of the given module and checks if the module was relocated.
 */
static bool isModuleRelocated(const void* baseAddress) {
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER) baseAddress;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        return false;
    }

    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS) ((const char*)baseAddress + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        return false;
    }

    return (((ULONG_PTR)baseAddress) != ((ULONG_PTR)ntHeaders->OptionalHeader.ImageBase));
}

/*
 * Clears cached memory info and cachedMappedFile objects.
 */
void WorkingSetProfiler::clearRegionInfo() {
    memset(&cachedMemInfo, 0, sizeof(cachedMemInfo));
    cachedMappedFile = NULL;
    cachedMappedFileOffset = 0;
}

/*
 * Gets information about a memory region containing given address, ensures
 * that the region belongs to a memory mapped file, detects the full 
 * name of the file, asks JQS profile object for a proper monitored file instance
 * and sets cachedMappedFile to the monitored file obtained.
 */
void WorkingSetProfiler::updateRegionInfo(ULONG_PTR addr) {
    if (((ULONG_PTR)cachedMemInfo.AllocationBase <= addr) &&
        (addr < (ULONG_PTR)cachedMemInfo.BaseAddress + cachedMemInfo.RegionSize))
    {
        // reuse last obtained memory region information 
        return;
    }

    // obtain memory region information

    clearRegionInfo();

    if (VirtualQuery((LPCVOID)addr, &cachedMemInfo, sizeof(cachedMemInfo)) != sizeof(cachedMemInfo)) {
        jqs_warn ("Unable to get memory region information: VirtualQuery failed on addr " PTR_FORMAT " (error %d)\n", addr, GetLastError());
        return;
    }

    if ((cachedMemInfo.Type & (MEM_IMAGE | MEM_MAPPED)) == 0) {
        jqs_info (5, "Region " PTR_FORMAT "-" PTR_FORMAT " - neither mapped file nor executable image\n",
                    (ULONG_PTR)cachedMemInfo.AllocationBase,
                    (ULONG_PTR)cachedMemInfo.BaseAddress + cachedMemInfo.RegionSize);
        return;
    }

    // executable image or memory-mapped file

    MonitoredFile* file = NULL;

    if (cachedMemInfo.Type & MEM_IMAGE) {
        vector<char> libPath(MAX_PATH);
        DWORD res = GetModuleFileName((HMODULE)cachedMemInfo.AllocationBase, &libPath[0], (DWORD)libPath.size());
        if ((res != 0) && (res < (DWORD)libPath.size())) {

            jqs_info (5, "Region " PTR_FORMAT "-" PTR_FORMAT " - library \"%s\"\n",
                         (ULONG_PTR)cachedMemInfo.AllocationBase,
                         (ULONG_PTR)cachedMemInfo.BaseAddress + cachedMemInfo.RegionSize, &libPath[0]);

            file = profile->getMonitoredFile(getFullPath(&libPath[0]), AK_Library);
            assert (file);

            if (isModuleRelocated(cachedMemInfo.AllocationBase)) {
                file->setRelocated();
            }
        }
    }

    if (!file) {
        vector<char> mappedPath(MAX_PATH);
        if (!_GetMappedFileName(CurProcessHandle, cachedMemInfo.AllocationBase,
                                &mappedPath[0], (DWORD)mappedPath.size()))
        {
            DWORD lastError = GetLastError();
            if (lastError == ERROR_FILE_INVALID) {
                jqs_info (5, "Region " PTR_FORMAT "-" PTR_FORMAT " - GetMappedFileName failed with ERROR_FILE_INVALID\n",
                            (ULONG_PTR)cachedMemInfo.AllocationBase,
                            (ULONG_PTR)cachedMemInfo.BaseAddress + cachedMemInfo.RegionSize);
            } else {
                jqs_warn("Unable to get memory region " PTR_FORMAT "-" PTR_FORMAT " information: GetMappedFileName failed (error %d).\n",
                         (ULONG_PTR)cachedMemInfo.AllocationBase,
                         (ULONG_PTR)cachedMemInfo.BaseAddress + cachedMemInfo.RegionSize, lastError);
            }
            return;
        }

        jqs_info (5, "Region " PTR_FORMAT "-" PTR_FORMAT " - mapped file \"%s\"\n",
                     (ULONG_PTR)cachedMemInfo.AllocationBase,
                     (ULONG_PTR)cachedMemInfo.BaseAddress + cachedMemInfo.RegionSize, &mappedPath[0]);

        file = profile->getMonitoredFile(getFullPath(convertMappedFileName(&mappedPath[0])), AK_Mapped);
        assert (file);

        {
            CriticalSection::Lock lock(mappingInfoLock);

            MappingInfo::const_iterator it = mappingInfo.find ((ULONG_PTR)cachedMemInfo.AllocationBase);
            if (it != mappingInfo.end()) {
                cachedMappedFileOffset = it->second;
            }
        }
    }

    cachedMappedFile = file;
}

/*
 * Updates cachedMemInfo and cachedMappedFile objects according to given address.
 * If the page belongs to a memory mapped file, it calculates offset of the page 
 * in the file and updates profile information of the cachedMappedFile.
 */
void WorkingSetProfiler::addWSPage (ULONG_PTR addr) {
    jqs_info (6, "WS page " PTR_FORMAT "\n", addr);

    if ((addr < (ULONG_PTR)SystemInfo.lpMinimumApplicationAddress) ||
        (addr > (ULONG_PTR)SystemInfo.lpMaximumApplicationAddress)) 
    {
        return;
    }

    updateRegionInfo(addr);

    if (cachedMappedFile) {
        jqs_info (6, "WS page " PTR_FORMAT ": mapped region of %s touched (offset: " UINT64_FORMAT ")\n", addr,
                  cachedMappedFile->getFileName().c_str(), cachedMappedFileOffset);
        uint64_t start = addr - (ULONG_PTR)cachedMemInfo.AllocationBase + cachedMappedFileOffset;
        uint64_t end   = start + PAGE_SIZE;
        cachedMappedFile->touchRegion(start, end);

    } else {
        jqs_info (6, "WS page " PTR_FORMAT ": not mapped region\n", addr);
    }
}

/*
 * The ordering object used to sort working set pages.
 * The PSAPI_WORKING_SET_BLOCK objects are reverse ordered by the value of VirtualPage.
 */
struct WSPagesReverseOrder {
    bool operator () (const PSAPI_WORKING_SET_BLOCK& a, 
                      const PSAPI_WORKING_SET_BLOCK& b) const
    {
        return a.VirtualPage > b.VirtualPage;
    }
};

/*
 * Obtains working set, sorts pages and updates the JQS profile 
 * iterating pages from higher to lower addresses.
 */
void WorkingSetProfiler::scanWorkingSet() {
    assert(_QueryWorkingSet);
    jqs_info (2, "Scanning working set...\n");

    clearRegionInfo();

    for (;;) {
        size_t capacity = workingSetBuffer.size();
        PSAPI_WORKING_SET_INFORMATION* wsInfo = (PSAPI_WORKING_SET_INFORMATION*) (&workingSetBuffer[0]);
        BOOL res = _QueryWorkingSet(CurProcessHandle, wsInfo, (DWORD)capacity);

        if (res) {
            if (capacity >= (sizeof (wsInfo->NumberOfEntries) +
                             sizeof (wsInfo->WorkingSetInfo[0])*wsInfo->NumberOfEntries))
            {
                break;
            }
        } else {
            DWORD err = GetLastError();
            if (err != ERROR_BAD_LENGTH) {
                workingSetBuffer.clear();
                jqs_warn("Unable to get process working set information: QueryWorkingSet failed (error %d).\n", err);
                return;
            }
        }

        workingSetBuffer.clear();
        workingSetBuffer.resize(capacity*2);
    }

    PSAPI_WORKING_SET_INFORMATION* wsInfo = (PSAPI_WORKING_SET_INFORMATION*) (&workingSetBuffer[0]);
    jqs_info (4, "working set: %d pages\n", wsInfo->NumberOfEntries);

    // Sort pages of working set in reverse order to reuse
    // information about the last obtained memory region if possible.

    std::sort(&wsInfo->WorkingSetInfo[0], 
              &wsInfo->WorkingSetInfo[wsInfo->NumberOfEntries], 
              WSPagesReverseOrder());

    for (size_t i = 0; i < wsInfo->NumberOfEntries; i++) {
        addWSPage(wsInfo->WorkingSetInfo[i].VirtualPage * PAGE_SIZE);
    }
}


/*
 * Obtains working set, and updates the JQS profile.
 */
void WorkingSetProfiler::takeSnapshot() {
    CriticalSection::Lock lock(profile->getLock());

    {
        TraceTime t("scan working set", print_times >= 2);
        scanWorkingSet();
    }
}


//////////////////////////////////////////////////////////////////////////

/*
 * Calculates the working set size using given amount of physical memory and 
 * PHYS_MEMORY_OCCUPATION_RATIO constant and sets the process working set size.
 */
void adjustWorkingSetSize (SIZE_T totalPhysicalMemory) {
    if (!_GetProcessWorkingSetSize || !_SetProcessWorkingSetSize) {
        return;
    }

    SIZE_T size = (SIZE_T) (totalPhysicalMemory * PHYS_MEMORY_OCCUPATION_RATIO);

    working_set_size_t wss = {size, size};
    set_process_working_set_size(&wss);
}


//////////////////////////////////////////////////////////////////////////

/*
 * The profiler thread, periodically takes snapshots of the process' memory
 * and updates the JQS profile.
 */
class ProfilerThread : public Thread {
    volatile bool stop;
    HANDLE stopEvent;
    WorkingSetProfiler* wsProfiler;

public:
    ProfilerThread(WorkingSetProfiler* wsProfiler_)
        : stop(false)
        , wsProfiler(wsProfiler_)
    {
        stopEvent = CreateEvent(NULL, false, false, NULL);
        if (stopEvent == NULL) {
            jqs_error ("CreateEvent failed (error %d)\n", GetLastError());
            stop = true;
        }
    }

    virtual ~ProfilerThread() {
        if (stopEvent) {
            CloseHandle(stopEvent);
        }
    }

    /*
     * Periodically takes snapshots of the process' memory
     * and updates the JQS profile.
     */
    virtual void run();

    /*
     * Arranges the profiler thread to stop.
     */
    void stopThread();
};

/*
 * Periodically takes snapshots of the process' memory
 * and updates the JQS profile.
 */
void ProfilerThread::run() {
    while (!stop) {

        wsProfiler->takeSnapshot();

        WaitForSingleObject(stopEvent, PROFILER_INTERVAL);
    }
}

/*
 * Arranges the profiler thread to stop.
 */
void ProfilerThread::stopThread() {
    stop = true;
    if (stopEvent) {
        SetEvent(stopEvent);
    }
}


//////////////////////////////////////////////////////////////////////////

/*
 * Holds information about particular opened file, namely current position
 * of the file pointer and proper monitored file object.
 */
struct OpenedFile {
    uint64_t pos;
    MonitoredFile* file;

    OpenedFile() 
        : pos(0)
        , file(NULL)
    {}

    OpenedFile(MonitoredFile* file_) 
        : pos(0)
        , file(file_)
    {}
};

/*
 * Utility class, remembers the last error value in constructor and 
 * restores it in destructor.
 */
class LastErrorSaver {
    DWORD lastError;

public:
    LastErrorSaver() {
        lastError = GetLastError();
    }

    ~LastErrorSaver() {
        SetLastError(lastError);
    }

    DWORD getError() const {
        return lastError;
    }
};

/*
 * Descriptor of a system function hook.
 */
struct Hook {
    const char* FunctionName;
    void* HookFunctionAddr;
    void* OrigFunctionAddr;
};

/*
 * Manages hooks. 
 * To monitor file operations, a set of hooks is installed. Hook is a function, 
 * which replaces system function, providing additional functionality, such as 
 * monitoring. Hooks have the same prototype and calling conventions as original 
 * system function. After performing monitoring functions, hook calls original 
 * function to perform respective operation.
 * As most file operations are performed on file handles rather then file names, 
 * the mapping of file handle to file name is maintained for opened files. Also, 
 * the current file position is monitored for each file.
 * The following file operations are replaced by hooks:
 *   - opening a file: hook calls respective system function and updates the 
 *     handle-to-name mapping if open is successful.
 *   - setting a file pointer: hook calls respective system function and updates 
 *     file position information.
 *   - reading a file: hook calls respective system function and updates profile 
 *     information for the file, according to the file name, current file position, 
 *     and amount of bytes read.
 */
class HooksManager {
    set<HMODULE> patchedModules;
    CriticalSection hooksInstallationLock;

    JQSProfile* profile;

    typedef map<HANDLE, OpenedFile> OpenedFiles;
    OpenedFiles openedFiles;

    void installHooksForModule(bool install, HMODULE hModule);

public:
    HooksManager(JQSProfile* profile);

    /*
     * Enumerates all modules in the process and patches PE import section.
     * Depending on the install flag the system functions called from particular
     * module are either redirected to proper hook functions or the original 
     * functions are restored.
     */
    void installHooks(bool install = true);
    /*
     * Removes module from the list of patched modules, since the module is 
     * being unloaded.
     */
    void removeModule(HMODULE hModule);
    /*
     * If given function exported from given module is replaced by a hook,
     * the function returns the address of a hook function. Otherwise, it
     * returns NULL.
     * This function is used to replace a result of hooked GetProcAddress()
     * system function.
     */
    FARPROC findHook(HMODULE hModule, const char* funcName);

    /*
     * Registers an open file system call, obtains proper monitored file object
     * from the profile and associates a new OpenedFile object with the handle.
     */
    void openFile (HANDLE hFile, const string& fileName);

    /*
     * Registers file closing.
     */
    void closeFile (HANDLE hFile);
  
    /*
     * Searches for the associated monitored file and discards it.
     */
    void discardFile (HANDLE hFile);

    /*
     * Updates file pointer of the associated opened file.
     */
    void setFilePos (HANDLE hFile, uint64_t pos);

    /*
     * Registers access to a region of file.
     */
    void readFile (HANDLE hFile, uint64_t size);
};

/*
 * Hooks manager instance.
 */
static HooksManager* hooksManager = NULL;


HooksManager::HooksManager(JQSProfile* profile_)
    : profile(profile_)
{
}

/*
 * Catches LoadLibrary call and installs the hooks for recently loaded 
 * libraries.
 */
HMODULE WINAPI hookLoadLibraryA(LPCSTR lpLibFileName) {
    jqs_info (4, "Hook called: LoadLibraryA(%s)\n", lpLibFileName);
    HMODULE hModule = LoadLibraryA(lpLibFileName);
    if (hModule) {
        LastErrorSaver err;
        hooksManager->installHooks();
    }
    return hModule;
}

// see comment above
HMODULE WINAPI hookLoadLibraryW(LPCWSTR lpLibFileName) {
    jqs_info (4, "Hook called: LoadLibraryW\n");
    HMODULE hModule = LoadLibraryW (lpLibFileName);
    if (hModule) {
        LastErrorSaver err;
        hooksManager->installHooks();
    }
    return hModule;
}

// see comment above
HMODULE WINAPI hookLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
    jqs_info (4, "Hook called: LoadLibraryExA(%s)\n", lpLibFileName);
    HMODULE hModule = LoadLibraryExA(lpLibFileName, hFile, dwFlags);
    if (hModule) {
        LastErrorSaver err;
        hooksManager->installHooks();
    }
    return hModule;
}

// see comment above
HMODULE WINAPI hookLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
    jqs_info (4, "Hook called: LoadLibraryExW(%S)\n", lpLibFileName);
    HMODULE hModule = LoadLibraryExW(lpLibFileName, hFile, dwFlags);
    if (hModule) {
        LastErrorSaver err;
        hooksManager->installHooks();
    }
    return hModule;
}

/*
 * Catches FreeLibrary call and removes the module from the list of loaded 
 * libraries.
 */
BOOL WINAPI hookFreeLibrary (HMODULE hLibModule) {
    jqs_info (4, "Hook called: FreeLibrary(%x)\n", hLibModule);
    hooksManager->removeModule(hLibModule);
    return FreeLibrary(hLibModule);
}

/*
 * Catches GetProcAddress, checks if the requested function should be replaced
 * with a hook and returns either the hook function address or the result of
 * original system function.
 */
FARPROC WINAPI hookGetProcAddress (HMODULE hModule, LPCSTR lpProcName) {
    if (((UINT_PTR)lpProcName) >= 0x10000) {
        // by function name
        jqs_info (4, "Hook called: GetProcAddress(%x, %s)\n", hModule, lpProcName);
        FARPROC hook = hooksManager->findHook (hModule, lpProcName);
        if (hook) {
            return hook;
        }
    } else {
        // by ordinal
        jqs_info (4, "Hook called: GetProcAddress(%x, %d)\n", hModule, lpProcName);
    }
    return GetProcAddress(hModule, lpProcName);

}

/*
 * Catches CreateFile and informs the hooks manager that the file is opened.
 */
HANDLE WINAPI hookCreateFileA(LPCSTR lpFileName, 
                              DWORD dwDesiredAccess, 
                              DWORD dwShareMode,
                              LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                              DWORD dwCreationDisposition,
                              DWORD dwFlagsAndAttributes,
                              HANDLE hTemplateFile)
{
    HANDLE hFile = CreateFileA(lpFileName, 
                               dwDesiredAccess, 
                               dwShareMode,
                               lpSecurityAttributes,
                               dwCreationDisposition,
                               dwFlagsAndAttributes,
                               hTemplateFile);
    LastErrorSaver err;

    jqs_info (4, "Hook called: CreateFileA(%s) = %x\n", lpFileName, hFile);

    if ((hFile != INVALID_HANDLE_VALUE) && (GetFileType(hFile) == FILE_TYPE_DISK)) {
        hooksManager->openFile (hFile, lpFileName);
        if (dwDesiredAccess & GENERIC_WRITE) {
            hooksManager->discardFile (hFile);
        }
    }
    return hFile;
}

// see the comment above
HANDLE WINAPI hookCreateFileW(LPCWSTR lpFileName,
                              DWORD dwDesiredAccess,
                              DWORD dwShareMode,
                              LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                              DWORD dwCreationDisposition,
                              DWORD dwFlagsAndAttributes,
                              HANDLE hTemplateFile)
{
    HANDLE hFile = CreateFileW(lpFileName, 
                               dwDesiredAccess, 
                               dwShareMode,
                               lpSecurityAttributes,
                               dwCreationDisposition,
                               dwFlagsAndAttributes,
                               hTemplateFile);
    LastErrorSaver err;

    jqs_info (4, "Hook called: CreateFileW(%S) = %x\n", lpFileName, hFile);

    if ((hFile != INVALID_HANDLE_VALUE) && (GetFileType(hFile) == FILE_TYPE_DISK)) {
        hooksManager->openFile (hFile, convUnicodeToString (lpFileName));
        if (dwDesiredAccess & GENERIC_WRITE) {
            hooksManager->discardFile (hFile);
        }
    }
    return hFile;
}

/*
 * Catches ReadFile and informs the hooks manager that the file is accessed.
 */
BOOL WINAPI hookReadFile(HANDLE hFile,
                         LPVOID lpBuffer,
                         DWORD nNumberOfBytesToRead,
                         LPDWORD lpNumberOfBytesRead,
                         LPOVERLAPPED lpOverlapped)
{
    jqs_info (4, "Hook called: ReadFile(%x)\n", hFile);

    BOOL res = ReadFile(hFile,
                        lpBuffer,
                        nNumberOfBytesToRead,
                        lpNumberOfBytesRead,
                        lpOverlapped);
    if (res) {
        LastErrorSaver err;
        hooksManager->readFile (hFile, *lpNumberOfBytesRead);
    }
    return res;
}

/*
 * Catches ReadFile and informs the hooks manager that current position of 
 * the file is changed.
 */
DWORD WINAPI hookSetFilePointer(HANDLE hFile,
                                LONG lDistanceToMove,
                                PLONG lpDistanceToMoveHigh,
                                DWORD dwMoveMethod)
{
    jqs_info (4, "Hook called: SetFilePointer(%x)\n", hFile);
    LONG distanceToMoveHigh = lpDistanceToMoveHigh ? *lpDistanceToMoveHigh : 0;
    DWORD res = SetFilePointer(hFile,
                               lDistanceToMove,
                               &distanceToMoveHigh,
                               dwMoveMethod);
    LastErrorSaver err;
    if ((res != INVALID_SET_FILE_POINTER) || (err.getError() != ERROR_SUCCESS)) {
        LARGE_INTEGER li;
        li.LowPart  = res;
        li.HighPart = distanceToMoveHigh;
        hooksManager->setFilePos (hFile, li.QuadPart);
    }
    if (lpDistanceToMoveHigh) {
        *lpDistanceToMoveHigh = distanceToMoveHigh;
    }
    return res;
}

// see comment above
BOOL WINAPI hookSetFilePointerEx(HANDLE hFile,
                                 LARGE_INTEGER liDistanceToMove,
                                 PLARGE_INTEGER lpNewFilePointer,
                                 DWORD dwMoveMethod)
{
    if (!_SetFilePointerEx) {
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }
    jqs_info (4, "Hook called: SetFilePointerEx(%x)\n", hFile);
    LARGE_INTEGER newFilePointer;
    BOOL res = _SetFilePointerEx(hFile,
                                  liDistanceToMove,
                                  &newFilePointer,
                                  dwMoveMethod);
    LastErrorSaver err;
    if (res) {
        hooksManager->setFilePos (hFile, newFilePointer.QuadPart);
    }
    if (lpNewFilePointer) {
        *lpNewFilePointer = newFilePointer;
    }
    return res;
}

/*
 * Catches MapViewOfFile and informs the working set profiler that a file
 * was mapped at specified offset.
 */
LPVOID WINAPI hookMapViewOfFile(HANDLE hFileMappingObject,
                                DWORD dwDesiredAccess,
                                DWORD dwFileOffsetHigh,
                                DWORD dwFileOffsetLow,
                                SIZE_T dwNumberOfBytesToMap)
{
    jqs_info (4, "Hook called: MapViewOfFile()\n");
    LPVOID res = MapViewOfFile(hFileMappingObject,
                               dwDesiredAccess,
                               dwFileOffsetHigh,
                               dwFileOffsetLow,
                               dwNumberOfBytesToMap);
    LastErrorSaver err;
    if (res != NULL) {
        ULARGE_INTEGER li;
        li.LowPart  = dwFileOffsetLow;
        li.HighPart = dwFileOffsetHigh;
        workingSetProfiler->addMappingInfo ((ULONG_PTR)res, li.QuadPart);
    }
    return res;
}

// see comment above
LPVOID WINAPI hookMapViewOfFileEx(HANDLE hFileMappingObject,
                                  DWORD dwDesiredAccess,
                                  DWORD dwFileOffsetHigh,
                                  DWORD dwFileOffsetLow,
                                  SIZE_T dwNumberOfBytesToMap,
                                  LPVOID lpBaseAddress)
{
    jqs_info (4, "Hook called: MapViewOfFileEx()\n");
    LPVOID res = MapViewOfFileEx(hFileMappingObject,
                                 dwDesiredAccess,
                                 dwFileOffsetHigh,
                                 dwFileOffsetLow,
                                 dwNumberOfBytesToMap,
                                 lpBaseAddress);
    LastErrorSaver err;
    if (res != NULL) {
        ULARGE_INTEGER li;
        li.LowPart  = dwFileOffsetLow;
        li.HighPart = dwFileOffsetHigh;
        workingSetProfiler->addMappingInfo ((ULONG_PTR)res, li.QuadPart);
    }
    return res;
}

/*
 * Catches UnmapViewOfFile and informs the working set profiler that a file
 * was unmapped.
 */
BOOL WINAPI hookUnmapViewOfFile(LPCVOID lpBaseAddress) {
    jqs_info (4, "Hook called: UnmapViewOfFile()\n");
    BOOL res = UnmapViewOfFile(lpBaseAddress);

    LastErrorSaver err;
    if (res) {
        workingSetProfiler->removeMappingInfo ((ULONG_PTR)lpBaseAddress);
    }
    return res;
}

/*
    File management functions that are not caught by the profiler:

    DuplicateHandle
    OpenFile [16-bit Windows]
    OpenFileById [Vista only]
    ReOpenFile [Vista only]
    ReadFileEx [Asynchronous I/O]
    ReadFileScatter
    SetEndOfFile
    SetFileIoOverlappedRange [Vista only]
    WriteFile
    WriteFileEx [Asynchronous I/O]
    WriteFileGather
    _lopen [16-bit Windows]
    _lcreat [16-bit Windows]
    _lread [16-bit Windows]
    _lwrite [16-bit Windows]
    _hread [16-bit Windows]
    _hwrite [16-bit Windows]
    _lclose [16-bit Windows]
    _llseek [16-bit Windows]
   
*/

#define KERNEL32_NAME       "KERNEL32.DLL"

/*
 * A set of hooks installed by a HookManager to kernel32.dll.
 */
static struct Hook Kernel32Hooks[] =
{
    {"LoadLibraryA",        hookLoadLibraryA,       NULL},
    {"LoadLibraryW",        hookLoadLibraryW,       NULL},
    {"LoadLibraryExA",      hookLoadLibraryExA,     NULL},
    {"LoadLibraryExW",      hookLoadLibraryExW,     NULL},
    {"FreeLibrary",         hookFreeLibrary,        NULL},
    {"GetProcAddress",      hookGetProcAddress,     NULL},
    {"CreateFileA",         hookCreateFileA,        NULL},
    {"CreateFileW",         hookCreateFileW,        NULL},
    {"ReadFile",            hookReadFile,           NULL},
    {"SetFilePointer",      hookSetFilePointer,     NULL},
    {"SetFilePointerEx",    hookSetFilePointerEx,   NULL},
    {"MapViewOfFile",       hookMapViewOfFile,      NULL},
    {"MapViewOfFileEx",     hookMapViewOfFileEx,    NULL},
    {"UnmapViewOfFile",     hookUnmapViewOfFile,    NULL},
};

/*
 * Number of hooks.
 */
const size_t N_KERNEL32_HOOKS = sizeof(Kernel32Hooks)/sizeof(Kernel32Hooks[0]);

/*
 * Parses PE header of given module and returns the pointer to 
 * IMAGE_IMPORT_DESCRIPTOR structure.
 */
static PIMAGE_IMPORT_DESCRIPTOR getModuleImportDescriptor(const char* baseAddress) {
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER) baseAddress;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        return NULL;
    }

    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS) (baseAddress + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        return NULL;
    }

    DWORD importDirVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    DWORD importDirSz = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;

    if ((importDirVA == 0) || (importDirSz == 0)) {
        return NULL;
    }

    return (PIMAGE_IMPORT_DESCRIPTOR) (baseAddress + importDirVA);
}

/*
 * Searches for an import descriptor for given library name in 
 * the module's import table.
 */
static PIMAGE_IMPORT_DESCRIPTOR findImportedLibrary(const char* baseAddress,
                                                    PIMAGE_IMPORT_DESCRIPTOR importDesc,
                                                    const char* libraryName)
{
    while (importDesc->Characteristics != 0) {
        const char* name = baseAddress + importDesc->Name;
        if (!_stricmp(name, libraryName)) {
            return importDesc;
        }
        importDesc++;
    }
    return NULL;
}

/*
 * Sets read/write protection flag for a page of memory, updates the location 
 * with a new value and restores the protection of the page.
 */
static bool patchReadonlyLocation(void* location, void* value) {
    DWORD oldProtect;
    if (!VirtualProtect (location, sizeof(value), PAGE_READWRITE, &oldProtect)) {
        jqs_warn ("Unable to patch location %x: VirtualProtect failed (error %d)\n", location, GetLastError());
        return false;
    }

    * ((void **)location) = value;

    if (!VirtualProtect (location, sizeof(value), oldProtect, &oldProtect)) {
        jqs_warn ("Unable to restore memory protection while patching location %x: VirtualProtect failed (error %d)\n", location, GetLastError());
    }

    return true;
}

/*
 * Patches given import entry with given hook function.
 */
static void patchImportEntry (char* baseAddress,
                              PIMAGE_IMPORT_DESCRIPTOR libImportDesc,
                              const char* libraryName,
                              const char* functionName,
                              void* hookFunctionAddr)
{
    PIMAGE_THUNK_DATA originalThunk = (PIMAGE_THUNK_DATA) (baseAddress + libImportDesc->OriginalFirstThunk);
    PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA) (baseAddress + libImportDesc->FirstThunk);

    while (originalThunk->u1.Ordinal != 0) {
        if (!(originalThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)) {
            PIMAGE_IMPORT_BY_NAME hintName = (PIMAGE_IMPORT_BY_NAME) (baseAddress + originalThunk->u1.AddressOfData);
            const char* funcName = (const char*) &hintName->Name[0];
            if (!strcmp(funcName, functionName)) {
                if (patchReadonlyLocation (&(thunk->u1.Function), hookFunctionAddr)) {
                    jqs_info (4, "Hook function %s$%s set to " PTR_FORMAT "\n", libraryName, functionName, baseAddress);
                }
                return;
            }
        }
        originalThunk++;
        thunk++;
    }

    jqs_info (4, "Unable to set hook: function %s$%s is not imported to " PTR_FORMAT "\n", libraryName, functionName, baseAddress);
}

/*
 * Patches PE import section of given loaded module.
 * Depending on the install flag the system functions called from the
 * module are either redirected to proper hook functions or restored to 
 * their original implementations.
 */
static void installHooksForImportedLibrary (bool install, 
                                            HMODULE hModule,
                                            const char* libraryName,
                                            size_t nHooks,
                                            const Hook* hooks)
{
    char* baseAddress = (char *) hModule;

    PIMAGE_IMPORT_DESCRIPTOR firstImportDesc = getModuleImportDescriptor(baseAddress);
    if (!firstImportDesc) {
        jqs_info (4, "Setting hooks for %x: PE module import not found\n", hModule);
        return;
    }

    PIMAGE_IMPORT_DESCRIPTOR libImportDesc = findImportedLibrary(baseAddress, firstImportDesc, libraryName);
    if (!libImportDesc) {
        jqs_info (4, "Setting hooks for %x: hooks are not installed as library %s is not imported\n", baseAddress, libraryName);
        return;
    }

    for (size_t i = 0; i < nHooks; i++) {
        const Hook& hook = hooks[i];
        if (hook.OrigFunctionAddr) {
            patchImportEntry (baseAddress, 
                              libImportDesc, 
                              libraryName,
                              hook.FunctionName, 
                              install ? hook.HookFunctionAddr : hook.OrigFunctionAddr);
        }
    }
}

/*
 * Patches PE import section of given loaded module.
 * Depending on the install flag the system functions called from the
 * module are either redirected to proper hook functions or restored to 
 * their original implementations.
 */
void HooksManager::installHooksForModule(bool install, HMODULE hModule) {
    if (hModule == hAgentInstance) {
        // avoid installation of hooks for JQS profiler agent library
        return;
    }

    CriticalSection::Lock lock(hooksInstallationLock);
    
    if (patchedModules.find(hModule) != patchedModules.end()) {
        return;
    }

    if (verbose >= 3) {
        vector<char> modName(MAX_PATH);

        // Get the full path to the module's file.
        if (GetModuleFileName(hModule, &modName[0], (DWORD)modName.size())) {
            // Print the module name and handle value.
            jqs_info (3, "%s hooks for %x: %s\n", 
                (install ? "Installing" : "Uninstalling"), hModule, &modName[0]);
        } else {
            jqs_warn ("%s hooks for %x: unable to get module name\n", 
                (install ? "Installing" : "Uninstalling"), hModule);
        }
    }

    patchedModules.insert(hModule);

    installHooksForImportedLibrary (install, hModule, KERNEL32_NAME, N_KERNEL32_HOOKS, Kernel32Hooks);
}

/*
 * Enumerates all modules in the process and patches PE import section.
 * Depending on the install flag the system functions called from particular
 * module are either redirected to proper hook functions or restored to 
 * their original implementations.
 */
void HooksManager::installHooks(bool install) {
    vector<HMODULE> buf(42);
    size_t nModules = 0;

    for (;;) {
        DWORD cb = (DWORD) (buf.size()*sizeof(HMODULE));
        DWORD cbNeeded = 0;
        if (!_EnumProcessModules (CurProcessHandle, &buf[0], cb, &cbNeeded)) {
            jqs_warn("Unable to enumerate modules of the process: EnumProcessModules failed (error %d)\n", GetLastError());
            return;
        }

        nModules = cbNeeded / sizeof(HMODULE);

        if (nModules <= buf.size()) {
            break;
        }

        buf.clear();
        buf.resize(nModules);
    }

    if (!install) {
        patchedModules.clear();
    }

    for (size_t i = 0; i < nModules; i++) {
        installHooksForModule (install, buf[i]);
    }
}

/*
 * Removes module from the list of patched modules, since the module is 
 * being unloaded.
 */
void HooksManager::removeModule(HMODULE hModule) {
    CriticalSection::Lock lock(hooksInstallationLock);
    patchedModules.erase(hModule);
}

/*
 * If given function exported from given module is replaced by a hook,
 * the function returns the address of a hook function. Otherwise, it
 * returns NULL.
 * This function is used to replace a result of hooked GetProcAddress()
 * system function.
 */
FARPROC HooksManager::findHook(HMODULE hModule, const char* funcName) {
    if (hModule == Kernel32Handle) {
        for (size_t i = 0; i < N_KERNEL32_HOOKS; i++) {
            if (!strcmp(funcName, Kernel32Hooks[i].FunctionName)) {
                return (FARPROC) (Kernel32Hooks[i].HookFunctionAddr);
            }
        }
    }
    return NULL;
}

/*
 * Registers an open file system call, obtains proper monitored file object
 * from the profile and associates a new OpenedFile object with the handle.
 */
void HooksManager::openFile (HANDLE hFile, const string& fileName) {
    CriticalSection::Lock lock(profile->getLock());

    MonitoredFile* file = profile->getMonitoredFile (getFullPath (fileName), AK_Read);
    assert(file);
    openedFiles [hFile] = OpenedFile(file);
}

/*
 * Registers file closing.
 */
void HooksManager::closeFile (HANDLE hFile) {
    CriticalSection::Lock lock(profile->getLock());
    openedFiles.erase (hFile);
}

/*
 * Searches for the associated monitored file and discards it.
 */
void HooksManager::discardFile (HANDLE hFile) {
    CriticalSection::Lock lock(profile->getLock());

    OpenedFiles::iterator iter = openedFiles.find (hFile);
    if (iter == openedFiles.end()) {
        return;
    }
    MonitoredFile* file = iter->second.file;
    file->discard ();
    openedFiles.erase (iter);
}

/*
 * Updates file pointer of the associated opened file.
 */
void HooksManager::setFilePos (HANDLE hFile, uint64_t pos) {
    CriticalSection::Lock lock(profile->getLock());
    OpenedFiles::iterator iter = openedFiles.find (hFile);
    if (iter != openedFiles.end()) {
        iter->second.pos = pos;
    }
}

/*
 * Registers access to a region of file.
 */
void HooksManager::readFile (HANDLE hFile, uint64_t size) {
    if (size == 0) {
        return;
    }

    CriticalSection::Lock lock(profile->getLock());

    OpenedFiles::iterator iter = openedFiles.find (hFile);
    if (iter == openedFiles.end()) {
        return;
    }
    MonitoredFile* file = iter->second.file;
    uint64_t pos = iter->second.pos;
    file->touchRegion (pos, pos + size);
    iter->second.pos = pos + size;
}


//////////////////////////////////////////////////////////////////////////

/*
 * Profiler thread instance.
 */
static ProfilerThread* profilerThread = NULL;

/*
 * JQS profile instance.
 */
static JQSProfile* jqsProfile = NULL;

/*
 * Initializes profiler, installs system hooks, adjusts working set size
 * and starts the profiler thread.
 */
static bool initProfiler() {
    openLogFile (getSatelliteFileName ("jqsagent.log", hAgentInstance));

    verbose = JQS_PROFILER_VERBOSE;
    print_times = JQS_PROFILER_TIMINGS;
    jqs_info (1, "Initializing JQS profiler...\n");

    initOSLayer();
    initTimer();

    if (!_QueryWorkingSet ||
        !_GetModuleInformation ||
        !_GetMappedFileName ||
        !_EnumProcessModules)
    {
        jqs_error ("JQS profiler does not support your system (PSAPI is not available)\n");
        return false;
    }

    Kernel32Handle = GetModuleHandle(KERNEL32_NAME);
    if (!Kernel32Handle) {
        jqs_error ("GetModuleHandle(%s) failed (error %d)\n", KERNEL32_NAME, GetLastError());
        return false;
    }
    for (size_t i = 0; i < N_KERNEL32_HOOKS; i++) {
        FARPROC proc = GetProcAddress(Kernel32Handle, Kernel32Hooks[i].FunctionName);
        Kernel32Hooks[i].OrigFunctionAddr = proc;
        if (!proc) {
            DWORD lastError = GetLastError();
            if (lastError == ERROR_PROC_NOT_FOUND) {
                jqs_info (4, "Function %s$%s was not found\n", 
                    KERNEL32_NAME, Kernel32Hooks[i].FunctionName);
            } else {
                jqs_warn("Unable to find function %s$%s (error %d)\n", 
                    KERNEL32_NAME, Kernel32Hooks[i].FunctionName, lastError);
            }
        }
    }

    jqsProfile = new JQSProfile(getSatelliteFileName ("jqs.profile", hAgentInstance),
                                getSatelliteFileName ("jqs.config.template", hAgentInstance));

    hooksManager = new HooksManager(jqsProfile);
    hooksManager->installHooks();

    workingSetProfiler = new WorkingSetProfiler(jqsProfile);
    profilerThread = new ProfilerThread(workingSetProfiler);

    MEMORYSTATUS memStatus;
    GlobalMemoryStatus (&memStatus);
    adjustWorkingSetSize (memStatus.dwTotalPhys);

    if (!profilerThread->start()) {
        jqs_error ("Unable to start JQS profiler thread\n");
        return false;
    }

    return true;
}

/*
 * Stops the profiler thread, uninstalls hooks and saves the updated profile.
 */
static void shutdownProfiler() {
    static bool stopped = false;
    if (stopped) {
        return;
    }
    stopped = true;

    profilerThread->stopThread();

    // restore original system functions
    hooksManager->installHooks(false);

    // save profile
    jqsProfile->save();

    jqs_info (1, "JQS profiler stopped\n");
}

/*
 * The JVMTI entry point, starts the profiler.
 */
JNIEXPORT jint JNICALL 
Agent_OnLoad(JavaVM *vm, char *options, void *reserved) {
    TRY {
        return initProfiler() ? 0 : -1;
    } CATCH_SYSTEM_EXCEPTIONS {
        return -1;
    }
}

/*
 * The JVMTI entry point, stops the profiler.
 */
JNIEXPORT void JNICALL 
Agent_OnUnload(JavaVM *vm) {
    TRY {
        shutdownProfiler();
    } CATCH_SYSTEM_EXCEPTIONS {
    }
}

/*
 * Since Internet Explorer does not call Agent_OnUnload upon exit, the
 * shutdownProfiler() is called when the agent library is unloaded.
 */
BOOL APIENTRY DllMain (HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hInstance);
            hAgentInstance = hInstance;
            break;

        case DLL_PROCESS_DETACH:
            TRY {
                shutdownProfiler();
            } CATCH_SYSTEM_EXCEPTIONS {
            }
            break;
    }
    return TRUE;
}
