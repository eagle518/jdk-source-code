/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "CWriteBuffer.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

CWriteBuffer::CWriteBuffer(int size) {

    m_buff = (char *) malloc(size);
    m_buff_length = size;
    m_data_length = 0;
}

CWriteBuffer::~CWriteBuffer() {
    
    if (m_buff != NULL) free(m_buff);
}

void CWriteBuffer::putInt(int x) {
    checkBuffSize(4);
    m_buff[m_data_length++] = (x >> 24) & 0xFF;
    m_buff[m_data_length++] = (x >> 16) & 0xFF;
    m_buff[m_data_length++] = (x >> 8) & 0xFF;
    m_buff[m_data_length++] = x & 0xFF;
    
}

void CWriteBuffer::putShort(short x) {

    checkBuffSize(2);
    m_buff[m_data_length++] = (x >> 8) & 0xFF;
    m_buff[m_data_length++] = x & 0xFF;
}

void CWriteBuffer::putString(const char * buff) {

    putString(buff,strlen(buff));
}

void CWriteBuffer::putString(const char * buff, int length) {

    putShort(length);
    checkBuffSize(length);
    strncpy(m_buff+m_data_length,buff,length);
    m_data_length = m_data_length + length;
}

void CWriteBuffer::checkBuffSize(int size) {

    int new_data_length = m_data_length + size;
    
    if (new_data_length >= m_buff_length) {
        m_buff_length = m_buff_length + size + CHUNK;
        m_buff = (char *) realloc(m_buff,m_buff_length);
    }
}

CWriteBuffer::operator char*() {
    return m_buff;
}

int CWriteBuffer::send(int fd) const {

    int offset = 0;
    int rc;

    while (offset < m_data_length) {
        rc = write(fd, m_buff+offset, m_data_length-offset);
        if (rc < 0) {
            return 0;
        }
        offset += rc;
    }
    return 1;
}
