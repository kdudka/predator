#ifndef H_GUARD_CL_PP_H
#define H_GUARD_CL_PP_H

struct ICodeListener;

/**
 * create ICgListener implementation (TODO: document)
 * @return on heap allocated instance of ICgListener object
 */
ICodeListener* createClPrettyPrint(int fd_out);

#endif /* H_GUARD_CL_PP_H */
