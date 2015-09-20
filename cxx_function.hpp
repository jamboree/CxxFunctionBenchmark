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

#define DISPATCH_CQ( MACRO, UNSAFE, QUALS ) MACRO( QUALS, UNSAFE ) MACRO( const QUALS, IGNORE )
#define DISPATCH_CV( MACRO, UNSAFE, QUALS ) DISPATCH_CQ( MACRO, UNSAFE, QUALS ) DISPATCH_CQ( MACRO, IGNORE, volatile QUALS )

// Apply a given macro over all reference qualifications.
#define DISPATCH_CVREFQ( MACRO, QUALS ) DISPATCH_CV( MACRO, IGNORE, & QUALS ) DISPATCH_CV( MACRO, IGNORE, && QUALS )

// Apply a given macro over all type qualifications.
#define DISPATCH_ALL( MACRO ) DISPATCH_CV( MACRO, UNPACK, ) DISPATCH_CVREFQ( MACRO, )

// Convert a member function signature to its free invocation counterpart.
template< typename sig >
struct implicit_object_to_parameter;

template< typename t >
struct add_reference
    { typedef t & type; };
template< typename t >
struct add_reference< t && >
    { typedef t && type; };

struct erasure_handle {}; // The member function should belong to a class derived from this base.

#define TYPE_CONVERT_CASE( QUALS, UNSAFE ) \
template< typename ret, typename ... arg > \
struct implicit_object_to_parameter< ret( arg ... ) QUALS > \
    { typedef ret ( * type )( add_reference< erasure_handle QUALS >::type, arg ... ); };
DISPATCH_ALL( TYPE_CONVERT_CASE )
#undef TYPE_CONVERT_CASE

/* Implement a vtable using metaprogramming. Why?
    1. Implement without polymorphic template instantiations (would need 2N of them).
    2. Eliminate overhead and ABI issues associated with RTTI and weak linkage.
    3. Allow static data entries as well as functions.
    
    The table is stored as a std::tuple of function pointers and type_info*'s.
    Entries that would be trivial or useless may be set to nullptr.
*/
enum class dispatch_slot {
    destructor,
    move_constructor_destructor,
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
struct erasure_base : erasure_handle {
    typedef std::tuple<
        void (*)( erasure_handle &, void * alloc ), // destructor
        void (*)( erasure_handle &&, void * dest, void * source_alloc, void * dest_alloc ), // move constructor + destructor
        void (*)( erasure_handle const &, void * dest, void * alloc ), // copy constructor
        
        void const * (*)( erasure_handle const & ), // target access
        std::type_info const &, // target_type
        std::type_info const *, // allocator_type
        
        typename implicit_object_to_parameter< sig >::type ... // dispatchers
    > dispatch_table;
    
    dispatch_table const & table;
    
