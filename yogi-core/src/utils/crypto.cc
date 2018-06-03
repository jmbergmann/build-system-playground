#include "crypto.h"
#include "../objects/logger.h"

#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <random>
#include <limits>

namespace utils {

const auto logger = objects::Logger::CreateStaticInternalLogger("Utils.Crypto");

ByteVector MakeSha256(const ByteVector& data) {
  ByteVector hash(SHA256_DIGEST_LENGTH);

  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, data.data(), static_cast<int>(data.size()));
  SHA256_Final(hash.data(), &sha256);

  return hash;
}

ByteVector GenerateRandomBytes(std::size_t n) {
  ByteVector bytes(n);

  auto res = RAND_bytes(bytes.data(), static_cast<int>(bytes.size()));
  if (res != 1) {
    char str[128];
    ERR_error_string_n(ERR_get_error(), str, sizeof(str));
    YOGI_LOG_WARNING(
        logger,
        "Could not generate "
            << n
            << " random bytes using OpenSSL. Generated from C++ STL instead.");

    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_int_distribution<> dist;
    for (auto& byte : bytes) {
      byte = static_cast<Byte>(dist(gen) % 256);
    }
  }

  return bytes;
}

}  // namespace utils
