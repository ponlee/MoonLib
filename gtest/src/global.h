#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <CppLog.h>

extern CppLog cppLog;
#ifdef CPP_LOG_INSTANCE
#undef CPP_LOG_INSTANCE
#endif
#define CPP_LOG_INSTANCE &cppLog

#endif
