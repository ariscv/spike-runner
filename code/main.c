#include <stdint.h>
#include "csrs.h"
#include "ee_printf.h"
#include <printk.h>
#include <platform.h>

#define MTIME       (*(volatile uint64_t *)(CLINT_BASE + 0xBFF8))
#define MTIMECMP    (*(volatile uint64_t *)(CLINT_BASE + 0x4000))

// 全局变量用于存储当前时间
uint64_t current_time = 0;

// 汇编指令用于启用中断
static inline void enable_interrupts() {
    CSRS(mstatus, 8); // 设置 MSTATUS.MIE = 1
    CSRS(mie, 128);   // 启用 MTIE (Machine Timer Interrupt Enable)
}

// 设置下一次定时器中断
void set_timer(uint64_t interval) {
    current_time += interval;
    MTIMECMP = current_time;
}


__attribute__((interrupt)) void trap_handler(){
	uint32_t cause = ~(1U<<31)&CSRR(mcause);
	ee_printf("trap: %d\n", cause);
	ee_printf("%d\n",(uint32_t)(((1ULL<<32)-1)&MTIME));
	
	// set_timer(1000);
	// CSRC(mip, MIP_SSIE);
	
	// asm volatile("ebreak");
}


int main(){
	csr_write(mtvec, trap_handler);
	// ee_printf("hello world%d\n",1212);
	// uint64_t time;
    // asm volatile("rdtime %0" : "=r"(time)); 
	
	    // 初始设置定时器
		current_time = MTIME;
		set_timer(1000);  // 设置一个初始的定时周期，例如 1 秒（根据系统时钟频率调整）
	
		// 启用中断
		enable_interrupts();

		while (1) {
			// ee_printf("%d\n",MTIME);
			asm volatile("wfi");  // 等待中断
			asm volatile("nop");
			asm volatile("nop");
			asm volatile("nop");
		}

	// printk("time: %d\n", time);
	// _serinit();
	// _puts("12\nABC\n\n21");
	return 0;
}