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
#include <map>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include "jqs.hpp"
#include "parse.hpp"
#include "utils.hpp"
#include "print.hpp"
#include "prefetch.hpp"
#include "os_utils.hpp"
#include "jqs_profile.hpp"

using namespace std;

/*
 * Token terminator strings, used in calls to next_token().
 */
#define TERM_WHITE_SPACE " \t\n"
#define TERM_EQUALS "="
#define TERM_QUOTE "\""

/*
 * String defining the characters that are not allowed to appear in
 * variable names.
 */
#define VAR_NAME_FORBIDDEN_CHARS  (DOLLAR_SIGN_S OPEN_BRACE_S CLOSE_BRACE_S TERM_QUOTE)

/*
 * The current line of the config file that's being parsed.
 */
static std::string line;
/*
 * The line number of the current line.
 */
static size_t lineno = 0;
/*
 * Current position in the current line.
 */
static size_t currentPos;


/*
 * Current configuration file section, either JQS commands section or 
 * JQS profile section.
 */
enum ConfigSection {
    CS_Unknown,
    CS_Commands,
    CS_Profile,
};
static ConfigSection curConfigSection = CS_Unknown;


/*
 * The map of configuration file variables to their values.
 */
typedef std::map<std::string, std::string> SymbolTable;
static SymbolTable SymTable;


/* Utility functions */

/*
 * string to memory size
 * function to convert a string representing an integer value
 * and an optional scaling factor and converts that into an
 * integer. For example, 1m is converted into 1048576. The
 * supported scaling factors are 1 bytes) (implied by the lack
 * of a scaling factor), K or k (kilobytes), M or m (megabytes),
 * and G or g (gigabytes). 
 */
bool strtomemsize(const string& str, size_t *pVal) {

    size_t value;
    if (sscanf(str.c_str(), "%lu", &value) != 1) {
        jqs_warn ("Syntax error at line %d: Invalid memory size: %s\n", lineno, str.c_str());
        return false;
    }

    size_t pos = 0;
    // skip over the digits to find the scale factor, if any
    while ((pos < str.length()) && isdigit(str[pos])) {
        pos++;
    }

    if (str.length() - pos > 1) {
        jqs_warn("Syntax error at line %d: invalid scale factor: %s (valid scale factors are G, M, K, or nothing)\n", 
            lineno, str.c_str() + pos);
        return false;
    }

    size_t scale;
    switch (str [pos]) {
        case 'G': case 'g':
            scale = G;
            break;

        case 'M': case 'm':
            scale = M;
            break;

        case 'K': case 'k':
            scale = K;
            break;

        default:
            scale = 1;
            break;
    }

    assert (pVal);
    *pVal = value * scale;
    return true;
}

/*
 * function to convert a string representing an integer value
 * into an unsigned integer. 
 */
bool strtouint(const string& str, unsigned int *pVal) {
    char* end = NULL;
    unsigned long value = strtoul(str.c_str(), &end, 10);
    if (((value == 0) && (errno != 0)) || (*end != 0)) {
        jqs_warn ("Syntax error at line %d: invalid integer value \"%s\"\n", lineno,str.c_str());
        return false;
    }
    if (value > UINT_MAX) {
        jqs_warn ("Syntax error at line %d: positive integer value out of range \"%s\"\n", lineno, str.c_str());
        return false;
    }
    assert (pVal);
    *pVal = (unsigned int) value;

    return true;
}

/*
 * function to convert a string representing an double value
 * into double. 
 */
bool strtodouble(const string& str, double *pVal) {
    char* end = NULL;
    double value = strtod(str.c_str(), &end);
    if (((value == 0.0) && (errno != 0)) || (*end != 0)) {
        jqs_warn ("Syntax error at line %d: invalid double value \"%s\"\n", lineno, str.c_str());
        return false;
    }
    assert (pVal);
    *pVal = value;

    return true;
}

/*
 * function to convert a string representing an double value
 * into double. 
 */
