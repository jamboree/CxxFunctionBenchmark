C++ Function Benchmark
======================

Benchmark for various C++ function implementations.

Currently include:
- stdex::function - A multi-signature function implementation authored by me :)
- multifunction - Example from [Boost.TypeErasure](http://www.boost.org/doc/html/boost_typeerasure/examples.html#boost_typeerasure.examples.multifunction), another multi-signature function.
- std::function - [Standard](http://en.cppreference.com/w/cpp/utility/functional/function).
- boost::function - The one from [Boost](http://www.boost.org/doc/libs/1_55_0/doc/html/function.html).
- func::function - From this [blog](http://probablydance.com/2013/01/13/a-faster-implementation-of-stdfunction/).
- generic::delegate - [Fast delegate in C++11](http://codereview.stackexchange.com/questions/14730/impossibly-fast-delegate-in-c11), also see [here](https://code.google.com/p/cpppractice/source/browse/trunk/).
- ~~ssvu::FastFunc~~ - Another Don Clugston's FastDelegate, as shown [here](https://groups.google.com/a/isocpp.org/forum/#!topic/std-discussion/QgvHF7YMi3o).

### Sample Result
Compiled with `g++ -O3 -std=c++11` (x64)
```
[function_pointer]
Perf< stdex::function<int(int)> >: 0.2921513600 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.4046780800 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.3741452800 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3940646400 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3347593600 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4309753600 [s] {checksum: 0}

[lambda]
Perf< stdex::function<int(int)> >: 2.5157312000 [s] {checksum: 0}
Perf< std::function<int(int)> >: 3.1139782400 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 3.1819388800 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 3.4007993600 [s] {checksum: 0}
Perf< func::function<int(int)> >: 2.9085382400 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 3.8033596800 [s] {checksum: 0}

[lambda_capture]
Perf< stdex::function<int(int)> >: 0.3101558400 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.3907011200 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4157737600 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3491366400 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3293046400 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.4413420800 [s] {checksum: 0}

[heavy_functor]
Perf< stdex::function<int(int)> >: 3.0200185600 [s] {checksum: 0}
Perf< std::function<int(int)> >: 3.1890566400 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 3.0931574400 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 3.3808544000 [s] {checksum: 0}
Perf< func::function<int(int)> >: 2.9002272000 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 3.8943337600 [s] {checksum: 0}

[non_assignable]
Perf< stdex::function<int(int)> >: 0.3474915200 [s] {checksum: 0}
Perf< std::function<int(int)> >: 0.4333593600 [s] {checksum: 0}
Perf< multifunction<int(int)> >: 0.4632025600 [s] {checksum: 0}
Perf< boost::function<int(int)> >: 0.3769683200 [s] {checksum: 0}
Perf< func::function<int(int)> >: 0.3429977600 [s] {checksum: 0}
Perf< generic::delegate<int(int)> >: 0.5359315200 [s] {checksum: 0}
```
