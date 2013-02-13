// This file contains no code inside

/**
 * @file doxygen.h
 * @brief Doxygen documentation (common part)
 */
 
/**
 * @mainpage
 * @note See the
 * <A HREF="http://www.fit.vutbr.cz/research/groups/verifit/tools/predator/"><B>project page</B></A>
 * for details about the <B>Predator</B> project, new releases, etc.
 * @version @include version.h
 *
 * @remark
 *
 * - symbolic heap: symid.hh, SymHeapCore, SymHeap
 *
 * - garbage collector: collectJunk(), destroyObjectAndCollectJunk(), LeakMonitor
 *
 * - generic join algorithm: joinSymHeaps(), joinData()
 *
 * - list segment discovery: discoverBestAbstraction()
 *
 * - list segment abstraction: abstractIfNeeded()
 *
 * - list segment concretization: concretizeObj(), spliceOutListSegment(), dlSegReplaceByConcrete()
 *
 * - symbolic state: SymState, SymHeapUnion, SymStateWithJoin
 *
 * - function call management: SymBackTrace, symcut.hh, SymCallCache, SymCallCtx
 *
 * - symbolic execution: SymProc, SymExecCore, execute()
 *
 * - symbolic heap plotter: symplot.hh, symdebug.hh
 *
 * - trace graph maintenance and visualization: Trace, Trace::Node
 *
 * - utilities: symutil.hh, symseg.hh
 *
 * - built-in dispatcher: handleBuiltIn()
 *
 * - configuration: config.h
 */
