/**
 * @(#)CRTable.java	1.18 04/04/13
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.jvm;

import java.util.*;

import com.sun.tools.javac.tree.*;
import com.sun.tools.javac.util.*;
import com.sun.tools.javac.util.List;
import com.sun.tools.javac.tree.Tree.*;

/** This class contains the CharacterRangeTable for some method
 *  and the hashtable for mapping trees or lists of trees to their
 *  ending positions.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class CRTable
implements CRTFlags {

    private final boolean crtDebug = false;

    /** The list of CRTable entries.
     */
    private ListBuffer<CRTEntry> entries = new ListBuffer<CRTEntry>();

    /** The hashtable for source positions.
     */
    private Map<Object,SourceRange> positions = new HashMap<Object,SourceRange>();

    /** The hashtable for ending positions stored in the parser.
     */
    private Map<Tree,Integer> endPositions;

    /** The tree of the method this table is intended for.
     *  We should traverse this tree to get source ranges.
     */
    Tree.MethodDef methodTree;

    /** Constructor
     */
    public CRTable(Tree.MethodDef tree, Map<Tree,Integer> endPositions) {
	this.methodTree = tree;
	this.endPositions = endPositions;
    }

    /** Create a new CRTEntry and add it to the entries.
     *  @param tree     The tree or the list of trees for which
     *                  we are storing the code pointers.
     *  @param flags    The set of flags designating type of the entry.
     *  @param startPc  The starting code position.
     *  @param endPc    The ending code position.
     */
    public void put(Object tree, int flags, int startPc, int endPc) {
        entries.append(new CRTEntry(tree, flags, startPc, endPc));
    }

    /** Compute source positions and write CRT to the databuf.
     *  @param databuf  The buffer to write bytecodes to.
     */
    public int writeCRT(ByteBuffer databuf) {

	int crtEntries = 0;

	new SourceComputer().csp(methodTree);    // compute source positions for the method
	
	for (List<CRTEntry> l = entries.toList(); l.nonEmpty(); l = l.tail) {

	    CRTEntry entry = l.head;

	    if (entry.startPc == entry.endPc)        // elimninate entires that doesn't produce bytecode, for example empty blocks and statements
		continue;

	    SourceRange pos = positions.get(entry.tree);
	    assert pos != null : "CRT: tree source positions are undefined";
	    if ((pos.startPos == Position.NOPOS) || (pos.endPos == Position.NOPOS))
		continue;

	    if (crtDebug) {
		System.out.println("Tree: " + entry.tree + ", type:" + getTypes(entry.flags));
		System.out.println("Start: line = " + Position.line(pos.startPos) + ", col = " + Position.column(pos.startPos) + ", pc = " + entry.startPc);
		System.out.println("End:   line = " + Position.line(pos.endPos) + ", col = " + Position.column(pos.endPos) + ", pc = " + (entry.endPc - 1));
	    }

	    // write attribute
	    databuf.appendChar(entry.startPc);
	    databuf.appendChar(entry.endPc - 1);    // 'endPc - 1' because endPc actually points to start of the next command
	    databuf.appendInt(pos.startPos);
	    databuf.appendInt(pos.endPos);
	    databuf.appendChar(entry.flags);

	    crtEntries++;
	}
	
	return crtEntries;
    }

    /** Return the number of the entries.
     */
    public int length() {
	return entries.length();
    }

    /** Return string describing flags enabled.
     */
    private String getTypes(int flags) {
	String types = "";
	if ((flags & CRT_STATEMENT)       != 0) types += " CRT_STATEMENT";
	if ((flags & CRT_BLOCK)           !=  0) types += " CRT_BLOCK";
	if ((flags & CRT_ASSIGNMENT)      != 0) types += " CRT_ASSIGNMENT";
	if ((flags & CRT_FLOW_CONTROLLER) != 0) types += " CRT_FLOW_CONTROLLER";
	if ((flags & CRT_FLOW_TARGET)     != 0) types += " CRT_FLOW_TARGET";
	if ((flags & CRT_INVOKE)          != 0) types += " CRT_INVOKE";
	if ((flags & CRT_CREATE)          != 0) types += " CRT_CREATE";
	if ((flags & CRT_BRANCH_TRUE)     != 0) types += " CRT_BRANCH_TRUE";
	if ((flags & CRT_BRANCH_FALSE)    != 0) types += " CRT_BRANCH_FALSE";
	return types;
    }

