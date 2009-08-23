#include "config.h"

#include "cl_factory.hh"
#include "cl_locator.hh"
#include "cl_pp.hh"

#include "cld_argsub.hh"
#include "cld_intchk.hh"
#include "cld_unilabel.hh"
#include "cld_uniregs.hh"

#include <ctype.h>
#include <map>
#include <string>
#include <vector>

#ifndef CL_DEBUG_CLD
#   define CL_DEBUG_CLD 0
#endif

using std::string;

namespace {
    typedef std::vector<string>                         TStringList;
    typedef std::map<string, string>                    TStringMap;

    inline bool isspaceWrap(char c) {
        return isspace(static_cast<unsigned char>(c));
    }

    bool parseCldString(TStringList &dst, const string &cldString) {
        enum {
            S_INIT,
            S_READING_CLD,
            S_READING_COMMA
        } state = S_INIT;

        string cld;
        for (const char *s = cldString.c_str(); *s; ++s) {
            switch (state) {
                case S_INIT:
                    if (isspaceWrap(*s))
                        break;

                    state = S_READING_CLD;
                    // go through!!

                case S_READING_CLD:
                    if ((',' == *s) || isspaceWrap(*s)) {
                        if (cld.empty())
                            return false;

                        dst.push_back(cld);
                        cld.clear();
                    }

                    if (isspaceWrap(*s))
                        state = S_READING_COMMA;
                    else if (',' == *s)
                        state = S_INIT;
                    else
                        cld.push_back(*s);
                    break;

                case S_READING_COMMA:
                    if (isspaceWrap(*s))
                        break;

                    if (',' == *s)
                        state = S_INIT;
                    else
                        return false;
            }
        }

        if (!cld.empty())
            dst.push_back(cld);

        return true;
    }
    bool parseConfigString(TStringMap &dst, const char *config_string) {
        enum {
            S_INIT,
            S_READING_KEY,
            S_READING_KEY_DONE,
            S_READING_LQ,
            S_READING_VAL,
            S_READING_ESC
        } state = S_INIT;

        string key;
        string value;
        for (const char *s = config_string; *s; ++s) {
            switch (state) {
                case S_INIT:
                    if (isspaceWrap(*s))
                        break;

                    state = S_READING_KEY;
                    // go through!!

                case S_READING_KEY:
                    if (key.empty() && (('=' == *s) || isspaceWrap(*s)))
                        return false;

                    if (isspaceWrap(*s))
                        state = S_READING_KEY_DONE;
                    else if ('=' == *s)
                        state = S_READING_LQ;
                    else
                        key.push_back(*s);
                    break;

                case S_READING_KEY_DONE:
                    if (isspaceWrap(*s))
                        break;

                    if ('=' == *s)
                        state = S_READING_LQ;
                    else
                        return false;

                case S_READING_LQ:
                    if (isspaceWrap(*s))
                        break;

                    if ('"' == *s)
                        state = S_READING_VAL;
                    else
                        return false;
                    break;

                case S_READING_VAL:
                    if ('"' == *s) {
                        // TODO: check key redef?
                        dst[key] = value;
                        key.clear();
                        value.clear();
                        state = S_INIT;
                    } else if ('\\' == *s) {
                        state = S_READING_ESC;
                    } else  {
                        value.push_back(*s);
                    }
                    break;

                case S_READING_ESC:
                    if ('"' == *s)
                        value.push_back('"');
                    else if ('\\' == *s)
                        value.push_back('\\');
                    else
                        return false;
                    state = S_READING_VAL;
                    break;

            }
        }
        return (key.empty() && value.empty());
    }

    template <typename TMap>
    bool hasKey(const TMap &map, const typename TMap::mapped_type &key) {
        return map.end() != map.find(key);
    }
}

class CldChainFactory {
    public:
        CldChainFactory();
        ICodeListener* create(const std::string &cldString,
                              ICodeListener *slave);

    private:
        typedef ICodeListener* (*TCreateFnc)(ICodeListener *);
        typedef std::map<string, TCreateFnc> TMap;
        TMap map_;
};

