/*
 * Copyright (c) 2003, 2006, Oracle and/or its affiliates. All rights reserved.
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

#ifndef _ELFMACROS_H_
#define _ELFMACROS_H_

#if defined(_LP64)
#define ELF_EHDR        Elf64_Ehdr
#define ELF_SHDR        Elf64_Shdr
#define ELF_PHDR        Elf64_Phdr
#define ELF_SYM         Elf64_Sym
#define ELF_NHDR        Elf64_Nhdr
#define ELF_DYN         Elf64_Dyn
#define ELF_ADDR        Elf64_Addr

#define ELF_ST_TYPE     ELF64_ST_TYPE

#else

#define ELF_EHDR        Elf32_Ehdr
#define ELF_SHDR        Elf32_Shdr
#define ELF_PHDR        Elf32_Phdr
#define ELF_SYM         Elf32_Sym
#define ELF_NHDR        Elf32_Nhdr
#define ELF_DYN         Elf32_Dyn
#define ELF_ADDR        Elf32_Addr

#define ELF_ST_TYPE     ELF32_ST_TYPE

#endif


#endif /* _ELFMACROS_H_ */
