/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   MemoryPlace.cc
* @brief  Implementation of the class that represents all types of memory places.
* @date   2012
*/

#include <iostream>
#include "MemoryPlace.h"

/**
* @brief Returns @c true if the current memory place represents an element of an
*        array (it could be arbitrarily nested in structure), @c false otherwise.
*/
bool MemoryPlace::representsElementOfArray() const
{
	// If the memory place represents an element of array, its name contains the
	// string "[]".
	size_t found = name.find("[]");

  	if (found == std::string::npos) {
  		return false;
  	} else {
  		return true;
  	}
}

