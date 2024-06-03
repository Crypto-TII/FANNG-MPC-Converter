#include <catch2/catch.hpp>

#include "../src/crypt_util.h"
#include <iomanip>
#include <string>
#include <vector>
#include <cstdint>

TEST_CASE("Test crypt_util seed_size") {
    auto seed = generate_seed();
    REQUIRE(seed.size() == seed_size);
}

TEST_CASE("Test crypt_util seed_str_size") {
    auto seed = generate_seed();
    auto s_seed = seed2string(seed);
    REQUIRE(s_seed.length() == seed_size * 2);
}

std::string bytes2hex(const uint8_t* bytes, size_t size) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < size; i++) {
        ss << std::setw(2) << static_cast<unsigned>(bytes[i]);
    }
    return ss.str();
}

TEST_CASE("Test crypt_util seed_str_conversion") {
    auto seed = generate_seed();
    INFO("seed:     " << bytes2hex(seed.data(), seed.size()))
    auto s_seed = seed2string(seed);
    INFO("seed_str: " << s_seed)
    auto seed2 = string2seed(s_seed);
    INFO("seed2:    " << bytes2hex(seed2.data(), seed2.size()))

    REQUIRE(seed == seed2);
}

TEST_CASE("Test crypt_util prg_deterministic") {
    const int block_size = 30;
    auto seed = generate_seed();
    auto prg1 = generate_prg(seed);
    std::vector<uint8_t> bytes1(block_size);
    prg1->GenerateBlock(bytes1.data(), bytes1.size());
    INFO("bytes1: " << bytes2hex(bytes1.data(), bytes1.size()))
    auto prg2 = generate_prg(seed);
    std::vector<uint8_t> bytes2(block_size);
    prg2->GenerateBlock(bytes2.data(), bytes2.size());
    INFO("bytes2: " << bytes2hex(bytes2.data(), bytes2.size()))
    bool same = true;
    for (int i = 0; i < block_size; i++) {
        if (bytes1[i] != bytes2[i]) {
            same = false;
            break;
        }
    }
    REQUIRE(same);
}

TEST_CASE("Test crypt_util prg_random_data") {
    const int block_size = 30;
    auto seed = generate_seed();
    auto prg1 = generate_prg(seed);
    std::vector<uint8_t> bytes1(block_size);
    prg1->GenerateBlock(bytes1.data(), bytes1.size());
    INFO("bytes1: " << bytes2hex(bytes1.data(), bytes1.size()))
    std::vector<uint8_t> bytes2(block_size);
    prg1->GenerateBlock(bytes2.data(), bytes2.size());
    INFO("bytes2: " << bytes2hex(bytes2.data(), bytes2.size()))
    bool same = true;
    for (int i = 0; i < block_size; i++) {
        if (bytes1[i] != bytes2[i]) {
            same = false;
            break;
        }
    }
    REQUIRE(!same);
}
