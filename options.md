## Predator plug-in options
For the GCC-plugin, use with the prefix `-fplugin-arg-libsl`. Default option in bold.

| Options             | Description                                 |
| ------------------- | ------------------------------------------- |
| `-help`             | Help                                        |
| `-verbose=<uint>`   | Turn on verbose mode                        |
| `-pid-file=<file>`  | Write PID of self to `<file>`               |
| `-preserve-ec`      | Do not affect the exit code                 |
| `-dry-run`          | Do not run the analysis                     |
| `-dump-pp[=<file>]` | Dump linearised CL code                     |
| `-dump-types`       | Dump also type info                         |
| `-gen-dot[=<file>]` | Generate CFGs                               |
| `-type-dot=<file>`  | Generate type graphs                        |
| `-args=<peer-args>` | Arguments given to the analyser (see below) |

| Peer arguments                  | Description |
| ------------------------------- | --- |
| `track_uninit`                  | Report usage of uninitialised values |
| `oom`                           | Simulate possible shortage of memory (`malloc` can fail) |
| `no_error_recovery`             | No error recovery, stop the analysis as soon as an error is detected |
| `memleak_is_error`              | Treat memory leaks as an error |
| `exit_leaks`                    | Report memory leaks while executing a no-return function |
| `verifier_error_is_error`       | Treat reaching of `__VERIFIER_error()` as an error |
| `error_label:<string>`          | Treat reaching of the given label as an error |
| `int_arithmetic_limit:<uint>`   | The highest integer number Predator can count to |
| `allow_cyclic_trace_graph`      | Create a node with two parents on entailment |
| `forbid_heap_replace`           | Do not replace a previously tracked node if en- tailed by a new one |
| `allow_three_way_join[:<uint>]` | Using the general join of possibly incomparable SMGs (so-called three-way join) <ol><li value="0">never</li> <li>only when joining nested sub-heaps</li> <li>also when joining SPCs if considered useful</li><b><li> always</li></b></ol> |
| `join_on_loop_edges_only[:<int>]` | <ol><li value="-1">never join, never check for entailment, always check for isomorphism</li> <li>join SPCs on each basic block entry</li><li>join only when traversing a loop-closing edge, entailment otherwise </li><li>join only when traversing a loop-closing edge, isomorphism otherwise</li><b><li>same as 2 but skips the isomorphism check if possible</li></b></ol> |
| `state_live_ordering[:<uint>]` | On the fly ordering of SPCs to be processed<ol><li value="0">do not try to optimise the order of heaps</li><li>reorder heaps when joining</li><b><li>reorder heaps when creating their union (list of SMGs) too</li></b></ol> |
| `no_plot` | Do not generate graphs (ignore all calls of `__sl_plot*()` and `__VERIFIER_plot()`) |
| `dump_fixed_point` | Dump SPCs of the obtained fixed-point |
| `detect_containers` | Detect low-level implementations of high-level list containers and operations over them (such as various initialisers, iterators, etc.) |
