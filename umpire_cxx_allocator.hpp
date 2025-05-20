#ifndef UMPIRE_CXX_ALLOCATOR_HPP___INCLUDED
#define UMPIRE_CXX_ALLOCATOR_HPP___INCLUDED

#include <umpire/Umpire.hpp>
#include <umpire/ResourceManager.hpp>

#if defined(__has_include)
#  if __has_include(<madness/world/archive.h>)
#    include <madness/world/archive.h>
#    define HAVE_MADNESS_WORLD_ARCHIVE_H
#  endif
#endif

#include <cassert>
#include <memory>
#include <type_traits>

namespace umpire {

namespace detail {

struct NullLock {
  static void lock() {}
  static void unlock() {}
};

template <typename Tag>
class MutexLock {
  static std::mutex mtx_;

 public:
  static void lock() { mtx_.lock(); }
  static void unlock() { mtx_.unlock(); }
};

template <typename Tag>
std::mutex MutexLock<Tag>::mtx_;

}  // namespace detail

/// wraps a Umpire allocator into a
/// *standard-compliant* C++ allocator

/// Optionally can be made thread safe by providing an appropriate \p StaticLock
/// \details based on the boilerplate by Howard Hinnant
/// (https://howardhinnant.github.io/allocator_boilerplate.html)
/// \tparam T type of allocated objects
/// \tparam StaticLock a type providing static `lock()` and `unlock()` methods ;
///         defaults to NullLock which does not lock
template <class T, class StaticLock = detail::NullLock>
class allocator_impl {
 public:
  using value_type = T;
  using pointer = value_type*;
  using const_pointer =
      typename std::pointer_traits<pointer>::template rebind<value_type const>;
  using void_pointer =
      typename std::pointer_traits<pointer>::template rebind<void>;
  using const_void_pointer =
      typename std::pointer_traits<pointer>::template rebind<const void>;

  using reference = T&;
  using const_reference = const T&;

  using difference_type =
      typename std::pointer_traits<pointer>::difference_type;
  using size_type = std::make_unsigned_t<difference_type>;

  allocator_impl(umpire::Allocator* umpalloc) noexcept
      : umpalloc_(umpalloc) {}

  template <class U>
  allocator_impl(
      const allocator_impl<U, StaticLock>& rhs) noexcept
      : umpalloc_(rhs.umpalloc_) {}

  /// allocates memory using umpire dynamic pool
  pointer allocate(size_t n) {
    assert(umpalloc_);

    // QuickPool::allocate_internal does not handle zero-size allocations
    size_t nbytes = n == 0 ? 1 : n * sizeof(T);
    pointer result = nullptr;
    auto* allocation_strategy = umpalloc_->getAllocationStrategy();

    // critical section
    StaticLock::lock();
    // this, instead of umpalloc_->allocate(n*sizeof(T)), profiles memory use
    // even if introspection is off
    result =
        static_cast<pointer>(allocation_strategy->allocate_internal(nbytes));
    StaticLock::unlock();

    return result;
  }

  /// deallocate memory using umpire dynamic pool
  void deallocate(pointer ptr, size_t n) {
    assert(umpalloc_);

    // QuickPool::allocate_internal does not handle zero-size allocations
    const auto nbytes = n == 0 ? 1 : n * sizeof(T);
    auto* allocation_strategy = umpalloc_->getAllocationStrategy();

    // N.B. with multiple threads would have to do this test in
    // the critical section of Umpire's ThreadSafeAllocator::deallocate
    StaticLock::lock();
    assert(nbytes <= allocation_strategy->getCurrentSize());
    // this, instead of umpalloc_->deallocate(ptr, nbytes), profiles memory use
    // even if introspection is off
    allocation_strategy->deallocate_internal(ptr, nbytes);
    StaticLock::unlock();
  }

  /// @return the underlying Umpire allocator
  const umpire::Allocator* umpire_allocator() const { return umpalloc_; }

 private:
  umpire::Allocator* umpalloc_;
};  // class allocator_impl

template <class T1, class T2, class StaticLock>
bool operator==(
    const allocator_impl<T1, StaticLock>& lhs,
    const allocator_impl<T2, StaticLock>& rhs) noexcept {
  return lhs.umpire_allocator() == rhs.umpire_allocator();
}

template <class T1, class T2, class StaticLock>
bool operator!=(
    const allocator_impl<T1, StaticLock>& lhs,
    const allocator_impl<T2, StaticLock>& rhs) noexcept {
  return !(lhs == rhs);
}

template <class T, class StaticLock, typename UmpireAllocatorAccessor>
class allocator
    : public allocator_impl<T, StaticLock> {
 public:
  using base_type = allocator_impl<T, StaticLock>;
  using typename base_type::const_pointer;
  using typename base_type::const_reference;
  using typename base_type::pointer;
  using typename base_type::reference;
  using typename base_type::value_type;

  allocator() noexcept : base_type(&UmpireAllocatorAccessor{}()) {}

  template <class U>
  allocator(
      const allocator<U, StaticLock, UmpireAllocatorAccessor>&
          rhs) noexcept
      : base_type(
            static_cast<const allocator_impl<U, StaticLock>&>(
                rhs)) {}

  template <typename T1, typename T2, class StaticLock_,
            typename UmpireAllocatorAccessor_>
  friend bool operator==(
      const allocator<T1, StaticLock_, UmpireAllocatorAccessor_>&
          lhs,
      const allocator<T2, StaticLock_, UmpireAllocatorAccessor_>&
          rhs) noexcept;
};  // class allocator

template <class T1, class T2, class StaticLock,
          typename UmpireAllocatorAccessor>
bool operator==(
    const allocator<T1, StaticLock, UmpireAllocatorAccessor>& lhs,
    const allocator<T2, StaticLock, UmpireAllocatorAccessor>&
        rhs) noexcept {
  return lhs.umpire_allocator() == rhs.umpire_allocator();
}

template <class T1, class T2, class StaticLock,
          typename UmpireAllocatorAccessor>
bool operator!=(
    const allocator<T1, StaticLock, UmpireAllocatorAccessor>& lhs,
    const allocator<T2, StaticLock, UmpireAllocatorAccessor>&
        rhs) noexcept {
  return !(lhs == rhs);
}

/// see
/// https://stackoverflow.com/questions/21028299/is-this-behavior-of-vectorresizesize-type-n-under-c11-and-boost-container/21028912#21028912
template <typename T, typename A>
class default_init_allocator : public A {
  using a_t = std::allocator_traits<A>;

