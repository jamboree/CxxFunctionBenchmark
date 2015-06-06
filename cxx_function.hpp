// cxx_function.hpp: major evolution for std::function
// Copyright 2015 by David Krauss.
// This source is released under the MIT license, http://opensource.org/licenses/MIT

#ifndef INCLUDED_CXX_FUNCTION_HPP
#define INCLUDED_CXX_FUNCTION_HPP

#include <cassert>
#include <cstring>
#include <exception>
#include <functional> // for std::bad_function_call and std::mem_fn
#include <memory>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

namespace cxx_function {

// Dispatch tag for in-place construction, for when explicit template arguments are unavailable (e.g. constructor calls).
template< typename >
struct in_place_t {};

#if __cplusplus >= 201402
template< typename t >
constexpr in_place_t< t > in_place = {};
#endif

namespace impl {

/* Implement a vtable using metaprogramming. Why?
    1. Implement without polymorphic template instantiations (would need 2N of them).
    2. Eliminate overhead and ABI issues associated with RTTI and weak linkage.
    3. Allow static data entries as well as functions.
    
    The table is stored as a std::tuple of PTMFs and type_info*'s.
    Entries that would be trivial or useless may be set to nullptr.
*/
enum class dispatch_slot {
    destructor,
    move_constructor,
    copy_constructor,
    target_access,
    target_type,
    allocator_type,
    
    base_index
};
constexpr int operator + ( dispatch_slot e ) { return static_cast< int >( e ); }

// "Abstract" base class for the island inside the wrapper class, e.g. std::function.
// This must appear first in the most-derived class layout.
template< typename ... sig >
struct erasure_base {
    typedef std::tuple<
        void (erasure_base::*)(), // destructor
        void (erasure_base::*)( void * dest, void * alloc ) &&, // move constructor
        void (erasure_base::*)( void * dest, void * alloc ) const &, // copy constructor
        
        void const * (erasure_base::*)() const, // target access
        std::type_info const &, // target_type
        std::type_info const *, // allocator_type
        
        sig erasure_base::* ... // dispatchers
    > dispatch_table;
    
    dispatch_table const & table;
    
