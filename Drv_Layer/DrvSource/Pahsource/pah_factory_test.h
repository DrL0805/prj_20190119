#ifndef __FACTORY_TEST_H__
#define __FACTORY_TEST_H__

#include <stdbool.h>
#include <stdint.h>
#include "pah8011es_user.h"

void HrmFactoryTest(uint16 ui16lightleak[3]);

void factory_test_mode(factory_test_e test_type,bool expo_en,uint8 expo_ch_b,uint8 expo_ch_c);

#endif
