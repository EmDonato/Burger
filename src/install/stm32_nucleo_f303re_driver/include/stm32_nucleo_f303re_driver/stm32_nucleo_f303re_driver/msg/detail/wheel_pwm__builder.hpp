// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from stm32_nucleo_f303re_driver:msg/WheelPwm.idl
// generated code does not contain a copyright notice

#ifndef STM32_NUCLEO_F303RE_DRIVER__MSG__DETAIL__WHEEL_PWM__BUILDER_HPP_
#define STM32_NUCLEO_F303RE_DRIVER__MSG__DETAIL__WHEEL_PWM__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "stm32_nucleo_f303re_driver/msg/detail/wheel_pwm__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace stm32_nucleo_f303re_driver
{

namespace msg
{

namespace builder
{

class Init_WheelPwm_pwm
{
public:
  explicit Init_WheelPwm_pwm(::stm32_nucleo_f303re_driver::msg::WheelPwm & msg)
  : msg_(msg)
  {}
  ::stm32_nucleo_f303re_driver::msg::WheelPwm pwm(::stm32_nucleo_f303re_driver::msg::WheelPwm::_pwm_type arg)
  {
    msg_.pwm = std::move(arg);
    return std::move(msg_);
  }

private:
  ::stm32_nucleo_f303re_driver::msg::WheelPwm msg_;
};

class Init_WheelPwm_header
{
public:
  Init_WheelPwm_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_WheelPwm_pwm header(::stm32_nucleo_f303re_driver::msg::WheelPwm::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_WheelPwm_pwm(msg_);
  }

private:
  ::stm32_nucleo_f303re_driver::msg::WheelPwm msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::stm32_nucleo_f303re_driver::msg::WheelPwm>()
{
  return stm32_nucleo_f303re_driver::msg::builder::Init_WheelPwm_header();
}

}  // namespace stm32_nucleo_f303re_driver

#endif  // STM32_NUCLEO_F303RE_DRIVER__MSG__DETAIL__WHEEL_PWM__BUILDER_HPP_