    constexpr erasure_base( dispatch_table const & in_table )
        : table( in_table ) {}
};

// Generic "virtual" functions to manage the wrapper payload lifetime.
template< typename derived >
struct erasure_special {
    void destroy()
        { static_cast< derived * >( this )-> ~ derived(); }
    void move( void * dest, void * ) &&
        { new (dest) derived( std::move( * static_cast< derived * >( this ) ) ); }
    void copy( void * dest, void * ) const &
        { new (dest) derived( * static_cast< derived const * >( this ) ); }
};

// These accessors generate "vtable" entries, but avoid instantiating functions that do not exist or would be trivial.
template< typename derived >
constexpr typename std::enable_if<
    ! std::is_trivially_destructible< derived >::value >::type
( derived::* erasure_destroy() ) ()
    { return & derived::destroy; }
template< typename derived >
constexpr typename std::enable_if<
    std::is_trivially_destructible< derived >::value >::type
( derived::* erasure_destroy() ) ()
    { return nullptr; }

template< typename derived >
constexpr typename std::enable_if<
    ! std::is_trivially_constructible< derived, derived >::value >::type
( derived::* erasure_move() ) ( void *, void * ) &&
    { return & derived::move; }
template< typename derived >
constexpr typename std::enable_if<
    std::is_trivially_constructible< derived, derived >::value >::type
( derived::* erasure_move() ) ( void *, void * ) &&
    { return nullptr; }

template< typename derived >
constexpr typename std::enable_if<
    std::is_copy_constructible< derived >::value
    && ! std::is_trivially_copy_constructible< derived >::value >::type
( derived::* erasure_copy() ) ( void *, void * ) const &
    { return & derived::copy; }
template< typename derived >
constexpr typename std::enable_if<
    ! std::is_copy_constructible< derived >::value
    || std::is_trivially_copy_constructible< derived >::value >::type
( derived::* erasure_copy() ) ( void *, void * ) const &
    { return nullptr; }

template< typename, typename = void >
struct is_allocator_erasure : std::false_type {};
template< typename erasure >
struct is_allocator_erasure< erasure, decltype(void(typename std::allocator_traits< typename erasure::common_allocator >::pointer{})) >
    : std::true_type {};

template< typename derived >
constexpr typename std::enable_if< is_allocator_erasure< derived >::value,
std::type_info const * >::type erasure_allocator_type()
    { return & typeid (typename derived::common_allocator); }

template< typename derived >
constexpr typename std::enable_if< ! is_allocator_erasure< derived >::value,
std::type_info const * >::type erasure_allocator_type()
    { return nullptr; }


template< typename >
struct const_unsafe_case; // internal tag for function signatures introduced for backward compatibility of const-qualified access

// Termination of the recursive template generated by the DISPATCH_CASE macro below.
#define DISPATCH_BASE_CASE( NAME ) \
template< typename derived, std::size_t n, typename ... sig > \
struct NAME ## _dispatch { \
    static_assert ( sizeof ... (sig) == 0, "An unsupported function signature was detected." ); \
    void operator () ( NAME ## _dispatch ) = delete; /* Feed the "using operator ();" declaration in next derived class. */ \
};

#define UNPACK(...) __VA_ARGS__
#define IGNORE(...)

// This macro generates a recursive template handling one type qualifier sequence (e.g. "volatile &" or "const."
// The final product converts a sequence of qualified signatures into an overload set, potentially with special cases for signatures of no qualification.
#define DISPATCH_CASE( \
    QUALS, /* The type qualifiers for this case. */ \
    UNSAFE, /* UNPACK if there are no qualifiers, IGNORE otherwise. Supports deprecated const-qualified access. */ \
    NAME, /* The base name of the class. "_dispatch" will be appended. */ \
    IMPL /* The function body calling the particular erasure object. */ \
) \
template< typename derived, std::size_t table_index, typename ret, typename ... args, typename ... sig > \
struct NAME ## _dispatch< derived, table_index, ret( args ... ) QUALS, sig ... > \
    : NAME ## _dispatch< derived, table_index+1, sig ... \
        UNSAFE (, const_unsafe_case< ret( args ... ) >) > { \
    using NAME ## _dispatch< derived, table_index+1, sig ... \
        UNSAFE (, const_unsafe_case< ret( args ... ) >) >::operator (); \
    ret operator () ( args ... a ) QUALS { UNPACK IMPL } \
};

#define DISPATCH_CQ( MACRO, UNSAFE, QUALS ) MACRO( QUALS, UNSAFE ) MACRO( const QUALS, IGNORE )
#define DISPATCH_CV( MACRO, UNSAFE, QUALS ) DISPATCH_CQ( MACRO, UNSAFE, QUALS ) DISPATCH_CQ( MACRO, IGNORE, volatile QUALS )

// Apply a given macro over all reference qualifications.
#define DISPATCH_CVREFQ( MACRO, QUALS ) DISPATCH_CV( MACRO, IGNORE, & QUALS ) DISPATCH_CV( MACRO, IGNORE, && QUALS )

// Apply a given macro over all type qualifications.
#define DISPATCH_ALL( MACRO ) DISPATCH_CV( MACRO, UNPACK, ) DISPATCH_CVREFQ( MACRO, )

// Work around the slicing intrinsic to the formation of pointer-to-members.
// A bogus base member is undefined, [expr.static.cast]/12.
// However, [expr.mptr.oper]/4 mentions dynamic type, which suggests that the overall usage is OK.
template< typename base, typename derived, typename t, typename actual_base >
t base::* ptm_cast( t actual_base::* ptm )
    { return static_cast< t base::* >( static_cast< t derived::* >( ptm ) ); }

// "vtable" generator macro.
#define DISPATCH_TABLE( NAME, TARGET_TYPE, TPARAM, TARG ) \
template< UNPACK TPARAM > \
typename erasure_base< sig ... >::dispatch_table const NAME< UNPACK TARG >::table = { \
    ptm_cast< erasure_base< sig ... >, NAME >( erasure_destroy< NAME >() ), \
    ptm_cast< erasure_base< sig ... >, NAME >( erasure_move< NAME >() ), \
    ptm_cast< erasure_base< sig ... >, NAME >( erasure_copy< NAME >() ), \
    ptm_cast< erasure_base< sig ... >, NAME >( & NAME::target_access ), \
    typeid (TARGET_TYPE), \
    erasure_allocator_type< NAME >(), \
    ptm_cast< erasure_base< sig ... >, NAME >( static_cast< sig NAME::* >( & NAME::operator () ) ) ... \
};


// Implement the uninitialized state.
DISPATCH_BASE_CASE( null )
#define NULL_CASE( QUALS, UNSAFE ) DISPATCH_CASE( QUALS, IGNORE, null, ( throw std::bad_function_call{}; ) )
DISPATCH_ALL( NULL_CASE )
#undef NULL_CASE

template< typename ... sig >
struct null_erasure
    : erasure_base< sig ... > // "vtable" interface class
    , erasure_special< null_erasure< sig ... > > // generic implementations of "virtual" functions
    , null_dispatch< null_erasure< sig ... >, 0, sig ... > { // dispatcher for specified call signatures
    static const typename erasure_base< sig ... >::dispatch_table table; // own "vtable"
    
    // The const qualifier is bogus. Rather than type-erase an identical non-const version, let the wrapper do a const_cast.
    void const * target_access() const { return nullptr; } // target<void>() still returns nullptr.
    
    null_erasure() noexcept
        : null_erasure::erasure_base( table ) {} // Initialize own "vtable pointer" at runtime.
};

DISPATCH_TABLE( null_erasure, void, ( typename ... sig ), ( sig ... ) )


template< typename t >
struct add_reference
    { typedef t & type; };
template< typename t >
struct add_reference< t && >
    { typedef t && type; };

// Implement erasures of objects which are small and have a well-defined call operator.
DISPATCH_BASE_CASE( local )
#define LOCAL_CASE( QUALS, UNSAFE ) DISPATCH_CASE( QUALS, IGNORE, local, ( \
    return static_cast< typename add_reference< derived QUALS >::type >( * this ) /* Preserve qualifiers but add "&" to avoid copying (*this). */ \
        .target( std::forward< args >( a ) ... ); /* Directly call the object, not a reference, to avoid possible virtual dispatch. */ \
) )
DISPATCH_ALL( LOCAL_CASE )
#undef LOCAL_CASE

template< typename target_type, typename ... sig >
struct local_erasure
    : erasure_base< sig ... >
    , erasure_special< local_erasure< target_type, sig ... > >
    , local_dispatch< local_erasure< target_type, sig ... >, 0, sig ... > {
    static const typename erasure_base< sig ... >::dispatch_table table;
    
    target_type target;
    
    void const * target_access() const { return & target; }
    
    template< typename ... arg >
    local_erasure( arg && ... a )
        : local_erasure::erasure_base( table )
        , target( std::forward< arg >( a ) ... ) {}
};

DISPATCH_TABLE( local_erasure, target_type, ( typename target_type, typename ... sig ), ( target_type, sig ... ) )

// Implement erasures of pointer-to-members, which need std::mem_fn instead of a direct call.
DISPATCH_BASE_CASE( ptm )
#define PTM_CASE( QUALS, UNSAFE ) DISPATCH_CASE( QUALS, IGNORE, ptm, ( \
    return std::mem_fn( static_cast< derived const volatile & >( * this ).target )( std::forward< args >( a ) ... ); \
) )
DISPATCH_ALL( PTM_CASE )
#undef PTM_CASE