bool strtopercent(const string& str, double *pVal) {
    if (!strtodouble (str, pVal)) {
        return false;
    }
    if ((*pVal < 0.0) || (100.0 < *pVal)) {
        jqs_warn ("Syntax error at line %d: invalid percent value \"%s\"\n", lineno, str.c_str());
        return false;
    }
    return true;
}


/*
 * Function to resolve any variable names in a given string. If a
 * variable name is not defined in the internal symbol table, the
 * variable will be searched for in the process's environment. If
 * the variable name is not found in either location, a warning
 * message is issued and a null string substituted for the variable
 * name. Nested variable names are not allowed, though multiple
 * variable names in a single string are allowed.
 *
 * This function will always return a copy of the original string,
 * even if no variable substitution was performed.
 */
string resolve_variables(const string& str) {
    // check for bad input
    //
    if (str.empty ()) {
        return str;
    }

    string resolved;

    // walk through the source string
    //
    size_t curPos = 0;
    while (curPos < str.length ()) {
        size_t startPos = curPos;

        while ((curPos < str.length ()) && (str[curPos] != DOLLAR_SIGN_C)) {
            curPos++;
        }

        if (curPos < str.length () - 1) {
            // found a $ character. Check the next character for OPEN_BRACE
            //
            if (str[curPos + 1] == OPEN_BRACE_C) {
                // we have the start of a variable name. first step is to copy
                // all the leading characters to the destination string. replace
                // the $ character with a '\0' and copy the string preceding the
                // variable name.
                //
                resolved += str.substr (startPos, curPos - startPos);

                // skip over the DOLLAR_SIGN and OPEN_BRACE
                curPos += 2;

                // collect the variable name characters
                startPos = curPos;
                while ((curPos < str.length ()) && (str [curPos] != CLOSE_BRACE_C)) {
                    curPos++;
                }

                if (curPos == str.length ()) {
                    // no CLOSE_BRACE character; syntax error;
                    //
                    jqs_warn("Syntax error at line %d: %c expected\n", lineno, CLOSE_BRACE_C);
                    return "";
                }

                // extract symbol name
                //
                string symname = str.substr (startPos, curPos - startPos);

                // lookup and append the symbol's value to the destination string
                // note that null symbol names are accepted, but a warning will be
                // issued before substituting a null value.
                //
                SymbolTable::const_iterator symIt = SymTable.find (symname);

                if (symIt == SymTable.end ()) {
                    // undefined symbol - try the process environment
                    //
                    const char* value = getenv(symname.c_str ());
                    if (value) {
                        resolved += value;

                    } else {
                        jqs_warn("Undefined symbol '%s' at line %d - substituting \"\"\n", 
                            symname.c_str (), lineno);
                    }

                } else {
                    resolved += symIt->second;
                }

                // continue the search at the character following the CLOSE_BRACE
                //
                curPos++;

            } else {
                // a lone $ characters -- retain and continue
                curPos++;
                resolved += str.substr (startPos, curPos - startPos);
            }
        } else {
            // copy remainder to destination
            //
            resolved += str.substr (startPos);
        }
    }
    return resolved;
}

/*
 * function to skip over white space in the line at currentPos.
 */
void skip_white() {
    while ((currentPos < line.length ()) && isspace(line [currentPos])) {
        currentPos++;
    }
}

/*
 * function to return the next token based on the given set of
 * terminal characters. If the next token is a terminal itself,
 * the terminal will be returned. The function skips over any
 * leading white space.
 */
string next_token(const char* terminals) {

    // skip any leading white space
    //
    skip_white();
    jqs_info (6, "  next_token(): looking at %s\n", (line.c_str () + currentPos));

    string token;

    size_t terminalPos = line.find_first_of (terminals, currentPos);

    if (terminalPos == string::npos) {
        // return all the remaining characters.
        token = line.substr (currentPos);
        currentPos = line.length ();

    } else {
        // return all characters up to the discovered terminal character
        if (terminalPos == currentPos) {
            // return the terminal character
            token = line[currentPos];
            ++currentPos;

        } else {
            // return the token preceding the terminal character
            token = line.substr (currentPos, terminalPos - currentPos);
            currentPos = terminalPos;
        }
    }
    jqs_info (6, "  next_token(): returning =>%s<=\n", token.c_str ());

    return token;
}

