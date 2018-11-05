C++ Function Benchmark
======================

Benchmark for various C++ function implementations.

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
- [embxx::util::StaticFunction](StaticFunction.h) - By Alex Robenko, see [here](https://github.com/arobenko/embxx) - either on master or develop branch.
- [Function](Function.h) - By Erik Rigtorp, see [here](https://github.com/rigtorp/Function)
- [folly::Function](folly/Function.h) - From Facebook Folly; see [here](https://github.com/facebook/folly/blob/master/folly/docs/Function.md)
- [bsl::function](bde/groups/bsl/bslstl/bslstl_function.h) - From Bloomberg BDE; see [here](https://github.com/bloomberg/bde)

### Sample Result
Compiled with MSVC (64-bit/Visual Studio 15.8.9/Release Build/Boost 1.68.0)

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

[function_pointer]
Perf< no_abstraction >: 0.1330217596 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.2122339664 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.2399393114 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.2386651034 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.2141137326 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.2151225938 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.2121811386 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.2123649795 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.2420536333 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.2389506757 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.2130780047 [s] {checksum: 0}
Perf< Function_ >:    0.2130749860 [s] {checksum: 0}

[compile_time_function_pointer]
Perf< no_abstraction >: 0.3972803010 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 1.5937800175 [s] {checksum: 0}
Perf< std::function<int(int)> >: 1.8513632305 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 1.6328985953 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 1.5960648980 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 1.5989849202 [s] {checksum: 0}
Perf< func::function<int(int)> >: 1.3389178922 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 1.6025210662 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 1.5986084840 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 1.8629920010 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 1.8321471718 [s] {checksum: 0}
Perf< Function_ >:    1.8688015562 [s] {checksum: 0}

[compile_time_delegate]
Perf< no_abstraction >: 0.0626496160 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.1589898226 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.2116471244 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.1846861995 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.1856941551 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.1645116925 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.1423058787 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.1793970741 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.2156849841 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.2176550105 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.1899209876 [s] {checksum: 0}
Perf< Function_ >:    0.1872478979 [s] {checksum: 0}

[heavy_functor]
Perf< stdex::function<int(int)> >: 1.5955586561 [s] {checksum: 0}
Perf< std::function<int(int)> >: 1.8375015018 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 1.6050371818 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 1.6027912430 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 1.6006808455 [s] {checksum: 0}
Perf< func::function<int(int)> >: 1.3345585372 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 1.6102707624 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 1.5997166615 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 1.8603795878 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 1.8277974767 [s] {checksum: 0}
Perf< Function_ >:    1.8649366700 [s] {checksum: 0}

[non_assignable]
Perf< stdex::function<int(int)> >: 0.1590927615 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.2110050395 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.1838614811 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.1871413366 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.1631176407 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.1413607129 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.2028013256 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.2168885537 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.2178153053 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.1893721816 [s] {checksum: 0}
Perf< Function_ >:    0.1874921136 [s] {checksum: 0}

[lambda_capture]
Perf< stdex::function<int(int)> >: 1.5930697092 [s] {checksum: 0}
Perf< std::function<int(int)> >: 1.9196068641 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 1.8563353887 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 1.6529472210 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 1.6056910397 [s] {checksum: 0}
Perf< func::function<int(int)> >: 1.5992049860 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 1.8260656285 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 1.8579377332 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 2.0478922288 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 1.8469540663 [s] {checksum: 0}
Perf< Function_ >:    1.7864459408 [s] {checksum: 0}

[stateless_lambda]
Perf< stdex::function<int(int)> >: 1.5926567463 [s] {checksum: 0}
Perf< std::function<int(int)> >: 1.8454776031 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 1.6209733850 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 1.6152552974 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 1.6308521949 [s] {checksum: 0}
Perf< func::function<int(int)> >: 1.3416845627 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 1.6141353468 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 1.6046740280 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 1.8767809781 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 1.8332145964 [s] {checksum: 0}
Perf< Function_ >:    1.8790269169 [s] {checksum: 0}
```

#### [overload.cpp](overload.cpp)
This shows the timing of each multi-method technique.
```
Perf< no_abstraction >: 0.0336299140 [s] {checksum: 3}
Perf< stdex::function<Sig...> >: 0.4114143195 [s] {checksum: 3}
Perf< multifunction<Sig...> >: 0.4737098380 [s] {checksum: 3}
Perf< cxx_function::function<Sig...> >: 0.4740204658 [s] {checksum: 3}
Perf< fu2::function<Sig...> >: 0.4370874521 [s] {checksum: 3}
Perf< virtual_base& >: 0.3996554416 [s] {checksum: 3}
```
