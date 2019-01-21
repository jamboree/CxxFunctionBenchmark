C++ Function Benchmark
======================

Benchmark for various C++ function implementations. This project focuses on invocation time.

Currently include:
- [stdex::function](stdex.hpp) - A multi-signature function implementation by [@jamboree](https://github.com/jamboree).
- multifunction - Example from [Boost.TypeErasure](http://www.boost.org/doc/html/boost_typeerasure/examples.html#boost_typeerasure.examples.multifunction), another multi-signature function.
- std::function - [Standard](http://en.cppreference.com/w/cpp/utility/functional/function).
- boost::function - The one from [Boost](http://www.boost.org/doc/libs/1_55_0/doc/html/function.html).
- [func::function](function.h) - From this [blog](http://probablydance.com/2013/01/13/a-faster-implementation-of-stdfunction/).
- [generic::delegate](delegate.hpp) - [Fast delegate in C++11](http://codereview.stackexchange.com/questions/14730/impossibly-fast-delegate-in-c11), also see [here](https://github.com/user1095108/generic).
- [~~fastdelegate::FastDelegate~~](clugston_styled/FastDelegate.h) - By Don Clugston, see [here](https://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible)
- [~~ssvu::FastFunc~~](clugston_styled/FastFunc.hpp) - Another Don Clugston's FastDelegate, as shown [here](https://gist.github.com/SuperV1234/6462221) and [here](https://groups.google.com/a/isocpp.org/forum/#!topic/std-discussion/QgvHF7YMi3o).
- [cxx_function::function](https://github.com/potswa/cxx_function) - By David Krauss
- [fu2::function](https://github.com/Naios/function2) - By Denis Blank
- [fixed_size_function](https://github.com/pmed/fixed_size_function) - By Pavel Medvedev
- [gnr::forwarder](forwarder.hpp) - [here](https://github.com/user1095108/generic).
- [embxx::util::StaticFunction](embxx/StaticFunction.h) - By Alex Robenko, see [here](https://github.com/arobenko/embxx) - either on master or develop branch.
- [Function](Function-rigtorp.h) - By Erik Rigtorp, see [here](https://github.com/rigtorp/Function)
- [folly::Function](folly/Function.h) - From Facebook Folly; see [here](https://github.com/facebook/folly/blob/master/folly/docs/Function.md)
- [bsl::function](bde/groups/bsl/bslstl/bslstl_function.h) - From Bloomberg BDE; see [here](https://github.com/bloomberg/bde)
- [stdext::inplace_function](inplace_function.h) - From SG14; see [here](https://github.com/WG21-SG14/SG14) and [here](https://github.com/WG21-SG14/SG14/blob/master/Docs/Proposals/NonAllocatingStandardFunction.pdf)
- [Delegate::Func](delegate.h) - A non-allocating implementation by Ben Diamand; see [here](https://github.com/bdiamand/Delegate)

### Sample Result
Compiled with MSVC (64-bit/Visual Studio 15.9.4/Release Build/Boost 1.69.0)

#### [various.cpp](various.cpp)
The size of each implementation is shown in the `[size]` section.
Keep in mind that bigger size does not necessarily mean that it's less space efficient, it may due to the decision on how much space is reserved for small object optimization.
Other sections show the timing of function invocation for each implementation when assigned different callable objects.
```
[size]
stdex::function<int(int)>: 24
std::function<int(int)>: 64
cxx_function::function<int(int)>: 40
multifunction<int(int)>: 32
boost::function<int(int)>: 40
func::function<int(int)>: 32
generic::delegate<int(int)>: 48
fu2::function<int(int)>: 32
fixed_size_function<int(int)>: 128
embxx_util_StaticFunction: 64
Function_: 56
inplace_function: 56
delegate: 48

[function_pointer]
Perf< no_abstraction >: 0.1955783324 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.3078637900 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3348745311 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.3068508420 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.3627002775 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3348277611 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.2790730726 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.2792239436 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.3069455890 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.3069486064 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.3624730658 [s] {checksum: 0}
Perf< Function_ >:    0.3345429166 [s] {checksum: 0}
Perf< inplace_function >: 0.2791053590 [s] {checksum: 0}
Perf< delegate >:     0.2789282364 [s] {checksum: 0}

[compile_time_function_pointer]
Perf< no_abstraction >: 0.0510583753 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.1956211798 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.2236116773 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.1978697615 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.2823780532 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.2821846366 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.1696668376 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.1681581273 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.1972167917 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.1974599958 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.2552595902 [s] {checksum: 0}
Perf< Function_ >:    0.2234502453 [s] {checksum: 0}
Perf< inplace_function >: 0.1701988088 [s] {checksum: 0}
Perf< delegate >:     0.1678216849 [s] {checksum: 0}

[compile_time_delegate]
Perf< no_abstraction >: 0.0580427985 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.1977517804 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.2258786653 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.1995896912 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.2515665693 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.2257857287 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.1682127426 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.1955499686 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.1979856304 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.1962928576 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.2790658308 [s] {checksum: 0}
Perf< Function_ >:    0.2237018982 [s] {checksum: 0}
Perf< inplace_function >: 0.1701191489 [s] {checksum: 0}
Perf< delegate >:     0.1701864374 [s] {checksum: 0}

[heavy_functor]
Perf< stdex::function<int(int)> >: 0.1699797441 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.2235661142 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.2254523038 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.2511525792 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.1959437420 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.1956558801 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.1869998060 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.1980782652 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.1982559913 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.2512943979 [s] {checksum: 0}
Perf< Function_ >:    0.2235646055 [s] {checksum: 0}
Perf< inplace_function >: 0.2215842725 [s] {checksum: 0}
Perf< delegate >:     0.1400502521 [s] {checksum: 0}

[non_assignable]
Perf< stdex::function<int(int)> >: 0.1976869058 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.2343666691 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.1719030479 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.2540701230 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.2255914069 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.1702887279 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.1783579137 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.2264733989 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.1687830351 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.2239915705 [s] {checksum: 0}
Perf< Function_ >:    0.1989358161 [s] {checksum: 0}
Perf< inplace_function >: 0.1721812540 [s] {checksum: 0}
Perf< delegate >:     0.1700907851 [s] {checksum: 0}

[lambda_capture]
Perf< stdex::function<int(int)> >: 0.1697537393 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.1985438532 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.1979403691 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.2246674727 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.1979823113 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.1698122772 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.1961975071 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.2511836586 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.1992982083 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.2265388769 [s] {checksum: 0}
Perf< Function_ >:    0.2262256687 [s] {checksum: 0}
Perf< inplace_function >: 0.2156960782 [s] {checksum: 0}
Perf< delegate >:     0.1420637768 [s] {checksum: 0}

[stateless_lambda]
Perf< stdex::function<int(int)> >: 0.1981473642 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.2530300182 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.1976594473 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.2544611807 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.2233192893 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.1701330290 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.1681895085 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.1956549749 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.2002571446 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.2546832629 [s] {checksum: 0}
Perf< Function_ >:    0.2262651969 [s] {checksum: 0}
Perf< inplace_function >: 0.1827283456 [s] {checksum: 0}
Perf< delegate >:     0.1678956117 [s] {checksum: 0}
```

#### [overload.cpp](overload.cpp)
This shows the timing of each multi-method technique.
```
Perf< no_abstraction >: 0.0390731812 [s] {checksum: 3}
Perf< stdex::function<Sig...> >: 0.4739417077 [s] {checksum: 3}
Perf< multifunction<Sig...> >: 0.7793043578 [s] {checksum: 3}
Perf< cxx_function::function<Sig...> >: 0.5055884136 [s] {checksum: 3}
Perf< fu2::function<Sig...> >: 0.4742775466 [s] {checksum: 3}
Perf< virtual_base& >: 0.5028338104 [s] {checksum: 3}
```
