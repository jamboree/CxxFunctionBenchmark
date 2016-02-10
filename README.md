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
Compiled with `g++ -O3 -std=c++14 -DNDEBUG` (64-bit/g++5.3.0/MinGW)

#### [various.cpp](various.cpp)
This test shows the internal size of each implementation in the `[size]` section.
Note that bigger size does not necessarily mean that it's less space efficient, it may due to the decision on how much space is used for small object optimization.
The rest part shows the timing of each implementation when assigned different callable objects.
```
[size]
stdex::function<int(int)>: 24
std::function<int(int)>: 32
cxx_function::function<int(int)>: 32
multifunction<int(int)>: 32
boost::function<int(int)>: 32
func::function<int(int)>: 32
generic::delegate<int(int)>: 56
ssvu::FastFunc<int(int)>: 40
fu2::function<int(int)>: 40

[function_pointer]
Perf< no_abstraction >: 0.2939271719 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.3526247343 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.4566822677 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.3691389638 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4132227755 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.4079642189 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3633898474 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4593472260 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4513318711 [s] {checksum: 0}

[compile_time_function_pointer]
Perf< no_abstraction >: 0.0930527404 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.2811758200 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3349437856 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.3452720990 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.3271380306 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3292356293 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3276167503 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.3924880688 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4055834204 [s] {checksum: 0}

[compile_time_delegate]
Perf< no_abstraction >: 0.2275358544 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.3281178700 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3548180129 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.4592150661 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4654515421 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3919911091 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3476964175 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4512685112 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4816550517 [s] {checksum: 0}

[lambda]
Perf< stdex::function<int(int)> >: 0.3051889247 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3503760958 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.3628842478 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.3209757946 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3372948241 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3142730789 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.3952301471 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.3925488688 [s] {checksum: 0}

[lambda_capture]
Perf< stdex::function<int(int)> >: 0.3031690860 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3768906388 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.3680196045 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4951856031 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3838560743 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3859239130 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4922768049 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4635488233 [s] {checksum: 0}

[heavy_functor]
Perf< stdex::function<int(int)> >: 0.2997751681 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3388301831 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.3435233001 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.3468218580 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3403156222 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3396359426 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.3831424748 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4705926588 [s] {checksum: 0}

[non_assignable]
Perf< stdex::function<int(int)> >: 0.3570794515 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3572237714 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.4494256324 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4548624289 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.4176397327 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3484343370 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.5342012581 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4777555342 [s] {checksum: 0}
```

#### [overload.cpp](overload.cpp)
This shows the timing of each multi-method technique.
```
with< no_abstraction >: 0.0712217144 [s] {checksum: 3}
with< stdex::function<Sig...> >: 0.5929125805 [s] {checksum: 3}
with< multifunction<Sig...> >: 0.7295508931 [s] {checksum: 3}
with< cxx_function::function<Sig...> >: 0.7109221050 [s] {checksum: 3}
with< virtual_base& >: 0.4269222868 [s] {checksum: 3}
```
