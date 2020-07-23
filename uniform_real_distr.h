#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace nonstd {
namespace detail {
    template <class Int>
    constexpr int entropy(Int range) {
        static_assert(std::is_unsigned_v<Int>);
        
        bool const overflow = range + Int(1) == Int(0);
        if (!overflow)
            ++range;
        
        int result = 0;
        while (range >>= 1) ++result;
        
        return result + Int(overflow);
    }
} // namespace detail

template <class Real = double, bool right_inc = false>
class uniform_real_distribution
{
    static_assert(
        std::is_floating_point_v<Real>,
        "uniform_real_distribution requires a floating point type");
    
    static_assert(
        std::is_same_v<Real, float> ||
        std::is_same_v<Real, double>,
        "currently only float or double are supported");
        
public:
    using result_type = Real;
    struct param_type
    {
        using distribution_type = uniform_real_distribution<Real>;
        
        Real a = Real(0);
        Real b = Real(1);
        
        param_type() = default;
        param_type(Real a, Real b = Real(1)) : a(a), b(b) {
            assert(a <= b);
            assert(b - a <= std::numeric_limits<Real>::max());
        }
        
        bool operator==(param_type const & rhs) const {
            return a == rhs.a && b == rhs.b;
        }
    };
    
    uniform_real_distribution() = default;
    explicit uniform_real_distribution(Real a, Real b = Real(1)) : _params(a, b) {}
    explicit uniform_real_distribution(param_type const & params) : _params(params) {}
    
    void reset(){}
    
    template <class Gen>
    result_type operator()(Gen & gen){ return operator()(gen, _params); }
    
    template <class Gen>
    result_type operator()(Gen & gen, param_type const & params) {
        static_assert(gen.min() < gen.max(), "a rng must provide at least 1 bit of entropy");
        assert(params.a < params.b);
        
        constexpr int entropy = detail::entropy(gen.max() - gen.min());
        constexpr bool is_float_v = std::is_same_v<Real, float>;
        constexpr int desired = is_float_v ? 24 : 53;
        using rnd_t = std::conditional_t<is_float_v, std::uint32_t, std::uint64_t>;
        constexpr rnd_t mask = is_float_v ? 0xffffff : 0x1fffffffffffff;
        constexpr Real divisor = is_float_v ? 16777216.0 : 9007199254740992.0;
        
        rnd_t rnd = 0;
        for (int i = 0; i < desired; i += entropy){
            rnd |= static_cast<rnd_t>(gen() - gen.min()) << i;
        }
        
        return params.a + (params.b - params.a) * (((rnd & mask) + Real(right_inc)) / divisor);
    }
    
    result_type a() const { return _params.a; }
    result_type b() const { return _params.b; }
    param_type param() const { return _params; }
    void param(param_type const & params) { _params = params; }
    
    result_type min() const {
        if constexpr(std::is_same_v<Real, float>)
            return Real(right_inc) / 16777216.0f;
        else
            return Real(right_inc) / 9007199254740992.0;
    }
    result_type max() const{
        if constexpr(std::is_same_v<Real, float>)
            return (0xffffff + Real(right_inc)) / 16777216.0f;
        else
            return (0x1fffffffffffff + Real(right_inc)) / 9007199254740992.0;
    }

private:
    param_type _params;
};

template <class Real>
bool operator==(uniform_real_distribution<Real> const & lhs,
                uniform_real_distribution<Real> const & rhs ) {
    return lhs.param() == rhs.param();
}
template <class Real>
bool operator!=(uniform_real_distribution<Real> const & lhs,
                uniform_real_distribution<Real> const & rhs ) {
    return !(lhs.param() == rhs.param());
}
} // namespace nonstd
