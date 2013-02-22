/*
 * Copyright (C) 2012 Pavel Raiskup <pavel@raiskup.cz>
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

#include "../config_cl.h"
#include "../util.hh"
#include "../builtins.hh"
#include "../pointsto.hh"
#include "../clplot.hh"
#include "../pointsto_assert.hh"

#include <cl/cl_msg.hh>
#include <cl/clutil.hh>
#include <cl/easy.hh>
#include <cl/storage.hh>

#include <boost/foreach.hpp>

// required by the gcc plug-in API
extern "C" {
    __attribute__ ((__visibility__ ("default"))) int plugin_is_GPL_compatible;
}

typedef const CodeStorage::Storage         &TStorage;
typedef const CodeStorage::Fnc             *TFnc;
typedef const CodeStorage::Block           *TBlock;
typedef const CodeStorage::PointsTo::Graph &TGraph;
typedef const CodeStorage::Insn            *TInsn;
typedef const CodeStorage::Var             *TVar;
typedef const struct cl_operand            &TOp;


struct PTCheckCtx {
    bool                expect_death;
    TStorage            stor;
    TFnc                fnc;

    PTCheckCtx(TStorage stor_) :
        expect_death(false),
        stor(stor_),
        fnc(0)
    {
    }
};

const char * cstString(const cl_operand &op)
{
    if (op.code != CL_OPERAND_CST)
        return 0;

    if (op.data.cst.code != CL_TYPE_STRING)
        return 0;

    return op.data.cst.data.cst_string.value;
}

/**
 * Find corresponding Var by variable name.
 *
 * fails when multiple variables have the requested name.  Note that this
 * function is very slow and should stay only here in chk_pt.cc for testing
 * purposes.
 */
TVar varLookup(TStorage stor, const char *name)
{
    TVar result = NULL;
    BOOST_FOREACH(TFnc fnc, stor.fncs) {
        BOOST_FOREACH(int vUid, fnc->vars) {
            TVar v = &stor.vars[vUid];
            if (v->name != name)
                continue;

            if (!result) {
                // found requested name first time
                result = v;
                continue;
            }

            if (v->uid == result->uid)
                // found multiple time the same uid -> it is ok
                continue;

            PT_ERROR("Multiple occurrence of symbol " << name);
            return NULL;
        }
    }

    return result;
}

TVar varLookupOp(TStorage stor, TOp op)
{
    const char *name = cstString(op);
    CL_BREAK_IF(!name);
    return varLookup(stor, name);
}

void chkPointsToFnc(TFnc fnc, TOp opA, TOp opB, bool expect)
{
    TStorage stor = *fnc->stor;
    TGraph ptg = fnc->ptg;

    TVar varA = varLookupOp(stor, opA);
    TVar varB = varLookupOp(stor, opB);
    CL_BREAK_IF(!varA || !varB);

    bool follows = CodeStorage::PointsTo::follows(ptg, varA, varB);
    if (follows != expect) {
        std::stringstream out;
        out << "points-to expect variable " << opA << " "
            << (expect ? "should" : "should _NOT_") << " follow " << opB
            << " in function " << nameOf(*fnc);
        CL_ERROR(out.str());
    }
}

void chkPointsTo(PTCheckCtx &ctx, TOp opA, TOp opB, bool expect)
{
    CL_BREAK_IF(opA.code == CL_OPERAND_CST || opB.code == CL_OPERAND_CST);

    const TVar varA = &ctx.stor.vars[varIdFromOperand(&opA)];
    const TVar varB = &ctx.stor.vars[varIdFromOperand(&opB)];

    bool follows = CodeStorage::PointsTo::followsGlobal(ctx.stor, varA, varB);
    if (follows != expect) {
        std::stringstream out;
        out << "points-to expect variable "
            << "'" << opA << "'"
            << (expect ? " should" : " should _NOT_") << " follow "
            << "'" << opB << "'"
            << "in any function";
        CL_ERROR(out.str());
    }
}

void chkIsPointed(
        PTCheckCtx                     &ctx,
        TOp                             op,
        bool                            expect)
{
    TVar var = &ctx.stor.vars[varIdFromOperand(&op)];
    bool pointed = CodeStorage::PointsTo::isPointedGlob(ctx.stor, var);
    if (expect != pointed) {
        std::stringstream out;
        out << "points-to expect variable "
            << "'" << op << "'"
            << (expect ? "should" : "should _NOT_")
            << " be pointed";
        CL_ERROR(out.str());
    }
}