template< typename target_type, typename ... sig >
struct ptm_erasure
    : erasure_base< sig ... >
    , erasure_special< ptm_erasure< target_type, sig ... > >
    , ptm_dispatch< ptm_erasure< target_type, sig ... >, 0, sig ... > {
    static const typename erasure_base< sig ... >::dispatch_table table;
    
    target_type target; // Do not use mem_fn here...
    
    void const * target_access() const { return & target; } // ... because the user can get read/write access to the target object.
    
    ptm_erasure( target_type a )
        : ptm_erasure::erasure_base( table )
        , target( a ) {}
};

DISPATCH_TABLE( ptm_erasure, target_type, ( typename target_type, typename ... sig ), ( target_type, sig ... ) )


// Implement erasures of objects that cannot be stored inside the wrapper.
/*  This does still store the allocator and pointer in the wrapper. A more general case should be added.
    However, there is a conundrum in rebinding an allocator to an instance of itself.
    Also, it's not clear that a native pointer will always be stable, as opposed to a fancy pointer.
    Fancy pointers exceeding the wrapper storage, with varying underlying referent storage, are another conundrum. */
// See preceding erasure classes for comments on their common architecture.
DISPATCH_BASE_CASE( allocator )
#define ALLOCATOR_CASE( QUALS, UNSAFE ) DISPATCH_CASE( QUALS, IGNORE, allocator, ( \
    return ( * static_cast< typename add_reference< decltype (std::declval< derived >().target) QUALS >::type >( \
            static_cast< typename add_reference< derived QUALS >::type >( * this ) \
        .target ) )( std::forward< args >( a ) ... ); \
) )
DISPATCH_ALL( ALLOCATOR_CASE )
#undef ALLOCATOR_CASE

// Use Allocator<char> as a common reference point, for the typeid operator and the instance in function_container.
// (The instance in the erasure object is always a bona fide Allocator<T>, though.)
template< typename allocator >
using common_allocator_rebind = typename std::allocator_traits< allocator >::template rebind_alloc< char >;

// The generic erasure_special implementations cannot handle transfers to unequivalent allocators (reallocating to a new memory pool).
// See specialization below. However, such transfers never happen given propagate_on_container_move_assignment or is_always_equal.
template< typename derived, typename v = void >
struct allocator_erasure_special
    : erasure_special< derived > {};

template< typename allocator, typename target_type, typename ... sig >
struct allocator_erasure
    : erasure_base< sig ... >
    , allocator // empty base class optimization (EBCO)
    , allocator_erasure_special< allocator_erasure< allocator, target_type, sig ... > >
    , allocator_dispatch< allocator_erasure< allocator, target_type, sig ... >, 0, sig ... > {
    using allocator_erasure::allocator_erasure_special::destroy; // Work around name collision from EBCO.
    typedef std::allocator_traits< allocator > allocator_traits;
    typedef common_allocator_rebind< allocator > common_allocator;
    
    static const typename erasure_base< sig ... >::dispatch_table table;
    
    typename allocator_traits::pointer target;
    
    allocator & alloc() { return static_cast< allocator & >( * this ); }
    allocator const & alloc() const { return static_cast< allocator const & >( * this ); }
    target_type * target_address() { return std::addressof( * target ); }
    void const * target_access() const { return std::addressof( * target ); }
    
    template< typename ... arg >
    explicit allocator_erasure( allocator const & in_alloc, arg && ... a )
        : allocator_erasure::erasure_base( table )
        , allocator( in_alloc )
        , target( allocator_traits::allocate( alloc(), 1 ) )
        { allocator_traits::construct( alloc(), target_address(), std::forward< arg >( a ) ... ); }
    
    ~ allocator_erasure() { // TODO QOI allow exceptions.
        if ( target ) {
            allocator_traits::destroy( alloc(), target_address() );
            allocator_traits::deallocate( alloc(), target, 1 ); // TODO deallocate even if destructor throws.
        }
    }
    
    // Move-construct within the same pool.
    allocator_erasure( allocator_erasure && o ) noexcept
        : allocator_erasure::erasure_base( table )
        , allocator( std::move( o.alloc() ) )
        , target( std::move( o.target ) )
        { o.target = nullptr; } // Prevent double deallocation. Could be trivial but for this.
        /*  Letting the wrapper do memset on the source of a trivial move, to trivialize the almost-trivial cases here, is possibly viable.
            It would violate the moved-from state of local erasures. However, they always get blown away after a move anyway.
            The optimization would need to check that Allocator::pointer is a native pointer. */
    
    // Move-construct into a different pool.
    allocator_erasure( std::allocator_arg_t, allocator const & dest_allocator, allocator_erasure && o )
        : allocator_erasure::erasure_base( table )
        , allocator( dest_allocator )
        , target( allocator_traits::allocate( alloc(), 1 ) )
        { allocator_traits::construct( alloc(), target_address(), std::move( * o.target ) ); }
    
    allocator_erasure( allocator_erasure const & o ) // This is only used if allocator_erasure_special maps to the generic erasure_special.
        : allocator_erasure( std::allocator_arg, o.alloc(), o ) {}
    
    // Common case: copy-construct with any allocator, same or different.
    allocator_erasure( std::allocator_arg_t, allocator const & dest_allocator, allocator_erasure const & o )
        : allocator_erasure::erasure_base( table )
        , allocator( dest_allocator )
        , target( allocator_traits::allocate( alloc(), 1 ) )
        { allocator_traits::construct( alloc(), target_address(), * o.target ); }
};

