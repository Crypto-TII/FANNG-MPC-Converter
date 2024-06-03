#include "crypt_util.h"
auto prg = generate_prg(seed);
vector<uint8_t> bytes(1'000);
prg->GenerateBlock(bytes, bytes.size());