    constexpr erasure_base( dispatch_table const & in_table )
        : table( in_table ) {}
};

// Generic "virtual" functions to manage the wrapper payload lifetime.
template< typename derived >
struct erasure_special {
    static void destroy( erasure_handle & self, void * ) noexcept
        { static_cast< derived & >( self ). ~ derived(); }
    static void move( erasure_handle && self, void * dest, void *, void * ) {
        new (dest) derived( std::move( static_cast< derived & >( self ) ) );
        destroy( self, {} );
    }
    static void copy( erasure_handle const & self, void * dest, void * )
        { new (dest) derived( static_cast< derived const & >( self ) ); }
};

// These accessors generate "vtable" entries, but avoid instantiating functions that do not exist or would be trivial.
// Most are specialized for allocator_erasure.
template< typename >
struct is_allocator_erasure : std::false_type {};

template< typename derived >
constexpr typename std::enable_if<
    ! std::is_trivially_destructible< derived >::value
    || is_allocator_erasure< derived >::value >::type
( * erasure_destroy() ) ( erasure_handle &, void * )
    { return & derived::destroy; }
template< typename derived >
constexpr typename std::enable_if<
    std::is_trivially_destructible< derived >::value
    && ! is_allocator_erasure< derived >::value >::type
( * erasure_destroy() ) ( erasure_handle &, void * )
    { return nullptr; }

template< typename erasure, typename = void >
struct erasure_trivially_movable : std::false_type {};
template< typename erasure >
struct erasure_trivially_movable< erasure, typename std::enable_if<
    std::is_trivially_constructible< erasure, erasure >::value
    && std::is_trivially_destructible< erasure >::value >::type > : std::true_type {};

template< typename derived >
constexpr typename std::enable_if< ! erasure_trivially_movable< derived >::value >::type
( * erasure_move() ) ( erasure_handle &&, void *, void *, void * )
    { return & derived::move; }
template< typename derived >
constexpr typename std::enable_if< erasure_trivially_movable< derived >::value >::type
( * erasure_move() ) ( erasure_handle &&, void *, void *, void * )
    { return nullptr; }

template< typename erasure >
struct erasure_nontrivially_copyable : std::integral_constant< bool,
    std::is_copy_constructible< erasure >::value
    && ! std::is_trivially_copy_constructible< erasure >::value > {};

template< typename derived >
constexpr typename std::enable_if< erasure_nontrivially_copyable< derived >::value >::type
( * erasure_copy() ) ( erasure_handle const &, void *, void * )
    { return & derived::copy; }
template< typename derived >
constexpr typename std::enable_if< ! erasure_nontrivially_copyable< derived >::value >::type
( * erasure_copy() ) ( erasure_handle const &, void *, void * )
    { return nullptr; }

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
    void operator () ( NAME ## _dispatch ) = delete; /* Feed the "using operator ();" or "using call;" declaration in next derived class. */ \
    static void call ( NAME ## _dispatch ) = delete; \
};

#define UNPACK(...) __VA_ARGS__
#define IGNORE(...)

// This macro generates a recursive template handling one type qualifier sequence, e.g. "volatile &" or "const."
// The final product converts a sequence of qualified signatures into an overload set, potentially with special cases for signatures of no qualification.
#define DISPATCH_CASE( \
    QUALS, /* The type qualifiers for this case. */ \
    UNSAFE, /* UNPACK if there are no qualifiers, IGNORE otherwise. Supports deprecated const-qualified access. */ \
    CLASS, /* The base name of the class. "_dispatch" will be appended. */ \
    FN, /* The function overload name. */ \
    ... /* The function definition. */ \
) \
template< typename derived, std::size_t table_index, typename ret, typename ... arg, typename ... sig > \
struct CLASS ## _dispatch< derived, table_index, ret( arg ... ) QUALS, sig ... > \
    : CLASS ## _dispatch< derived, table_index+1, sig ... \
        UNSAFE (, const_unsafe_case< ret( arg ... ) >) > { \
    using CLASS ## _dispatch< derived, table_index+1, sig ... \
        UNSAFE (, const_unsafe_case< ret( arg ... ) >) >::FN; \
    __VA_ARGS__ \
};

#define ERASURE_DISPATCH_CASE( QUALS, CLASS, ... ) DISPATCH_CASE( QUALS, IGNORE, CLASS, call, \
    static ret call( typename add_reference< erasure_handle QUALS >::type self, arg ... a ) { __VA_ARGS__ } )

// "vtable" generator macro.
#define DISPATCH_TABLE( NAME, TARGET_TYPE, TPARAM, TARG ) \
template< UNPACK TPARAM > \
typename erasure_base< sig ... >::dispatch_table const NAME< UNPACK TARG >::table = typename NAME::dispatch_table{ \
    erasure_destroy< NAME >(), \
    erasure_move< NAME >(), \
    erasure_copy< NAME >(), \
    & NAME::target_access, \
    typeid (TARGET_TYPE), \
    erasure_allocator_type< NAME >(), \
    & std::conditional< false, sig, NAME >::type::call ... \
};


// Implement the uninitialized state.
DISPATCH_BASE_CASE( null )
#define NULL_CASE( QUALS, UNSAFE ) ERASURE_DISPATCH_CASE( QUALS, null, \
    (void) sizeof ... (a); (void) & self; throw std::bad_function_call{}; \
)
DISPATCH_ALL( NULL_CASE )
#undef NULL_CASE

template< typename ... sig >
struct null_erasure
    : erasure_base< sig ... > // "vtable" interface class
    , erasure_special< null_erasure< sig ... > > // generic implementations of "virtual" functions
    , null_dispatch< null_erasure< sig ... >, 0, sig ... > { // dispatcher for specified call signatures
    static const typename erasure_base< sig ... >::dispatch_table table; // own "vtable"
    
    // The const qualifier is bogus. Rather than type-erase an identical non-const version, let the wrapper do a const_cast.
    static void const * target_access( erasure_handle const & ) { return nullptr; } // target<void>() still returns nullptr.
    
    null_erasure() noexcept
        : null_erasure::erasure_base( table ) {} // Initialize own "vtable pointer" at runtime.
};

