/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of sl.
 *
 * sl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * sl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sl.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cl_seplog.hh"

#include "cl_private.hh"
#include "cl_storage.hh"
#include "storage.hh"

#include <iostream>

#include <boost/foreach.hpp>

#include "../seplog/symbolic-heap.cc"

typedef line_id TLineId;
typedef std::pair<TLineId, symbolic_state*> execution_continuation_t;
typedef std::list<execution_continuation_t> execution_result_t;

class SlCommand {
    public:
        Operation op;
        object_id var1;
        object_id var2;
        field_id f_id;
        TLineId line[3];

    public:
        SlCommand(Operation o, int v1 = 0, int v2 = 0, int i = 0):
            op(o),
            var1(static_cast<object_id>(v1)),
            var2(static_cast<object_id>(v2)),
            f_id(static_cast<field_id>(i))
        {
        }

    public:
        execution_result_t execute(const symbolic_state *pre) const;

    private:
        void assignVV(execution_result_t &re, const symbolic_state *pre) const;
        void assignVM(execution_result_t &re, const symbolic_state *pre) const;
        void assignMV(execution_result_t &re, const symbolic_state *pre) const;
        void ifeq(execution_result_t &result, const symbolic_state *pre) const;
};

void SlCommand::assignVV(execution_result_t &result, const symbolic_state *pre)
    const
{
    symbolic_state *s = pre->clone(); // new symbolic state, sharing all
    s->assign_v_v(var1,var2);
    result.push_back(execution_continuation_t(line[1],s));
}

void SlCommand::assignVM(execution_result_t &result, const symbolic_state *pre)
    const
{
    symbolic_state *s = pre->clone(); // new symbolic state, sharing all
    if (!s->obj_is_var(var1) || !s->var_is_pointer(var2))
        TRAP;

    object_id target = s->var_points_to(var2);
    // FIXME: copy-pasted!!!
    if(target==nil) {
        std::cout << "*** NULL->f pointer dereference\n";
        std::cout << "at " << var2 << "\n";
        s->print();
        std::cout << "\n";
    } else if(s->obj_is_sls(target)) { // if pointer target is abstract
        object_id ls = target;
        if(s->sls_length(ls)==LS0P) { // one possibility: empty ls
            value_id next = s->sls_get_next(ls);
            // FIXME: copy-pasted!!!
            if(next==nil_value) {
                std::cout << "\n"
                    << "*** possible NULL pointer dereference at listsegment0+"
                    << "at " << var2 << "\n";
                s->print();
                std::cout << "\n";
            } else {
                throw "TODO: assign_V_M v, ls0p->nonnil ";
            }
        }
        // both cases ??
        s->sls_concretize(ls);
    }
    s->assign_v_m(var1,target,f_id);
    result.push_back(execution_continuation_t(line[1],s));
}

void SlCommand::assignMV(execution_result_t &result, const symbolic_state *pre)
    const
{
    symbolic_state *s = pre->clone(); // new symbolic state, sharing all
    if (!s->obj_is_var(var2) || !s->var_is_pointer(var1))
        TRAP;

    object_id target = s->var_points_to(var1);
    // FIXME: copy-pasted!!!
    if(target==nil) {
        std::cout << "*** NULL->f pointer dereference\n";
        std::cout << "at " << var2 << "\n";
        s->print();
        std::cout << "\n";
    } else if(s->obj_is_sls(target)) { // if pointer target is abstract
        object_id ls = target;
        if(s->sls_length(ls)==LS0P) { // one possibility: empty ls
            value_id next = s->sls_get_next(ls);
            // FIXME: copy-pasted!!!
            if(next==nil_value) {
                std::cout << "\n"
                    << "*** possible NULL pointer dereference at listsegment0+"
                    << "at " << var2 << "\n";
                s->print();
                std::cout << "\n";
            } else {
                throw "TODO: assign_V_M v, ls0p->nonnil ";
            }
        }
        // both cases ??
        s->sls_concretize(ls);
    }

    s->assign_m_v(target,f_id,var2);
    result.push_back(execution_continuation_t(line[1],s));
}

