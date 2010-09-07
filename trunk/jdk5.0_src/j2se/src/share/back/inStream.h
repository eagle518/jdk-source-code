/*
 * @(#)inStream.h	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_INSTREAM_H
#define JDWP_INSTREAM_H

#include "transport.h"
#include "FrameID.h"

struct bag;

typedef struct PacketInputStream {
    jbyte *current;
    jint left;
    jdwpError error;
    jdwpPacket packet;
    struct bag *refs;
} PacketInputStream;

void inStream_init(PacketInputStream *stream, jdwpPacket packet);

jint inStream_id(PacketInputStream *stream);
jbyte inStream_command(PacketInputStream *stream);

jboolean inStream_readBoolean(PacketInputStream *stream);
jbyte inStream_readByte(PacketInputStream *stream);
jbyte* inStream_readBytes(PacketInputStream *stream, 
                          int length, jbyte *buf);
jchar inStream_readChar(PacketInputStream *stream);
jshort inStream_readShort(PacketInputStream *stream);
jint inStream_readInt(PacketInputStream *stream);
jlong inStream_readLong(PacketInputStream *stream);
jfloat inStream_readFloat(PacketInputStream *stream);
jdouble inStream_readDouble(PacketInputStream *stream);
jlong inStream_readObjectID(PacketInputStream *stream);
FrameID inStream_readFrameID(PacketInputStream *stream);
jmethodID inStream_readMethodID(PacketInputStream *stream);
jfieldID inStream_readFieldID(PacketInputStream *stream);
jlocation inStream_readLocation(PacketInputStream *stream);

jobject inStream_readObjectRef(PacketInputStream *stream);
jclass inStream_readClassRef(PacketInputStream *stream);
jthread inStream_readThreadRef(PacketInputStream *stream);
jthreadGroup inStream_readThreadGroupRef(PacketInputStream *stream);
jobject inStream_readClassLoaderRef(PacketInputStream *stream);
jstring inStream_readStringRef(PacketInputStream *stream);
jarray inStream_readArrayRef(PacketInputStream *stream);

char *inStream_readString(PacketInputStream *stream);
jvalue inStream_readValue(struct PacketInputStream *in, jbyte *typeKeyPtr);

jdwpError inStream_skipBytes(PacketInputStream *stream, jint count);

jboolean inStream_endOfInput(PacketInputStream *stream);
jdwpError inStream_error(PacketInputStream *stream);

void inStream_destroy(PacketInputStream *stream);

#endif /* _INSTREAM_H */
