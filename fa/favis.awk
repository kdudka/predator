#!/usr/bin/awk -f
# favis.awk: transforms a forest automaton into a dot file
#
BEGIN {
  print "digraph  \"forest automaton\" {";
  print "  label=<<FONT POINT-SIZE=\"18\">forest automaton</FONT>>;";
  print "  clusterrank=local;";
  print "  labelloc=t;";
  print "";

  cntTrans = 0;
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

    childNum = split(children, childArr, ",");
    selNum = split(transition, selArr, ",") - 1;
    if ((selNum <= 0) || childNum <= 0) {
      printf("%s:%d: invalid format: %s\n", FILENAME, FNR, $0) > "/dev/stderr";
      exit 1;
    }
    
    # take the node name out
    nodeName = selArr[1];
    for (i = 1; i <= selNum; ++i) {
      selArr[i] = selArr[i + 1];
    }
  }
  else { # for leaf transitions
    childNum = 0;
    children = "";
    for (val in childArr) {
      delete childArr[val];
    }

    selNum = 0;
    for (val in selArr) {
      delete selArr[val];
    }

    if ("->" != substr(rest, 1, 2)) {
      printf("%s:%d: invalid format: %s\n", FILENAME, FNR, $0) > "/dev/stderr";
      exit 1;
    }

    nodeName = transition;

    parent = substr(rest, 3);
  }

  if (selNum != childNum) {
    printf("%s:%d: selectors and children do not match: %s\n", FILENAME, FNR, $0) > "/dev/stderr";
    exit 1;
  }

  print "  subgraph \"cluster_trans_" cntTrans "\" {";
  print "    rank=same;";
  print "    label=\"trans_" cntTrans "\";";
  print "    labeljust=left;";
  print "    color=black;";
  print "    fontcolor=black;";
  print "    bgcolor=orange;";
  print "    style=dashed;";
  print "    penwidth=1.0;";
  print "";

  if (childNum > 0) {    # for non-leafs
    print "    \"" parent "\" [shape=ellipse, style=filled, fillcolor=lightblue, label=\"" nodeName "\"];";
  }
  else { # for leaves
    print "    \"" parent "\" [shape=box, style=filled, fillcolor=red, label=\"" nodeName "\"];";
  }

  for (i = 1; i <= childNum; ++i) {
    childName = "trans_" cntTrans "_" childArr[i];
    print "    \"" childName "\" [shape=box, style=filled, fillcolor=pink, label=\"" selArr[i] "\"];";
    print "    \"" parent "\" -> \"" childName "\" [label=\""  "\"];";
    print "    \"" childName "\" -> \"" childArr[i] "\";";
  }

  print "  }";
  print "";

  ++cntTrans;
}

END {
  print "}";
}
