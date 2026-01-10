// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from stm32_nucleo_f303re_driver:msg/WheelPwm.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "stm32_nucleo_f303re_driver/msg/detail/wheel_pwm__rosidl_typesupport_introspection_c.h"
#include "stm32_nucleo_f303re_driver/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "stm32_nucleo_f303re_driver/msg/detail/wheel_pwm__functions.h"
#include "stm32_nucleo_f303re_driver/msg/detail/wheel_pwm__struct.h"


// Include directives for member types
// Member `header`
#include "std_msgs/msg/header.h"
// Member `header`
#include "std_msgs/msg/detail/header__rosidl_typesupport_introspection_c.h"
// Member `pwm`
#include "rosidl_runtime_c/primitives_sequence_functions.h"

#ifdef __cplusplus
extern "C"
{
#endif

void stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__WheelPwm_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  stm32_nucleo_f303re_driver__msg__WheelPwm__init(message_memory);
}

void stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__WheelPwm_fini_function(void * message_memory)
{
  stm32_nucleo_f303re_driver__msg__WheelPwm__fini(message_memory);
}

size_t stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__size_function__WheelPwm__pwm(
  const void * untyped_member)
{
  const rosidl_runtime_c__int32__Sequence * member =
    (const rosidl_runtime_c__int32__Sequence *)(untyped_member);
  return member->size;
}

const void * stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__get_const_function__WheelPwm__pwm(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__int32__Sequence * member =
    (const rosidl_runtime_c__int32__Sequence *)(untyped_member);
  return &member->data[index];
}

void * stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__get_function__WheelPwm__pwm(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__int32__Sequence * member =
    (rosidl_runtime_c__int32__Sequence *)(untyped_member);
  return &member->data[index];
}

void stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__fetch_function__WheelPwm__pwm(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const int32_t * item =
    ((const int32_t *)
    stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__get_const_function__WheelPwm__pwm(untyped_member, index));
  int32_t * value =
    (int32_t *)(untyped_value);
  *value = *item;
}

void stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__assign_function__WheelPwm__pwm(
  void * untyped_member, size_t index, const void * untyped_value)
{
  int32_t * item =
    ((int32_t *)
    stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__get_function__WheelPwm__pwm(untyped_member, index));
  const int32_t * value =
    (const int32_t *)(untyped_value);
  *item = *value;
}

bool stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__resize_function__WheelPwm__pwm(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__int32__Sequence * member =
    (rosidl_runtime_c__int32__Sequence *)(untyped_member);
  rosidl_runtime_c__int32__Sequence__fini(member);
  return rosidl_runtime_c__int32__Sequence__init(member, size);
}

static rosidl_typesupport_introspection_c__MessageMember stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__WheelPwm_message_member_array[2] = {
  {
    "header",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(stm32_nucleo_f303re_driver__msg__WheelPwm, header),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "pwm",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_INT32,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(stm32_nucleo_f303re_driver__msg__WheelPwm, pwm),  // bytes offset in struct
    NULL,  // default value
    stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__size_function__WheelPwm__pwm,  // size() function pointer
    stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__get_const_function__WheelPwm__pwm,  // get_const(index) function pointer
    stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__get_function__WheelPwm__pwm,  // get(index) function pointer
    stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__fetch_function__WheelPwm__pwm,  // fetch(index, &value) function pointer
    stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__assign_function__WheelPwm__pwm,  // assign(index, value) function pointer
    stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__resize_function__WheelPwm__pwm  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__WheelPwm_message_members = {
  "stm32_nucleo_f303re_driver__msg",  // message namespace
  "WheelPwm",  // message name
  2,  // number of fields
  sizeof(stm32_nucleo_f303re_driver__msg__WheelPwm),
  stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__WheelPwm_message_member_array,  // message members
  stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__WheelPwm_init_function,  // function to initialize message memory (memory has to be allocated)
  stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__WheelPwm_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__WheelPwm_message_type_support_handle = {
  0,
  &stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__WheelPwm_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_stm32_nucleo_f303re_driver
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, stm32_nucleo_f303re_driver, msg, WheelPwm)() {
  stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__WheelPwm_message_member_array[0].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, std_msgs, msg, Header)();
  if (!stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__WheelPwm_message_type_support_handle.typesupport_identifier) {
    stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__WheelPwm_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &stm32_nucleo_f303re_driver__msg__WheelPwm__rosidl_typesupport_introspection_c__WheelPwm_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
