/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <map>

#include "messages.hpp"
#include "os_utils.hpp"

using namespace std;



struct MsgEntry {
    int id;
    const char *key;
    const char *message;
};

/*
 * A list of messages hardcoded to JQS.
 */
static MsgEntry messages[] = {
    {   0, // no zero id
        "error.internal.badmsg",
        "internal error, unknown message"},

    {   MSG_JQSUsage,
        "message.jqs.usage",
        "Usage: jqs <mode> [<options>] \n"
        "\n"
        "The following modes are supported: \n"
        "  -help         print this help message and exit \n"
        "  -register     install JQS service and register browser startup detectors \n"
        "  -unregister   uninstall JQS service and unregister startup detectors \n"
        "  -enable       enable JQS service \n"
        "  -disable      disable JQS service \n"
        "  -pause        pause prefetching \n"
        "  -resume       resume prefetching \n"
        "  -version      print version of the associated Java Runtime \n"
        "\n"
        "Options include: \n"
        "  -config <config>      set JQS configuration file \n"
        "  -profile <profile>    set JQS profile file \n"
        "  -logfile <logfile>    set JQS log file \n"
        "  -verbose <level>      verbose operation \n"
        "\n"},

    {   MSG_JQSRegistered,
        "message.jqs.registered",
        "JQS service successfully installed."},

    {   MSG_JQSRegisterFailed,
        "error.jqs.register",
        "Unable to install JQS service."},

    {   MSG_JQSUnregistered,
        "message.jqs.unregistered",
        "JQS service successfully removed."},

    {   MSG_JQSUnregisterFailed,
        "error.jqs.unregister",
        "Unable to remove JQS service."},

    {   MSG_JQSEnabled,
        "message.jqs.enabled",
        "JQS service enabled successfully."},
    
    {   MSG_JQSEnableFailed,
        "error.jqs.enable", 
        "Unable to enable JQS service."},

    {   MSG_JQSDisabled,
        "message.jqs.disabled",
        "JQS service disabled successfully."},

    {   MSG_JQSDisableFailed,
        "error.jqs.disable", 
        "Unable to disable JQS service."},

    {   MSG_JQSPaused,
        "message.jqs.paused",
        "JQS service paused successfully."},

    {   MSG_JQSPauseFailed,
        "error.jqs.pause",
        "Unable to pause JQS service."},

    {   MSG_JQSResumed,
        "message.jqs.resumed",
        "JQS service resumed successfully."},

    {   MSG_JQSResumeFailed,
        "error.jqs.resume",
        "Unable to resume JQS service."},

    {   MSG_JQSRequiresAdminPrivileges,
        "error.no.admin.privileges",
        "JQS requires Administrator privileges."},
};


//////////////////////////////////////////////////////////////////////////

/*
 * Wrapper for a property file. Responsible for parsing and storing
 * the contents of the file.
 * The implementation of the parser is a refactored version of property 
 * parser taken from JavaWS sources.
 */
class PropertyFile {
    typedef map<string, string> Properties;
    Properties properties;

    const char* parseNextOption(const string& fileName, const char* p);

public:

    /*
     * Parses given property file, return true on success.
     */
    bool parse (const string& fileName);

    /*
     * Returns property value by given key or NULL if the value is not found.
     */
    const char* getPropertyValue(const char* key) const;

};

/*
 * The proeprty file object instance.
 */
static PropertyFile* propertyFile = NULL;



/*
 * Parses given property file, return true on success.
 */
bool PropertyFile::parse (const string& fileName) {

    // find size of file and check existence
    struct stat statBuf;
    if (stat(fileName.c_str(), &statBuf) != 0) {
        return false;
    }

    FILE* file = fopen(fileName.c_str (), "rt");
    if (!file) {
        return false;
    }
    size_t fileSize = statBuf.st_size;

    vector<char> buf(fileSize+1);
    size_t curPos = 0;
    while (curPos < fileSize) {
        size_t n = fread (&buf[curPos], 1, buf.size() - curPos, file);
        if (n) {
            curPos += n;

        } else if (ferror (file)) {
            jqs_error ("Failed to read file %s: %s\n", fileName.c_str (), strerror (errno));
            fclose (file);
            return false;

        } else {
            break;
        }
    }
    fclose(file);
    buf[curPos] = '\0';

    const char *s = &buf[0];

    do {
        s = parseNextOption(fileName, s);
    } while (s);

    return true;
}


/* 
 * This method iterates through a property file, parsing one
 * (option, value) pair at a time. It automatically skips
 * comments and blank lines. Returns the advanced
 * position in the input buffer, or NULL when end of file
 * is reached.
 *
 * The file is parsed pretty much according to the spec.
 * for the java.util.Properties output format. However,
 * multi-line values are supported differently.
 */
