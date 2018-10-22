//
//  owned_ptr.h
//  ptrs
//
//  Created by Zeyang Li on 5/30/18.
//  Copyright © 2018 Zeyang Li. All rights reserved.
//

#pragma once

#include <cstdint>
#include <cassert>
#include <functional>

namespace cr::smart_ptrs::move_only
{
#define CR_DEBUG 0
    //-----------------------------
    // DO NOT SUPPORT ARRAYS !!!
    //
    // shared_ptr
    // weak_ptr
    // enable_shared_from_this
    //-----------------------------
    class shared_block
    {
    private:
        long shared = 0;
        long weak = 0;
    public:
        void (*deleter)(void*);

    public:
        shared_block(void (*del)(void*)) noexcept
            : deleter(del)
        {}
        
        inline void add_shared() noexcept
        {
#if CR_DEBUG
            assert(shared >= 0 && "invalid shared counter");
#endif
            shared += 1;
        }
        inline void add_weak() noexcept
        {
#if CR_DEBUG
            assert(weak >= 0 && "invalid weak counter");
#endif
            weak += 1;
        }
        inline void release_shared() noexcept
        {
#if CR_DEBUG
            assert(shared > 0 && "double shared release");
#endif
            shared -= 1;
        }
        inline void release_weak() noexcept
        {
#if CR_DEBUG
            assert(weak > 0 && "double weak release");
#endif
            weak -= 1;
        }
        
        long shared_count() const noexcept { return shared; }
        long weak_count() const noexcept { return weak; }
    };
    
    template<typename T>
    class shared_ptr;
    
    template<typename T>
    class enable_shared_from_this;
    
    
    template<typename T>
    class weak_ptr
    {
    public:
        typedef T element_type;
        
    private:
        // it is possible to have m_block->use_count() == 0 while m_ptr != nullptr
        // this is when shared_ptr resets itself
        shared_block* m_block = nullptr;
        T* m_ptr              = nullptr;
        
        template<typename U>
        using valid_cast = typename std::enable_if<std::is_convertible<U*, element_type*>::value, U>::type;
        
    public:
        constexpr weak_ptr() noexcept {}
        constexpr weak_ptr(std::nullptr_t) noexcept {}

        template<typename U, typename = valid_cast<U>>
        weak_ptr(const shared_ptr<U>& owned) noexcept;
        
        weak_ptr(const weak_ptr& other) noexcept
            : m_block(other.m_block), m_ptr(other.m_ptr)
        {
            if(m_block)
                m_block->add_weak();
        }
        
        template<typename U, typename = valid_cast<U>>
        weak_ptr(const weak_ptr<U>& other) noexcept
            : m_block(other.m_block), m_ptr(other.m_ptr)
        {
            if(m_block)
                m_block->add_weak();
        }
        
        // used for static casting only
        template<typename U>
        weak_ptr(const weak_ptr<U>& other, T* raw) noexcept
            : m_block(other.m_block), m_ptr(raw)
        {
            if(m_block)
                m_block->add_weak();
        }
        
        ~weak_ptr() noexcept
        {
            if(m_block)
            {
                m_block->release_weak();
                
                if(m_block->weak_count() == 0 && m_block->shared_count() == 0)
                    delete m_block;
            }
        }
        
        void swap(weak_ptr& other) noexcept
        {
            std::swap(m_ptr, other.m_ptr);
            std::swap(m_block, other.m_block);
        }
        
        void reset() noexcept { weak_ptr().swap(*this); }
        
        long use_count() const noexcept { return m_block ? m_block->shared_count() : 0; }
        
        bool expired() const noexcept { return m_block == nullptr || m_block->shared_count() == 0; }

        template<typename U> bool owner_before(const shared_ptr<U>& other) const noexcept { return m_block < other.m_block; }
        template<typename U> bool owner_before(const weak_ptr<U>& other)   const noexcept { return m_block < other.m_block; }
        
        inline shared_ptr<T> lock();
        inline shared_ptr<T> lock() const;
    
        weak_ptr& operator=(const weak_ptr& other) noexcept
        {
            // copy swap
            weak_ptr(other).swap(*this);
            return *this;
        }
        
        template<typename U, typename = valid_cast<U>>
        weak_ptr& operator=(const weak_ptr<U>& other) noexcept
        {
            // swap with a temp object for assignment
            weak_ptr(other).swap(*this);
            return *this;
        }
            
