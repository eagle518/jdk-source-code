#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)classLoader.cpp	1.165 04/05/21 16:00:21 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_classLoader.cpp.incl"
#include <sys/stat.h>


// Entry points in zip.dll for loading zip/jar file entries

typedef void * * (JNICALL *ZipOpen_t)(const char *name, char **pmsg);
typedef void * * (JNICALL *FindEntry_t)(jzfile *zip, const char *name, jint *sizeP, jint *nameLen);
typedef jboolean (JNICALL *ReadEntry_t)(jzfile *zip, jzentry *entry, unsigned char *buf, char *namebuf);
typedef jboolean (JNICALL *ReadMappedEntry_t)(jzfile *zip, jzentry *entry, unsigned char **buf, char *namebuf);
typedef void * * (JNICALL *GetNextEntry_t)(jzfile *zip, jint n);

static ZipOpen_t         ZipOpen            = NULL;
static FindEntry_t       FindEntry          = NULL;
static ReadEntry_t       ReadEntry          = NULL;
static ReadMappedEntry_t ReadMappedEntry    = NULL;
static GetNextEntry_t    GetNextEntry       = NULL;
static canonicalize_fn_t CanonicalizeEntry  = NULL;

// Globals

PerfCounter*    ClassLoader::_perf_accumulated_time = NULL;
PerfCounter*    ClassLoader::_perf_classes_inited = NULL;
PerfCounter*    ClassLoader::_perf_class_init_time = NULL;
PerfCounter*    ClassLoader::_perf_class_verify_time = NULL;

ClassPathEntry* ClassLoader::_first_entry         = NULL;
ClassPathEntry* ClassLoader::_last_entry          = NULL;
PackageHashtable* ClassLoader::_package_hash_table = NULL;

ClassPathEntry::ClassPathEntry() {
  set_next(NULL);
}


ClassPathDirEntry::ClassPathDirEntry(char* dir) : ClassPathEntry() {
  _dir = dir;
}


ClassFileStream* ClassPathDirEntry::open_stream(const char* name) {
  // construct full path name
  char path[JVM_MAXPATHLEN];
  if (jio_snprintf(path, sizeof(path), "%s%s%s", _dir, os::file_separator(), name) == -1) {
    return NULL;
  }
  // check if file exists
  struct stat st;
  if (os::stat(path, &st) == 0) {
    // found file, open it
    int file_handle = hpi::open(path, 0, 0);
    if (file_handle != -1) {
      // read contents into resource array
      u1* buffer = NEW_RESOURCE_ARRAY(u1, st.st_size);
      size_t num_read = os::read(file_handle, (char*) buffer, st.st_size);
      // close file
      hpi::close(file_handle);
      // construct ClassFileStream
      if (num_read == (size_t)st.st_size) {
        return new ClassFileStream(buffer, st.st_size, _dir);    // Resource allocated
      }
    }
  }
  return NULL;
}


ClassPathZipEntry::ClassPathZipEntry(jzfile* zip, char* zip_name) : ClassPathEntry() {
  _zip = zip;
  _zip_name = zip_name;
}


ClassFileStream* ClassPathZipEntry::open_stream(const char* name) {
  // enable call to C land
  JavaThread* thread = JavaThread::current();
  ThreadToNativeFromVM ttn(thread);
  HandleMark hm(thread);
  // check whether zip archive contains name
  jint filesize, name_len;
  jzentry* entry = (*FindEntry)(_zip, name, &filesize, &name_len);
  if (entry == NULL) return NULL;
  u1* buffer;
  char* filename = NEW_RESOURCE_ARRAY(char, name_len + 1);

  // file found, get pointer to class in mmaped jar file.
  if (ReadMappedEntry == NULL ||
      !(*ReadMappedEntry)(_zip, entry, &buffer, filename)) {
      // mmaped access not available, perhaps due to compression,
      // read contents into resource array
      buffer     = NEW_RESOURCE_ARRAY(u1, filesize);
      if (!(*ReadEntry)(_zip, entry, buffer, filename)) return NULL;
  }
  // return result
  return new ClassFileStream(buffer, filesize, _zip_name);    // Resource allocated
}