/*
 * Function to parse a quoted string. This function only supports
 * simple quoting - a single matched pair of double quote (")
 * characters. It does support variable expansion within the string,
 * though. 
 */
string parse_quoted_string() {
    string str = next_token(TERM_WHITE_SPACE TERM_QUOTE);

    if (!str.empty ()) {
        if (str[0] == '"') {
            // note: only supports basic quoting of the value.
            // i.e. no escaped quotes within quotes.
            //
            str = next_token(TERM_QUOTE);

            if (line [currentPos++] != '"') {
                jqs_warn("Syntax error at line %d: '\"' expected\n", lineno);
                return "";
            }
        }
    }

    if (str.empty ()) {
        return str;
    }

    str = resolve_variables(str);

    jqs_info (6, "  resolved %s\n", str.c_str ());

    return str;
}

/*
 * this function is responsible for parsing a file name from a
 * directive.
 */
bool parse_filename(QSEntry* ent) {

    string filename = parse_quoted_string();

    if (filename.empty ()) {
        jqs_warn("Syntax error at line %d: file name expected\n", lineno);
        return false;
    }
    
    ent->setFileName (filename);
    return true;
}

/*
 * this function is responsible for parsing a SET directive.
 */
bool parse_set() {
    // this is a crude attempt at var=val parsing. we probably want
    // something a bit more robust going forward. this implementation
    // accepts any non-white space characters for the variable name.
    // we may want to limit what characters can be included in a variable
    // name. 
    //

    string name = next_token(TERM_WHITE_SPACE TERM_EQUALS);
    if (name.empty ()) {
        jqs_warn("Syntax error at line %d: variable name expected\n", lineno);
        return false;
    }

    if (name.find_first_of(VAR_NAME_FORBIDDEN_CHARS) != string::npos) {
        jqs_warn("Syntax error at line %d: variable name should not contain '%s' characters\n", lineno, VAR_NAME_FORBIDDEN_CHARS);
        return false;
    }

    string equal = next_token(TERM_WHITE_SPACE TERM_EQUALS);
    if (equal.empty () || equal[0] != '=') {
        jqs_warn("Syntax error at line %d: '=' expected\n", lineno);
        return false;
    }

    string value = parse_quoted_string();

    if (currentPos < line.length ()) {
        // skip any trailing white space
        //
        skip_white();

        // check for extraneous characters following the value
        //
        if (currentPos < line.length ()) {
            jqs_warn("Syntax error at line %d: extra characters after value\n", lineno);
            return false;
        }
    }

    // add new symbol or modify the existing symbol's value
    //
    SymTable [name] = value;

    return true;
}

/*
 * this function is responsible for parsing an OPTION directive.
 */
bool parse_option() {
    // this is a crude attempt at var=val parsing. we probably want
    // something a bit more robust going forward. this implementation
    // accepts any non-white space characters for the variable name.
    // we may want to limit what characters can be included in a variable
    // name. 
    //

    string name = next_token(TERM_WHITE_SPACE TERM_EQUALS);
    if (name.empty ()) {
        jqs_warn("Syntax error at line %d: option name expected\n", lineno);
        return false;
    }

    string equal = next_token(TERM_WHITE_SPACE TERM_EQUALS);
    if (equal.empty () || equal[0] != '=') {
        jqs_warn("Syntax error at line %d: '=' expected\n", lineno);
        return false;
    }

    string value = parse_quoted_string();

    if (currentPos < line.length ()) {
        // skip any trailing white space
        //
        skip_white();

        // check for extraneous characters following the value
        //
        if (currentPos < line.length ()) {
            jqs_warn("Syntax error at line %d: extra characters after value\n", lineno);
            return false;
        }
    }

    if (name == OPTION_INTERVAL) {
        return strtouint (value, &interval);
            
    } else if (name == OPTION_MEMORY_LIMIT) {
        return strtomemsize (value, &memoryLimit);

    } else if (name == OPTION_BOOT_DISK_TIME_THRESHOLD) {
        return strtopercent(value, &bootThresholds.diskTime);

    } else if (name == OPTION_BOOT_PROCESSOR_TIME_THRESHOLD) {
        return strtopercent(value, &bootThresholds.processorTime);

    } else if (name == OPTION_REFRESH_DISK_TIME_THRESHOLD) {
        return strtopercent(value, &refreshThresholds.diskTime);

    } else if (name == OPTION_REFRESH_PROCESSOR_TIME_THRESHOLD) {
        return strtopercent(value, &refreshThresholds.processorTime);

    } else {
        jqs_warn("Unknown option at line %d: %s\n", lineno, name.c_str ());
    }
    return true;
}

