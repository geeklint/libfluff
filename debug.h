#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef DEBUG

#include <stdio.h>
#define LOGD(s) printf("DEBUG %s\n", s);

#else

#define LOGD(s)

#endif

#endif /* DEBUG_H_ */
