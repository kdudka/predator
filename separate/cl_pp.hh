#ifndef H_GUARD_CL_PP_H
#define H_GUARD_CL_PP_H

#include <cstdio>

struct ICodeListener;

/**
 * create ICgListener implementation (TODO: document)
 * @param output file to write to (as declared in <cstdio>)
 * @return on heap allocated instance of ICgListener object
 */
ICodeListener* createClPrettyPrint(FILE *output);

#endif /* H_GUARD_CL_PP_H */
