#ifndef __CRYPT_UTIL_H__
#define __CRYPT_UTIL_H__

#include "AES_RNG.h"
#include <memory>
#include <string>
#include <vector>

extern const int field_size; // = 16 : 128 bits = 16 bytes
extern const int seed_size; // = 16 : 128 bits = 16 bytes

/// @brief generate a random seed
/// @return array of bytes wrapped in a SecByteBlock
CryptoPP::SecByteBlock generate_seed();

/// @brief assign a seed from a vector of bytes
/// @param bytes a vector of bytes
/// @return a SecByteBlock containing the bytes of the seed
CryptoPP::SecByteBlock assign_seed(std::vector<uint8_t>& bytes);

/// @brief convert a seed to a string
/// @param seed an array of bytes wrapped in a SecByteBlock
/// @return a decimal string representation of the seed
std::string seed2string(const CryptoPP::SecByteBlock& seed);

/// @brief convert a string to a seed
/// @param s_seed a decimal string representation of a seed
/// @return an array of bytes wrapped in a SecByteBlock
CryptoPP::SecByteBlock string2seed(std::string_view s_seed);

/// @brief generate a deterministic (based on AES) random number generator
/// @param seed an array of bytes wrapped in a SecByteBlock
/// @return the RNG object
std::shared_ptr<AES_RNG> generate_prg(const CryptoPP::SecByteBlock& seed);

/// @brief convert 128-bit integer from network/big-endian to host endianess
/// @param val 128-bit integer
/// @return val in host endianess
__int128 ntoh128(__int128 val);

std::string read_keycert(const std::string& filepath);

int get_material_size(const std::string& material_type, const int material_num);
std::vector<std::vector<int>> get_NN_triple_dimensions(int w, int h, int s, int kh, int kw, int s_, int stride, int padding);

#endif  //__CRYPT_UTIL_H__