DISPATCH_TABLE( null_erasure, void, ( typename ... sig ), ( sig ... ) )


// Implement erasures of objects which are small and have a well-defined call operator.
DISPATCH_BASE_CASE( local )
#define LOCAL_CASE( QUALS, UNSAFE ) ERASURE_DISPATCH_CASE( QUALS, local, \
    return static_cast< typename add_reference< derived QUALS >::type >( self ) /* Preserve qualifiers but add "&" to avoid copying (*this). */ \
        .target( std::forward< arg >( a ) ... ); /* Directly call the object, not a reference, to avoid possible virtual dispatch. */ \
)
DISPATCH_ALL( LOCAL_CASE )
#undef LOCAL_CASE

template< typename target_type, typename ... sig >
struct local_erasure
    : erasure_base< sig ... >
    , erasure_special< local_erasure< target_type, sig ... > >
    , local_dispatch< local_erasure< target_type, sig ... >, 0, sig ... > {
    static const typename erasure_base< sig ... >::dispatch_table table;
    
    target_type target;
    
    static void const * target_access( erasure_handle const & self )
        { return & static_cast< local_erasure const & >( self ).target; }
    
    template< typename ... arg >
    local_erasure( arg && ... a )
        : local_erasure::erasure_base( table )
        , target( std::forward< arg >( a ) ... ) {}
};

DISPATCH_TABLE( local_erasure, target_type, ( typename target_type, typename ... sig ), ( target_type, sig ... ) )

// Implement erasures of pointer-to-members, which need std::mem_fn instead of a direct call.
DISPATCH_BASE_CASE( ptm )
#define PTM_CASE( QUALS, UNSAFE ) ERASURE_DISPATCH_CASE( QUALS, ptm, \
    return std::mem_fn( static_cast< typename add_reference< derived QUALS >::type >( self ).target )( std::forward< arg >( a ) ... ); \
)
DISPATCH_ALL( PTM_CASE )
#undef PTM_CASE

template< typename target_type, typename ... sig >
struct ptm_erasure
    : erasure_base< sig ... >
    , erasure_special< ptm_erasure< target_type, sig ... > >
    , ptm_dispatch< ptm_erasure< target_type, sig ... >, 0, sig ... > {
    static const typename erasure_base< sig ... >::dispatch_table table;
    
    target_type target; // Do not use mem_fn here...
    
    static void const * target_access( erasure_handle const & self )
        { return & static_cast< ptm_erasure const & >( self ).target; } // ... because the user can get read/write access to the target object.
    
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
#define ALLOCATOR_CASE( QUALS, UNSAFE ) ERASURE_DISPATCH_CASE( QUALS, allocator, \
    return ( * static_cast< typename add_reference< decltype (std::declval< derived >().target) QUALS >::type >( \
            static_cast< typename add_reference< derived QUALS >::type >( self ) \
        .target ) )( std::forward< arg >( a ) ... ); \
)
DISPATCH_ALL( ALLOCATOR_CASE )
#undef ALLOCATOR_CASE

// Use Allocator<char> as a common reference point, for the typeid operator and the instance in function_container.
// (The instance in the erasure object is always a bona fide Allocator<T>, though.)
template< typename allocator >
using common_allocator_rebind = typename std::allocator_traits< allocator >::template rebind_alloc< char >;

template< typename t, typename = void, typename = void >
struct is_always_equal : std::false_type {};
template< typename t >
struct is_always_equal< t, void, typename std::enable_if< std::allocator_traits< t >::is_always_equal::value >::type >
    : std::true_type {};
template< typename t, typename v >
struct is_always_equal< t, v, typename std::enable_if< decltype (std::declval< t >() == std::declval< t >())::value >::type >
    : std::true_type {};
template< typename t >
struct is_always_equal< std::allocator< t > >
    : std::true_type {};

