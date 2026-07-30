#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

void Menu_Init(void);
bool Menu_Update(void);        /* true=有刷新 false=空闲 */
void Menu_TaskDone(void);
bool Menu_IsActive(void);
void Menu_SetEventTime(uint32_t ms);  /* ATOAS过A/ATOB过B时间 */
int  Menu_GetTask(void);

#endif
