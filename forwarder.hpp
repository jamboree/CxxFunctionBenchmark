#ifndef GNR_FORWARDER_HPP
# define GNR_FORWARDER_HPP
# pragma once

#include <cassert>

// std::size_t
#include <cstddef>

#include <functional>

#include <type_traits>

#include <utility>

namespace gnr
{

namespace
{

constexpr auto const default_noexcept =
#if defined(__cpp_exceptions) && __cpp_exceptions
false;
#else
true;
#endif // __cpp_exceptions

constexpr auto const default_size = 4 * sizeof(void*);

}

template <typename F,
  std::size_t N = default_size,
  bool NE = default_noexcept
>
class forwarder;

template <typename R, typename ...A, std::size_t N, bool NE>
class forwarder<R (A...), N, NE>
{
  R (*stub_)(void*, A&&...) noexcept(NE) {};

  std::aligned_storage_t<N> store_;

  template<typename T, typename ...U, std::size_t M>
  friend bool operator==(forwarder<T (U...), M> const&,
    std::nullptr_t) noexcept;
  template<typename T, typename ...U, std::size_t M>
  friend bool operator==(std::nullptr_t,
    forwarder<T (U...), M> const&) noexcept;

  template<typename T, typename ...U, std::size_t M>
  friend bool operator!=(forwarder<T (U...), M> const&,
    std::nullptr_t) noexcept;
  template<typename T, typename ...U, std::size_t M>
  friend bool operator!=(std::nullptr_t,
    forwarder<T (U...), M> const&) noexcept;

public:
  using result_type = R;

  enum : std::size_t { size = N };

public:
  forwarder() = default;

  forwarder(forwarder const&) = default;

  forwarder(forwarder&&) = default;

  template <typename F,
    typename = std::enable_if_t<!std::is_same<std::decay_t<F>, forwarder>{}>
  >
  forwarder(F&& f) noexcept
  {
    assign(std::forward<F>(f));
  }

  forwarder& operator=(forwarder const&) = default;

  forwarder& operator=(forwarder&&) = default;

  template <typename F,
    typename = std::enable_if_t<!std::is_same<std::decay_t<F>, forwarder>{}>
  >
  forwarder& operator=(F&& f) noexcept
  {
    assign(std::forward<F>(f));

    return *this;
  }

  explicit operator bool() const noexcept { return stub_; }

  R operator()(A... args) const
    noexcept(
      noexcept(
        stub_(const_cast<void*>(static_cast<void const*>(&store_)),
          std::forward<A>(args)...
        )
      )
    )
  {
    //assert(stub_);
    return stub_(const_cast<void*>(static_cast<void const*>(&store_)),
      std::forward<A>(args)...
    );
  }

  void assign(std::nullptr_t) noexcept
  {
    reset();
  }

  template <typename F>
  void assign(F&& f) noexcept
  {
    using functor_type = std::decay_t<F>;

    static_assert(sizeof(functor_type) <= sizeof(store_),
      "functor too large");
    static_assert(std::is_trivially_copyable<functor_type>{},
      "functor not trivially copyable");

    ::new (static_cast<void*>(&store_)) functor_type(std::forward<F>(f));

    stub_ = [](void* const ptr, A&&... args) noexcept(noexcept(NE)) -> R
    {
#if __cplusplus <= 201402L
      return (*static_cast<functor_type*>(ptr))(
        std::forward<A>(args)...);
#else
      return std::invoke(*static_cast<functor_type*>(ptr),
        std::forward<A>(args)...);
#endif // __cplusplus
    };
  }

  void reset() noexcept { stub_ = nullptr; }

  void swap(forwarder& other) noexcept
  {
    std::swap(*this, other);
  }

  void swap(forwarder&& other) noexcept
  {
    std::swap(*this, std::move(other));
  }

  template <typename T>
  auto target() noexcept
  {
    return reinterpret_cast<T*>(&store_);
  }

  template <typename T> 
  auto target() const noexcept
  {
    return reinterpret_cast<T const*>(&store_);
  }
};

template<typename R, typename ...A, std::size_t N>
bool operator==(forwarder<R (A...), N> const& f,
  std::nullptr_t const) noexcept
{
  return nullptr == f.stub_ ;
}

template<typename R, typename ...A, std::size_t N>
bool operator==(std::nullptr_t const,
  forwarder<R (A...), N> const& f) noexcept
{
  return nullptr == f.stub_;
}

template<typename R, typename ...A, std::size_t N>
bool operator!=(forwarder<R (A...), N> const& f,
  std::nullptr_t const) noexcept
{
  return !operator==(nullptr, f);
}

template<typename R, typename ...A, std::size_t N>
bool operator!=(std::nullptr_t const,
  forwarder<R (A...), N> const& f) noexcept
{
  return !operator==(nullptr, f);
}

}

#endif // GNR_FORWARDER_HPP
