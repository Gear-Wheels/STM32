#include "stm32f10x.h"                  // Device header

void AD_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);	// 72MHz / 6 = 12MHz
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
		
	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	
	ADC_Cmd(ADC1, ENABLE);
	
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) == SET);
}

uint16_t AD_GetValue(uint8_t ADC_Channel)
{
	ADC_RegularChannelConfig(ADC1, ADC_Channel, 1, ADC_SampleTime_55Cycles5);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	return ADC_GetConversionValue(ADC1);
}

/**
 * @brief 多次采样ADC通道0并返回平均值（软件滤波）
 * @param times 采样次数（例如：10 表示连续采样10次）
 * @param ADC_Channel 需要转换的通道（例如：1,2,3等通道）
 * @return 平均值（12位ADC结果，范围0~4095）
 * @note 采用阻塞式轮询，会占用CPU直到采样完成
 */
uint16_t AD_GetValue_Average(uint8_t times, uint8_t ADC_Channel)
{
    uint32_t sum = 0;               // 32位累加器，防止溢出（12位ADC最大4095，累加100次可达409500）
	ADC_RegularChannelConfig(ADC1, ADC_Channel, 1, ADC_SampleTime_55Cycles5);    
    for (uint8_t i = 0; i < times; i++)   // 循环 times 次，i 从 0 开始，条件用 < 保证恰好循环 times 次
    {
        // 软件触发ADC1开始一次转换（因为初始化时设置为单次转换模式）
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);
        
        // 轮询等待转换完成：检查EOC（End Of Conversion）标志位，为RESET则继续等待
        while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
        
        // 读取本次转换结果（ADC数据寄存器中的12位右对齐值），并累加到sum中
        // sum += x 等价于 sum = sum + x
        sum += ADC_GetConversionValue(ADC1);
    }
    
    // 返回平均值：总和 / 采样次数，结果强制转换为16位（平均值不会超过4095）
    return (uint16_t)(sum / times);
}
