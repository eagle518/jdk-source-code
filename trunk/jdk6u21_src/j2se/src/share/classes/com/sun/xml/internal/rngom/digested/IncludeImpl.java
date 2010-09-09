package com.sun.xml.internal.rngom.digested;

import com.sun.xml.internal.rngom.ast.builder.Annotations;
import com.sun.xml.internal.rngom.ast.builder.BuildException;
import com.sun.xml.internal.rngom.ast.builder.Include;
import com.sun.xml.internal.rngom.ast.builder.IncludedGrammar;
import com.sun.xml.internal.rngom.ast.builder.Scope;
import com.sun.xml.internal.rngom.ast.om.Location;
import com.sun.xml.internal.rngom.ast.om.ParsedPattern;
import com.sun.xml.internal.rngom.parse.IllegalSchemaException;
import com.sun.xml.internal.rngom.parse.Parseable;

import java.util.HashSet;
import java.util.Set;

/**
 * @author Kohsuke Kawaguchi (kk@kohsuke.org)
 */
final class IncludeImpl extends GrammarBuilderImpl implements Include {

    private Set overridenPatterns = new HashSet();
    private boolean startOverriden = false;

    public IncludeImpl(DGrammarPattern p, Scope parent, DSchemaBuilderImpl sb) {
        super(p, parent, sb);
    }

    public void define(String name, Combine combine, ParsedPattern pattern, Location loc, Annotations anno) throws BuildException {
        super.define(name, combine, pattern, loc, anno);
        if(name==START)
            startOverriden = true;
        else
            overridenPatterns.add(name);
    }

    public void endInclude(Parseable current, String uri, String ns, Location loc, Annotations anno) throws BuildException, IllegalSchemaException {
        current.parseInclude(uri,sb,new IncludedGrammarImpl(grammar,parent,sb),ns);
    }

    private class IncludedGrammarImpl extends GrammarBuilderImpl implements IncludedGrammar {
        public IncludedGrammarImpl(DGrammarPattern p, Scope parent, DSchemaBuilderImpl sb) {
            super(p, parent, sb);
        }

        public void define(String name, Combine combine, ParsedPattern pattern, Location loc, Annotations anno) throws BuildException {
            // check for overridden pattern
            if(name==START) {
                if(startOverriden)
                    return;
            } else {
                if(overridenPatterns.contains(name))
                    return;
            }
            // otherwise define
            super.define(name, combine, pattern, loc, anno);
        }

        public ParsedPattern endIncludedGrammar(Location loc, Annotations anno) throws BuildException {
            return null;
        }
    }
}
