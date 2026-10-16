#pragma once

#ifndef bitRead
#define bitRead(value, bit) (((value) >> (bit)) & 0x01U)
#endif