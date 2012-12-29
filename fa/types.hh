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

#ifndef TYPES_H
#define TYPES_H

// Standard library headers
#include <string>
#include <ostream>
#include <cassert>
#include <stdexcept>
#include <vector>

// Boost headers
#include <boost/functional/hash.hpp>

/**
 * @file types.hh
 * Data and SelData - data types for describing program data and selectors
 */

/**
 * @brief  Structure describing selectors
 *
 * This structure describes data selectors.
 *
 * @todo: write documentation... what the hell is 'displ'???
 */
struct SelData
{
	size_t       offset;  ///< offset in a structure
	int          size;    ///< size of the type
	int          displ;   ///< @todo write dox
	std::string  name;    ///< name of the selector

	/**
	 * @brief  Constructor
	 *
	 * Constructs an object from its components.
	 *
	 * @param[in]  offset  Offset of the selector
	 * @param[in]  size    Size of the selector
	 * @param[in]  displ   TODO write dox
	 * @param[in]  name    Name of the selector
	 */
	SelData(
		size_t                   offset,
		int                      size,
		int                      displ,
		const std::string&       name) :
		offset(offset),
		size(size),
		displ(displ),
		name(name)
	{ }

#if 0
	/**
	 * @brief  Construct selector information from arguments
	 *
	 * Constructs selector information from a list of arguments.
	 * @todo: see xxxx for the format
	 *
	 * @param[in]  args  The list of arguments
	 *
	 * @returns  Constructed selector information
	 */
	static SelData fromArgs(const std::vector<std::string>& args) {
		if (args.size() != 4) {
			throw std::runtime_error("incorrect number of arguments");
		}

		/// @todo smarter conversion of strings to integers
	 	return SelData(atol(args[1].c_str()), atol(args[2].c_str()),
			atol(args[3].c_str()));
	}
#endif

	/**
	 * @brief  Computes the hash value
	 *
	 * Overloaded function for computation of hash value of an object of type
	 * SelData.
	 *
	 * @param[in]  v  The object to be hashed
	 *
	 * @returns  The hash of the object
	 *
	 * @todo  Improve the distribution of the hash function
	 */
	friend size_t hash_value(const SelData& v)
	{
		return boost::hash_value(v.offset + v.size + v.displ);
	}

	/**
	 * @brief  Equality operator
	 *
	 * Operator checking equality.
	 *
	 * @param[in]  rhs  The object to check equality with
	 *
	 * @returns  @p true if the object is equal to @p rhs, @p false otherwise
	 */
	bool operator==(const SelData& rhs) const
	{
		return this->offset == rhs.offset &&
			this->size == rhs.size &&
			this->displ == rhs.displ;
	}

	/**
	 * @brief  The output stream operator
	 *
	 * The std::ostream << operator for conversion to a string.
	 *
	 * @param[in,out]  os  The output stream
	 * @param[in]      x   The value to be appended to the stream
	 *
	 * @returns  The modified output stream
	 */
	friend std::ostream& operator<<(std::ostream& os, const SelData& x)
	{
		// assert that a selector has a name
		assert(!x.name.empty());

		os << x.name << '[' << x.offset << ':' << x.size << ':';
		if (x.displ >= 0)
		{
			os << '+';
		}

		return os << x.displ << ']';
	}
};

/**
 * @brief  Enumeration of types of stored data
 *
 * This enumeration defines type of stored data.
 */
enum class data_type_e
{
	t_undef,          ///< undefined value
	t_unknw,          ///< unknown value
	t_native_ptr,     ///< native memory pointer for pointers to CFG (e.g. return address)
	t_void_ptr,       ///< void pointer (only has a size)
	t_ref,            ///< reference to a tree automaton
	t_int,            ///< integer
	t_bool,           ///< Boolean
	t_struct,         ///< structure
	t_other           ///< other type
};

namespace std
{
	template <>
	struct hash<data_type_e>
	{
		size_t operator()(const data_type_e& dataType) const
		{
			return std::hash<size_t>()(static_cast<size_t>(dataType));
		}
	};
}

/**
 * @brief  Structure for information about stored data
 *
 * This structure holds information about stored data, i.e., its size, type, and
 * additional information depending on the type.
 */