template< typename allocator, typename target_type, typename ... sig >
struct allocator_erasure_special< allocator_erasure< allocator, target_type, sig ... >,
    typename std::enable_if< ! std::allocator_traits< allocator >::propagate_on_container_move_assignment::value
        IGNORE( && ! std::allocator_traits< allocator >::is_always_equal::value ) >::type > // Workaround libc++ bug, recently fixed. TODO pull the fix.
    : erasure_special< allocator_erasure< allocator, target_type, sig ... > > {
    typedef allocator_erasure< allocator, target_type, sig ... > derived;
    typedef common_allocator_rebind< allocator > common_allocator;
    allocator const & alloc() const { return static_cast< derived const & >( * this ); }
    
    void move( void * dest, void * dest_allocator_v ) && { // dest_allocator_v is nullptr for [unique_]function, non-null for [unique_]function_container.
        auto dest_allocator = static_cast< common_allocator * >( dest_allocator_v ); // The wrapper verified the safety of this using typeid.
        if ( ! dest_allocator || * dest_allocator == alloc() ) {
            new (dest) derived( static_cast< derived && >( * this ) ); // Same pool. Move the pointer, not the object.
        } else {
            auto & e = * new (dest) derived( std::allocator_arg, * dest_allocator, static_cast< derived && >( * this ) ); // Different pool. Reallocate.
            * dest_allocator = e.alloc(); // Update the wrapper Allocator instance with the new copy, potentially updated by the new allocation. Is this useful?
        }
    }
    void copy( void * dest, void * dest_allocator_v ) const & { // dest_allocator_v is nullptr for [unique_]function, non-null for [unique_]function_container.
        auto dest_allocator_p = static_cast< common_allocator * >( dest_allocator_v );
        // Structure the control flow differently to avoid instantiating the copy constructor.
        allocator const & dest_allocator = dest_allocator_p?
            static_cast< allocator const & >( * dest_allocator_p ) : alloc();
        auto & e = * new (dest) derived( std::allocator_arg, dest_allocator, static_cast< derived const & >( * this ) );
        if ( dest_allocator_p ) * dest_allocator_p = e.alloc(); // Likewise, update the wrapper Allocator instance with the new copy. Is this useful?
    }
};

DISPATCH_TABLE( allocator_erasure, target_type, ( typename allocator, typename target_type, typename ... sig ), ( allocator, target_type, sig ... ) )


// Metaprogramming for checking a potential target against a list of signatures.
template< bool ... cond >
struct logical_intersection
    : std::true_type {};
template< bool ... cond >
struct logical_intersection< true, cond ... >
    : logical_intersection< cond ... >::type {};
template< bool ... cond >
struct logical_intersection< false, cond ... >
    : std::false_type {};

template< typename t, typename sig, typename = void >
struct is_callable : std::false_type {};

#define IS_CALLABLE_CASE( QUALS, UNSAFE ) \
template< typename t, typename ret, typename ... arg > \
struct is_callable< t, ret( arg ... ) QUALS, \
    typename std::enable_if< std::is_convertible< \
        typename std::result_of< typename add_reference< t QUALS >::type ( arg ... ) >::type \
    , ret >::value >::type > \
    : std::true_type {};

DISPATCH_ALL( IS_CALLABLE_CASE )
#undef IS_CALLABLE_CASE

template< typename sig >
struct is_callable< std::nullptr_t, sig >
    : std::true_type {};

template< typename ... sig >
struct is_all_callable {
    template< typename t >
    using temp = typename logical_intersection< is_callable< t, sig >::value ... >::type;
};

template< typename self, typename ... sig >
struct is_copyable_all_callable {
    template< typename t, typename = void >
    struct temp : std::integral_constant< bool,
        std::is_copy_constructible< t >::value
        && is_all_callable< sig ... >::template temp< t >::value > {};
    
    template< typename v > // Presume that self is a copyable wrapper, since that is what uses this metafunction.
    struct temp< self, v > : std::true_type {};
};

// Map privileged types to noexcept specifications.
template< typename source >
struct is_noexcept_erasable : std::false_type {};
template<>
struct is_noexcept_erasable< std::nullptr_t > : std::true_type {};
template< typename t >
struct is_noexcept_erasable< t * > : std::true_type {};
template< typename t, typename c >
struct is_noexcept_erasable< t c::* > : std::true_type {};
template< typename t >
struct is_noexcept_erasable< std::reference_wrapper< t > > : std::true_type {};

// Generate the wrapper dispatcher in the same way as the erasure dispatchers.
DISPATCH_BASE_CASE( wrapper )
#define WRAPPER_CASE( QUALS, UNSAFE ) DISPATCH_CASE( QUALS, UNSAFE, wrapper, ( \
    auto && self = static_cast< typename add_reference< derived QUALS >::type >( * this ); \
    return ( std::forward< decltype (self) >( self ).erasure() \
            .* std::get< + dispatch_slot::base_index + table_index >( self.erasure().table ) ) \
        ( std::forward< args >( a ) ... ); \
) )
DISPATCH_ALL( WRAPPER_CASE )
#undef WRAPPER_CASE

// Additionally implement the legacy casting away of const, but with a warning.
template< typename derived, std::size_t n, typename ret, typename ... args, typename ... more >
struct wrapper_dispatch< derived, n, const_unsafe_case< ret( args ... ) >, more ... >
    : wrapper_dispatch< derived, n, more ... > {
    using wrapper_dispatch< derived, n, more ... >::operator ();
    [[deprecated( "It is unsafe to call a std::function of non-const signature through a const access path." )]]
    ret operator () ( args ... a ) const {
        return const_cast< derived & >( static_cast< derived const & >( * this ) )
            ( std::forward< args >( a ) ... );
    }
};

