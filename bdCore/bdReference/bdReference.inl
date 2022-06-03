// SPDX-License-Identifier: GPL-3.0-or-later

template<typename T>
inline bdReference<T>::bdReference()
{
    m_ptr = NULL;
    if (m_ptr)
    {
        m_ptr->addRef();
    }
}

template<typename T>
inline bdReference<T>::bdReference(const bdReference<T>& other)
{
    m_ptr = other.m_ptr;
    if (m_ptr)
    {
        m_ptr->addRef();
    }
}

template<typename T>
inline bdReference<T>::~bdReference()
{
    if (m_ptr)
    {
        if (!m_ptr->releaseRef())
        {
            if (m_ptr)
            {
               m_ptr->~T();
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
        m_ptr->addRef();
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
inline T* bdReference<T>::operator->() const
{
    return m_ptr;
}

template<typename T>
inline void bdReference<T>::operator=(T* p)
{
    if (m_ptr)
    {
        if (!m_ptr->releaseRef())
        {
            if (m_ptr)
            {
               m_ptr->~T();
            }
        }
    }
    m_ptr = p;
    if (m_ptr)
    {
        m_ptr->addRef();
    }
}

template<typename T>
inline bdReference<T>& bdReference<T>::operator=(const bdReference<T>& other)
{
    if (other.m_ptr != m_ptr)
    {
        if (m_ptr)
        {
            if (!m_ptr->releaseRef())
            {
                if (m_ptr)
                {
                    m_ptr->~T();
                }
            }
        }
        m_ptr = other.m_ptr;
        if (m_ptr)
        {
            m_ptr->addRef();
        }
    }
    return *this;
}

template<typename T>
inline bdBool bdReference<T>::operator==(const bdReference<T>& other) const
{
    return m_ptr == other.m_ptr;
}

template<typename T>
inline T* bdReference<T>::operator*() const
{
    return m_ptr;
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