template< typename allocator, typename target_type, typename ... sig >
struct allocator_erasure
    : erasure_base< sig ... >
    , allocator // empty base class optimization (EBCO)
    , allocator_dispatch< allocator_erasure< allocator, target_type, sig ... >, 0, sig ... > {
    typedef std::allocator_traits< allocator > allocator_traits;
    typedef common_allocator_rebind< allocator > common_allocator;
    
    static const typename erasure_base< sig ... >::dispatch_table table;
    
    typename allocator_traits::pointer target;
    
    allocator & alloc() { return static_cast< allocator & >( * this ); }
    allocator const & alloc() const { return static_cast< allocator const & >( * this ); }
    target_type * target_address() { return std::addressof( * target ); }
    static void const * target_access( erasure_handle const & self )
        { return std::addressof( * static_cast< allocator_erasure const & >( self ).target ); }
    
    template< typename ... arg >
    void construct_safely( arg && ... a ) try {
        allocator_traits::construct( alloc(), target_address(), std::forward< arg >( a ) ... );
    } catch (...) {
        allocator_traits::deallocate( alloc(), target, 1 ); // Does not throw according to [allocator.requirements] §17.6.3.5 and DR2384.
        throw;
    } // The wrapper allocator instance cannot be updated following a failed initialization because the erasure allocator is already gone.
    
    template< typename ... arg >
    allocator_erasure( std::allocator_arg_t, allocator const & in_alloc, arg && ... a )
        : allocator_erasure::erasure_base( table )
        , allocator( in_alloc )
        , target( allocator_traits::allocate( alloc(), 1 ) )
        { construct_safely( std::forward< arg >( a ) ... ); }
    
    // Move-construct into a different pool.
    allocator_erasure( std::allocator_arg_t, allocator const & dest_allocator, allocator_erasure && o )
        : allocator_erasure( std::allocator_arg, dest_allocator, std::move( * o.target ) ) {}
    
    // Common case: copy-construct with any allocator, same or different.
    allocator_erasure( std::allocator_arg_t, allocator const & dest_allocator, allocator_erasure const & o )
        : allocator_erasure( std::allocator_arg, dest_allocator, * o.target ) {}
    
    void move( std::true_type, void * dest, void *, void * ) noexcept { // Call ordinary move constructor.
        new (dest) allocator_erasure( std::move( * this ) ); // Move the pointer, not the object. Don't call the allocator at all.
        this-> ~ allocator_erasure();
    }
    void move( std::false_type, void * dest, void * source_allocator_v, void * dest_allocator_v ) {
        auto * dest_allocator_p = static_cast< common_allocator * >( dest_allocator_v ); // The wrapper verified the safety of this using typeid.
        if ( ! dest_allocator_p || * dest_allocator_p == alloc() ) {
            move( std::true_type{}, dest, source_allocator_v, dest_allocator_v ); // same pool
        } else { // different pool
            auto & e = * new (dest) allocator_erasure( std::allocator_arg, static_cast< allocator >( * dest_allocator_p ), // Reallocate.
                std::move_if_noexcept( * this ) ); // Protect user against their own throwing move constructors.
            * dest_allocator_p = e.alloc(); // Update the wrapper allocator instance with the new copy, potentially updated by the new allocation.
            destroy( * this, source_allocator_v );
        }
    }
    // [*_]allocator_v points to the wrapper allocator instance, if any.
    static void move( erasure_handle && self_base, void * dest, void * source_allocator_v, void * dest_allocator_v ) {
        auto & self = static_cast< allocator_erasure & >( self_base );
        // is_always_equal is usually false here, because it correlates with triviality which short-circuits this function.
        std::move( self ).move( is_always_equal< allocator >{}, dest, source_allocator_v, dest_allocator_v );
    }
    static void copy( erasure_handle const & self_base, void * dest, void * dest_allocator_v ) {
        auto * dest_allocator_p = static_cast< common_allocator * >( dest_allocator_v );
        auto & self = static_cast< allocator_erasure const & >( self_base );
        // Structure the control flow differently to avoid instantiating the copy constructor.
        allocator const & dest_allocator = dest_allocator_p?
            static_cast< allocator const & >( * dest_allocator_p ) : self.alloc();
        auto & e = * new (dest) allocator_erasure( std::allocator_arg, dest_allocator, self );
        if ( dest_allocator_p ) * dest_allocator_p = static_cast< common_allocator const & >( e.alloc() ); // Likewise, update the wrapper allocator instance with the new copy.
    }
    static void destroy( erasure_handle & self_base, void * allocator_v ) noexcept {
        auto & self = static_cast< allocator_erasure & >( self_base );
        allocator_traits::destroy( self.alloc(), self.target_address() );
        allocator_traits::deallocate( self.alloc(), self.target, 1 );
        if ( allocator_v ) * static_cast< common_allocator * >( allocator_v ) = static_cast< common_allocator const & >( self.alloc() );
        self. ~ allocator_erasure();
    }
};