void ClassLoader::setup_bootstrap_search_path() {
  assert(_first_entry == NULL, "should not setup bootstrap class search path twice");
  char* sys_class_path = os::strdup(Arguments::get_sysclasspath());
  if (TraceClassLoading && Verbose) {  
    tty->print_cr("[Bootstrap loader class path=%s]", sys_class_path);
  }

  int len = (int)strlen(sys_class_path);
  int end = 0;

  // Iterate over class path entries
  for (int start = 0; start < len; start = end) {
    while (sys_class_path[end] && sys_class_path[end] != os::path_separator()[0]) {
      end++;
    }
    char* path = NEW_C_HEAP_ARRAY(char, end-start+1);
    strncpy(path, &sys_class_path[start], end-start);
    path[end-start] = '\0';
    update_class_path_entry_list(path);
    while (sys_class_path[end] == os::path_separator()[0]) {
      end++;
    }
  }
}

void ClassLoader::create_class_path_entry(char *path, struct stat st, ClassPathEntry **new_entry) {
  if ((st.st_mode & S_IFREG) == S_IFREG) {
    // Regular file, should be a zip file
    // Canonicalized filename
    char canonical_path[JVM_MAXPATHLEN];
    assert(!is_init_completed(), "sanity check. Just to make sure the exception code below will work");
    if (!get_canonical_path(path, canonical_path, JVM_MAXPATHLEN)) {
      // This matches the classic VM
      EXCEPTION_MARK;
      THROW_MSG(vmSymbols::java_io_IOException(), "Bad pathname");          
    }
    char* error_msg = NULL;
    jzfile* zip;
    {
      // enable call to C land
      JavaThread* thread = JavaThread::current();
      ThreadToNativeFromVM ttn(thread);
      HandleMark hm(thread);
      zip = (*ZipOpen)(canonical_path, &error_msg);
    }
    if (zip != NULL && error_msg == NULL) {
      *new_entry = new ClassPathZipEntry(zip, path);
      if (TraceClassLoading) {
        tty->print_cr("[Opened %s]", path);
      }
    }
  } else {
    // Directory
    *new_entry = new ClassPathDirEntry(path);
    if (TraceClassLoading) {
      tty->print_cr("[Path %s]", path);
    }
  }      
}

void ClassLoader::add_to_list(ClassPathEntry *new_entry) {
  if (new_entry != NULL) {
    if (_last_entry == NULL) {
      _first_entry = _last_entry = new_entry;
    } else {
      _last_entry->set_next(new_entry);
      _last_entry = new_entry;
    }
  }
}

void ClassLoader::update_class_path_entry_list(const char *path) {
  struct stat st;
  if (os::stat((char *)path, &st) == 0) {
    // File or directory found
    ClassPathEntry* new_entry = NULL;
    create_class_path_entry((char *)path, st, &new_entry);
    // Add new entry to linked list 
    add_to_list(new_entry);
  }
}

void ClassLoader::load_zip_library() {
  assert(ZipOpen == NULL, "should not load zip library twice");
  // First make sure native library is loaded
  os::native_java_library();
  // Load zip library
  char path[JVM_MAXPATHLEN];
  char ebuf[1024];
  hpi::dll_build_name(path, sizeof(path), Arguments::get_dll_dir(), "zip");
  void* handle = hpi::dll_load(path, ebuf, sizeof ebuf);
  if (handle == NULL) {
    vm_exit_during_initialization("Unable to load ZIP library", path);
  }
  // Lookup zip entry points
  ZipOpen      = CAST_TO_FN_PTR(ZipOpen_t, hpi::dll_lookup(handle, "ZIP_Open"));
  FindEntry    = CAST_TO_FN_PTR(FindEntry_t, hpi::dll_lookup(handle, "ZIP_FindEntry"));
  ReadEntry    = CAST_TO_FN_PTR(ReadEntry_t, hpi::dll_lookup(handle, "ZIP_ReadEntry"));
  ReadMappedEntry = CAST_TO_FN_PTR(ReadMappedEntry_t, hpi::dll_lookup(handle, "ZIP_ReadMappedEntry"));
#ifndef PRODUCT
  GetNextEntry = CAST_TO_FN_PTR(GetNextEntry_t, hpi::dll_lookup(handle, "ZIP_GetNextEntry"));
#endif
  if (ZipOpen == NULL || FindEntry == NULL || ReadEntry == NULL) {
    vm_exit_during_initialization("Corrupted ZIP library", path);
  }

  // Lookup canonicalize entry in libjava.dll  
  void *javalib_handle = os::native_java_library();
  CanonicalizeEntry = CAST_TO_FN_PTR(canonicalize_fn_t, hpi::dll_lookup(javalib_handle, "Canonicalize"));
  // This lookup only works on 1.3. Do not check for non-null here
}