/* ************************************************************************
 * Traversal methods
 *************************************************************************/

    /**
     *  This class contains methods to compute source positions for trees.
     *  Extends Tree.Visitor to traverse the abstract syntax tree.
     */
    class SourceComputer extends Tree.Visitor {

	/** The result of the tree traversal methods.
	 */
	SourceRange result;

	/** Visitor method: compute source positions for a single node.
         */
	public SourceRange csp(Tree tree) {
	    if (tree == null) return null;
	    tree.accept(this);
	    if (result != null) {
    	        positions.put(tree, result);
	    }
	    return result;
        }

        /** Visitor method: compute source positions for a list of nodes.
         */
        public SourceRange csp(List<Tree> trees) {
	    if ((trees == null) || !(trees.nonEmpty())) return null;
	    SourceRange list_sr = new SourceRange();
	    for (List<Tree> l = trees; l.nonEmpty(); l = l.tail) {
	        list_sr.mergeWith(csp(l.head));
	    }
	    positions.put(trees, list_sr);
	    return list_sr;
        }

        /**  Visitor method: compute source positions for
         *    a list of case blocks of switch statements.
         */
        public SourceRange cspCases(List<Case> trees) {
            if ((trees == null) || !(trees.nonEmpty())) return null;
    	    SourceRange list_sr = new SourceRange();
	    for (List<Case> l = trees; l.nonEmpty(); l = l.tail) {
	        list_sr.mergeWith(csp(l.head));
	    }
	    positions.put(trees, list_sr);
	    return list_sr;
        }

        /**  Visitor method: compute source positions for
         *   a list of catch clauses in try statements.
         */
        public SourceRange cspCatchers(List<Catch> trees) {
	    if ((trees == null) || !(trees.nonEmpty())) return null;
	    SourceRange list_sr = new SourceRange();
	    for (List<Catch> l = trees; l.nonEmpty(); l = l.tail) {
	        list_sr.mergeWith(csp(l.head));
	    }
	    positions.put(trees, list_sr);
	    return list_sr;
        }

        public void visitMethodDef(MethodDef tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.body));
	    result = sr;
        }

        public void visitVarDef(VarDef tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    csp(tree.vartype);
	    sr.mergeWith(csp(tree.init));
	    result = sr;
        }

        public void visitSkip(Skip tree) {
	    SourceRange sr = new SourceRange(startPos(tree), startPos(tree));    // endPos is the same as startPos for the empty statement
	    result = sr;
        }

        public void visitBlock(Block tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    csp(tree.stats);    // doesn't compare because block's ending position is defined
	    result = sr;
        }

        public void visitDoLoop(DoLoop tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.body));
	    sr.mergeWith(csp(tree.cond));
	    result = sr;
        }

        public void visitWhileLoop(WhileLoop tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.cond));
	    sr.mergeWith(csp(tree.body));
	    result = sr;
        }

        public void visitForLoop(ForLoop tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.init));
	    sr.mergeWith(csp(tree.cond));
	    sr.mergeWith(csp(tree.step));
	    sr.mergeWith(csp(tree.body));
	    result = sr;
        }

        public void visitForeachLoop(ForeachLoop tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.var));
	    sr.mergeWith(csp(tree.expr));
	    sr.mergeWith(csp(tree.body));
	    result = sr;
        }

        public void visitLabelled(Labelled tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.body));
	    result = sr;
        }

        public void visitSwitch(Switch tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.selector));
	    sr.mergeWith(cspCases(tree.cases));
	    result = sr;
        }

        public void visitCase(Case tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.pat));
	    sr.mergeWith(csp(tree.stats));
	    result = sr;
        }

        public void visitSynchronized(Synchronized tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.lock));
	    sr.mergeWith(csp(tree.body));
	    result = sr;
        }

        public void visitTry(Try tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.body));
	    sr.mergeWith(cspCatchers(tree.catchers));
	    sr.mergeWith(csp(tree.finalizer));
	    result = sr;
        }

        public void visitCatch(Catch tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.param));
	    sr.mergeWith(csp(tree.body));
	    result = sr;
        }

        public void visitConditional(Conditional tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.cond));
	    sr.mergeWith(csp(tree.truepart));
	    sr.mergeWith(csp(tree.falsepart));
	    result = sr;
        }

        public void visitIf(If tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.cond));
	    sr.mergeWith(csp(tree.thenpart));
	    sr.mergeWith(csp(tree.elsepart));
	    result = sr;
        }

        public void visitExec(Exec tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.expr));
	    result = sr;
        }

        public void visitBreak(Break tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    result = sr;
        }

        public void visitContinue(Continue tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    result = sr;
        }

        public void visitReturn(Return tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.expr));
	    result = sr;
        }

        public void visitThrow(Throw tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.expr));
	    result = sr;
        }

        public void visitAssert(Assert tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.cond));
	    sr.mergeWith(csp(tree.detail));
	    result = sr;
        }

        public void visitApply(Apply tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.meth));
	    sr.mergeWith(csp(tree.args));
	    result = sr;
        }

        public void visitNewClass(NewClass tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.encl));
	    sr.mergeWith(csp(tree.clazz));
	    sr.mergeWith(csp(tree.args));
	    sr.mergeWith(csp(tree.def));
	    result = sr;
        }

        public void visitNewArray(NewArray tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.elemtype));
	    sr.mergeWith(csp(tree.dims));
	    sr.mergeWith(csp(tree.elems));
	    result = sr;
        }

        public void visitParens(Parens tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.expr));
	    result = sr;
        }

        public void visitAssign(Assign tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.lhs));
	    sr.mergeWith(csp(tree.rhs));
	    result = sr;
        }

        public void visitAssignop(Assignop tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.lhs));
	    sr.mergeWith(csp(tree.rhs));
	    result = sr;
        }

        public void visitUnary(Unary tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.arg));
	    result = sr;
        }

        public void visitBinary(Binary tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.lhs));
	    sr.mergeWith(csp(tree.rhs));
	    result = sr;
        }

        public void visitTypeCast(TypeCast tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.clazz));
	    sr.mergeWith(csp(tree.expr));
	    result = sr;
        }

        public void visitTypeTest(TypeTest tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.expr));
	    sr.mergeWith(csp(tree.clazz));
	    result = sr;
        }

        public void visitIndexed(Indexed tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.indexed));
	    sr.mergeWith(csp(tree.index));
	    result = sr;
        }

        public void visitSelect(Select tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.selected));
	    result = sr;
        }

        public void visitIdent(Ident tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    result = sr;
        }

        public void visitLiteral(Literal tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    result = sr;
        }

        public void visitTypeIdent(TypeIdent tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    result = sr;
        }

        public void visitTypeArray(TypeArray tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.elemtype));
	    result = sr;
        }

        public void visitTypeApply(TypeApply tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.clazz));
	    sr.mergeWith(csp(tree.arguments));
	    result = sr;
        }

        public void visitTypeParameter(TypeParameter tree) {
	    SourceRange sr = new SourceRange(startPos(tree), endPos(tree));
	    sr.mergeWith(csp(tree.bounds));
	    result = sr;
        }

        public void visitTypeArgument(TypeArgument tree) {
	    result = null;
        }

        public void visitErroneous(Erroneous tree) {
	    result = null;
        }

        public void visitTree(Tree tree) {
	    assert false;
        }

        /** The start position of given tree.
         */
        public int startPos(Tree tree) {
	    if (tree == null) return Position.NOPOS;
	    return tree.pos;	
        }

        /** The end position of given tree, if it has
         *  defined endpos, NOPOS otherwise.
         */
        public int endPos(Tree tree) {
	    if (tree == null) return Position.NOPOS;
	    if (tree.tag == Tree.BLOCK)
	        return ((Block) tree).endpos;
	    Integer endpos = endPositions.get(tree);
	    if (endpos != null)
	        return endpos.intValue();
	    return Position.NOPOS;	
        }
    }

    /** This class contains a CharacterRangeTableEntry.
     */
    class CRTEntry {

        /** A tree or a list of trees to obtain source positions.
	 */
	Object tree;
	
	/** The flags described in the CharacterRangeTable spec.
	 */
	int flags;
	
	/** The starting code position of this entry.
	 */
	int startPc;

	/** The ending code position of this entry.
	 */
	int endPc;
	
	/** Constructor */
	CRTEntry(Object tree, int flags, int startPc, int endPc) {
	    this.tree = tree;
	    this.flags = flags;
	    this.startPc = startPc;
	    this.endPc = endPc;
	}
    }


    /** This class contains source positions
     *  for some tree or list of trees.
     */
    class SourceRange {

        /** The starting source position.
	 */
	int startPos;
	
	/** The ending source position.
	 */
	int endPos;
	
	/** Constructor */
	SourceRange() {
	    startPos = Position.NOPOS;
	    endPos = Position.NOPOS;
	}

	/** Constructor */
	SourceRange(int startPos, int endPos) {
	    this.startPos = startPos;
	    this.endPos = endPos;
	}
	
	/** Compare the starting and the ending positions
	 *  of the source range and combines them assigning
	 *  the widest range to this.
	 */
	SourceRange mergeWith(SourceRange sr) {
	    if (sr == null) return this;
	    if (startPos == Position.NOPOS)
		startPos = sr.startPos;
	    else if (sr.startPos != Position.NOPOS)
		startPos = (startPos < sr.startPos ? startPos : sr.startPos);
	    if (endPos == Position.NOPOS)
		endPos = sr.endPos;
	    else if (sr.endPos != Position.NOPOS)
		endPos = (endPos > sr.endPos ? endPos : sr.endPos);
	    return this;
	}
    }

}
