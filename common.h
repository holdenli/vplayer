// A fix for some issue with C macros and the ffmpeg headers
#ifdef __cplusplus

#define __STDC_CONSTANT_MACROS

#ifdef _STDINT_H

#undef _STDINT_H

#endif

#include <stdint.h>

#endif
