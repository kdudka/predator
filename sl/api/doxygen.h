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
 * - garbage collector: collectJunk(), destroyRootAndCollectJunk(), LeakMonitor
 *
 * - generic join algorithm: joinSymHeaps(), joinDataReadOnly(), joinData()
 *
 * - list segment discovery: discoverBestAbstraction()
 *
 * - list segment abstraction: abstractIfNeeded()
 *
 * - list segment concretization: concretizeObj(), spliceOutAbstractPath()
 *
 * - symbolic state: SymState, SymHeapUnion, SymStateWithJoin
 *
 * - function call management: SymBackTrace, symcut.hh, SymCallCache, SymCallCtx
 *
 * - symbolic execution: SymProc, SymExecCore, execute()
 *
 * - symbolic heap plotter: symplot.hh, symdebug.hh
 *
 * - utilities: symutil.hh, symseg.hh
 *
 * - built-in dispatcher: handleBuiltIn()
 *
 * - built-in header: sl.h
 *
 * - configuration: config.h
 */
