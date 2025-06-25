#include "main.h"

#if BOARD_VERSION == Version3_75
void MX_GPIO_Init(void)
{
  
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, RESERVED_Pin|EN_5V_Pin|EN_3P8V_Pin|ON_N25_Pin
                          |COL_B4_Pin|SPI2_CS_ADC_Pin|One_Wire_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, UART4_WU_Pin|ON_OWEN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, COL_B3_Pin|COL_B2_Pin|COL_B1_Pin|ON_DISP_Pin
                          |ON_RS_Pin|GPIO_PIN_4|SPI2_CS_ROM_Pin|ON_ROM_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : RESERVED_Pin EN_5V_Pin EN_3P3V_Pin ON_N25_Pin
                           COL_B4_Pin SPI2_CS_ADC_Pin One_Wire_Pin */
  GPIO_InitStruct.Pin = RESERVED_Pin|EN_5V_Pin|EN_3P8V_Pin|ON_N25_Pin
                          |COL_B4_Pin|SPI2_CS_ADC_Pin|One_Wire_Pin|EN_3P3V_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;  // –û—Ç–∫–ª—é—á–µ–Ω–∏–µ –ø–æ–¥—Ç—è–∂–µ–∫
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  /*Configure GPIO pins : UART4_WU_Pin ON_OWEN_Pin */
  GPIO_InitStruct.Pin = UART4_WU_Pin|ON_OWEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : NUM_RES_Pin USART1_DATA_DETECT_Pin */
  GPIO_InitStruct.Pin = NUM_RES_Pin|USART1_DATA_DETECT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : STR_B1_Pin STR_B2_Pin STR_B3_Pin */
  GPIO_InitStruct.Pin = STR_B1_Pin|STR_B2_Pin|STR_B3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : STR_B4_Pin */
  GPIO_InitStruct.Pin = STR_B4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(STR_B4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : COL_B3_Pin COL_B2_Pin COL_B1_Pin ON_DISP_Pin
                           ON_RS_Pin PB4 ON_ROM_Pin */
  GPIO_InitStruct.Pin = COL_B3_Pin|COL_B2_Pin|COL_B1_Pin|ON_DISP_Pin
                          |ON_RS_Pin|GPIO_PIN_4|ON_ROM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SDMMC1_DET_Pin */
  GPIO_InitStruct.Pin = SDMMC1_DET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SDMMC1_DET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI2_CS_ROM_Pin */
  GPIO_InitStruct.Pin = SPI2_CS_ROM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SPI2_CS_ROM_GPIO_Port, &GPIO_InitStruct);

  /**/
  __HAL_SYSCFG_FASTMODEPLUS_ENABLE(SYSCFG_FASTMODEPLUS_PB6);
}


//!  ŒÕ‘»Õ”–¿÷»ﬂ GPIO ƒÀﬂ œÀ¿“€ 3.79

#elif BOARD_VERSION == Version3_79
void MX_GPIO_Init(void)
{
  
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, RESERVED_Pin|EN_5V_Pin|EN_3P8V_Pin|ON_N25_Pin
                          |COL_B4_Pin|SPI2_CS_ADC_Pin|One_Wire_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, UART4_WU_Pin|ON_OWEN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, COL_B3_Pin|COL_B2_Pin|COL_B1_Pin|ON_DISP_Pin
                          |ON_RS_Pin|GPIO_PIN_4|SPI2_CS_ROM_Pin|ON_ROM_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : RESERVED_Pin EN_5V_Pin EN_3P3V_Pin ON_N25_Pin
                           COL_B4_Pin SPI2_CS_ADC_Pin One_Wire_Pin */
  GPIO_InitStruct.Pin = RESERVED_Pin|EN_5V_Pin|EN_3P8V_Pin|ON_N25_Pin
                          |COL_B4_Pin|SPI2_CS_ADC_Pin|One_Wire_Pin|EN_3P3V_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;  // –û—Ç–∫–ª—é—á–µ–Ω–∏–µ –ø–æ–¥—Ç—è–∂–µ–∫
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  /*Configure GPIO pins : UART4_WU_Pin ON_OWEN_Pin */
  GPIO_InitStruct.Pin = UART4_WU_Pin|ON_OWEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : NUM_RES_Pin USART1_DATA_DETECT_Pin */
  GPIO_InitStruct.Pin = NUM_RES_Pin|USART1_DATA_DETECT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : STR_B1_Pin STR_B2_Pin STR_B3_Pin */
  GPIO_InitStruct.Pin = STR_B1_Pin|STR_B2_Pin|STR_B3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : STR_B4_Pin */
  GPIO_InitStruct.Pin = STR_B4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(STR_B4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : COL_B3_Pin COL_B2_Pin COL_B1_Pin ON_DISP_Pin
                           ON_RS_Pin PB4 ON_ROM_Pin */
  GPIO_InitStruct.Pin = COL_B3_Pin|COL_B2_Pin|COL_B1_Pin|ON_DISP_Pin
                          |ON_RS_Pin|GPIO_PIN_4|ON_ROM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SDMMC1_DET_Pin */
  GPIO_InitStruct.Pin = SDMMC1_DET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SDMMC1_DET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI2_CS_ROM_Pin */
  GPIO_InitStruct.Pin = SPI2_CS_ROM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SPI2_CS_ROM_GPIO_Port, &GPIO_InitStruct);

  /**/
  __HAL_SYSCFG_FASTMODEPLUS_ENABLE(SYSCFG_FASTMODEPLUS_PB6);
}

#else
  #error "ÕÂ Á‡‰‡Ì‡ ËÎË ÌÂ‚ÂÌ‡ BOARD_VERSION"
#endif