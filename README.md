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
fu2::function<int(int)>: 40

[function_pointer]
Perf< no_abstraction >: 0.3103266979 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.3930032573 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.4384391409 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4048384686 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3642173096 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.4289271318 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4474289095 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4101610338 [s] {checksum: 0}

[compile_time_function_pointer]
Perf< no_abstraction >: 0.0948516111 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.3054994933 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3320486388 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.3247071917 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3383932849 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3094601371 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.3890742935 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.3851613298 [s] {checksum: 0}

[compile_time_delegate]
Perf< no_abstraction >: 0.2350802257 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.3383824048 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.4250746481 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4621998037 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3788282037 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3465440127 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4270445700 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4830267037 [s] {checksum: 0}

[lambda]
Perf< stdex::function<int(int)> >: 0.3134233409 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3365472031 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.3210115082 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3563981021 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.2956556438 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.3784445233 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4069811907 [s] {checksum: 0}

[lambda_capture]
Perf< stdex::function<int(int)> >: 0.3129580604 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.4082266319 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4177444010 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3543888202 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3088639765 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4495399516 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4186756019 [s] {checksum: 0}

[heavy_functor]
Perf< stdex::function<int(int)> >: 0.2710863402 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3341798408 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.3170371044 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3361158427 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.2908674792 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.3839290086 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4583521200 [s] {checksum: 0}

[non_assignable]
Perf< stdex::function<int(int)> >: 0.3645773100 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.4154650388 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4756644566 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3696365149 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3509990570 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4979358380 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4458388280 [s] {checksum: 0}
```

#### [overload.cpp](overload.cpp)
This shows the timing of each multi-method technique.
```
with< no_abstraction >: 0.0904054689 [s] {checksum: 3}
with< stdex::function<Sig...> >: 0.6127457961 [s] {checksum: 3}
with< multifunction<Sig...> >: 0.7170735095 [s] {checksum: 3}
with< virtual_base& >: 0.6407742850 [s] {checksum: 3}
```
