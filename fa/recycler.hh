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

#ifndef RECYCLER_H
#define RECYCLER_H

#include <vector>

template <class T>
class Recycler {

	std::vector<T*> buffer_;

public:

	Recycler() :
		buffer_{}
	{ }

	~Recycler() { this->clear(); }

	void clear() {

		for (auto obj : this->buffer_)
			delete obj;

		this->buffer_.clear();

	}

	T* alloc() {

		T* obj;

		if (this->buffer_.empty()) {
			obj = new T();
		} else {
			obj = this->buffer_.back();
			this->buffer_.pop_back();
		}

		return obj;

	}

	template <class F>
	T* alloc(F f) {

		T* obj;

		if (this->buffer_.empty()) {
			obj = f();
		} else {
			obj = this->buffer_.back();
			this->buffer_.pop_back();
		}

		return obj;

	}
/*
	T* alloc(Args&&... params) {

		T* obj;

		if (this->buffer_.empty()) {
			obj = new T(std::forward<Args>(params)...);
		} else {
			obj = this->buffer_.back();
			this->buffer_.pop_back();
			obj->init(std::forward<Args>(params)...);
		}

		return obj;

	}
*/
	struct NoOp {
		void operator()(Recycler<T>&, T*) {}
	};

	template <class F = NoOp>
	void recycle(T* obj, F f = NoOp()) {

		this->buffer_.push_back(obj);

		f(*this, obj);

	}

};

#endif
