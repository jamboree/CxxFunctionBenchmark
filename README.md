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
fu2::function<int(int)>: 48

[function_pointer]
Perf< no_abstraction >: 0.3286194148 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.3767377994 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.4807560062 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.3882401958 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4234715445 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3923073945 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.4452798575 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4719214490 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4047896305 [s] {checksum: 0}

[compile_time_function_pointer]
Perf< no_abstraction >: 0.0897049313 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.2958821453 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3641301235 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.3778190791 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.3241598963 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3528683671 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3260763757 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4045848305 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4053073903 [s] {checksum: 0}

[compile_time_delegate]
Perf< no_abstraction >: 0.2305730462 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.3440424499 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3596264449 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.4668491306 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4504404959 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3979345927 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3625934840 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4672430505 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.5083233573 [s] {checksum: 0}

[lambda]
Perf< stdex::function<int(int)> >: 0.2708261533 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3635150837 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.3583154053 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.3510440477 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3386315716 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3100507808 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4104049887 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4347112209 [s] {checksum: 0}

[lambda_capture]
Perf< stdex::function<int(int)> >: 0.2985829445 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.4066587499 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.3610744445 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4516372955 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3513240476 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3234498165 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4802673663 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4442859378 [s] {checksum: 0}

[heavy_functor]
Perf< stdex::function<int(int)> >: 0.2570539977 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3547432465 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.3871566761 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.3587218052 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3386427716 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3344293330 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4125509080 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4088446692 [s] {checksum: 0}

[non_assignable]
Perf< stdex::function<int(int)> >: 0.3667157227 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3461320492 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.4813867260 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4495272162 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.4026929911 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3706642014 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.5464155051 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.5045559985 [s] {checksum: 0}
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
