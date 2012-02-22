/*
 * Copyright (C) 2011 Jiri Simacek
 *
 * This file is part of forester.
 *
 * forester is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * forester is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with forester.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMPILER_H
#define COMPILER_H

#include <ostream>
#include <vector>
#include <unordered_map>

#include <cl/storage.hh>
#include <cl/cl_msg.hh>
#include <cl/cldebug.hh>
#include <cl/clutil.hh>

#include "abstractinstruction.hh"
#include "treeaut.hh"
#include "label.hh"

namespace CodeStorage {
    struct Fnc;
    struct Storage;
    struct Insn;
}

std::ostream& operator<<(std::ostream& os, const cl_loc& loc);

class Compiler {

public:

	struct Assembly {

		std::vector<AbstractInstruction*> code_;
		std::unordered_map<const struct CodeStorage::Fnc*, AbstractInstruction*> functionIndex_;
		size_t regFileSize_;

		~Assembly() { this->clear(); }

		void clear() {

			for (auto instr : this->code_)
				delete instr;

			this->code_.clear();
			this->functionIndex_.clear();
			this->regFileSize_ = 0;

		}

		AbstractInstruction* getEntry(const struct CodeStorage::Fnc* f) const {

			auto iter = this->functionIndex_.find(f);
			assert(iter != this->functionIndex_.end());
			return iter->second;

		}

		friend std::ostream& operator<<(std::ostream& os, const Assembly& as) {

			AbstractInstruction* prev = NULL;

			size_t c = 0;

			const CodeStorage::Insn* lastInsn = nullptr;

			for (auto instr : as.code_) {

				if ((instr->getType() == fi_type_e::fiJump) && prev) {
					switch (prev->getType()) {
						case fi_type_e::fiBranch:
						case fi_type_e::fiJump:
							prev = instr;
							continue;
						default:
							break;
					}
				}

				prev = instr;

				if (instr->insn() && (instr->insn() != lastInsn)) {

					os << instr->insn()->loc << ' ' << *instr->insn() << std::endl;

					lastInsn = instr->insn();

				}

				if (instr->isTarget())
					os << instr << ':' << std::endl;

				os << "\t\t" << *instr << std::endl;

				++c;

			}

			return os << "code size: " << c << " instructions" << std::endl;

		}

	};

public:

	Compiler(TA<label_type>::Backend& fixpointBackend, TA<label_type>::Backend& taBackend,
		class BoxMan& boxMan);

	~Compiler();

	void compile(Assembly& assembly, const CodeStorage::Storage &stor, const CodeStorage::Fnc& entry);

private:

	class Core;
	Core *core_;

};

#endif