        template<typename U, typename = valid_cast<U>>
        weak_ptr& operator=(const shared_ptr<U>& other) noexcept;
            
        // debug only stuff
        shared_block* __get_block() const noexcept { return m_block; }
        // end of debug stuff
        
        // friends
        template<typename U> friend class weak_ptr;
        template<typename U> friend class shared_ptr;
    };

    
    template<typename T>
    class shared_ptr
    {
    public:
        typedef T element_type;
        typedef weak_ptr<T> weak_type;
    private:
        // invariant:
        // 1. m_ptr & m_block either both nullptr, or both not null
        // 2. when shared_ptr == nullptr, m_ptr & m_block must both be null
        shared_block* m_block = nullptr;
        T*            m_ptr   = nullptr;
        
        template<typename U> using valid_cast = typename std::enable_if<std::is_convertible<U*, element_type*>::value, U>::type;
    private:
        template <typename U, typename R,
                  typename = typename std::enable_if<std::is_convertible<R*, const enable_shared_from_this<U>*>::value, void>::type>
        void enable_weak_this(const enable_shared_from_this<U>* ptr, R* raw) noexcept;
        
        void enable_weak_this(...) noexcept {}
        
        // construct from a weak pointer
        shared_ptr(const weak_ptr<T>& weak) noexcept
        {
            if(!weak.expired())
            {
                m_ptr = weak.m_ptr;
                m_block = weak.m_block;
                m_block->add_shared();
            }
        }
        
    public:
        constexpr shared_ptr() noexcept {}
        constexpr shared_ptr(std::nullptr_t) noexcept {}
        
        template<typename U, typename = valid_cast<U>>
        shared_ptr(U* raw) noexcept
            : m_block(raw ? new (std::nothrow) shared_block(default_delete<U>) : nullptr), m_ptr(raw)
        {
            if(m_block)
            {
                m_block->add_shared();
                enable_weak_this(raw, raw); // aquiring a raw pointer, enable weak_this
            }
        }
        
        // used for casting only
        template <typename U> //typename = valid_cast<U>>
        shared_ptr(const shared_ptr<U>& ptr, T* p) noexcept
            : m_block(ptr.m_block), m_ptr(p)
        {
            if(m_block)
                m_block->add_shared();
        }
        
        shared_ptr(shared_ptr&& rv) noexcept
            : m_ptr(rv.m_ptr), m_block(rv.m_block)
        {
            rv.m_ptr = nullptr;
            rv.m_block = nullptr;
        }
        
        template<typename U, typename = valid_cast<U>>
        shared_ptr(shared_ptr<U>&& rv) noexcept
            : m_ptr(rv.m_ptr), m_block(rv.m_block)
        {
            rv.m_ptr = nullptr;
            rv.m_block = nullptr;
        }
        
        
        shared_ptr& operator=(shared_ptr&& rv) noexcept
        {
            shared_ptr(std::move(rv)).swap(*this);
            return *this;
        }
        
        template<class U, typename = valid_cast<U>>
        shared_ptr<T>& operator=(shared_ptr<U>&& rv)
        {
            shared_ptr(std::move(rv)).swap(*this);
            return *this;
        }

        ~shared_ptr() noexcept
        {
            if(m_block)
            {
                if(m_block->shared_count() == 1)
                {
                    //-----------------------------
                    // if m_ptr points enable_shared_this object
                    // delete m_ptr will invoke weak_ptr destructor,
                    // which will delete m_block.
                    // So we must delete m_ptr first, then release,
                    // to avoid the deletion in weak_ptr destructor
                    //-----------------------------
                    m_block->deleter((void*)m_ptr);
                    m_block->release_shared();
                    
                    if(m_block->weak_count() == 0)
                        delete m_block;
                }
                else
                {
                    m_block->release_shared();
                }
            }
        }

        shared_ptr& operator= (std::nullptr_t) noexcept
        {
            shared_ptr().swap(*this);
            return *this;
        }

        inline void reset()     noexcept { shared_ptr().swap(*this); }
        inline void reset(T* p) noexcept { shared_ptr(p).swap(*this); }
        
        /// @brief Swap method for the copy-and-swap idiom (copy constructor and swap method)
        void swap(shared_ptr<T>& other) noexcept
        {
            std::swap(m_ptr, other.m_ptr);
            std::swap(m_block, other.m_block);
        }
        
        inline long use_count() const noexcept { return m_block ? m_block->shared_count() : 0; }
        inline bool unique()    const noexcept { return use_count() == 1; }
        inline T*   get()       const noexcept { return m_ptr; }
        
