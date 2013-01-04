#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_dma.h>
#include "spidma.h"

// enum spiSpeed { SPI_SLOW, SPI_MEDIUM, SPI_FAST };

static const uint16_t speeds[] = {
    [SPI_SLOW] = SPI_BaudRatePrescaler_64,
    [SPI_MEDIUM] = SPI_BaudRatePrescaler_8,
    [SPI_FAST] = SPI_BaudRatePrescaler_2
};

void spiInit(SPI_TypeDef *SPIx) {
    SPI_InitTypeDef SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_StructInit(&GPIO_InitStructure);
    SPI_StructInit(&SPI_InitStructure);

    if (SPIx == SPI2) {
        // Enable clocks
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |
                               RCC_APB2Periph_GPIOC |
                               RCC_APB2Periph_AFIO, ENABLE);
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

        // Initialize GPIOB and GPIOC pins
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
        GPIO_WriteBit(GPIOC, GPIO_Pin_6, 1);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    } else {
        return;
    }

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = speeds[SPI_SLOW];
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPIx, &SPI_InitStructure);
    SPI_Cmd(SPIx, ENABLE);
}

int spiReadWrite(SPI_TypeDef *SPIx, uint8_t *rbuf,
                 const uint8_t *tbuf, int cnt, enum spiSpeed speed) {

    int i;

    SPIx->CR1 = (SPIx->CR1 & ~SPI_BaudRatePrescaler_256) |
                speeds[speed];

    for (i = 0; i < cnt; i++) {
        if (tbuf) {
            SPI_I2S_SendData(SPIx, *tbuf++);
        } else {
            SPI_I2S_SendData(SPIx, 0xff);
        }

        while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);
        if (rbuf) {
            *rbuf++ = SPI_I2S_ReceiveData(SPIx);
        } else {
            SPI_I2S_ReceiveData(SPIx);
        }
    }
    return i;
}

int spiReadWrite16(SPI_TypeDef *SPIx, uint16_t *rbuf,
                   const uint16_t *tbuf, int cnt, enum spiSpeed speed) {

    SPI_DataSizeConfig(SPIx, SPI_DataSize_16b);

    int i;

    SPIx->CR1 = (SPIx->CR1 & ~SPI_BaudRatePrescaler_256) |
                speeds[speed];

    for (i = 0; i < cnt; i++) {
        if (tbuf) {
            SPI_I2S_SendData(SPIx, *tbuf++);
        } else {
            SPI_I2S_SendData(SPIx, 0xff);
        }

        while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);
        if (rbuf) {
            *rbuf++ = SPI_I2S_ReceiveData(SPIx);
        } else {
            SPI_I2S_ReceiveData(SPIx);
        }
    }

    SPI_DataSizeConfig(SPIx, SPI_DataSize_8b);
    return i;
}

int xchng_datablock(SPI_TypeDef *SPIx, int half, void *tbuf,
                    void *rbuf, unsigned count) {
    if (count < 4) {
        if (half == 8)
            spiReadWrite(SPIx, rbuf, tbuf, count, SPI_FAST);
        else if (half == 16)
            spiReadWrite16(SPIx, rbuf, tbuf, count, SPI_FAST);
    } else
        dmaExgBytes(rbuf, tbuf, half, count);
}

/*! start !*/
int dmaExgBytes(void *rbuf, void *tbuf, int half, unsigned count) {
    DMA_InitTypeDef DMA_InitStructure;
    uint16_t dummy[] = {0xffff};

    DMA_DeInit(DMA1_Channel4);
    DMA_DeInit(DMA1_Channel5);

    // Common to both channels

    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(SPI2->DR));
    if (half == 8) {
        SPI_DataSizeConfig(SPI2, SPI_DataSize_8b);
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    } else {
        SPI_DataSizeConfig(SPI2, SPI_DataSize_16b);
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    }
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_BufferSize = count;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    // Rx Channel

    if (rbuf) {
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)rbuf;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    } else {
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dummy;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    }
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_Init(DMA1_Channel4, &DMA_InitStructure);

    // Tx channel

    if (tbuf) {
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)tbuf;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    } else {
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dummy;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    }
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);

    /*! enable !*/

    // Enable channels

    DMA_Cmd(DMA1_Channel4, ENABLE);
    DMA_Cmd(DMA1_Channel5, ENABLE);

    // Enable SPI TX/RX request

    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);

    // Wait for completion

    while (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);

    // Disable channels

    DMA_Cmd(DMA1_Channel4, DISABLE);
    DMA_Cmd(DMA1_Channel5, DISABLE);

    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, DISABLE);

    SPI_DataSizeConfig(SPI2, SPI_DataSize_8b);

    return count;
}
/*! stop !*/
