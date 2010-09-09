package com.sun.xml.internal.rngom.ast.builder;

import com.sun.xml.internal.rngom.ast.om.Location;
import com.sun.xml.internal.rngom.ast.om.ParsedPattern;
import com.sun.xml.internal.rngom.ast.om.ParsedElementAnnotation;
import com.sun.xml.internal.rngom.parse.Parseable;

/**
 * {@link Scope} for &lt;grammar> element of the &lt;include>d grammar.
 * <p>
 * This object builds &lt;define>s in the included grammar that
 * override the definitions in the original grammar.
 */
public interface IncludedGrammar<
    P extends ParsedPattern,
    E extends ParsedElementAnnotation,
    L extends Location,
    A extends Annotations<E,L,CL>,
    CL extends CommentList<L>> extends GrammarSection<P,E,L,A,CL>, Scope<P,E,L,A,CL> {

    /**
     *
     * @return
     *      technically, an included gramamr does not produce a pattern,
     *      but this allows {@link Parseable#parseInclude(String, SchemaBuilder, IncludedGrammar, String)}
     *      to return the result from {@link IncludedGrammar} nicely.
     *
     *      <p>
     *      The value returned from this method will be returned from the abovementioned method.
     */
    P endIncludedGrammar(L loc, A anno) throws BuildException;
}
