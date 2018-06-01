#ifndef GBBOII_UTILS_H
#define GBBOII_UTILS_H

#if defined(DEBUG)

  #include <stdio.h>
  #define debug_print(fmt, args...) printf("DEBUG: %s:%d:%s(): " fmt, \
      __FILE__, __LINE__, __func__, ##args)

  #define debug_print_q(fmt, args...) printf(fmt, ##args)

#else

  #define debug_print(fmt, args...)
  #define debug_print_q(fmt, args...)

#endif

#endif
