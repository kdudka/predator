/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   MemoryPlace.h
* @brief  Class represents all types of memory places.
* @date   2012
*/

#ifndef GUARD_MEMORY_PLACE_H
#define GUARD_MEMORY_PLACE_H

#include <string>

/**
* @brief Class represents all types of memory places.
*
* This class represents the memory place, such as variable, item in a structure
* or element in an array.
*/
class MemoryPlace {
	private:
		std::string name;
		bool artificial;

	public:
		/// Creates new memory place.
		MemoryPlace(std::string name, bool artificial): name(name),
														artificial(artificial) {}

		/// Returns the name of the memory place (as it is represented in a
		// program).
		std::string asString() const { return name; }

		/// Returns @c true if memory place was created by a compiler, @c false
		/// otehrwise.
		bool isArtificial() const    { return artificial; }

		bool representsElementOfArray() const;
};

#endif
