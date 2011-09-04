/*
 * Copyright (C) 2010 Jiri Simacek
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

#ifndef LOOP_ANALYSER_H
#define LOOP_ANALYSER_H

#include <unordered_set>

#include <cl/storage.hh>

struct LoopAnalyser {

	struct BlockListItem {
		
		BlockListItem* prev;
		const CodeStorage::Block* block;

		BlockListItem(BlockListItem* prev, const CodeStorage::Block* block)
			: prev(prev), block(block) {}

		static bool lookup(const BlockListItem* item, const CodeStorage::Block* block) {
			if (!item)
				return false;
			if (item->block == block)
				return true;
			return BlockListItem::lookup(item->prev, block);
		}

	};

	std::unordered_set<const CodeStorage::Insn*> entryPoints;

	void visit(const CodeStorage::Block* block, std::unordered_set<const CodeStorage::Block*>& visited, BlockListItem* prev) {

		BlockListItem item(prev, block);

		if (!visited.insert(block).second) {
			if (BlockListItem::lookup(prev, block))
				this->entryPoints.insert(*block->begin());
			return;
		}

		for (auto target : block->targets())
			this->visit(target, visited, &item);
		
	}
	
	void init(const CodeStorage::Block* block) {

		std::unordered_set<const CodeStorage::Block*> visited;
		this->entryPoints.clear();
		this->visit(block, visited, NULL);

	}

	bool isEntryPoint(const CodeStorage::Insn* insn) const {
		return this->entryPoints.find(insn) != this->entryPoints.end();
	}
	
};

#endif
