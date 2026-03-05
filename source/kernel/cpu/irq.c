#include "cpu/irq.h"
#include "cpu/cpu.h"
#include "comm/cpu_instr.h"
#include "os_cfg.h"


static void do_default_handler (const char * message) {
    for (;;) {}
}

void do_handler_unknown (void) {
	do_default_handler("Unknown exception.");
}