/*
 * this function is responsible for parsing an UNSET directive.
 */
bool parse_unset() {
    string name = next_token(TERM_WHITE_SPACE TERM_EQUALS);
    if (name.empty ()) {
        jqs_warn("Syntax error at line %d: variable name expected\n", lineno);
        return false;
    }

    if (currentPos < line.length ()) {
        // skip any trailing white space
        //
        skip_white();

        // check for extraneous characters following the value
        //
        if (currentPos < line.length ()) {
            jqs_warn("Syntax error at line %d: extra characters after value\n", lineno);
            return false;
        }
    }

    // delete the symbol, if it exists.
    //
    SymbolTable::iterator sym = SymTable.find (name);
    bool found = (sym != SymTable.end ());

    if (found) {
        SymTable.erase (sym);
    } else {
        jqs_warn("Undefined symbol at line %d: '%s'\n", lineno, name.c_str());
    }
    return found;
}

/*
 * helper function, used by parse_pagesize() and parse_bufsize()
 */
bool parse_memsize(const char* optionName, size_t* pVal) {
    skip_white();
    if (line[currentPos] != '=') {
        jqs_warn("Syntax error at line %d: '=' expected - option %s is ignored\n", lineno, optionName);
        return false;
    }
    currentPos++;

    string option_value = next_token(TERM_WHITE_SPACE);
    if (option_value.empty ()) {
        jqs_warn("Syntax error at line %d: value expected - option %s is ignored\n", lineno, optionName);
        return false;
    }
    return strtomemsize(option_value, pVal);
}

/*
 * this function is responsible for parsing a page size option.
 */
bool parse_pagesize(QSEntry* ent) {
    size_t size;
    if (!parse_memsize("pagesize", &size)) {
        return false;
    }
#ifndef JQS_FILTER
    size_t realPageSize = get_pagesize(size);
    if (realPageSize != size) {
        jqs_warn ("Config file line %d: page size " SIZET_FORMAT " is not supported, using " SIZET_FORMAT "\n", lineno, size, realPageSize);
    }
    ent->setPageSize (realPageSize);
#endif
    return true;
}

/*
 * this function is responsible for parsing a buffer size option.
 */
bool parse_bufsize(QSEntry* ent) {
    size_t size;
    if (!parse_memsize("bufsize", &size)) {
        return false;
    }
    ent->setBufferSize (size);
    return true;
}

/*
 * this function is responsible for parsing a LOAD directive.
 */
bool parse_load(QSEntry* ent) {

    if (!parse_filename(ent)) {
        return false;
    }

    // parse the command specific options
    //
    for (;;) {
        string option_name = next_token(TERM_WHITE_SPACE TERM_EQUALS);
        if (option_name.empty()) {
            break;
        }

        if (option_name == OPTION_LOCKED) {
            ent->setLocked (true);

        } else if (option_name == OPTION_PAGESIZE) {
            parse_pagesize (ent);

        } else {
            jqs_warn("Unknown option at line %d: %s - ignored\n", lineno, option_name.c_str());
        }
    }

    if (is_executable(ent->getFileName())) {
        jqs_warn("%s command at line %d is ignored: it should not be used for executables, use %s command instead\n", CMD_LOAD, lineno, CMD_LOADLIB);
        return false;
    }

    return true;
}

