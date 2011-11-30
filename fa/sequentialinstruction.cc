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

#include "jump.hh"

#include "sequentialinstruction.hh"

void SequentialInstruction::finalize(
	const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>& codeIndex,
	std::vector<AbstractInstruction*>::const_iterator cur
) {

	this->next_ = *(cur + 1);

	// shorcut jump instruction
	if (this->next_->getType() == fi_type_e::fiJump) {

		do {

			this->next_ = ((FI_jmp*)this->next_)->getTarget(codeIndex);

		} while (this->next_->getType() == fi_type_e::fiJump);

	}

}
