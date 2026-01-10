// generated from
// rosidl_typesupport_fastrtps_c/resource/rosidl_typesupport_fastrtps_c__visibility_control.h.in
// generated code does not contain a copyright notice

#ifndef STM32_NUCLEO_F303RE_DRIVER__MSG__ROSIDL_TYPESUPPORT_FASTRTPS_C__VISIBILITY_CONTROL_H_
#define STM32_NUCLEO_F303RE_DRIVER__MSG__ROSIDL_TYPESUPPORT_FASTRTPS_C__VISIBILITY_CONTROL_H_

#if __cplusplus
extern "C"
{
#endif

// This logic was borrowed (then namespaced) from the examples on the gcc wiki:
//     https://gcc.gnu.org/wiki/Visibility

#if defined _WIN32 || defined __CYGWIN__
  #ifdef __GNUC__
    #define ROSIDL_TYPESUPPORT_FASTRTPS_C_EXPORT_stm32_nucleo_f303re_driver __attribute__ ((dllexport))
    #define ROSIDL_TYPESUPPORT_FASTRTPS_C_IMPORT_stm32_nucleo_f303re_driver __attribute__ ((dllimport))
  #else
    #define ROSIDL_TYPESUPPORT_FASTRTPS_C_EXPORT_stm32_nucleo_f303re_driver __declspec(dllexport)
    #define ROSIDL_TYPESUPPORT_FASTRTPS_C_IMPORT_stm32_nucleo_f303re_driver __declspec(dllimport)
  #endif
  #ifdef ROSIDL_TYPESUPPORT_FASTRTPS_C_BUILDING_DLL_stm32_nucleo_f303re_driver
    #define ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_stm32_nucleo_f303re_driver ROSIDL_TYPESUPPORT_FASTRTPS_C_EXPORT_stm32_nucleo_f303re_driver
  #else
    #define ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_stm32_nucleo_f303re_driver ROSIDL_TYPESUPPORT_FASTRTPS_C_IMPORT_stm32_nucleo_f303re_driver
  #endif
#else
  #define ROSIDL_TYPESUPPORT_FASTRTPS_C_EXPORT_stm32_nucleo_f303re_driver __attribute__ ((visibility("default")))
  #define ROSIDL_TYPESUPPORT_FASTRTPS_C_IMPORT_stm32_nucleo_f303re_driver
  #if __GNUC__ >= 4
    #define ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_stm32_nucleo_f303re_driver __attribute__ ((visibility("default")))
  #else
    #define ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_stm32_nucleo_f303re_driver
  #endif
#endif

#if __cplusplus
}
#endif

#endif  // STM32_NUCLEO_F303RE_DRIVER__MSG__ROSIDL_TYPESUPPORT_FASTRTPS_C__VISIBILITY_CONTROL_H_
