// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from stm32_nucleo_f303re_driver:msg/WheelPwm.idl
// generated code does not contain a copyright notice
#include "stm32_nucleo_f303re_driver/msg/detail/wheel_pwm__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `header`
#include "std_msgs/msg/detail/header__functions.h"
// Member `pwm`
#include "rosidl_runtime_c/primitives_sequence_functions.h"

bool
stm32_nucleo_f303re_driver__msg__WheelPwm__init(stm32_nucleo_f303re_driver__msg__WheelPwm * msg)
{
  if (!msg) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__init(&msg->header)) {
    stm32_nucleo_f303re_driver__msg__WheelPwm__fini(msg);
    return false;
  }
  // pwm
  if (!rosidl_runtime_c__int32__Sequence__init(&msg->pwm, 0)) {
    stm32_nucleo_f303re_driver__msg__WheelPwm__fini(msg);
    return false;
  }
  return true;
}

void
stm32_nucleo_f303re_driver__msg__WheelPwm__fini(stm32_nucleo_f303re_driver__msg__WheelPwm * msg)
{
  if (!msg) {
    return;
  }
  // header
  std_msgs__msg__Header__fini(&msg->header);
  // pwm
  rosidl_runtime_c__int32__Sequence__fini(&msg->pwm);
}

bool
stm32_nucleo_f303re_driver__msg__WheelPwm__are_equal(const stm32_nucleo_f303re_driver__msg__WheelPwm * lhs, const stm32_nucleo_f303re_driver__msg__WheelPwm * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__are_equal(
      &(lhs->header), &(rhs->header)))
  {
    return false;
  }
  // pwm
  if (!rosidl_runtime_c__int32__Sequence__are_equal(
      &(lhs->pwm), &(rhs->pwm)))
  {
    return false;
  }
  return true;
}

bool
stm32_nucleo_f303re_driver__msg__WheelPwm__copy(
  const stm32_nucleo_f303re_driver__msg__WheelPwm * input,
  stm32_nucleo_f303re_driver__msg__WheelPwm * output)
{
  if (!input || !output) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__copy(
      &(input->header), &(output->header)))
  {
    return false;
  }
  // pwm
  if (!rosidl_runtime_c__int32__Sequence__copy(
      &(input->pwm), &(output->pwm)))
  {
    return false;
  }
  return true;
}

stm32_nucleo_f303re_driver__msg__WheelPwm *
stm32_nucleo_f303re_driver__msg__WheelPwm__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  stm32_nucleo_f303re_driver__msg__WheelPwm * msg = (stm32_nucleo_f303re_driver__msg__WheelPwm *)allocator.allocate(sizeof(stm32_nucleo_f303re_driver__msg__WheelPwm), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(stm32_nucleo_f303re_driver__msg__WheelPwm));
  bool success = stm32_nucleo_f303re_driver__msg__WheelPwm__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
stm32_nucleo_f303re_driver__msg__WheelPwm__destroy(stm32_nucleo_f303re_driver__msg__WheelPwm * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    stm32_nucleo_f303re_driver__msg__WheelPwm__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence__init(stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  stm32_nucleo_f303re_driver__msg__WheelPwm * data = NULL;

  if (size) {
    data = (stm32_nucleo_f303re_driver__msg__WheelPwm *)allocator.zero_allocate(size, sizeof(stm32_nucleo_f303re_driver__msg__WheelPwm), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = stm32_nucleo_f303re_driver__msg__WheelPwm__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        stm32_nucleo_f303re_driver__msg__WheelPwm__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence__fini(stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      stm32_nucleo_f303re_driver__msg__WheelPwm__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence *
stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence * array = (stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence *)allocator.allocate(sizeof(stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence__destroy(stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence__are_equal(const stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence * lhs, const stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!stm32_nucleo_f303re_driver__msg__WheelPwm__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence__copy(
  const stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence * input,
  stm32_nucleo_f303re_driver__msg__WheelPwm__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(stm32_nucleo_f303re_driver__msg__WheelPwm);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    stm32_nucleo_f303re_driver__msg__WheelPwm * data =
      (stm32_nucleo_f303re_driver__msg__WheelPwm *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!stm32_nucleo_f303re_driver__msg__WheelPwm__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          stm32_nucleo_f303re_driver__msg__WheelPwm__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!stm32_nucleo_f303re_driver__msg__WheelPwm__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
