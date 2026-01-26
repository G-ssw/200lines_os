/*
 * 16位引导代码
 * 二级引导，负责进行硬件检测，进行内存检测，进入保护模式，加载内核，并跳转到内核运行
 * 作者：SYKBDDM
*/

__asm__(".code16gcc");

#include "loader.h"

static boot_info_t boot_info;

static void show_msg(const char* msg){
    char c;
    while((c = *msg++) != '\0'){
        __asm__ __volatile__ (
            "movb %[ch], %%al\n\t"
            "movb $0x0e, %%ah\n\t"
            "int $0x10"
            :
            :[ch]"r"(c)
            :
        );
    }
}

static void detect_memory(void){
    uint32_t contID = 0;
    uint32_t signature, bytes;
    SMAP_entry_t smap_entry;

    show_msg("try to detect memory...\r\n");

    boot_info.ram_region_count = 0;
    for (int i = 0; i < BOOT_RAM_REGION_MAX; i++){
        SMAP_entry_t * entry = &smap_entry;

        __asm__ __volatile__(
            "int  $0x15"
			: "=a"(signature), "=c"(bytes), "=b"(contID)
			: "a"(0xE820), "b"(contID), "c"(24), "d"(0x534D4150), "D"(entry));
        if (signature != 0x534D4150) {
            show_msg("failed.\r\n");
			return;
		}

        // todo: 20字节
		if (bytes > 20 && (entry->ACPI & 0x0001) == 0){
			continue;
		}

        if (entry->Type == 1){
            boot_info.ram_region_cfg[boot_info.ram_region_count].start = entry->BaseL;
            boot_info.ram_region_cfg[boot_info.ram_region_count].size = entry->LengthL;
            boot_info.ram_region_count++;
        }

        if (contID == 0){
            break;
        }


    }
    show_msg("done.\r\n");
}

/*
 * GDT表
*/
uint16_t gdt_table[][4] = {
    {0, 0, 0, 0},
    {0xFFFF, 0X0000, 0X9A00, 0X00CF}, // 代码段
    {0xFFFF, 0X0000, 0X9200, 0X00CF}, // 数据段
};

/*
 * 进入保护模式
*/
static void enter_protect_mode(void){
    // 关中断
    cli();

    // 开启A20地址线，使其可以访问1MB以上内存
    uint8_t portA;
    portA = inb(0x92);
    outb(0x92, portA | 0x02);

    // 加载GDT，设置CR0，跳转到保护模式下运行
    lgdt((uint32_t)gdt_table, sizeof(gdt_table));

    // 设置CR0的PE位
    uint32_t cr0 = read_cr0();
    cr0 |= 0x1;
    write_cr0(cr0);

    // 远跳转到保护模式下的代码段
    far_jump(0x08, (uint32_t)protect_mode_entry);


}  


void loader_entry(void) {
    show_msg("Loader has taken control!\r\n");
    detect_memory();
    enter_protect_mode();
    while(1);
}