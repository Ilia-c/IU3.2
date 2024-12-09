/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    bsp_driver_sd.c 
 * @brief   This file includes a generic uSD card driver in blocking mode.
 *          DMA and related callbacks have been disabled.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

#include "bsp_driver_sd.h"

extern SD_HandleTypeDef hsd1;

/**
  * @brief  Initializes the SD card device in blocking mode.
  * @retval SD status
  */
__weak uint8_t BSP_SD_Init(void)
{
  uint8_t sd_state = MSD_OK;

  /* Check if the SD card is plugged in the slot */
  if (BSP_SD_IsDetected() != SD_PRESENT)
  {
    return MSD_ERROR_SD_NOT_PRESENT;
  }

  /* HAL SD initialization */
  sd_state = HAL_SD_Init(&hsd1);

  /* Если вы хотите сразу перейти в 4-битный режим (опционально): */
  /*
  if (sd_state == MSD_OK)
  {
    if (HAL_SD_ConfigWideBusOperation(&hsd1, SDMMC_BUS_WIDE_4B) != HAL_OK)
    {
      sd_state = MSD_ERROR;
    }
  }
  */

  return sd_state;
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in polling/blocking mode.
  * @param  pData: Pointer to the buffer that will contain the data to receive
  * @param  ReadAddr: Address from where data is to be read (in block units)
  * @param  NumOfBlocks: Number of SD blocks to read
  * @param  Timeout: Timeout for read operation (in ms)
  * @retval SD status
  */
__weak uint8_t BSP_SD_ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
  uint8_t sd_state = MSD_OK;

  if (HAL_SD_ReadBlocks(&hsd1, (uint8_t *)pData, ReadAddr, NumOfBlocks, Timeout) != HAL_OK)
  {
    sd_state = MSD_ERROR;
  }

  return sd_state;
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in polling/blocking mode.
  * @param  pData: Pointer to the buffer that contains the data to write
  * @param  WriteAddr: Address where data is to be written (in block units)
  * @param  NumOfBlocks: Number of SD blocks to write
  * @param  Timeout: Timeout for write operation (in ms)
  * @retval SD status
  */
__weak uint8_t BSP_SD_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
  uint8_t sd_state = MSD_OK;

  if (HAL_SD_WriteBlocks(&hsd1, (uint8_t *)pData, WriteAddr, NumOfBlocks, Timeout) != HAL_OK)
  {
    sd_state = MSD_ERROR;
  }

  return sd_state;
}

/* Отключаем DMA-функции или просто оставляем их возвращать ошибку, 
   чтобы не использовать DMA режим. */

/*
__weak uint8_t BSP_SD_ReadBlocks_DMA(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks)
{
  // DMA не используется, можно возвращать ошибку или не определять
  return MSD_ERROR;
}

__weak uint8_t BSP_SD_WriteBlocks_DMA(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks)
{
  // DMA не используется, можно возвращать ошибку или не определять
  return MSD_ERROR;
}
*/

/**
  * @brief  Erases the specified memory area of the given SD card.
  * @param  StartAddr: Start block address
  * @param  EndAddr: End block address
  * @retval SD status
  */
__weak uint8_t BSP_SD_Erase(uint32_t StartAddr, uint32_t EndAddr)
{
  uint8_t sd_state = MSD_OK;

  if (HAL_SD_Erase(&hsd1, StartAddr, EndAddr) != HAL_OK)
  {
    sd_state = MSD_ERROR;
  }

  return sd_state;
}

/**
  * @brief  Gets the current SD card data state.
  * @retval SD_TRANSFER_OK if no transfer is ongoing, SD_TRANSFER_BUSY otherwise
  */
__weak uint8_t BSP_SD_GetCardState(void)
{
  return ((HAL_SD_GetCardState(&hsd1) == HAL_SD_CARD_TRANSFER ) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}

/**
  * @brief  Get SD information about specific SD card.
  * @param  CardInfo: Pointer to HAL_SD_CardInfoTypedef structure
  * @retval None
  */
__weak void BSP_SD_GetCardInfo(BSP_SD_CardInfo *CardInfo)
{
  HAL_SD_GetCardInfo(&hsd1, CardInfo);
}

/* Колбэки, связанные с DMA не будут вызываться, можно оставить их пустыми: */
__weak void BSP_SD_AbortCallback(void)
{
}

__weak void BSP_SD_WriteCpltCallback(void)
{
}

__weak void BSP_SD_ReadCpltCallback(void)
{
}

/**
 * @brief  Detects if SD card is correctly plugged in the memory slot or not.
 * @retval SD_PRESENT if present, SD_NOT_PRESENT otherwise
 */
__weak uint8_t BSP_SD_IsDetected(void)
{
  // Здесь должна быть реализация определения присутствия карты
  // Если нет такой логики, всегда возвращаем SD_PRESENT:
  return SD_PRESENT;
}
