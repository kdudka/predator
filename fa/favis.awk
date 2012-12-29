#!/usr/bin/awk -f
# favis.awk: transforms a forest automaton into a dot file
#
BEGIN {
  print "digraph  \"forest automaton\" {";
  print "  label=<<FONT POINT-SIZE=\"18\">forest automaton</FONT>>;";
  print "  clusterrank=local;";
  print "  labelloc=t;";
  print "";
}

/^</ { # transitions
  gsub(" ", "");
  lBrackPos = index($0, ">");
  if (0 == lBrackPos) {
    printf("%s:%d: invalid format: %s\n", FILENAME, FNR, $0) > "/dev/stderr";
    exit 1;
  }

  transition = substr($0, 2, lBrackPos - 2);
  rest = substr($0, lBrackPos + 1);
  if (substr(rest, 1, 1) == "(") { # for non-leaf transitions
    lBrackPos = index(rest, ")");
    if (0 == lBrackPos) {
      printf("%s:%d: invalid format: %s\n", FILENAME, FNR, $0) > "/dev/stderr";
      exit 1;
    }
    children = substr(rest, 2, lBrackPos - 2);
    rest = substr(rest, lBrackPos);

    if (")->" != substr(rest, 1, 3)) {
      printf("%s:%d: invalid format: %s\n", FILENAME, FNR, $0) > "/dev/stderr";
      exit 1;
    }

    parent = substr(rest, 4);
  }
  else { # for leaf transitions
    children = "";

    if ("->" != substr(rest, 1, 2)) {
      printf("%s:%d: invalid format: %s\n", FILENAME, FNR, $0) > "/dev/stderr";
      exit 1;
    }

    parent = substr(rest, 3);
  }

  print "Parent: " parent ", transition: " transition ", children: " children;
}

END {
  print "}";
}