void SlCommand::ifeq(execution_result_t &result, const symbolic_state *pre)
    const
{
    //eval_condition(); // for now only var=var (and pointers)
    if(pre->var_is_equal(var1,var2)) { // same value ==> condition always true
        std::cout << "if(true) // var1==var2\n";
        // known exactly, go only single way
        symbolic_state *s = pre->clone(); // new symbolic state, sharing all
        // add true to s is not needed, var1=var2 already holds
        result.push_back(execution_continuation_t(line[2],s));  // if true
        return;
    }

    // values are not the same ==> maybe nondeterministic:
    // condition false or true - we don't know
    std::cout << "if(var1!=var2)\n";
    if(!pre->var_is_pointer(var1) || !pre->var_is_pointer(var2)) {
        std::cout << "if(nonptr!=nonptr)\n";
        TRAP;
    }

    std::cout << "if(ptr1 ?=? ptr2)\n";
    // both pointers
    object_id target1 = pre->var_points_to(var1);
    object_id target2 = pre->var_points_to(var2);
    if(pre->obj_is_nonempty(target1) && pre->obj_is_nonempty(target2)) {
        std::cout << "if(ptr1!=ptr2) deterministic\n";
        // values different
        // var1!=var2
        symbolic_state *s = pre->clone(); // new symbolic state, sharing all
        // add NEQ(var1,var2) ?    is for speed only?
        result.push_back(execution_continuation_t(line[1],s)); // if false
    } else if (pre->obj_is_nonempty(target1) && !pre->obj_is_nonempty(target2))
    {
        std::cout << "if(ptr!=ptr2pe) nondet\n";
        //TODO: wrap to function
        // target2 is abstract: possibly empty segment
        // this value should be shared
        value_id t2next = pre->sls_get_next(target2);
        if(pre->val_points_to(t2next) == target1) {
            // if possibly empty next is equal
            std::cout << " -- nondet A == empty listsegment\n";
            symbolic_state *s = pre->clone(); // new symbolic state, sharing all
            // ensure pe => empty
            s->sls_eliminate_empty(target2); // remove LS0P
            result.push_back(execution_continuation_t(line[2],s));  // if true
        }
        std::cout << " -- nondet B == nonempty listsegment\n";
        // and possibly non-true // var1!=var2
        symbolic_state *s = pre->clone(); // new symbolic state, sharing all
        // add NEQ(var1,var2) ?
        s->sls_set_length(target2,LS1P);
        result.push_back(execution_continuation_t(line[1],s)); // if false
    } else if (!pre->obj_is_nonempty(target1) && pre->obj_is_nonempty(target2))
    {
        std::cout << "if(ptr2pe!=ptr)  nondet\n";
        //TODO: wrap to function
        // target1 is possibly empty segment
        // a) empty variant
        // this value should be shared
        value_id t1next = pre->sls_get_next(target1);
        if(pre->val_points_to(t1next) == target2) {
            // if possibly empty next is equal
            std::cout << " -- nondet A == empty listsegment\n";
            symbolic_state *s = pre->clone(); // new symbolic state, sharing all
            // ensure pe => empty
            s->sls_eliminate_empty(target1); // remove LS0P
            result.push_back(execution_continuation_t(line[2],s));  // if true
        }
        std::cout << " -- nondet B == nonempty listsegment\n";
        // b) nonempty variant
        // var1!=var2
        symbolic_state *s = pre->clone(); // new symbolic state, sharing all
        // add NEQ(var1,var2) ?    is for speed only?
        s->sls_set_length(target1,LS1P);
        result.push_back(execution_continuation_t(line[1],s)); // if false
    } else if (!pre->obj_is_nonempty(target1) && !pre->obj_is_nonempty(target2))
    {
        std::cout << "if(ptr2pe!=ptr2pe)\n";
        // both targets possibly empty
        throw "EXEC: TODO: pe == pe ";
    }
}