template< typename ... sig >
class wrapper_base
    : public wrapper_dispatch< wrapper_base< sig ... >, 0, sig ... > {
    template< template< typename ... > class, typename, typename ... >
    friend class wrapper;
    typedef std::aligned_storage< sizeof (void *[3]) >::type effective_storage_type;
protected:
    std::aligned_storage< sizeof (void *[4]), alignof(effective_storage_type) >::type storage;
    void * storage_address() { return & storage; }
    
    // These functions enter or recover from invalid states.
    // They get on the right side of [basic.life]/7.4, but mind the exceptions.
    
    void destroy() noexcept {
        auto nontrivial = std::get< + dispatch_slot::destructor >( erasure().table );
        if ( nontrivial ) ( erasure() .* nontrivial )();
    }
    
    // Default, move, and copy construction.
    void init( in_place_t< std::nullptr_t >, std::nullptr_t ) noexcept
        { new (storage_address()) null_erasure< sig ... >; }
    
    // Pointers are local callables.
    template< typename t >
    void init( in_place_t< t * >, t * p ) noexcept {
        if ( p ) new (storage_address()) local_erasure< t *, sig ... >( p );
        else init( in_place_t< std::nullptr_t >{}, nullptr );
    }
    // PTMs are like local callables.
    template< typename t, typename c >
    void init( in_place_t< t c::* >, t c::* ptm ) noexcept {
        if ( ptm ) new (storage_address()) ptm_erasure< t c::*, sig ... >( ptm );
        else init( in_place_t< std::nullptr_t >{}, nullptr );
    }
public:
    #define ERASURE_ACCESS( QUALS, UNSAFE ) \
        erasure_base< sig ... > QUALS erasure() QUALS { return reinterpret_cast< erasure_base< sig ... > QUALS >( storage ); }
    DISPATCH_CVREFQ( ERASURE_ACCESS, )
    #undef ERASURE_ACCESS
    
    std::type_info const & target_type() const noexcept
        { return std::get< + dispatch_slot::target_type >( erasure().table ); }
    
    template< typename want >
    want const * target() const noexcept {
        if ( typeid (want) != target_type() ) return nullptr;
        return static_cast< want const * >( ( erasure() .* std::get< + dispatch_slot::target_access >( erasure().table ) )() );
    }
    template< typename want >
    want * target() noexcept
        { return const_cast< want * >( static_cast< wrapper_base const & >( * this ).target< want >() ); }
    
    explicit operator bool () const noexcept
        { return target_type() != typeid (void); }
};

struct allocator_mismatch_error : std::exception // This should be implemented in a .cpp file, but stay header-only for now.
    { virtual char const * what() const noexcept override { return "An object could not be transferred into an incompatible memory allocation scheme."; } };

// Mix-in a persistent allocator to produce a [unique_]function_container.
template< typename allocator >
class wrapper_allocator
    : common_allocator_rebind< allocator > {
    typedef std::allocator_traits< common_allocator_rebind< allocator > > allocator_traits;
    
    void do_swap( std::true_type, wrapper_allocator & o ) noexcept
        { using std::swap; swap( actual_allocator(), o.actual_allocator() ); }
    void do_swap( std::false_type, wrapper_allocator & o ) noexcept
        { assert ( actual_allocator() == o.actual_allocator() && "Cannot swap containers while not-swapping their unequal allocators." ); } // Roughly match libc++.
protected:
    wrapper_allocator( wrapper_allocator && ) noexcept = default;
    
    /*  Do select_on_container_copy_construction here, although the "container" is really represented
        by the set of all equivalent allocators in various independent target objects. */
    wrapper_allocator( wrapper_allocator const & o )
        : common_allocator_rebind< allocator >( allocator_traits::select_on_container_copy_construction( o ) )
        {}
    
    // Likewise the rationale for POCMA, POCCA, and POCS. They propagate freely between targets, not between containers.
    wrapper_allocator & operator = ( wrapper_allocator && o ) noexcept {
        if ( typename allocator_traits::propagate_on_container_move_assignment() ) actual_allocator() = std::move( o.actual_allocator() );
        return * this;
    }
    wrapper_allocator & operator = ( wrapper_allocator const & o ) noexcept {
        if ( typename allocator_traits::propagate_on_container_copy_assignment() ) actual_allocator() = o;
        return * this;
    }
    void swap( wrapper_allocator & o ) noexcept
        { do_swap( allocator_traits::propagate_on_container_swap(), o ); }
    static constexpr bool noexcept_move_assign = allocator_traits::propagate_on_container_move_assignment::value IGNORE ( || allocator_traits::is_always_equal::value );
    static constexpr bool noexcept_move_adopt = false; // Adoption may produce an allocator_mismatch_error.
    
    template< typename t >
    typename std::decay< t >::type reallocate( t && o )
        { return { std::allocator_arg, actual_allocator(), std::forward< t >( o ) }; }
    
    common_allocator_rebind< allocator > & actual_allocator()
        { return * this; }
    
    // Determine whether to use the stored allocator for an initialization or assignment.
    template< typename erasure_base > // Templating this is a bit bogus, since it only uses the "vtable header," but the type system doesn't know that.
    common_allocator_rebind< allocator > * compatible_allocator( erasure_base const & e ) {
        std::type_info const * type = std::get< + dispatch_slot::allocator_type >( e.table ); // Get dynamic type of the source allocator.
        if ( type == nullptr ) return nullptr; // It's a local erasure. Allocators don't apply. Avoid the potentially expensive type_info::operator==.
        if ( * type != typeid (actual_allocator()) ) throw allocator_mismatch_error{}; // Oh no!
        return & actual_allocator();
    }
public: // Include the container-like interface.
    wrapper_allocator() = default;
    wrapper_allocator( std::allocator_arg_t, allocator const & in_alloc ) noexcept
        : common_allocator_rebind< allocator >( in_alloc ) {}
    
    typedef allocator allocator_type;
    allocator get_allocator() const
        { return * this; }
};

// Stub-out the container support.
class wrapper_no_allocator {
protected:
    static constexpr bool noexcept_move_assign = true;
    static constexpr bool noexcept_move_adopt = true;
    
    template< typename t >
    t && reallocate( t && o )
        { return std::forward< t >( o ); }
    
    // This defines the default allocation of targets generated by non-container functions. Should probably be get_default_resource instead.
    typedef std::allocator< char > allocator_type;
    std::allocator< char > actual_allocator() { return {}; }
    
    template< typename erasure_base >
    static constexpr void * compatible_allocator( erasure_base const & e )
        { return nullptr; }
    
