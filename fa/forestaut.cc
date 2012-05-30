/*
 * Copyright (C) 2010 Jiri Simacek
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

#include "forestaut.hh"
#include "tatimint.hh"

std::ostream& operator<<(std::ostream& os, const TreeAut& ta) {
	TAWriter<label_type> writer(os);
	os << '[';
	for (std::set<size_t>::iterator j = ta.getFinalStates().begin(); j != ta.getFinalStates().end(); ++j)
		writer.writeState(*j);
	os << " ]" << std::endl;;
	writer.writeTransitions(ta, FA::WriteStateF());
	return os;
}

std::ostream& operator<<(std::ostream& os, const FA& fa)
{
	os << '[';
	for (const Data& var : fa.variables_)
	{
		os << ' ' << var;
	}
	os << " ]" << std::endl;

	for (size_t i = 0; i < fa.roots.size(); ++i)
	{
		if (!fa.roots[i])
			continue;

		os << "===" << std::endl << "root " << i << " [" << fa.connectionGraph.data[i] << ']';

		TAWriter<label_type> writer(os);

		for (auto state : fa.roots[i]->getFinalStates())
		{
			writer.writeState(state);
		}

		writer.endl();
		writer.writeTransitions(*fa.roots[i], FA::WriteStateF());
	}

	return os;
}