 public:
  using reference = typename A::reference;  // std::allocator<T>::reference
                                            // deprecated in C++17, but thrust
                                            // still relying on this
  using const_reference = typename A::const_reference;  // ditto

  template <typename U>
  struct rebind {
    using other =
        default_init_allocator<U, typename a_t::template rebind_alloc<U>>;
  };

  using A::A;

  default_init_allocator(A const& a) noexcept : A(a) {}
  default_init_allocator(A&& a) noexcept : A(std::move(a)) {}

  template <typename U>
  void construct(U* ptr) noexcept(
      std::is_nothrow_default_constructible<U>::value) {
    ::new (static_cast<void*>(ptr)) U;
  }
  template <typename U, typename... Args>
  void construct(U* ptr, Args&&... args) {
    a_t::construct(static_cast<A&>(*this), ptr, std::forward<Args>(args)...);
  }
};

}  // namespace umpire

#ifdef HAVE_MADNESS_WORLD_ARCHIVE_H
namespace madness {
namespace archive {

template <class Archive, class T, class StaticLock>
struct ArchiveLoadImpl<Archive,
                       umpire::allocator_impl<T, StaticLock>> {
  static inline void load(
      const Archive& ar,
      umpire::allocator_impl<T, StaticLock>& allocator) {
    std::string allocator_name;
    ar & allocator_name;
    allocator = umpire::allocator_impl<T, StaticLock>(
        umpire::ResourceManager::getInstance().getAllocator(allocator_name));
  }
};

template <class Archive, class T, class StaticLock>
struct ArchiveStoreImpl<
    Archive, umpire::allocator_impl<T, StaticLock>> {
  static inline void store(
      const Archive& ar,
      const umpire::allocator_impl<T, StaticLock>& allocator) {
    ar & allocator.umpire_allocator()->getName();
  }
};

template <class Archive, typename T, typename A>
struct ArchiveLoadImpl<Archive, umpire::default_init_allocator<T, A>> {
  static inline void load(const Archive& ar,
                          umpire::default_init_allocator<T, A>& allocator) {
    if constexpr (!std::allocator_traits<A>::is_always_equal::value) {
      A base_allocator;
      ar & base_allocator;
      allocator = umpire::default_init_allocator<T, A>(base_allocator);
    }
  }
};

template <class Archive, typename T, typename A>
struct ArchiveStoreImpl<Archive, umpire::default_init_allocator<T, A>> {
  static inline void store(
      const Archive& ar,
      const umpire::default_init_allocator<T, A>& allocator) {
    if constexpr (!std::allocator_traits<A>::is_always_equal::value) {
      ar& static_cast<const A&>(allocator);
    }
  }
};

}  // namespace archive
}  // namespace madness

namespace madness {
namespace archive {

template <class Archive, class T, class StaticLock,
          typename UmpireAllocatorAccessor>
struct ArchiveLoadImpl<Archive, umpire::allocator<
                                    T, StaticLock, UmpireAllocatorAccessor>> {
  static inline void load(
      const Archive& ar,
      umpire::allocator<T, StaticLock,
                                         UmpireAllocatorAccessor>& allocator) {
    allocator = umpire::allocator<T, StaticLock,
                                                   UmpireAllocatorAccessor>{};
  }
};

template <class Archive, class T, class StaticLock,
          typename UmpireAllocatorAccessor>
struct ArchiveStoreImpl<Archive, umpire::allocator<
                                     T, StaticLock, UmpireAllocatorAccessor>> {
  static inline void store(
      const Archive& ar,
      const umpire::allocator<
          T, StaticLock, UmpireAllocatorAccessor>& allocator) {}
};

}  // namespace archive
}  // namespace madness

#undef HAVE_MADNESS_WORLD_ARCHIVE_H
#endif  // HAVE_MADNESS_WORLD_ARCHIVE_H

#endif  // UMPIRE_CXX_ALLOCATOR_HPP___INCLUDED