// PackageInfo data exists in order to support the java.lang.Package
// class.  A Package object provides information about a java package
// (version, vendor, etc.) which originates in the manifest of the jar
// file supplying the package.  For application classes, the ClassLoader
// object takes care of this.

// For system (boot) classes, the Java code in the Package class needs
// to be able to identify which source jar file contained the boot
// class, so that it can extract the manifest from it.  This table
// identifies java packages with jar files in the boot classpath.

// Because the boot classpath cannot change, the classpath index is
// sufficient to identify the source jar file or directory.  (Since
// directories have no manifests, the directory name is not required,
// but is available.)

// When using sharing -- the pathnames of entries in the boot classpath
// may not be the same at runtime as they were when the archive was
// created (NFS, Samba, etc.).  The actual files and directories named
// in the classpath must be the same files, in the same order, even
// though the exact name is not the same.

class PackageInfo: public BasicHashtableEntry {
public:
  const char* _pkgname;       // Package name
  int _classpath_index;	      // Index of directory or JAR file loaded from

  PackageInfo* next() {
    return (PackageInfo*)BasicHashtableEntry::next();
  }

  const char* pkgname()           { return _pkgname; }
  void set_pkgname(char* pkgname) { _pkgname = pkgname; }

  const char* filename() {
    return ClassLoader::classpath_entry(_classpath_index)->name();
  }

  void set_index(int index) {
    _classpath_index = index;
  }
};


class PackageHashtable : public BasicHashtable {
private:
  inline unsigned int compute_hash(const char *s, int n) {
    unsigned int val = 0;
    while (--n >= 0) {
      val = *s++ + 31 * val;
    }
    return val;
  }

  PackageInfo* bucket(int index) {
    return (PackageInfo*)BasicHashtable::bucket(index);
  }

  PackageInfo* get_entry(int index, unsigned int hash,
                         const char* pkgname, size_t n) {
    for (PackageInfo* pp = bucket(index); pp != NULL; pp = pp->next()) {
      if (pp->hash() == hash &&
          strncmp(pkgname, pp->pkgname(), n) == 0 &&
          pp->pkgname()[n] == '\0') {
        return pp;
      }
    }
    return NULL;
  }

public:
  PackageHashtable(int table_size)
    : BasicHashtable(table_size, sizeof(PackageInfo)) {}

  PackageHashtable(int table_size, HashtableBucket* t, int number_of_entries)
    : BasicHashtable(table_size, sizeof(PackageInfo), t, number_of_entries) {}

  PackageInfo* get_entry(const char* pkgname, int n) {
    unsigned int hash = compute_hash(pkgname, n);
    return get_entry(hash_to_index(hash), hash, pkgname, n);
  }

  PackageInfo* new_entry(char* pkgname, int n) {
    unsigned int hash = compute_hash(pkgname, n);
    PackageInfo* pp;
    pp = (PackageInfo*)BasicHashtable::new_entry(hash);
    pp->set_pkgname(pkgname);
    return pp;
  }

  void add_entry(PackageInfo* pp) {
    int index = hash_to_index(pp->hash());
    BasicHashtable::add_entry(index, pp);
  }

  void copy_pkgnames(const char** packages) {
    int n = 0;
    for (int i = 0; i < table_size(); ++i) {
      for (PackageInfo* pp = bucket(i); pp != NULL; pp = pp->next()) {
        packages[n++] = pp->pkgname();
      }
    }
    assert(n == number_of_entries(), "just checking");
  }

  void copy_table(char** top, char* end, PackageHashtable* table);
};


