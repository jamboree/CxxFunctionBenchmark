C++ Function Benchmark
======================

Benchmark for various C++ function implementations.

Currently include:
- [stdex::function](stdex.hpp) - A multi-signature function implementation authored by me :)
- multifunction - Example from [Boost.TypeErasure](http://www.boost.org/doc/html/boost_typeerasure/examples.html#boost_typeerasure.examples.multifunction), another multi-signature function.
- std::function - [Standard](http://en.cppreference.com/w/cpp/utility/functional/function).
- boost::function - The one from [Boost](http://www.boost.org/doc/libs/1_55_0/doc/html/function.html).
- [func::function](function.h) - From this [blog](http://probablydance.com/2013/01/13/a-faster-implementation-of-stdfunction/).
- [generic::delegate](delegate.hpp) - [Fast delegate in C++11](http://codereview.stackexchange.com/questions/14730/impossibly-fast-delegate-in-c11), also see [here](https://github.com/user1095108/generic).
- [~~fastdelegate::FastDelegate~~](clugston_styled/FastDelegate.h) - By Don Clugston, see [here](https://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible)
- [~~ssvu::FastFunc~~](clugston_styled/FastFunc.hpp) - Another Don Clugston's FastDelegate, as shown [here](https://gist.github.com/SuperV1234/6462221) and [here](https://groups.google.com/a/isocpp.org/forum/#!topic/std-discussion/QgvHF7YMi3o).
- [cxx_function::function](https://github.com/potswa/cxx_function) - By David Krauss
- [fu2::function](http://naios.github.io/function2/) - By Denis Blank
- [fixed_size_function](https://github.com/pmed/fixed_size_function) - By Pavel Medvedev
- [gnr::forwarder](forwarder.hpp) - [here](https://github.com/user1095108/generic).
- [embxx::util::StaticFunction](StaticFunction.h) - By Alex Robenko, see [here](https://github.com/arobenko/embxx) - either on master or develop branch.
- [Function](Function.h) - By Erik Rigtorp, see [here](https://github.com/rigtorp/Function)

### Sample Result
Compiled with `g++ -O3 -std=c++17 -DNDEBUG` (64-bit/g++6.3.0/Debian)

#### [various.cpp](various.cpp)
The size of each implementation is shown in the `[size]` section.
Keep in mind that bigger size does not necessarily mean that it's less space efficient, it may due to the decision on how much space is reserved for small object optimization.
Other sections show the timing of function invocation for each implementation when assigned different callable objects.
```
[size]
stdex::function<int(int)>: 24
std::function<int(int)>: 32
cxx_function::function<int(int)>: 32
multifunction<int(int)>: 32
boost::function<int(int)>: 32
func::function<int(int)>: 32
generic::delegate<int(int)>: 48
ssvu::FastFunc<int(int)>: 40
fu2::function<int(int)>: 32
fixed_size_function<int(int)>: 128
gnr_forwarder: 64
embxx_util_StaticFunction: 64

[function_pointer]
Perf< no_abstraction >: 0.1289256000 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.2058285690 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.1802342480 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.1801737160 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.2058923480 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.2058691890 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.1802219080 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.2061577750 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.2058619020 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.2097225940 [s] {checksum: 0}
Perf< gnr_forwarder >: 0.1830669430 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.1805845430 [s] {checksum: 0}

[compile_time_function_pointer]
Perf< no_abstraction >: 0.4351440410 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 1.2887670140 [s] {checksum: 0}
Perf< std::function<int(int)> >: 1.2970851790 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 1.3579650290 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 1.5456079470 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 1.2915591720 [s] {checksum: 0}
Perf< func::function<int(int)> >: 1.2892671050 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 1.3306471250 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 1.2898244390 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 1.4961672200 [s] {checksum: 0}
Perf< gnr_forwarder >: 1.4465658510 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 1.5629812100 [s] {checksum: 0}

[compile_time_delegate]
Perf< no_abstraction >: 0.6912519020 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 1.5463906880 [s] {checksum: 0}
Perf< std::function<int(int)> >: 1.4930048310 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 1.5482560370 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 1.6140930180 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 1.4045358160 [s] {checksum: 0}
Perf< func::function<int(int)> >: 1.3385393970 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 1.6504875350 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 1.8179735180 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 2.0437727990 [s] {checksum: 0}
Perf< gnr_forwarder >: 1.7408435590 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 1.6159000050 [s] {checksum: 0}

[lambda]
Perf< stdex::function<int(int)> >: 1.5452568910 [s] {checksum: 0}
Perf< std::function<int(int)> >: 1.3033105070 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 1.5468615110 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 1.2902803330 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 1.5454352390 [s] {checksum: 0}
Perf< func::function<int(int)> >: 1.2907922800 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 1.5462030510 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 1.5455065390 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 1.6342289650 [s] {checksum: 0}
Perf< gnr_forwarder >: 1.4307872090 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 1.3791516070 [s] {checksum: 0}

[lambda_capture]
Perf< stdex::function<int(int)> >: 1.2894260190 [s] {checksum: 0}
Perf< std::function<int(int)> >: 1.5462926690 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 1.4566374560 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 1.4465339330 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 1.2995729940 [s] {checksum: 0}
Perf< func::function<int(int)> >: 1.2912875910 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 1.6997674130 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 1.5460056580 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 1.6284502350 [s] {checksum: 0}
Perf< gnr_forwarder >: 1.6119621950 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 1.4028525370 [s] {checksum: 0}

[heavy_functor]
Perf< stdex::function<int(int)> >: 1.2896630640 [s] {checksum: 0}
Perf< std::function<int(int)> >: 1.5452496180 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 1.5464160470 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 1.5455833060 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 1.5455709530 [s] {checksum: 0}
Perf< func::function<int(int)> >: 1.2892575070 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 1.5461874570 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 1.2899274400 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 1.6340354600 [s] {checksum: 0}
Perf< gnr_forwarder >: 1.4505575760 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 1.3804517890 [s] {checksum: 0}

[non_assignable]
Perf< stdex::function<int(int)> >: 1.5453513470 [s] {checksum: 0}
Perf< std::function<int(int)> >: 1.4913542030 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 1.5502585200 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 1.5976331620 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 1.5460646450 [s] {checksum: 0}
Perf< func::function<int(int)> >: 1.3371734510 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 1.8964442520 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 1.8175570490 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 2.0424740460 [s] {checksum: 0}
Perf< gnr_forwarder >: 1.7304754280 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 1.6222692270 [s] {checksum: 0}
```

#### [overload.cpp](overload.cpp)
This shows the timing of each multi-method technique.
```
with< no_abstraction >: 0.0350349190 [s] {checksum: 3}
with< stdex::function<Sig...> >: 0.5442975940 [s] {checksum: 3}
with< multifunction<Sig...> >: 0.5424543900 [s] {checksum: 3}
with< cxx_function::function<Sig...> >: 0.3608255140 [s] {checksum: 3}
with< fu2::function<Sig...> >: 0.5393762050 [s] {checksum: 3}
with< virtual_base& >: 0.1433718300 [s] {checksum: 3}
```
