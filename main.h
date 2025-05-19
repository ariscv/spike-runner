#ifndef _MAIN_H_
#define _MAIN_H_

#include "sim.h"

#define Assert(cond, format, ...) \
  do { \
    if (!(cond)) { \
        (fflush(stdout), fprintf(stderr, format "\n", ##  __VA_ARGS__)); \
      assert(cond); \
    } \
  } while (0)

#define CONFIG_MSIZE (2048 << 20)
int mygdbstub(sim_t* s);

#endif /* _MAIN_H_ */