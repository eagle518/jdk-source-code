#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)forms.cpp	1.153 03/12/23 16:38:49 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// FORMS.CPP - Definitions for ADL Parser Generic & Utility Forms Classes
#include "adlc.hpp"

//------------------------------Static Initializers----------------------------
// allocate arena used by forms
Arena  *Form::arena = Form::generate_arena(); //  = Form::generate_arena();
Arena *Form::generate_arena() {
  return (new Arena); 
}

//------------------------------NameList---------------------------------------
// reserved user-defined string
const char  *NameList::_signal   = "$$SIGNAL$$";  

// Constructor and Destructor
NameList::NameList() : _cur(0), _max(4), _iter(0), _justReset(true) { 
  _names = (const char**)malloc(_max*sizeof(char*));
}
NameList::~NameList() {
  // The following free is a double-free, and crashes the program:
  //free(_names);                   // not owner of strings
}

void   NameList::addName(const char *name) {
  if (_cur == _max) _names =(const char**)realloc(_names,(_max *=2)*sizeof(char*));
  _names[_cur++] = name;
}

void   NameList::add_signal() {
  addName( _signal );
}
void   NameList::clear() {
  _cur   = 0;
  _iter  = 0;
  _justReset = true;
  // _max   = 4; Already allocated
}

int    NameList::count()  const { return _cur; }

void   NameList::reset()   { _iter = 0; _justReset = true;}
const char  *NameList::iter()    { 
  if (_justReset) {_justReset=false; return (_iter < _cur ? _names[_iter] : NULL);}
  else return (_iter <_cur-1 ? _names[++_iter] : NULL);
}
const char  *NameList::current() { return (_iter < _cur ? _names[_iter] : NULL); }

// Return 'true' if current entry is signal
bool  NameList::current_is_signal() {
  const char *entry = current();
  return is_signal(entry);
}

// Return true if entry is a signal
bool  NameList::is_signal(const char *entry) {
  return ( (strcmp(entry,NameList::_signal) == 0) ? true : false);
}

// Search for a name in the list
bool   NameList::search(const char *name) {
  const char *entry;
  for(reset(); (entry = iter()) != NULL; ) {
    if(!strcmp(entry,name)) return true;
  }
  return false;
}

// Return index of name in list
int    NameList::index(const char *name) {
  int         cnt = 0;
  const char *entry;
  for(reset(); (entry = iter()) != NULL; ) {
    if(!strcmp(entry,name)) return cnt;
    cnt++;
  }
  return Not_in_list;
}

// Return name at index in list
const char  *NameList::name(intptr_t  index) {
  return ( index < _cur ? _names[index] : NULL);
}

void   NameList::dump() { output(stderr); }

void   NameList::output(FILE *fp) {
  fprintf(fp, "\n");

  // Run iteration over all entries, independent of position of iterator.
  const char *name       = NULL;
  int         iter       = 0;
  bool        justReset  = true;

  while( ( name  = (justReset ? 
                    (justReset=false, (iter < _cur ? _names[iter] : NULL)) :
                    (iter < _cur-1 ? _names[++iter] : NULL)) )
         != NULL ) {
    fprintf( fp, "  %s,\n", name);
  }
  fprintf(fp, "\n");
}

//------------------------------NameAndList------------------------------------
// Storage for a name and an associated list of names
NameAndList::NameAndList(char *name) : _name(name) {
}
NameAndList::~NameAndList() {
}

// Add to entries in list
void NameAndList::add_entry(const char *entry) {
  _list.addName(entry);
}

// Access the name and its associated list.
const char *NameAndList::name()  const {  return _name;  }
void        NameAndList::reset()       { _list.reset();  }
const char *NameAndList::iter()        { return _list.iter(); }

// Return the "index" entry in the list, zero-based
const char *NameAndList::operator[](int index) {
  assert( index >= 0, "Internal Error(): index less than 0.");

  _list.reset();
  const char *entry = _list.iter();
  // Iterate further if it isn't at index 0.
  for ( int position = 0; position != index; ++position ) {
    entry = _list.iter();
  }

  return entry;
}


void   NameAndList::dump() { output(stderr); }
void   NameAndList::output(FILE *fp) {
  fprintf(fp, "\n");

  // Output the Name
  fprintf(fp, "Name == %s", (_name ? _name : "") );

  // Output the associated list of names
  const char *name;
  fprintf(fp, " (");
  for (reset(); (name = iter()) != NULL;) {
    fprintf(fp, "  %s,\n", name);
  }
  fprintf(fp, ")");
  fprintf(fp, "\n");
}

//------------------------------Form-------------------------------------------
OpClassForm   *Form::is_opclass()     const {
  return NULL;
}

OperandForm   *Form::is_operand()     const {
  return NULL;
}

InstructForm  *Form::is_instruction() const {
  return NULL;
}

MachNodeForm  *Form::is_machnode() const {
  return NULL;
}

AttributeForm *Form::is_attribute() const {
  return NULL;
}

Effect        *Form::is_effect() const {
  return NULL;
}

ResourceForm  *Form::is_resource() const {
  return NULL;
}

PipeClassForm *Form::is_pipeclass() const {
  return NULL;
}

Form::DataType Form::ideal_to_const_type(const char *name) const {
  if( name == NULL ) { return Form::none; }

  if (strcmp(name,"ConI")==0) return Form::idealI;
  if (strcmp(name,"ConP")==0) return Form::idealP;
  if (strcmp(name,"ConL")==0) return Form::idealL;
  if (strcmp(name,"ConF")==0) return Form::idealF;
  if (strcmp(name,"ConD")==0) return Form::idealD;
  if (strcmp(name,"Bool")==0) return Form::idealI;

  return Form::none;
}