    wrapper_no_allocator() = default;
     // This is used internally but not publicly accessible. (It's publicly *visible* because inheriting constructors don't work very well.)
    template< typename ignored >
    wrapper_no_allocator( std::allocator_arg_t, ignored const & ) noexcept {}
    
    void swap( wrapper_no_allocator & ) {}
};

template< template< typename ... > class is_targetable, typename allocator_manager, typename ... sig >
class wrapper
    : public allocator_manager
    , wrapper_base< sig ... > {
    template< template< typename ... > class, typename, typename ... >
    friend class wrapper;
    
    using wrapper::wrapper_base::storage;
    using wrapper::wrapper_base::storage_address;
    using wrapper::wrapper_base::init;
    using wrapper::wrapper_base::destroy;
    
    // Queries on potential targets.
    template< typename source >
    struct is_small {
        static const bool value = sizeof (local_erasure< source, sig ... >) <= sizeof (storage)
            && alignof (source) <= alignof (decltype (storage))
            && ! std::uses_allocator< source, typename allocator_manager::allocator_type >::value
            && std::is_nothrow_move_constructible< source >::value;
    };
    
    template< typename, typename = void >
    struct is_compatibly_wrapped : std::false_type {};
    template< typename source >
    struct is_compatibly_wrapped< source, typename std::enable_if<
            std::is_same< typename wrapper::wrapper_base, typename source::wrapper_base >::value >::type >
        : std::true_type {};
    
    template< typename source >
    typename std::enable_if< is_compatibly_wrapped< source >::value >::type
    init( in_place_t< source >, source && s ) {
        typename source::wrapper & o = s;
        auto nontrivial = std::get< + dispatch_slot::move_constructor >( o.erasure().table );
        if ( ! nontrivial ) std::memcpy( storage_address(), & o.storage, sizeof (storage) );
        else ( std::move( o ).erasure() .* nontrivial )( storage_address(), allocator_manager::compatible_allocator( o.erasure() ) );
        o.destroy();
        o.init( in_place_t< std::nullptr_t >{}, nullptr );
    }
    template< typename source >
    typename std::enable_if< is_compatibly_wrapped< source >::value >::type
    init( in_place_t< source >, source const & s ) {
        static_assert ( std::is_copy_constructible< source >::value, "Allocator construction request bypassed unique_function copyability." );
        typename wrapper::wrapper_base const & o = s;
        auto nontrivial = std::get< + dispatch_slot::copy_constructor >( o.erasure().table );
        if ( ! nontrivial ) std::memcpy( storage_address(), & o.storage, sizeof (storage) );
        else ( o.erasure() .* nontrivial )( storage_address(), allocator_manager::compatible_allocator( o.erasure() ) );
    }
    
    template< typename allocator, typename source, typename ... arg >
    typename std::enable_if< is_compatibly_wrapped< source >::value || is_small< source >::value >::type
    init( std::allocator_arg_t, allocator const &, in_place_t< source > t, arg && ... a )
        { init( t, std::forward< arg >( a ) ... ); }
    
    // Local erasures.
    template< typename source, typename ... arg >
    typename std::enable_if< is_small< source >::value >::type
    init( in_place_t< source >, arg && ... a )
        { new (storage_address()) local_erasure< source, sig ... >( std::forward< arg >( a ) ... ); }
    
    // Allocated erasures.
    template< typename in_allocator, typename source, typename ... arg >
    typename std::enable_if< ! is_compatibly_wrapped< source >::value && ! is_small< source >::value >::type
    init( std::allocator_arg_t, in_allocator && alloc, in_place_t< source >, arg && ... a ) {
        typedef typename std::allocator_traits< typename std::decay< in_allocator >::type >::template rebind_alloc< source > allocator;
        typedef allocator_erasure< allocator, source, sig ... > erasure;
        static_assert ( is_allocator_erasure< erasure >::value, "" );
        // TODO: Add a new erasure template to put the fancy pointer on the heap.
        static_assert ( sizeof (erasure) <= sizeof storage, "Stateful allocator or fancy pointer is too big for polymorphic function wrapper." );
        auto & e = * new (storage_address()) erasure( alloc, std::forward< arg >( a ) ... );
        allocator_manager::actual_allocator() = e.alloc();
    }
    template< typename source, typename ... arg >
    typename std::enable_if< ! is_compatibly_wrapped< source >::value && ! is_small< source >::value >::type
    init( in_place_t< source > t, arg && ... a )
        { init( std::allocator_arg, allocator_manager::actual_allocator(), t, std::forward< arg >( a ) ... ); }
    
    template< typename compatible >
    wrapper & finish_assign ( compatible && next ) noexcept {
        static_assert ( is_compatibly_wrapped< typename std::decay< compatible >::type >::value, "Assignment exception safety violation." );
        destroy();
        init( in_place_t< compatible >{}, std::move( next ) );
        //this->actual_allocator() = next.actual_allocator(); -- would be nice to update local allocator state, but can't really keep it consistent.
        return * this;
    }
public:
    using wrapper::wrapper_base::operator ();
    using wrapper::wrapper_base::target;
    using wrapper::wrapper_base::target_type;
    using wrapper::wrapper_base::operator bool;
    
    wrapper() noexcept
        { init( in_place_t< std::nullptr_t >{}, nullptr ); }
    wrapper( wrapper && s ) noexcept
        : allocator_manager( std::move( s ) )
        { init( in_place_t< wrapper >{}, std::move( s ) ); }
    wrapper( wrapper const & s )
        : allocator_manager( s )
        { init( in_place_t< wrapper >{}, s ); }
    
    template< typename allocator >
    wrapper( std::allocator_arg_t, allocator const & alloc )
        : allocator_manager( std::allocator_arg, alloc )
        { init( in_place_t< std::nullptr_t >{}, nullptr ); }
    
    template< typename source,
        typename std::enable_if<
            is_targetable< typename std::decay< source >::type >::value
            && std::is_constructible< typename std::decay< source >::type, source >::value
            && ! std::is_base_of< wrapper, typename std::decay< source >::type >::value
        >::type * = nullptr >
    wrapper( source && s )
    noexcept( is_noexcept_erasable< typename std::decay< source >::type >::value
            || ( allocator_manager::noexcept_move_adopt && is_compatibly_wrapped< source >::value ) )
        { init( in_place_t< typename std::decay< source >::type >{}, std::forward< source >( s ) ); }

    // Prevent slicing fallback to copy/move constructor.
    template< typename source,
        typename std::enable_if<
            ! is_targetable< typename std::decay< source >::type >::value
            || ! std::is_constructible< typename std::decay< source >::type, source >::value
        >::type * = nullptr >
    wrapper( source && s ) = delete;
    
    template< typename allocator, typename source,
        typename = typename std::enable_if<
            is_targetable< typename std::decay< source >::type >::value
        >::type >
    wrapper( std::allocator_arg_t, allocator const & alloc, source && s )
    noexcept( is_noexcept_erasable< typename std::decay< source >::type >::value || is_compatibly_wrapped< source >::value )
        : allocator_manager( std::allocator_arg, alloc )
        { init( std::allocator_arg, alloc, in_place_t< typename std::decay< source >::type >{}, std::forward< source >( s ) ); }
    
    template< typename source, typename ... arg,
        typename = typename std::enable_if<
            is_targetable< source >::value
            && std::is_constructible< source, arg ... >::value
        >::type >
    wrapper( in_place_t< source > t, arg && ... a )
    noexcept( is_noexcept_erasable< source >::value
        || ( is_compatibly_wrapped< source >::value && std::is_nothrow_constructible< source, arg ... >::value ) )
        { init( t, std::forward< arg >( a ) ... ); }
    
    template< typename allocator, typename source, typename ... arg,
        typename = typename std::enable_if<
            is_targetable< source >::value
        >::type >
    wrapper( std::allocator_arg_t, allocator const & alloc, in_place_t< source > t, arg && ... a )
    noexcept( is_noexcept_erasable< source >::value
        || ( is_compatibly_wrapped< source >::value && std::is_nothrow_constructible< source, arg ... >::value ) )
        : allocator_manager( std::allocator_arg, alloc )
        { init( std::allocator_arg, alloc, t, std::forward< arg >( a ) ... ); }
    
    ~ wrapper() noexcept
        { destroy(); }
    
    wrapper & operator = ( wrapper && s )
    noexcept ( allocator_manager::noexcept_move_assign ) {
        allocator_manager::operator = ( std::move( s ) );
        return finish_assign< wrapper >( allocator_manager::reallocate( std::move( s ) ) );
    }
    wrapper & operator = ( wrapper const & s ) {
        allocator_manager::operator = ( s );
        return finish_assign< wrapper >( allocator_manager::reallocate( s ) );
    }
    
    template< typename source,
        typename std::enable_if<
            is_compatibly_wrapped< typename std::decay< source >::type >::value
            && std::is_constructible< typename std::decay< source >::type, source >::value
            && ! std::is_base_of< wrapper, typename std::decay< source >::type >::value
        >::type * = nullptr >
    wrapper &
    operator = ( source && s )
    noexcept( allocator_manager::noexcept_move_adopt ) {
        return finish_assign< typename std::decay< source >::type >
            ( allocator_manager::reallocate( std::forward< source >( s ) ) );
    }
    
    template< typename source,
        typename std::enable_if<
            is_targetable< typename std::decay< source >::type >::value
            && std::is_constructible< typename std::decay< source >::type, source >::value
            && ! is_compatibly_wrapped< typename std::decay< source >::type >::value
        >::type * = nullptr >
    wrapper &
    operator = ( source && s )
    noexcept( is_noexcept_erasable< typename std::decay< source >::type >::value )
        { return finish_assign( wrapper{ std::allocator_arg, this->actual_allocator(), std::forward< source >( s ) } ); }
    
    template< typename allocator, typename source,
        typename = typename std::enable_if<
            is_targetable< typename std::decay< source >::type >::value
        >::type >
    wrapper &
    assign( source && s, allocator const & alloc )
    noexcept( is_noexcept_erasable< typename std::decay< source >::type >::value || is_compatibly_wrapped< source >::value )
        { return finish_assign( wrapper{ std::allocator_arg, alloc, std::forward< source >( s ) } ); }
    
    template< typename source, typename ... arg,
        typename = typename std::enable_if<
            is_targetable< source >::value
            && std::is_constructible< source, arg ... >::value
        >::type >
    wrapper &
    emplace_assign( arg && ... a )
    noexcept( is_noexcept_erasable< source >::value
            // The next three-part condition simply allows is_noexcept_erasable to propagate from an emplacement that gets elided.
            // noexcept_move_adopt doesn't come into play because if the in-place construction uses an allocator, it's already throwing.
        || ( is_compatibly_wrapped< source >::value && std::is_nothrow_constructible< source, arg ... >::value ) )
        { return finish_assign( wrapper{ std::allocator_arg, this->actual_allocator(), in_place_t< source >{}, std::forward< arg >( a ) ... } ); }
    
    template< typename source, typename allocator, typename ... arg,
        typename = typename std::enable_if<
            is_targetable< source >::value
        >::type >
    wrapper &
    allocate_assign( allocator const & alloc, arg && ... a )
    noexcept( is_noexcept_erasable< source >::value
        || ( is_compatibly_wrapped< source >::value && std::is_nothrow_constructible< source, arg ... >::value ) )
        { return finish_assign( wrapper{ std::allocator_arg, alloc, in_place_t< source >{}, std::forward< arg >( a ) ... } ); }
    
    void swap( wrapper & o ) noexcept {
        this->allocator_manager::swap( o );
        /*  Use the two-assignment algorithm. Each target is safely restored to its original allocator.
            Calling std::swap would be sufficient, except that POCMA might not be set. */
        wrapper temp( std::move( * this ) );
        destroy();
        init( in_place_t< wrapper >{}, std::move( o ) );
        o.destroy();
        o.init( in_place_t< wrapper >{}, std::move( temp ) );
    }
};

}

