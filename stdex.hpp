/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2014 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_FUNCTION_HPP_INCLUDED
#define STDEX_FUNCTION_HPP_INCLUDED


#include <functional>
#include <type_traits>
#include <typeinfo>
// std::is_trivially_move_constructible is not well supported, so I resort to
// Boost here :/
#include <boost/type_traits/has_trivial_move_constructor.hpp>


namespace stdex
{
    template<class F, F* f>
    struct function_wrapper;
    
    template<class R, class... Ts, R(*f)(Ts...)>
    struct function_wrapper<R(Ts...), f>
    {
        template<class... As>
        R operator()(As&&... as) const
        {
            return f(std::forward<As>(as)...);
        }
    };
    
    template<class T, class F, F(T::*f)>
    struct method_wrapper;
    
    template<class T, class R, class... Ts, R(T::*f)(Ts...)>
    struct method_wrapper<T, R(Ts...), f>
    {
        explicit method_wrapper(T* that)
          : that(that)
        {}

        template<class... As>
        R operator()(As&&... as) const
        {
            return (that->*f)(std::forward<As>(as)...);
        }
        
    private:
        
        T* that;
    };
}

namespace stdex { namespace detail
{
    template<class R, class... Ts>
    R bad_call(void* /*data*/, void*, Ts... /*args*/)
    {
        throw std::bad_function_call();
    }
    
    enum class ctrl_code
    {
        del, copy, move, get
    };
    
    inline bool null_ctrl(void** /*data*/, void** /*dst*/, ctrl_code /*code*/)
    {
        return false;
    }
    
    template<class T>
    struct is_emplaceable
      : std::integral_constant<bool, sizeof(T) <= sizeof(void*)
            && alignof(void*) % alignof(T) == 0
            && std::is_nothrow_move_constructible<T>::value
            && std::is_nothrow_destructible<T>::value>
    {};
    
    template<class F, class Alloc = std::allocator<void>, class = void>
    struct function_manager
    {
        struct wrapper
          : Alloc::template rebind<wrapper>::other, F
        {
            typedef typename Alloc::template rebind<wrapper>::other alloc_base;
            using F::operator();
            
            wrapper(alloc_base&& alloc, F&& f)
              : alloc_base(std::move(alloc)), F(std::move(f))
            {}
        };
        
        static void create(void** data, F& f, Alloc const& alloc = Alloc())
        {
            typename wrapper::alloc_base a(alloc);
            *data = new (a.allocate(1)) wrapper(std::move(a), std::move(f));
        }
        
        template<class R, class... Ts>
        static R fwd(void* data, void*, Ts... args)
        {
            return (*static_cast<wrapper*>(data))(std::forward<Ts>(args)...);
        }
        
        static bool ctrl(void** src, void** dst, ctrl_code code)
        {
            wrapper* data = static_cast<wrapper*>(*src);
            switch (code)
            {
            case ctrl_code::copy:
                {
                    typename wrapper::alloc_base a(*data);
                    *dst = new(a.allocate(1))
                        wrapper(std::move(a), F(static_cast<F&&>(*data)));
                    break;
                }
            case ctrl_code::move:
                *dst = *src;
                break;
            case ctrl_code::del:
                {
                    typename wrapper::alloc_base a(std::move(*data));
                    data->~wrapper();
                    a.deallocate(data, 1);
                    break;
                }
            case ctrl_code::get:
                *dst = const_cast<void*>(static_cast<void const*>(&typeid(F)));
                *++dst = static_cast<F*>(data);
            }
            return true;
        }
    };
    
    template<class F>
    struct fwd_emplaceable
    {
        template<class R, class... Ts>
        static R fwd(void*, void* data, Ts... args)
        {
            return (*static_cast<F*>(data))(std::forward<Ts>(args)...);
        }
    };
    
    template<class F>
    struct fwd_emplaceable<F*>
    {
        template<class R, class... Ts>
        static R fwd(void* fp, void*, Ts... args)
        {
            return reinterpret_cast<F*>(fp)(std::forward<Ts>(args)...);
        }
    };
    
    template<class F, F* f>
    struct fwd_emplaceable<function_wrapper<F, f> >
    {
        template<class R, class... Ts>
        static R fwd(void*, void*, Ts... args)
        {
            return f(std::forward<Ts>(args)...);
        }
    };
    
    template<class T, class F, F(T::*f)>
    struct fwd_emplaceable<method_wrapper<T, F, f> >
    {
        template<class R, class... Ts>
        static R fwd(void* data, void*, Ts... args)
        {
            return (static_cast<T*>(data)->*f)(std::forward<Ts>(args)...);
        }
    };

