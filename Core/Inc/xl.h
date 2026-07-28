/**
 * xl.h — 循迹模块
 */
#ifndef XL_H
#define XL_H

#include <stdint.h>
#include <stdbool.h>

void XL_Init(void);
void XL_Update(void);

bool XL_IsCross(void);
bool XL_IsLost(void);
bool XL_IsSeeking(void);       /* 旋转寻线中 → SM 1ms加速 */
bool XL_IsOnLine(void);
int8_t XL_LastSide(void);

#endif