/*
 * this function is responsible for parsing a LOADLIB directive.
 */
bool parse_loadlib(QSEntry* ent) {

    if (!parse_filename(ent)) {
        return false;
    }

    // parse the command specific options
    //
    for (;;) {
        string option_name = next_token(TERM_WHITE_SPACE TERM_EQUALS);
        if (option_name.empty()) {
            break;
        }

        if (option_name == OPTION_LOCKED) {
            ent->setLocked (true);

        } else if (option_name == OPTION_PAGESIZE) {
            parse_pagesize (ent);

        } else if (OPTION_LIBPATH_SUPPORTED && (option_name == OPTION_LIBPATH)) {
            skip_white();
            if (line[currentPos++] != '=') {
                jqs_warn("Syntax error at line %d: '=' expected - %s option ignored\n", lineno, OPTION_LIBPATH);
                continue;
            }

            string path = parse_quoted_string();
            if (path.empty ()) {
                jqs_warn("Syntax error at line %d: value expected - %s option ignored\n", lineno, OPTION_LIBPATH);
                continue;
            }

            ent->setLibPath (path);

        } else {
            jqs_warn("Unknown option at line %d: %s - ignored\n", 
                lineno, option_name.c_str ());
        }
    }
    return true;
}

/*
 * Auxiliary function, parses libdepend option of the refreshlib directive.
 * All dependent libs found are added to given entry.
 */
void parse_libdepend (QSEntry* ent, const string& depend) {

    // march through the ';' delimited library paths
    //
    size_t curPos = 0;
    while (curPos < depend.length()) {
        // skip any leading white space
        //
        while ((curPos < depend.length()) && isspace(depend [curPos])) {
            curPos++;
        }

        // find the next module name; no embedded white space allowed!
        //
        size_t startPos = curPos;
        while ((curPos < depend.length()) && depend [curPos] != ';') {
            curPos++;
        }

        if ((startPos != curPos) && (startPos < depend.length())) {
            // allocate a new temporary quick starter entry for this dependent dll
            //
            QSEntry* mod = new QSEntry(QS_REFRESHLIB);
            mod->setFileName (depend.substr (startPos, curPos - startPos));
            ent->addDependentLib (mod);
        }
        // skip the ';' char
        //
        curPos++;
    }
}

/*
 * this function is responsible for parsing a REFRESHLIB directive.
 */
bool parse_refreshlib(QSEntry* ent) {

    if (!parse_filename(ent)) {
        return false;
    }

    // parse the command specific options
    //
    for (;;) {
        string option_name = next_token(TERM_WHITE_SPACE TERM_EQUALS);
        if (option_name.empty()) {
            break;
        }

        if (option_name == OPTION_PAGESIZE) {
            parse_pagesize (ent);

        } else if (OPTION_LIBPATH_SUPPORTED && (option_name == OPTION_LIBPATH)) {
            skip_white();
            if (line[currentPos++] != '=') {
                jqs_warn("Syntax error at line %d: '=' expected - %s option ignored\n", lineno, OPTION_LIBPATH);
                continue;
            }

            string path = parse_quoted_string();
            if (path.empty ()) {
                jqs_warn("Syntax error at line %d: value expected - %s option ignored\n", lineno, OPTION_LIBPATH);
                continue;
            }

            ent->setLibPath (path);

        } else if (OPTION_LIBDEPEND_SUPPORTED && (option_name == OPTION_LIBDEPEND)) {
            skip_white();
            if (line[currentPos++] != '=') {
                jqs_warn("Syntax error at line %d: '=' expected - %s option ignored\n", lineno, OPTION_LIBDEPEND);
                continue;
            }

            string path = parse_quoted_string();
            if (path.empty ()) {
                jqs_warn("Syntax error at line %d: value expected - %s option ignored\n", lineno, OPTION_LIBDEPEND);
                continue;
            }
            parse_libdepend (ent, path);

        } else {
            jqs_warn("Unknown option at line %d: %s - ignored\n", lineno, option_name.c_str());
        }
    }
    return true;
}

