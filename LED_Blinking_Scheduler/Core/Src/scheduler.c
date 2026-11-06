/*
 * scheduler.c
 * Cooperative scheduler.
 *
 * Ý tưởng:
 *  - Mỗi task lưu thời điểm đến hạn TUYỆT ĐỐI (NextRun, đơn vị tick).
 *  - Mảng luôn sắp xếp tăng dần theo NextRun => SCH_tasks_G[0] là task sớm nhất.
 *  - SCH_Update() (ISR) chỉ làm sys_ticks++  => O(1), không đụng vào mảng,
 *    nên không có race với main và không bao giờ mất tick.
 *  - SCH_Dispatch_Tasks() (main loop) chạy task khi sys_ticks >= NextRun.
 *    Task periodic được đặt lại NextRun += Period (không bị lệch dần).
 *  - "Thời gian còn lại" của task = NextRun - sys_ticks.
 */
#include "scheduler.h"
#include <stdio.h>

/* ==================== BIẾN TOÀN CỤC ==================== */
sTask SCH_tasks_G[SCH_MAX_TASKS];
uint8_t Error_code_G = 0;

static uint32_t task_count = 0;               // số task đang có
static volatile uint32_t sys_ticks = 0;       // đếm tick, chỉ ISR ghi
static uint32_t next_task_id = 1;             // bộ cấp ID duy nhất
static uint32_t running_id = NO_TASK_ID;      // ID task đang chạy (để cho phép tự xóa)
static uint8_t running_deleted = 0;           // task đang chạy bị yêu cầu xóa

#ifdef SCH_REPORT_ERRORS
static uint8_t  last_error_code = 0;          // mã lỗi đã báo gần nhất
static uint32_t error_report_start = 0;       // thời điểm (ms) bắt đầu báo lỗi hiện tại
#endif

/* ==================== HÀM PRIVATE ==================== */

/* Chèn task vào đúng vị trí để mảng vẫn sắp xếp theo NextRun.
 * Trùng NextRun thì task thêm sau đứng sau => chạy đúng thứ tự thêm vào. */
static void insert_sorted(const sTask *t) {
    uint32_t pos = task_count;
    for (uint32_t i = 0; i < task_count; i++) {
        // So sánh kiểu có dấu để đúng cả khi sys_ticks tràn 32 bit
        if ((int32_t)(t->NextRun - SCH_tasks_G[i].NextRun) < 0) {
            pos = i;
            break;
        }
    }
    for (uint32_t j = task_count; j > pos; j--) {
        SCH_tasks_G[j] = SCH_tasks_G[j - 1];
    }
    SCH_tasks_G[pos] = *t;
    task_count++;
}

/* Xóa phần tử ở vị trí index, dồn mảng sang trái. */
static void remove_at(uint32_t index) {
    for (uint32_t k = index; k + 1 < task_count; k++) {
        SCH_tasks_G[k] = SCH_tasks_G[k + 1];
    }
    task_count--;
    SCH_tasks_G[task_count] = (sTask){0};
}

/* ==================== IMPLEMENTATION ==================== */

void SCH_Init(void) {
    for (uint32_t i = 0; i < SCH_MAX_TASKS; i++) {
        SCH_tasks_G[i] = (sTask){0};
    }
    task_count = 0;
    sys_ticks = 0;
    next_task_id = 1;
    running_id = NO_TASK_ID;
    running_deleted = 0;
    Error_code_G = 0;
}

/* Gọi trong ngắt timer. O(1). */
void SCH_Update(void) {
    sys_ticks++;
}

uint32_t get_time(void) {
    return sys_ticks * TIMER_TICK_MS;   // ms, độ chính xác 1 tick (10 ms)
}

uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD) {
    if (pFunction == 0) {
        return NO_TASK_ID;
    }
    if (task_count >= SCH_MAX_TASKS) {
        Error_code_G = ERROR_SCH_TOO_MANY_TASKS;
        return NO_TASK_ID;
    }

    sTask t;
    t.pTask   = pFunction;
    t.NextRun = sys_ticks + DELAY;
    t.Period  = PERIOD;
    t.TaskID  = next_task_id++;

    insert_sorted(&t);
    return t.TaskID;
}

