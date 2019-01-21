#include <iostream>
#include <random>
#include <string>
#include <chrono>
#include <memory>
#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/builtin.hpp>
#include <boost/type_erasure/callable.hpp>
#include <boost/function.hpp>
#include "function.h"
#include "delegate.hpp"
#include "stdex.hpp"
#include "function2.hpp"
#include "fixed_size_function.hpp"
#include "embxx/StaticFunction.h"
#include "Function-rigtorp.h"
#include "cxx_function.hpp"
#include "inplace_function.h"

#define DELEGATE_ARGS_SIZE 40
#define DELEGATE_ARGS_ALIGN 8
#include "delegate.h"
typedef Delegate::FuncTrivial<int, int> delegate;

// Some optional stuff...
#ifdef ADD_SSVU
#define OPT_SSVU
#include "FastFunc.hpp"
#else
#define OPT_SSVU(...)
#endif
#ifdef ADD_CLUGSTON
#define OPT_CLUGSTON
#include "FastDelegate.h"
typedef fastdelegate::FastDelegate1<int, int> FastDelegate1;
#else
#define OPT_CLUGSTON(...)
#endif
#ifdef ADD_GNR
#define OPT_GNR
#include "forwarder.hpp"
typedef gnr::forwarder<int(int), 48> gnr_forwarder;
#else
#define OPT_GNR(...)
#endif
#ifdef ADD_FOLLY
#define OPT_FOLLY
#include "folly/Function.h"
#else
#define OPT_FOLLY(...)
#endif
#ifdef ADD_BDE
#define OPT_BDE
#include "bslstl_function.h"
#else
#define OPT_BDE(...)
#endif

// Measurement.
#include "measure.hpp"  

#define MAX_REPEAT 100000

struct no_abstraction;

typedef generic::delegate<int(int)> generic_delegate;
typedef embxx::util::StaticFunction<int(int), 48> embxx_util_StaticFunction;
typedef Function<int(int), 56> Function_;
typedef stdext::inplace_function<int(int), 48> inplace_function;


template<class... Sig>
using multifunction =
    boost::type_erasure::any<
        boost::mpl::vector<
            boost::type_erasure::copy_constructible<>,
            boost::type_erasure::typeid_<>,
            boost::type_erasure::relaxed,
            boost::type_erasure::callable<Sig>...
        >
    >;

int plain(int val)
{
    return val * 2;
}

struct func1
{
    int operator()(int val)
    {
        return val * 2;
    }
    
    int a[10];
};

struct func2
{
    int operator()(int val)
    {
        return val * a;
    }
    
    int& a;
};

struct A
{
    A(): a(2) {}

    int f(int val)
    {
        return val * a;
    }
    
    int a;
};

namespace cases
{
    template<class F>
    struct base : test::base
    {
        base() {}

        template<class Fn>
        explicit base(Fn&& fn)
          : f(std::forward<Fn>(fn))
        {}
        
        void benchmark()
        {
            this->val += f(this->val);
        }
        
        F f;
    };

    template<class F>
    struct function_pointer : base<F>
    {
        function_pointer()
          : base<F>(&plain)
        {}
    };
    
    template<>
    struct function_pointer<no_abstraction>
      : function_pointer<int(*)(int)>
    {};

    template<class F>
    struct compile_time_function_pointer : base<F>
    {
        compile_time_function_pointer()
          : base<F>(stdex::function_wrapper<int(int), &plain>())
        {}
    };
    
    template<>
    struct compile_time_function_pointer<generic_delegate>
      : base<generic_delegate>
    {
        compile_time_function_pointer()
          : base<generic_delegate>(generic_delegate::from<&plain>())
        {}
    };
    
    template<>
    struct compile_time_function_pointer<no_abstraction>
      : compile_time_function_pointer<stdex::function_wrapper<int(int), &plain> >
    {};

    template<class F>
    struct compile_time_delegate : base<F>
    {
        compile_time_delegate()
          : base<F>(stdex::method_wrapper<A, int(int), &A::f>(&a))
        {}
        
        A a;
    };
    
    template<>
    struct compile_time_delegate<generic_delegate>
      : base<generic_delegate>
    {
        compile_time_delegate()
          : base<generic_delegate>(generic_delegate::from<A, &A::f>(&a))
        {}
        
        A a;
    };
    
    template<>
    struct compile_time_delegate<no_abstraction>
      : compile_time_delegate<stdex::method_wrapper<A, int(int), &A::f> >
    {};
            
    template<class F>
    struct stateless_lambda : base<F>
    {
        stateless_lambda()
        {
            this->f = [](int val)
            {
                return val * 2;
            };
        }
    };
    
    template<class F>
    struct lambda_capture : base<F>
    {
        lambda_capture()
        {
            int a = 2;
            this->f = [a](int val)
            {
                return val * a;
            };
        }
    };
    
    template<class F>
    struct heavy_functor : base<F>
    {
        heavy_functor()
          : base<F>(func1())
        {}
    };
    