template< typename allocator, typename target_type, typename ... sig >
struct is_allocator_erasure< allocator_erasure< allocator, target_type, sig ... > > : std::true_type {};

template< typename allocator, typename target_type, typename ... sig >
struct erasure_trivially_movable< allocator_erasure< allocator, target_type, sig ... >, typename std::enable_if<
    std::is_trivially_constructible< allocator_erasure< allocator, target_type, sig ... >, allocator_erasure< allocator, target_type, sig ... > >::value
    && std::is_trivially_destructible< allocator_erasure< allocator, target_type, sig ... > >::value >::type >
    : is_always_equal< allocator > {};

template< typename allocator, typename target_type, typename ... sig >
struct erasure_nontrivially_copyable< allocator_erasure< allocator, target_type, sig ... > >
    : std::is_copy_constructible< target_type > {};

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
#define WRAPPER_CASE( QUALS, UNSAFE ) DISPATCH_CASE( QUALS, UNSAFE, wrapper, operator (), \
    ret operator () ( arg ... a ) QUALS { \
        auto && self = static_cast< typename add_reference< derived QUALS >::type >( * this ); \
        return std::get< + dispatch_slot::base_index + table_index >( self.erasure().table ) \
            ( std::forward< decltype (self) >( self ).erasure(), std::forward< arg >( a ) ... ); \
    } \
)
DISPATCH_ALL( WRAPPER_CASE )
#undef WRAPPER_CASE

