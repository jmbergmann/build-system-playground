#pragma once

#define YOGI_DEFINE_FLAG_OPERATORS(enum_type)                                 \
  inline enum_type operator|(enum_type a, enum_type b) {                      \
    return static_cast<enum_type>(static_cast<int>(a) | static_cast<int>(b)); \
  }                                                                           \
  inline enum_type operator&(enum_type a, enum_type b) {                      \
    return static_cast<enum_type>(static_cast<int>(a) & static_cast<int>(b)); \
  }                                                                           \
  inline enum_type& operator|=(enum_type& a, enum_type b) { return a = a | b; }