execution_result_t SlCommand::execute(const symbolic_state *pre) const {
    execution_result_t result;       // empty list

    // 1) clone precondition
    // 2) update cloned state by command
    // 3) reduce state space (entailment,join)

    switch(op) {
        case EXIT:
            break;

        case ASSIGN_V_V:    // a = b;
            this->assignVV(result, pre);
            break;

        case ASSIGN_V_M:    // var1 = var2->field; OR var1 = var2.field;
            this->assignVM(result, pre);
            break;

        case ASSIGN_M_V:    // a->item = b;
            this->assignMV(result, pre);
            break;

        case IFEQ:          // if(a==b)
            this->ifeq(result, pre);
            break;

        default:
            TRAP;
    }

    std::cout << "- comamnd executed\n";
    return result;
}

class PrecondExample: public Precondition {
    public:
        PrecondExample() {
            type_id stype = types.add_structure("structX",2);
            type_id ptype = types.add_pointer_to(stype);
            types.add_structure_field(stype,"data",ptype);
            field_id nextid = types.add_structure_field(stype,"next",ptype);
            if (1 != nextid)
                TRAP;
            // empty initial symbolic_state
            symbolic_state *s = symbolic_state::create();
            s->var_new(ptype,"r");
            object_id varL = s->var_new(ptype,"l");

            // ls(_,nil,lambda)
            object_id ls = s->sls_create(LS0P,nil_value,stype,nextid);
            s->var_assign_pointer_at(varL,ls);

            s->var_new(ptype,"p");
            std::cout <<  "Global initial state\n";
            s->print();
            this->push_back(s);
        }
};

namespace {
void runSepLog(CodeStorage::Storage &stor, CodeStorage::Fnc &fnc) {
    std::vector<SlCommand> program;
#define add_line(C) static_cast<TLineId>(program.size()); do { \
        SlCommand c = (C); \
        TLineId ln = static_cast<TLineId>(program.size()); \
        c.line[0] = ln; \
        c.line[1] = TLineId(ln+1); \
        program.push_back(c); \
    } while (0)
    object_id null=nil;
    object_id r=static_cast<object_id>(2);
    object_id l=static_cast<object_id>(3);
    object_id p=static_cast<object_id>(5);
    field_id next=static_cast<field_id>(1); // check

    add_line(SlCommand(ASSIGN_V_V,r,null));
    TLineId L1 = add_line(SlCommand(IFEQ,l,null /*,next2unknown*/));
    add_line(SlCommand(ASSIGN_V_V,p,l));
    add_line(SlCommand(ASSIGN_V_M,l,l,next));
    add_line(SlCommand(ASSIGN_M_V,p,r,next));
    TLineId L2 = add_line(SlCommand(ASSIGN_V_V,r,p));
    program[L2].line[1] = L1; // GOTO L1
    TLineId L3 = add_line(SlCommand(EXIT));
    program[L1].line[2] = L3;

    AbstractProgramState precondition; // state of execution for all commands
    TLineId start = TLineId(0);
    precondition[start] = PrecondExample();

    std::queue<TLineId> todo;   // queue of lines/commands to analyze
    todo.push(start);              // start on this line
    while(!todo.empty()) {
        // get next line number to execute
        TLineId line = todo.front(); todo.pop();
        // execute the line command
        const SlCommand &cmd = program[line];  // get description

        Precondition &pre =  precondition[line]; // OR list of symbolic states
        pre.print();

        // for all precondition parts
        // to eliminate duplicate scheduling of continuations
        std::set<TLineId> already_scheduled;
        unsigned sscount=0;
        for(Precondition::const_iterator i=pre.begin(); i!=pre.end(); ++i) {
            // for all components (= symbolic states) of precondition
            // compute postconditions

            // next symbolic state in precondition
            const symbolic_state *ss = *i;
            std::cout << "precondition[" << line << "][" << sscount++ << "] ";
            std::cout << "= state#" << ss->state_number();
            if(ss->is_done()) {
                std::cout << " DONE\n";
                continue;
            }
            std::cout << "\n";

            execution_result_t r = cmd.execute(ss); // execute command

            ss->mark_done(); // state is executed

            // result is the container of pairs (next line, symbolic state)
            while(r.size()>0) {
                // get and parse single continuation
                execution_continuation_t ec = r.front();
                r.pop_front();

                TLineId next_cmd = ec.first;
                symbolic_state *p = ec.second;

                // =======================================================
                std::cout << "Result[line#" << next_cmd << "]: ";
                p->print_short();

                // merge with existing states (entailment,join,etc)
                // TODO

                // =======================================================
                // for now we add unchanged
                // add part of postcondition to next preconditions
                precondition[next_cmd].push_back(p);
                // if unchanged do nothing = continue
                if(already_scheduled.count(next_cmd)==0) {
                    // analyze continuation later (TODO: FIFO or LIFO ?)
                    todo.push(next_cmd);
                    already_scheduled.insert(next_cmd);
                }
            } // while result item
        } // for precondition item

        TRAP;
    } // while nonempty todo-list
}
} // namespace

