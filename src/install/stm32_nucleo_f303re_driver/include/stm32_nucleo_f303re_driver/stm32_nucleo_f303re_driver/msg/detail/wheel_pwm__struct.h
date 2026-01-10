// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from stm32_nucleo_f303re_driver:msg/WheelPwm.idl
// generated code does not contain a copyright notice

#ifndef STM32_NUCLEO_F303RE_DRIVER__MSG__DETAIL__WHEEL_PWM__STRUCT_H_
#define STM32_NUCLEO_F303RE_DRIVER__MSG__DETAIL__WHEEL_PWM__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__struct.h"
// Member 'pwm'
#include "rosidl_runtime_c/primitives_sequence.h"

/// Struct defined in msg/WheelPwm in the package stm32_nucleo_f303re_driver.
/**
  * Time reference for logging and identification
 */
typedef struct stm32_nucleo_f303re_driver__msg__WheelPwm
{
  std_msgs__msg__Header header;
  /// PWM values applied to the wheels
  /// Convention:
  /// pwm[0] -> left wheel
  /// pwm[1] -> right wheel
  rosidl_runtime_c__int32__Sequence pwm;
} stm32_nucleo_f303re_driver__msg__WheelPwm;

// Struct for a sequence of stm32_nucleo_f303re_driver__msg__WheelPwm.
typedef struct stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence
{
  stm32_nucleo_f303re_driver__msg__WheelPwm * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // STM32_NUCLEO_F303RE_DRIVER__MSG__DETAIL__WHEEL_PWM__STRUCT_H_
