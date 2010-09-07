/*
 * Copyright (c) 2005, 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

#include "incls/_precompiled.incl"
#include "incls/_classFileError.cpp.incl"

// Keep these in a separate file to prevent inlining

void ClassFileParser::classfile_parse_error(const char* msg, TRAPS) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbolHandles::java_lang_ClassFormatError(),
                       msg, _class_name->as_C_string());
}

void ClassFileParser::classfile_parse_error(const char* msg, int index, TRAPS) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbolHandles::java_lang_ClassFormatError(),
                       msg, index, _class_name->as_C_string());
}

void ClassFileParser::classfile_parse_error(const char* msg, const char *name, TRAPS) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbolHandles::java_lang_ClassFormatError(),
                       msg, name, _class_name->as_C_string());
}

void ClassFileParser::classfile_parse_error(const char* msg, int index, const char *name, TRAPS) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbolHandles::java_lang_ClassFormatError(),
                       msg, index, name, _class_name->as_C_string());
}

void StackMapStream::stackmap_format_error(const char* msg, TRAPS) {
  ResourceMark rm(THREAD);
  Exceptions::fthrow(
    THREAD_AND_LOCATION,
    vmSymbolHandles::java_lang_ClassFormatError(),
    "StackMapTable format error: %s", msg
  );
}