class SymExec {
    public:
        SymExec(CodeStorage::Storage &stor):
            stor_(stor)
        {
        }

        void exec(const CodeStorage::Fnc &);

    protected:
        void exec(const CodeStorage::Insn &);
        void exec(const CodeStorage::Block &);

    private:
        CodeStorage::Storage        &stor_;
        const CodeStorage::Fnc      *fnc_;
        const CodeStorage::Block    *bb_;
        LocationWriter              lw_;
};

class ClSepLog: public ClStorageBuilder {
    public:
        ClSepLog(const char *configString);
        virtual ~ClSepLog();

    protected:
        virtual void run(CodeStorage::Storage &);
};

// /////////////////////////////////////////////////////////////////////////////
// SymExec implementation
void SymExec::exec(const CodeStorage::Insn &insn) {
    using namespace CodeStorage;
    lw_ = &insn.loc;

    CL_MSG_STREAM(cl_debug, lw_ << "debug: executing insn...");
}

void SymExec::exec(const CodeStorage::Block &bb) {
    using namespace CodeStorage;
    bb_ = &bb;

    CL_MSG_STREAM(cl_debug, lw_ << "debug: entering " << bb.name() << "...");
    BOOST_FOREACH(const Insn *insn, bb) {
        this->exec(*insn);
    }
}

void SymExec::exec(const CodeStorage::Fnc &fnc) {
    using namespace CodeStorage;
    fnc_ = &fnc;
    lw_ = &fnc.def.loc;

    CL_MSG_STREAM(cl_debug, lw_ << "debug: entering "
            << nameOf(fnc) << "()...");

    CL_DEBUG("looking for entry block...");
    const ControlFlow &cfg = fnc.cfg;
    const Block *entry = cfg.entry();
    if (!entry) {
        CL_MSG_STREAM(cl_error, lw_ << "error: "
                << nameOf(fnc) << ": "
                << "entry block not found");
        return;
    }
    this->exec(*entry);
}


// /////////////////////////////////////////////////////////////////////////////
// ClSepLog implementation
ClSepLog::ClSepLog(const char *) {
    // TODO
}

ClSepLog::~ClSepLog() {
    // TODO
}

void ClSepLog::run(CodeStorage::Storage &stor) {
    CL_DEBUG("looking for 'main()' at gl scope...");
    CodeStorage::Fnc *main = stor.glFncByName["main"];
    if (!main) {
        CL_MSG_STREAM_INTERNAL(cl_error,
                "error: main() not declared at global scope");
        return;
    }

    // this seems to be the way to go...
    SymExec se(stor);
    se.exec(*main);

    // temporary testing arena
    try {
        runSepLog(stor, *main);
    }
    catch (const char *msg) {
        CL_MSG_STREAM_INTERNAL(cl_error, "error: "
                << "*** symbolic_heap exception *** "
                << msg);
    }
}


// /////////////////////////////////////////////////////////////////////////////
// public interface, see cl_seplog.hh for more details
ICodeListener* createClSepLog(const char *configString) {
    return new ClSepLog(configString);
}
