#pragma once
#ifndef LIGHTPTR_HPP
# define LIGHTPTR_HPP

#include <cassert>

#include <atomic>

#include <memory>

#include <utility>

#include <type_traits>

namespace generic
{

namespace detail
{
  using counter_type = unsigned;

  using atomic_type = ::std::atomic<counter_type>;

  template <typename T>
  using deleter_type = void (*)(T*);

  template <typename U>
  struct ref_type
  {
    using type = U&;
  };

  template <>
  struct ref_type<void>
  {
    using type = void;
  };

  template <typename T>
  inline void dec_ref(atomic_type* const counter_ptr,
    T* const ptr, deleter_type<T> const deleter)
  {
    if (counter_ptr && (counter_type(1) ==
      counter_ptr->fetch_sub(counter_type(1), ::std::memory_order_relaxed)))
    {
      delete counter_ptr;

      typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
      (void)sizeof(type_must_be_complete);
      deleter(ptr);
    }
    // else do nothing
  }

  template <>
  inline void dec_ref(atomic_type* const counter_ptr,
    void* const ptr, deleter_type<void> const deleter)
  {
    if (counter_ptr && (counter_type(1) ==
      counter_ptr->fetch_sub(counter_type(1), ::std::memory_order_relaxed)))
    {
      delete counter_ptr;

      deleter(ptr);
    }
    // else do nothing
  }

  inline void inc_ref(atomic_type* const counter_ptr)
  {
    //assert(counter_ptr);
    counter_ptr->fetch_add(counter_type(1), ::std::memory_order_relaxed);
  }
}

template <typename T>
struct light_ptr
{
  template <typename U, typename V>
  struct deletion_type
  {
    using type = V;
  };

  template <typename U, typename V>
  struct deletion_type<U[], V>
  {
    using type = V[];
  };

  template <typename U, typename V, ::std::size_t N>
  struct deletion_type<U[N], V>
  {
    using type = V[];
  };

  template <typename U>
  struct remove_array
  {
    using type = U;
  };

  template <typename U>
  struct remove_array<U[]>
  {
    using type = U;
  };

  template <typename U, ::std::size_t N>
  struct remove_array<U[N]>
  {
    using type = U;
  };

  using element_type = typename remove_array<T>::type;

  using deleter_type = detail::deleter_type<element_type>;

  light_ptr() = default;

  template <typename U>
  explicit light_ptr(U* const p, deleter_type const d = default_deleter<U>)
  {
    reset(p, d);
  }

  ~light_ptr() { detail::dec_ref(counter_ptr_, ptr_, deleter_); }

  light_ptr(light_ptr const& other) { *this = other; }

  light_ptr(light_ptr&& other) noexcept { *this = ::std::move(other); }

  light_ptr& operator=(light_ptr const& rhs)
  {
    if (*this != rhs)
    {
      detail::dec_ref(counter_ptr_, ptr_, deleter_);

      counter_ptr_ = rhs.counter_ptr_;
      ptr_ = rhs.ptr_;

      deleter_ = rhs.deleter_;

      detail::inc_ref(counter_ptr_);
    }
    // else do nothing

    return *this;
  }

  light_ptr& operator=(light_ptr&& rhs) noexcept
  {
    if (*this != rhs)
    {
      counter_ptr_ = rhs.counter_ptr_;
      ptr_ = rhs.ptr_;

      deleter_ = rhs.deleter_;

      rhs.counter_ptr_ = nullptr;
      rhs.ptr_ = nullptr;
    }
    // else do nothing

    return *this;
  }

  light_ptr& operator=(::std::nullptr_t const) noexcept { reset(); }

  bool operator<(light_ptr const& rhs) const noexcept
  {
    return get() < rhs.get();
  }

  bool operator==(light_ptr const& rhs) const noexcept
  {
    return counter_ptr_ == rhs.counter_ptr_;
  }

  bool operator!=(light_ptr const& rhs) const noexcept
  {
    return !operator==(rhs);
  }

  bool operator==(::std::nullptr_t const) const noexcept
  {
    return !ptr_;
  }

  bool operator!=(::std::nullptr_t const) const noexcept
  {
    return ptr_;
  }

  explicit operator bool() const noexcept { return ptr_; }

  typename detail::ref_type<T>::type
  operator*() const noexcept
  {
    return *static_cast<T*>(static_cast<void*>(ptr_));
  }

  T* operator->() const noexcept
  {
    return static_cast<T*>(static_cast<void*>(ptr_));
  }

  element_type* get() const noexcept { return ptr_; }

  void reset() { reset(nullptr); }

  void reset(::std::nullptr_t const)
  {
    detail::dec_ref(counter_ptr_, ptr_, deleter_);

    counter_ptr_ = nullptr;
    ptr_ = nullptr;
  }

  template <typename U>
  void reset(U* const p, deleter_type const d = default_deleter<U>)
  {
    detail::dec_ref(counter_ptr_, ptr_, deleter_);

    counter_ptr_ = new detail::atomic_type(detail::counter_type(1));
    ptr_ = p;

    deleter_ = d;
  }

  void swap(light_ptr& other) noexcept
  {
    ::std::swap(counter_ptr_, other.counter_ptr_);
    ::std::swap(ptr_, other.ptr_);
    ::std::swap(deleter_, other.deleter_);
  }

  bool unique() const noexcept
  {
    return detail::counter_type(1) == use_count();
  }

  detail::counter_type use_count() const noexcept
  {
    return counter_ptr_ ?
      counter_ptr_->load(::std::memory_order_relaxed) :
      detail::counter_type{};
  }

  template <typename U>
  static void default_deleter(element_type* const p)
  {
    ::std::default_delete<typename deletion_type<T, U>::type>()(
      static_cast<U*>(p));
  }

private:
  detail::atomic_type* counter_ptr_{};

  element_type* ptr_{};

  deleter_type deleter_;
};

template<class T, class ...Args>
inline light_ptr<T> make_light(Args&& ...args)
{
  return light_ptr<T>(new T(::std::forward<Args>(args)...));
}

}

namespace std
{
  template <typename T>
  struct hash<::generic::light_ptr<T> >
  {
    size_t operator()(::generic::light_ptr<T> const& l) const noexcept
    {
      return hash<typename ::generic::light_ptr<T>::element_type*>()(l.get());
    }
  };
}

#endif // LIGHTPTR_HPP
