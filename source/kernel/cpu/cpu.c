#include "cpu/cpu.h"
#include "os_cfg.h"
#include "comm/cpu_instr.h"

static segment_desc_t gdt_table[GDT_TABLE_SIZE]; // 全局描述符表

void segment_desc_set(uint16_t selector, uint32_t base, uint32_t limit, uint8_t type, uint8_t flags) {
    uint16_t index = selector >> 3;
    if (index >= GDT_TABLE_SIZE) return;

    // 处理大段界限，设置粒度位
    /*
    小BUG搞了好久
    为什么是错的？因为它直接修改了传入的 limit 参数，导致后续计算 attr 时使用了错误的 limit 值。正确的做法是先计算 attr，再根据需要调整 limit。原先是向下取整 deepseek给的是向上取整 结果给limit后面几位都改了,导致attr计算错误
    错误代码为：(limit + 0xFFF) / 0x1000 中的 0xFFF
    
        if (limit > 0xfffff) {
		attr |= 0x8000;
		limit /= 0x1000;
	}
    */
   if (limit > 0xFFFFF) {
        flags |= (1 << 3);          // 设置 G 位
        limit = (limit) / 0x1000; // 转换为4KB粒度
    }
    

    // 使用宏填充描述符
    SEGMENT_DESC_SET(gdt_table[index], base, limit, type, flags);
}

void init_gdt(void){
    for (int i = 0; i < GDT_TABLE_SIZE; i++) {
        segment_desc_set(i * sizeof(segment_desc_t), 0, 0, 0, 0); // 初始化所有表项为无效
    }
    // 代码段（4GB，32位，特权级0）
    segment_desc_set(KERNEL_SELECTOR_CS, 0x00000000, 0xFFFFFFFF,
                 SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_CODE | SEG_TYPE_RW,
                 SEG_D);

    // 数据段（4GB，32位，特权级0）
    segment_desc_set(KERNEL_SELECTOR_DS, 0x00000000, 0xFFFFFFFF,
                 SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_DATA | SEG_TYPE_RW,
                 SEG_D);

    // 加载 GDT
    lgdt((uint32_t)gdt_table, sizeof(gdt_table));

}

void cpu_init(void) {
    init_gdt();
} 