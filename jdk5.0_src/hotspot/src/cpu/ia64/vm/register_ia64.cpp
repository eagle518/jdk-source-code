#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)register_ia64.cpp	1.6 03/12/23 16:36:48 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_register_ia64.cpp.incl"


const char* RegisterImpl::name() const {
  const char* names[number_of_registers] = {
	"gr0",   "gp",    "gr2",   "gr3",   "gr4",   "gr5",   "gr6",   "gr7",
	"gr8",   "gr9",   "gr10",  "gr11",  "sp",    "tp",    "gr14",  "gr15",
	"gr16",  "gr17",  "gr18",  "gr19",  "gr20",  "gr21",  "gr22",  "gr23",
	"gr24",  "gr25",  "gr26",  "gr27",  "gr28",  "gr29",  "gr30",  "gr31",
	// Stacked/rotating
	"gr32",  "gr33",  "gr34",  "gr35",  "gr36",  "gr37",  "gr38",  "gr39",
	"gr40",  "gr41",  "gr42",  "gr43",  "gr44",  "gr45",  "gr46",  "gr47",
	"gr48",  "gr49",  "gr50",  "gr51",  "gr52",  "gr53",  "gr54",  "gr55",
	"gr56",  "gr57",  "gr58",  "gr59",  "gr60",  "gr61",  "gr62",  "gr63",
	"gr64",  "gr65",  "gr66",  "gr67",  "gr68",  "gr69",  "gr70",  "gr71",
	"gr72",  "gr73",  "gr74",  "gr75",  "gr76",  "gr77",  "gr78",  "gr79",
	"gr80",  "gr81",  "gr82",  "gr83",  "gr84",  "gr85",  "gr86",  "gr87",
	"gr88",  "gr89",  "gr90",  "gr91",  "gr92",  "gr93",  "gr94",  "gr95",
	"gr96",  "gr97",  "gr98",  "gr99",  "gr100", "gr101", "gr102", "gr103",
	"gr104", "gr105", "gr106", "gr107", "gr108", "gr109", "gr110", "gr111",
	"gr112", "gr113", "gr114", "gr115", "gr116", "gr117", "gr118", "gr119",
	"gr124", "gr125", "gr126", "gr127", "gr128", "gr129", "gr130", "gr131"
	};
  return is_valid() ? names[encoding()] : "noreg";
}

const char* FloatRegisterImpl::name() const {
  const char* names[number_of_registers] = {
      "fr0",   "fr1",   "fr2",   "fr3",   "fr4",   "fr5",   "fr6",   "fr7",
      "fr8",   "fr9",   "fr10",  "fr11",  "fr12", "fr13",   "fr14",  "fr15",
      "fr16",  "fr17",  "fr18",  "fr19",  "fr20",  "fr21",  "fr22",  "fr23",
      "fr24",  "fr25",  "fr26",  "fr27",  "fr28",  "fr29",  "fr30",  "fr31",
      // Stacked/Rotating
      "fr32",  "fr33",  "fr34",  "fr35",  "fr36",  "fr37",  "fr38",  "fr39",
      "fr40",  "fr41",  "fr42",  "fr43",  "fr44",  "fr45",  "fr46",  "fr47",
      "fr48",  "fr49",  "fr50",  "fr51",  "fr52",  "fr53",  "fr54",  "fr55",
      "fr56",  "fr57",  "fr58",  "fr59",  "fr60",  "fr61",  "fr62",  "fr63",
      "fr64",  "fr65",  "fr66",  "fr67",  "fr68",  "fr69",  "fr70",  "fr71",
      "fr72",  "fr73",  "fr74",  "fr75",  "fr76",  "fr77",  "fr78",  "fr79",
      "fr80",  "fr81",  "fr82",  "fr83",  "fr84",  "fr85",  "fr86",  "fr87",
      "fr88",  "fr89",  "fr90",  "fr91",  "fr92",  "fr93",  "fr94",  "fr95",
      "fr96",  "fr97",  "fr98",  "fr99",  "fr100", "fr101", "fr102", "fr103",
      "fr104", "fr105", "fr106", "fr107", "fr108", "fr109", "fr110", "fr111",
      "fr112", "fr113", "fr114", "fr115", "fr116", "fr117", "fr118", "fr119",
      "fr124", "fr125", "fr126", "fr127", "fr128", "fr129", "fr130", "fr131"
  };
  return is_valid() ? names[encoding()] : "noreg";
}

