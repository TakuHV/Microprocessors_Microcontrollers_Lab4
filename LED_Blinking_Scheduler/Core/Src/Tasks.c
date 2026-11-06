#include "Tasks.h"
#include "scheduler.h"
#include "main.h"
#include <stdio.h>

/* Timestamp lấy từ get_time() của scheduler (ms, độ phân giải 10 ms) */

uint32_t Task10ms_ID = NO_TASK_ID;

/* ==================== 5 TASK LED ==================== */

void Task_LED1(void) {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    printf("Task 1 (0.5s) - Timestamp: %lu ms\r\n", (unsigned long)get_time());
}

void Task_LED2(void) {
    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
    printf("Task 2 (1.0s) - Timestamp: %lu ms\r\n", (unsigned long)get_time());
}

void Task_LED3(void) {
    HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
    printf("Task 3 (1.5s) - Timestamp: %lu ms\r\n", (unsigned long)get_time());
}

void Task_LED4(void) {
    HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
    printf("Task 4 (2.0s) - Timestamp: %lu ms\r\n", (unsigned long)get_time());
}

void Task_LED5(void) {
    HAL_GPIO_TogglePin(LED5_GPIO_Port, LED5_Pin);
    printf("Task 5 (2.5s) - Timestamp: %lu ms\r\n", (unsigned long)get_time());
}

/* ==================== TASK DEMO  ==================== */

/* Chạy mỗi 10 ms, in timestamp */
void Task_10ms(void) {
    printf("[10ms] Timestamp: %lu ms\r\n", (unsigned long)get_time());
}

/* Các task one-shot: Period = 0, chỉ chạy 1 lần rồi tự bị gỡ khỏi scheduler */
void Task_OneShot1(void) {
    printf(">>> One-shot 1 (delay 1s) - Timestamp: %lu ms\r\n", (unsigned long)get_time());
}

void Task_OneShot2(void) {
    printf(">>> One-shot 2 (delay 3s) - Timestamp: %lu ms\r\n", (unsigned long)get_time());
}

void Task_OneShot3(void) {
    printf(">>> One-shot 3 (delay 4s) - Timestamp: %lu ms\r\n", (unsigned long)get_time());
}

/* One-shot dùng để xóa Task_10ms bằng ID */
void Task_Stop10ms(void) {
    uint8_t result = SCH_Delete_Task(Task10ms_ID);
    printf(">>> Delete Task_10ms (ID=%lu): %s - Timestamp: %lu ms\r\n",
           (unsigned long)Task10ms_ID,
           (result == RETURN_NORMAL) ? "OK" : "FAIL",
           (unsigned long)get_time());
}

/* Task này được thêm rồi bị xóa ngay trong main  */
void Task_Cancelled(void) {
    printf("!!! Task_Cancelled DA CHAY (SAI: no phai bi xoa truoc khi chay)\r\n");
}
