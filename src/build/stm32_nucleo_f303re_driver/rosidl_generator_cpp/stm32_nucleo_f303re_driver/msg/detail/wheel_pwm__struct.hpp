// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from stm32_nucleo_f303re_driver:msg/WheelPwm.idl
// generated code does not contain a copyright notice

#ifndef STM32_NUCLEO_F303RE_DRIVER__MSG__DETAIL__WHEEL_PWM__STRUCT_HPP_
#define STM32_NUCLEO_F303RE_DRIVER__MSG__DETAIL__WHEEL_PWM__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__stm32_nucleo_f303re_driver__msg__WheelPwm __attribute__((deprecated))
#else
# define DEPRECATED__stm32_nucleo_f303re_driver__msg__WheelPwm __declspec(deprecated)
#endif

namespace stm32_nucleo_f303re_driver
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct WheelPwm_
{
  using Type = WheelPwm_<ContainerAllocator>;

  explicit WheelPwm_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_init)
  {
    (void)_init;
  }

  explicit WheelPwm_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_alloc, _init)
  {
    (void)_init;
  }

  // field types and members
  using _header_type =
    std_msgs::msg::Header_<ContainerAllocator>;
  _header_type header;
  using _pwm_type =
    std::vector<int32_t, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<int32_t>>;
  _pwm_type pwm;

  // setters for named parameter idiom
  Type & set__header(
    const std_msgs::msg::Header_<ContainerAllocator> & _arg)
  {
    this->header = _arg;
    return *this;
  }
  Type & set__pwm(
    const std::vector<int32_t, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<int32_t>> & _arg)
  {
    this->pwm = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    stm32_nucleo_f303re_driver::msg::WheelPwm_<ContainerAllocator> *;
  using ConstRawPtr =
    const stm32_nucleo_f303re_driver::msg::WheelPwm_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<stm32_nucleo_f303re_driver::msg::WheelPwm_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<stm32_nucleo_f303re_driver::msg::WheelPwm_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      stm32_nucleo_f303re_driver::msg::WheelPwm_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<stm32_nucleo_f303re_driver::msg::WheelPwm_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      stm32_nucleo_f303re_driver::msg::WheelPwm_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<stm32_nucleo_f303re_driver::msg::WheelPwm_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<stm32_nucleo_f303re_driver::msg::WheelPwm_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<stm32_nucleo_f303re_driver::msg::WheelPwm_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__stm32_nucleo_f303re_driver__msg__WheelPwm
    std::shared_ptr<stm32_nucleo_f303re_driver::msg::WheelPwm_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__stm32_nucleo_f303re_driver__msg__WheelPwm
    std::shared_ptr<stm32_nucleo_f303re_driver::msg::WheelPwm_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const WheelPwm_ & other) const
  {
    if (this->header != other.header) {
      return false;
    }
    if (this->pwm != other.pwm) {
      return false;
    }
    return true;
  }
  bool operator!=(const WheelPwm_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct WheelPwm_

// alias to use template instance with default allocator
using WheelPwm =
  stm32_nucleo_f303re_driver::msg::WheelPwm_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace stm32_nucleo_f303re_driver

#endif  // STM32_NUCLEO_F303RE_DRIVER__MSG__DETAIL__WHEEL_PWM__STRUCT_HPP_
