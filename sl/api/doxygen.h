// This file contains no code inside

/**
 * @file doxygen.h
 * @brief Doxygen documentation (common part)
 */
 
/**
 * @mainpage
 * @author Kamil Dudka <kdudka@redhat.com>
 * @version @include version.h
 * @note The @b up2date @b sources are available (among other ways) via
 * <A HREF="http://github.com/kdudka/predator"><B>github</B></A>.
 *
 * @remark
 *
 * - symbolic heap: symid.hh, SymHeapCore, SymHeap
 *
 * - garbage collector: collectJunk(), destroyRootAndCollectJunk()
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