void PackageHashtable::copy_table(char** top, char* end,
                                  PackageHashtable* table) {
  // Copy (relocate) the table to the shared space.
  BasicHashtable::copy_table(top, end);

  // Calculate the space needed for the package name strings.
  int i;
  int n = 0;
  for (i = 0; i < table_size(); ++i) {
    for (PackageInfo* pp = table->bucket(i);
                      pp != NULL;
                      pp = pp->next()) {
      n += (int)(strlen(pp->pkgname()) + 1);
    }
  }
  if (*top + n + sizeof(intptr_t) >= end) {
    warning("\nThe shared miscellaneous data space is not large "
            "enough to \npreload requested classes.  Use "
            "-XX:SharedMiscDataSize= to increase \nthe initial "
            "size of the miscellaneous data space.\n");
    exit(2);
  }

  // Copy the table data (the strings) to the shared space.
  n = align_size_up(n, sizeof(HeapWord));
  *(intptr_t*)(*top) = n;
  *top += sizeof(intptr_t);

  for (i = 0; i < table_size(); ++i) {
    for (PackageInfo* pp = table->bucket(i);
                      pp != NULL;
                      pp = pp->next()) {
      int n1 = (int)(strlen(pp->pkgname()) + 1);
      pp->set_pkgname((char*)memcpy(*top, pp->pkgname(), n1));
      *top += n1;
    }
  }
  *top = (char*)align_size_up((intptr_t)*top, sizeof(HeapWord));
}


void ClassLoader::copy_package_info_buckets(char** top, char* end) {
  _package_hash_table->copy_buckets(top, end);
}

void ClassLoader::copy_package_info_table(char** top, char* end) {
  _package_hash_table->copy_table(top, end, _package_hash_table);
}


PackageInfo* ClassLoader::lookup_package(const char *pkgname) {
  const char *cp = strrchr(pkgname, '/');
  if (cp != NULL) {
    // Package prefix found
    int n = cp - pkgname + 1;
    return _package_hash_table->get_entry(pkgname, n);
  }
  return NULL;
}


bool ClassLoader::add_package(const char *pkgname, int classpath_index) {
  assert(pkgname != NULL, "just checking");
  // First check for previously loaded entry
  PackageInfo* pp = lookup_package(pkgname);
  if (pp != NULL) {
    // Existing entry found, check source of package
    pp->set_index(classpath_index);
    return true;
  }

  const char *cp = strrchr(pkgname, '/');
  if (cp != NULL) {
    // Package prefix found
    int n = cp - pkgname + 1;

    char* new_pkgname = NEW_C_HEAP_ARRAY(char, n + 1);
    if (new_pkgname == NULL) {
      return false;
    }

    memcpy(new_pkgname, pkgname, n);
    new_pkgname[n] = '\0';
    pp = _package_hash_table->new_entry(new_pkgname, n);
    pp->set_index(classpath_index);
    
    // Insert into hash table

    _package_hash_table->add_entry(pp);
  }
  return true;
}


oop ClassLoader::get_system_package(const char* name, TRAPS) {
  PackageInfo* pp;
  {
    MutexLocker ml(PackageTable_lock, THREAD);
    pp = lookup_package(name);
  }
  if (pp == NULL) {
    return NULL;
  } else {
    Handle p = java_lang_String::create_from_str(pp->filename(), THREAD);
    return p();
  }
}


objArrayOop ClassLoader::get_system_packages(TRAPS) {
  ResourceMark rm(THREAD);
  int nof_entries;
  const char** packages;
  {
    MutexLocker ml(PackageTable_lock, THREAD);
    // Allocate resource char* array containing package names
    nof_entries = _package_hash_table->number_of_entries();
    if ((packages = NEW_RESOURCE_ARRAY(const char*, nof_entries)) == NULL) {
      return NULL;
    }
    _package_hash_table->copy_pkgnames(packages);
  }
  // Allocate objArray and fill with java.lang.String
  objArrayOop r = oopFactory::new_objArray(SystemDictionary::string_klass(),
                                           nof_entries, CHECK_0);
  objArrayHandle result(THREAD, r);
  for (int i = 0; i < nof_entries; i++) {
    Handle str = java_lang_String::create_from_str(packages[i], CHECK_0);
    result->obj_at_put(i, str());
  }

  return result();
}