        explicit inline operator bool() const noexcept { return use_count() >= 1; }
        
        inline typename std::add_lvalue_reference<element_type>::type
        operator*() const noexcept { return *m_ptr; }
        
        inline T* operator->() const noexcept { return m_ptr; }
        
        template <typename U>
        bool owner_before(const shared_ptr<U>& other) const noexcept { return m_block < other.m_block; }
        template <typename U>
        bool owner_before(const weak_ptr<U>& other) const noexcept {return m_block < other.m_block; }
        
        
        template<typename U>
        inline static void manual_destruct(void* ptr) { ((U*)ptr)->~U(); }
        
        template<typename U>
        inline static void default_delete(void* ptr) { delete (U*)ptr; }
        
        static inline size_t getAlignedSize(size_t base, size_t align)
        {
            size_t extra = (size_t)base % align;
            size_t padding = extra > 0 ? align - extra : 0;
            return base + padding;
        }
        
        // statics
        template<typename ...Args>
        static shared_ptr<T> make_shared(Args&& ... args)
        {
            //--------------------------------------------------
            // alignment is different for different classes and
            // on different cpus. alignof gives the alignment
            // requirement of the class
            //--------------------------------------------------
//            shared_ptr<T> ptr;
//            size_t aligned_block_size = getAlignedSize(sizeof(shared_block), alignof(T));
//            uint8_t* mem = (uint8_t*)operator new(aligned_block_size + sizeof(T));
//            ptr.m_block = new (mem) shared_block(manual_destruct<T>);
//            ptr.m_ptr = new (mem+aligned_block_size) T(std::forward<Args>(args)...);
//            if(ptr.m_block)
//                ptr.m_block->add_shared();
//            ptr.enable_weak_this(ptr.m_ptr, ptr.m_ptr);
//            return ptr;
            
            return shared_ptr<T>(new T(std::forward<Args>(args)...)); // no extra handling
        }
        
        // friends
        template<typename U> friend class shared_ptr;
        template<typename U> friend class weak_ptr;
        template<typename U> friend class enable_shared_from_this;
    };

    // comparison operators
    template<typename T, typename U> inline bool operator== (const shared_ptr<T>& l, const shared_ptr<U>& r) noexcept { return (l.get() == r.get()); }
    template<typename T>             inline bool operator== (const shared_ptr<T>& l, std::nullptr_t) noexcept { return l.get() == nullptr; }
    template<typename T>             inline bool operator== (std::nullptr_t, const shared_ptr<T>& r) noexcept { return r.get() == nullptr; }

    template<typename T, typename U> inline bool operator!= (const shared_ptr<T>& l, const shared_ptr<U>& r) noexcept { return (l.get() != r.get()); }
    template<typename T>             inline bool operator!= (const shared_ptr<T>& l, std::nullptr_t) noexcept { return l.get() != nullptr; }
    template<typename T>             inline bool operator!= (std::nullptr_t, const shared_ptr<T>& r) noexcept { return r.get() != nullptr; }

    template<typename T, typename U> inline bool operator<  (const shared_ptr<T>& l, const shared_ptr<U>& r) noexcept { return l.get() <  r.get(); }
    template<typename T>             inline bool operator<  (const shared_ptr<T>& l, std::nullptr_t) noexcept { return std::less<T*>()(l.get(), nullptr); }
    template<typename T>             inline bool operator<  (std::nullptr_t, const shared_ptr<T>& r) noexcept { return std::less<T*>()(nullptr, r.get()); }
            
    template<typename T, typename U> inline bool operator<= (const shared_ptr<T>& l, const shared_ptr<U>& r) noexcept { return l.get() <= r.get(); }
    template<typename T>             inline bool operator<= (const shared_ptr<T>& l, std::nullptr_t) noexcept { return std::less_equal<T*>()(l.get(), nullptr); }
    template<typename T>             inline bool operator<= (std::nullptr_t, const shared_ptr<T>& r) noexcept { return std::less_equal<T*>()(nullptr, r.get()); }
            
    template<typename T, typename U> inline bool operator>  (const shared_ptr<T>& l, const shared_ptr<U>& r) noexcept { return l.get() > r.get(); }
    template<typename T>             inline bool operator>  (const shared_ptr<T>& l, std::nullptr_t) noexcept { return std::greater<T*>()(l.get(), nullptr); }
    template<typename T>             inline bool operator>  (std::nullptr_t, const shared_ptr<T>& r) noexcept { return std::greater<T*>()(nullptr, r.get()); }
            
