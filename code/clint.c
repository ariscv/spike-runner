/**
 * @Author: FF
 * @Date:   2024-12-25 14:24:52
 * @Last Modified by:   FF
 * @Last Modified time: 2025-01-06 18:19:06
 */


 #include <stdbool.h>
//  #include "../common/default_sys.h"
//  #include "../common/encoding.h"
//  #include "../common/rv_gpio.h"
 
 
 #define U32                     *(volatile unsigned int *)
 #define CoreGen_CLINT_BASE      0x00060000
 #define MTIMECTRL               0x00060080
 #define MY_TIME                 100		                    /* Set timecmp value */
 typedef unsigned long long      uint64_t;
 
 int time_tick = 0;
 
 void clint_csr_cfg()
 {
	 unsigned int csr_tmp;
 
	 /* mie.MEIE-> bit11, mie.MTIE -> bit7, mie.MSIE -> bit3  */
	 csr_tmp = read_csr(mie);
	 write_csr(mie, (csr_tmp | 0xFFFF0888));
 
	 /* 仅打开外部中断，用于测试PLIC和NMI */
	 /* write_csr(mie, (csr_tmp | 0xFFFF0800)); */
 
	 /* mstatus.MIE M模式下中断总开关 */
	 csr_tmp = read_csr(mstatus);
	 write_csr(mstatus,(csr_tmp | 0x8));
 
	 /* set mtimectrl bit11 bit15 to open plic*/
	 U32(MTIMECTRL) |= ((1 << 11) | (1 << 15));
 
	 /* set mtimectrl bit12  to open NMI*/
	 U32(MTIMECTRL) |= ((1 << 12) | (1 << 15));
 }
 
 void CLINT_msip_write(unsigned int msip_num)
 {
	 U32(CoreGen_CLINT_BASE + 0x10) = msip_num;
 }
 
 //mtime write and read
 void CLINT_mtime_write( uint64_t mtime_num )
 {
	 unsigned int LOW, HIGH;
	 LOW = mtime_num;
	 HIGH = mtime_num >> 32;
	 U32( CoreGen_CLINT_BASE + 0) = LOW;
	 U32( CoreGen_CLINT_BASE + 4) = HIGH;
 }
 //mtime write and read
 uint64_t CLINT_mtime_read()
 {
	 uint64_t LOW, HIGH;
	 uint64_t mtime_read;
	 LOW = U32(CoreGen_CLINT_BASE + 0);
	 HIGH = U32(CoreGen_CLINT_BASE + 4);
	 mtime_read = (HIGH << 32) | LOW;
	 return mtime_read;
 }
 
 uint64_t CLINT_mtime_cmp_read()
 {
	 uint64_t LOW, HIGH;
	 uint64_t mtime_cmp_read;
	 LOW = U32(CoreGen_CLINT_BASE + 8);
	 HIGH = U32(CoreGen_CLINT_BASE + 12);
	 mtime_cmp_read = (HIGH << 32) | LOW;
	 return mtime_cmp_read;
 }
 void CLINT_mtime_cmp_write(uint64_t mtime_cmp_num)
 {
	 unsigned int LOW, HIGH;
 
	 LOW = mtime_cmp_num;
	 HIGH = mtime_cmp_num >> 32;
	 U32(CoreGen_CLINT_BASE + 8) = LOW;
	 U32(CoreGen_CLINT_BASE + 12) = HIGH;
 }
 
 
 
 /* Test plic code */
 // plictime write
 void CLINT_mplictime_write( uint32_t mplictime_num )
 {
	 U32( CoreGen_CLINT_BASE + 0xf8) = mplictime_num;
 }
 
 // mplictime read
 uint32_t CLINT_mplictime_read()
 {
	 uint32_t mplictime_read = 0;
 
	 mplictime_read = U32( CoreGen_CLINT_BASE + 0xf8);
 
	 return mplictime_read;
 }
 
 uint32_t CLINT_mplictime_cmp_read()
 {
	 uint32_t mplictime_cmp_read = 0;
 
	 mplictime_cmp_read = U32( CoreGen_CLINT_BASE + 0xfc);
 
	 return mplictime_cmp_read;
 }
 
 void CLINT_mplictime_cmp_write(uint32_t mtime_cmp_num)
 {
	 U32( CoreGen_CLINT_BASE + 0xfc) = mtime_cmp_num;
 }
 
 
 /* ------------Test NMI code------------- */
 
 /* nmitime write */
 void CLINT_nmitime_write( uint32_t mitime_num )
 {
	 U32( CoreGen_CLINT_BASE + 0xf0) = mitime_num;
 }
 
 /* nmitime read */
 uint32_t CLINT_nmitime_read()
 {
	 uint32_t nmitime_read = 0;
 
	 nmitime_read = U32( CoreGen_CLINT_BASE + 0xf0);
 
	 return nmitime_read;
 }
 
 void CLINT_nmitime_cmp_write(uint32_t nmitime_cmp_num)
 {
	 U32( CoreGen_CLINT_BASE + 0xf4) = nmitime_cmp_num;
 }
 
 uint32_t CLINT_nmitime_cmp_read()
 {
	 uint32_t nmitime_cmp_read = 0;
 
	 nmitime_cmp_read = U32( CoreGen_CLINT_BASE + 0xf4);
 
	 return nmitime_cmp_read;
 }
 
 /* -----------Interrupt handler function-------------- */
 int a = 0;
 void __attribute__((interrupt)) simple_exc_handler(void)
 {
	 uint32_t cause = get_mcause();
 
	 cause = cause & 0x1f;
	 /* Machine timer interrupt */
	 if( cause == 7 ) {
		 time_tick++;
		 CLINT_mtime_write( 0 );
		 CLINT_mtime_cmp_write( CLINT_mtime_read() + MY_TIME );
	 }
	 /* Machine sofeware interrupt */
	 else if( cause == 3 ) {
		 time_tick += 0x100000;
		 CLINT_msip_write(0);
	 }
	 /* Machine external interrupt */
	 else if( cause == 11 ) {
		 a++;
		 CLINT_mplictime_cmp_write(500*a);
	 }
	 /* NMItime interrupter server */
	 else if( cause == 15 ) {
		 a++;
		 CLINT_nmitime_cmp_write(500*a);
	 }
 }
 
 void CLINT_Init(void)
 {
	 CLINT_msip_write(0);
 
	 /* mtime config */
	 CLINT_mtime_write(0);
	 CLINT_mtime_cmp_write(MY_TIME);
 
	 /* mplictime config */
	 CLINT_mplictime_write(0);
	 CLINT_mplictime_cmp_write(MY_TIME);
 
	 /* nmitime config*/
	 CLINT_nmitime_write(0);
	 CLINT_nmitime_cmp_write(MY_TIME);
 
	 clint_csr_cfg();
 
 }
 
 
 volatile int tttt=0;
 void soft_delay_ms( int t )
 {
	 for(int i = 0; i < t; ++i ) {
		 for(int j = 0; j < 5000; ++j ) {
			 tttt++;
		 }
	 }
 }
 
 void wfi_test() {
	 /*
		 Wait for Interrupt (WFI) instruction to put the processor
		 in a low-power state until an interrupt occurs
	 */
	 asm volatile(
		 "wfi"
		 : /* no output operands */
		 : /* no input operands */
		 : /* no clobbered registers */
	 );
 }
 
 
 volatile int mytick=0;
 int main(void)
 {
	 CLINT_Init();
 
	 mytick=0;
	 while(true) {
		 /* sofeware interrupt trigger */
		 CLINT_msip_write(1);
		 /* test interrupt weak up wfi instrution */
		 wfi_test();
		 soft_delay_ms( 10 );
		 soft_delay_ms( 10 );
		 soft_delay_ms( 10 );
		 mytick++;
	 }
 }
 
 
 
 
 