instanceKlassHandle ClassLoader::load_classfile(symbolHandle h_name, TRAPS) {
  VTuneClassLoadMarker clm;
  ResourceMark rm(THREAD);
  EventMark m("loading class " INTPTR_FORMAT, h_name());
  ThreadProfilerMark tpm(ThreadProfilerMark::classLoaderRegion);

  stringStream st;
  // st.print() uses too much stack space while handling a StackOverflowError
  // st.print("%s.class", h_name->as_utf8());
  st.print_raw(h_name->as_utf8());
  st.print_raw(".class");
  char* name = st.as_string();

  // Lookup stream for parsing .class file
  ClassFileStream* stream = NULL;
  int classpath_index = 0;
  {
    PerfTraceTime vmtimer(perf_accumulated_time());
    ClassPathEntry* e = _first_entry;
    while (e != NULL) {
      stream = e->open_stream(name);
      if (stream != NULL) {
        break;
      }
      e = e->next();
      ++classpath_index;
    }
  }

  instanceKlassHandle h(THREAD, klassOop(NULL));
  if (stream != NULL) {

    // class file found, parse it
    ClassFileParser parser(stream);
    Handle class_loader;
    Handle protection_domain;
    symbolHandle parsed_name;
    instanceKlassHandle result = parser.parseClassFile(h_name, 
                                                       class_loader, 
                                                       protection_domain, 
                                                       parsed_name,
                                                       CHECK_(h));

    // add to package table
    if (add_package(name, classpath_index)) {
      h = result;
    }
  }

  return h;
}


void ClassLoader::create_package_info_table(HashtableBucket *t, int length,
                                            int number_of_entries) {
  assert(_package_hash_table == NULL, "One package info table allowed.");
  assert(length == package_hash_table_size * sizeof(HashtableBucket),
         "bad shared package info size.");
  _package_hash_table = new PackageHashtable(package_hash_table_size, t,
                                             number_of_entries);
}


void ClassLoader::create_package_info_table() {
    assert(_package_hash_table == NULL, "shouldn't have one yet");
    _package_hash_table = new PackageHashtable(package_hash_table_size);
}


// Initialize the class loader's access to methods in libzip.  Parse and
// process the boot classpath into a list ClassPathEntry objects.  Once
// this list has been created, it must not change (see class PackageInfo).

void ClassLoader::initialize() {
  assert(_package_hash_table == NULL, "should have been initialized by now.");
  EXCEPTION_MARK;

  if (UsePerfData) {
    // jvmstat performance counters
    _perf_accumulated_time =
                 PerfDataManager::create_counter(SUN_CLS, "time",
                                                 PerfData::U_Ticks, CHECK);

    _perf_classes_inited =
                 PerfDataManager::create_counter(SUN_CLS, "initializedClasses",
                                                 PerfData::U_Events, CHECK);

    _perf_class_init_time =
                 PerfDataManager::create_counter(SUN_CLS, "classInitTime",
                                                 PerfData::U_Ticks, CHECK);

    _perf_class_verify_time =
                 PerfDataManager::create_counter(SUN_CLS, "classVerifyTime",
                                                 PerfData::U_Ticks, CHECK);

  }

  // lookup zip library entry points
  load_zip_library();
  // initialize search path
  setup_bootstrap_search_path();
}


jlong ClassLoader::classloader_time_ms() {
  return UsePerfData ?
    Management::ticks_to_ms(_perf_accumulated_time->get_value()) : -1;
}

jlong ClassLoader::class_init_count() {
  return UsePerfData ? _perf_classes_inited->get_value() : -1;
}

jlong ClassLoader::class_init_time_ms() {
  return UsePerfData ? 
    Management::ticks_to_ms(_perf_class_init_time->get_value()) : -1;
}

jlong ClassLoader::class_verify_time_ms() {
  return UsePerfData ? 
    Management::ticks_to_ms(_perf_class_verify_time->get_value()) : -1;
}

