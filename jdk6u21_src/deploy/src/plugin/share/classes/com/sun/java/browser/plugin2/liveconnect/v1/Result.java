/*
 * @(#)Result.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.browser.plugin2.liveconnect.v1;

/** Represents a result returned to the JavaScript engine from method
    invocations and other operations. <P>

    This class exists solely to discriminate how primitive values are
    returned to the JavaScript engine. Under certain conditions it is
    desirable or required to return the boxing object for a primitive
    value to JavaScript rather than the primitive value itself. For
    example, a Java method might return {@code java.lang.Double}
    rather than {@code double}, and the programmer might expect to
    operate upon the {@code Double} object from JavaScript.
    Additionally, if the programmer explicitly creates a new instance
    of a boxing object using for example the syntax {@code new
    java.lang.Integer()}, most JavaScript engines will raise an error
    if the returned value is a JavaScript numeric value rather than an
    object value. <P>
*/

public final class Result {
    private Object value;
    private boolean skipUnboxing;

    /** Creates a new Result object wrapping the given value.

        @param value the value to wrap
        @param skipUnboxing whether boxing objects should be preserved rather than unboxed
    */
    public Result(Object value, boolean skipUnboxing) {
        this.value = value;
        this.skipUnboxing = skipUnboxing;
    }


    /** Returns the value this Result object wraps.
        
        @return the value this Result object wraps
    */
    public Object value() {
        return value;
    }

    /** If this Result wraps a primitive value's boxing object,
        indicates whether the automatic unboxing step should be
        skipped when sending that value to the JavaScript engine.

        @return whether the automatic unboxing step for primitive
        boxing objects should be skipped
    */
    public boolean skipUnboxing() {
        return skipUnboxing;
    }
}
