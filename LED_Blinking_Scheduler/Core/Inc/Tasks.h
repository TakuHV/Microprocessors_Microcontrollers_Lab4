/*
 * Tasks.h
 * Khai báo các task cho scheduler
 */
#ifndef APP_TASKS_H_
#define APP_TASKS_H_

#include "main.h"
#include "scheduler.h"      // để có TIMER_TICK_MS, NO_TASK_ID

/* ==================== TASK TIMING CONSTANTS ==================== */
// Chu kỳ tính bằng ms
#define TASK_LED1_DELAY_MS      500
#define TASK_LED2_DELAY_MS      1000
#define TASK_LED3_DELAY_MS      1500
#define TASK_LED4_DELAY_MS      2000
#define TASK_LED5_DELAY_MS      2500
#define TASK_10MS_DELAY_MS      10

// Chu kỳ tính bằng tick (1 tick = TIMER_TICK_MS = 10 ms)
#define TASK_LED1_TICKS         (TASK_LED1_DELAY_MS / TIMER_TICK_MS)   // 50
#define TASK_LED2_TICKS         (TASK_LED2_DELAY_MS / TIMER_TICK_MS)   // 100
#define TASK_LED3_TICKS         (TASK_LED3_DELAY_MS / TIMER_TICK_MS)   // 150
#define TASK_LED4_TICKS         (TASK_LED4_DELAY_MS / TIMER_TICK_MS)   // 200
#define TASK_LED5_TICKS         (TASK_LED5_DELAY_MS / TIMER_TICK_MS)   // 250
#define TASK_10MS_TICKS         (TASK_10MS_DELAY_MS / TIMER_TICK_MS)   // 1

/* ID của Task_10ms, lưu lại để xóa sau này bằng SCH_Delete_Task */
extern uint32_t Task10ms_ID;

/* ==================== TASK FUNCTION PROTOTYPES ==================== */
void Task_LED1(void);   // đảo LED1 mỗi 500 ms
void Task_LED2(void);   // đảo LED2 mỗi 1000 ms
void Task_LED3(void);   // đảo LED3 mỗi 1500 ms
void Task_LED4(void);   // đảo LED4 mỗi 2000 ms
void Task_LED5(void);   // đảo LED5 mỗi 2500 ms

void Task_10ms(void);        // in timestamp mỗi 10 ms
void Task_OneShot1(void);    // one-shot (chạy 1 lần)
void Task_OneShot2(void);    // one-shot
void Task_OneShot3(void);    // one-shot
void Task_Stop10ms(void);    // one-shot: gọi SCH_Delete_Task để xóa Task_10ms
void Task_Cancelled(void);   // bị xóa trước khi chạy => KHÔNG BAO GIỜ được in

#endif /* APP_TASKS_H_ */
