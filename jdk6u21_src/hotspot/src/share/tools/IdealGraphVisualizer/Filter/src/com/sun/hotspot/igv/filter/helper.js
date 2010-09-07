/*
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
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

 /**
 *
 * @author Thomas Wuerthinger
 */

function colorize(property, regexp, color) {
    var f = new ColorFilter("");
    f.addRule(new ColorFilter.ColorRule(new MatcherSelector(new Properties.RegexpPropertyMatcher(property, regexp)), color));
    f.apply(graph);
}

function remove(property, regexp) {
    var f = new RemoveFilter("");
    f.addRule(new RemoveFilter.RemoveRule(new MatcherSelector(new Properties.RegexpPropertyMatcher(property, regexp)), false, false));
    f.apply(graph);
}

function split(property, regexp) {
    var f = new SplitFilter("", new MatcherSelector(new Properties.RegexpPropertyMatcher(property, regexp)));
    f.apply(graph);
}

function removeInputs(property, regexp, from, to) {
    var f = new RemoveInputsFilter("");
    if(from == undefined && to == undefined) {
        f.addRule(new RemoveInputsFilter.RemoveInputsRule(new MatcherSelector(new Properties.RegexpPropertyMatcher(property, regexp))));
    } else if(to == undefined) {
        f.addRule(new RemoveInputsFilter.RemoveInputsRule(new MatcherSelector(new Properties.RegexpPropertyMatcher(property, regexp)), from));
    } else {
        f.addRule(new RemoveInputsFilter.RemoveInputsRule(new MatcherSelector(new Properties.RegexpPropertyMatcher(property, regexp)), from, to));
    }
    f.apply(graph);
}

var black = Color.black;
var blue = Color.blue;
var cyan = Color.cyan;
var darkGray = Color.darkGray;
var gray = Color.gray;
var green = Color.green;
var lightGray = Color.lightGray;
var magenta = Color.magenta;
var orange = Color.orange;
var pink = Color.pink
var red = Color.red;
var yellow = Color.yellow;
var white = Color.white;
