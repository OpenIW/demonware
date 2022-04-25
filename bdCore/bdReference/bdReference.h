// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

template <typename T>
class bdReference
{
public:
	T* m_ptr;

	bdReference();
	bdReference(const bdReference<T>* other);
	~bdReference();
	bdReference(T* p);
	bdBool isNull();
	bdBool notNull();

	T* operator->();
	void operator=(T* p);
	bdReference<T>* operator=(const bdReference<T>* other);
	bdBool operator==(const bdReference<T>* other);
	T* operator*();
};

#include "bdReference.inl"