int ClassLoader::compute_Object_vtable() {
  // hardwired for JDK1.2 -- would need to duplicate class file parsing
  // code to determine actual value from file
  // Would be value '11' if finals were in vtable
  int JDK_1_2_Object_vtable_size = 5;
  return JDK_1_2_Object_vtable_size * vtableEntry::size();
}


void classLoader_init() {
  ClassLoader::initialize();
}


bool ClassLoader::get_canonical_path(char* orig, char* out, int len) {
  assert(orig != NULL && out != NULL && len > 0, "bad arguments");        
  if (CanonicalizeEntry != NULL) {
    JNIEnv* env = JavaThread::current()->jni_environment();
    if ((CanonicalizeEntry)(env, hpi::native_path(orig), out, len) < 0) {    
      return false;  
    }    
  } else {
    // On JDK 1.2.2 the Canonicalize does not exist, so just do nothing
    strncpy(out, orig, len);
    out[len - 1] = '\0';    
  }
  return true;
}




#ifndef PRODUCT

void ClassLoader::verify() {
  _package_hash_table->verify();
}


// CompileTheWorld
//
// Iterates over all class path entries and forces compilation of all methods
// in all classes found. Currently, only zip/jar archives are searched.
// 
// The classes are loaded by the Java level bootstrap class loader, and the
// initializer is called. If DelayCompilationDuringStartup is true (default),
// the interpreter will run the initialization code. Note that forcing 
// initialization in this way could potentially lead to initialization order
// problems, in which case we could just force the initialization bit to be set.


// We need to iterate over the contents of a zip/jar file, so we replicate the
// jzcell and jzfile definitions from zip_util.h but rename jzfile to real_jzfile,
// since jzfile already has a void* definition.
//
// Note that this is only used in debug mode.
//
// HotSpot integration note:
// Matches zip_util.h 1.14 99/06/01 from jdk1.3 beta H build


// JDK 1.3 version
typedef struct real_jzentry13 { 	/* Zip file entry */
    char *name;	  	  	/* entry name */
    jint time;            	/* modification time */
    jint size;	  	  	/* size of uncompressed data */
    jint csize;  	  	/* size of compressed data (zero if uncompressed) */
    jint crc;		  	/* crc of uncompressed data */
    char *comment;	  	/* optional zip file comment */
    jbyte *extra;	  	/* optional extra data */
    jint pos;	  	  	/* position of LOC header (if negative) or data */
} real_jzentry13;

typedef struct real_jzfile13 {  /* Zip file */
    char *name;	  	        /* zip file name */
    jint refs;		        /* number of active references */
    jint fd;		        /* open file descriptor */
    void *lock;		        /* read lock */
    char *comment; 	        /* zip file comment */
    char *msg;		        /* zip error message */
    void *entries;          	/* array of hash cells */
    jint total;	  	        /* total number of entries */
    unsigned short *table;      /* Hash chain heads: indexes into entries */
    jint tablelen;	        /* number of hash eads */
    real_jzfile13 *next;        /* next zip file in search list */
    jzentry *cache;             /* we cache the most recently freed jzentry */
    /* Information on metadata names in META-INF directory */
    char **metanames;           /* array of meta names (may have null names) */
    jint metacount;	        /* number of slots in metanames array */
    /* If there are any per-entry comments, they are in the comments array */
    char **comments;
} real_jzfile13;

// JDK 1.2 version
typedef struct real_jzentry12 {  /* Zip file entry */
    char *name;                  /* entry name */
    jint time;                   /* modification time */
    jint size;                   /* size of uncompressed data */
    jint csize;                  /* size of compressed data (zero if uncompressed) */
    jint crc;                    /* crc of uncompressed data */
    char *comment;               /* optional zip file comment */
    jbyte *extra;                /* optional extra data */
    jint pos;                    /* position of LOC header (if negative) or data */
    struct real_jzentry12 *next; /* next entry in hash table */
} real_jzentry12;

