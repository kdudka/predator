/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of predator.
 *
 * predator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * predator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with predator.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config_cl.h"

#include <cl/cl_msg.hh>

#include "cl_dotgen.hh"
#include "cl_easy.hh"
#include "cl_factory.hh"
#include "cl_locator.hh"
#include "cl_pp.hh"
#include "cl_typedot.hh"

#include "clf_intchk.hh"
#include "clf_unilabel.hh"
#include "clf_unswitch.hh"

#include "util.hh"

#include <ctype.h>
#include <map>
#include <string>
#include <vector>

#ifndef CL_DEBUG_CLF
#   define CL_DEBUG_CLF 0
#endif

#ifndef DEBUG_CL_FACTORY
#   define DEBUG_CL_FACTORY 0
#endif

#if DEBUG_CL_FACTORY
#   define CL_FACTORY_DEBUG(what) CL_DEBUG(what)
#else
#   define CL_FACTORY_DEBUG(what) (void) 0
#endif

using std::string;

namespace {
    typedef std::vector<string>                         TStringList;
    typedef std::map<string, string>                    TStringMap;

    inline bool isspaceWrap(char c) {
        return isspace(static_cast<unsigned char>(c));
    }

    bool parseClfString(TStringList &dst, const string &clfString) {
        enum {
            S_INIT,
            S_READING_CLF,
            S_READING_COMMA
        } state = S_INIT;

        string clf;
        for (const char *s = clfString.c_str(); *s; ++s) {
            switch (state) {
                case S_INIT:
                    if (isspaceWrap(*s))
                        break;

                    state = S_READING_CLF;
                    // go through!!

                case S_READING_CLF:
                    if ((',' == *s) || isspaceWrap(*s)) {
                        if (clf.empty())
                            return false;

                        dst.push_back(clf);
                        clf.clear();
                    }

                    if (isspaceWrap(*s))
                        state = S_READING_COMMA;
                    else if (',' == *s)
                        state = S_INIT;
                    else
                        clf.push_back(*s);
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

        if (!clf.empty())
            dst.push_back(clf);

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
}

class ClfChainFactory {
    public:
        ClfChainFactory();
        ICodeListener* create(const std::string &clfString,
                              ICodeListener *slave);

    private:
        typedef ICodeListener* (*TCreateFnc)(ICodeListener *);
        typedef std::map<string, TCreateFnc> TMap;
        TMap map_;
};

// /////////////////////////////////////////////////////////////////////////////
// ClfChainFactory implementation
namespace {
    ICodeListener* createClfUniLabelGl(ICodeListener *slave) {
        return createClfUniLabel(slave, CL_SCOPE_GLOBAL);
    }

    ICodeListener* createClfUniLabelStatic(ICodeListener *slave) {
        return createClfUniLabel(slave, CL_SCOPE_STATIC);
    }

    ICodeListener* createClfUniLabelFnc(ICodeListener *slave) {
        return createClfUniLabel(slave, CL_SCOPE_FUNCTION);
    }
}

ClfChainFactory::ClfChainFactory()
{
    map_["unify_labels_gl"]         = createClfUniLabelGl;
    map_["unify_labels_static"]     = createClfUniLabelStatic;
    map_["unify_labels_fnc"]        = createClfUniLabelFnc;
    map_["unfold_switch"]           = createClfUnfoldSwitch;
}

ICodeListener* ClfChainFactory::create(const std::string &clfString,
                                       ICodeListener *slave)
{
    CL_FACTORY_DEBUG("ClfChainFactory: clfString: " << clfString);
    TStringList clfList;
    if (!parseClfString(clfList, clfString)) {
        CL_ERROR("ivalid clf= option");
        return 0;
    }

    ICodeListener *chain = slave;

    TStringList::reverse_iterator i;
    for (i = clfList.rbegin(); chain && (i != clfList.rend()); ++i) {
        const string &clf = *i;
        CL_FACTORY_DEBUG("ClfChainFactory: looking for filter: " << clf);
        TMap::iterator i = map_.find(clf);
        if (i == map_.end()) {
            CL_ERROR("code_listener filter not found: " << clf);
            return 0;
        }

        chain = (i->second)(chain);
        if (chain)
            CL_FACTORY_DEBUG("ClfChainFactory: filter '" << clf
                    << "' created successfully");

#if CL_DEBUG_CLF
        chain = createClfIntegrityChk(chain);
        if (chain)
            CL_FACTORY_DEBUG("ClfChainFactory: integrity checker for '" << clf
                    << "' created successfully");
        else
            return 0;
#endif
    }

    return chain;
}

// /////////////////////////////////////////////////////////////////////////////
// ClFactory implementation
namespace {
    ICodeListener* createClPrettyPrintDef(const char *str) {
        return createClPrettyPrint(str, false);
    }

    ICodeListener* createClPrettyPrintWithTypes(const char *str) {
        return createClPrettyPrint(str, true);
    }
}

struct ClFactory::Private {
    typedef ICodeListener* (*TCreateFnc)(const char *);
    typedef std::map<string, TCreateFnc>                TMap;

    TMap                            map;
    ClfChainFactory                 clfFactory;
};

ClFactory::ClFactory():
    d(new Private)
{
    d->map["dotgen"]        = &createClDotGenerator;
    d->map["easy"]          = &createClEasy;
    d->map["locator"]       = &createClLocator;
    d->map["pp"]            = &createClPrettyPrintDef;
    d->map["pp_with_types"] = &createClPrettyPrintWithTypes;
    d->map["typedot"]       = &createClTypeDotGenerator;
}

ClFactory::~ClFactory()
{
    delete d;
}

ICodeListener* ClFactory::create(const char *config_string)
{
    CL_FACTORY_DEBUG("ClFactory: config_string: " << config_string);

    TStringMap args;
    if (!parseConfigString(args, config_string)) {
        CL_ERROR("invalid config_string given to ClFactory");
        return 0;
    }
    if (!hasKey(args, "listener")) {
        CL_ERROR("no listener= option given to ClFactory");
        return 0;
    }

    const string &name = args["listener"];
    CL_FACTORY_DEBUG("ClFactory: looking for listener: " << name);

    Private::TMap::iterator i = d->map.find(name);
    if (i == d->map.end()) {
        CL_ERROR("listener not found: " << name);
        return 0;
    }

    const string &listenerArgs = args["listener_args"];
    CL_FACTORY_DEBUG("ClFactory: creating listener '" << name << "' "
             "with args '" << listenerArgs << "'");

    ICodeListener *cl = (i->second)(listenerArgs.c_str());
    if (!cl)
        return 0;

    cl = createClfIntegrityChk(cl);
    if (cl) {
        CL_FACTORY_DEBUG(
                "ClFactory: createClfIntegrityChk() completed successfully");
    } else {
        return 0;
    }

    if (hasKey(args, "clf"))
        cl = d->clfFactory.create(args["clf"], cl);

    return cl;
}
