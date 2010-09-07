/*
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
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

class ArgIterator {
    String[] args;
    int i;
    ArgIterator(String[] args) {
        this.args = args;
        this.i = 0;
    }
    String get() { return args[i]; }
    boolean hasMore() { return args != null && i  < args.length; }
    boolean next() { return ++i < args.length; }
}

abstract class ArgHandler {
    public abstract void handle(ArgIterator it);

}

class ArgRule {
    String arg;
    ArgHandler handler;
    ArgRule(String arg, ArgHandler handler) {
        this.arg = arg;
        this.handler = handler;
    }

    boolean process(ArgIterator it) {
        if (match(it.get(), arg)) {
            handler.handle(it);
            return true;
        }
        return false;
    }
    boolean match(String rule_pattern, String arg) {
        return arg.equals(rule_pattern);
    }
}

class ArgsParser {
    ArgsParser(String[] args,
               ArgRule[] rules,
               ArgHandler defaulter) {
        ArgIterator ai = new ArgIterator(args);
        while (ai.hasMore()) {
            boolean processed = false;
            for (int i=0; i<rules.length; i++) {
                processed |= rules[i].process(ai);
                if (processed) {
                    break;
                }
            }
            if (!processed) {
                if (defaulter != null) {
                    defaulter.handle(ai);
                } else {
                    System.err.println("ERROR: unparsed \""+ai.get()+"\"");
                    ai.next();
                }
            }
        }
    }
}
