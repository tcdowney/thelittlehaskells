#include <stm32f10x.h>
#include <stm32f10x_dac.h>
#include <stm32f10x_dma.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_rcc.h>
#include "audiodma.h"
#include <misc.h>
#include <tone.h>

uint8_t Audiobuf[];
int audioplayerHalf;
int audioplayerWhole;

#define A440LEN (sizeof(a440)/sizeof(uint16_t))
uint16_t outbuf[A440LEN];

GPIO_InitTypeDef           GPIO_InitStructure;
DAC_InitTypeDef            DAC_InitStructure;
DMA_InitTypeDef            DMA_InitStructure;
TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
NVIC_InitTypeDef           NVIC_InitStructure;

void audioplayerStart() {
  DMA_Cmd(DMA1_Channel3, ENABLE);
  DAC_Cmd(DAC_Channel_1, ENABLE);
  DAC_DMACmd(DAC_Channel_1, ENABLE);
  TIM_Cmd(TIM3, ENABLE);
}

void audioplayerStop() {
  DMA_Cmd(DMA1_Channel3, DISABLE);
  DAC_Cmd(DAC_Channel_1, DISABLE);
  DAC_DMACmd(DAC_Channel_1, DISABLE);
  TIM_Cmd(TIM3, DISABLE);
}

void audioplayerInit(unsigned int sampleRate)
{
  int i;
  for (i = 0; i < A440LEN; i++)
    outbuf[i] = a440[i];

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC | RCC_APB1Periph_TIM3, ENABLE);

  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* 24,000,000/44100 =~ 544 */

  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
  TIM_TimeBaseStructure.TIM_Period = 24000000 / sampleRate;          
  TIM_TimeBaseStructure.TIM_Prescaler = 0x0;       
  TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  /* TIM3 TRGO selection */

  TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);

  /* DAC channel1 Configuration */

  DAC_StructInit(&DAC_InitStructure);
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_T3_TRGO;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
  DAC_Init(DAC_Channel_1, &DAC_InitStructure);

  /*! DMAInitstart !*/
  // DMA Channel 3 Configuration 

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  DMA_DeInit(DMA1_Channel3);

  DMA_StructInit(&DMA_InitStructure);
  DMA_InitStructure.DMA_PeripheralBaseAddr = &DAC->DHR12R1;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_BufferSize = AUDIOBUFSIZE;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) Audiobuf; 
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

  DMA_Init(DMA1_Channel3, &DMA_InitStructure);

  //  Enable Interrupts for complete and half transfers

  DMA_ITConfig(DMA1_Channel3, DMA_IT_TC | DMA_IT_HT , ENABLE);

  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;   //HIGHEST_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  /*! DMAInitend !*/
}

/*! DMAHandlerstart !*/
int completed; // count cycles

void DMA1_Channel3_IRQHandler(void)
{
  int i;
  if (DMA_GetITStatus(DMA1_IT_TC3)){      // Transfer complete
    for (i = A440LEN/2; i < A440LEN; i++)
      outbuf[i] = a440[i];
    completed++;
    DMA_ClearITPendingBit(DMA1_IT_TC3);
  }
  else if (DMA_GetITStatus(DMA1_IT_HT3)){ // Half Transfer complete
    for (i = 0; i < A440LEN/2; i++)
      outbuf[i] = a440[i];
    DMA_ClearITPendingBit(DMA1_IT_HT3);
  }
}
/*! DMAHandlerstop !*/

