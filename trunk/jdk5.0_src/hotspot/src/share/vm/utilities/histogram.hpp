#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)histogram.hpp	1.11 03/12/23 16:44:48 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This class provides a framework for collecting various statistics.
// The current implementation is oriented towards counting invocations
// of various types, but that can be easily changed.
//
// To use it, you need to declare a Histogram*, and a subtype of
// HistogramElement:
//
//  HistogramElement* MyHistogram;
//
//  class MyHistogramElement : public HistogramElement {
//    public:
//      MyHistogramElement(char* name);
//  };
//
//  MyHistogramElement::MyHistogramElement(char* elementName) {
//    _name = elementName;
//
//    if(MyHistogram == NULL)
//      MyHistogram = new Histogram("My Call Counts",100);
//
//    MyHistogram->add_element(this);
//  }
//
//  #define MyCountWrapper(arg) static MyHistogramElement* e = new MyHistogramElement(arg); e->increment_count()
//
// This gives you a simple way to count invocations of specfic functions:
//
// void a_function_that_is_being_counted() {
//   MyCountWrapper("FunctionName");
//   ...
// }
//
// To print the results, invoke print() on your Histogram*.

#ifdef ASSERT

class HistogramElement : public CHeapObj {
 protected:
  jint _count;
  const char* _name;

 public:
  HistogramElement();
  virtual int count();
  virtual const char* name();
  virtual void increment_count();
  void print_on(outputStream* st) const;
  virtual int compare(HistogramElement* e1,HistogramElement* e2);
};

class Histogram : public CHeapObj {
 protected:
  GrowableArray<HistogramElement*>* _elements;
  GrowableArray<HistogramElement*>* elements() { return _elements; }
  const char* _title;
  const char* title() { return _title; }
  static int sort_helper(HistogramElement** e1,HistogramElement** e2);
  virtual void print_header(outputStream* st);
  virtual void print_elements(outputStream* st);

 public:
  Histogram(const char* title,int estimatedSize);
  virtual void add_element(HistogramElement* element);
  void print_on(outputStream* st) const;
};

#endif
