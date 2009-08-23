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
         * @todo document
         */
        ICodeListener* create(const char *config_string);

    private:
        struct Private;
        Private *d;
};

#endif /* H_GUARD_CL_FACTORY_H */
