#include "cpu/irq.h"
#include "cpu/cpu.h"
#include "comm/cpu_instr.h"
#include "tools/log.h"
#include "os_cfg.h"

void dump_exception_frame(exception_frame_t *frame) {
	log_printf("Exception Frame Dump:\n");
	log_printf("EIP: 0x%08x, CS: 0x%04x, EFLAGS: 0x%08x\n", frame->eip, frame->cs, frame->eflags);
	log_printf("IRQ: %d, Error Code: 0x%d\n", frame->num, frame->error_code);
	log_printf("Registers: EAX=0x%08x, EBX=0x%08x, ECX=0x%08x, EDX=0x%08x\n",
			   frame->eax, frame->ebx, frame->ecx, frame->edx);
	log_printf("ESI=0x%08x, EDI=0x%08x, EBP=0x%08x, ESP=0x%08x\n",
			   frame->esi, frame->edi, frame->ebp, frame->esp);
	log_printf("Segment Registers: DS=0x%04x, ES=0x%04x, FS=0x%04x, GS=0x%04x\n",
			   frame->ds, frame->es, frame->fs, frame->gs);
}

static void do_default_handler (exception_frame_t * frame,const char * message) {
	log_printf("\n---------------------------------\n");
	log_printf("Exception: %s\n", message);
	dump_exception_frame(frame);
	log_printf("---------------------------------\n");
    for (;;) {hlt();}
}

void do_handler_unknown (exception_frame_t * frame) {
	do_default_handler(frame, "Unknown exception.");
}

void do_handler_divider(exception_frame_t * frame) {
	do_default_handler(frame, "Device Error.");
}

void do_handler_Debug(exception_frame_t * frame) {
	do_default_handler(frame, "Debug Exception");
}

void do_handler_NMI(exception_frame_t * frame) {
	do_default_handler(frame, "NMI Interrupt.");
}

void do_handler_breakpoint(exception_frame_t * frame) {
	do_default_handler(frame, "Breakpoint.");
}

void do_handler_overflow(exception_frame_t * frame) {
	do_default_handler(frame, "Overflow.");
}

void do_handler_bound_range(exception_frame_t * frame) {
	do_default_handler(frame, "BOUND Range Exceeded.");
}

void do_handler_invalid_opcode(exception_frame_t * frame) {
	do_default_handler(frame, "Invalid Opcode.");
}

void do_handler_device_unavailable(exception_frame_t * frame) {
	do_default_handler(frame, "Device Not Available.");
}

void do_handler_double_fault(exception_frame_t * frame) {
	do_default_handler(frame, "Double Fault.");
}

void do_handler_invalid_tss(exception_frame_t * frame) {
	do_default_handler(frame, "Invalid TSS");
}

void do_handler_segment_not_present(exception_frame_t * frame) {
	do_default_handler(frame, "Segment Not Present.");
}

void do_handler_stack_segment_fault(exception_frame_t * frame) {
	do_default_handler(frame, "Stack-Segment Fault.");
}

void do_handler_general_protection(exception_frame_t * frame) {
	do_default_handler(frame, "General Protection.");
}

void do_handler_page_fault(exception_frame_t * frame) {
	do_default_handler(frame, "Page Fault.");
}

void do_handler_fpu_error(exception_frame_t * frame) {
	do_default_handler(frame, "X87 FPU Floating Point Error.");
}

void do_handler_alignment_check(exception_frame_t * frame) {
	do_default_handler(frame, "Alignment Check.");
}

void do_handler_machine_check(exception_frame_t * frame) {
	do_default_handler(frame, "Machine Check.");
}

void do_handler_smd_exception(exception_frame_t * frame) {
	do_default_handler(frame, "SIMD Floating Point Exception.");
}

void do_handler_virtual_exception(exception_frame_t * frame) {
	do_default_handler(frame, "Virtualization Exception.");
}

uint32_t irq_enter_protection(void){
	uint32_t eflags = read_elfags();
	cli(); // 进入临界区，禁止中断
	return eflags; // 返回之前的 EFLAGS 状态
}

void irq_exit_protection(uint32_t eflags){
	write_eflags(eflags); // 恢复之前的 EFLAGS 状态，可能会重新启用中断
}



