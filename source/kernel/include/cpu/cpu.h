#ifndef CPU_H
#define CPU_H

#include "comm/types.h"
#include "cpu/irq.h"

#define SEG_G				(1 << 3)
#define SEG_D				(1 << 2)
#define SEG_P_PRESENT	    (1 << 7)
#define SEG_DPL0			(0 << 5)
#define SEG_DPL3			(3 << 5)
#define SEG_S_SYSTEM		(0 << 4)
#define SEG_S_NORMAL		(1 << 4)
#define SEG_TYPE_CODE		(1 << 3)
#define SEG_TYPE_DATA		(0 << 3)
#define SEG_TYPE_RW			(1 << 1)


#define GATE_TYPE_IDT		(0xE << 8)		// 中断32位门描述符
#define GATE_P_PRESENT		(1 << 15)		// 是否存在
#define GATE_DPL0			(0 << 13)		// 特权级0，最高特权级
#define GATE_DPL3			(3 << 13)		// 特权级3，最低权限

#pragma pack(1)
/********** 全局描述符 **********/
typedef struct _segment_desc_t {
    uint16_t limit15_0;
    uint16_t base15_0;
    uint8_t base23_16;
    uint16_t attr;
    uint8_t base31_24;
} segment_desc_t;

/********** 中断门描述符 **********/
typedef struct _gate_desc_t {
	uint16_t offset15_0;
	uint16_t selector;
	uint16_t attr;
	uint16_t offset31_16;
}gate_desc_t;
#pragma pack()

// ---------- 用于构造 16 位 attr 值 ----------
// 组合 type（8位）、limit_high（4位）、flags（4位）为完整的 attr
#define MAKE_ATTR(type, limit_high, flags) \
    ( ((uint16_t)(type) & 0xFF) | (((uint16_t)(limit_high) & 0xF) << 8) | (((uint16_t)(flags) & 0xF) << 12) )

// 从 20 位 limit 和 flags 自动提取 limit_high 构造 attr
#define ATTR_FROM_LIMIT(type, limit, flags) \
    MAKE_ATTR(type, ((limit) >> 16) & 0xF, flags)

// 一次性设置整个描述符的宏（方便用于新函数）
#define SEGMENT_DESC_SET(desc, base, limit, type, flags) \
    do { \
        (desc).limit15_0 = (uint16_t)((limit) & 0xFFFF); \
        (desc).base15_0 = (uint16_t)((base) & 0xFFFF); \
        (desc).base23_16 = (uint8_t)(((base) >> 16) & 0xFF); \
        (desc).attr = ATTR_FROM_LIMIT(type, limit, flags); \
        (desc).base31_24 = (uint8_t)(((base) >> 24) & 0xFF); \
    } while(0)

// 函数声明
void cpu_init(void);
void segment_desc_set(uint16_t selector, uint32_t base, uint32_t limit, uint8_t type, uint8_t flags);
void gate_desc_set(uint8_t vector, uint16_t selector_cs, uint32_t offset, uint16_t attr);
void irq_install(int irq_num, irq_handler_t handler);
void irq_enable(int irq_num);
void irq_disable(int irq_num);
void irq_global_enable();
void irq_global_disable();
#endif