    template<class F>
    struct non_assignable : base<F>
    {
        non_assignable()
          : base<F>(func2{a})
        {}
        
        int a = 2;
    };
}

#define DECLARE_BENCHMARK(name, list)                                           \
template<template<class> class Perf>                                            \
void benchmark_##name()                                                         \
{                                                                               \
    std::cout << "[" #name << "]\n";                                            \
    BOOST_SPIRIT_TEST_BENCHMARK(                                                \
        MAX_REPEAT,                                                             \
        list                                                                    \
    )                                                                           \
    std::cout << "\n";                                                          \
}                                                                               \
/***/

#define BENCHMARK(name) benchmark_##name<cases::name>()
#define SHOW_SIZE(name) std::cout << #name << ": " << sizeof(name) << "\n"

DECLARE_BENCHMARK(function_pointer,
    (Perf< no_abstraction >)
    (Perf< stdex::function<int(int)> >)
    (Perf< std::function<int(int)> >)
    (Perf< cxx_function::function<int(int)> >)
    (Perf< multifunction<int(int)> >)
    (Perf< boost::function<int(int)> >)
    (Perf< func::function<int(int)> >)
    (Perf< generic::delegate<int(int)> >)
    (Perf< fu2::function<int(int)> >)
    (Perf< fixed_size_function<int(int)> >)
    (Perf< embxx_util_StaticFunction >)
    (Perf< Function_ >)
    (Perf< inplace_function >)
    (Perf< delegate >)
    OPT_CLUGSTON(Perf< FastDelegate1 >)
    OPT_SSVU(Perf< ssvu::FastFunc<int(int)> >)
    OPT_GNR(Perf< gnr_forwarder >)
    OPT_FOLLY(Perf< folly::Function<int(int)> >)
    OPT_BDE(Perf< bsl::function<int(int)> >)
)

DECLARE_BENCHMARK(compile_time_function_pointer,
    (Perf< no_abstraction >)
    (Perf< stdex::function<int(int)> >)
    (Perf< std::function<int(int)> >)
    (Perf< cxx_function::function<int(int)> >)
    (Perf< multifunction<int(int)> >)
    (Perf< boost::function<int(int)> >)
    (Perf< func::function<int(int)> >)
    (Perf< generic::delegate<int(int)> >)
    (Perf< fu2::function<int(int)> >)
    (Perf< fixed_size_function<int(int)> >)
    (Perf< embxx_util_StaticFunction >)
    (Perf< Function_ >)
    (Perf< inplace_function >)
    (Perf< delegate >)
    //OPT_CLUGSTON(Perf< FastDelegate1 >)
    //OPT_SSVU(Perf< ssvu::FastFunc<int(int)> >)
    OPT_GNR(Perf< gnr_forwarder >)
    OPT_FOLLY(Perf< folly::Function<int(int)> >)
    OPT_BDE(Perf< bsl::function<int(int)> >)
)

DECLARE_BENCHMARK(compile_time_delegate,
    (Perf< no_abstraction >)
    (Perf< stdex::function<int(int)> >)
    (Perf< std::function<int(int)> >)
    (Perf< cxx_function::function<int(int)> >)
    (Perf< multifunction<int(int)> >)
    (Perf< boost::function<int(int)> >)
    (Perf< func::function<int(int)> >)
    (Perf< generic::delegate<int(int)> >)
    (Perf< fu2::function<int(int)> >)
    (Perf< fixed_size_function<int(int)> >)
    (Perf< embxx_util_StaticFunction >)
    (Perf< Function_ >)
    (Perf< inplace_function >)
    (Perf< delegate >)
    //OPT_CLUGSTON(Perf< FastDelegate1 >)
    //OPT_SSVU(Perf< ssvu::FastFunc<int(int)> >)
    OPT_GNR(Perf< gnr_forwarder >)
    OPT_FOLLY(Perf< folly::Function<int(int)> >)
    OPT_BDE(Perf< bsl::function<int(int)> >)
)

DECLARE_BENCHMARK(heavy_functor,
    (Perf< stdex::function<int(int)> >)
    (Perf< std::function<int(int)> >)
    (Perf< cxx_function::function<int(int)> >)
    (Perf< multifunction<int(int)> >)
    (Perf< boost::function<int(int)> >)
    (Perf< func::function<int(int)> >)
    (Perf< generic::delegate<int(int)> >)
    (Perf< fu2::function<int(int)> >)
    (Perf< fixed_size_function<int(int)> >)
    (Perf< embxx_util_StaticFunction >)
    (Perf< Function_ >)
    (Perf< inplace_function >)
    (Perf< delegate >)
    //OPT_CLUGSTON(Perf< FastDelegate1 >)
    //OPT_SSVU(Perf< ssvu::FastFunc<int(int)> >)
    OPT_GNR(Perf< gnr_forwarder >)
    OPT_FOLLY(Perf< folly::Function<int(int)> >)
    OPT_BDE(Perf< bsl::function<int(int)> >)
)

