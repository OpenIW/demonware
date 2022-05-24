// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdSingletonRegistryImpl
{
public:
    typedef void (*bdSingletonDestroyFunction)(void);
protected:
    bdFastArray<void(*)()> m_destroyFunctions;
    bdBool m_cleaningUp;
public:
    bdSingletonRegistryImpl();
    virtual ~bdSingletonRegistryImpl();
    bdBool add(const bdSingletonDestroyFunction destroyFunction);
    void cleanUp();
};

template<typename T>
class bdSingleton
{
protected:
    static T* m_instance;
public:
    static T* getInstance()
    {
        if (!m_instance)
        {
            m_instance = new T();
            if (m_instance)
            {
                if (!bdSingleton<bdSingletonRegistryImpl>::getInstance()->add(&destroyInstance))
                {
                    if (m_instance)
                    {
                        delete m_instance;
                    }
                    m_instance = NULL;
                }
            }
        }
        return m_instance;
    }
    static void destroyInstance()
    {
        if (m_instance)
        {
            delete m_instance;
            m_instance = NULL;
        }
    }
};