/*
 * this function is responsible for parsing a REFRESH directive.
 */
bool parse_refresh(QSEntry* ent) {

    if (!parse_filename(ent)) {
        return false;
    }

    // parse the command specific options
    //
    for (;;) {
        string option_name = next_token(TERM_WHITE_SPACE TERM_EQUALS);
        if (option_name.empty()) {
            break;
        }

        if (option_name == OPTION_MAPPED) {
            ent->setMapped (true);

        } else if (option_name == OPTION_PAGESIZE) {
            parse_pagesize (ent);

        } else if(option_name == OPTION_BUFSIZE) {
            parse_bufsize (ent);
        
        } else {
            jqs_warn("Unknown option at line %d: %s - ignored\n", lineno, option_name.c_str());
        }
    }


    if (ent->isMapped() && is_executable(ent->getFileName())) {
        jqs_warn("%s %s command at line %d is ignored: it should not be used for executables, use %s command instead\n", CMD_REFRESH, OPTION_MAPPED, lineno, CMD_REFRESHLIB);
        return false;
    }

    return true;
}


/*
 * this function is responsible for parsing a REFRESHDIR directive.
 */
bool parse_refreshdir(QSEntry* ent) {

    if (!parse_filename(ent)) {
        return false;
    }

    // parse the command specific options
    //
    for (;;) {
        string option_name = next_token(TERM_WHITE_SPACE TERM_EQUALS);
        if (option_name.empty()) {
            break;
        }

        if (option_name == OPTION_RECURSIVE) {
            ent->setRecursive (true);

        } else {
            jqs_warn("Unknown option at line %d: %s - ignored\n", lineno, option_name.c_str());
        }
    }
    return true;
}

/*
 * get quick starter entry
 *
 * this function is responsible for reading the next
 * quick starter entry from the configuration file.
 *
 */
QSEntry* parse_qsent() {
    string cmd = next_token(TERM_WHITE_SPACE);

    if (cmd.empty ()) {
        // empty line in config file
        return NULL;

    } else if (cmd[0] == COMMENT_C) {
        // commented line
        return NULL;
    }

    QSEntry* ent = NULL;
    bool parsed = false;

    if (cmd == CMD_LOAD) {
        ent = new QSEntry(QS_LOAD);
        parsed = parse_load(ent);

    } else if (cmd == CMD_REFRESH) {
        ent = new QSEntry(QS_REFRESH);
        parsed = parse_refresh(ent);

    } else if (cmd == CMD_SET) {
        parsed = parse_set();

    } else if (cmd == CMD_OPTION) {
        parsed = parse_option();

    } else if (cmd == CMD_UNSET) {
        parsed = parse_unset();

    } else if (cmd == CMD_LOADLIB) {
        ent = new QSEntry(QS_LOADLIB);
        parsed = parse_loadlib(ent);

    } else if (cmd == CMD_REFRESHLIB) {
        ent = new QSEntry(QS_REFRESHLIB);
        parsed = parse_refreshlib(ent);

    } else if (cmd == CMD_REFRESHDIR) {
        ent = new QSEntry(QS_REFRESHDIR);
        parsed = parse_refreshdir(ent);
    
    } else if (cmd == CMD_PROFILE) {
        curConfigSection = CS_Profile;
        parsed = true;

    } else {
        jqs_warn("Unrecognized command at line %d\n", lineno);
        return NULL;
    }

    if (!parsed) {
        fprintf(stderr,"==>  %s\n", line.c_str ());
        delete ent;
        ent = NULL;
    }

    return ent;
}

/*
 * Adds new symbol or modifies the existing symbol's value.
 */
void setConfigVariable(const std::string& name, const std::string& value)
{
    SymTable [name] = value;
}

void parse_profile_command (const std::string& configFileName, 
                            FILE* configFile, FILE* configFileFiltered);

