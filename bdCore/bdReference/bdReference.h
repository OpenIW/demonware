// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

template <typename T>
class bdReference
{
public:
	T* m_ptr;

	bdReference();
	bdReference(const bdReference<T>* other);
	bdReference(bdReference<T>* other);
	bdReference(T* p);
	~bdReference();
	bdBool isNull() const;
	bdBool notNull() const;
	T* operator->();
	const T* operator->() const;
	bdBool operator!() const;
	void operator=(T* p);
	bdReference<T>* operator=(const bdReference<T>* other);
	bdBool operator==(const bdReference<T>* other);
	T* operator*() const;
};

template <typename Cast, typename Reference>
Cast* reference_cast(bdReference<Reference> referenceClass);

#include "bdReference.inl"