// Additionally implement the legacy casting away of const, but with a warning.
template< typename derived, std::size_t n, typename ret, typename ... arg, typename ... more >
struct wrapper_dispatch< derived, n, const_unsafe_case< ret( arg ... ) >, more ... >
    : wrapper_dispatch< derived, n, more ... > {
    using wrapper_dispatch< derived, n, more ... >::operator ();
    [[deprecated( "It is unsafe to call a std::function of non-const signature through a const access path." )]]
    ret operator () ( arg ... a ) const {
        return const_cast< derived & >( static_cast< derived const & >( * this ) )
            ( std::forward< arg >( a ) ... );
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
    
    // init and destroy enter or recover from invalid states.
    // They get on the right side of [basic.life]/7.4, but mind the exceptions.
    
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
    
    // Implement erasure type verification for always-local targets without touching RTTI.
    bool verify_type_impl( void * ) const noexcept
        { return & erasure().table == & null_erasure< sig ... >::table; }
    
    template< typename t >
    bool verify_type_impl( std::reference_wrapper< t > * ) const noexcept
        { return & erasure().table == & local_erasure< std::reference_wrapper< t >, sig ... >::table; }
    
    template< typename t >
    bool verify_type_impl( t ** ) const noexcept
        { return & erasure().table == & local_erasure< t *, sig ... >::table; }
    
    template< typename t, typename c >
    bool verify_type_impl( t c::** ) const noexcept
        { return & erasure().table == & ptm_erasure< t c::*, sig ... >::table; }
    
    // User-defined class types are never guaranteed to be local. There could exist some allocator for which uses_allocator is true.
    // RTTI could be replaced here by a small variable template linked from the table. Since we need it anyway, just use RTTI.
    template< typename want >
    bool verify_type_impl( want * ) const noexcept
        { return target_type() == typeid (want); }
public:
    #define ERASURE_ACCESS( QUALS, UNSAFE ) \
        erasure_base< sig ... > QUALS erasure() QUALS { return reinterpret_cast< erasure_base< sig ... > QUALS >( storage ); }
    DISPATCH_CVREFQ( ERASURE_ACCESS, )
    #undef ERASURE_ACCESS
    
    std::type_info const & target_type() const noexcept
        { return std::get< + dispatch_slot::target_type >( erasure().table ); }
    
    template< typename want >
    bool verify_type() const noexcept {
        static_assert ( ! std::is_reference< want >::value, "function does not support reference-type targets." );
        static_assert ( ! std::is_const< want >::value && ! std::is_volatile< want >::value, "function does not support cv-qualified targets." );
        return verify_type_impl( (want *) nullptr );
    }
    template< typename want >
    bool verify_type() const volatile noexcept
        { return const_cast< wrapper_base const * >( this )->verify_type< want >(); }
    
    void const * complete_object_address() const noexcept
        { return std::get< + dispatch_slot::target_access >( erasure().table ) ( erasure() ); }
    void const volatile * complete_object_address() const volatile noexcept
        { return const_cast< wrapper_base const * >( this )->complete_object_address(); }
    
    template< typename want >
    want const * target() const noexcept {
        if ( ! verify_type< want >() ) return nullptr;
        return static_cast< want const * >( std::get< + dispatch_slot::target_access >( erasure().table ) ( erasure() ) );
    }
    template< typename want >
    want * target() noexcept
        { return const_cast< want * >( static_cast< wrapper_base const & >( * this ).target< want >() ); }
    
    explicit operator bool () const volatile noexcept
        { return ! verify_type< void >(); }
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
    static constexpr bool noexcept_move_assign = allocator_traits::propagate_on_container_move_assignment::value || is_always_equal< allocator >::value;
    static constexpr bool noexcept_move_adopt = false; // Adoption may produce an allocator_mismatch_error.
    
    template< typename derived, typename t >
    derived reallocate( t && o )
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
    common_allocator_rebind< allocator > * any_allocator()
        { return & actual_allocator(); }
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
    
    template< typename, typename t >
    t && reallocate( t && o )
        { return std::forward< t >( o ); }
    
    template< typename derived, typename t >
    derived reallocate( t const & o )
        { return o; }
    
    // This defines the default allocation of targets generated by non-container functions.
    typedef wrapper_no_allocator allocator_type;
    std::allocator< char > actual_allocator() { return {}; }
    
    template< typename erasure_base >
    static constexpr void * compatible_allocator( erasure_base const & )
        { return nullptr; }
    static constexpr void * any_allocator()
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
    
    // Queries on potential targets.
    template< typename source, typename allocator = typename allocator_manager::allocator_type >
    struct is_small {
        static const bool value = sizeof (local_erasure< source, sig ... >) <= sizeof (storage)
            && alignof (source) <= alignof (decltype (storage))
            && ! std::uses_allocator< source, allocator >::value
            && std::is_nothrow_move_constructible< source >::value;
    };
    
    template< typename, typename = void >
    struct is_compatibly_wrapped : std::false_type
        { static const bool with_compatible_allocation = false; };
    template< typename source >
    struct is_compatibly_wrapped< source, typename std::enable_if<
            std::is_same< typename wrapper::wrapper_base, typename source::wrapper_base >::value >::type >
        : std::true_type {
        static const bool with_compatible_allocation = allocator_manager::noexcept_move_adopt
            || std::is_same< typename wrapper::allocator_type, typename source::allocator_type >::value;
    };
    
    // Adopt by move.
    template< typename source >
    typename std::enable_if< is_compatibly_wrapped< source >::value >::type
    init( in_place_t< source >, source && s ) {
        typename source::wrapper & o = s;
        auto nontrivial = std::get< + dispatch_slot::move_constructor_destructor >( o.erasure().table );
        if ( ! nontrivial ) {
            std::memcpy( storage_address(), & o.storage, sizeof (storage) );
        } else {
            nontrivial( std::move( o ).erasure(), storage_address(), o.any_allocator(), allocator_manager::compatible_allocator( o.erasure() ) );
        }
        o.init( in_place_t< std::nullptr_t >{}, nullptr );
    }
    // Adopt by copy.
    template< typename source >
    typename std::enable_if< is_compatibly_wrapped< source >::value >::type
    init( in_place_t< source >, source const & s ) {
        typename wrapper::wrapper_base const & o = s;
        auto nontrivial = std::get< + dispatch_slot::copy_constructor >( o.erasure().table );
        if ( ! nontrivial ) std::memcpy( storage_address(), & o.storage, sizeof (storage) );
        else nontrivial( o.erasure(), storage_address(), allocator_manager::compatible_allocator( o.erasure() ) );
    }
    
    // In-place construction of a compatible source uses a temporary to check its constraints.
    template< typename source, typename ... arg >
    typename std::enable_if< is_compatibly_wrapped< source >::value && ! std::is_same< source, wrapper >::value >::type
    init( in_place_t< source > t, arg && ... a )
        { init( t, source( std::forward< arg >( a ) ... ) ); }
    
    // Discard an allocator argument that is unused or already permanently retained.
    template< typename allocator, typename source, typename ... arg >
    typename std::enable_if< is_small< source >::value
        || ( is_compatibly_wrapped< source >::value && std::is_same< allocator, typename allocator_manager::allocator_type >::value ) >::type
    init( std::allocator_arg_t, allocator const &, in_place_t< source > t, arg && ... a )
        { init( t, std::forward< arg >( a ) ... ); }
    
    // Otherwise, given an allocator and constructor arguments for obtaining a compatible source, normalize allocation by introducing a temporary container object.
    template< typename allocator, typename source, typename ... arg >
    typename std::enable_if< is_compatibly_wrapped< source >::value && ! std::is_same< allocator, typename allocator_manager::allocator_type >::value >::type
    init( std::allocator_arg_t, allocator const & alloc, in_place_t< source > t, arg && ... a )
        { init( in_place_t< wrapper< is_targetable, wrapper_allocator< allocator >, sig ... > >{}, std::allocator_arg, alloc, t, std::forward< arg >( a ) ... ); }
    
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
        new (storage_address()) erasure( std::allocator_arg, alloc, std::forward< arg >( a ) ... );
    }
    template< typename source, typename ... arg >
    typename std::enable_if< ! is_compatibly_wrapped< source >::value && ! is_small< source >::value >::type
    init( in_place_t< source > t, arg && ... a )
        { init( std::allocator_arg, allocator_manager::actual_allocator(), t, std::forward< arg >( a ) ... ); }
    
    wrapper & finish_assign ( wrapper && next ) noexcept {
        destroy();
        init( in_place_t< wrapper >{}, std::move( next ) );
        this->actual_allocator() = next.actual_allocator();
        return * this;
    }
    
    void destroy() noexcept {
        auto nontrivial = std::get< + dispatch_slot::destructor >( this->erasure().table );
        if ( nontrivial ) nontrivial( this->erasure(), allocator_manager::any_allocator() );
    }
public:
    using wrapper::wrapper_base::operator ();
    using wrapper::wrapper_base::target;
    using wrapper::wrapper_base::target_type;
    using wrapper::wrapper_base::verify_type;
    using wrapper::wrapper_base::operator bool;
    using wrapper::wrapper_base::complete_object_address;
    
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
            || is_compatibly_wrapped< source >::with_compatible_allocation )
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
    noexcept( is_noexcept_erasable< typename std::decay< source >::type >::value )
        : allocator_manager( std::allocator_arg, alloc )
        { init( std::allocator_arg, alloc, in_place_t< typename std::decay< source >::type >{}, std::forward< source >( s ) ); }
    
    template< typename source, typename ... arg,
        typename = typename std::enable_if< is_targetable< source >::value >::type >
    wrapper( in_place_t< source > t, arg && ... a )
    noexcept( is_noexcept_erasable< source >::value )
        { init( t, std::forward< arg >( a ) ... ); }
    
    template< typename allocator, typename source, typename ... arg,
        typename = typename std::enable_if<
            is_targetable< source >::value
        >::type >
    wrapper( std::allocator_arg_t, allocator const & alloc, in_place_t< source > t, arg && ... a )
    noexcept( is_noexcept_erasable< source >::value )
        : allocator_manager( std::allocator_arg, alloc )
        { init( std::allocator_arg, alloc, t, std::forward< arg >( a ) ... ); }
    
    ~ wrapper() noexcept
        { destroy(); }
    
    wrapper & operator = ( wrapper && s )
    noexcept ( allocator_manager::noexcept_move_assign ) {
        if ( & s == this ) return * this;
        allocator_manager::operator = ( std::move( s ) );
        return finish_assign( allocator_manager::template reallocate< wrapper >( std::move( s ) ) );
    }
    wrapper & operator = ( wrapper const & s ) {
        if ( & s == this ) return * this;
        allocator_manager::operator = ( s );
        return finish_assign( allocator_manager::template reallocate< wrapper >( s ) );
    }
    
    template< typename source,
        typename std::enable_if<
            is_compatibly_wrapped< typename std::decay< source >::type >::value
            && std::is_constructible< typename std::decay< source >::type, source >::value
            && ! std::is_base_of< wrapper, typename std::decay< source >::type >::value
        >::type * = nullptr >
    wrapper &
    operator = ( source && s )
    noexcept( is_compatibly_wrapped< source >::with_compatible_allocation && allocator_manager::noexcept_move_assign )
        { return finish_assign( allocator_manager::template reallocate< wrapper >( std::forward< source >( s ) ) ); }
    
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
        typename = typename std::enable_if< is_targetable< typename std::decay< source >::type >::value >::type >
    wrapper &
    assign( source && s, allocator const & alloc )
    noexcept( is_noexcept_erasable< typename std::decay< source >::type >::value || is_compatibly_wrapped< source >::value )
        { return finish_assign( wrapper{ std::allocator_arg, alloc, std::forward< source >( s ) } ); }
    
    template< typename source, typename ... arg,
        typename = typename std::enable_if< is_targetable< source >::value >::type >
    wrapper &
    emplace_assign( arg && ... a )
    noexcept( is_noexcept_erasable< source >::value )
        { return finish_assign( wrapper{ std::allocator_arg, this->actual_allocator(), in_place_t< source >{}, std::forward< arg >( a ) ... } ); }
    
    template< typename source, typename allocator, typename ... arg,
        typename = typename std::enable_if< is_targetable< source >::value >::type >
    wrapper &
    allocate_assign( allocator const & alloc, arg && ... a )
    noexcept( is_noexcept_erasable< source >::value ) {
        return finish_assign( wrapper< is_targetable, wrapper_allocator< allocator >, sig ... >
            { std::allocator_arg, alloc, in_place_t< source >{}, std::forward< arg >( a ) ... } );
    }
    
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
    using function::wrapper::verify_type;
    using function::wrapper::operator bool;
    using function::wrapper::complete_object_address;
    
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
    using unique_function::wrapper::verify_type;
    using unique_function::wrapper::operator bool;
    using unique_function::wrapper::complete_object_address;
    
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
    using function_container::wrapper::verify_type;
    using function_container::wrapper::operator bool;
    using function_container::wrapper::complete_object_address;
    
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
    using unique_function_container::wrapper::verify_type;
    using unique_function_container::wrapper::operator bool;
    using unique_function_container::wrapper::complete_object_address;
    
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
#undef ERASURE_DISPATCH_CASE
#undef DISPATCH_CQ
#undef DISPATCH_CV
#undef DISPATCH_CVREFQ
#undef DISPATCH_ALL
#undef DISPATCH_TABLE