/*
 * Parses given JQS configuration file and fills g_QSEntries with quick starter
 * entry objects.
 * If the doFilter flag is set, the function generates a filtered configuration 
 * file leaving entries only for those files which are present on current system.
 */
void parseConfigFile(const std::string& filename, bool doFilter) {

    FILE* configFile = fopen (filename.c_str (), "rt");
    if (!configFile) {
        jqs_error("Unable to open %s: %s\n", filename.c_str (), strerror(errno));
        jqs_exit(1);
    }

    FILE* configFileFiltered = NULL;
    if (doFilter) {
        string filteredConfigName = filename + FILTERED_FILE_NAME_SUFFIX;
        configFileFiltered = fopen (filteredConfigName.c_str (), "wt");
        if (!configFileFiltered) {
            jqs_error("could not open %s for writing: %s\n", filteredConfigName.c_str (), strerror(errno));
            jqs_exit(1);
        }
    }

    jqs_info (3, "Parsing config %s\n", filename.c_str ());
    curConfigSection = CS_Commands;

    // parse lines of the following form
    //    cmd ["]file["] [option[=value]]* 
    //    # comment
    //    \n
    //

    while (getline (configFile, line)) {
        lineno++;
        currentPos = 0;

        assert (curConfigSection == CS_Commands);
        QSEntry* ent = parse_qsent();
        if (ent != NULL) {
            if (configFileFiltered) {
                struct stat statbuf;
                if (stat(ent->getFileName(), &statbuf) == 0) {
                    // file exists
                    fprintf (configFileFiltered, "%s\n", line.c_str ());
                    g_QSEntries.push_back(ent);
                }
            } else {
                g_QSEntries.push_back(ent);
            }

        } else {
            if (configFileFiltered) {
                fprintf (configFileFiltered, "%s\n", line.c_str ());
            }
        }

        if (curConfigSection == CS_Profile) {
            if (!profileFileName.empty ()) {
                jqs_warn ("It is not allowed to use configuration file with the profile "
                          "information inside and a separate profile file at the same time. "
                          "The former profile information is ignored.\n");
            } else {
                parse_profile_command (filename, configFile, configFileFiltered); // errors are reported inside, if any
            }
            break;
        }
    }
    if (configFileFiltered) {
        fclose (configFileFiltered);
        configFileFiltered = NULL;
    }

    fclose (configFile);
    configFile = NULL;
    curConfigSection = CS_Unknown;
}


//////////////////////////////////////////////////////////////////////////

/*
 * The JQS profile content handler implementation, attaches the profile 
 * information to proper entries from g_QSEntries list.
 * It is also able to write filtered profile using the JQSProfileWriter
 * specified in the constructor.
 * The instance of the object is intended to be passed to ReadJQSProfile
 * function.
 */
class JQSProfileParser : public JQSProfileContentHandler {
    // short file name -> QSEntries
    typedef map<string, QSEntries> QSEntryMap;
    QSEntryMap qsEntryMap;

    // pointer to an element of qsEntryMap 
    const QSEntries* qsEntries;

    JQSProfileWriter* filteredWriter;
    string            curFile;

public:
    JQSProfileParser(JQSProfileWriter* filteredWriter);

    /*
     * JQSProfileContentHandler implementation
     */
    virtual void openFileSection(const std::string& file);
    virtual void setConfig(const std::string& config);
    virtual void addRegion (uint64_t start, uint64_t end);
    virtual void closeFileSection();

};

/*
 * Receives JQSProfileWriter object instance to be used to write
 * filtered profile with. If NULL is passes the filtered profile
 * is not created.
 * Initializes internal mapping of short file names to the list of
 * quick starter entries having the same short name.
 */