const char* PredicateRegisterImpl::name() const {
  const char* names[number_of_registers] = {
      "pr0",   "pr1",   "pr2",   "pr3",   "pr4",   "pr5",   "pr6",   "pr7",
      "pr8",   "pr9",   "pr10",  "pr11",  "pr12", "pr13",   "pr14",  "pr15",
      "pr16",  "pr17",  "pr18",  "pr19",  "pr20",  "pr21",  "pr22",  "pr23",
      "pr24",  "pr25",  "pr26",  "pr27",  "pr28",  "pr29",  "pr30",  "pr31",
      "pr32",  "pr33",  "pr34",  "pr35",  "pr36",  "pr37",  "pr38",  "pr39",
      "pr40",  "pr41",  "pr42",  "pr43",  "pr44",  "pr45",  "pr46",  "pr47",
      "pr48",  "pr49",  "pr50",  "pr51",  "pr52",  "pr53",  "pr54",  "pr55",
      "pr56",  "pr57",  "pr58",  "pr59",  "pr60",  "pr61",  "pr62",  "pr63"
  };
  return is_valid() ? names[encoding()] : "noreg";
}

const char* BranchRegisterImpl::name() const {
  const char* names[number_of_registers] = {
      "br0",   "br1",   "br2",   "br3",   "br4",   "br5",   "br6",   "br7"
  };
  return is_valid() ? names[encoding()] : "noreg";
}

const char* ApplicationRegisterImpl::name() const {
  const char* names[number_of_registers] = {
      "ar0",   "ar1",   "ar2",   "ar3",   "ar4",   "ar5",   "ar6",   "ar7",
      "ar8",   "ar9",   "ar10",  "ar11",  "ar12", "ar13",   "ar14",  "ar15",
      "rsc",   "bsp",   "bspstore",
                                 "rnat",  "ar20",  "ar21",  "ar22",  "ar23",
      "ar24",  "ar25",  "ar26",  "ar27",  "ar28",  "ar29",  "ar30",  "ar31",
      "ccv",   "ar33",  "ar34",  "ar35",  "unat",  "ar37",  "ar38",  "ar39",
      "fpsr",  "ar41",  "ar42",  "ar43",  "ar44",  "ar45",  "ar46",  "ar47",
      "ar48",  "ar49",  "ar50",  "ar51",  "ar52",  "ar53",  "ar54",  "ar55",
      "ar56",  "ar57",  "ar58",  "ar59",  "ar60",  "ar61",  "ar62",  "ar63",
      "pfs",   "lc",    "ec",    "ar67",  "ar68",  "ar69",  "ar70",  "ar71",
      "ar72",  "ar73",  "ar74",  "ar75",  "ar76",  "ar77",  "ar78",  "ar79",
      "ar80",  "ar81",  "ar82",  "ar83",  "ar84",  "ar85",  "ar86",  "ar87",
      "ar88",  "ar89",  "ar90",  "ar91",  "ar92",  "ar93",  "ar94",  "ar95",
      "ar96",  "ar97",  "ar98",  "ar99",  "ar100", "ar101", "ar102", "ar103",
      "ar104", "ar105", "ar106", "ar107", "ar108", "ar109", "ar110", "ar111",
      "ar112", "ar113", "ar114", "ar115", "ar116", "ar117", "ar118", "ar119",
      "ar124", "ar125", "ar126", "ar127", "ar128", "ar129", "ar130", "ar131"
  };
  return is_valid() ? names[encoding()] : "noreg";
}
