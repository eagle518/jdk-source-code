#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)jvm_db_helper.c	1.9 04/03/17 12:08:20 JVM_DB"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proc_service.h>

#include "libproc.h"
#include "libjvm_db.h"

static int debug = 0;

static void failed(int err, const char * file, int line) {
  if (debug) {
    fprintf(stderr, "failed %d at %s:%d\n", err, file, line);
  }
}

static void warn(const char * file, int line, const char * msg) {
  if (debug) {
    fprintf(stderr, "warning: %s at %s:%d\n", msg, file, line);
  }
}

static void warn1(const char * file, int line, const char * msg, intptr_t arg1) {
  if (debug) {
    fprintf(stderr, "warning: ");
    fprintf(stderr, msg, arg1);
    fprintf(stderr, " at %s:%d\n", file, line);
  }
}

static int find_jlong_constant(jvm_agent_t* J, const char *name, uint64_t* valuep);

#define CHECK_FAIL(err) \
	if (err != PS_OK) { failed(err, __FILE__, __LINE__); goto fail; }
#define WARN1(msg, arg1)  warn1(__FILE__, __LINE__, msg, arg1)

#define FIND_JLONG_CONSTANT(const_name) \
		find_jlong_constant(J, #const_name, &J->const_name)

#define TYPE_CMP(name) \
		(vmp->typeName[0] == name[0] && strcmp(name, vmp->typeName) == 0)

#define FIELD_CMP(name) \
		(strcmp(name, vmp->fieldName) == 0)

typedef struct VMStructEntry {
  const char * typeName;           /* The type name containing the given field (example: "Klass") */
  const char * fieldName;          /* The field name within the type           (example: "_name") */
  uint64_t offset;                 /* Offset of field within structure; only used for nonstatic fields */
  uint64_t address;                /* Address of field; only used for static fields */
} VMStructEntry;


typedef struct VMTypeEntry {
  const char* typeName;            /* Type name (example: "methodOopDesc") */
  const char* superclassName;      /* Superclass name, or null if none (example: "oopDesc") */
  uint64_t size;                   /* Size, in bytes, of the type */
} VMTypeEntry;

struct jvm_agent {
  struct ps_prochandle* P;
  int DATA_MODEL;
  int POINTER_SIZE;

  uint64_t gHotSpotVMStructEntryArrayStride;
  uint64_t gHotSpotVMStructEntryTypeNameOffset;
  uint64_t gHotSpotVMStructEntryFieldNameOffset;
  uint64_t gHotSpotVMStructEntryOffsetOffset;
  uint64_t gHotSpotVMStructEntryAddressOffset;

  uint64_t gHotSpotVMTypeEntryArrayStride;
  uint64_t gHotSpotVMTypeEntryTypeNameOffset;
  uint64_t gHotSpotVMTypeEntrySizeOffset;
  uint64_t gHotSpotVMTypeEntrySuperclassNameOffset;

  int OFFSET_interpreter_frame_method;
  int OFFSET_Klass_name;
  int OFFSET_constantPoolOopDesc_pool_holder;

  int OFFSET_HeapBlockHeader_used;
  int OFFSET_AccessFlags_flags;
  int OFFSET_oopDesc_klass;
  int OFFSET_nmethod_method;

  int OFFSET_methodOopDesc_constMethod;
  int OFFSET_methodOopDesc_constants;
  int OFFSET_methodOopDesc_name_index;
  int OFFSET_methodOopDesc_signature_index;

  int OFFSET_constMethodOopDesc_flags;
  int OFFSET_constMethodOopDesc_code_size;
  int OFFSET_constMethodOopDesc_name_index;
  int OFFSET_constMethodOopDesc_signature_index;

  int OFFSET_symbolOopDesc_body;
  int OFFSET_symbolOopDesc_length;

  int OFFSET_CodeHeap_memory;
  int OFFSET_CodeHeap_segmap;
  int OFFSET_CodeHeap_log2_segment_size;

  int OFFSET_VirtualSpace_low_boundary;
  int OFFSET_VirtualSpace_high_boundary;
  int OFFSET_VirtualSpace_low;
  int OFFSET_VirtualSpace_high;

  int OFFSET_SIZE_CodeBlob;
  int OFFSET_CodeBlob_name;
  int OFFSET_CodeBlob_header_size;
  int OFFSET_CodeBlob_relocation_size;
  int OFFSET_CodeBlob_instructions_offset;
  int OFFSET_CodeBlob_data_offset;
  int OFFSET_CodeBlob_oops_offset;
  int OFFSET_CodeBlob_oops_length;
  int OFFSET_CodeBlob_frame_size;
  int OFFSET_CodeBlob_oop_maps;

  int SIZE_constMethodOopDesc;
  int SIZE_HeapBlockHeader;
  int SIZE_oopDesc;
  int SIZE_constantPoolOopDesc;
  int32_t  SIZE_CodeCache_log2_segment;

  int SIZE_nmethod;
  int SIZE_CodeBlob;
  int SIZE_BufferBlob;
  int SIZE_SingletonBlob;
  int SIZE_RuntimeStub;
  int SIZE_SafepointBlob;

  uint64_t nmethod_vtbl;
  uint64_t CodeBlob_vtbl;
  uint64_t BufferBlob_vtbl;
  uint64_t RuntimeStub_vtbl;

  uint64_t I2CAdapter_vtbl;

  uint64_t Universe_methodKlassObj_address;
  uint64_t CodeCache_heap_address;

  uint64_t Universe_methodKlassObj;
  uint64_t CodeCache_low;
  uint64_t CodeCache_high;
  uint64_t CodeCache_segmap_low;
  uint64_t CodeCache_segmap_high;
};


static void print_jvm_defs(jvm_agent_t* J);

int main(int argc, char ** argv) {
  pid_t child_pid;
  struct ps_prochandle* proc;
  jvm_agent_t* agent;
  int perr;


  if (argc > 1) {
    sscanf(argv[1], "%d", &child_pid);
    argc -= 2;
    argv += 2;
  } else {
    exit(1);
  }
  proc = Pgrab(child_pid, PGRAB_RDONLY, &perr);
  if (proc == NULL) {
    fprintf(stderr, "Pgrab: %s\n", Pgrab_error(perr));
    exit(1);
  }
  agent = Jagent_create(proc);
  print_jvm_defs(agent);
  Jagent_destroy(agent);
}

static int
read_string(struct ps_prochandle *P,
	char *buf, 		/* caller's buffer */
	size_t size,		/* upper limit on bytes to read */
	uintptr_t addr)		/* address in process */
{
  int err = PS_OK;
  while (size-- > 1 && err == PS_OK) {
    err = ps_pread(P, addr, buf, 1);
    if (*buf == '\0') {
      return PS_OK;
    }
    addr += 1;
    buf += 1;
  }
  return -1;
}

static int read_pointer(jvm_agent_t* J, uint64_t base, uint64_t* ptr) {
  int err = -1;
  uint32_t ptr32;
  switch (J->DATA_MODEL) {
  case PR_MODEL_LP64:
    err = ps_pread(J->P, base, ptr, sizeof(uint64_t));
    break;
  case PR_MODEL_ILP32:
    err = ps_pread(J->P, base, &ptr32, sizeof(uint32_t));
    *ptr = ptr32;
    break;
  }
  return err;
}

static int read_string_pointer(jvm_agent_t* J, uint64_t base, const char ** stringp) {
  uint64_t ptr;
  int err;
  char buffer[1024];

  *stringp = NULL;
  err = read_pointer(J, base, &ptr);
  CHECK_FAIL(err);
  if (ptr != 0) {
    err = read_string(J->P, buffer, sizeof(buffer), ptr);
    CHECK_FAIL(err);
    *stringp = strdup(buffer);
  }
  return PS_OK;

 fail:
  return err;
}

static int parse_vmstruct_entry(jvm_agent_t* J, uint64_t base, VMStructEntry* vmp) {
  int err;
  uint64_t ptr;

  err = read_string_pointer(J,
		base + J->gHotSpotVMStructEntryTypeNameOffset, &vmp->typeName);
  CHECK_FAIL(err);
  err = read_string_pointer(J,
		base + J->gHotSpotVMStructEntryFieldNameOffset, &vmp->fieldName);
  CHECK_FAIL(err);
  err = ps_pread(J->P,
		base + J->gHotSpotVMStructEntryOffsetOffset, &vmp->offset, sizeof(vmp->offset));
  CHECK_FAIL(err);
  err = read_pointer(J,
		base + J->gHotSpotVMStructEntryAddressOffset, &vmp->address);
  CHECK_FAIL(err);

  return PS_OK;

 fail:
  if (vmp->typeName != NULL) free((void*)vmp->typeName);
  if (vmp->fieldName != NULL) free((void*)vmp->fieldName);
  return err;
}

static int parse_vmtype_entry(jvm_agent_t* J, uint64_t base, VMTypeEntry* vmp) {
  int err;
  uint64_t ptr;

  err = read_string_pointer(J,
		base + J->gHotSpotVMTypeEntryTypeNameOffset, &vmp->typeName);
  CHECK_FAIL(err);
  err = read_string_pointer(J,
		base + J->gHotSpotVMTypeEntrySuperclassNameOffset, &vmp->superclassName);
  CHECK_FAIL(err);
  err = ps_pread(J->P,
		base + J->gHotSpotVMTypeEntrySizeOffset, &vmp->size, sizeof(vmp->size));
  CHECK_FAIL(err);

  return PS_OK;

 fail:
  if (vmp->typeName != NULL) free((void*)vmp->typeName);
  if (vmp->superclassName != NULL) free((void*)vmp->superclassName);
  return err;
}

static int parse_vmstructs(jvm_agent_t* J) {
  VMStructEntry  vmVar;
  VMStructEntry* vmp = &vmVar;
  uint64_t gHotSpotVMStructs;
  psaddr_t sym_addr;
  uint64_t base;
  int err;

  err = ps_pglobal_lookup(J->P, "libjvm.so", "gHotSpotVMStructs", &sym_addr);
  CHECK_FAIL(err);
  err = read_pointer(J, sym_addr, &gHotSpotVMStructs);
  CHECK_FAIL(err);
  base = gHotSpotVMStructs;

  err = FIND_JLONG_CONSTANT(gHotSpotVMStructEntryArrayStride);
  CHECK_FAIL(err);
  err = FIND_JLONG_CONSTANT(gHotSpotVMStructEntryTypeNameOffset);
  CHECK_FAIL(err);
  err = FIND_JLONG_CONSTANT(gHotSpotVMStructEntryFieldNameOffset);
  CHECK_FAIL(err);
  err = FIND_JLONG_CONSTANT(gHotSpotVMStructEntryOffsetOffset);
  CHECK_FAIL(err);
  err = FIND_JLONG_CONSTANT(gHotSpotVMStructEntryAddressOffset);
  CHECK_FAIL(err);

  err = PS_OK;
  while (err == PS_OK) {
    memset(vmp, 0, sizeof(VMStructEntry));
    err = parse_vmstruct_entry(J, base, vmp);
    if (err != PS_OK || vmp->typeName == NULL) {
      break;
    }
    if (TYPE_CMP("methodOopDesc")) {
      if (FIELD_CMP("_constMethod")) {
        J->OFFSET_methodOopDesc_constMethod = vmp->offset;
      } else if (FIELD_CMP("_constants")) {
        J->OFFSET_methodOopDesc_constants = vmp->offset;
      } else if (FIELD_CMP("_name_index")) {
        J->OFFSET_methodOopDesc_name_index = vmp->offset;
      } else if (FIELD_CMP("_signature_index")) {
        J->OFFSET_methodOopDesc_signature_index = vmp->offset;
      }
    } else if (TYPE_CMP("constMethodOopDesc")) {
      if (FIELD_CMP("_flags")) {
        J->OFFSET_constMethodOopDesc_flags = vmp->offset;
      } else if (FIELD_CMP("_code_size")) {
        J->OFFSET_constMethodOopDesc_code_size = vmp->offset;
      } else if (FIELD_CMP("_name_index")) {
        J->OFFSET_constMethodOopDesc_name_index = vmp->offset;
      } else if (FIELD_CMP("_signature_index")) {
        J->OFFSET_constMethodOopDesc_signature_index = vmp->offset;
      }
    } else if (TYPE_CMP("constantPoolOopDesc")) {
      if (FIELD_CMP("_pool_holder")) {
        J->OFFSET_constantPoolOopDesc_pool_holder = vmp->offset;
      }
    } else if (TYPE_CMP("symbolOopDesc")) {
      if (FIELD_CMP("_body")) {
        J->OFFSET_symbolOopDesc_body = vmp->offset;
      } else if (FIELD_CMP("_length")) {
        J->OFFSET_symbolOopDesc_length = vmp->offset;
      }
    } else if (TYPE_CMP("AccessFlags")) {
      if (FIELD_CMP("_flags")) {
        J->OFFSET_AccessFlags_flags = vmp->offset;
      }
    } else if (TYPE_CMP("oopDesc")) {
      if (FIELD_CMP("_klass")) {
        J->OFFSET_oopDesc_klass = vmp->offset;
      }
    } else if (TYPE_CMP("Klass")) {
      if (FIELD_CMP("_name")) {
        J->OFFSET_Klass_name = vmp->offset;
      }
    } else if (TYPE_CMP("CodeCache")) {
      if (FIELD_CMP("_heap")) {
        err = read_pointer(J, vmp->address, &J->CodeCache_heap_address);
      }
    } else if (TYPE_CMP("Universe")) {
      if (FIELD_CMP("_methodKlassObj")) {
        J->Universe_methodKlassObj_address = vmp->address;
      }
    } else if (TYPE_CMP("CodeBlob")) {
      if (FIELD_CMP("_size")) {
        J->OFFSET_SIZE_CodeBlob = vmp->offset;
      } else 
      if (FIELD_CMP("_name")) {
        J->OFFSET_CodeBlob_name = vmp->offset;
      } else 
      if (FIELD_CMP("_header_size")) {
        J->OFFSET_CodeBlob_header_size = vmp->offset;
      } else 
      if (FIELD_CMP("_instructions_offset")) {
        J->OFFSET_CodeBlob_instructions_offset = vmp->offset;
      }
    } else if (TYPE_CMP("HeapBlock::Header")) {
      if (FIELD_CMP("_used")) {
        J->OFFSET_HeapBlockHeader_used = vmp->offset;
      }
    } else if (TYPE_CMP("nmethod")) {
      if (FIELD_CMP("_method")) {
        J->OFFSET_nmethod_method = vmp->offset;
      }
    } else if (TYPE_CMP("CodeHeap")) {
      if (FIELD_CMP("_memory")) {
        J->OFFSET_CodeHeap_memory = vmp->offset;
      } else if (FIELD_CMP("_segmap")) {
        J->OFFSET_CodeHeap_segmap = vmp->offset;
      } else if (FIELD_CMP("_log2_segment_size")) {
        J->OFFSET_CodeHeap_log2_segment_size = vmp->offset;
      }
    } else if (TYPE_CMP("VirtualSpace")) {
      if (FIELD_CMP("_low_boundary")) {
        J->OFFSET_VirtualSpace_low_boundary = vmp->offset;
      } else if (FIELD_CMP("_high_boundary")) {
        J->OFFSET_VirtualSpace_high_boundary = vmp->offset;
      } else if (FIELD_CMP("_low")) {
        J->OFFSET_VirtualSpace_low = vmp->offset;
      } else if (FIELD_CMP("_high")) {
        J->OFFSET_VirtualSpace_high = vmp->offset;
      }
    }
    CHECK_FAIL(err);

    base += J->gHotSpotVMStructEntryArrayStride;
    if (vmp->typeName != NULL) free((void*)vmp->typeName);
    if (vmp->fieldName != NULL) free((void*)vmp->fieldName);
  }

  return PS_OK;

 fail:
  if (vmp->typeName != NULL) free((void*)vmp->typeName);
  if (vmp->fieldName != NULL) free((void*)vmp->fieldName);
  return -1;
}

static int parse_vmtypes(jvm_agent_t* J) {
  VMTypeEntry  vmVar;
  VMTypeEntry* vmp = &vmVar;
  uint64_t gHotSpotVMTypes;
  psaddr_t sym_addr;
  uint64_t base;
  int err;

  err = ps_pglobal_lookup(J->P, "libjvm.so", "gHotSpotVMTypes", &sym_addr);
  CHECK_FAIL(err);
  err = read_pointer(J, sym_addr, &gHotSpotVMTypes);
  CHECK_FAIL(err);
  base = gHotSpotVMTypes;

  err = FIND_JLONG_CONSTANT(gHotSpotVMTypeEntryArrayStride);
  CHECK_FAIL(err);
  err = FIND_JLONG_CONSTANT(gHotSpotVMTypeEntryTypeNameOffset);
  CHECK_FAIL(err);
  err = FIND_JLONG_CONSTANT(gHotSpotVMTypeEntrySizeOffset);
  CHECK_FAIL(err);
  err = FIND_JLONG_CONSTANT(gHotSpotVMTypeEntrySuperclassNameOffset);
  CHECK_FAIL(err);

  err = PS_OK;
  while (err == PS_OK) {
    memset(vmp, 0, sizeof(VMTypeEntry));
    err = parse_vmtype_entry(J, base, vmp);
    if (err != PS_OK || vmp->typeName == NULL) {
      break;
    }
    if (TYPE_CMP("oopDesc")) {
      J->SIZE_oopDesc = vmp->size;
    } else if (TYPE_CMP("constMethodOopDesc")) {
      J->SIZE_constMethodOopDesc = vmp->size;
    } else if (TYPE_CMP("HeapBlock::Header")) {
      J->SIZE_HeapBlockHeader = vmp->size;
    } else if (TYPE_CMP("CodeBlob")) {
      J->SIZE_CodeBlob = vmp->size;
    } else if (TYPE_CMP("constantPoolOopDesc")) {
      J->SIZE_constantPoolOopDesc = vmp->size;
    } else if (TYPE_CMP("RuntimeStub")) {
      J->SIZE_RuntimeStub = vmp->size;
    } else if (TYPE_CMP("BufferBlob")) {
      J->SIZE_BufferBlob = vmp->size;
    } else if (TYPE_CMP("nmethod")) {
      J->SIZE_nmethod = vmp->size;
    } else if (TYPE_CMP("SingletonBlob")) {
      J->SIZE_SingletonBlob = vmp->size;
    } else if (TYPE_CMP("SafepointBlob")) {
      J->SIZE_SafepointBlob = vmp->size;
    }
    CHECK_FAIL(err);

    base += J->gHotSpotVMTypeEntryArrayStride;
    if (vmp->typeName != NULL) free((void*)vmp->typeName);
    if (vmp->superclassName != NULL) free((void*)vmp->superclassName);
  }

  return PS_OK;

 fail:
  if (vmp->typeName != NULL) free((void*)vmp->typeName);
  if (vmp->superclassName != NULL) free((void*)vmp->superclassName);
  return -1;
}

static int read_volatiles(jvm_agent_t* J) {
  uint64_t ptr;
  int err;

  err = read_pointer(J, J->Universe_methodKlassObj_address, &J->Universe_methodKlassObj);
  CHECK_FAIL(err);

  err = read_pointer(J, J->CodeCache_heap_address + J->OFFSET_CodeHeap_memory +
                     J->OFFSET_VirtualSpace_low, &J->CodeCache_low);
  CHECK_FAIL(err);
  err = read_pointer(J, J->CodeCache_heap_address + J->OFFSET_CodeHeap_memory +
                     J->OFFSET_VirtualSpace_high, &J->CodeCache_high);
  CHECK_FAIL(err);
  err = read_pointer(J, J->CodeCache_heap_address + J->OFFSET_CodeHeap_segmap +
                     J->OFFSET_VirtualSpace_low, &J->CodeCache_segmap_low);
  CHECK_FAIL(err);
  err = read_pointer(J, J->CodeCache_heap_address + J->OFFSET_CodeHeap_segmap +
                     J->OFFSET_VirtualSpace_high, &J->CodeCache_segmap_high);
  CHECK_FAIL(err);
  err = ps_pread(J->P, J->CodeCache_heap_address + J->OFFSET_CodeHeap_log2_segment_size,
                 &J->SIZE_CodeCache_log2_segment, sizeof(J->SIZE_CodeCache_log2_segment));
  CHECK_FAIL(err);

  return PS_OK;

 fail:
  return err;
}

static int find_symbol(jvm_agent_t* J, const char *name, uint64_t* valuep) {
  psaddr_t sym_addr;
  int err;

  err = ps_pglobal_lookup(J->P, "libjvm.so", name, &sym_addr);
  if (err != PS_OK) goto fail;
  *valuep = sym_addr;
  return PS_OK;

 fail:
  return err;
}

static int find_jlong_constant(jvm_agent_t* J, const char *name, uint64_t* valuep) {
  psaddr_t sym_addr;
  int err = ps_pglobal_lookup(J->P, "libjvm.so", name, &sym_addr);
  if (err == PS_OK) {
    err = ps_pread(J->P, sym_addr, valuep, sizeof(uint64_t));
    return err;
  }
  *valuep = -1;
  return -1;
}

jvm_agent_t *Jagent_create(struct ps_prochandle *P) {
  jvm_agent_t* J = (jvm_agent_t*)calloc(sizeof(struct jvm_agent), 1);
  psaddr_t sym_addr;
  int err;

  debug = getenv("LIBJVMDB_DEBUG") != NULL;

  J->P = P;

  err = ps_pdmodel(J->P, &J->DATA_MODEL);
  CHECK_FAIL(err);

  switch (J->DATA_MODEL) {
  case PR_MODEL_LP64:
    J->POINTER_SIZE = 8;
    break;
  case PR_MODEL_ILP32:
    J->POINTER_SIZE = 4;
    break;
  }

  err = find_symbol(J, "__1cHnmethodG__vtbl_", &J->nmethod_vtbl);
  CHECK_FAIL(err);
  err = find_symbol(J, "__1cKBufferBlobG__vtbl_", &J->BufferBlob_vtbl);
  if (err != PS_OK) J->BufferBlob_vtbl = 0;
  err = find_symbol(J, "__1cICodeBlobG__vtbl_", &J->CodeBlob_vtbl);
  CHECK_FAIL(err);
  err = find_symbol(J, "__1cLRuntimeStubG__vtbl_", &J->RuntimeStub_vtbl);
  CHECK_FAIL(err);
  err = find_symbol(J, "__1cKI2CAdapterG__vtbl_", &J->I2CAdapter_vtbl);
  if (err != PS_OK) J->I2CAdapter_vtbl = 0;

  err = parse_vmstructs(J);
  CHECK_FAIL(err);
  err = parse_vmtypes(J);
  CHECK_FAIL(err);

#if defined(sparc) || defined(__sparc)
  J->OFFSET_interpreter_frame_method = 8;
#elif defined(i386) || defined(__i386)
  J->OFFSET_interpreter_frame_method = -8;
#endif

  err = read_volatiles(J);
  CHECK_FAIL(err);

  return J;
  
 fail:
  Jagent_destroy(J);
  return NULL;
}

void Jagent_destroy(jvm_agent_t *J) {
  if (J != NULL) {
    free(J);
  }
}

#define GEN_CONST(const_name) \
    printf("#define " #const_name " %d\n", J->const_name)

#define GEN_ADDR(const_name) \
    printf("#define " #const_name " %#llx\n", J->const_name)

static void print_jvm_defs(jvm_agent_t* J) {

  GEN_CONST(DATA_MODEL);
  GEN_CONST(POINTER_SIZE);
  printf("\n");

  GEN_CONST(OFFSET_interpreter_frame_method);
  GEN_CONST(OFFSET_Klass_name);
  GEN_CONST(OFFSET_constantPoolOopDesc_pool_holder);
  printf("\n");

  GEN_CONST(OFFSET_HeapBlockHeader_used);
  GEN_CONST(OFFSET_AccessFlags_flags);
  GEN_CONST(OFFSET_oopDesc_klass);
  printf("\n");

  GEN_CONST(OFFSET_nmethod_method);
  printf("\n");

  GEN_CONST(OFFSET_methodOopDesc_constMethod);
  GEN_CONST(OFFSET_methodOopDesc_constants);
  GEN_CONST(OFFSET_methodOopDesc_name_index);
  GEN_CONST(OFFSET_methodOopDesc_signature_index);
  printf("\n");

  GEN_CONST(OFFSET_constMethodOopDesc_flags);
  GEN_CONST(OFFSET_constMethodOopDesc_code_size);
  GEN_CONST(OFFSET_constMethodOopDesc_name_index);
  GEN_CONST(OFFSET_constMethodOopDesc_signature_index);
  printf("\n");

  GEN_CONST(OFFSET_symbolOopDesc_length);
  GEN_CONST(OFFSET_symbolOopDesc_body);
  printf("\n");

  GEN_CONST(OFFSET_CodeHeap_memory);
  GEN_CONST(OFFSET_CodeHeap_segmap);
  GEN_CONST(OFFSET_CodeHeap_log2_segment_size);
  printf("\n");

  GEN_CONST(OFFSET_VirtualSpace_low_boundary);
  GEN_CONST(OFFSET_VirtualSpace_high_boundary);
  GEN_CONST(OFFSET_VirtualSpace_low);
  GEN_CONST(OFFSET_VirtualSpace_high);
  printf("\n");

  GEN_CONST(OFFSET_CodeBlob_name);
  printf("\n");

  GEN_CONST(SIZE_oopDesc);
  GEN_CONST(SIZE_constMethodOopDesc);
  GEN_CONST(SIZE_HeapBlockHeader);
  GEN_CONST(SIZE_constantPoolOopDesc);
  GEN_CONST(SIZE_CodeCache_log2_segment);
  printf("\n");

  GEN_CONST(SIZE_nmethod);
  GEN_CONST(SIZE_CodeBlob);
  GEN_CONST(SIZE_BufferBlob);
  GEN_CONST(SIZE_SingletonBlob);
  GEN_CONST(SIZE_RuntimeStub);
  GEN_CONST(SIZE_SafepointBlob);
  printf("\n");

  GEN_ADDR(nmethod_vtbl);
  GEN_ADDR(CodeBlob_vtbl);
  GEN_ADDR(BufferBlob_vtbl);
  GEN_ADDR(RuntimeStub_vtbl);
  printf("\n");

  GEN_ADDR(I2CAdapter_vtbl);
  printf("\n");

  GEN_ADDR(Universe_methodKlassObj_address);
  GEN_ADDR(CodeCache_heap_address);
  printf("\n");

  printf("/* Volatiles */\n");
  GEN_ADDR(Universe_methodKlassObj);
  GEN_ADDR(CodeCache_low);
  GEN_ADDR(CodeCache_high);
  GEN_ADDR(CodeCache_segmap_low);
  GEN_ADDR(CodeCache_segmap_high);
  printf("\n");

  return;
}
