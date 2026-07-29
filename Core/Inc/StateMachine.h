#ifndef STATEMACHINE_H
#define STATEMACHINE_H

typedef enum { TASK_NONE, TASK_ATOA, TASK_ASTATIC, TASK_ATOB, TASK_ATOAS } TaskType;

void SM_Init(void);
void SM_Run(void);
TaskType SM_GetTask(void);

#endif
