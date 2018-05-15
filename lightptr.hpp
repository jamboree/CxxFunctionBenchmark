#ifndef GNR_LIGHTPTR_HPP
# define GNR_LIGHTPTR_HPP
# pragma once

#include <cassert>

#include <atomic>

#include <memory>

#include <type_traits>

#include <utility>

namespace gnr
{

namespace
{

using counter_type = unsigned;

using atomic_counter_type = std::atomic<counter_type>;

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

template <typename U>
using ref_type_t = typename ref_type<U>::type;

}

template <typename T>
class light_ptr
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

  template <typename U, typename V, std::size_t N>
  struct deletion_type<U[N], V>
  {
    using type = V[];
  };

  template <typename ...A>
  using deletion_type_t = typename deletion_type<A...>::type;

  using element_type = std::remove_extent_t<T>;

  class counter_base
  {
    friend class light_ptr;

    using invoker_type = void (*)(counter_base*, element_type*) noexcept;

    atomic_counter_type counter_{};

    invoker_type const invoker_;

  protected:
    explicit counter_base(counter_type const c,
      invoker_type const invoker) noexcept :
      counter_(c),
      invoker_(invoker)
    {
    }

  public:
    template <typename U>
    std::enable_if_t<!std::is_void<U>{}> dec_ref(U* const ptr) noexcept
    {
      if (counter_type(1) ==
        counter_.fetch_sub(counter_type(1), std::memory_order_relaxed))
      {
        using type_must_be_complete = char[sizeof(U) ? 1 : -1];
        (void)sizeof(type_must_be_complete);
        invoker_(this, ptr);
      }
      // else do nothing
    }

    template <typename U>
    std::enable_if_t<std::is_void<U>{}> dec_ref(U* const ptr) noexcept
    {
      if (counter_type(1) ==
        counter_.fetch_sub(counter_type(1), std::memory_order_relaxed))
      {
        invoker_(this, ptr);
      }
      // else do nothing
    }

    void inc_ref() noexcept
    {
      counter_.fetch_add(counter_type(1), std::memory_order_relaxed);
    }
  };

  template <typename D>
  class counter : public counter_base
  {
    std::decay_t<D> const d_;

    static void invoked(counter_base* const ptr,
      element_type* const e) noexcept
    {
      auto const c(static_cast<counter<D>*>(ptr));

      // invoke deleter on the element
      c->d_(e);

      // delete from a static member function
      delete c;
    }

  public:
    explicit counter(counter_type const c, D&& d) noexcept :
      counter_base(c, invoked),
      d_(std::forward<D>(d))
    {
    }
  };

private:
  template <typename U> friend struct std::hash;

  counter_base* counter_{};

  element_type* ptr_;

public:
  light_ptr() = default;

  template <typename U>
  explicit light_ptr(U* const p)
  {
    reset(p);
  }

  template <typename U, typename D>
  explicit light_ptr(U* const p, D&& d)
  {
    reset(p, std::forward<D>(d));
  }

  light_ptr(light_ptr const& other) noexcept { *this = other; }

  light_ptr(light_ptr&& other) noexcept { *this = std::move(other); }

  ~light_ptr() noexcept
  {
    if (counter_)
    {
      counter_->dec_ref(ptr_);
    }
    // else do nothing
  }

  light_ptr& operator=(light_ptr const& rhs) noexcept
  {
    if (*this != rhs)
    {
      if (counter_)
      {
        counter_->dec_ref(ptr_);
      }
      // else do nothing

      if ((counter_ = rhs.counter_))
      {
        counter_->inc_ref();
      }
      // else do nothing

      ptr_ = rhs.ptr_;
    }
    // else do nothing

    return *this;
  }

  light_ptr& operator=(light_ptr&& rhs) noexcept
  {
    counter_ = rhs.counter_;
    rhs.counter_ = nullptr;

    ptr_ = rhs.ptr_;

    return *this;
  }

  light_ptr& operator=(std::nullptr_t const) noexcept
  {
    reset();

    return *this;
  }

  bool operator<(light_ptr const& rhs) const noexcept
  {
    return counter_ < rhs.counter_;
  }

  bool operator==(light_ptr const& rhs) const noexcept
  {
    return counter_ == rhs.counter_;
  }

  bool operator!=(light_ptr const& rhs) const noexcept
  {
    return !operator==(rhs);
  }

  bool operator==(std::nullptr_t const) const noexcept { return !counter_; }

  bool operator!=(std::nullptr_t const) const noexcept { return counter_; }

  explicit operator bool() const noexcept { return counter_; }

  ref_type_t<T> operator*() const noexcept
  {
    return *reinterpret_cast<T*>(ptr_);
  }

  auto operator->() const noexcept { return reinterpret_cast<T*>(ptr_); }

  template <typename U = T,
    typename = std::enable_if_t<std::is_array<U>{}>
  >
  ref_type_t<element_type> operator[](std::size_t const i) const noexcept
  {
    return ptr_[i];
  }

  auto get() const noexcept { return ptr_; }

  void reset() noexcept { reset(nullptr); }

  void reset(std::nullptr_t const) noexcept
  {
    if (counter_)
    {
      counter_->dec_ref(ptr_);

      counter_ = nullptr;
    }
    // else do nothing
  }

  template <typename U>
  void reset(U* const p)
  {
    reset(p,
      [](element_type* const p) noexcept {
        std::default_delete<deletion_type_t<T, U>>()(
          static_cast<U*>(p)
        );
      }
    );
  }

  template <typename U, typename D>
  void reset(U* const p, D&& d)
  {
    if (counter_)
    {
      counter_->dec_ref(ptr_);
    }
    // else do nothing

    counter_ = p ?
      new counter<D>(counter_type(1), std::forward<D>(d)) :
      nullptr;

    ptr_ = p;
  }

  void swap(light_ptr& other) noexcept
  {
    std::swap(counter_, other.counter_);
    std::swap(ptr_, other.ptr_);
  }

  bool unique() const noexcept
  {
    return counter_type(1) == use_count();
  }

  counter_type use_count() const noexcept
  {
    return counter_ ?
      counter_->counter_.load(std::memory_order_relaxed) :
      counter_type{};
  }
};

template<class T, typename ...A>
inline light_ptr<T> make_light(A&& ...args)
{
  return light_ptr<T>(new T(std::forward<A>(args)...));
}

}

namespace std
{
  template <typename T>
  struct hash<gnr::light_ptr<T> >
  {
    size_t operator()(gnr::light_ptr<T> const& l) const noexcept
    {
      return hash<typename gnr::light_ptr<T>::element_type*>()(l.ptr_);
    }
  };
}

#endif // GNR_LIGHTPTR_HPP
