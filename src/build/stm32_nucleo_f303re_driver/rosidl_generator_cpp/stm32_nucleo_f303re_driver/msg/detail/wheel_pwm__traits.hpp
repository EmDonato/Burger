// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from stm32_nucleo_f303re_driver:msg/WheelPwm.idl
// generated code does not contain a copyright notice

#ifndef STM32_NUCLEO_F303RE_DRIVER__MSG__DETAIL__WHEEL_PWM__TRAITS_HPP_
#define STM32_NUCLEO_F303RE_DRIVER__MSG__DETAIL__WHEEL_PWM__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "stm32_nucleo_f303re_driver/msg/detail/wheel_pwm__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__traits.hpp"

namespace stm32_nucleo_f303re_driver
{

namespace msg
{

inline void to_flow_style_yaml(
  const WheelPwm & msg,
  std::ostream & out)
{
  out << "{";
  // member: header
  {
    out << "header: ";
    to_flow_style_yaml(msg.header, out);
    out << ", ";
  }

  // member: pwm
  {
    if (msg.pwm.size() == 0) {
      out << "pwm: []";
    } else {
      out << "pwm: [";
      size_t pending_items = msg.pwm.size();
      for (auto item : msg.pwm) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const WheelPwm & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: header
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "header:\n";
    to_block_style_yaml(msg.header, out, indentation + 2);
  }

  // member: pwm
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.pwm.size() == 0) {
      out << "pwm: []\n";
    } else {
      out << "pwm:\n";
      for (auto item : msg.pwm) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const WheelPwm & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace stm32_nucleo_f303re_driver

namespace rosidl_generator_traits
{

[[deprecated("use stm32_nucleo_f303re_driver::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const stm32_nucleo_f303re_driver::msg::WheelPwm & msg,
  std::ostream & out, size_t indentation = 0)
{
  stm32_nucleo_f303re_driver::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use stm32_nucleo_f303re_driver::msg::to_yaml() instead")]]
inline std::string to_yaml(const stm32_nucleo_f303re_driver::msg::WheelPwm & msg)
{
  return stm32_nucleo_f303re_driver::msg::to_yaml(msg);
}

template<>
inline const char * data_type<stm32_nucleo_f303re_driver::msg::WheelPwm>()
{
  return "stm32_nucleo_f303re_driver::msg::WheelPwm";
}

template<>
inline const char * name<stm32_nucleo_f303re_driver::msg::WheelPwm>()
{
  return "stm32_nucleo_f303re_driver/msg/WheelPwm";
}

template<>
struct has_fixed_size<stm32_nucleo_f303re_driver::msg::WheelPwm>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<stm32_nucleo_f303re_driver::msg::WheelPwm>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<stm32_nucleo_f303re_driver::msg::WheelPwm>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // STM32_NUCLEO_F303RE_DRIVER__MSG__DETAIL__WHEEL_PWM__TRAITS_HPP_
