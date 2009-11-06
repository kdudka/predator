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

namespace Storage {

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

    Var(EVar, const struct cl_operand *);
    ~Var();
};

class VarDb {
    public:
        VarDb();
        ~VarDb();

        void insert(const struct cl_operand *);
        const Var& operator[](int) const;

        // TODO: iterator?

    private:
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
        struct Private;
        Private *d;
};

class Block;
typedef std::vector<struct cl_operand> TOperandList;
typedef std::vector<const Block *> TTargetList;

struct Insn {
    enum cl_insn_e              code;
    int                         /* FIXME: type info lost */ subCode;
    struct cl_location          loc;
    TOperandList                operands;
    TTargetList                 targets;

    /// deep copy
    Insn(const struct cl_insn *);

    /// free all resources allocated by the constructor
    ~Insn();
};

/// @todo reverse links to precedent blocks?
/// @todo consider using a value type instead
class Block {
    private:
        typedef std::vector<Insn> TList;

    public:
        typedef TList::const_iterator const_iterator;

    public:
        Block(const char *name):
            name_(name)
        {
        }

        const std::string& name() const {
            return name_;
        }

        void append(const struct cl_insn *cli) {
            insns_.push_back(/* Insn object construction here */ cli);
        }

        const TTargetList& targets() const {
            const Insn &last = insns_[insns_.size() - 1];
            return last.targets;
        }

        // read-only access to internal vector
        const_iterator begin()               const { return insns_.begin();  }
        const_iterator end()                 const { return insns_.end();    }
        size_t size()                        const { return insns_.size();   }
        const Insn& operator[](unsigned idx) const { return insns_[idx];     }

    private:
        TList insns_;
        std::string name_;
};

class ControlFlow {
    private:
        typedef std::vector<const Block *> TList;

    public:
        typedef TList::const_iterator const_iterator;

    public:
        ControlFlow();
        ~ControlFlow();

        const Block &entry() const {
            const Block *entry = bbs_[0];
            return *entry;
        }

        // TODO: list of exits?
        // TODO: list of aborts?

        /// ControlFlow class takes ownership of given objects
        void insert(Block *);

        // read-only access to internal vector
        const_iterator begin()                const { return bbs_.begin();  }
        const_iterator end()                  const { return bbs_.end();    }
        size_t size()                         const { return bbs_.size();   }
        const Block& operator[](unsigned idx) const { return *(bbs_[idx]);  }

    private:
        TList bbs_;
};

struct File;
struct Fnc {
    File                        *file;
    struct cl_operand           def;
    VarDb                       vars;
    ControlFlow                 cfg;
};

class FncMap {
    private:
        typedef std::vector<Insn> TList;

    public:
        typedef TList::const_iterator const_iterator;

    public:
        FncMap();
        ~FncMap();

        Fnc& operator[](int uid);
        const Fnc& operator[](int uid) const;

        // read-only access to internal vector
        const_iterator begin()               const { return fncs_.begin();  }
        const_iterator end()                 const { return fncs_.end();    }
        size_t size()                        const { return fncs_.size();   }

    private:
        TList fncs_;
        struct Private;
        Private *d;
};

struct File {
    const std::string           name;
    VarDb                       vars;
    FncMap                      fncs;

    File(const char *name_):
        name(name_)
    {
    }
};

class FileMap {
    private:
        typedef std::vector<Insn> TList;

    public:
        typedef TList::const_iterator const_iterator;

    public:
        FileMap();
        ~FileMap();

        File& operator[](const char *name);
        const File& operator[](const char *name) const;

        // read-only access to internal vector
        const_iterator begin()               const { return files_.begin();  }
        const_iterator end()                 const { return files_.end();    }
        size_t size()                        const { return files_.size();   }

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

// TODO: move to separate module
class StorageBuilder: public ICodeListener {
    // TODO
};

} // namespace Storage

#endif /* H_GUARD_STORAGE_H */
