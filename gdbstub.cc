#include <getopt.h>     // included for `getopt_long`
#include <libgen.h>     // included for `basename`
#include <stdlib.h>     // included for `EXIT_SUCCESS|EXIT_FAILURE`
extern "C" {
#include <gdbstub.h>
}
#include <list>

#include "mmu.h"
#include "sim.h"
#include <iostream>

#include "main.h"

#define GDB_CORE_ID (1)

class bp_t{
  public:
  bp_t(){
    // addr_list.clear();
    std::cout << "===size:" <<addr_list.size() << std::endl;
  }
  bool add_bp(uint32_t addr){
    if(is_full())
      return false;
    addr_list.push_back(addr);
    return true;
  }
  bool del_bp(uint32_t addr){
    // printf("===del_bp: %x\n", addr);
    addr_list.remove(addr);
    // print();
    return true;
  }
  bool judge_eq(uint32_t addr){
    if(addr_list.empty()) return false;
    std::list<uint32_t>::iterator it=addr_list.begin();
    // print();
    // printf("judge_eq(%x)\n",addr);
    for(;it!=addr_list.end();it++)
    {
        // printf("judge_eq item: %x\n",*it);
        if(*it==addr){
          // printf("judge_eq ok: %x\n",*it);
          return true;
        }
          
    }
    
    return false;
  }
  void print(){
    std::list<uint32_t>::iterator it=addr_list.begin();
     printf("breakpoint lits(%d)\n",addr_list.size());
     if(addr_list.size()==0)
       return;
     int i=0;
    for(;it!=addr_list.end();it++)
    {
         printf(" breakpoint %d: at %x\n",++i,*it);
        
    }
    return;
  }
  bool is_full(){
    return bp_cnt>=2;
  }
  private:
  std::list<uint32_t> addr_list;
  uint32_t addr[2];
  int bp_cnt=0;
};
static bp_t bp;

static size_t emu_get_reg_bytes(int regno __attribute__((unused)))
{
    return 4;
}

static int emu_read_reg(void *args, int regno, void *reg_value)
{
    sim_t *s = (sim_t *) args;
    if (regno > 32) {
        return EFAULT;
    }
	state_t *state = s->get_core(GDB_CORE_ID)->get_state();
    if (regno == 32) {
        memcpy(reg_value, &state->pc, 4);
    } else {
        memcpy(reg_value, &state->XPR[regno], 4);
    }
    return 0;
}

static int emu_write_reg(void *args, int regno, void *data)
{
    sim_t *s = (sim_t *) args;

    if (regno > 32) {
        return EFAULT;
    }
	state_t *state = s->get_core(GDB_CORE_ID)->get_state();
    if (regno == 32) {
        memcpy(&state->pc, data, 4);
    } else {
        memcpy((void*)&state->XPR[regno], data, 4);
    }
    return 0;
}

static int emu_read_mem(void *args, size_t addr, size_t len, void *val)
{
// printf("read_mem addr=%x,len=%d\n",addr,len );
    sim_t *s = (sim_t *) args;
    if (addr + len > CONFIG_MSIZE) {
        return EFAULT;
    }
	processor_t *p = s->get_core(GDB_CORE_ID);
    mmu_t* mmu = p->get_mmu();

    for (size_t i = 0; i < len; i++) {
      try{
        *((uint8_t*)val+i)= mmu->load<uint8_t>(addr+i);
      }
      catch(trap_t &t){
          return EFAULT;
      }
    }
    
    return 0;
}

static int emu_write_mem(void *args, size_t addr, size_t len, void *val)
{
    sim_t *s = (sim_t *) args;
    if (addr + len > CONFIG_MSIZE) {
        return EFAULT;
    }
	processor_t *p = s->get_core(GDB_CORE_ID);
      mmu_t* mmu = p->get_mmu();

    for (size_t i = 0; i < len; i++) {
      mmu->store<uint8_t>(addr+i, *((uint8_t*)val+i));
    }
    return 0;
}

static gdb_action_t emu_cont(void *args)
{
    sim_t *s = (sim_t *) args;
	processor_t *p = s->get_core(GDB_CORE_ID);
	state_t *state = s->get_core(GDB_CORE_ID)->get_state();
        while ( 1) {
          // printf("emu_cont:pc= %x\n",(uint32_t)state->pc);
          uint32_t  last_pc=state->pc;
        s->diff_step(1);
        if( bp.judge_eq((uint32_t)state->pc)||p->is_waiting_for_interrupt()){
          printf("emu_cont breakpoint ok:pc= %x,last_pc=%x\n",(uint32_t)state->pc,last_pc);
         if(last_pc!=state->pc) break;  
        }
    }

    
    return ACT_RESUME;
}

static gdb_action_t emu_stepi(void *args)
{
    sim_t *s = (sim_t *) args;

    s->diff_step(1);

    return ACT_RESUME;
}

static bool emu_set_bp(void *args, size_t addr, bp_type_t type)
{
  // printf("Setting breakpoint at 0x%x\n", addr);
    sim_t *s = (sim_t *) args;
    if (type != BP_SOFTWARE || bp.is_full())
        return false;

    bp.add_bp(addr);
    return true;
}

static bool emu_del_bp(void *args, size_t addr, bp_type_t type)
{
  // printf("Deleting breakpoint at 0x%x\n", addr);
    sim_t *s = (sim_t *) args;

    // It's fine when there's no matching breakpoint, just doing nothing
    // printf("Deleting breakpoint at 0x%x\n", addr);
    if (type != BP_SOFTWARE )
        return true;
    // if( !bp.is_full() || !bp.judge_eq(addr))
    //   return true;
    bp.del_bp(addr);
	// bp.print();
    // printf("Deleted breakpoint at 0x%x\n", addr);
    return true;
}

static void emu_on_interrupt(void *args)
{
    struct emu *emu = (struct emu *) args;
    // emu_halt(emu);
}
static    void emu_set_cpu(void *args, int cpuid){
  
}
static    int emu_get_cpu(void *args)
{
  return 0;
}

int mygdbstub(sim_t* s){
	  gdbstub_t gdbstub;
  struct target_ops emu_ops ;
  
    emu_ops.get_reg_bytes = emu_get_reg_bytes;
    emu_ops.read_reg = emu_read_reg;
    emu_ops.write_reg = emu_write_reg;
    emu_ops.read_mem = emu_read_mem;
    emu_ops.write_mem = emu_write_mem;
    emu_ops.cont = emu_cont;
    emu_ops.stepi = emu_stepi;
    emu_ops.set_bp = emu_set_bp;
    emu_ops.del_bp = emu_del_bp;
    emu_ops.on_interrupt = emu_on_interrupt;
    emu_ops.set_cpu = emu_set_cpu;
    emu_ops.get_cpu = emu_get_cpu;
 
    arch_info_t arch_info={
                          .smp = 1,
                          .reg_num = 33,
                          
                      };
    arch_info.target_desc = TARGET_RV32;

      if (!gdbstub_init(&gdbstub, &emu_ops,
                      arch_info,
                      "127.0.0.1:1234")) {
        fprintf(stderr, "Fail to create socket.\n");
        return -1;
    }
    gdbstub_run(&gdbstub,s);
    // s->diff_step(-1);
    // s->diff_get_regs(NULL);
  // }
  gdbstub_close(&gdbstub);
  return 0;
}