#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)typeArrayKlass.cpp	1.106 03/12/23 16:42:09 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_typeArrayKlass.cpp.incl"

bool typeArrayKlass::compute_is_subtype_of(klassOop k) {
  if (!k->klass_part()->oop_is_typeArray()) {
    return arrayKlass::compute_is_subtype_of(k);
  }

  typeArrayKlass* tak = typeArrayKlass::cast(k);
  if (dimension() != tak->dimension()) return false;

  return type() == tak->type();
}

klassOop typeArrayKlass::create_klass(BasicType type, int scale, TRAPS) {
  typeArrayKlass o;

  symbolHandle sym(symbolOop(NULL));
  // bootstrapping: don't create sym if symbolKlass not created yet
  if (Universe::symbolKlassObj() != NULL) {
    sym = oopFactory::new_symbol_handle(external_name(type), CHECK_0);
  }   
  KlassHandle klassklass (THREAD, Universe::typeArrayKlassKlassObj());

  arrayKlassHandle k = base_create_array_klass(o.vtbl_value(), header_size(), klassklass, CHECK_0);
  typeArrayKlass* ak = typeArrayKlass::cast(k());
  ak->set_name(sym());
  ak->set_type(type);
  ak->set_scale(scale);
  ak->set_size_helper(-1-exact_log2(scale));
  ak->set_array_header_in_bytes(arrayOopDesc::header_size(type) * wordSize);
  ak->set_max_length(arrayOopDesc::max_array_length(type));
  assert(k()->size() > header_size(), "bad size");

  // Call complete_create_array_klass after all instance variables have been initialized.
  KlassHandle super (THREAD, k->super());
  complete_create_array_klass(k, super, CHECK_0);

  return k();
}

typeArrayOop typeArrayKlass::allocate(int length, TRAPS) {  
  assert(scale() > 0, "bad scale");
  if (length >= 0) {
    if (length <= max_length()) {
      size_t size = typeArrayOopDesc::object_size(scale(), length, array_header_in_bytes() / wordSize);    
      KlassHandle h_k(THREAD, as_klassOop());
      typeArrayOop t;
      CollectedHeap* ch = Universe::heap();
      if (size < ch->large_typearray_limit()) {
        t = (typeArrayOop)CollectedHeap::array_allocate(h_k, (int)size, length, CHECK_0);
      } else {
        t = (typeArrayOop)CollectedHeap::large_typearray_allocate(h_k, (int)size, length, CHECK_0);
      }
      return t;
    } else {
      THROW_OOP_0(Universe::out_of_memory_error_array_size());
    }
  } else {
    THROW_0(vmSymbols::java_lang_NegativeArraySizeException());
  }
}

typeArrayOop typeArrayKlass::allocate_permanent(int length, TRAPS) {  
  if (length < 0) THROW_0(vmSymbols::java_lang_NegativeArraySizeException());
  int size = typeArrayOopDesc::object_size(scale(), length, array_header_in_bytes() / wordSize);
  KlassHandle h_k(THREAD, as_klassOop());  
  typeArrayOop t = (typeArrayOop)
    CollectedHeap::permanent_array_allocate(h_k, size, length, CHECK_0);
  return t;
}

oop typeArrayKlass::multi_allocate(int rank, jint* last_size, int next_dim_step, TRAPS) {  
  assert( next_dim_step == 1  ||  next_dim_step == -1,  "next_dim_step better be one of these");

  // For typeArrays this is only called for the last dimension
  assert(rank == 1, "just checking");
  int length = *last_size;
  return allocate(length, THREAD);
}


void typeArrayKlass::copy_array(arrayOop s, int src_pos, arrayOop d, int dst_pos, int length, TRAPS) {
  assert(s->is_typeArray(), "must be type array");

  // Check destination
  if (!d->is_typeArray() || type() != typeArrayKlass::cast(d->klass())->type()) {
    THROW(vmSymbols::java_lang_ArrayStoreException());
  }

  // Check is all offsets and lengths are non negative
  if (src_pos < 0 || dst_pos < 0 || length < 0) {
    THROW(vmSymbols::java_lang_ArrayIndexOutOfBoundsException());
  }
  // Check if the ranges are valid
  if  ( (((unsigned int) length + (unsigned int) src_pos) > (unsigned int) s->length())
     || (((unsigned int) length + (unsigned int) dst_pos) > (unsigned int) d->length()) ) {
    THROW(vmSymbols::java_lang_ArrayIndexOutOfBoundsException());
  }

  // This is an attempt to make the copy_array fast.
  // NB: memmove takes care of overlapping memory segments.
  // Potential problem: memmove is not guaranteed to be word atomic
  // Revisit in Merlin
  int sc  = scale();
  int ihs = array_header_in_bytes() / wordSize;
  char* src = (char*) ((oop*)s + ihs) + (src_pos * sc);
  char* dst = (char*) ((oop*)d + ihs) + (dst_pos * sc);
  memmove(dst, src, length * sc);
}


