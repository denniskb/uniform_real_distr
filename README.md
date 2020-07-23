# uniform_real_distr
The standard suffers from a [defect](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0952r0.html) where it over-constraints `std::generate_canonical`. `std::generate_canonical` is used to generate (uniformly distributed) random numbers on the interval `[0, 1)` and is a core building block of many distributions such as `std::uniform_real_distribution`, `std::exponential_distribution`, etc. Currently, it is impossible to satisfy all constraints simultaneously, those are:

> - The result must lie in [0, 1).
> - The algorithm is specified exactly and the underlying URBG must be invoked a specific, fixed number of times for a given set of parameters.
> - The results must be uniformly distributed.

This forces standard library implementers to choose: They can either adopt the standard-dictated algorithm, not adhering to the mathematical constraints ([as done by Microsoft](https://github.com/microsoft/STL/issues/1074) for example), or they can deviate from the specified algorithm in favor of adhering to the mathematical constraints (as done by GCC for example). For more details on the issue please consult the aforementioned paper.

`nonstd::uniform_real_distribution` goes the "GCC route" and consciously deviates from the standard-prescribed algorithm in order to offer a mathematically sound uniform real distribution to Microsoft-based developers as a standalone header. It is a (close to) drop-in replacement with the following caveats:

- (De-)serialization from/to streams is not (yet) supported.
- It only supports (32bit) floats and (64bit) doubles (for now).

Other than that it offers the following features:

- Generates perfectly, uniformly distributed values strictly on the interval `[a, b)` for a=0.
- All values lie within `[min(), max()]` with `max() < b` for a=0.
- Supports all (standard-conforming) RNGs.
- Guarantees to invoke the RNG at most `ceil({24|53} / log2(rng.max()-rng.min()+1))` times for `{float|double}` respectively.
- Matches the performance of `std::uniform_real_distribution`.
- Allows to generate right-inclusive random numbers on the interval `(a, b]` (for a=0) by passing `true` as the `right_inc` template argument: `nonstd::uniform_real_distribution<float, true>` (default value is `false`).

`nonstd::uniform_real_distribution` uses the well-established technique in (real) random number generation of directly filling the matnissa with (uniform) random bits and simply discarding any potentially left-over entropy [as described by Vigna Sebastiano](http://prng.di.unimi.it/) (section "Generating uniform doubles in the unit interval"). 

This header was extracted from a fairly well-tested production-level codebase and is provided "bona fide" as a helpful tool to others. It does not aim to compete with/replace any other implementations. Use in high-stakes scenarios is not recommended and done at your own risk.

[Discussion on reddit](https://www.reddit.com/r/cpp/comments/hw4cdl/nonstduniform_real_distribution_a_dropin/)
