C++ Function Benchmark
======================

Benchmark for various C++ function implementations.

Currently include:
- [stdex::function](stdex.hpp) - A multi-signature function implementation authored by me :)
- multifunction - Example from [Boost.TypeErasure](http://www.boost.org/doc/html/boost_typeerasure/examples.html#boost_typeerasure.examples.multifunction), another multi-signature function.
- std::function - [Standard](http://en.cppreference.com/w/cpp/utility/functional/function).
- boost::function - The one from [Boost](http://www.boost.org/doc/libs/1_55_0/doc/html/function.html).
- [func::function](function.h) - From this [blog](http://probablydance.com/2013/01/13/a-faster-implementation-of-stdfunction/).
- [generic::delegate](delegate.hpp) - [Fast delegate in C++11](http://codereview.stackexchange.com/questions/14730/impossibly-fast-delegate-in-c11), also see [here](https://code.google.com/p/cpppractice/source/browse/trunk/).
- [~~ssvu::FastFunc~~](FastFunc.hpp) - Another Don Clugston's FastDelegate, as shown [here](https://groups.google.com/a/isocpp.org/forum/#!topic/std-discussion/QgvHF7YMi3o).
- [cxx_function::function](https://github.com/potswa/cxx_function) - By David Krauss
- [fu2::function](http://naios.github.io/function2/) - By Denis Blank

### Sample Result
Compiled with `g++ -O3 -std=c++11` (64-bit)

#### [various.cpp](various.cpp)
This test shows the internal size of each implementation at the beginning.
Note that bigger does not necessarily mean that it's less space efficient, it may due to the decision on how much space is used for small object optimization.
The rest part shows the timing of each implementation when assigned different callable objects.
```
[size]
stdex::function<int(int)>: 24
std::function<int(int)>: 32
multifunction<int(int)>: 32
boost::function<int(int)>: 32
func::function<int(int)>: 32
generic::delegate<int(int)>: 56
ssvu::FastFunc<int(int)>: 40

[function_pointer]
Perf< no_abstraction >: 0.3105436800 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.3560464000 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.4071398400 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4019267200 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3702688000 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3932073600 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4362761600 [s] {checksum: 0}

[compile_time_function_pointer]
Perf< no_abstraction >: 0.0835740800 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.3015212800 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3304796800 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.2954928000 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3464038400 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3027420800 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.3599926400 [s] {checksum: 0}

[compile_time_delegate]
Perf< no_abstraction >: 0.2044451200 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.3466518400 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.4033782400 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4393961600 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3593200000 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3500022400 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4184252800 [s] {checksum: 0}

[lambda]
Perf< stdex::function<int(int)> >: 2.9724387200 [s] {checksum: 0}
Perf< std::function<int(int)> >: 3.3818099200 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 3.0920300800 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 3.4442572800 [s] {checksum: 0}
Perf< func::function<int(int)> >: 2.8562697600 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 3.6796515200 [s] {checksum: 0}

[lambda_capture]
Perf< stdex::function<int(int)> >: 0.3049206400 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.4018169600 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4377216000 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3488982400 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3103478400 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4399792000 [s] {checksum: 0}

[heavy_functor]
Perf< stdex::function<int(int)> >: 3.0993760000 [s] {checksum: 0}
Perf< std::function<int(int)> >: 3.1490044800 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 3.1722976000 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 3.3530864000 [s] {checksum: 0}
Perf< func::function<int(int)> >: 2.7634384000 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 3.7161500800 [s] {checksum: 0}

[non_assignable]
Perf< stdex::function<int(int)> >: 0.3395964800 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3935945600 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4375843200 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3609814400 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3286144000 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.5016777600 [s] {checksum: 0}
```

#### [overload.cpp](overload.cpp)
This shows the timing of each multi-method technique.
```
with< no_abstraction >: 0.0904054689 [s] {checksum: 3}
with< stdex::function<Sig...> >: 0.6127457961 [s] {checksum: 3}
with< multifunction<Sig...> >: 0.7170735095 [s] {checksum: 3}
with< virtual_base& >: 0.6407742850 [s] {checksum: 3}
```