// The actual classes all just pull their interfaces out of private inheritance.
template< typename ... sig >
class function
    : impl::wrapper< impl::is_copyable_all_callable< function< sig ... >, sig ... >::template temp, impl::wrapper_no_allocator, sig ... > {
    template< template< typename ... > class, typename, typename ... >
    friend class impl::wrapper;
public:
    using function::wrapper::wrapper;
    
    function() noexcept = default; // Investigate why these are needed. Compiler bug?
    function( function && s ) noexcept = default;
    function( function const & ) = default;
    function & operator = ( function && s ) noexcept = default;
    function & operator = ( function const & o ) = default;
    
    using function::wrapper::operator ();
    using function::wrapper::operator =;
    using function::wrapper::swap;
    using function::wrapper::target;
    using function::wrapper::target_type;
    using function::wrapper::operator bool;
    
    using function::wrapper::assign;
    using function::wrapper::emplace_assign;
    using function::wrapper::allocate_assign;
    // No allocator_type or get_allocator.
};

template< typename ... sig >
class unique_function
    : impl::wrapper< impl::is_all_callable< sig ... >::template temp, impl::wrapper_no_allocator, sig ... > {
    template< template< typename ... > class, typename, typename ... >
    friend class impl::wrapper;
public:
    using unique_function::wrapper::wrapper;
    
    unique_function() noexcept = default;
    unique_function( unique_function && s ) noexcept = default;
    unique_function( unique_function const & ) = delete;
    unique_function & operator = ( unique_function && s ) noexcept = default;
    unique_function & operator = ( unique_function const & o ) = delete;
    
    using unique_function::wrapper::operator ();
    using unique_function::wrapper::operator =;
    using unique_function::wrapper::swap;
    using unique_function::wrapper::target;
    using unique_function::wrapper::target_type;
    using unique_function::wrapper::operator bool;
    
    using unique_function::wrapper::assign;
    using unique_function::wrapper::emplace_assign;
    using unique_function::wrapper::allocate_assign;
    // No allocator_type or get_allocator.
};

