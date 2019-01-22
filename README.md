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
inplace_function: 48
Delegate: 56

[function_pointer]
Perf< no_abstraction >: 0.1581233938 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.2369854883 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.2639573047 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.2635273223 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.2633797704 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.2632382534 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.2367664236 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.2370002737 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.2363500196 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.2627183519 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.2630701831 [s] {checksum: 0}
Perf< Function_ >:    0.2629322870 [s] {checksum: 0}
Perf< inplace_function >: 0.2364676990 [s] {checksum: 0}
Perf< Delegate >:     0.2101452315 [s] {checksum: 0}

[compile_time_function_pointer]
Perf< no_abstraction >: 0.0533959710 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.1576716859 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.1884306668 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.1845623337 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.1847705357 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.1844458613 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.1580304572 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.1585376856 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.1911532853 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.1854126428 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.1866678897 [s] {checksum: 0}
Perf< Function_ >:    0.2034746200 [s] {checksum: 0}
Perf< inplace_function >: 0.1583309923 [s] {checksum: 0}
Perf< Delegate >:     0.1323627700 [s] {checksum: 0}

[compile_time_delegate]
Perf< no_abstraction >: 0.0546856165 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.1580582175 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.1906249349 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.1849874883 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.1854241090 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.1845475484 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.1583859094 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.1584073331 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.2108410486 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.1846688487 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.1865227518 [s] {checksum: 0}
Perf< Function_ >:    0.1851751718 [s] {checksum: 0}
Perf< inplace_function >: 0.1593198010 [s] {checksum: 0}
Perf< Delegate >:     0.1395119443 [s] {checksum: 0}

[heavy_functor]
Perf< stdex::function<int(int)> >: 0.1579305806 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.1896611708 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.1585654459 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.2108531183 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.1583361219 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.1845744034 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.1842995164 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.1584091435 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.2105242195 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.1869382506 [s] {checksum: 0}
Perf< Function_ >:    0.2071549678 [s] {checksum: 0}
Perf< inplace_function >: 0.1854696721 [s] {checksum: 0}
Perf< Delegate >:     0.1586031636 [s] {checksum: 0}

[non_assignable]
Perf< stdex::function<int(int)> >: 0.1844521979 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.1860469046 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.1845928097 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.2108033309 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.2104690007 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.1583373289 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.1647982296 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.2116792880 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.1589755133 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.1867176772 [s] {checksum: 0}
Perf< Function_ >:    0.1863730877 [s] {checksum: 0}
Perf< inplace_function >: 0.1588889134 [s] {checksum: 0}
Perf< Delegate >:     0.1401612932 [s] {checksum: 0}

[lambda_capture]
Perf< stdex::function<int(int)> >: 0.1594558867 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.1756666764 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.1849205015 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.2112505126 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.1586876514 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.1578183326 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.1855617034 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.2103977896 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.1852678066 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.2107188431 [s] {checksum: 0}
Perf< Function_ >:    0.1974609010 [s] {checksum: 0}
Perf< inplace_function >: 0.1851042624 [s] {checksum: 0}
Perf< Delegate >:     0.1584510857 [s] {checksum: 0}

[stateless_lambda]
Perf< stdex::function<int(int)> >: 0.1588940430 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.1799158079 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.1848345050 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.2104273603 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.1843752537 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.1583047408 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.1582446941 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.1841890788 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.1860281966 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.1871841704 [s] {checksum: 0}
Perf< Function_ >:    0.2044821367 [s] {checksum: 0}
Perf< inplace_function >: 0.1845451344 [s] {checksum: 0}
Perf< Delegate >:     0.1324608361 [s] {checksum: 0}
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
