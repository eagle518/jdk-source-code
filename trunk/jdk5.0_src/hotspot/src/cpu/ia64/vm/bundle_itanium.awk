# @(#)bundle_itanium.awk	1.11 03/01/23 10:56:29 JVM
#
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
#

BEGIN {
  if (debug == "")
    debug = 0;
  s = 0;
  p = 2;

  # The following are the supported bundle formats for
  # the itanium processor
  seq[0]   = "";
  seq[++s] = "M|M I";
  seq[++s] = "M I|I";
  seq[++s] = "M I I";
  seq[++s] = "M L X";
  seq[++s] = "M M I";
  seq[++s] = "M F I";
  seq[++s] = "M M F";
  seq[++s] = "M I B";
  seq[++s] = "M B B";
  seq[++s] = "B B B";
  seq[++s] = "M M B";
  seq[++s] = "M F B";

  # The following are the maximum number of instructions
  # for the itanium processor
  max_a = 4;  # A-instructions
  max_m = 2;  # M-instructions
  max_i = 2;  # I-instructions
  max_f = 2;  # F-instructions
  max_b = 3;  # B-instructions
  max_l = 2;  # L-instructions

  # Iterate over all the combinations of usages, and see if
  # there are any bundles or pairs of bundles that can implement
  # this combination
  for (unit_l = 0; unit_l <= max_l; unit_l++) {
    for (unit_b = 0; unit_b <= max_b; unit_b++) {
      for (unit_f = 0; unit_f <= max_f; unit_f++) {
        for (unit_i = 0; unit_i <= max_i; unit_i++) {
          for (unit_m = 0; unit_m <= max_m; unit_m++) {
            for (unit_a = 0; unit_a <= max_a; unit_a++) {

              # Generate a unique index for looking up the value
              ndx = unit_a + (max_a+1) * (unit_m + (max_m+1) * (unit_i + (max_i+1) * (unit_f + (max_f+1) * (unit_b + (max_b+1) * unit_l))));

	      if (debug) printf "L:%d B:%d F:%d I:%d M:%d A:%d ndx=%d\n", unit_l, unit_b, unit_f, unit_i, unit_m, unit_a, ndx

              # default is 0
              choice_len [ndx,1] = 0;
              partial_len[ndx,1] = 0;

              # 0 case is no usage at all, just give up
              if (ndx == 0)
                continue;

              # Check to see if the sum of the I, A, and M instructions exceeds the
              # total number of functional units
              if (unit_a + unit_m + unit_i > max_m + max_i)
                continue;

              s2 = s;

              # See if there is a single bundle that can
              # implement the usage. If so, then don't use
              # any multiple bundle combinations
              for (i1 = 1; i1 <= s; i1++) {
                try = seq[i1];
                if (will_work(try) != 0) {
                  if (debug) printf "found %s works\n", try;
                  s2 = 0;
                  break;
                }
              }

              # Iterate over all legal bundle formats. Note that
              # 0 indicates no bundle.
              for (i1 = 1; i1 <= s; i1++) {
                for (i2 = 0; i2 <= s2; i2++) {
                  try = seq[i1];
                  if ( i2 > 0 )
                    try = try "  " seq[i2];

                  # If the 1st bundle has a forced split, then we cannot
                  # use a 2 bundle combination
                  if (i2 > 0 && index(seq[i1], "|") > 0)
                    continue;

                  # These bundles stall after the 1st bundle
                  if (i2 > 0 && seq[i1] == "B B B")
                    continue;
                  if (i2 > 0 && seq[i1] == "M B B")
                    continue;
                  if (i2 > 0 && seq[i1] == "M M F")
                    continue;

                  # This bundle stalls before and after the "M M F" bundle
                  if (seq[i2] == "M M F")
                    continue;

                  # Count the number of branch instruction positions
                  bs = 0;
                  for (b1 = 1; b1 <= length(try); b1++)
                    if (substr(try,b1,1) == "B") bs++;

                  # If the first bundle ends with a "B", and is not a
                  # nop, then a stall will happen
                  if (i2 > 0 && seq[i1] == "M I B" && unit_b == bs)
                    continue;
                  if (i2 > 0 && seq[i1] == "M F B" && unit_b == bs)
                    continue;
                  if (i2 > 0 && seq[i1] == "M M B" && unit_b == bs)
                    continue;

                  # See if this meets the constraints
                  if (!will_work(try))
                    continue;

                  new_len = s2 > 0 ? 2 : 1;

                  # See if the last bundle has a forced split
                  partial = index(try, "|");

                  # Has a forced split, record it 
                  if (partial > 0) {
                    if (partial_len[ndx, 0] == 0 || partial_len[ndx, 0] > new_len ||
                        ( partial_len[ndx, 0] == new_len &&
                          partial < index(seq[i1] "  " seq[i2], "|") ) ) {
                      partial_len[ndx, 0] = new_len;
                      partial_val[ndx, 0, 1] = i1;
                      partial_val[ndx, 0, 2] = i2;
                    }
                  }
   
                  # Has no forced split, record it. Shorter numbers of bundles are better.
                  else if (choice_len[ndx, 0] == 0 || choice_len[ndx, 0] > new_len) {
                    choice_len[ndx, 0] = new_len;
                    choice_val[ndx, 0, 1] = i1;
                    choice_val[ndx, 0, 2] = i2;
                  }
                }
              }

              if( partial_len[ndx, 0] > 0 ) {
                if( partial_len[ndx, 0] == 1 ) {
                  x = seq[partial_val[ndx, 0, 1]];
                  i1 = index(x,"|");
                  x = substr(x,1,i1-1) " " substr(x,i1+1);

                  for (i1 = 1; i1 <= s; i1++) {
                    if (seq[i1] == x) {
                      choice_val[ndx, 0, 1] = i1;
                      break;
                    }
                  }
                }

                else if( partial_len[ndx, 0] == 2 ) {
                  x = seq[partial_val[ndx, 0, 2]];
                  i1 = index(x,"|");
                  x = substr(x,1,i1-1) " " substr(x,i1+1);

                  for (i1 = 1; i1 <= s; i1++) {
                    if (seq[i1] == x) {
                      choice_val[ndx, 0, 1] = partial_val[ndx, 0, 1];
                      choice_val[ndx, 0, 2] = i1;
                      break;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  # Do it again, but this time only choose first bundles that
  # are split.
  for (i1 = 1; i1 <= s; i1++) {
    ndx = index(seq[i1],"|");
    if (ndx == 0)
       continue;
    pre  = substr(seq[i1], 1, ndx);
    post = substr(seq[i1], ndx+1);

    for (unit_l = 0; unit_l <=  max_l; unit_l++) {
      for (unit_b = 0; unit_b <= max_b; unit_b++) {
        for (unit_f = 0; unit_f <= max_f; unit_f++) {
          for (unit_i = 0; unit_i <= max_i; unit_i++) {
            for (unit_m = 0; unit_m <= max_m; unit_m++) {
              for (unit_a = 0; unit_a <= max_a; unit_a++) {

                ndx = unit_a + (max_a+1) * (unit_m + (max_m+1) * (unit_i + (max_i+1) * (unit_f + (max_f+1) * (unit_b + (max_b+1) * unit_l))));

	        if (debug) printf "L:%d B:%d F:%d I:%d M:%d A:%d ndx=%d\n", unit_l, unit_b, unit_f, unit_i, unit_m, unit_a, ndx

                if (ndx == 0)
                  continue;

                if (unit_a + unit_m + unit_i > max_m + max_i)
                  continue;

                s2 = s;

                try = post;

                if (doesnt_help(try)) {
                  if (debug) printf "found %s doesn't help\n", try;
                  s2 = -1;
                }
                else if (will_work(try) != 0) {
                  if (debug) printf "found %s works\n", try;
                  s2 = 0;
                }

                for (i2 = 0; i2 <= s2; i2++) {
                  try = post;
                  if ( i2 > 0 )
                    try = try "  " seq[i2];

                  if (seq[i2] == "M M F")
                    continue;

                  bs = 0;
                  for (b1 = 1; b1 <= length(try); b1++)
                    if (substr(try,b1,1) == "B") bs++;

                  if (!will_work(try))
                    continue;

                  new_len = s2 > 0 ? 2 : 1;

                  partial = index(try, "|");

                  if (partial > 0) {
                    if (partial_len[ndx, i1] == 0 || partial_len[ndx, i1] > new_len ||
                        ( partial_len[ndx, i1] == new_len &&
                          partial < index(seq[i1] "  " seq[i2], "|") ) ) {
                      partial_len[ndx, i1] = new_len;
                      partial_val[ndx, i1, 1] = i1;
                      partial_val[ndx, i1, 2] = i2;
                    }
                  }
   
                  else if (choice_len[ndx, i1] == 0 || choice_len[ndx, i1] > new_len) {
                    choice_len[ndx, i1] = new_len;
                    choice_val[ndx, i1, 1] = i1;
                    choice_val[ndx, i1, 2] = i2;
                  }
                }

                if( partial_len[ndx, i1] == 2 ) {
                  x = seq[partial_val[ndx, i1, 2]];
                  j1 = index(x,"|");
                  x = substr(x,1,j1-1) " " substr(x,j1+1);

                  for (j1 = 1; j1 <= s; j1++) {
                    if (seq[j1] == x) {
                      choice_val[ndx, i1, 1] = partial_val[ndx, i1, 1];
                      choice_val[ndx, i1, 2] = j1;
                      break;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  printf "#ifdef USE_PRAGMA_IDENT_SRC\n"
  printf "#pragma ident \"%@(#)bundle_itanium.awk	1.11% %03/01/23% %10:56:29% JVM\"\n"
  printf "#endif\n"
  printf "/*\n"
  printf " * Copyright 1993-2002 Sun Microsystems, Inc.  All rights reserved.\n"
  printf " * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.\n"
  printf " */\n"

  max_a = 4;  # A-instructions
  max_m = 2;  # M-instructions
  max_i = 2;  # I-instructions
  max_f = 2;  # F-instructions
  max_b = 3;  # B-instructions
  max_l = 2;  # L-instructions
  # Emit a limit array for the maximum number of each kind
  # of instruction
  printf "\nstatic const uint8_t limit_table[Inst::Kind_count] = {\n";
  printf "  %d, // M_inst\n", max_m;
  printf "  %d, // F_inst\n", max_f;
  printf "  %d, // I_inst\n", max_i;
  printf "  %d, // B_inst\n", max_b;
  printf "  %d, // L_inst\n", max_l; # note that L and X have same count
  printf "  %d, // X_inst\n", max_l; # note that L and X have same count
  printf "  %d  // A_inst\n", max_a;
  printf "};\n";

  # Scan the solutions, and build up a list of all the
  # bundle choices found. Since the number of solutions is
  # small, and the size of the dataset is large, we will
  # generate an array which indexes a solution array to
  # save space

  solution_count = 0;

  # The 0 solution indicates no solution is possible
  solution_string[""] = solution_count;
  solutions[solution_count++] = "";
 
  # Only look for default and split cases
  for (i1 = 0; i1 <= s; i1++) {
    if (i1 > 0 && index(seq[i1],"|") == 0)
      continue;

    # The array is unchanging
    printf "\nstatic const uint8_t ";

    if (i1 == 0)
      printf "emit_table";
    else
      printf "emit_%s_table", seqname(i1);

    # Dimensionality is based on number of functional units
    printf "[%d][%d][%d][%d][%d][%d][2] = {\n", (max_l+1),(max_b+1),(max_f+1),(max_i+1),(max_m+1),(max_a+1);

    # Iterate over all the unit usage
    for (unit_l = 0; unit_l <=  max_l; unit_l++) {
      for (unit_b = 0; unit_b <= max_b; unit_b++) {
        for (unit_f = 0; unit_f <= max_f; unit_f++) {
          for (unit_i = 0; unit_i <= max_i; unit_i++) {
            for (unit_m = 0; unit_m <= max_m; unit_m++) {
              for (unit_a = 0; unit_a <= max_a; unit_a++) {

                # compute the index
                ndx = unit_a + (max_a+1) * (unit_m + (max_m+1) * (unit_i + (max_i+1) * (unit_f + (max_f+1) * (unit_b + (max_b+1) * unit_l))));

                # Level of "{" before
                before = 0;
                unit_a == 0 && ++before &&
                unit_m == 0 && ++before &&
                unit_i == 0 && ++before &&
                unit_f == 0 && ++before &&
                unit_b == 0 && ++before;

                # Level of "}" after
                after = 0;
                unit_a == max_a && ++after &&
                unit_m == max_m && ++after &&
                unit_i == max_i && ++after &&
                unit_f == max_f && ++after &&
                unit_b == max_b && ++after;

                # Print preceeding indents
                l = 0;

                # Compute the solution
                solution1 = "";

                if (choice_len[ndx, i1] > 0) {
                  sn1 = seqname(choice_val[ndx, i1, 1]);
                  solution1 = "IPF_Bundle::" sn1;
                  if (choice_len[ndx, i1] > 1)
                    solution1 = solution1 ", " substr("     ", 1, 5-length(sn1));
                  else
                    solution1 = solution1 "s" substr("     ", 1, 4-length(sn1));
                }

                if (choice_len[ndx, i1] > 1) {
                  sn2 = seqname(choice_val[ndx, i1, 2]);
                  solution1 = solution1 "IPF_Bundle::" sn2 "s" substr("     ", 1, 4-length(sn2));
                }

                solution1 = remove_trailing_blanks(solution1);

                if ( solution_string[solution1] == "" ) {
                  solution_string[solution1] = solution_count;
                  solutions[solution_count++] = solution1;
                }


                l = 0;
                solution2 = "";

                if (partial_len[ndx, i1] > 0) {
                  sn1 = seqname(partial_val[ndx, i1, 1]);
                  solution2 = "IPF_Bundle::" sn1;
                  if (partial_len[ndx, i1] > 1)
                    solution2 = solution2 ", " substr("     ", 1, 5-length(sn1));
                  else if (i1 == 0)
                    solution2 = solution2 "s" substr("     ", 1, 4-length(sn1));
                  else
                    solution2 = solution2 substr("     ", 1, 5-length(sn1));
                }

                if (partial_len[ndx, i1] > 1) {
                  sn2 = seqname(partial_val[ndx, i1, 2]);
                  solution2 = solution2 "IPF_Bundle::" sn2 "s" substr("     ", 1, 4-length(sn2));
                }

                solution2 = remove_trailing_blanks(solution2);

                if ( solution_string[solution2] == "" ) {
                  solution_string[solution2] = solution_count;
                  solutions[solution_count++] = solution2;
                }

                printf "  %5s", substr("{{{{{",1,before);

                # Print out the line from the array
                printf "{%3d,%3d }", solution_string[solution1], solution_string[solution2];

                # and the trailing comment
                printf "%-6s // %3d -> LX:%1d B:%1d F:%1d I:%1d M:%1d A:%1d\n",
                  substr("}}}}}",1,after) ",",
                  ndx, unit_l, unit_b, unit_f, unit_i, unit_m, unit_a;
              }
            }
          }
        }
      }
    }

    printf "};\n";
  }

  # Print out the actual solution list
  printf "\nstatic const EmitSpecifier solutions[%d] = {\n", solution_count;
  for (x = 0; x < solution_count; x++) {
    printf "  EmitSpecifier(%s)%s  %*s// %2d\n",
      solutions[x],
      (x < solution_count-1) ? "," : " ",
      40-length(solutions[x]), "",
      x;
  }
  printf "};\n";
}

function seqname(ndx, s, i) {
  s = seq[ndx];
  for (i = index(s, " "); i > 0; i = index(s, " "))
    s = substr(s,1,i-1) substr(s,i+1);
  for (i = index(s, "|"); i > 0; i = index(s, "|"))
    s = substr(s,1,i-1) "s" substr(s,i+1);
  return (s);
}

function will_work(s, ca, cm, ci, cf, cb, cl, cx, ua, um, ui, uf, ub, ul, ux, tc, t) {
  ca = cm = ci = cf = cb = cl = cx = 0;
  ua = unit_a; um = unit_m; ui = unit_i; uf = unit_f; ub = unit_b; ul = unit_l; ux = unit_x;

  # Count the available instruction slots
  for (t = 1; t <= length(s); t++) {
    tc = substr(s,t,1);
    if      (tc == "B") cb++;
    else if (tc == "F") cf++;
    else if (tc == "I") ci++;
    else if (tc == "M") cm++;
    else if (tc == "L") cl++;
    else if (tc == "X") cx++;
    else if (tc == "|") break;
  }

  for (t = 1; t <= ua; t++) {
    if (cm > um)
      um++;
    else if (ci > ui)
      ui++;
    else
      return 0;
  }

  # Verify that there are enough slots
  if (cl < ul) {
    if (debug) printf "*** %s: Not enough L units: %d < %d\n", s, cl, ul;
    return 0;
  }

  if (cb < ub) {
    if (debug) printf "*** %s: Not enough B units: %d < %d\n", s, cb, ub;
    return 0;
  }

  if (cf < uf) {
    if (debug) printf "*** %s: Not enough F units: %d < %d\n", s, cf, uf;
    return 0;
  }

  if (ci < ui) {
    if (debug) printf "*** %s: Not enough I units: %d < %d\n", s, ci, ui;
    return 0;
  }

  if (cm < um) {
    if (debug) printf "*** %s: Not enough M units: %d < %d\n", s, cm, um;
    return 0;
  }

  if (cl > max_l) {
    if (debug) printf "*** %s: Too many L units: %d > %d\n", s, cl, max_l;
    return 0;
  }

  if (cb > max_b) {
    if (debug) printf "*** %s: Too many B units: %d > %d\n", s, cb, max_b;
    return 0;
  }

  if (cf+cl > max_f) {
    if (debug) printf "*** %s: Too many F units: %d+%d > %d\n", s, cf, cl, max_f;
    return 0;
  }

  if (ci+cx > max_i) {
    if (debug) printf "*** %s: Too many I units: %d+%d > %d\n", s, ci, cx, max_i;
    return 0;
  }

  if (cm > max_m) {
    if (debug) printf "*** %s: Too many M units: %d > %d\n", s, cm, max_m;
    return 0;
  }

  return 1;
}

function doesnt_help(s, ca, cm, ci, cf, cb, cl, cx, ua, um, ui, uf, ub, ul, ux, tc, t) {
  ca = cm = ci = cf = cb = cl = cx = 0;
  ua = unit_a; um = unit_m; ui = unit_i; uf = unit_f; ub = unit_b; ul = unit_l; ux = unit_x;

  # Count the available instruction slots
  for (t = 1; t <= length(s); t++) {
    tc = substr(s,t,1);
    if      (tc == "B") cb++;
    else if (tc == "F") cf++;
    else if (tc == "I") ci++;
    else if (tc == "M") cm++;
    else if (tc == "L") cl++;
    else if (tc == "X") cx++;
    else if (tc == "|") break;
  }

  if ( unit_a > 0 && (cm > 0 || ci > 0) )
    return 0;

  if ( unit_m > 0 && cm > 0 )
    return 0;

  if ( unit_i > 0 && ci > 0 )
    return 0;

  return 1;
}

function remove_trailing_blanks(s,i) {
  for (i = length(s); i > 0; i--)
    if (substr(s,i,1) != " ")
      break;
  return substr(s,1,i);
}