struct Data
{
	/**
	 * @brief  Type for information about nested data
	 *
	 * Type for holding information about nested data in a structure, i.e., offset
	 * in the structure and type information.
	 */
	typedef std::pair<size_t /* offset */, Data> item_info;

	/// The type of the data
	data_type_e type;

	/// The size of the data
	int size;

	/// Union with additional information about the data
	union
	{
		void*	  d_native_ptr;               ///< real memory pointer
		size_t	d_void_ptr_size;            ///< void pointer size

		/// information about reference
		struct {
			size_t root;         ///< reference to a tree automaton
			int    displ;        ///< displacement
		} d_ref;

		int		d_int;                        ///< value of represented integer
		bool	d_bool;                       ///< value of represented Boolean
		std::vector<item_info>* d_struct;   ///< nested data types for structure
	};

	/**
	 * @brief  Constructor
	 *
	 * Constructor from type information.
	 *
	 * @param[in]  type  The type of data
	 */
	Data(
		data_type_e        type = data_type_e::t_undef) :
		type(type),
		size(0)
	{ }

	/**
	 * @brief  Copy constructor
	 *
	 * Copying constructor.
	 *
	 * @param[in]  data  The object to be copied
	 */
	Data(
		const Data&          data) :
		type(data.type),
		size(data.size)
	{
		// fill the additional type information according to the type of data
		switch (data.type)
		{
			case data_type_e::t_native_ptr:
				this->d_native_ptr = data.d_native_ptr; break;
			case data_type_e::t_void_ptr:
				this->d_void_ptr_size = data.d_void_ptr_size; break;
			case data_type_e::t_ref:
				this->d_ref.root = data.d_ref.root;
				this->d_ref.displ = data.d_ref.displ; break;
			case data_type_e::t_int:
				this->d_int = data.d_int; break;
			case data_type_e::t_bool:
				this->d_bool = data.d_bool; break;
			case data_type_e::t_struct:
				this->d_struct = new std::vector<item_info>(*data.d_struct); break;
			default: break;
		}
	}

	/**
	 * @brief  Destructor
	 *
	 * The destructor.
	 */
	~Data() { this->clear(); }

	/**
	 * @brief  The assignment operator
	 *
	 * The assignment operator.
	 *
	 * @param[in]  rhs  The object to be copied
	 *
	 * @returns  Copy of the object
	 */
	Data& operator=(const Data& rhs)
	{
		if (this == &rhs) { return *this; }

		this->clear();
		this->type = rhs.type;
		this->size = rhs.size;

		// fill the additional type information according to the type of data
		/// @todo: remove duplicit code
		switch (rhs.type)
		{
			case data_type_e::t_native_ptr:
				this->d_native_ptr = rhs.d_native_ptr; break;
			case data_type_e::t_void_ptr:
				this->d_void_ptr_size = rhs.d_void_ptr_size; break;
			case data_type_e::t_ref:
				this->d_ref.root = rhs.d_ref.root;
				this->d_ref.displ = rhs.d_ref.displ; break;
			case data_type_e::t_int:
				this->d_int = rhs.d_int; break;
			case data_type_e::t_bool:
				this->d_bool = rhs.d_bool; break;
			case data_type_e::t_struct:
				this->d_struct = new std::vector<item_info>(*rhs.d_struct); break;
			default: break;
		}

		return *this;
	}

	/**
	 * @brief  Creates an undefined value
	 *
	 * Creates a type and value information for undefined value.
	 *
	 * @returns  Type and value information for undefined value
	 */
	static Data createUndef() { return Data(data_type_e::t_undef); }

	/**
	 * @brief  Creates an unknown value
	 *
	 * Creates a type and value information for unknown value.
	 *
	 * @returns  Type and value information for unknown value
	 */
	static Data createUnknw() { return Data(data_type_e::t_unknw); }

	/**
	 * @brief  Creates a native memory pointer
	 *
	 * Creates a type and value information for a native memory pointer.
	 *
	 * @param[in]  ptr  The pointer
	 *
	 * @returns  The type and value information for the pointer
	 */
	static Data createNativePtr(void* ptr)
	{
		Data data(data_type_e::t_native_ptr);
		data.d_native_ptr = ptr;
		return data;
	}

