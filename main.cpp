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
        
namespace cases
{
    template<class F>
    struct base : test::base
    {
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
        {
            this->f = &plain;
        }
    };
    
    template<class F>
    struct lambda : base<F>
    {
        lambda()
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
        lambda_capture(int a = 2)
        {
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
        {
            this->f = func1();
        }
    };
    
    template<class F>
    struct non_assignable : base<F>
    {
        non_assignable()
        {
            this->f = func2{a};
        }
        
        int a = 2;
    };
}

template<template<class> class Perf>
void benchmark(char const* name)
{
    std::cout << "[" << name << "]\n";
    BOOST_SPIRIT_TEST_BENCHMARK(
        MAX_REPEAT,
        (Perf< stdex::function<int(int)> >)
        (Perf< std::function<int(int)> >)
        (Perf< multifunction<int(int)> >)
        (Perf< boost::function<int(int)> >)
        (Perf< func::function<int(int)> >)
        (Perf< generic::delegate<int(int)> >)
        //(Perf< ssvu::FastFunc<int(int)> >)
    )
    std::cout << std::endl;
}
     
#define BENCHMARK(name) benchmark<cases::name>(#name)
#define SHOW_SIZE(name) \
std::cout << #name << ": " << sizeof(name) << std::endl;


int main(int argc, char *argv[])
{
    std::cout << "[size]\n";
    SHOW_SIZE(stdex::function<int(int)>);
    SHOW_SIZE(std::function<int(int)>);
    SHOW_SIZE(multifunction<int(int)>);
    SHOW_SIZE(boost::function<int(int)>);
    SHOW_SIZE(func::function<int(int)>);
    SHOW_SIZE(generic::delegate<int(int)>);
    SHOW_SIZE(ssvu::FastFunc<int(int)>);
    std::cout << std::endl;
    
    BENCHMARK(function_pointer);
    BENCHMARK(lambda);
    BENCHMARK(lambda_capture);
    BENCHMARK(heavy_functor);
    BENCHMARK(non_assignable);

    // This is ultimately responsible for preventing all the test code
    // from being optimized away.  Change this to return 0 and you
    // unplug the whole test's life support system.
    return test::live_code != 0;
}