    template<typename T, typename U> inline bool operator>= (const shared_ptr<T>& l, const shared_ptr<U>& r) noexcept { return l.get() >= r.get(); }
    template<typename T>             inline bool operator>= (std::nullptr_t, const shared_ptr<T>& r) noexcept { return std::greater_equal<T*>()(nullptr, r.get()); }
    template<typename T>             inline bool operator>= (const shared_ptr<T>& l, std::nullptr_t) noexcept { return std::greater_equal<T*>()(l.get(), nullptr); }
            

    template<typename T, typename U>
    using array_consistency_check = typename std::enable_if<std::is_array<T>::value == std::is_array<U>::value>::type;
            
    // static cast of shared_ptr
    template<typename T, typename U, typename = typename std::enable_if<
        !std::is_array<T>::value && !std::is_array<U>::value,
        shared_ptr<T>>::type>
    shared_ptr<T> static_pointer_cast(const shared_ptr<U>& ptr) noexcept
    {
        return shared_ptr<T>(ptr, static_cast<T*>(ptr.get()));
    }
    
    // dynamic cast of shared_ptr
    template<typename T, typename U, typename = typename std::enable_if<
        !std::is_array<T>::value && !std::is_array<U>::value,
        shared_ptr<T>>::type>
    shared_ptr<T> dynamic_pointer_cast(const shared_ptr<U>& ptr) noexcept
    {
        T* p = dynamic_cast<T*>(ptr.get());
        return p ? shared_ptr<T>(ptr, p) : shared_ptr<T>();
    }
            
            
    template<class T, class U, typename = typename std::enable_if<
        std::is_array<T>::value == std::is_array<U>::value,
        shared_ptr<T>>::type>
    shared_ptr<T> const_pointer_cast(const shared_ptr<U>& ptr) noexcept
    {
        typedef typename std::remove_extent<T>::type RT;
        return shared_ptr<T>(ptr, const_cast<RT*>(ptr.get()));
    }

    template<typename T>
    inline void swap(shared_ptr<T>& x, shared_ptr<T>& y) noexcept
    {
        x.swap(y);
    }

    template<typename T, typename ...Args>
    inline shared_ptr<T> make_shared(Args&& ... args)
    {
        return shared_ptr<T>::make_shared(std::forward<Args>(args)...);
    }
    
    //-----------------------------
    // weak_ptr implementations
    //-----------------------------
    template<typename T>
    template<typename U, typename>
    weak_ptr<T>::weak_ptr(const shared_ptr<U>& owned) noexcept
    {
        if(owned)
        {
            m_ptr = owned.m_ptr;
            m_block = owned.m_block;
            if(m_block)
                m_block->add_weak();
        }
    }
            
    template<typename T>
    template<typename U, typename>
    weak_ptr<T>& weak_ptr<T>::operator= (const shared_ptr<U>& other) noexcept
    {
        weak_ptr(other).swap(*this);
        return *this;
    }
        
    template<typename T>
    shared_ptr<T> weak_ptr<T>::lock()
    {
        return shared_ptr<T>(*this);
    }
            
    template<typename T>
    shared_ptr<T> weak_ptr<T>::lock() const
    {
        return shared_ptr<T>(*this);
    }

    // pointer casts
    template<class T, class U, typename = array_consistency_check<T, U>>
    weak_ptr<T> static_pointer_cast(const weak_ptr<U>& ptr) noexcept
    {
        return weak_ptr<T>(ptr, static_cast<T*>(ptr.lock().get()));
    }
    
    template<class T, class U, typename = array_consistency_check<T, U>>
    weak_ptr<T> dynamic_pointer_cast(const weak_ptr<U>& ptr) noexcept
    {
        T* raw = dynamic_cast<T*>(ptr.lock().get());
        return raw ? weak_ptr<T>(ptr, raw) : weak_ptr<T>();
    }
    
    template<class T, class U, typename = array_consistency_check<T, U>>
    weak_ptr<T> const_pointer_cast(const weak_ptr<U>& ptr) noexcept
    {
        typedef typename std::remove_extent<T>::type RT;
        return weak_ptr<T>(ptr, const_cast<RT*>(ptr.lock().get()));
    }
            
            
    template <class _Tp = void>
    struct owner_less
    {
        template <class T, class U>
        bool operator()( shared_ptr<T> const& x, shared_ptr<U> const& y) const noexcept { return x.owner_before(y); }

