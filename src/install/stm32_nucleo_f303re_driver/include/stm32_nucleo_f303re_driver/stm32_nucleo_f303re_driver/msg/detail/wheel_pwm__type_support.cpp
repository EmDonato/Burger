// generated from rosidl_typesupport_introspection_cpp/resource/idl__type_support.cpp.em
// with input from stm32_nucleo_f303re_driver:msg/WheelPwm.idl
// generated code does not contain a copyright notice

#include "array"
#include "cstddef"
#include "string"
#include "vector"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_interface/macros.h"
#include "stm32_nucleo_f303re_driver/msg/detail/wheel_pwm__struct.hpp"
#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/message_type_support_decl.hpp"
#include "rosidl_typesupport_introspection_cpp/visibility_control.h"

namespace stm32_nucleo_f303re_driver
{

namespace msg
{

namespace rosidl_typesupport_introspection_cpp
{

void WheelPwm_init_function(
  void * message_memory, rosidl_runtime_cpp::MessageInitialization _init)
{
  new (message_memory) stm32_nucleo_f303re_driver::msg::WheelPwm(_init);
}

void WheelPwm_fini_function(void * message_memory)
{
  auto typed_message = static_cast<stm32_nucleo_f303re_driver::msg::WheelPwm *>(message_memory);
  typed_message->~WheelPwm();
}

size_t size_function__WheelPwm__pwm(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<int32_t> *>(untyped_member);
  return member->size();
}

const void * get_const_function__WheelPwm__pwm(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<int32_t> *>(untyped_member);
  return &member[index];
}

void * get_function__WheelPwm__pwm(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<int32_t> *>(untyped_member);
  return &member[index];
}

void fetch_function__WheelPwm__pwm(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const int32_t *>(
    get_const_function__WheelPwm__pwm(untyped_member, index));
  auto & value = *reinterpret_cast<int32_t *>(untyped_value);
  value = item;
}

void assign_function__WheelPwm__pwm(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<int32_t *>(
    get_function__WheelPwm__pwm(untyped_member, index));
  const auto & value = *reinterpret_cast<const int32_t *>(untyped_value);
  item = value;
}

void resize_function__WheelPwm__pwm(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<int32_t> *>(untyped_member);
  member->resize(size);
}

static const ::rosidl_typesupport_introspection_cpp::MessageMember WheelPwm_message_member_array[2] = {
  {
    "header",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    ::rosidl_typesupport_introspection_cpp::get_message_type_support_handle<std_msgs::msg::Header>(),  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(stm32_nucleo_f303re_driver::msg::WheelPwm, header),  // bytes offset in struct
    nullptr,  // default value
    nullptr,  // size() function pointer
    nullptr,  // get_const(index) function pointer
    nullptr,  // get(index) function pointer
    nullptr,  // fetch(index, &value) function pointer
    nullptr,  // assign(index, value) function pointer
    nullptr  // resize(index) function pointer
  },
  {
    "pwm",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(stm32_nucleo_f303re_driver::msg::WheelPwm, pwm),  // bytes offset in struct
    nullptr,  // default value
    size_function__WheelPwm__pwm,  // size() function pointer
    get_const_function__WheelPwm__pwm,  // get_const(index) function pointer
    get_function__WheelPwm__pwm,  // get(index) function pointer
    fetch_function__WheelPwm__pwm,  // fetch(index, &value) function pointer
    assign_function__WheelPwm__pwm,  // assign(index, value) function pointer
    resize_function__WheelPwm__pwm  // resize(index) function pointer
  }
};

static const ::rosidl_typesupport_introspection_cpp::MessageMembers WheelPwm_message_members = {
  "stm32_nucleo_f303re_driver::msg",  // message namespace
  "WheelPwm",  // message name
  2,  // number of fields
  sizeof(stm32_nucleo_f303re_driver::msg::WheelPwm),
  WheelPwm_message_member_array,  // message members
  WheelPwm_init_function,  // function to initialize message memory (memory has to be allocated)
  WheelPwm_fini_function  // function to terminate message instance (will not free memory)
};

static const rosidl_message_type_support_t WheelPwm_message_type_support_handle = {
  ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  &WheelPwm_message_members,
  get_message_typesupport_handle_function,
};

}  // namespace rosidl_typesupport_introspection_cpp

}  // namespace msg

}  // namespace stm32_nucleo_f303re_driver


namespace rosidl_typesupport_introspection_cpp
{

template<>
ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
get_message_type_support_handle<stm32_nucleo_f303re_driver::msg::WheelPwm>()
{
  return &::stm32_nucleo_f303re_driver::msg::rosidl_typesupport_introspection_cpp::WheelPwm_message_type_support_handle;
}

}  // namespace rosidl_typesupport_introspection_cpp

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, stm32_nucleo_f303re_driver, msg, WheelPwm)() {
  return &::stm32_nucleo_f303re_driver::msg::rosidl_typesupport_introspection_cpp::WheelPwm_message_type_support_handle;
}

#ifdef __cplusplus
}
#endif