	/**
	 * @brief  Creates a reference to data
	 *
	 * Creates a type and value information about a reference to data.
	 *
	 * @param[in]  root  Tree automaton reference
	 * @param[in]  displ Displacement in the structure
	 *
	 * @returns  The type and value information for a reference
	 */
	static Data createRef(
		size_t                 root,
		int                    displ = 0)
	{
		Data data(data_type_e::t_ref);
		data.d_ref.root = root;
		data.d_ref.displ = displ;
		return data;
	}

	/**
	 * @brief  Creates a structure
	 *
	 * Creates a type and value information about a structure
	 *
	 * @param[in]  items  The nested data items
	 *
	 * @returns  The type and value information for a structure
	 */
	static Data createStruct(
		const std::vector<item_info>&      items = std::vector<item_info>())
	{
		Data data(data_type_e::t_struct);
		data.d_struct = new std::vector<item_info>(items);
		return data;
	}

	/**
	 * @brief  Creates a void pointer
	 *
	 * Creates a type and value information about a void pointer.
	 *
	 * @param[in]  size  Size of the pointer
	 *
	 * @returns  The type and value information about a void pointer
	 */
	static Data createVoidPtr(size_t size = 0)
	{
		Data data(data_type_e::t_void_ptr);
		data.d_void_ptr_size = size;
		return data;
	}

	/**
	 * @brief  Creates an integer
	 *
	 * Creates a type and value information about an integer.
	 *
	 * @param[in]  x  The value of the integer
	 *
	 * @returns  The type and value information about an integer
	 */
	static Data createInt(int x)
	{
		Data data(data_type_e::t_int);
		data.d_int = x;
		return data;
	}

	/**
	 * @brief  Creates a Boolean
	 *
	 * Creates a type and value information about a Boolean
	 *
	 * @param[in]  x  The value of the Boolean
	 *
	 * @returns  The type and value information about a Boolean
	 */
	static Data createBool(bool x)
	{
		Data data(data_type_e::t_bool);
		data.d_bool = x;
		return data;
	}

#if 0
	/**
	 * @brief  Construct type and value information from arguments
	 *
	 * Constructs type and value information from a list of arguments.
	 * @todo: see xxxx for the format
	 *
	 * @param[in]  args  The list of arguments
	 *
	 * @returns  Constructed type and value information
	 */
	static Data fromArgs(const std::vector<std::string>& args) {
		if (args[1] == "int") {
			if (args.size() != 3) {
				throw std::runtime_error("incorrect number of arguments");
			}

			Data data(data_type_e::t_int);
			/// @todo use a more elaborate conversion (checking correctness)
			data.d_int = atol(args[2].c_str());
			return data;
		}
		else if (args[1] == "ref") {
			if (args.size() != 4) {
				throw std::runtime_error("incorrect number of arguments");
			}

			Data data(data_type_e::t_ref);
			/// @todo use a more elaborate conversion (checking correctness)
			data.d_ref.root = atol(args[2].c_str());
			/// @todo use a more elaborate conversion (checking correctness)
			data.d_ref.displ = atol(args[3].c_str());
			return data;
		}
		else if (args[1] == "undef") {
			if (args.size() != 2) {
				throw std::runtime_error("incorrect number of arguments");
			}

			return Data(data_type_e::t_undef);
		}
		else
		{
			throw std::runtime_error("non-parsable arguments");
		}
	}
#endif

	/**
	 * @brief  Clears the structure
	 *
	 * Clears the structure.
	 */
	void clear()
	{
		if (this->type == data_type_e::t_struct)
		{
			delete this->d_struct;
			this->d_struct = nullptr;
		}

		this->type = data_type_e::t_undef;
	}

	/**
	 * @brief  Are the type and value defined?
	 *
	 * Returns @p false if the type is @p t_undef, @p true otherwise.
	 *
	 * @returns  @p false if the type is @p t_undef, @p true otherwise
	 */
	bool isDefined() const
	{
		return !isUndef();
	}