    template<class F, class Alloc>
    struct function_manager<F, Alloc,
        typename std::enable_if<is_emplaceable<F>::value>::type>
      : fwd_emplaceable<F>
    {
        static void create(void** data, F& f, Alloc const& = Alloc())
        {
            new(data) F(std::move(f));
        }

        static bool ctrl(void** src, void** dst, ctrl_code code)
        {
            F* data = static_cast<F*>(static_cast<void*>(src));
            switch (code)
            {
            case ctrl_code::copy:
                new(dst) F(*data);
                break;
            case ctrl_code::move:
                new(dst) F(std::move(*data));
                if (boost::has_trivial_move_constructor<F>::value)
                    return false;
                // no break
            case ctrl_code::del:
                data->~F();
                break;
            case ctrl_code::get:
                *dst = const_cast<void*>(static_cast<void const*>(&typeid(F)));
                *++dst = data;
            }
            return true;
        }
    };
    
    template<class F>
    struct trampoline
    {
        trampoline() {}

        trampoline(F* f)
          : f(f)
        {}
    
        F* f;
    };
}}

namespace stdex
{
    template<class... Sig>
    class function;
    
    template<>
    class function<>
    {
    protected:
        
        struct internal_tag {};
        
        template<class T>
        struct is_subset_of
          : std::integral_constant<bool, true>
        {};
        
        explicit function(internal_tag) {}
        
        template<class F, class Alloc>
        function(internal_tag, F& f, Alloc const& alloc)
          : _ctrl(detail::function_manager<F, Alloc>::ctrl)
        {
            detail::function_manager<F, Alloc>::create(&_data, f, alloc);
        }
        
        template<class... Sig>
        function(internal_tag, function<Sig...>& other, bool& moved)
          : _ctrl(other._ctrl)
        {
            moved = _ctrl(&other._data, &_data, detail::ctrl_code::move);
        }
        
    public:
        
        function() noexcept
          : _ctrl(detail::null_ctrl)
        {}
    
        function(std::nullptr_t) noexcept
          : _ctrl(detail::null_ctrl)
        {}
    
        template<class F, class Alloc = std::allocator<void> >
        function(F f, Alloc const& alloc = Alloc())
          : _ctrl(detail::function_manager<F, Alloc>::ctrl)
        {
            detail::function_manager<F, Alloc>::create(&_data, f, alloc);
        }
        
        template<class R2, class... T2s>
        function(R2(*p)(T2s...)) noexcept
        {
            if (p)
                init_raw(p);
            else
                init_null();
        }
    
        // copy
        function(function const& other)
          : _ctrl(other._ctrl)
        {
            _ctrl(&other._data, &_data, detail::ctrl_code::copy);
        }
    
        // copy from superset
        template<class... Sig>
        function(function<Sig...> const& other)
          : _ctrl(other._ctrl)
        {
            _ctrl(&other._data, &_data, detail::ctrl_code::copy);
        }
        
        // move / move from superset
        template<class... Sig>
        function(function<Sig...>&& other) noexcept
          : _ctrl(other._ctrl)
        {
            if (_ctrl(&other._data, &_data, detail::ctrl_code::move))
                other.init_null();
        }
        
        function& operator=(function other) noexcept
        {
            swap(other);
            return *this;
        }
        
        void swap(function& other) noexcept
        {
            void* tmp;
            _ctrl(&_data, &tmp, detail::ctrl_code::move);
            other._ctrl(&other._data, &_data, detail::ctrl_code::move);
            _ctrl(&tmp, &other._data, detail::ctrl_code::move);
            std::swap(_ctrl, other._ctrl);
        }
                
        ~function()
        {
            _ctrl(&_data, nullptr, detail::ctrl_code::del);
        }
        
        explicit operator bool() const noexcept
        {
            return _ctrl != detail::null_ctrl;
        }

        std::type_info const& target_type() const
        {
            void* ret[2] =
                {const_cast<void*>(static_cast<void const*>(&typeid(void)))};
            _ctrl(&_data, ret, detail::ctrl_code::get);
            return *static_cast<std::type_info const*>(ret[0]);
        }
        
        template<class T> 
        T* target() noexcept
        {
            void* ret[2] =
                {const_cast<void*>(static_cast<void const*>(&typeid(void)))};
            _ctrl(&_data, ret, detail::ctrl_code::get);
            if (*static_cast<std::type_info const*>(ret[0]) == typeid(T))
                return static_cast<T*>(ret[1]);
            else
                return nullptr;
        }
    
        template<class T>
        T const* target() const noexcept
        {
            return const_cast<function*>(this)->template target<T>();
        }
    
