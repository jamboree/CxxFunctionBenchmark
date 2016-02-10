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
#include "FastFunc.hpp"
#include "stdex.hpp"
#ifndef _WIN32
    #include "cxx_function.hpp"
#else
    #include "cxx_function_msvc.hpp"
#endif
#include "measure.hpp"  


#define MAX_REPEAT 100000


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

template<int i>
struct tag {};

struct empty_base {};

struct virtual_base
{
    virtual int operator()(tag<0>) = 0;
    virtual int operator()(tag<1>) = 0;
    virtual int operator()(tag<2>) = 0;
};

template<class Base>
struct functor : Base
{
    int operator()(tag<0>)
    {
        return 0;
    }
    
    int operator()(tag<1>)
    {
        return 1;
    }
    
    int operator()(tag<2>)
    {
        return 2;
    }
};

template<class F>
struct use_base
{
    typedef empty_base type;
};

template<>
struct use_base<virtual_base&>
{
    typedef virtual_base type;
};

template<class F>
struct with : test::base
{
    with()
      : f(h)
    {}
    
    void benchmark()
    {
        this->val += f(tag<0>());
        this->val += f(tag<1>());
        this->val += f(tag<2>());
    }
    
    F f;
    functor<typename use_base<F>::type> h;
};

template<class... Sig>
void benchmark()
{
    typedef functor<empty_base> no_abstraction;
    
    BOOST_SPIRIT_TEST_BENCHMARK(
        MAX_REPEAT,
        (with< no_abstraction >)
        (with< stdex::function<Sig...> >)
        (with< multifunction<Sig...> >)
        (with< cxx_function::function<Sig...> >)
        (with< virtual_base& >)
    )
}

int main(int /*argc*/, char* /*argv*/[])
{
    benchmark<int(tag<0>), int(tag<1>), int(tag<2>)>();

    // This is ultimately responsible for preventing all the test code
    // from being optimized away.  Change this to return 0 and you
    // unplug the whole test's life support system.
    return test::live_code != 0;
}
