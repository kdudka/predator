/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
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
#include <cl/clutil.hh>

#include <cl/storage.hh>

#include "util.hh"

#include <sstream>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

bool operator==(const struct cl_type &a, const struct cl_type &b)
{
    // go through the given types recursively and match UIDs etc.
    typedef std::pair<const struct cl_type *, const struct cl_type *> TItem;
    std::stack<TItem> todo;
    push(todo, &a, &b);
    while (!todo.empty()) {
        const struct cl_type *cltA, *cltB;
        boost::tie(cltA, cltB) = todo.top();
        todo.pop();

        if (cltA->uid == cltB->uid)
            // UID matched, go next
            continue;

        const enum cl_type_e code = cltA->code;
        if (cltB->code != code)
            // code mismatch
            return false;

        const int cnt = cltA->item_cnt;
        if (cltB->item_cnt != cnt)
            // mismatch in the count of sub-types
            return false;

        switch (code) {
            case CL_TYPE_VOID:
            case CL_TYPE_INT:
            case CL_TYPE_CHAR:
            case CL_TYPE_BOOL:
            case CL_TYPE_REAL:
            case CL_TYPE_ENUM:
                if (cltA->size != cltB->size)
                    // size mismatch
                    return false;

                if (cltA->is_unsigned != cltB->is_unsigned)
                    // signedness mismatch
                    return false;

                // FIXME: we simply ignore differences that gcc seems important!
                return true;

            case CL_TYPE_UNKNOWN:
                return false;

            case CL_TYPE_STRING:
                // should be used only by cl_cst, see the dox
                CL_TRAP;
                return false;

            case CL_TYPE_PTR:
            case CL_TYPE_FNC:
            case CL_TYPE_STRUCT:
            case CL_TYPE_UNION:
            case CL_TYPE_ARRAY:
                // nest into types
                for (int i = 0; i < cnt; ++i) {
                    const struct cl_type_item *ciA = cltA->items + i;
                    const struct cl_type_item *ciB = cltB->items + i;
                    if (ciA->name && ciB->name && !STREQ(ciA->name, ciB->name))
                        return false;

                    push(todo, ciA->type, ciB->type);
                }
        }
    }

    // all OK
    return true;
}

const struct cl_type* targetTypeOfPtr(const struct cl_type *clt)
{
    if (!clt || clt->code != CL_TYPE_PTR)
        return /* not a pointer */ 0;

    CL_BREAK_IF(clt->item_cnt != 1);
    clt = clt->items[/* target */ 0].type;

    CL_BREAK_IF(!clt);
    return clt;
}

const struct cl_type* targetTypeOfArray(const struct cl_type *clt)
{
    CL_BREAK_IF(!clt || clt->code != CL_TYPE_ARRAY || clt->item_cnt != 1);

    clt = clt->items[/* target */ 0].type;
    CL_BREAK_IF(!clt);
    return clt;
}

bool seekRefAccessor(const struct cl_accessor *ac)
{
    for(; ac; ac = ac->next) {
        if (CL_ACCESSOR_REF != ac->code)
            continue;

        // there should be no more accessors after the first CL_ACCESSOR_REF
        CL_BREAK_IF(ac->next);
        return true;
    }

    // not found
    return false;
}

int intCstFromOperand(const struct cl_operand *op)
{
    CL_BREAK_IF(CL_OPERAND_CST != op->code);

    const struct cl_cst &cst = op->data.cst;
    CL_BREAK_IF(CL_TYPE_INT != cst.code);

    return cst.data.cst_int.value;
}

int varIdFromOperand(const struct cl_operand *op, const char **pName)
{
    CL_BREAK_IF(CL_OPERAND_VAR != op->code);
    if (pName)
        *pName = op->data.var->name;

    return op->data.var->uid;
}

bool fncNameFromCst(const char **pName, const struct cl_operand *op)
{
    if (CL_OPERAND_CST != op->code)
        return false;

    const struct cl_cst &cst = op->data.cst;
    if (CL_TYPE_FNC != cst.code)
        return false;

    // FIXME: the name of this helper is quite misleading!!!
    if (CL_SCOPE_GLOBAL != op->scope || !cst.data.cst_fnc.is_extern)
        return false;

    const char *fncName = cst.data.cst_fnc.name;
    *pName = fncName;
    return !!fncName;
}

bool fncUidFromOperand(int *pUid, const struct cl_operand *op)
{
    if (CL_OPERAND_CST != op->code)
        return false;

    const struct cl_cst &cst = op->data.cst;
    if (CL_TYPE_FNC != cst.code)
        return false;

    *pUid = cst.data.cst_fnc.uid;
    return true;
}

std::string varToString(
        const CodeStorage::Storage      &stor,
        const int                       uid,
        const struct cl_loc             **pLoc)
{
    const CodeStorage::Var &var = stor.vars[uid];
    if (pLoc)
        *pLoc = &var.loc;

    std::ostringstream str;
    str << "#" << var.uid;

    const std::string &name = var.name;
    if (!name.empty())
        str << ":" << name;

    return str.str();
}

int offsetByIdxChain(const struct cl_type *clt, const TFieldIdxChain &ic)
{
    int off = 0;

    BOOST_FOREACH(const int idx, ic) {
        CL_BREAK_IF(clt->item_cnt <= idx);
        const struct cl_type_item *item = clt->items + idx;
        off += item->offset;
        clt = item->type;
    }

    return off;
}
