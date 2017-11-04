#ifndef _COMMON_TYPES_H_
#define _COMMON_TYPES_H_

#include <stdint.h>

typedef uint8_t lcp_element_t;
#define LCP_EL_SIZE sizeof(lcp_element_t)
#define lcp_minus1 ((lcp_element_t)(~0))

typedef uint8_t lcp_out_element_t;
#define LCP_OUT_EL_SIZE sizeof(lcp_out_element_t)

typedef uint32_t idx_element_t;
#define IDX_EL_SIZE sizeof(idx_element_t)


#endif
