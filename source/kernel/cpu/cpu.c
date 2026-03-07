#include "cpu/cpu.h"
#include "os_cfg.h"
#include "comm/cpu_instr.h"
#include "cpu/irq.h"

static segment_desc_t gdt_table[GDT_TABLE_SIZE]; // 全局描述符表
static gate_desc_t idt_table[IDT_TABLE_SIZE]; // 中断描述符表

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

void gate_desc_set(uint8_t vector, uint16_t selector_cs, uint32_t offset, uint16_t attr) {
    if (vector >= IDT_TABLE_SIZE) {
        return; // 向量号超出范围
    }
    gate_desc_t *desc = &idt_table[vector];
    desc->offset15_0  = offset & 0xFFFF;
    desc->selector    = selector_cs;
    // 组合属性：低8位为 type，高8位中位8~11为 flags，位12~15保留（置0）
    desc->attr        = attr;
    desc->offset31_16 = (offset >> 16) & 0xFFFF;
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

void irq_install(int irq_num, irq_handler_t handler) {
    if (irq_num < 0 || irq_num >= IDT_TABLE_SIZE) {
        return; // 无效的中断号
    }
    gate_desc_set(irq_num, KERNEL_SELECTOR_CS, (uint32_t)handler, 
                  GATE_P_PRESENT | GATE_DPL0 | GATE_TYPE_IDT);
}

void irq_init(void) {
    // 将所有 IDT 表项设置为默认的中断处理函数
    for (int i = 0; i < IDT_TABLE_SIZE; i++) {
        gate_desc_set(i, KERNEL_SELECTOR_CS, (uint32_t)exception_handler_unknown,
                      GATE_P_PRESENT | GATE_DPL0 | GATE_TYPE_IDT);
    }

    // 设置异常处理接口
    irq_install(IRQ0_DE, exception_handler_divider);
	irq_install(IRQ1_DB, exception_handler_Debug);
	irq_install(IRQ2_NMI, exception_handler_NMI);
	irq_install(IRQ3_BP, exception_handler_breakpoint);
	irq_install(IRQ4_OF, exception_handler_overflow);
	irq_install(IRQ5_BR, exception_handler_bound_range);
	irq_install(IRQ6_UD, exception_handler_invalid_opcode);
	irq_install(IRQ7_NM, exception_handler_device_unavailable);
	irq_install(IRQ8_DF, exception_handler_double_fault);
	irq_install(IRQ10_TS, exception_handler_invalid_tss);
	irq_install(IRQ11_NP, exception_handler_segment_not_present);
	irq_install(IRQ12_SS, exception_handler_stack_segment_fault);
	irq_install(IRQ13_GP, exception_handler_general_protection);
	irq_install(IRQ14_PF, exception_handler_page_fault);
	irq_install(IRQ16_MF, exception_handler_fpu_error);
	irq_install(IRQ17_AC, exception_handler_alignment_check);
	irq_install(IRQ18_MC, exception_handler_machine_check);
	irq_install(IRQ19_XM, exception_handler_smd_exception);
	irq_install(IRQ20_VE, exception_handler_virtual_exception);

    // 加载 IDTR 寄存器
    lidt((uint32_t)idt_table, sizeof(idt_table));
}

void cpu_init(void) {
    init_gdt();
    irq_init();
} 