// create a klass of array holding typeArrays
klassOop typeArrayKlass::array_klass_impl(bool or_null, int n, TRAPS) {
  typeArrayKlassHandle h_this(THREAD, as_klassOop());
  return array_klass_impl(h_this, or_null, n, THREAD);
}

klassOop typeArrayKlass::array_klass_impl(typeArrayKlassHandle h_this, bool or_null, int n, TRAPS) {  
  int dimension = h_this->dimension();
  assert(dimension <= n, "check order of chain");
    if (dimension == n) 
      return h_this();

  objArrayKlassHandle  h_ak(THREAD, h_this->higher_dimension());
  if (h_ak.is_null()) {    
    if (or_null)  return NULL;

    ResourceMark rm;
    JavaThread *jt = (JavaThread *)THREAD;
    {
      MutexLocker mc(Compile_lock, THREAD);   // for vtables
      // Atomic create higher dimension and link into list
      MutexLocker mu(MultiArray_lock, THREAD);
    
      h_ak = objArrayKlassHandle(THREAD, h_this->higher_dimension());
      if (h_ak.is_null()) {
        // We grab locks above and the allocate_objArray_klass() code
        // path needs to post OBJECT_ALLOC events for the newly
        // allocated objects. We can't post events while holding locks
        // so we store the information on the side until after we
        // release the locks.
        if (Universe::jvmpi_alloc_event_enabled()) {
          jt->set_deferred_obj_alloc_events(
            new GrowableArray<DeferredObjAllocEvent *>(dimension + 1, true));
        }
        klassOop oak = objArrayKlassKlass::cast(
          Universe::objArrayKlassKlassObj())->allocate_objArray_klass(
          dimension + 1, h_this, CHECK_0);
        h_ak = objArrayKlassHandle(THREAD, oak);
        h_ak->set_lower_dimension(h_this());
        h_this->set_higher_dimension(h_ak());
        assert(h_ak->oop_is_objArray(), "incorrect initialization of objArrayKlass");    
      }       
    }

    GrowableArray<DeferredObjAllocEvent *>* deferred_list =
      jt->deferred_obj_alloc_events();
    if (deferred_list != NULL) {
      if (deferred_list->length() > 0) {
        Universe::jvmpi_post_deferred_obj_alloc_events(deferred_list);
      }
      jt->set_deferred_obj_alloc_events(NULL);
    }
  }
  if (or_null) {
    return h_ak->array_klass_or_null(n); 
  }
  return h_ak->array_klass(n, CHECK_0);
}

klassOop typeArrayKlass::array_klass_impl(bool or_null, TRAPS) {
  return array_klass_impl(or_null, dimension() +  1, THREAD);
}

int typeArrayKlass::oop_size(oop obj) const { 
  assert(obj->is_typeArray(),"must be a type array");
  typeArrayOop t = typeArrayOop(obj);
  return t->object_size();
}


void typeArrayKlass::oop_follow_contents(oop obj) {
  assert(obj->is_typeArray(),"must be a type array");
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::typeArrayKlass never moves.
}


int typeArrayKlass::oop_adjust_pointers(oop obj) {
  assert(obj->is_typeArray(),"must be a type array");
  typeArrayOop t = typeArrayOop(obj);
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::typeArrayKlass never moves.
  return t->object_size();
}


int typeArrayKlass::oop_oop_iterate(oop obj, OopClosure* blk) {
  assert(obj->is_typeArray(),"must be a type array");
  typeArrayOop t = typeArrayOop(obj);
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::typeArrayKlass never moves.
  return t->object_size();
}


