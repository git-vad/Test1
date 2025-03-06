/**
 * @file Generator.cс
 * @author vladimir_ad@mail.ru
 * @brief Файл содержит реализацию Генератора
 * @version 0.1
 * @date 2025-03-05 
 */

#include <main.h>
#include "Generator.h"
#include "GeneratorUnit.h"

/** 
 * Структура для работы с таймером
 */ 
TIM_HandleTypeDef tim;
TIM_OC_InitTypeDef sConfigOC = {0};

TGenerator Generator1(2250000, 
    [](){ __HAL_TIM_SET_COUNTER(&tim, 0); HAL_TIM_Base_Start_IT(&tim); HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);},
    [](){   HAL_TIM_Base_Stop_IT(&tim); HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);},
    [](){ __HAL_TIM_DISABLE_IT(&tim, TIM_IT_CC1);},
    [](){ __HAL_TIM_ENABLE_IT(&tim, TIM_IT_CC1);}
);

// Интерфейс для работы с контроллером движения.
generator::IGenerator *pGenerator;

void GeneratorInit(void)
{
  pGenerator = &Generator1;
    
  // Настраиваем таймер.
  // Таймер должен непрерывно считать от 0 до 0xffff. При частоте синхронизации 72МГц
  // приемлемое значение прескалера 32. 
  tim.Instance = TIM2;
  tim.Init.Prescaler = 31;
  tim.Init.CounterMode = TIM_COUNTERMODE_UP;
  tim.Init.Period = 0xFFFF;
  tim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  tim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_Base_Init(&tim);
       
  sConfigOC.OCMode = TIM_OCMODE_TOGGLE;  // Режим переключения на совпадении
  sConfigOC.Pulse = 0;  // Устанавливаем начальное значение
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_OC_ConfigChannel(&tim, &sConfigOC, TIM_CHANNEL_1);
  
  // Настраиваем GPIO
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/** @details Обработчик прерывания от таймера.
 * Переключат заданный пин в противоположное состояние.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  
  if (htim->Instance == TIM2)
  {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8);
    // Извлекаем значение следующего интервала для отчета.
    uint16_t next;
    if(Generator1.getNextCCR(next) == generator::RESULT::OK)
    {
      __HAL_TIM_SET_COMPARE(&tim, TIM_CHANNEL_1, next);    
    }
    else
    {
      HAL_TIM_Base_Stop_IT(&tim);
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    }
    
  }
}