        template <class T, class U>
        bool operator()( shared_ptr<T> const& x, weak_ptr<U> const& y) const noexcept { return x.owner_before(y); }

        template <class T, class U>
        bool operator()( weak_ptr<T> const& x, shared_ptr<U> const& y) const noexcept { return x.owner_before(y); }

        template <class T, class U>
        bool operator()( weak_ptr<T> const& x, weak_ptr<U> const& y) const noexcept { return x.owner_before(y); }

        typedef void is_transparent;
    };
            
    template <typename T>
    struct owner_less<shared_ptr<T>>// : std::binary_function<cr::shared_ptr<T>, cr::shared_ptr<T>, bool>            // compability issue with vs 2017
    {
        typedef bool result_type;
        bool operator()(shared_ptr<T> const& x, shared_ptr<T> const& y) const noexcept { return x.owner_before(y); }
        bool operator()(shared_ptr<T> const& x, weak_ptr<T> const& y)   const noexcept { return x.owner_before(y); }
        bool operator()(weak_ptr<T> const& x,   shared_ptr<T> const& y) const noexcept { return x.owner_before(y); }
    };
    
    template <typename T>
    struct owner_less<weak_ptr<T>>// : std::binary_function<cr::weak_ptr<T>, cr::weak_ptr<T>, bool>            // compability issue with vs 2017
    {
        typedef bool result_type;

        bool operator()(weak_ptr<T> const& x,   weak_ptr<T> const& y) const noexcept { return x.owner_before(y); }
        bool operator()(shared_ptr<T> const& x, weak_ptr<T> const& y) const noexcept { return x.owner_before(y); }
        bool operator()(weak_ptr<T> const& x, shared_ptr<T> const& y) const noexcept { return x.owner_before(y); }
    };
            
    template<typename T>
    inline void swap(weak_ptr<T>& x, weak_ptr<T>& y) noexcept
    {
        x.swap(y);
    }
            
    //-----------------------------
    // enable_shared_from_this
    //-----------------------------
    template<typename T>
    class enable_shared_from_this
    {
        mutable weak_ptr<T> weak_this;
    protected:
        enable_shared_from_this() noexcept {}
        enable_shared_from_this(enable_shared_from_this const&) noexcept {}

        enable_shared_from_this& operator=(enable_shared_from_this const&) noexcept { return *this; }
        
        virtual ~enable_shared_from_this() {}
        
    public:
        shared_ptr<T> shared_from_this() noexcept { return shared_ptr<T>(weak_this); }
        shared_ptr<T const> shared_from_this() const noexcept { return shared_ptr<const T>(weak_this); }
        
        weak_ptr<T> weak_from_this() noexcept { return weak_this; }
        weak_ptr<const T> weak_from_this() const noexcept { return weak_this; }
        
        template <typename U> friend class shared_ptr;
   };

    template<typename T>
    template<typename U, typename R, typename>
    void shared_ptr<T>::enable_weak_this(const enable_shared_from_this<U>* ptr, R* raw) noexcept
    {
        typedef typename std::remove_cv<U>::type RU;
        if(ptr && ptr->weak_this.expired())
        {
            ptr->weak_this = shared_ptr<RU>(*this, const_cast<RU*>(static_cast<const U*>(raw)));
        }
    }
}
     
namespace std
{
    //-------------------------------------------------------------
    // A set of std extensions for cr::shared_ptr & cr::weak_ptr
    //-------------------------------------------------------------
    
    // overload hash for shared_ptr
    template <typename T>
    struct hash<cr::smart_ptrs::move_only::shared_ptr<T>>
    {
        typedef cr::smart_ptrs::move_only::shared_ptr<T>      argument_type;
        typedef size_t                 result_type;
        
        result_type operator()(const argument_type& __ptr) const noexcept
        {
            return hash<T*>()(__ptr.get());
        }
    };
    
    // overload swaps
    template<typename T>
    inline void swap(cr::smart_ptrs::move_only::shared_ptr<T>& x, cr::smart_ptrs::move_only::shared_ptr<T>& y) noexcept { x.swap(y); }
    template<typename T>
    inline void swap(cr::smart_ptrs::move_only::weak_ptr<T>& x, cr::smart_ptrs::move_only::weak_ptr<T>& y) noexcept { x.swap(y); }
}
