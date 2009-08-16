#ifndef H_GUARD_CL_LOCATOR_H
#define H_GUARD_CL_LOCATOR_H

struct ICodeListener;

/**
 * create ICgListener implementation (TODO: document)
 * @return on heap allocated instance of ICgListener object
 */
ICodeListener* createClLocator(int fd_out);

#endif /* H_GUARD_CL_LOCATOR_H */