Form::DataType Form::ideal_to_sReg_type(const char *name) const {
  if( name == NULL ) { return Form::none; }

  if (strcmp(name,"sRegI")==0) return Form::idealI;
  if (strcmp(name,"sRegP")==0) return Form::idealP;
  if (strcmp(name,"sRegF")==0) return Form::idealF;
  if (strcmp(name,"sRegD")==0) return Form::idealD;
  if (strcmp(name,"sRegL")==0) return Form::idealL;
  return Form::none;
}

Form::DataType Form::ideal_to_Reg_type(const char *name) const {
  if( name == NULL ) { return Form::none; }

  if (strcmp(name,"RegI")==0) return Form::idealI;
  if (strcmp(name,"RegP")==0) return Form::idealP;
  if (strcmp(name,"RegF")==0) return Form::idealF;
  if (strcmp(name,"RegD")==0) return Form::idealD;
  if (strcmp(name,"RegL")==0) return Form::idealL;

  return Form::none;
}

// True if 'opType', an ideal name, loads or stores.
Form::DataType Form::is_load_from_memory(const char *opType) const {
  if( strcmp(opType,"LoadB")==0 )  return Form::idealB;
  if( strcmp(opType,"LoadC")==0 )  return Form::idealC;
  if( strcmp(opType,"LoadD")==0 )  return Form::idealD;
  if( strcmp(opType,"LoadD_unaligned")==0 )  return Form::idealD;
  if( strcmp(opType,"LoadF")==0 )  return Form::idealF;
  if( strcmp(opType,"LoadI")==0 )  return Form::idealI;
  if( strcmp(opType,"LoadKlass")==0 )  return Form::idealP;
  if( strcmp(opType,"LoadL")==0 )  return Form::idealL;
  if( strcmp(opType,"LoadL_unaligned")==0 )  return Form::idealL;
  if( strcmp(opType,"LoadPLocked")==0 )  return Form::idealP;
  if( strcmp(opType,"LoadLLocked")==0 )  return Form::idealL;
  if( strcmp(opType,"LoadP")==0 )  return Form::idealP;
  if( strcmp(opType,"LoadRange")==0 )  return Form::idealI;
  if( strcmp(opType,"LoadS")==0 )  return Form::idealS;
  assert( strcmp(opType,"Load") != 0, "Must type Loads" );
  return Form::none;
}

Form::DataType Form::is_store_to_memory(const char *opType) const {
  if( strcmp(opType,"StoreB")==0)  return Form::idealB;
  if( strcmp(opType,"StoreCM")==0)  return Form::idealB;
  if( strcmp(opType,"StoreC")==0)  return Form::idealC;
  if( strcmp(opType,"StoreD")==0)  return Form::idealD;
  if( strcmp(opType,"StoreF")==0)  return Form::idealF;
  if( strcmp(opType,"StoreI")==0)  return Form::idealI;
  if( strcmp(opType,"StoreL")==0)  return Form::idealL;
  if( strcmp(opType,"StoreP")==0)  return Form::idealP;
  assert( strcmp(opType,"Store") != 0, "Must type Stores" );
  return Form::none;
}

Form::InterfaceType Form::interface_type(FormDict &globals) const {
  return Form::no_interface;
}

//------------------------------FormList---------------------------------------
// Destructor
FormList::~FormList()  { 
  // // This list may not own its elements
  // Form *cur  = _root;
  // Form *next = NULL;
  // for( ; (cur = next) != NULL; ) {
  //   next = (Form *)cur->_next;
  //   delete cur;
  // }
};

//------------------------------FormDict---------------------------------------
// Constructor
FormDict::FormDict( CmpKey cmp, Hash hash, Arena *arena )
  : _form(cmp, hash, arena) {
}
FormDict::~FormDict() {
}

// Return # of name-Form pairs in dict
int FormDict::Size(void) const { 
  return _form.Size(); 
}

// Insert inserts the given key-value pair into the dictionary.  The prior
// value of the key is returned; NULL if the key was not previously defined.
const Form  *FormDict::Insert(const char *name, Form *form) {
  return (Form*)_form.Insert((void*)name, (void*)form);
}

// Finds the value of a given key; or NULL if not found.  
// The dictionary is NOT changed.
const Form  *FormDict::operator [](const char *name) const {
  return (Form*)_form[name];
}

//------------------------------FormDict::private------------------------------
// Disable public use of constructor, copy-ctor, operator =, operator ==
FormDict::FormDict( ) : _form(cmpkey,hashkey) {
  assert( false, "NotImplemented");
}
FormDict::FormDict( const FormDict & ) : _form(cmpkey,hashkey) {
  assert( false, "NotImplemented");
}
FormDict &FormDict::operator =( const FormDict &rhs) {
  assert( false, "NotImplemented");
  _form = rhs._form;
  return *this;
}
// == compares two dictionaries; they must have the same keys (their keys
// must match using CmpKey) and they must have the same values (pointer
// comparison).  If so 1 is returned, if not 0 is returned.
bool FormDict::operator ==(const FormDict &d) const {
  assert( false, "NotImplemented");
  return false;
}

// Print out the dictionary contents as key-value pairs
static void dumpkey (const void* key)  { fprintf(stdout, "%s", key); }
static void dumpform(const void* form) { fflush(stdout); ((Form*)form)->dump(); }

void FormDict::dump() {
  _form.print(dumpkey, dumpform);
}

//------------------------------SourceForm-------------------------------------
SourceForm::SourceForm(char* code) : _code(code) { }; // Constructor
SourceForm::~SourceForm() {
}

void SourceForm::dump() {                    // Debug printer
  output(stderr);
}

void SourceForm::output(FILE *fp) {
  fprintf(fp,"\n//%s\n%s\n",classname(),(_code?_code:"")); 
} 

