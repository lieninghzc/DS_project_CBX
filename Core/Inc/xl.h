/**
 * xl.h — 循迹模块
 */
#ifndef XL_H
#define XL_H

#include <stdint.h>
#include <stdbool.h>

void XL_Init (void);
void XL_Start (void);
void XL_Reset(void);  /* 任务结束清全部残留 */
void XL_Update_AToA (void);  /* A→A 计时绕圈 */
void XL_Update_AToAS (void); /* A→A 绕圈+控球 */
void XL_Update_AToB (void);
extern bool g_atob_done;

bool XL_IsCross (void);
bool XL_IsLost (void);
bool XL_IsSeeking (void); /* 旋转寻线中 → SM 1ms加速 */
bool XL_IsOnLine (void);
int8_t XL_LastSide (void);
uint8_t XL_GetSensors (void);
int XL_GetState (void);
int XL_GetMission (void); /* 到达A点次数 */

#endif
