#pragma once

#include <type_traits>

namespace principia {
namespace base {
namespace internal_traits {

template<typename, typename = void, typename = void>
struct has_sfinae_read_from_message : std::false_type {};

template<typename T>
struct has_sfinae_read_from_message<
    T, std::void_t<decltype(&T::template ReadFromMessage<>)>>
    : std::true_type {};

template<typename, typename = void, typename = void>
struct has_unconditional_read_from_message : std::false_type {};

template<typename T>
struct has_unconditional_read_from_message<
    T, std::void_t<decltype(&T::ReadFromMessage)>>
    : std::true_type {};

}  // namespace internal_traits

// True if and only if T has a (possibly templated) static member function named
// ReadFromMessage.
template<typename T>
inline constexpr bool is_serializable_v =
    internal_traits::has_sfinae_read_from_message<T>::value ||
    internal_traits::has_unconditional_read_from_message<T>::value;

}  // namespace base
}  // namespace principia
