#include "../config.h"
#include "timestamp.h"

#include <boost/uuid/uuid.hpp>
#include <boost/endian/arithmetic.hpp>

#include <string>
#include <vector>
#include <chrono>
#include <algorithm>

namespace utils {
namespace detail {

template <typename BigEndianType, typename T>
inline void SerializeInteger(std::vector<char>* buffer, const T& val) {
  BigEndianType big = val;
  buffer->resize(buffer->size() + sizeof(big));
  std::memcpy(buffer->data() + buffer->size() - sizeof(big), &big, sizeof(big));
}

template <typename BigEndianType, typename T>
inline bool DeserializeInteger(T* val, const std::vector<char>& buffer,
                               std::vector<char>::const_iterator* it) {
  BigEndianType big;
  if (std::distance(*it, buffer.end()) < sizeof(big)) {
    return false;
  }

  std::copy(*it, *it + sizeof(big), reinterpret_cast<char*>(&big));
  *it += sizeof(big);
  *val = big;
  return true;
}

}  // namespace detail

template <typename T>
inline void Serialize(std::vector<char>* buffer, const T& val) {
  static_assert(false, "Missing specialization");
}

template <typename T>
bool Deserialize(T* val, const std::vector<char>& buffer,
                 std::vector<char>::const_iterator* it) {
  static_assert(false, "Missing specialization");
  return false;
}

// unsigned short
template <>
inline void Serialize<unsigned short>(std::vector<char>* buffer,
                                      const unsigned short& val) {
  detail::SerializeInteger<boost::endian::big_uint16_t>(buffer, val);
}

template <>
inline bool Deserialize<unsigned short>(unsigned short* val,
                                        const std::vector<char>& buffer,
                                        std::vector<char>::const_iterator* it) {
  return detail::DeserializeInteger<boost::endian::big_uint16_t>(val, buffer,
                                                                 it);
}

// int
template <>
inline void Serialize<int>(std::vector<char>* buffer, const int& val) {
  detail::SerializeInteger<boost::endian::big_int32_t>(buffer, val);
}

template <>
inline bool Deserialize<int>(int* val, const std::vector<char>& buffer,
                             std::vector<char>::const_iterator* it) {
  return detail::DeserializeInteger<boost::endian::big_int32_t>(val, buffer,
                                                                it);
}

// size_t
template <>
inline void Serialize<std::size_t>(std::vector<char>* buffer,
                                   const std::size_t& val) {
  detail::SerializeInteger<boost::endian::big_uint32_t>(buffer, val);
}

template <>
inline bool Deserialize<std::size_t>(std::size_t* val,
                                     const std::vector<char>& buffer,
                                     std::vector<char>::const_iterator* it) {
  return detail::DeserializeInteger<boost::endian::big_uint32_t>(val, buffer,
                                                                 it);
}

// std::chrono::nanoseconds
template <>
inline void Serialize<std::chrono::nanoseconds>(
    std::vector<char>* buffer, const std::chrono::nanoseconds& dur) {
  detail::SerializeInteger<boost::endian::big_int64_t,
                           std::chrono::nanoseconds::rep>(buffer, dur.count());
}

template <>
inline bool Deserialize<std::chrono::nanoseconds>(
    std::chrono::nanoseconds* dur, const std::vector<char>& buffer,
    std::vector<char>::const_iterator* it) {
  std::chrono::nanoseconds::rep n;
  if (!detail::DeserializeInteger<boost::endian::big_int64_t>(&n, buffer, it)) {
    return false;
  }

  *dur = std::chrono::nanoseconds(n);
  return true;
}

// Timestamp
template <>
inline void Serialize<Timestamp>(std::vector<char>* buffer,
                                 const Timestamp& time) {
  Serialize(buffer, time.NanosecondsSinceEpoch());
}

template <>
inline bool Deserialize<Timestamp>(Timestamp* time,
                                   const std::vector<char>& buffer,
                                   std::vector<char>::const_iterator* it) {
  std::chrono::nanoseconds n;
  if (!Deserialize<std::chrono::nanoseconds>(&n, buffer, it)) {
    return false;
  }

  *time = Timestamp(n);
  return true;
}

// std::string
template <>
inline void Serialize<std::string>(std::vector<char>* buffer,
                                   const std::string& str) {
  buffer->insert(buffer->end(), str.c_str(), str.c_str() + str.size() + 1);
}

template <>
inline bool Deserialize<std::string>(std::string* str,
                                     const std::vector<char>& buffer,
                                     std::vector<char>::const_iterator* it) {
  auto end = std::find(*it, buffer.cend(), '\0');
  if (end == buffer.end()) {
    return false;
  }

  *str = std::string(*it, end);
  *it = end + 1;
  return true;
}

// boost::uuids::uuid
template <>
inline void Serialize<boost::uuids::uuid>(std::vector<char>* buffer,
                                          const boost::uuids::uuid& uuid) {
  buffer->insert(buffer->end(), uuid.begin(), uuid.end());
}

template <>
inline bool Deserialize<boost::uuids::uuid>(
    boost::uuids::uuid* uuid, const std::vector<char>& buffer,
    std::vector<char>::const_iterator* it) {
  if (std::distance(*it, buffer.cend()) < sizeof(*uuid)) {
    return false;
  }

  std::copy_n(*it, sizeof(*uuid), uuid->begin());
  *it += sizeof(*uuid);
  return true;
}

}  // namespace utils
