/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

const int BytesPerInstWord = 16;

// Kludge to fix bug in gcc for ia64 in converting float to double for denorms
extern double ia64_double_zero;

const int StackAlignmentInBytes = 16;