JQSProfileParser::JQSProfileParser (JQSProfileWriter* filteredWriter_) 
    : qsEntries (NULL)
    , filteredWriter(filteredWriter_)
{
    for (size_t i = 0; i < g_QSEntries.size(); i++) {
        QSEntry* entry = g_QSEntries[i];

        {
            string shortName = getBaseName (entry->getFileName());
            qsEntryMap [shortName].push_back(entry);
        }

        const QSEntries& dependentLibs = entry->getDependentLibs ();
        for (size_t i = 0; i < dependentLibs.size (); i++) {
            QSEntry* mod = dependentLibs[i];
            string shortName = getBaseName (mod->getFileName());
            qsEntryMap [shortName].push_back(mod);
        }
    }
}

/*
 * JQS profile content handler callback. Prepares to receive the profile
 * for files with given short name.
 */
void JQSProfileParser::openFileSection(const std::string& file) {
    assert (qsEntries == NULL);
    curFile = file;

    QSEntryMap::const_iterator it = qsEntryMap.find(file);
    if (it == qsEntryMap.end ()) {
        jqs_info (4, "Config file does not contain entry for %s, while profile file does\n", file.c_str());
        return;
    }
    qsEntries = &(it->second);
}

/*
 * JQS profile content handler callback. 
 * Creates new profile entry in the filtered file, if necessary.
 */
void JQSProfileParser::setConfig(const std::string& config) {
    if (filteredWriter && qsEntries) {
        filteredWriter->openFileSection (curFile.c_str (), config.c_str ());
    }
}

/*
 * JQS profile content handler callback. 
 * Attaches given region information to proper quick starter entries. 
 * Writes the region to filtered file, if necessary.
 */
void JQSProfileParser::addRegion (uint64_t start, uint64_t end) {
    if (filteredWriter && qsEntries) {
        filteredWriter->writeRegion(start, end);
    }
    if (qsEntries) {
        for(size_t i = 0; i < qsEntries->size (); i++) {
            (*qsEntries) [i]->addRegion (start, end);
        }
    }
}

/*
 * JQS profile content handler callback. 
 */
void JQSProfileParser::closeFileSection() {
    if (filteredWriter && qsEntries) {
        filteredWriter->closeFileSection ();
    }
    qsEntries = NULL;
}

/*
 * This function is used to parse a part of the configuration file containing 
 * the profile information. If a configFileFiltered is specified the function 
 * emits the filtered contents to the file.
 */
void parse_profile_command (const std::string& configFileName,
                            FILE* configFile, 
                            FILE* configFileFiltered) 
{
    JQSProfileWriter* filteredProfileWriter = NULL;
    if (configFileFiltered) {
        filteredProfileWriter = new JQSProfileWriter(configFileFiltered, NULL);
    }
    JQSProfileParser parser(filteredProfileWriter);
    ReadJQSProfile(configFileName.c_str (), &parser, configFile, lineno);
    delete filteredProfileWriter;
}

/*
 * Parses given JQS profile file and attaches profile information to proper 
 * quick starter entry object added to g_QSEntries.
 * If the doFilter flag is set, the function generates a filtered profile file 
 * leaving entries only for those files which are present in both JQS 
 * configuration file and on current system.
 */
void parseProfile(const std::string& profileFileName, bool doFilter) {
    struct stat statbuf;
    if (stat(profileFileName.c_str(), &statbuf) < 0) {
        jqs_warn("Profile \"%s\" is ignored: %s\n", profileFileName.c_str(), strerror(errno));
        return;
    }

    jqs_info (3, "Parsing profile %s\n", profileFileName.c_str ());

    if (doFilter) {
        string profileFileNameFiltered = profileFileName + FILTERED_FILE_NAME_SUFFIX;

        FILE* outFiltered = fopen(profileFileNameFiltered.c_str(), "w");
        if (!outFiltered) {
            jqs_error ("Unable to write %s: %s\n", profileFileNameFiltered.c_str(), strerror(errno));
            jqs_exit (1);
        }
        JQSProfileWriter filteredProfileWriter(outFiltered, NULL);
        JQSProfileParser parser(&filteredProfileWriter);
        ReadJQSProfile(profileFileName.c_str(), &parser);
        fclose (outFiltered);

    } else {
        JQSProfileParser parser(NULL);
        ReadJQSProfile(profileFileName.c_str(), &parser);
    }

}
