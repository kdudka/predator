// This file contains no code inside

/**
 * @file doxygen.h
 * @brief Doxygen documentation (common part)
 */
 
/**
 * @mainpage
 * @note See the
 * <A HREF="http://www.fit.vutbr.cz/research/groups/verifit/tools/code-listener/"><B>project page</B></A>
 * for details about the <B>Code Listener</B> project, new releases, etc.
 * @version @include version_cl.h
 *
 * @remark
 *
 * - an entry point for @b easy analyzers: easy.hh
 *      - clEasyRun()
 *
 * - code listener API: code_listener.h
 *      - struct cl_operand
 *      - struct cl_type
 *      - struct cl_accessor
 *
 * - code storage API: storage.hh
 *      - CodeStorage::Storage
 *      - CodeStorage::Fnc
 *      - CodeStorage::ControlFlow
 *      - CodeStorage::Block
 *      - CodeStorage::Insn
 *      - CodeStorage::Var
 *
 * - listeners:
 *      - cl_pp.hh
 *      - cl_locator.hh
 *      - cl_dotgen.hh
 *      - cl_typedot.hh
 *      - cl_storage.hh
 *      - cl_easy.hh
 *
 * - filters:
 *      - clf_intchk.hh
 *      - clf_unilabel.hh
 *      - clf_unswitch.hh
 *
 * - error/warning messages: cl_msg.hh
 *      - #CL_ERROR, #CL_ERROR_MSG
 *      - #CL_WARN, #CL_WARN_MSG
 *      - #CL_NOTE, #CL_NOTE_MSG
 *      - #CL_DEBUG, #CL_DEBUG_MSG
 *
 * - other utilities: clutil.hh
 *      - targetTypeOfPtr()
 *      - seekRefAccessor()
 *      - intCstFromOperand()
 *      - varIdFromOperand()
 *      - traverseTypeIc()
 *
 * - internals: cl.hh, cl_filter.hh, cl_factory.hh
 *      - ICodeListener
 *      - ClFilterBase
 *      - ClFactory
 *
 * - configuration: config_cl.h
 */
