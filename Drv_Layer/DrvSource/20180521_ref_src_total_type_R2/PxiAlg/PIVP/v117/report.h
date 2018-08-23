#ifndef VP_REPORT_H_
#define VP_REPORT_H_

#include <stdint.h>

struct Report {
	uint8_t report_valid_flag;
	uint8_t timeout_flag;
    float regulate_ppgsp;
    float regulate_ppgdp;
    float estimate_ppgsp;
    float estimate_ppgdp;
	int signal_stability;
};

#endif  // VP_REPORT_H_
