// SPDX-License-Identifier: GPL-3.0-or-later

template<typename T>
inline bdReference<T>::bdReference()
{
    m_ptr = NULL;
}

template<typename T>
inline bdReference<T>::bdReference(const bdReference<T>* other)
{
    this->m_ptr = other->m_ptr;
    if (this->m_ptr)
    {
        reinterpret_cast<bdReferencable*>(this->m_ptr)->addRef();
    }
}

template<typename T>
inline bdReference<T>::~bdReference()
{
    if (this->m_ptr)
    {
        if (!reinterpret_cast<bdReferencable*>(this->m_ptr)->releaseRef())
        {
            if (this->m_ptr)
            {
                reinterpret_cast<bdReferencable*>(this->m_ptr)->~bdReferencable();
            }
            this->m_ptr = NULL;
        }
    }
}

template<typename T>
inline bdReference<T>::bdReference(T* p)
{
    this->m_ptr = p;
    if (this->m_ptr)
    {
        reinterpret_cast<bdReferencable*>(this->m_ptr)->addRef();
    }
}

template<typename T>
inline bdBool bdReference<T>::isNull() const
{
    return this->m_ptr == NULL;
}

template<typename T>
inline bdBool bdReference<T>::notNull() const
{
    return this->m_ptr != NULL;
}

template<typename T>
inline T* bdReference<T>::operator->()
{
    return m_ptr;
}

template<typename T>
inline const T* bdReference<T>::operator->() const
{
    return m_ptr;
}

template<typename T>
inline void bdReference<T>::operator=(T* p)
{
    if (this->m_ptr)
    {
        if (!reinterpret_cast<bdReferencable*>(this->m_ptr)->releaseRef())
        {
            if (this->m_ptr)
            {
                reinterpret_cast<bdReferencable*>(this->m_ptr)->~bdReferencable();
            }
        }
    }
    this->m_ptr = p;
    if (this->m_ptr)
    {
        reinterpret_cast<bdReferencable*>(this->m_ptr)->addRef();
    }
}

template<typename T>
inline bdReference<T>* bdReference<T>::operator=(const bdReference<T>* other)
{
    if (other != this)
    {
        if (this->m_ptr)
        {
            if (!reinterpret_cast<bdReferencable*>(this->m_ptr)->releaseRef())
            {
                if (this->m_ptr)
                {
                    reinterpret_cast<bdReferencable*>(this->m_ptr)->~bdReferencable();
                }
            }
        }
        this->m_ptr = other->m_ptr;
        if (this->m_ptr)
        {
            reinterpret_cast<bdReferencable*>(this->m_ptr)->addRef();
        }
    }
    return this;
}

template<typename T>
inline bdBool bdReference<T>::operator==(const bdReference<T>* other)
{
    return m_ptr == other->m_ptr;
}

template<typename T>
inline T* bdReference<T>::operator*()
{
    return this->m_ptr;
}