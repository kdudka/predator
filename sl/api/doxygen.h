// This file contains no code inside

/**
 * @file doxygen.h
 * @brief Doxygen documentation (common part)
 */
 
/**
 * @mainpage
 * @author Kamil Dudka <kdudka@redhat.com>
 * @version @include version.h
 * @note The @b up2date @b sources are available (among other ways) through a <A
 * HREF="http://www.stud.fit.vutbr.cz/~xdudka00/cgi-bin/gitweb.cgi?p=tools">
 * <B>gitweb interface</B></A>.
 *
 * @remark
 *
 * - Documentation of code_listener.h might be a viable start. Reading this
 *   should be sufficient if you are going to write your own compiler plug-in
 *   or whatever to read the intermediate code from.
 *
 * - If you want to write your own code listener, look at the ICodeListener
 *   interface. And if you need to write a new code listener decorator,
 *   ClDecoratorBase abstract class is the place to go.
 *
 * - If you prefer to work with a traversable program model instead of callback
 *   sequence, you should start with CodeStorage namespace documentation, in
 *   particular the CodeStorage::Storage class.  Using this approach all
 *   necessary data are gathered into on heap allocated object model and then
 *   available for various types of analysis.
 *
 * - If you are interested in @b symbolic @b execution, see SymHeap,
 *   SymHeapUnion, SymHeapProcessor and SymExec classes and the symid.hh header.
 *
 * - If you need some equipment for debugging of the symbolic execution, look at
 *   symdump.hh and the class SymHeapPlotter.
 *
 * - If you are interested in participation on this project and don't know where
 *   to start, you can pick one of the outstanding <A
 *   HREF="todo.html"><B>todo-list</B></A> items.
 */
