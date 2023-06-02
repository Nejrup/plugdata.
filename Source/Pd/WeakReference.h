/*
 // Copyright (c) 2015-2022 Pierre Guillot and Timothy Schoen.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */
#pragma once

#include <functional>
#include <unordered_map>
#include <mutex>

#include <m_pd.h>

using pd_weak_reference = std::atomic<bool>;


namespace pd
{

class Instance;
struct WeakReference
{
    WeakReference(void* p, Instance* instance);
    ~WeakReference();
    
    void setThis() const;
    
    template<typename T>
    struct Ptr
    {
        
        Ptr(T* pointer, const pd_weak_reference& ref) : weakRef(ref), ptr(pointer)
        {
            sys_lock();
        }
        
        ~Ptr()
        {
            sys_unlock();
        }
        
        operator bool() const {
            return weakRef;
        }
        
        T* get()
        {
            return weakRef ? ptr : nullptr;
        }
    
        template<typename C>
        C* cast()
        {
            return weakRef ? reinterpret_cast<C*>(ptr) : nullptr;
        }
    
        T* operator->() {
            return ptr;
        }
        
        const pd_weak_reference& weakRef;
        T* ptr;
        
        JUCE_DECLARE_NON_COPYABLE(Ptr);
    };
    
    template<typename T>
    Ptr<T> get() const
    {
        setThis();
        return Ptr<T>(reinterpret_cast<T*>(ptr), weakRef);
    }
    
    template<typename T>
    T* getRaw() const
    {
        setThis();
        return weakRef ? reinterpret_cast<T*>(ptr) : nullptr;
    }
    
    template<typename T>
    T* getRawUnchecked() const
    {
        return reinterpret_cast<T*>(ptr);
    }
    
private:
    t_pd* ptr;
    Instance* pd;
    pd_weak_reference weakRef = true;
};

}