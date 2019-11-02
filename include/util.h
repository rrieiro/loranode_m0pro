/**
 * \file    util.h
 * \author  Robson Costa
 * \date    24/07/2019
 */
#if !defined(__UTIL_H__)
#define __UTIL_H__

#include <Arduino.h>

uint16_t f2sflt16(float);
uint16_t f2sflt12(float);
uint16_t f2uflt16(float);
uint16_t f2uflt12(float);

#endif // __UTIL_H__