	/**
	 * @brief  Are the type and value undefined?
	 *
	 * Returns @p true if the type is @p t_undef, @p false otherwise.
	 *
	 * @returns  @p true if the type is @p t_undef, @p false otherwise
	 */
	bool isUndef() const
	{
		return this->type == data_type_e::t_undef;
	}

	/**
	 * @brief  Are the type and value unknown?
	 *
	 * Returns @p true if the type is @p t_unknw, @p false otherwise.
	 *
	 * @returns  @p true if the type is @p t_unknw, @p false otherwise
	 */
	bool isUnknw() const
	{
		return this->type == data_type_e::t_unknw;
	}

	/**
	 * @brief  Is the type a native pointer?
	 *
	 * Returns @p true if the type is @p t_native_ptr, @p false otherwise.
	 *
	 * @returns  @p true if the type is @p t_native_ptr, @p false otherwise
	 */
	bool isNativePtr() const
	{
		return this->type == data_type_e::t_native_ptr;
	}

	/**
	 * @brief  Is the type a void pointer?
	 *
	 * Returns @p true if the type is @p t_void_ptr, @p false otherwise.
	 *
	 * @returns  @p true if the type is @p t_void_ptr, @p false otherwise
	 */
	bool isVoidPtr() const
	{
		return this->type == data_type_e::t_void_ptr;
	}

	/**
	 * @brief  Is the type a NULL void pointer?
	 *
	 * Returns @p true if the type is a @p NULL, i.e., a t_int with a
	 * value 0, @p false otherwise.
	 *
	 * @returns  @p true if the type is @p NULL, @p false otherwise
	 */
	bool isNull() const
	{
		return this->type == data_type_e::t_int && this->d_int == 0;
	}

	/**
	 * @brief  Is the type a reference?
	 *
	 * Returns @p true if the type is @p t_ref, @p false otherwise.
	 *
	 * @returns  @p true if the type is @p t_ref, @p false otherwise
	 */
	bool isRef() const
	{
		return this->type == data_type_e::t_ref;
	}

	/**
	 * @brief  Is the type a reference to given root?
	 *
	 * Returns @p true if the type is @p t_ref and the value references the given
	 * tree automaton, @p false otherwise.
	 *
	 * @param[in]  root  Identifier of the tree automaton
	 *
	 * @returns  @p true if the type is @p t_ref and the value references the
	 *           given tree automaton, @p false otherwise
	 */
	bool isRef(size_t root) const
	{
		return this->type == data_type_e::t_ref && this->d_ref.root == root;
	}

	/**
	 * @brief  Is the type a structure?
	 *
	 * Returns @p true if the type is @p t_struct, @p false otherwise.
	 *
	 * @returns  @p true if the type is @p t_struct, @p false otherwise
	 */
	bool isStruct() const
	{
		return this->type == data_type_e::t_struct;
	}

	/**
	 * @brief  Is the type a Boolean?
	 *
	 * Returns @p true if the type is @p t_bool, @p false otherwise.
	 *
	 * @returns  @p true if the type is @p t_bool, @p false otherwise
	 */
	bool isBool() const
	{
		return this->type == data_type_e::t_bool;
	}

	/**
	 * @brief  Is the type an Integer?
	 *
	 * Returns @p true if the type is @p t_int, @p false otherwise.
	 *
	 * @returns  @p true if the type is @p t_int, @p false otherwise
	 */
	bool isInt() const
	{
		return this->type == data_type_e::t_int;
	}

	/**
	 * @brief  Retrieves the structure's selectors information
	 *
	 * Retrieves information about the structure's selectors.
	 *
	 * @returns  Selectors of the structure
	 */
	std::vector<item_info>& GetStruct() const
	{
		// Assertions
		assert(data_type_e::t_struct == this->type);
		assert(nullptr != this->d_struct);
		return *this->d_struct;
	}