// /////////////////////////////////////////////////////////////////////////////
// CldChainFactory implementation
namespace {
    ICodeListener* createCldUniLabelGl(ICodeListener *slave) {
        return createCldUniLabel(slave, CL_SCOPE_GLOBAL);
    }

    ICodeListener* createCldUniLabelStatic(ICodeListener *slave) {
        return createCldUniLabel(slave, CL_SCOPE_STATIC);
    }

    ICodeListener* createCldUniLabelFnc(ICodeListener *slave) {
        return createCldUniLabel(slave, CL_SCOPE_FUNCTION);
    }

}

CldChainFactory::CldChainFactory() {
    map_["unify_labels_gl"]         = createCldUniLabelGl;
    map_["unify_labels_static"]     = createCldUniLabelStatic;
    map_["unify_labels_fnc"]        = createCldUniLabelFnc;
    map_["unify_regs"]              = createCldUniRegs;
    map_["arg_subst"]               = createCldArgSubst;
}

ICodeListener* CldChainFactory::create(const std::string &cldString,
                                       ICodeListener *slave)
{
    CL_DEBUG("CldChainFactory: cldString: " << cldString);
    TStringList cldList;
    if (!parseCldString(cldList, cldString)) {
        CL_INTERNAL_ERROR("ivalid cld= option");
        return 0;
    }

    ICodeListener *chain = slave;

    TStringList::reverse_iterator i;
    for (i = cldList.rbegin(); chain && (i != cldList.rend()); ++i) {
        const string &cld = *i;
        CL_DEBUG("CldChainFactory: looking for decorator: " << cld);
        TMap::iterator i = map_.find(cld);
        if (i == map_.end()) {
            CL_INTERNAL_ERROR("code_listener decorator not found: " << cld);
            return 0;
        }

#if CL_DEBUG_CLD
        chain = createCldIntegrityChk(chain);
        if (chain)
            CL_DEBUG("CldChainFactory: integrity checker for '" << cld
                    << "' created successfully");
        else
            return 0;
#endif

        chain = (i->second)(chain);
        if (chain)
            CL_DEBUG("CldChainFactory: decorator '" << cld
                    << "' created successfully");
    }

    return chain;
}

// /////////////////////////////////////////////////////////////////////////////
// ClFactory implementation
struct ClFactory::Private {
    typedef ICodeListener* (*TCreateFnc)
        (/* TODO: pass args to listener's factory */);
    typedef std::map<string, TCreateFnc>                TMap;

    TMap                            map;
    CldChainFactory                 cldFactory;
};

ClFactory::ClFactory():
    d(new Private)
{
    d->map["pp"]        = &createClPrettyPrint;
    d->map["locator"]   = &createClLocator;
}

ClFactory::~ClFactory() {
    delete d;
}

ICodeListener* ClFactory::create(const char *config_string) {
    CL_DEBUG("ClFactory: config_string: " << config_string);

    TStringMap args;
    if (!parseConfigString(args, config_string)) {
        CL_INTERNAL_ERROR("invalid config_string given to ClFactory");
        return 0;
    }
    if (!hasKey(args, "listener")) {
        CL_INTERNAL_ERROR("no listener= option given to ClFactory");
        return 0;
    }

    string name = args["listener"];
    CL_DEBUG("ClFactory: looking for listener: " << name);

    Private::TMap::iterator i = d->map.find(name);
    if (i == d->map.end()) {
        CL_INTERNAL_ERROR("listener not found: " << name);
        return 0;
    }

    ICodeListener *cl = (i->second)
        (/* TODO: pass args to listener's constructor */);
    if (!cl)
        return 0;
    CL_DEBUG("ClFactory: listener '" << name << "' successfully created");

    if (hasKey(args, "cld")) {
        cl = d->cldFactory.create(args["cld"], cl);
        if (!cl)
            return 0;
    }

    cl = createCldIntegrityChk(cl);
    if (cl)
        CL_DEBUG("ClFactory: createCldIntegrityChk() completed successfully");

    return cl;
}
