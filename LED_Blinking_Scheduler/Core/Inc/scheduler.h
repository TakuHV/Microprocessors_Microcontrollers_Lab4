#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include <stdint.h>
#include "main.h"

#define SCH_MAX_TASKS           40
#define NO_TASK_ID              0       // ID hợp lệ bắt đầu từ 1
#define TIMER_TICK_MS           10      // Timer ngắt mỗi 10 ms

#define SCH_REPORT_ERRORS               // Báo lỗi qua UART (printf). Comment dòng này để tắt
#define SCH_ERROR_REPORT_MS     60000   // Giữ mã lỗi 60 s rồi tự xóa về 0
#define SCH_ENABLE_SLEEP                // Cho CPU ngủ (WFI) giữa các tick. Comment dòng này để tắt

#define ERROR_SCH_TOO_MANY_TASKS                    1
#define ERROR_SCH_CANNOT_DELETE_TASK                2

#define RETURN_ERROR            0
#define RETURN_NORMAL           1

typedef struct {
    void (*pTask)(void);        // Con trỏ tới hàm task
    uint32_t NextRun;           // Thời điểm đến hạn (tick tuyệt đối, tính từ lúc SCH_Init)
    uint32_t Period;            // Chu kỳ (tick). 0 = one-shot
    uint32_t TaskID;            // ID duy nhất của task (>= 1)
} sTask;

extern sTask SCH_tasks_G[SCH_MAX_TASKS];   // Luôn sắp xếp tăng dần theo NextRun
extern uint8_t Error_code_G;


/** Khởi tạo scheduler (gọi TRƯỚC khi bật timer). */
void SCH_Init(void);

/** Gọi trong ngắt timer mỗi TIMER_TICK_MS. Chỉ tăng bộ đếm tick: O(1). */
void SCH_Update(void);

/** Gọi trong while(1) của main: chạy các task đã đến hạn. */
void SCH_Dispatch_Tasks(void);

/**
 * @brief Thêm task
 * @param DELAY  số tick chờ trước lần chạy đầu (0 = chạy ngay)
 * @param PERIOD số tick giữa các lần chạy (0 = chạy 1 lần)
 * @return TaskID (>= 1), hoặc NO_TASK_ID nếu mảng đầy
 */
uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD);

/** Xóa task theo TaskID. */
uint8_t SCH_Delete_Task(const uint32_t TASK_ID);

/** Thời gian hệ thống (ms), độ phân giải TIMER_TICK_MS. */
uint32_t get_time(void);

void SCH_Report_Status(void);
void SCH_Go_To_Sleep(void);
uint32_t SCH_Get_Current_Size(void);

#endif /* INC_SCHEDULER_H_ */
