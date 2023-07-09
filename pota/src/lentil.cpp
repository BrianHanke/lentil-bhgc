#include <ai.h>

AtMutex l_critsec;
inline void lentil_crit_sec_enter();
inline void lentil_crit_sec_leave();