void chkExistsLcl(PTCheckCtx &ctx, TOp op, bool expect)
{
    TFnc fnc = ctx.fnc;
    TVar var = varLookupOp(ctx.stor, op);
    CL_BREAK_IF(!var);

    if (!!(existsUid(fnc->ptg, var->uid)) != expect) {
        std::stringstream out;
        out << "points-to expect variable "
            << "'" << op << "'"
            << (expect ? "should" : "should _NOT_")
            << " exist in function '" << nameOf(*fnc) << "'";
        CL_ERROR(out.str());
    }
}

void setExpectFailed(PTCheckCtx &ctx)
{
    ctx.expect_death = true;
}

void chkBlock(PTCheckCtx &ctx, const TBlock bb)
{
    const TStorage stor = ctx.stor;
    BOOST_FOREACH(TInsn insn, *bb) {
        if (insn->code != CL_INSN_CALL)
            continue;
        int fncId;
        if (!fncUidFromOperand(&fncId, &insn->operands[1])) {
            CL_BREAK_IF("indirect call");
        }

        TFnc fnc = stor.fncs[fncId];
        const char *name = nameOf(*fnc);
        if (!STREQ("PT_ASSERT", name))
            continue;

        TOp typeOp = insn->operands[2];
        CL_BREAK_IF(typeOp.code != CL_OPERAND_CST
                        || typeOp.data.cst.code != CL_TYPE_INT);

        PTAssertType type = (PTAssertType)typeOp.data.cst.data.cst_int.value;

        bool expect = true;
        switch (type) {
            case PT_ASSERT_MAY_NOT_POINT_LOCAL:
                expect = false;
                // fall through!
            case PT_ASSERT_MAY_POINT_LOCAL:
                CL_BREAK_IF(insn->operands.size() != 5);
                chkPointsToFnc(ctx.fnc, insn->operands[3],
                        insn->operands[4], expect);
                break;
            case PT_ASSERT_MAY_NOT_POINT:
                expect = false;
                // fall through!
            case PT_ASSERT_MAY_POINT:
                CL_BREAK_IF(insn->operands.size() != 5);
                chkPointsTo(ctx, insn->operands[3], insn->operands[4], expect);
                break;
            case PT_ASSERT_MAY_NOT_BE_POINTED:
                expect = false;
                // fall through!
            case PT_ASSERT_MAY_BE_POINTED:
                CL_BREAK_IF(insn->operands.size() != 4);
                chkIsPointed(ctx, insn->operands[3], expect);
                break;
            case PT_ASSERT_EXISTS_LOCAL_NOT:
                expect = false;
                // fall through!
            case PT_ASSERT_EXISTS_LOCAL:
                CL_BREAK_IF(insn->operands.size() != 4);
                chkExistsLcl(ctx, insn->operands[3], expect);
                break;
            case PT_ASSERT_BUILD_FAIL:
                setExpectFailed(ctx);
                break;
            default:
                CL_BREAK_IF("unexpected type of points-to builtin");
        }
    }
}

void parseConfig(PTCheckCtx &, const std::string &conf)
{
    if (conf.size() == 0)
        return;
    PT_DEBUG(0, "config: " << conf);
#define STR_BEGINS(str, str2) (strncmp(str1, str2, sizeof(str2)) == 0)
}

void clEasyRun(const CodeStorage::Storage &stor, const char *config)
{
    CL_DEBUG("chk_pt started...");
    PTCheckCtx ctx(stor);
    parseConfig(ctx, config);

    BOOST_FOREACH(TFnc fnc, stor.fncs) {
        if (!isDefined(*fnc))
            continue;

        const CodeStorage::ControlFlow &cfg = fnc->cfg;
        ctx.fnc = fnc;

        BOOST_FOREACH(const TBlock bb, cfg)
            chkBlock(ctx, bb);
    }

    if (isDead(stor.ptd) != ctx.expect_death) {
        CL_ERROR("construction of PT graph should "
                << (ctx.expect_death ? ">> fail <<" : ">> succeed <<") );
    }

    CodeStorage::CallGraph::plotGraph(stor);
    CodeStorage::PointsTo::plotGraph(stor);
}
