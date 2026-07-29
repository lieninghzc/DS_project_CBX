/**
 * xl.h — 循迹模块
 */
#ifndef XL_H
#define XL_H

#include <stdint.h>
#include <stdbool.h>

void XL_Init(void);
void XL_Start(void);   /* 直入RUN, 不等MISSION_1 */
void XL_Update(void);

bool XL_IsCross(void);
bool XL_IsLost(void);
bool XL_IsSeeking(void);       /* 旋转寻线中 → SM 1ms加速 */
bool XL_IsOnLine(void);
int8_t XL_LastSide(void);
uint8_t XL_GetSensors(void);
int XL_GetState(void);
int XL_GetMission(void);   /* 到达A点次数 */

#endif
