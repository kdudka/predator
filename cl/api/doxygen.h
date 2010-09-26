// This file contains no code inside

/**
 * @file doxygen.h
 * @brief Doxygen documentation (common part)
 */
 
/**
 * @mainpage
 * @author Kamil Dudka <kdudka@redhat.com>
 * @version @include version_cl.h
 * @note The @b up2date @b sources are available (among other ways) through
 * <A HREF="http://github.com/kdudka/predator"><B>github</B></A>.
 *
 * @remark
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
 * - decorators:
 *      - cld_intchk.hh
 *      - cld_unilabel.hh
 *      - cld_univars.hh
 *      - cld_unswitch.hh
 *
 * - error/warning messages: cl_msg.hh, location.hh,
 *      - Location
 *      - LocationWriter
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
 * - internals: cl.hh, cl_decorator.hh, cl_factory.hh
 *      - ICodeListener
 *      - ClDecoratorBase
 *      - ClFactory
 *
 * - configuration: config_cl.h
 */