const char* PropertyFile::parseNextOption(const string& fileName, const char* p) {
    // Check if we are at the end
    if (*p == '\0') {
        return NULL;
    }

    const char* mark;

    // Skip whitespace, newlines, and comments
    do {
        mark = p;
        while(iswspace(*p) || *p == '\r' || *p == '\n') {
            p++;
        }
        if (*p == '#') {
            p++;
            while(*p && (*p != '\n' && *p != '\r')) {
                p++;
            }
        }
    } while (mark != p);    

    // Are at end of the buffer?
    if (*p == '\0') {
        return NULL;
    }

    // Parse key
    mark = p;
    
    // Find end of option
    while(*p && (!(iswspace(*p) || *p == ':' || *p == '=')) || 
                 ((mark != p) && (*(p-1) == '\\')))
    {
        p++;
    }

    string key(mark, p);
    
    // Skip until start of value
    while(iswspace(*p)) {
        p++;
    }
    if (*p && (*p == ':' || *p == '=')) {
        p++;
        while(iswspace(*p) && *p != '\n' && *p != '\r') {
            p++;
        }
    }
    // End of stream?
    if (*p == '\0') {
        jqs_info (4, "File %s: key \"%s\" does not have value\n", fileName.c_str(), key.c_str());
        return NULL;
    }

    // Find end of value and '\' is a joining of sentence 
    mark = p;
    while(*p && (*p != '\n'|| *(p-1) == '\\') 
             && (*p != '\r'|| *(p-1) == '\\'))
    {
        p++;
    }

    // Trim trailing whitespaces
    const char* end = p;
    while (end > mark && iswspace(end[-1])) {
        end--;
    }

    vector<unicodechar_t> valBuf;
    valBuf.reserve (end-mark+1);

    // Handle potential escape characters in value
    const char* str = mark;
    while (str < end) {
        unicodechar_t ch = *str;
        if (*str != '\\') {
            str++;
        } else {
            str++;
            switch(*str) {
                case 't':
                    ch = '\t';
                    str++;
                    break;

                case 'n':
                    ch = '\n';
                    str++;
                    break;

                case 'r':
                    ch = '\r';
                    str++;
                    break;

                case 'u':
                    {
                        str++;
                        for (int i = 0; i < 4; i++) {
                            if (!isxdigit(str[i])) {
                                jqs_info (4, "File %s: incorrect unicode escape (key \"%s\")\n", fileName.c_str(), key.c_str());
                                return NULL;
                            }
                        }
                        unsigned int hex;
                        sscanf(str, "%4x", &hex);
                        str += 4;
                        ch = (unicodechar_t)hex;
                    }
                    break;

                default:
                    ch = *str;
                    str++;
                    break;
            }
        }

        valBuf.push_back (ch);
    }
    valBuf.push_back ('\0');

    properties[key] = convUnicodeToString(&valBuf[0]);

    return p;
}

/*
 * Returns property value by given key or NULL if the value is not found.
 */
const char* PropertyFile::getPropertyValue(const char* key) const {
    Properties::const_iterator it = properties.find(key);
    if (it != properties.end()) {
        return it->second.c_str();
    } else {
        return NULL;
    }
}


//////////////////////////////////////////////////////////////////////////

/*
 * Initializes the message module, reads suitable message file acording 
 * to system locale.
 */
static void initializeMessages() {
    propertyFile = new PropertyFile();

    string localeString = getLocaleString();

    string messagesDir = getSatelliteFileName (MESSAGES_DIR_RELATIVE_LOCATION);

    // construct name of localized message property file, such as:
    //   messages_en_US.properties
    string msgFileName = messagesDir + 
                         FILE_SEPARATOR_CHAR +
                         "jqsmessages_" +
                         localeString +
                         ".properties";

    bool ok = propertyFile->parse(msgFileName);

    if (!ok) {
        size_t p = localeString.find_last_of ('_');
        if (p != string::npos) {
            localeString.erase (p);
        }
        msgFileName = messagesDir + 
                      FILE_SEPARATOR_CHAR +
                      "jqsmessages_" +
                      localeString +
                      ".properties";

        ok = propertyFile->parse(msgFileName);
    }

    if (!ok) {
        // No localized file for this local, try non-localized:
        //   messages.properties
        msgFileName = messagesDir + 
                      FILE_SEPARATOR_CHAR +
                      "jqsmessages.properties";

        ok = propertyFile->parse(msgFileName);
    }

    if (!ok) {
        // now we have classic - double fault - fatal error generating error msg
        // we have to fall back to default messages
        jqs_warn("Failed to load messages file.\n");
    }
}

/*
 * Returns message string by given message ID, taking into account system locale.
 */
const char* getMsgString(MessageID messageID) {
    static CriticalSection msgLock;
    static bool msgs_inProgress = false;
    static bool msgs_initialized = false;

    CriticalSection::Lock lock(msgLock);

    const char *key = messages[0].key;
    const char *defaultMessage = messages[0].message;

    size_t len = sizeof(messages)/sizeof(messages[0]);
    for (size_t i = 0; i < len; i++) {
        if (messages[i].id == messageID) {
            key = messages[i].key;
            defaultMessage = messages[i].message;
            break;
        }
    }

    if (msgs_inProgress) {
        // double fault - error generating error msg, return default
        return defaultMessage;
    }
    msgs_inProgress = true;

    if (!msgs_initialized) {
        initializeMessages();
        msgs_initialized = true;
    }

    const char* value = propertyFile->getPropertyValue(key);
    if (value == NULL) {
        value = defaultMessage;
    }

    msgs_inProgress = false;
    return value;
}
