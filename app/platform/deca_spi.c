/*! ----------------------------------------------------------------------------
 * @file	deca_spi.c
 * @brief	SPI access functions - 使用我们的SPI驱动
 *
 * @attention
 *
 * Copyright 2015 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */

#include "deca_spi.h"
#include "deca_device_api.h"
#include "bsp_dw1000.h"
#include "bsp_gpio.h"

/****************************************************************************//**
 *
 * 								DW1000 SPI section
 *
 *******************************************************************************/

int openspi(void)
{
	return 0;
}

int closespi(void)
{
	return 0;
}

static volatile uint8_t spi_fast_enabled;
static volatile uint32_t spi_transfer_errors;

int deca_spi_set_fast(uint8 enabled)
{
    if (enabled && (hspi2.State != HAL_SPI_STATE_READY
        || (hspi2.Instance->CR1 & (SPI_CR1_SPE | SPI_CR1_MSTR | SPI_CR1_RXONLY | SPI_CR1_BIDIMODE | SPI_CR1_CRCEN))
            != (SPI_CR1_SPE | SPI_CR1_MSTR)
        || (hspi2.Instance->CR2 & (SPI_CR2_DS | SPI_CR2_FRXTH))
            != (SPI_DATASIZE_8BIT | SPI_CR2_FRXTH)))
        return -1;
    spi_fast_enabled = enabled != 0;
    return 0;
}

uint8 deca_spi_is_fast(void)
{
    return spi_fast_enabled;
}

uint32 deca_spi_error_count(void)
{
    return spi_transfer_errors;
}

/* 与 HAL 的 8 位双线收发使用相同的 DR 访问宽度；每发送一字节就取走 RX，
 * 避免 OVR。预算使用有界轮询，关闭中断期间不依赖 SysTick 超时。 */
__STATIC_FORCEINLINE int spi_exchange_byte(uint8_t tx, uint8_t *rx, uint32_t *budget)
{
    while ((hspi2.Instance->SR & SPI_SR_TXE) == 0)
    {
        if (*budget == 0)
            return -1;
        (*budget)--;
    }
    *(__IO uint8_t *)&hspi2.Instance->DR = tx;
    while ((hspi2.Instance->SR & SPI_SR_RXNE) == 0)
    {
        if (*budget == 0)
            return -1;
        (*budget)--;
    }
    *rx = *(__IO uint8_t *)&hspi2.Instance->DR;
    return (hspi2.Instance->SR & (SPI_SR_OVR | SPI_SR_MODF | SPI_SR_FRE)) ? -1 : 0;
}

static int spi_transfer_fast(uint16_t header_length, const uint8_t *header,
    uint32_t length, const uint8_t *tx, uint8_t *rx)
{
    uint32_t irq_mask = __get_PRIMASK();
    __disable_irq();
    uint32_t budget = SystemCoreClock / 1000U + 1U;
    uint8_t discard;
    int result = -1;
    if ((header_length != 0 && header == NULL) || (length != 0 && tx == NULL && rx == NULL))
        goto done;

    while ((hspi2.Instance->SR & SPI_SR_BSY) != 0)
    {
        if (budget == 0)
            goto done;
        budget--;
    }
    /* 同步清空上次传输残留的 RX FIFO。 */
    while ((hspi2.Instance->SR & SPI_SR_RXNE) != 0)
    {
        discard = *(__IO uint8_t *)&hspi2.Instance->DR;
        if (budget == 0)
            goto done;
        budget--;
    }
    if (hspi2.Instance->SR & SPI_SR_OVR)
    {
        discard = *(__IO uint8_t *)&hspi2.Instance->DR;
        (void)hspi2.Instance->SR;
    }
    if ((hspi2.Instance->SR & (SPI_SR_MODF | SPI_SR_FRE)) != 0)
        goto done;

    DW1000_CS_LOW();
    for (uint16_t i = 0; i < header_length; i++)
        if (spi_exchange_byte(header[i], &discard, &budget) != 0)
            goto done;
    for (uint32_t i = 0; i < length; i++)
    {
        uint8_t received;
        if (spi_exchange_byte(tx != NULL ? tx[i] : 0xFFU, &received, &budget) != 0)
            goto done;
        if (rx != NULL)
            rx[i] = received;
    }
    while ((hspi2.Instance->SR & SPI_SR_BSY) != 0)
    {
        if (budget == 0)
            goto done;
        budget--;
    }
    result = 0;
done:
    DW1000_CS_HIGH();
    if (result != 0)
        spi_transfer_errors++;
    __set_PRIMASK(irq_mask);
    return result;
}

/**
 * @brief  写数据到DW1000 SPI
 * @param  headerLength: 头部长度
 * @param  headerBuffer: 头部数据
 * @param  bodyLength: 数据长度
 * @param  bodyBuffer: 数据内容
 * @retval 0=成功
 */
int writetospi(uint16_t headerLength,
			   const uint8_t *headerBuffer,
			   uint32_t bodyLength,
			   const uint8_t *bodyBuffer)
{
	if (spi_fast_enabled)
		return spi_transfer_fast(headerLength, headerBuffer, bodyLength, bodyBuffer, NULL);
	/* 直接使用SPI总线，保留官方驱动的header格式 */
	DW1000_CS_LOW();
	HAL_StatusTypeDef status = HAL_OK;
	if (headerLength != 0)
		status = HAL_SPI_Transmit(&hspi2, (uint8_t *)headerBuffer, headerLength, 100);
	if (status == HAL_OK && bodyLength != 0)
		status = HAL_SPI_Transmit(&hspi2, (uint8_t *)bodyBuffer, (uint16_t)bodyLength, 500);
	DW1000_CS_HIGH();
	if (status != HAL_OK)
		spi_transfer_errors++;
	return status == HAL_OK ? 0 : -1;
}

/**
 * @brief  从DW1000 SPI读取数据
 * @param  headerLength: 头部长度
 * @param  headerBuffer: 头部数据
 * @param  readlength: 读取长度
 * @param  readBuffer: 读取缓冲区
 * @retval 0=成功
 */
int readfromspi(uint16_t headerLength,
				const uint8_t *headerBuffer,
				uint32_t readlength,
				uint8_t *readBuffer)
{
	if (spi_fast_enabled)
		return spi_transfer_fast(headerLength, headerBuffer, readlength, NULL, readBuffer);
	/* 直接使用SPI总线，保留官方驱动的header格式 */
	DW1000_CS_LOW();
	HAL_StatusTypeDef status = HAL_OK;
	if (headerLength != 0)
		status = HAL_SPI_Transmit(&hspi2, (uint8_t *)headerBuffer, headerLength, 100);
	if (status == HAL_OK && readlength != 0)
		status = HAL_SPI_Receive(&hspi2, readBuffer, (uint16_t)readlength, 500);
	DW1000_CS_HIGH();
	if (status != HAL_OK)
		spi_transfer_errors++;
	return status == HAL_OK ? 0 : -1;
}

/****************************************************************************//**
 *
 * 								END OF DW1000 SPI section
 *
 *******************************************************************************/