int typeArrayKlass::oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr) {
  assert(obj->is_typeArray(),"must be a type array");
  typeArrayOop t = typeArrayOop(obj);
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::typeArrayKlass never moves.
  return t->object_size();
}


void typeArrayKlass::oop_copy_contents(PSPromotionManager* pm, oop obj) {
  assert(obj->is_typeArray(),"must be a type array");
}


void typeArrayKlass::initialize(TRAPS) {
  // Nothing to do. Having this function is handy since objArrayKlasses can be
  // initialized by calling initialize on their bottom_klass, see objArrayKlass::initialize
}


const char* typeArrayKlass::external_name(BasicType type) {
  switch (type) {
    case T_BOOLEAN: return "[Z";
    case T_CHAR:    return "[C";
    case T_FLOAT:   return "[F";
    case T_DOUBLE:  return "[D";
    case T_BYTE:    return "[B";
    case T_SHORT:   return "[S";
    case T_INT:     return "[I";
    case T_LONG:    return "[J";
    default: ShouldNotReachHere();
  }
  return NULL;
}


#ifndef PRODUCT
// Printing

static void print_boolean_array(typeArrayOop ta, int print_len, outputStream* st) {
  for (int index = 0; index < print_len; index++) {
    st->print_cr(" - %3d: %s", index, (ta->bool_at(index) == 0) ? "false" : "true");
  }
}


static void print_char_array(typeArrayOop ta, int print_len, outputStream* st) {
  for (int index = 0; index < print_len; index++) {
    jchar c = ta->char_at(index);
    st->print_cr(" - %3d: %x %c", index, c, isprint(c) ? c : ' ');
  }
}


static void print_float_array(typeArrayOop ta, int print_len, outputStream* st) {
  for (int index = 0; index < print_len; index++) {
    st->print_cr(" - %3d: %g", index, ta->float_at(index));
  }
}


static void print_double_array(typeArrayOop ta, int print_len, outputStream* st) {
  for (int index = 0; index < print_len; index++) {
    st->print_cr(" - %3d: %g", index, ta->double_at(index));
  }
}


static void print_byte_array(typeArrayOop ta, int print_len, outputStream* st) {
  for (int index = 0; index < print_len; index++) {
    jbyte c = ta->byte_at(index);
    st->print_cr(" - %3d: %x %c", index, c, isprint(c) ? c : ' ');
  }
}


static void print_short_array(typeArrayOop ta, int print_len, outputStream* st) {
  for (int index = 0; index < print_len; index++) {
    int v = ta->ushort_at(index);
    st->print_cr(" - %3d: 0x%x\t %d", index, v, v);
  }
}


static void print_int_array(typeArrayOop ta, int print_len, outputStream* st) {
  for (int index = 0; index < print_len; index++) {
    jint v = ta->int_at(index);
    st->print_cr(" - %3d: 0x%x %d", index, v, v);
  }
}


static void print_long_array(typeArrayOop ta, int print_len, outputStream* st) {
  for (int index = 0; index < print_len; index++) {
    jlong v = ta->long_at(index);
    st->print_cr(" - %3d: 0x%x 0x%x", index, high(v), low(v));
  }
}


void typeArrayKlass::oop_print_on(oop obj, outputStream* st) {
  arrayKlass::oop_print_on(obj, st);
  typeArrayOop ta = typeArrayOop(obj);
  int print_len = MIN2((intx) ta->length(), MaxElementPrintSize);
  switch (type()) {
    case T_BOOLEAN: print_boolean_array(ta, print_len, st); break;
    case T_CHAR:    print_char_array(ta, print_len, st);    break;
    case T_FLOAT:   print_float_array(ta, print_len, st);   break;
    case T_DOUBLE:  print_double_array(ta, print_len, st);  break;
    case T_BYTE:    print_byte_array(ta, print_len, st);    break;
    case T_SHORT:   print_short_array(ta, print_len, st);   break;
    case T_INT:     print_int_array(ta, print_len, st);     break;
    case T_LONG:    print_long_array(ta, print_len, st);    break;
    default: ShouldNotReachHere();
  }
  int remaining = ta->length() - print_len;
  if (remaining > 0) {
    tty->print_cr(" - <%d more elements, increase MaxElementPrintSize to print>", remaining);
  }
}


const char* typeArrayKlass::internal_name() const {
  return Klass::external_name();
}
#endif // PRODUCT
