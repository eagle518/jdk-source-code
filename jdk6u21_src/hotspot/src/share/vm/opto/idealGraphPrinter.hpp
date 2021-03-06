/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
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

#ifndef PRODUCT

class Compile;
class PhaseIFG;
class PhaseChaitin;
class Matcher;
class Node;
class InlineTree;
class ciMethod;

class IdealGraphPrinter
{
private:

  enum State
  {
    Invalid,
    Valid,
    New
  };

private:

  static const char *INDENT;
  static const char *TOP_ELEMENT;
  static const char *GROUP_ELEMENT;
  static const char *GRAPH_ELEMENT;
  static const char *PROPERTIES_ELEMENT;
  static const char *EDGES_ELEMENT;
  static const char *PROPERTY_ELEMENT;
  static const char *EDGE_ELEMENT;
  static const char *NODE_ELEMENT;
  static const char *NODES_ELEMENT;
  static const char *CONTROL_FLOW_ELEMENT;
  static const char *REMOVE_EDGE_ELEMENT;
  static const char *REMOVE_NODE_ELEMENT;
  static const char *METHOD_NAME_PROPERTY;
  static const char *BLOCK_NAME_PROPERTY;
  static const char *BLOCK_DOMINATOR_PROPERTY;
  static const char *BLOCK_ELEMENT;
  static const char *SUCCESSORS_ELEMENT;
  static const char *SUCCESSOR_ELEMENT;
  static const char *METHOD_IS_PUBLIC_PROPERTY;
  static const char *METHOD_IS_STATIC_PROPERTY;
  static const char *TRUE_VALUE;
  static const char *NODE_NAME_PROPERTY;
  static const char *EDGE_NAME_PROPERTY;
  static const char *NODE_ID_PROPERTY;
  static const char *FROM_PROPERTY;
  static const char *TO_PROPERTY;
  static const char *PROPERTY_NAME_PROPERTY;
  static const char *GRAPH_NAME_PROPERTY;
  static const char *INDEX_PROPERTY;
  static const char *METHOD_ELEMENT;
  static const char *INLINE_ELEMENT;
  static const char *BYTECODES_ELEMENT;
  static const char *METHOD_BCI_PROPERTY;
  static const char *METHOD_SHORT_NAME_PROPERTY;
  static const char *ASSEMBLY_ELEMENT;

  elapsedTimer _walk_time;
  elapsedTimer _output_time;
  elapsedTimer _build_blocks_time;

  static int _file_count;
  networkStream *_stream;
  xmlStream *_xml;
  outputStream *_output;
  ciMethod *_current_method;
  int _depth;
  char buffer[128];
  bool _should_send_method;
  PhaseChaitin* _chaitin;
  bool _traverse_outs;
  Compile *C;

  static void pre_node(Node* node, void *env);
  static void post_node(Node* node, void *env);

  void print_indent();
  void print_method(ciMethod *method, int bci, InlineTree *tree);
  void print_inline_tree(InlineTree *tree);
  void visit_node(Node *n, void *param);
  void walk_nodes(Node *start, void *param);
  void begin_elem(const char *s);
  void end_elem();
  void begin_head(const char *s);
  void end_head();
  void print_attr(const char *name, const char *val);
  void print_attr(const char *name, intptr_t val);
  void print_prop(const char *name, const char *val);
  void print_prop(const char *name, int val);
  void tail(const char *name);
  void head(const char *name);
  void text(const char *s);
  intptr_t get_node_id(Node *n);
  IdealGraphPrinter();
  ~IdealGraphPrinter();

public:

  static void clean_up();
  static IdealGraphPrinter *printer();

  bool traverse_outs();
  void set_traverse_outs(bool b);
  outputStream *output();
  void print_inlining(Compile* compile);
  void begin_method(Compile* compile);
  void end_method();
  void print_method(Compile* compile, const char *name, int level=1, bool clear_nodes = false);
  void print(Compile* compile, const char *name, Node *root, int level=1, bool clear_nodes = false);
  void print_xml(const char *name);


};

#endif
