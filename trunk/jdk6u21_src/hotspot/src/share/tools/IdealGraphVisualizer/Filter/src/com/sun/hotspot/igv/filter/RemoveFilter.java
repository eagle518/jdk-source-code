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
package com.sun.hotspot.igv.filter;

import com.sun.hotspot.igv.graph.Diagram;
import com.sun.hotspot.igv.graph.Figure;
import com.sun.hotspot.igv.graph.InputSlot;
import com.sun.hotspot.igv.graph.Selector;
import com.sun.hotspot.igv.data.Properties;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 *
 * @author Thomas Wuerthinger
 */
public class RemoveFilter extends AbstractFilter {

    private List<RemoveRule> rules;
    private String name;

    public RemoveFilter(String name) {
        this.name = name;
        rules = new ArrayList<RemoveRule>();
    }

    public String getName() {
        return name;
    }

    public void apply(Diagram diagram) {

        for (RemoveRule r : rules) {

            List<Figure> list = r.getSelector().selected(diagram);
            Set<Figure> figuresToRemove = new HashSet<Figure>();

            List<Figure> protectedFigures = null;
            if (r.getRemoveAllWithoutPredecessor()) {
                protectedFigures = diagram.getRootFigures();
            }

            for (Figure f : list) {
                if (r.getRemoveOnlyInputs()) {
                    List<InputSlot> inputSlots = new ArrayList<InputSlot>();
                    for (InputSlot is : f.getInputSlots()) {
                        inputSlots.add(is);
                    }
                    for (InputSlot is : inputSlots) {
                        f.removeSlot(is);
                    }

                    f.createInputSlot();
                } else {
                    figuresToRemove.add(f);
                }
            }

            if (r.getRemoveAllWithoutPredecessor()) {
                boolean progress = true;
                while (progress) {
                    List<Figure> rootFigures = diagram.getRootFigures();
                    progress = false;
                    for (Figure f : rootFigures) {
                        if (!protectedFigures.contains(f)) {
                            figuresToRemove.add(f);
                            progress = true;
                        }
                    }
                }
            }

            diagram.removeAllFigures(figuresToRemove);
        }
    }

    public void addRule(RemoveRule rule) {
        rules.add(rule);
    }

    public static class RemoveRule {

        private Selector selector;
        private boolean removeAllWithoutPredecessor;
        private boolean removeOnlyInputs;

        public RemoveRule(Selector selector, boolean b) {
            this(selector, b, false);
        }

        public RemoveRule(Selector selector, boolean removeAllWithoutPredecessor, boolean removeOnlyInputs) {
            this.selector = selector;
            this.removeOnlyInputs = removeOnlyInputs;
            this.removeAllWithoutPredecessor = removeAllWithoutPredecessor;
        }

        public Selector getSelector() {
            return selector;
        }

        public boolean getRemoveOnlyInputs() {
            return removeOnlyInputs;
        }

        public boolean getRemoveAllWithoutPredecessor() {
            return removeAllWithoutPredecessor;
        }
    }
}
