#ifndef __AES_RNG_H__
#define __AES_RNG_H__

#include <cryptopp/cryptlib.h>
#include <cryptopp/config_int.h>
#include <cryptopp/secblock.h>
#include <cryptopp/smartptr.h>
#include <cryptopp/osrng.h>
#include <cryptopp/aes.h>
#include <cryptopp/sha.h>

/// @brief AES Random Number Generator - taken from Crypto++ library WIKI
/// @details This class is a deterministic random number generator based on AES
/// @snippet doc/snippets/aes_rng.cpp AES_RNG
/// @see [CryptoPP::AES_RNG](https://www.cryptopp.com/wiki/RandomNumberGenerator#AES_RNG)
class AES_RNG : public CryptoPP::RandomNumberGenerator, public CryptoPP::NotCopyable {
public:
  explicit AES_RNG(const CryptoPP::byte *seed = nullptr, size_t length = 0)
      : m_keyed(EntropyHelper(seed, length)) {}

  bool CanIncorporateEntropy() const override { return true; }

  void IncorporateEntropy(const CryptoPP::byte *input, size_t length) override {
    m_keyed = EntropyHelper(input, length, false);
  }

  void GenerateIntoBufferedTransformation(CryptoPP::BufferedTransformation &target,
                                          const std::string &channel,
                                          CryptoPP::lword size) override {
    if (!m_keyed) {
      m_pCipher->SetKey(m_key, m_key.size());
      m_keyed = true;
    }

    while (size > 0) {
      m_pCipher->ProcessBlock(m_seed);
      size_t len = std::min((size_t)16, size);
      target.ChannelPut(channel, m_seed, len);
      size -= len;
    }
  }

protected:
  // Sets up to use the cipher. It's a helper to allow a throw
  //   in the contructor during initialization.  Returns true
  //   if the cipher was keyed, and false if it was not.
  bool EntropyHelper(const CryptoPP::byte *input, size_t length, bool ctor = true) {
    if (ctor) {
      memset(m_key, 0x00, m_key.size());
      memset(m_seed, 0x00, m_seed.size());
    }

    CryptoPP::AlignedSecByteBlock seed(32 + 16);
    CryptoPP::SHA512 hash;

    if (input && length) {
      // Use the user supplied seed.
      hash.Update(input, length);
    } else {
      // No seed or size. Use the OS to gather entropy.
      CryptoPP::OS_GenerateRandomBlock(false, seed, seed.size());
      hash.Update(seed, seed.size());
    }

    hash.Update(m_key.data(), m_key.size());
    hash.TruncatedFinal(seed.data(), seed.size());

    memcpy(m_key.data(), seed.data() + 0, 32);
    memcpy(m_seed.data(), seed.data() + 32, 16);

    // Return false. This allows the constructor to complete
    //   initialization before the pointer m_pCipher is used.
    return false;
  }

private:
  CryptoPP::FixedSizeSecBlock<CryptoPP::byte, 32> m_key;
  CryptoPP::FixedSizeSecBlock<CryptoPP::byte, 16> m_seed;
  CryptoPP::member_ptr<CryptoPP::BlockCipher> m_pCipher{ new CryptoPP::AES::Encryption };      
  bool m_keyed;
};

#endif // __AES_RNG_H__