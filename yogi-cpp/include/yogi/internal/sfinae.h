#ifndef YOGI_INTERNAL_SFINAE_H
#define YOGI_INTERNAL_SFINAE_H

#include <type_traits>
#include <string>

#define YOGI_DEFINE_SFINAE_METHOD_TESTER(class_name, test_expression)     \
  namespace internal {                                                    \
  template <typename T>                                                   \
  struct class_name {                                                     \
   private:                                                               \
    template <typename U>                                                 \
    static auto test(int)                                                 \
        -> decltype(std::declval<U>() test_expression, std::true_type()); \
                                                                          \
    template <typename>                                                   \
    static std::false_type test(...);                                     \
                                                                          \
   public:                                                                \
    static constexpr bool value =                                         \
        std::is_same<decltype(test<T>(0)), std::true_type>::value;        \
  };                                                                      \
  }  // namespace internal

#endif  // YOGI_INTERNAL_SFINAE_H
