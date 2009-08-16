#ifndef H_GUARD_CLD_UNILABEL_H
#define H_GUARD_CLD_UNILABEL_H

#include "cl_private.hh"    // for enum cl_scope_e

/**
 * create ICgListener implementation (TODO: document)
 * @return on heap allocated instance of ICgListener object
 */
ICodeListener* createCldUniLabel(ICodeListener *, cl_scope_e scope);

#endif /* H_GUARD_CLD_UNILABEL_H */
