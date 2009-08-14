#ifndef H_GUARD_CL_FACTORY_H
#define H_GUARD_CL_FACTORY_H

#include "cl_private.hh"

/**
 * factory responsible for creating ICodeListener for desired call graph format
 */
class ClFactory {
    public:
        ClFactory();
        ~ClFactory();

        /**
         * @param fmt format used to dump call graph data
         * @return on heap allocated ICodeListener object
         */
        ICodeListener* create(const char *fmt, int fd_out);

    private:
        struct Private;
        Private *d;
};

#endif /* H_GUARD_CL_FACTORY_H */
