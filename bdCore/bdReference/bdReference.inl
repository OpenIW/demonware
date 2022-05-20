// SPDX-License-Identifier: GPL-3.0-or-later

template<typename T>
inline bdReference<T>::bdReference()
{
    m_ptr = NULL;
    // Will never enter this statement. But it exists in the inline func
    /*
    ref->m_ptr = 0;
    if ( ref->m_ptr )
      InterlockedIncrement(&ref->m_ptr->m_refCount);
    */
    if (m_ptr)
    {
        reinterpret_cast<bdReferencable*>(m_ptr)->addRef();
    }
}

template<typename T>
inline bdReference<T>::bdReference(const bdReference<T>* other)
{
    m_ptr = other->m_ptr;
    if (m_ptr)
    {
        reinterpret_cast<bdReferencable*>(m_ptr)->addRef();
    }
}

template<typename T>
inline bdReference<T>::bdReference(bdReference<T>* other)
{
    m_ptr = other->m_ptr;
    if (m_ptr)
    {
        reinterpret_cast<bdReferencable*>(m_ptr)->addRef();
    }
}

template<typename T>
inline bdReference<T>::~bdReference()
{
    if (m_ptr)
    {
        if (!reinterpret_cast<bdReferencable*>(m_ptr)->releaseRef())
        {
            if (m_ptr)
            {
                reinterpret_cast<bdReferencable*>(m_ptr)->~bdReferencable();
            }
            m_ptr = NULL;
        }
    }
}

template<typename T>
inline bdReference<T>::bdReference(T* p)
{
    m_ptr = p;
    if (m_ptr)
    {
        reinterpret_cast<bdReferencable*>(m_ptr)->addRef();
    }
}

template<typename T>
inline bdBool bdReference<T>::isNull() const
{
    return m_ptr == NULL;
}

template<typename T>
inline bdBool bdReference<T>::notNull() const
{
    return m_ptr != NULL;
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
inline T* bdReference<T>::operator*() const
{
    return this->m_ptr;
}

template<typename Cast, typename Reference>
inline Cast* reference_cast(bdReference<Reference> referenceClass)
{
    return dynamic_cast<Cast*>(referenceClass.operator->());
}

template<typename T>
inline bdBool bdReference<T>::operator!() const
{
    return m_ptr == BD_NULL;
}