typedef struct real_jzfile12 {  /* Zip file */
    char *name;                 /* zip file name */
    jint refs;                  /* number of active references */
    jint fd;                    /* open file descriptor */
    void *lock;                 /* read lock */
    char *comment;              /* zip file comment */
    char *msg;                  /* zip error message */
    real_jzentry12 *entries;    /* array of zip entries */
    jint total;                 /* total number of entries */
    real_jzentry12 **table;     /* hash table of entries */
    jint tablelen;              /* number of buckets */
    jzfile *next;               /* next zip file in search list */
} real_jzfile12;


void ClassPathDirEntry::compile_the_world(Handle loader, TRAPS) {
  // For now we only compile all methods in all classes in zip/jar files
  tty->print_cr("CompileTheWorld : Skipped classes in %s", _dir);
  tty->cr();
}


bool ClassPathDirEntry::is_rt_jar() {
  return false;
}

void ClassPathZipEntry::compile_the_world(Handle loader, TRAPS) {
  if (Universe::is_jdk12x_version()) {
    compile_the_world12(loader, THREAD);
  } else {
    compile_the_world13(loader, THREAD);
  }
  if (HAS_PENDING_EXCEPTION) {
    if (PENDING_EXCEPTION->is_a(SystemDictionary::OutOfMemoryError_klass())) {
      CLEAR_PENDING_EXCEPTION;
      tty->print_cr("\nCompileTheWorld : Ran out of memory\n");
      size_t used = Universe::heap()->permanent_used();
      size_t capacity = Universe::heap()->permanent_capacity();
      tty->print_cr("Permanent generation used %dK of %dK", used/K, capacity/K);
      tty->print_cr("Increase size by setting e.g. -XX:MaxPermSize=%dK\n", capacity*2/K);
    } else {
      tty->print_cr("\nCompileTheWorld : Unexpected exception occurred\n");
    }
  }
}

// Version that works for JDK 1.3.x
void ClassPathZipEntry::compile_the_world13(Handle loader, TRAPS) {
  real_jzfile13* zip = (real_jzfile13*) _zip;
  tty->print_cr("CompileTheWorld : Compiling all classes in %s", zip->name);
  tty->cr();
  // Iterate over all entries in zip file
  for (int n = 0; ; n++) {
    real_jzentry13 * ze = (real_jzentry13 *)((*GetNextEntry)(_zip, n));
    if (ze == NULL) break;
    ClassLoader::compile_the_world_in(ze->name, loader, CHECK);
  }
}


// Version that works for JDK 1.2.x
void ClassPathZipEntry::compile_the_world12(Handle loader, TRAPS) {
  real_jzfile12* zip = (real_jzfile12*) _zip;
  tty->print_cr("CompileTheWorld : Compiling all classes in %s", zip->name);
  tty->cr();
  // Iterate over all entries in zip file
  for (int n = 0; ; n++) {
    real_jzentry12 * ze = (real_jzentry12 *)((*GetNextEntry)(_zip, n));
    if (ze == NULL) break;
    ClassLoader::compile_the_world_in(ze->name, loader, CHECK);
  }
}

bool ClassPathZipEntry::is_rt_jar() {
  if (Universe::is_jdk12x_version()) {
    return is_rt_jar12();
  } else {
    return is_rt_jar13();
  }
}

// JDK 1.3 version
bool ClassPathZipEntry::is_rt_jar13() {
  real_jzfile13* zip = (real_jzfile13*) _zip;
  int len = (int)strlen(zip->name);
  // Check whether zip name ends in "rt.jar"
  // This will match other archives named rt.jar as well, but this is
  // only used for debugging.
  return (len >= 6) && (strcasecmp(zip->name + len - 6, "rt.jar") == 0);
}

// JDK 1.2 version
bool ClassPathZipEntry::is_rt_jar12() {
  real_jzfile12* zip = (real_jzfile12*) _zip;
  int len = (int)strlen(zip->name);
  // Check whether zip name ends in "rt.jar"
  // This will match other archives named rt.jar as well, but this is
  // only used for debugging.
  return (len >= 6) && (strcasecmp(zip->name + len - 6, "rt.jar") == 0);
}


