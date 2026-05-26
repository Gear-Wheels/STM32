#ifndef ___AD_
#define ___AD_

extern uint16_t AD_Value[4];

void AD_Init(void);
uint16_t AD_GetValue_Average(uint8_t times, uint8_t ADC_Channel);

#endif
