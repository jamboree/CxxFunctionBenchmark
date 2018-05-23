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
FastDelegate1: 24
ssvu::FastFunc<int(int)>: 40
fu2::function<int(int)>: 32
fixed_size_function<int(int)>: 128
gnr_forwarder: 64
embxx_util_StaticFunction: 64
Function_: 56

[function_pointer]
Perf< no_abstraction >: 0.1288982620 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 0.1802238570 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.2058876670 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 0.1803517340 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.1802829260 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.2058794960 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.1802563850 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.2060581670 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 0.1802537290 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 0.1853615050 [s] {checksum: 0}
Perf< gnr_forwarder >: 0.1803345000 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 0.1802848970 [s] {checksum: 0}
Perf< Function_ >:    0.1802979720 [s] {checksum: 0}

[compile_time_function_pointer]
Perf< no_abstraction >: 0.3934555440 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 1.5451997980 [s] {checksum: 0}
Perf< std::function<int(int)> >: 1.5459942540 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 1.5461171210 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 1.5450458270 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 1.5452900910 [s] {checksum: 0}
Perf< func::function<int(int)> >: 1.2891927270 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 1.3323708480 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 1.5452935910 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 1.4699039690 [s] {checksum: 0}
Perf< gnr_forwarder >: 1.3481359820 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 1.5468700840 [s] {checksum: 0}
Perf< Function_ >:    1.5467078300 [s] {checksum: 0}

[compile_time_delegate]
Perf< no_abstraction >: 0.6910022930 [s] {checksum: 0}
Perf< stdex::function<int(int)> >: 1.3061602740 [s] {checksum: 0}
Perf< std::function<int(int)> >: 1.5484581910 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 1.6332273820 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 1.5920799810 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 1.4077582930 [s] {checksum: 0}
Perf< func::function<int(int)> >: 1.3359535000 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 1.6271561910 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 1.5549385590 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 1.9666569350 [s] {checksum: 0}
Perf< gnr_forwarder >: 1.6086427330 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 1.5334078040 [s] {checksum: 0}
Perf< Function_ >:    1.6022175490 [s] {checksum: 0}

[heavy_functor]
Perf< stdex::function<int(int)> >: 1.2890043320 [s] {checksum: 0}
Perf< std::function<int(int)> >: 1.2936670380 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 1.3891772660 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 1.3025428230 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 1.2925984040 [s] {checksum: 0}
Perf< func::function<int(int)> >: 1.2891075260 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 1.3324250970 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 1.2900123970 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 1.5319183420 [s] {checksum: 0}
Perf< gnr_forwarder >: 1.5473598640 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 1.3502855220 [s] {checksum: 0}
Perf< Function_ >:    1.5471582460 [s] {checksum: 0}

[non_assignable]
Perf< stdex::function<int(int)> >: 1.5462126420 [s] {checksum: 0}
Perf< std::function<int(int)> >: 1.5755652510 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 1.6353873220 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 1.6095705330 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 1.4090110690 [s] {checksum: 0}
Perf< func::function<int(int)> >: 1.3380219750 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 1.8247514010 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 1.6780532600 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 1.9525207950 [s] {checksum: 0}
Perf< gnr_forwarder >: 1.6540586500 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 1.5273487910 [s] {checksum: 0}
Perf< Function_ >:    1.5972135040 [s] {checksum: 0}

[lambda_capture]
Perf< stdex::function<int(int)> >: 1.5460207890 [s] {checksum: 0}
Perf< std::function<int(int)> >: 1.5463801470 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 1.4500088330 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 1.5489294890 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 1.5449383350 [s] {checksum: 0}
Perf< func::function<int(int)> >: 1.2895905430 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 1.6830352960 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 1.5461697310 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 1.6821093720 [s] {checksum: 0}
Perf< gnr_forwarder >: 1.4132016220 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 1.3603302370 [s] {checksum: 0}
Perf< Function_ >:    1.3916453080 [s] {checksum: 0}

[lambda]
Perf< stdex::function<int(int)> >: 1.2889468560 [s] {checksum: 0}
Perf< std::function<int(int)> >: 1.5455471370 [s] {checksum: 0}
Perf< cxx_function::function<int(int)> >: 1.5464877670 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 1.2895574060 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 1.5452744220 [s] {checksum: 0}
Perf< func::function<int(int)> >: 1.2907299890 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 1.5466072320 [s] {checksum: 0}
Perf< fu2::function<int(int)> >: 1.5454115640 [s] {checksum: 0}
Perf< fixed_size_function<int(int)> >: 1.4694047830 [s] {checksum: 0}
Perf< gnr_forwarder >: 1.3485369340 [s] {checksum: 0}
Perf< embxx_util_StaticFunction >: 1.5466635040 [s] {checksum: 0}
Perf< Function_ >:    1.5461067630 [s] {checksum: 0}
```

#### [overload.cpp](overload.cpp)
This shows the timing of each multi-method technique.
```
with< no_abstraction >: 0.1754783920 [s] {checksum: 3}
with< stdex::function<Sig...> >: 3.6054651620 [s] {checksum: 3}
with< multifunction<Sig...> >: 4.0471627540 [s] {checksum: 3}
with< cxx_function::function<Sig...> >: 3.6831113920 [s] {checksum: 3}
with< fu2::function<Sig...> >: 3.5997253920 [s] {checksum: 3}
with< virtual_base& >: 1.3165000820 [s] {checksum: 3}
```