template< typename allocator, typename ... sig >
class function_container
    : impl::wrapper< impl::is_copyable_all_callable< function< sig ... >, sig ... >::template temp, impl::wrapper_allocator< allocator >, sig ... > {
    template< template< typename ... > class, typename, typename ... >
    friend class impl::wrapper;
public:
    using function_container::wrapper::wrapper;
    
    function_container() noexcept = default; // Investigate why these are needed. Compiler bug?
    function_container( function_container && s ) noexcept = default;
    function_container( function_container const & ) = default;
    function_container & operator = ( function_container && s ) = default;
    function_container & operator = ( function_container const & o ) = default;
    
    using function_container::wrapper::operator ();
    using function_container::wrapper::operator =;
    using function_container::wrapper::swap;
    using function_container::wrapper::target;
    using function_container::wrapper::target_type;
    using function_container::wrapper::operator bool;
    
    using typename function_container::wrapper::allocator_type;
    using function_container::wrapper::get_allocator;
    using function_container::wrapper::emplace_assign;
    // No assign or allocate_assign.
};

template< typename allocator, typename ... sig >
class unique_function_container
    : impl::wrapper< impl::is_all_callable< sig ... >::template temp, impl::wrapper_allocator< allocator >, sig ... > {
    template< template< typename ... > class, typename, typename ... >
    friend class impl::wrapper;
public:
    using unique_function_container::wrapper::wrapper;
    
    unique_function_container() noexcept = default;
    unique_function_container( unique_function_container && s ) noexcept = default;
    unique_function_container( unique_function_container const & ) = delete;
    unique_function_container & operator = ( unique_function_container && s ) = default;
    unique_function_container & operator = ( unique_function_container const & o ) = delete;
    
    using unique_function_container::wrapper::operator ();
    using unique_function_container::wrapper::operator =;
    using unique_function_container::wrapper::swap;
    using unique_function_container::wrapper::target;
    using unique_function_container::wrapper::target_type;
    using unique_function_container::wrapper::operator bool;
    
    using typename unique_function_container::wrapper::allocator_type;
    using unique_function_container::wrapper::get_allocator;
    using unique_function_container::wrapper::emplace_assign;
    // No assign or allocate_assign.
};

#define DEFINE_WRAPPER_OPS( NAME ) \
template< typename ... sig > \
bool operator == ( NAME< sig ... > const & a, std::nullptr_t ) \
    { return !a; } \
template< typename ... sig > \
bool operator != ( NAME< sig ... > const & a, std::nullptr_t ) \
    { return a; } \
template< typename ... sig > \
bool operator == ( std::nullptr_t, NAME< sig ... > const & a ) \
    { return !a; } \
template< typename ... sig > \
bool operator != ( std::nullptr_t, NAME< sig ... > const & a ) \
    { return a; } \
template< typename ... sig > \
void swap( NAME< sig ... > & lhs, NAME< sig ... > & rhs ) \
    noexcept(noexcept( lhs.swap( rhs ) )) \
    { lhs.swap( rhs ); }

DEFINE_WRAPPER_OPS( function )
DEFINE_WRAPPER_OPS( unique_function )
DEFINE_WRAPPER_OPS( function_container )
DEFINE_WRAPPER_OPS( unique_function_container )

#undef DEFINE_WRAPPER_OPS
#undef DISPATCH_BASE_CASE
#undef UNPACK
#undef IGNORE
#undef DISPATCH_CASE
#undef DISPATCH_CQ
#undef DISPATCH_CV
#undef DISPATCH_CVREFQ
#undef DISPATCH_ALL
#undef DISPATCH_TABLE

}

#endif
