#ifndef H_GUARD_CLD_UNIREGS_H
#define H_GUARD_CLD_UNIREGS_H

#include "cl_private.hh"    // for enum cl_scope_e

/**
 * create ICgListener implementation (TODO: document)
 * @return on heap allocated instance of ICgListener object
 */
ICodeListener* createCldUniRegs(ICodeListener *);

#endif /* H_GUARD_CLD_UNIREGS_H */
