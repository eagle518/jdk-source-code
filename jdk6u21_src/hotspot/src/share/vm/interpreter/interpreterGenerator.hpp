/*
 * Copyright (c) 1997, 2007, Oracle and/or its affiliates. All rights reserved.
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

// This file contains the platform-independent parts
// of the interpreter generator.


class InterpreterGenerator: public CC_INTERP_ONLY(CppInterpreterGenerator)
                                   NOT_CC_INTERP(TemplateInterpreterGenerator) {

public:

InterpreterGenerator(StubQueue* _code);

#include "incls/_interpreterGenerator_pd.hpp.incl"

};