#if __cplusplus >= 201402 // Return type deduction really simplifies these.
// See proposal, "std::recover: undoing type erasure"

template< typename erasure >
void const * recover_address( erasure & e, std::false_type )
    { return e.complete_object_address(); }

template< typename erasure >
void const * recover_address( erasure & e, std::true_type )
    { return e.referent_address(); }

template< typename want, typename erasure >
constexpr auto * recover( erasure * e ) noexcept {
    static_assert ( ! std::is_void< want >::value, "Recovering a void* is meaningless. Perhaps you want static_cast< void const * >( e )." );
    // Add const qualifier to potential reference type T. References don't propagate const.
    typedef std::conditional_t< std::is_const< erasure >::value, want const, want > prop_const;
    typedef std::conditional_t< std::is_volatile< erasure >::value, prop_const volatile, prop_const > prop_cv;
    typedef std::remove_reference_t< prop_cv > cv_object;
    return e->template verify_type< want >()?
        ( cv_object * ) recover_address( e, std::is_reference< want >{} )
        : nullptr;
}

struct bad_type_recovery : std::exception
    { virtual char const * what() const noexcept override { return "An object was not found with its expected type."; } };

template< typename want, typename erasure_ref >
constexpr auto && recover( erasure_ref && e ) {
    typedef std::remove_reference_t< erasure_ref > erasure;
    typedef std::conditional_t< std::is_const< erasure >::value, want const, want > prop_const;
    typedef std::conditional_t< std::is_volatile< erasure >::value, prop_const volatile, prop_const > prop_cv;
    typedef std::conditional_t< std::is_lvalue_reference< erasure_ref >::value, prop_cv &, prop_cv && > prop_cvref;
    if ( e.template verify_type< want >() ) {
        return static_cast< prop_cvref >( * ( std::decay_t< want > * ) e.complete_object_address () );
    } else throw bad_type_recovery{};
}
#endif

}

#endif