uint8_t SCH_Delete_Task(const uint32_t TASK_ID) {
    // Task đang chạy tự xóa mình (hoặc bị xóa từ chính nó): đánh dấu, Dispatch sẽ không nạp lại
    if (TASK_ID != NO_TASK_ID && TASK_ID == running_id) {
        running_deleted = 1;
        return RETURN_NORMAL;
    }

    for (uint32_t i = 0; i < task_count; i++) {
        if (SCH_tasks_G[i].TaskID == TASK_ID) {
            remove_at(i);
            return RETURN_NORMAL;
        }
    }
    Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
    return RETURN_ERROR;
}

void SCH_Dispatch_Tasks(void) {
    // Task đầu mảng là task sớm nhất: đến hạn thì chạy, chưa đến hạn thì dừng
    while (task_count > 0 &&
           (int32_t)(sys_ticks - SCH_tasks_G[0].NextRun) >= 0) {

        sTask t = SCH_tasks_G[0];   // lấy bản sao
        remove_at(0);               // gỡ khỏi mảng trước khi chạy

        running_id = t.TaskID;
        running_deleted = 0;
        t.pTask();                  // chạy task đến khi xong (cooperative)
        running_id = NO_TASK_ID;

        if (t.Period > 0 && !running_deleted) {
            t.NextRun += t.Period;  // tính từ lịch cũ => không drift
            insert_sorted(&t);
        }
        // Period == 0: one-shot, đã bị gỡ khỏi mảng
    }

    SCH_Report_Status();
    SCH_Go_To_Sleep();
}

#ifdef SCH_REPORT_ERRORS
static const char *error_name(uint8_t code) {
    switch (code) {
        case ERROR_SCH_TOO_MANY_TASKS:     return "TOO_MANY_TASKS";
        case ERROR_SCH_CANNOT_DELETE_TASK: return "CANNOT_DELETE_TASK";
        default:                           return "UNKNOWN";
    }
}
#endif

/*
 * Báo cáo lỗi: được gọi cuối mỗi lần Dispatch (trong main loop).
 *  - Khi Error_code_G đổi sang một mã lỗi mới => in 1 dòng qua UART và bắt đầu đếm giờ.
 *  - Sau SCH_ERROR_REPORT_MS (60 s) => tự xóa mã lỗi về 0.
 * Dùng get_time() (ms) để đếm giờ vì hàm này được gọi theo vòng lặp, không theo tick.
 */
void SCH_Report_Status(void) {
#ifdef SCH_REPORT_ERRORS
    if (Error_code_G != last_error_code) {
        last_error_code = Error_code_G;
        if (Error_code_G != 0) {
            error_report_start = get_time();
            printf("[SCH ERROR] code=%u (%s) - Timestamp: %lu ms\r\n",
                   (unsigned)Error_code_G,
                   error_name(Error_code_G),
                   (unsigned long)error_report_start);
        }
    } else if (Error_code_G != 0) {
        if ((uint32_t)(get_time() - error_report_start) >= SCH_ERROR_REPORT_MS) {
            Error_code_G = 0;       // hết thời gian báo lỗi => xóa
            last_error_code = 0;
        }
    }
#endif
}

/*
 * Cho CPU ngủ (Sleep mode) đến khi có ngắt tiếp theo.
 * Scheduler luôn có ngắt đều (TIM2 mỗi 10 ms, SysTick mỗi 1 ms) nên CPU tự thức dậy.
 * Task đến hạn giữa hai lần thức dậy chỉ bị trễ tối đa ~1 ms (chu kỳ SysTick).
 */
void SCH_Go_To_Sleep(void) {
#ifdef SCH_ENABLE_SLEEP
    CLEAR_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);   // Sleep mode (không phải Stop/Standby)
    __WFI();                                      // Wait For Interrupt
#endif
}

uint32_t SCH_Get_Current_Size(void) {
    return task_count;
}
