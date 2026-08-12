#include <bits/vt.h>
#include <stdint.h>

#define VT_GETSTATE2 0x56F0

struct vt_stat2 {
	uint64_t v_active;
	uint64_t v_count;
	uint64_t v_state_words;
	uint64_t v_state_ptr;
};
