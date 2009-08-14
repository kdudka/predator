#ifndef H_GUARD_CL_FACTORY_H
#define H_GUARD_CL_FACTORY_H

#include "cl_private.hh"

#include <cstdio>

/**
 * factory responsible for creating ICodeListener for desired call graph format
 */
class ClFactory {
    public:
        ClFactory();
        ~ClFactory();

        /**
         * @param fmt format used to dump call graph data
         * @param file to write output to (as declared in <cstdio>)
         * @return on heap allocated ICodeListener object
         */
        ICodeListener* create(const char *fmt, FILE *output);

    private:
        struct Private;
        Private *d;
};

#endif /* H_GUARD_CL_FACTORY_H */
