#include "cpu/cpu.h"
#include "os_cfg.h"
#include "comm/cpu_instr.h"

static segment_desc_t gdt_table[GDT_TABLE_SIZE]; // 全局描述符表

void segment_desc_set(uint16_t selector, uint32_t base, uint32_t limit, uint8_t type, uint8_t attr) {
    uint16_t index = selector >> 3; // 选择子右移3位得到表项索引
    if (index >= GDT_TABLE_SIZE) {
        return;
    }
    
    // 处理大段界限，设置粒度位
    if (limit > 0xFFFFF) {
        attr |= (1 << 3);          // 设置 G 位（attr 的第 3 位）
        // 转换为 4KB 粒度，并确保结果在 20 位范围内
        limit = (limit + 0xFFF) / 0x1000; // 向上取整，避免截断损失
        if (limit > 0xFFFFF) {
            limit = 0xFFFFF;       // 限制最大值
        }
    }

    gdt_table[index].base_low = base & 0xFFFF;
    gdt_table[index].base_middle = (base >> 16) & 0xFF;
    gdt_table[index].base_high = (base >> 24) & 0xFF;

    gdt_table[index].limit_low = limit & 0xFFFF;
    gdt_table[index].limit_high = (limit >> 16) & 0x0F;

    gdt_table[index].type = type;
    gdt_table[index].attr = attr;
}

void init_gdt(void){
    for (int i = 0; i < GDT_TABLE_SIZE; i++) {
        segment_desc_set(i * sizeof(segment_desc_t), 0, 0, 0, 0); // 初始化所有表项为无效
    }

}

void cpu_init(void) {
    init_gdt();
} 