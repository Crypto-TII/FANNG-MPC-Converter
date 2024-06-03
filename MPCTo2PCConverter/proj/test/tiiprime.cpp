#include <catch2/catch.hpp>
#include "../src/tii_prime.h"

#include <iomanip>
#include <string>
#include <vector>
#include <cstdint>

const std::string __p_str = "170141183460469231731687303715887185921";
const std::string __halfP_str = "85070591730234615865843651857943592960";
const uint128_t __p = static_cast<uint128_t>(__p_str);
const int128_t __halfP = static_cast<int128_t>(__halfP_str);

TEST_CASE("Test TiiPrime prime_to_string") {
    auto p1 = TiiPrime(__halfP);
    std::string s1 = p1.to_string();
    INFO("s1 = " << s1)
    std::string s2 = __halfP_str;
    INFO("s2 = " << s2)
    REQUIRE(s1 == s2);
}


// numbers in TiiPrime are in the range [-halfP, halfP]
// so if we add halfP to halfP we get -1
TEST_CASE("Test TiiPrime prime_addition_with_overflow") {
    auto p1 = TiiPrime(__halfP);
    INFO("p1 = " << p1.to_string())
    auto p2 = TiiPrime(__halfP);
    INFO("p2 = " << p2.to_string())
    auto p3 = p1 + p2;
    INFO("p3 = " << p3.to_string())
    REQUIRE(p3 == TiiPrime(-1));
}

TEST_CASE("Test TiiPrime prime_addition_and_subtraction") {
    auto p1 = TiiPrime(__halfP);
    auto p2 = p1;
    INFO("initial value:     " << p1.to_string())
    for (int i = 0; i < 100; i++) {
        p2 = p2 + TiiPrime(i*i, true);
    }
    INFO("after addition:    " << p2.to_string())
    for (int i = 0; i < 100; i++) {
        p2 -= TiiPrime(i*i, true);
    }
    INFO("after subtraction: " << p2.to_string())
    REQUIRE(p2 == p1);
}
