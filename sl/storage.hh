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

#ifndef H_GUARD_STORAGE_H
#define H_GUARD_STORAGE_H

#include "code_listener.h"
#include "cl_private.hh"

#include <string>
#include <vector>

namespace CodeStorage {

enum EVar {
    VAR_VOID,
    VAR_GL,
    VAR_LC,
    VAR_FNC_ARG
};

struct Var {
    EVar                        code;
    struct cl_location          loc;
    const struct cl_type        *clt;
    int                         uid;

    Var();
    ~Var();

    // NOTE: default assignment operator should be sufficient for a value type
    Var(EVar, const struct cl_operand *);
};

class VarDb {
    private:
        typedef std::vector<Var> TList;

    public:
        typedef TList::const_iterator const_iterator;
        typedef const_iterator iterator;

    public:
        VarDb();
        ~VarDb();
        VarDb(const VarDb &);
        VarDb& operator=(const VarDb &);

        Var& operator[](int uid);
        const Var& operator[](int uid) const;

        // read-only access to internal vector
        const_iterator begin() const { return vars_.begin(); }
        const_iterator end()   const { return vars_.end();   }
        size_t size()          const { return vars_.size();  }

    private:
        TList vars_;
        struct Private;
        Private *d;
};

class TypeDb {
    public:
        TypeDb();
        ~TypeDb();

        void insert(const struct cl_type *);
        const struct cl_type* operator[](int) const;

    private:
        // disable copy constructor and assignment
        TypeDb(const TypeDb &);
        TypeDb& operator=(const TypeDb &);
        struct Private;
        Private *d;
};

class Block;
class ControlFlow;

typedef std::vector<struct cl_operand> TOperandList;
typedef std::vector<const Block *> TTargetList;

struct Insn {
    enum cl_insn_e              code;
    int                         /* FIXME: type info lost */ subCode;
    struct cl_location          loc;
    TOperandList                operands;
    TTargetList                 targets;
};

/// @todo reverse links to precedent blocks?
class Block {
    private:
        typedef std::vector<Insn> TList;

    public:
        typedef TList::const_iterator const_iterator;
        typedef const_iterator iterator;

    public:
        /**
         * constructor useful to place objects into std::vector, but do NOT try
         * to call Block::append() on objects constructed this way. It would
         * crash on a NULL pointer dereference.
         */
        Block():
            cfg_(0)
        {
        }

        Block(/* TODO: comment at */ ControlFlow *cfg, const char *name):
            cfg_(cfg),
            name_(name)
        {
        }

        const std::string& name() const {
            return name_;
        }

        Insn& append() {
            unsigned idx = insns_.size();
            insns_.push_back();
            return insns_[idx];
        }

        const TTargetList& targets() const {
            if (insns_.empty())
                // Oops, we are asked for targets without no insns inside. We
                // can still return a reference to an empty vector in such
                // cases, but is it actually useful?
                TRAP;

            const Insn &last = insns_[insns_.size() - 1];
            return last.targets;
        }

        // read-only access to internal vector
        const_iterator begin()               const { return insns_.begin(); }
        const_iterator end()                 const { return insns_.end();   }
        size_t size()                        const { return insns_.size();  }
        const Insn& operator[](unsigned idx) const { return insns_[idx];    }

    private:
        TList insns_;
        ControlFlow *cfg_;
        std::string name_;
};

class ControlFlow {
    private:
        typedef std::vector<Block> TList;

    public:
        typedef TList::const_iterator const_iterator;
        typedef const_iterator iterator;

    public:
        ControlFlow();
        ~ControlFlow();
        ControlFlow(const ControlFlow &);
        ControlFlow& operator=(const ControlFlow &);

        const Block &entry() const {
            return bbs_[0];
        }

        // TODO: list of exits?
        // TODO: list of aborts?

        Block& operator[](const char *name);
        const Block& operator[](const char *name) const;

        // read-only access to internal vector
        const_iterator begin()                const { return bbs_.begin(); }
        const_iterator end()                  const { return bbs_.end();   }
        size_t size()                         const { return bbs_.size();  }
        const Block& operator[](unsigned idx) const { return bbs_[idx];    }

    private:
        TList bbs_;
        struct Private;
        Private *d;
};

struct File;
struct Fnc {
    File                        *file;
    struct cl_operand           def;
    VarDb                       vars;
    ControlFlow                 cfg;

    Fnc() {
        def.code = CL_OPERAND_VOID;
    }
};

class FncMap {
    private:
        typedef std::vector<Fnc> TList;

    public:
        typedef TList::const_iterator const_iterator;
        typedef const_iterator iterator;

    public:
        FncMap();
        ~FncMap();
        FncMap(const FncMap &);
        FncMap& operator=(const FncMap &);

        Fnc& operator[](int uid);
        const Fnc& operator[](int uid) const;

        // read-only access to internal vector
        const_iterator begin() const { return fncs_.begin(); }
        const_iterator end()   const { return fncs_.end();   }
        size_t size()          const { return fncs_.size();  }

    private:
        TList fncs_;
        struct Private;
        Private *d;
};

struct File {
    /* const */ std::string     name;
    VarDb                       vars;
    FncMap                      fncs;

    File(const std::string &name_):
        name(name_)
    {
    }
};

class FileMap {
    private:
        typedef std::vector<File> TList;

    public:
        typedef TList::const_iterator const_iterator;
        typedef const_iterator iterator;

    public:
        FileMap();
        ~FileMap();
        FileMap(const FileMap &);
        FileMap& operator=(const FileMap &);

        File& operator[](const char *name);
        const File& operator[](const char *name) const;

        // read-only access to internal vector
        const_iterator begin() const { return files_.begin(); }
        const_iterator end()   const { return files_.end();   }
        size_t size()          const { return files_.size();  }

    private:
        TList files_;
        struct Private;
        Private *d;
};

struct Storage {
    TypeDb                      types;
    VarDb                       glVars;
    FileMap                     files;
    FncMap                      orphans;
};

} // namespace CodeStorage

#endif /* H_GUARD_STORAGE_H */
