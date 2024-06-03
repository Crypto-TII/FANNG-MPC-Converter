#include "crypt_util.h"
#include <cryptopp/hex.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <cassert>
#include <cstdint>
#include <endian.h>
#include <iomanip>

using std::string;
using std::vector;
using std::shared_ptr;

using CryptoPP::SecByteBlock;

const int field_size = 16;  // 128 bits = 16 bytes

const int seed_size = 32;  // 256 bits = 32 bytes

SecByteBlock generate_seed() {
    SecByteBlock seed(seed_size);
    CryptoPP::OS_GenerateRandomBlock(false, seed, seed.size());
    return seed;
}

SecByteBlock assign_seed(vector<uint8_t>& bytes) {
    assert(bytes.size() == seed_size);
    SecByteBlock seed(seed_size);
    for (int i = 0; i < seed_size; i++) {
        seed[i] = bytes[i];
    }
    return seed;    
}

string seed2string(const SecByteBlock& seed) {
    string s;
    CryptoPP::HexEncoder hex(new CryptoPP::StringSink(s));
    hex.Put(seed, seed.size());
    hex.MessageEnd();
    return s;
}

SecByteBlock string2seed(std::string_view s_seed) {
    SecByteBlock seed(seed_size);
    CryptoPP::HexDecoder decoder;
    decoder.Put((const CryptoPP::byte*)s_seed.data(), s_seed.length());
    decoder.MessageEnd();
    decoder.Get(seed, seed.size());
    return seed;
}

shared_ptr<AES_RNG> generate_prg(const SecByteBlock& seed) {
  auto prg_ptr = std::make_shared<AES_RNG>(seed, seed.size());
  return prg_ptr;
}

const bool is_big_endian = htobe16(1) == 1;

__int128 ntoh128(__int128 val) {
  if (is_big_endian) {
    return val;
  }
  return be64toh(val >> 64) | (__int128)be64toh(val) << 64;
}


string read_keycert(const std::string& filepath) {
  std::ifstream file(filepath);
  
  if (!file.is_open()) {
      std::cerr << "Error opening file: " << filepath << std::endl;
      return "";
  }

  std::stringstream buffer;
  buffer << file.rdbuf();

  return buffer.str();
}

std::vector<std::vector<int>> get_NN_triple_dimensions(int w, int h, int s, int kh, int kw, int s_, int stride, int padding) {
    int w_in = w + (padding * 2);
    int h_in = h + (padding * 2);
    int w_out = w_in - kw + 1;
    int h_out = h_in - kh + 1;
    if (stride == 2) {
        int w_out_p = w_out / stride;
        int h_out_p = h_out / stride;
        if ((w_in % stride) > 0) {
            w_out_p += 1;
        }
        if ((h_in % stride) > 0) {
            h_out_p += 1;
        }
        w_out = w_out_p;
        h_out = h_out_p;
    }
    std::vector<int> A = {w_in * h_in, s};
    std::vector<int> B = {kh * kw, s * s_};
    std::vector<int> C = {w_out * h_out, s_};
    return {A, B, C};
}

int get_material_size(const std::string& material_type, const int material_num) {
    if (material_type == "TRIPLE_BEAVER" || material_type == "BEAVER") {
        return 3*material_num;
    } else if (material_type.substr(0, 13) == "TRIPLE_MATRIX") {
        int a;
        int b;
        int c;
        sscanf(material_type.c_str(), "TRIPLE_MATRIX_%d_%d_%d", &a, &b, &c);
        return (a * b + b * c + a * c)*material_num;
    } else if (material_type.substr(0, 7) == "LENET11") {
        int a;
        int b;
        int c;
        int d;
        int e;
        int f;
        sscanf(material_type.c_str(), "LENET11_%d_%d_%d_%d_%d_%d", &a, &b, &c, &d, &e, &f);
        return (a * b + c * d + e * f)*material_num;
    } else if (material_type.substr(0, 12) == "PRUNEDRESNET") {
        int w;
        int h;
        int s;
        int kh;
        int kw;
        int s_; 
        int stride;
        int padding;
        sscanf(material_type.c_str(), "PRUNEDRESNET_%d_%d_%d_%d_%d_%d_%d_%d", &w, &h, &s, &kh, &kw, &s_, &stride, &padding);
        vector<vector<int>> matrix = get_NN_triple_dimensions(w, h, s, kh, kw, s_, stride, padding);
        return (matrix[0][0]*matrix[0][1] + matrix[1][0]*matrix[1][1] + matrix[2][0]*matrix[2][1])*material_num;
    } else {
        return 1*material_num;
    }
}