DECLARE_BENCHMARK(non_assignable,
    (Perf< stdex::function<int(int)> >)
    (Perf< std::function<int(int)> >)
    (Perf< cxx_function::function<int(int)> >)
    (Perf< multifunction<int(int)> >)
    (Perf< boost::function<int(int)> >)
    (Perf< func::function<int(int)> >)
    (Perf< generic::delegate<int(int)> >)
    (Perf< fu2::function<int(int)> >)
    (Perf< fixed_size_function<int(int)> >)
    (Perf< embxx_util_StaticFunction >)
    (Perf< Function_ >)
    (Perf< inplace_function >)
    (Perf< delegate >)
    //OPT_CLUGSTON(Perf< FastDelegate1 >)
    //OPT_SSVU(Perf< ssvu::FastFunc<int(int)> >)
    OPT_GNR(Perf< gnr_forwarder >)
    OPT_FOLLY(Perf< folly::Function<int(int)> >)
    OPT_BDE(Perf< bsl::function<int(int)> >)
)

DECLARE_BENCHMARK(lambda_capture,
    (Perf< stdex::function<int(int)> >)
    (Perf< std::function<int(int)> >)
    (Perf< cxx_function::function<int(int)> >)
    (Perf< multifunction<int(int)> >)
    (Perf< boost::function<int(int)> >)
    (Perf< func::function<int(int)> >)
    (Perf< generic::delegate<int(int)> >)
    (Perf< fu2::function<int(int)> >)
    (Perf< fixed_size_function<int(int)> >)
    (Perf< embxx_util_StaticFunction >)
    (Perf< Function_ >)
    (Perf< inplace_function >)
    (Perf< delegate >)
    //OPT_CLUGSTON(Perf< FastDelegate1 >)
    OPT_SSVU(Perf< ssvu::FastFunc<int(int)> >)
    OPT_GNR(Perf< gnr_forwarder >)
    OPT_FOLLY(Perf< folly::Function<int(int)> >)
    OPT_BDE(Perf< bsl::function<int(int)> >)
)

DECLARE_BENCHMARK(stateless_lambda,
    (Perf< stdex::function<int(int)> >)
    (Perf< std::function<int(int)> >)
    (Perf< cxx_function::function<int(int)> >)
    (Perf< multifunction<int(int)> >)
    (Perf< boost::function<int(int)> >)
    (Perf< func::function<int(int)> >)
    (Perf< generic::delegate<int(int)> >)
    (Perf< fu2::function<int(int)> >)
    (Perf< fixed_size_function<int(int)> >)
    (Perf< embxx_util_StaticFunction >)
    (Perf< Function_ >)
    (Perf< inplace_function >)
    (Perf< delegate >)
    OPT_CLUGSTON(Perf< FastDelegate1 >)
    OPT_SSVU(Perf< ssvu::FastFunc<int(int)> >)
    OPT_GNR(Perf< gnr_forwarder >)
    OPT_FOLLY(Perf< folly::Function<int(int)> >)
    OPT_BDE(Perf< bsl::function<int(int)> >)
)

int main(int /*argc*/, char* /*argv*/[])
{
    std::cout << "[size]\n";
    SHOW_SIZE(stdex::function<int(int)>);
    SHOW_SIZE(std::function<int(int)>);
    SHOW_SIZE(cxx_function::function<int(int)>);
    SHOW_SIZE(multifunction<int(int)>);
    SHOW_SIZE(boost::function<int(int)>);
    SHOW_SIZE(func::function<int(int)>);
    SHOW_SIZE(generic::delegate<int(int)>);
    SHOW_SIZE(fu2::function<int(int)>);
    SHOW_SIZE(fixed_size_function<int(int)>);
    SHOW_SIZE(embxx_util_StaticFunction);
    SHOW_SIZE(Function_);
    SHOW_SIZE(inplace_function);
    SHOW_SIZE(delegate);
    OPT_CLUGSTON(SHOW_SIZE(FastDelegate1));
    OPT_SSVU(SHOW_SIZE(ssvu::FastFunc<int(int)>));
    OPT_GNR(SHOW_SIZE(gnr_forwarder));
    OPT_FOLLY(SHOW_SIZE(folly::Function<int(int)>));
    OPT_BDE(SHOW_SIZE(bsl::function<int(int)>));
    std::cout << "\n";
    
    BENCHMARK(function_pointer);
    BENCHMARK(compile_time_function_pointer);
    BENCHMARK(compile_time_delegate);
    BENCHMARK(heavy_functor);
    BENCHMARK(non_assignable);
    BENCHMARK(lambda_capture);
    BENCHMARK(stateless_lambda);
    
    // This is ultimately responsible for preventing all the test code
    // from being optimized away.  Change this to return 0 and you
    // unplug the whole test's life support system.
    return test::live_code != 0;
}
