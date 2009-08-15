#ifndef H_GUARD_CLD_ARGSUB_H
#define H_GUARD_CLD_ARGSUB_H

#include "cl_private.hh"    // for enum cl_scope_e

/**
 * create ICgListener implementation (TODO: document)
 * @return on heap allocated instance of ICgListener object
 */
ICodeListener* createCldArgSubst(ICodeListener *);

#endif /* H_GUARD_CLD_ARGSUB_H */
