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
- [fixed_size_function](https://github.com/pmed/fixed_size_function) - By Pavel Medvedev

### Sample Result
Compiled with `g++ -O3 -std=c++14 -DNDEBUG` (64-bit/g++5.3.0/MinGW)

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
generic::delegate<int(int)>: 56
ssvu::FastFunc<int(int)>: 40
fu2::function<int(int)>: 48
fixed_size_function<int(int)>: 128

[function_pointer]
Perf< no_abstraction >: 0.3097261791 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.3481479514 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.4749179120 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.3596407551 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4085937307 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3796730815 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.4384196603 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4661153492 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4108554915 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.4526590249 [s] {checksum: 0}

[compile_time_function_pointer]
Perf< no_abstraction >: 0.0808029059 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.2908244131 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3351834673 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.3552733937 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.3171137015 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3285364251 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3148621808 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.3863633236 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.3963418868 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.4241716557 [s] {checksum: 0}

[compile_time_delegate]
Perf< no_abstraction >: 0.2200538304 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.3107501794 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3433393099 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.4407835011 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4477111833 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.4012967684 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3358036275 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4259671763 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4842375950 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.5439505741 [s] {checksum: 0}

[lambda]
Perf< stdex::function<int(int)> >: 0.2879140121 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3474465112 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.3415665093 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.3167393014 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3431556298 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3105994594 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.3951735665 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4066385301 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.4255831762 [s] {checksum: 0}

[lambda_capture]
Perf< stdex::function<int(int)> >: 0.2971885751 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3859566035 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.3332081066 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4276551768 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3382449082 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3208087427 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4915601573 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4220785351 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.4740663917 [s] {checksum: 0}

[heavy_functor]
Perf< stdex::function<int(int)> >: 0.3288033052 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3807524418 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.3348333871 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.3301335456 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3301117856 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3194522622 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.3797671615 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4055642898 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.4199678144 [s] {checksum: 0}

[non_assignable]
Perf< stdex::function<int(int)> >: 0.3443457102 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3453594705 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.4353002993 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4613886276 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.4041297293 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3529303529 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.5186436860 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.4996455999 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.5608420995 [s] {checksum: 0}
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