	/**
	 * @brief  Computes the hash value
	 *
	 * Overloaded function for computation of hash value of an object of type
	 * Data.
	 *
	 * @param[in]  v  The object to be hashed
	 *
	 * @returns  The hash of the object
	 *
	 * @todo  Improve the distribution of the hash function
	 */
	friend size_t hash_value(const Data& v)
	{
		size_t seed = std::hash<data_type_e>()(v.type);
		switch (v.type)
		{
			case data_type_e::t_undef: break;
			case data_type_e::t_unknw: break;
			case data_type_e::t_native_ptr:
				boost::hash_combine(seed, v.d_native_ptr);
				break;
			case data_type_e::t_void_ptr:
				boost::hash_combine(seed, v.d_void_ptr_size);
				break;
			case data_type_e::t_ref:
				boost::hash_combine(seed, v.d_ref.root);
				boost::hash_combine(seed, v.d_ref.displ);
				break;
			case data_type_e::t_int:
				boost::hash_combine(seed, v.d_int);
				break;
			case data_type_e::t_bool:
				boost::hash_combine(seed, v.d_bool);
				break;
			case data_type_e::t_struct:
				boost::hash_combine(seed, *v.d_struct);
				break;
			case data_type_e::t_other:
				boost::hash_combine(seed, v.d_void_ptr_size);
				break;
			default:
				assert(false);              // fail gracefully
		}

		return seed;
	}

	/**
	 * @brief  Equality operator
	 *
	 * Operator checking equality.
	 *
	 * @param[in]  rhs  The object to check equality with
	 *
	 * @returns  @p true if the object is equal to @p rhs, @p false otherwise
	 */
	bool operator==(const Data& rhs) const
	{
		// check if the types match
		if (this->type != rhs.type)
			return false;

		// if the types match, check values
		switch (this->type) {
			case data_type_e::t_undef:
				return true;
			case data_type_e::t_unknw:
				return true;
			case data_type_e::t_void_ptr:
				return this->d_void_ptr_size == rhs.d_void_ptr_size;
			case data_type_e::t_native_ptr:
				return this->d_native_ptr == rhs.d_native_ptr;
			case data_type_e::t_ref:
				return this->d_ref.root == rhs.d_ref.root &&
					this->d_ref.displ == rhs.d_ref.displ;
			case data_type_e::t_int:
				return this->d_int == rhs.d_int;
			case data_type_e::t_bool:
				return this->d_bool == rhs.d_bool;
			case data_type_e::t_struct:
				return *this->d_struct == *rhs.d_struct;
			default:
				return false;
		}
	}


	/**
	 * @brief  Non-equality operator
	 *
	 * Operator checking non-equality.
	 *
	 * @param[in]  rhs  The object to check non-equality with
	 *
	 * @returns  @p true if the object is not equal to @p rhs, @p false otherwise
	 */
	bool operator!=(const Data& rhs) const
	{
		return !(*this == rhs);
	}


	/**
	 * @brief  The output stream operator
	 *
	 * The std::ostream << operator for conversion to a string.
	 *
	 * @param[in,out]  os  The output stream
	 * @param[in]      x   The value to be appended to the stream
	 *
	 * @returns  The modified output stream
	 */
	friend std::ostream& operator<<(std::ostream& os, const Data& x)
	{
		switch (x.type)
		{
			case data_type_e::t_undef:
				os << "(undef)"; break;
			case data_type_e::t_unknw:
				os << "(unknw)"; break;
			case data_type_e::t_native_ptr:
				os << "(native_ptr)" << x.d_native_ptr; break;
			case data_type_e::t_void_ptr:
				os << "(void_ptr)" << x.d_void_ptr_size; break;
			case data_type_e::t_ref:
				os << "(ref)" << x.d_ref.root << '+' << x.d_ref.displ; break;
			case data_type_e::t_int:
				os << "(int)" << x.d_int; break;
			case data_type_e::t_bool:
				os << "(bool)" << x.d_bool; break;
			case data_type_e::t_struct:
				os << "{ ";
				for (auto i = x.d_struct->begin(); i != x.d_struct->end(); ++i) {
					os << '+' << i->first << ':' << i->second << ' ';
				}
				os << "}";
				break;
			default:
				os << "(other)"; break;
		}
		return os;
	}
};

/**
 * @brief  The data type representing an array of @p Data values
 */
typedef std::vector<Data> DataArray;

#endif