void ClassLoader::compile_the_world() {
  EXCEPTION_MARK;
  HandleMark hm(THREAD);
  ResourceMark rm(THREAD);
  // Make sure we don't run with background compilation
  BackgroundCompilation = false;
  // Find bootstrap loader
  Handle system_class_loader (THREAD, SystemDictionary::java_system_loader());
  // Wait for compiler to be initialized
  NOT_CORE(while (!CompileBroker::compiler()->is_initialized()) os::yield();)
  // Iterate over all bootstrap class path entries
  ClassPathEntry* e = _first_entry;
  while (e != NULL) {
    // We stop at rt.jar, unless it is the first bootstrap path entry
    if (e->is_rt_jar() && e != _first_entry) break;
    e->compile_the_world(system_class_loader, CATCH);
    e = e->next();
  }
  tty->print_cr("CompileTheWorld : Done");
  {
    // Print statistics as if before normal exit:
    extern void print_statistics();
    print_statistics();
  }
  vm_exit(0);
}

int ClassLoader::_compile_the_world_counter = 0;

void ClassLoader::compile_the_world_in(char* name, Handle loader, TRAPS) {
#ifndef CORE
  int len = (int)strlen(name);
  if (len > 6 && strcmp(".class", name + len - 6) == 0) {
    // We have a .class file
    char buffer[2048];
    strncpy(buffer, name, len - 6);
    buffer[len-6] = 0;
    // If the file has a period after removing .class, it's not really a
    // valid class file.  The class loader will check everything else.
    if (strchr(buffer, '.') == NULL) {
      _compile_the_world_counter++;
      if (_compile_the_world_counter >= CompileTheWorldStartAt && _compile_the_world_counter <= CompileTheWorldStopAt) {
        // Construct name without extension
        symbolHandle sym = oopFactory::new_symbol_handle(buffer, CHECK);
        // Use loader to load and initialize class
        klassOop ik = SystemDictionary::resolve_or_null(sym, loader, Handle(), THREAD);
        instanceKlassHandle k (THREAD, ik);
        if (k.not_null() && !HAS_PENDING_EXCEPTION) {
          k->initialize(THREAD);
        }
        bool exception_occurred = HAS_PENDING_EXCEPTION;
        CLEAR_PENDING_EXCEPTION;
        if (k.is_null() || (exception_occurred && !CompileTheWorldIgnoreInitErrors)) {
          // If something went wrong (e.g. ExceptionInInitializerError) we skip this class
          tty->print_cr("CompileTheWorld (%d) : Skipping %s", _compile_the_world_counter, buffer);
        } else {
          tty->print_cr("CompileTheWorld (%d) : %s", _compile_the_world_counter, buffer);
          // Preload all classes to get around uncommon traps
          if (CompileTheWorldPreloadClasses) {
            constantPoolKlass::preload_and_initialize_all_classes(k->constants(), THREAD);
            if (HAS_PENDING_EXCEPTION) {
              // If something went wrong in preloading we just ignore it
              CLEAR_PENDING_EXCEPTION;
              tty->print_cr("Preloading failed for (%d) %s", _compile_the_world_counter, buffer);
            }
          }
          // Iterate over all methods in class
          for (int n = 0; n < k->methods()->length(); n++) {
            methodHandle m (THREAD, methodOop(k->methods()->obj_at(n)));
            if (CompilationPolicy::canBeCompiled(m)) {
              // Force compilation           
              CompileBroker::compile_method(m, InvocationEntryBci,
                                            methodHandle(), 0, "CTW", THREAD);
              if (HAS_PENDING_EXCEPTION) {
                CLEAR_PENDING_EXCEPTION;
                tty->print_cr("CompileTheWorld (%d) : Skipping method: %s", _compile_the_world_counter, m->name()->as_C_string());
              }
  	    if (TieredCompilation) {
  	      // Clobber the first compile and force second tier compilation
  	      m->set_code(NULL);
  	      CompileBroker::compile_method(m, InvocationEntryBci,
                                            methodHandle(), 0, "CTW", THREAD);
  	      if (HAS_PENDING_EXCEPTION) {
  		CLEAR_PENDING_EXCEPTION;
  		tty->print_cr("CompileTheWorld (%d) : Skipping method: %s", _compile_the_world_counter, m->name()->as_C_string());
  	      }
  	    }
            }
          }
        }
      }
    }
  }
#endif //CORE
}

#endif //PRODUCT