        // dummy
        template<class T>
        void operator()();
    
    protected:
    
        template<class F>
        void init_raw(F f)
        {
            _ctrl = detail::function_manager<F>::ctrl;
            detail::function_manager<F>::create(&_data, f);
        }
        
        void init_null()
        {
            _ctrl = detail::null_ctrl;
        }
        
        void steal(function& other)
        {
            swap(other);
        }
        
        bool (*_ctrl)(void**, void**, detail::ctrl_code);
        mutable void* _data; // may store small object inplace
    };

    template<class R, class... Ts, class... Rest>
    class function<R(Ts...), Rest...>
      : function<Rest...> // avoid slicing
      , public detail::trampoline<R(void*, void*, Ts...)>
    {
        typedef function<Rest...> base_type;
        typedef detail::trampoline<R(void*, void*, Ts...)> caller;
        
    protected:

        typedef typename base_type::internal_tag internal_tag;
        
        template<class T>
        struct is_subset_of
          : std::integral_constant<bool, std::is_base_of<caller, T>::value
             && base_type::template is_subset_of<T>::value>
        {};
        
        explicit function(internal_tag tag)
          : base_type(tag)
        {}
        
        template<class F, class Alloc>
        function(internal_tag tag, F& f, Alloc const& alloc)
          : base_type(tag, f, alloc)
          , caller(detail::function_manager<F, Alloc>::template fwd<R, Ts...>)
        {}
    
        template<class... Sig>
        function(internal_tag tag, function<Sig...>& other, bool& moved)
          : base_type(tag, other, moved), caller(other)
        {}
    
    public:
        
        function() noexcept
          : caller(detail::bad_call<R, Ts...>)
        {}
    
        function(std::nullptr_t) noexcept
          : caller(detail::bad_call<R, Ts...>)
        {}
        
        template<class F, class Alloc = std::allocator<void> >
        function(F f, Alloc const& alloc = Alloc())
          : base_type(internal_tag(), f, alloc)
          , caller(detail::function_manager<F, Alloc>::template fwd<R, Ts...>)
        {}
        
        template<class R2, class... T2s>
        function(R2(*p)(T2s...)) noexcept
          : base_type(internal_tag())
        {
            if (p)
                init_raw(p);
            else
                init_null();
        }

        // copy
        function(function const& other)
          : base_type(static_cast<base_type const&>(other)), caller(other)
        {}

        // copy from superset
        template<class... Sig>
        function(function<Sig...> const& other, typename std::enable_if<
            is_subset_of<function<Sig...> >::value>::type* = 0)
          : base_type(other), caller(other)
        {}
        
        // move / move from superset
        template<class... Sig>
        function(function<Sig...>&& other, typename std::enable_if<is_subset_of<
            function<Sig...> >::value, bool>::type moved = false) noexcept
          : base_type(internal_tag(), other, moved), caller(other)
        {
            if (moved)
                other.init_null();
        }

        function& operator=(function other) noexcept
        {
            steal(other);
            return *this;
        }
        
        void swap(function& other) noexcept
        {
            base_type::swap(other);
            std::swap(caller::f, other.caller::f);
        }
    
        R operator()(Ts... args) const
        {
            return
                caller::f(this->_data, &this->_data, std::forward<Ts>(args)...);
        }
            
        using base_type::operator();
        using base_type::operator bool;
        using base_type::target_type;
        using base_type::target;

    protected:
    
        template<class... Sig>
        friend class function;
        
        template<class F>
        void init_raw(F f)
        {
            caller::f = detail::function_manager<F>::template fwd<R, Ts...>;
            base_type::init_raw(f);
        }
        
        void init_null()
        {
            caller::f = detail::bad_call<R, Ts...>;
            base_type::init_null();
        }
        
        void steal(function& other) noexcept
        {
            base_type::steal(other);
            caller::f = other.caller::f;
        }
    };
    
    template<class... Sig>
    inline void swap(function<Sig...>& a, function<Sig...>& b)
    {
        a.swap(b);
    }
    
    template<class... Sig>
    inline bool operator==(function<Sig...> const& f, std::nullptr_t)
    {
        return !f;
    }
    
    template<class... Sig>
    inline bool operator==(std::nullptr_t, function<Sig...> const& f)
    {
        return !f;
    }
    
    template<class... Sig>
    inline bool operator!=(function<Sig...> const& f, std::nullptr_t)
    {
        return f;
    }
    
    template<class... Sig>
    inline bool operator!=(std::nullptr_t, function<Sig...> const& f)
    {
        return f;
    }
}


#endif

