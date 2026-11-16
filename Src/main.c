/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c  LAB6
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_host.h"
#include "seg7.h"
#include <stdint.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */



/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

I2S_HandleTypeDef hi2s3;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim7;

/* USER CODE BEGIN PV */
int DelayValue = 50;

/*
 * Battleship board storage.
 *
 * Vertical map: 2 rows x 16 columns.
 *   Bits 31..16 = top row, columns 0..15.
 *   Bits 15..0  = bottom row, columns 0..15.
 *
 * Horizontal map: 3 rows x 8 columns.
 *   Bits  7..0  = row 0, columns 0..7.
 *   Bits 15..8  = row 1, columns 0..7.
 *   Bits 23..16 = row 2, columns 0..7.
 *   Bits 31..24 are unused.
 *
 * A set bit (1) means that a boat occupies that cell.
 */
volatile uint32_t Player1_Vertical_Map = 0;
volatile uint32_t Player1_Horizontal_Map = 0;
volatile uint32_t Player2_Vertical_Map = 0;
volatile uint32_t Player2_Horizontal_Map = 0;

/* Placement state: Player 1 places first, then Player 2. */
volatile uint8_t Current_Player = 1;
volatile uint8_t Boats_Placed = 0;       /* 0..4 for each player */
volatile uint8_t Placement_Complete = 0;

/* Cursor/map state. */
volatile uint8_t Cursor_X = 0;
volatile uint8_t Cursor_Y = 0;
volatile uint8_t Selected_Map = 0;       /* 0 = vertical, 1 = horizontal */
volatile uint8_t Cursor_Blink_On = 1;    /* For the board-display renderer */
volatile uint8_t Last_Placement_Succeeded = 0;
volatile uint8_t Last_Placement_Failed = 0;

/* Potentiometers: PA0 = left/right, PA1 = up/down. */
#define POT_LEFT_RIGHT_CHANNEL  0U
#define POT_UP_DOWN_CHANNEL     1U

/* Switch 6 = PC10 map selection; Switch 7 = PC11 placement confirmation.
 * This implementation assumes the switches pull the input LOW when active.
 */
#define MAP_SELECT_PIN          (1U << 10)
#define PLACE_BUTTON_PIN        (1U << 11)
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2S3_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM7_Init(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
static void Battleship_ADC_Init(void);
static uint16_t Battleship_ADC_Read(uint8_t channel);
static void Battleship_Update_Cursor(void);
static void Battleship_Render_LEDs(void);
static uint8_t Battleship_Try_Place_Boat(void);
static uint8_t Battleship_Is_Button_Pressed(void);
//void Play_Note(int note,int size,int tempo,int space);
//extern void Seven_Segment_Digit (unsigned char digit, unsigned char hex_char, unsigned char dot);
//extern void Seven_Segment(unsigned int HexValue);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

char ramp = 0;
char RED_BRT = 0;
char GREEN_BRT = 0;
char BLUE_BRT = 0;
char RED_STEP = 1;
char GREEN_STEP = 2;
char BLUE_STEP = 3;
char DIM_Enable = 0;
char Music_ON = 0;
int TONE = 0;
int COUNT = 0;
int INDEX = 0;
int Note = 0;
int Save_Note = 0;
int Vibrato_Depth = 1;
int Vibrato_Rate = 40;
int Vibrato_Count = 0;
char Animate_On = 1;
char Message_Length = 0;
char *Message_Pointer;
char *Save_Pointer;
int Delay_msec = 0;
int Delay_counter = 0;

int CRC_Tx = 0xd37e11c9;
int CRC_Rx = 0;

/* HELLO ECE-330L */
/* BATTLESHIP title message */
/* BATTLESHIP title marquee */
char Message[] =
{
    /* Leading spaces: lets BATTLESHIP scroll in from the right */
    SPACE, SPACE, SPACE, SPACE,
    SPACE, SPACE, SPACE, SPACE,

    /* Game title */
    CHAR_B, CHAR_A, CHAR_T, CHAR_T, CHAR_L,
    CHAR_E, CHAR_S, CHAR_H, CHAR_I, CHAR_P,

    /* Trailing spaces: lets BATTLESHIP scroll off the left */
    SPACE, SPACE, SPACE, SPACE,
    SPACE, SPACE, SPACE, SPACE
};
/* Declare array for Song */
Music Song[100];


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2S3_Init();
  MX_SPI1_Init();
  MX_USB_HOST_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */

  /*** Configure GPIOs ***/
  GPIOD->MODER = 0x55555555; // set all Port D pins to outputs
  GPIOA->MODER |= 0x000000FF; // Port A mode register - make A0 to A3 analog pins
  GPIOE->MODER |= 0x55555555; // Port E mode register - make E0 to E15 outputs
  GPIOC->MODER |= 0x0; // Port C mode register - all inputs
  GPIOE->ODR = 0xFFFF; // Set all Port E pins high

  /*** Initialize Battleship title marquee ***/
  Save_Pointer = Message;
  Message_Pointer = Message;

  Message_Length = sizeof(Message);

  Delay_msec = 200;
  Delay_counter = 0;

  /*** Configure the placement controls ***/
  /* PC10 = Switch 6 (map select), PC11 = Switch 7 (place). */
  GPIOC->MODER &= ~((3U << (10U * 2U)) | (3U << (11U * 2U)));
  GPIOC->PUPDR &= ~((3U << (10U * 2U)) | (3U << (11U * 2U)));
  GPIOC->PUPDR |=  ((1U << (10U * 2U)) | (1U << (11U * 2U))); /* Pull-ups */

  /*** Configure ADC1 for the two potentiometers on PA0 and PA1 ***/
  Battleship_ADC_Init();

  /* Start with the placement cursor at the first cell. */
  Cursor_X = 0;
  Cursor_Y = 0;
  Selected_Map = 0;
  Cursor_Blink_On = 1;
  Last_Placement_Succeeded = 0;
  Last_Placement_Failed = 0;

  /*** Turn on CRC Clock in AHB1ENR to enable CRC hardware ***/


  /*****************************************************************************************************
  These commands are handled as part of the MX_TIM7_Init() function and don't need to be enabled
  RCC->AHB1ENR |= 1<<5; // Enable clock for timer 7
  __enable_irq(); // Enable interrupts
  NVIC_EnableIRQ(TIM7_IRQn); // Enable Timer 7 Interrupt in the NVIC controller
  *******************************************************************************************************/

  TIM7->PSC = 199; //250Khz timer clock prescaler value, 250Khz = 50Mhz / 200
  TIM7->ARR = 1; // Count to 1 then generate interrupt (divide by 2), 125Khz interrupt rate to increment byte counter for 78Hz PWM
  TIM7->DIER |= 1; // Enable timer 7 interrupt
  TIM7->CR1 |= 1; // Enable timer counting

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  /* Track the current physical state so a held button only places once. */
  uint8_t previous_place_pressed = Battleship_Is_Button_Pressed();
  uint32_t last_confirm_time = 0;
  uint32_t last_cursor_update = 0;
  uint32_t last_blink_update = HAL_GetTick();

  while (1)
  {
    /* USER CODE END WHILE */
    MX_USB_HOST_Process();

    /* Switch 6 selects which board the potentiometers control.
     * LOW = vertical map; HIGH = horizontal map.
     */
    if ((GPIOC->IDR & MAP_SELECT_PIN) == 0U)
      Selected_Map = 0;
    else
      Selected_Map = 1;

    uint32_t now = HAL_GetTick();

    /* Read the two potentiometers often enough for smooth cursor movement. */
    if (!Placement_Complete && (now - last_cursor_update >= 25U))
    {
      Battleship_Update_Cursor();
      last_cursor_update = now;
    }

    /* Blink the cursor on the 2-row x 16-column peripheral LED display. */
    if (now - last_blink_update >= 250U)
    {
      Cursor_Blink_On = (uint8_t)!Cursor_Blink_On;
      last_blink_update = now;
    }

    Battleship_Render_LEDs();

    /* Switch 7 confirms the currently selected boat location.
     * A rising edge (released -> pressed) prevents repeated placement
     * while the button is held down.
     */
    uint8_t place_pressed = Battleship_Is_Button_Pressed();

    if (!Placement_Complete &&
        place_pressed &&
        !previous_place_pressed &&
        (now - last_confirm_time >= 180U))
    {
      if (Battleship_Try_Place_Boat())
      {
        Last_Placement_Succeeded = 1;
        Last_Placement_Failed = 0;
      }
      else
      {
        Last_Placement_Succeeded = 0;
        Last_Placement_Failed = 1;
      }

      last_confirm_time = now;
    }

    previous_place_pressed = place_pressed;

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2S3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S3_Init(void)
{

  /* USER CODE BEGIN I2S3_Init 0 */

  /* USER CODE END I2S3_Init 0 */

  /* USER CODE BEGIN I2S3_Init 1 */

  /* USER CODE END I2S3_Init 1 */
  hi2s3.Instance = SPI3;
  hi2s3.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_96K;
  hi2s3.Init.CPOL = I2S_CPOL_LOW;
  hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s3.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S3_Init 2 */

  /* USER CODE END I2S3_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 0;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 65535;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_I2C_SPI_GPIO_Port, CS_I2C_SPI_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LD4_Pin|LD3_Pin|LD5_Pin|LD6_Pin
                          |Audio_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : CS_I2C_SPI_Pin */
  GPIO_InitStruct.Pin = CS_I2C_SPI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS_I2C_SPI_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = OTG_FS_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PDM_OUT_Pin */
  GPIO_InitStruct.Pin = PDM_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(PDM_OUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BOOT1_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CLK_IN_Pin */
  GPIO_InitStruct.Pin = CLK_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(CLK_IN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD4_Pin LD3_Pin LD5_Pin LD6_Pin
                           Audio_RST_Pin */
  GPIO_InitStruct.Pin = LD4_Pin|LD3_Pin|LD5_Pin|LD6_Pin
                          |Audio_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MEMS_INT2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
 * @brief Configure ADC1 for single conversions on PA0 (ADC channel 0)
 *        and PA1 (ADC channel 1).
 */
static void Battleship_ADC_Init(void)
{
  /* ADC1 clock and ADC common clock prescaler (PCLK2 / 4). */
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
  ADC->CCR = (ADC->CCR & ~(3U << 16)) | (1U << 16);

  /* PA0 and PA1 are analog inputs. The existing project also configures
   * PA2 and PA3 as analog, so preserve that existing configuration.
   */
  GPIOA->MODER |= 0x000000FFU;
  GPIOA->PUPDR &= ~0x0000000FU;

  /* 84-cycle sampling time for channels 0 and 1. */
  ADC1->CR1 = 0;
  ADC1->CR2 = 0;
  ADC1->SMPR2 &= ~((7U << 0) | (7U << 3));
  ADC1->SMPR2 |=  ((4U << 0) | (4U << 3));

  /* One conversion in the regular sequence; right aligned, 12-bit. */
  ADC1->SQR1 &= ~(0xFU << 20);
  ADC1->CR2 |= ADC_CR2_ADON;
}

/**
 * @brief Read one ADC channel and return a 12-bit result (0..4095).
 */
static uint16_t Battleship_ADC_Read(uint8_t channel)
{
  volatile uint32_t discard;

  /* Clear any previous result and select the requested regular channel. */
  discard = ADC1->DR;
  (void)discard;
  ADC1->SQR3 = (uint32_t)(channel & 0x1FU);

  /* Start a software-triggered conversion and wait for completion. */
  ADC1->CR2 |= ADC_CR2_SWSTART;
  while ((ADC1->SR & ADC_SR_EOC) == 0U)
  {
    /* Wait for ADC conversion. */
  }

  return (uint16_t)(ADC1->DR & 0x0FFFU);
}

/**
 * @brief Convert the potentiometer positions into board coordinates.
 */
static void Battleship_Update_Cursor(void)
{
  uint16_t horizontal_value = Battleship_ADC_Read(POT_LEFT_RIGHT_CHANNEL);
  uint16_t vertical_value = Battleship_ADC_Read(POT_UP_DOWN_CHANNEL);

  uint8_t width;
  uint8_t height;

  if (Selected_Map == 0U)
  {
    width = 16U;
    height = 2U;
  }
  else
  {
    width = 8U;
    height = 3U;
  }

  Cursor_X = (uint8_t)(((uint32_t)horizontal_value * width) / 4096U);
  Cursor_Y = (uint8_t)(((uint32_t)vertical_value * height) / 4096U);

  /* Defensive clamp in case of an unexpected ADC value. */
  if (Cursor_X >= width)  Cursor_X = (uint8_t)(width - 1U);
  if (Cursor_Y >= height) Cursor_Y = (uint8_t)(height - 1U);
}

/**
 * @brief Render the selected player's map and blinking cursor on the
 *        peripheral board's 2-row x 16-column LED display.
 *
 * Assumed wiring: GPIOD[15:0] drives the top LED row and GPIOE[15:0]
 * drives the bottom LED row. LEDs are assumed active-high.
 *
 * The vertical map uses its natural 2x16 layout. The horizontal map's
 * 3x8 cells are flattened into LED positions 0..23: the first 16 cells
 * occupy the top LED row, and the remaining 8 occupy the first 8 LEDs
 * of the bottom row.
 */
static void Battleship_Render_LEDs(void)
{
  uint32_t vertical_map;
  uint32_t horizontal_map;
  uint16_t led_top = 0U;
  uint16_t led_bottom = 0U;

  if (Current_Player == 1U)
  {
    vertical_map = Player1_Vertical_Map;
    horizontal_map = Player1_Horizontal_Map;
  }
  else
  {
    vertical_map = Player2_Vertical_Map;
    horizontal_map = Player2_Horizontal_Map;
  }

  if (Selected_Map == 0U)
  {
    led_top = (uint16_t)((vertical_map >> 16) & 0xFFFFU);
    led_bottom = (uint16_t)(vertical_map & 0xFFFFU);

    if (Cursor_Blink_On && Cursor_X < 16U && Cursor_Y < 2U)
    {
      uint16_t cursor_bit = (uint16_t)(1U << Cursor_X);
      if (Cursor_Y == 0U)
        led_top ^= cursor_bit;
      else
        led_bottom ^= cursor_bit;
    }
  }
  else
  {
    /* Flatten the 3x8 logical map into 24 consecutive LED positions. */
    for (uint8_t cell = 0U; cell < 24U; cell++)
    {
      if ((horizontal_map & (1UL << cell)) != 0U)
      {
        if (cell < 16U)
          led_top |= (uint16_t)(1U << cell);
        else
          led_bottom |= (uint16_t)(1U << (cell - 16U));
      }
    }

    if (Cursor_Blink_On && Cursor_X < 8U && Cursor_Y < 3U)
    {
      uint8_t cell = (uint8_t)(Cursor_Y * 8U + Cursor_X);
      if (cell < 16U)
        led_top ^= (uint16_t)(1U << cell);
      else
        led_bottom ^= (uint16_t)(1U << (cell - 16U));
    }
  }

  /* Port D = top LED row; Port E = bottom LED row. */
  GPIOD->ODR = (GPIOD->ODR & 0xFFFF0000U) | led_top;
  GPIOE->ODR = (GPIOE->ODR & 0xFFFF0000U) | led_bottom;
}

/**
 * @brief Return nonzero while Switch 7 (PC11) is pressed.
 *        The input is active-low because PC11 is configured with a pull-up.
 */
static uint8_t Battleship_Is_Button_Pressed(void)
{
  return ((GPIOC->IDR & PLACE_BUTTON_PIN) == 0U) ? 1U : 0U;
}

/**
 * @brief Attempt to place the next boat for the current player.
 *
 * Each player places three single-cell boats followed by two double-cell
 * boats. A vertical double occupies the top and bottom cells in one column.
 * A horizontal double occupies two adjacent columns in the selected row.
 *
 * @return 1 if placement succeeded; 0 if it overlaps or is out of bounds.
 */
static uint8_t Battleship_Try_Place_Boat(void)
{
  volatile uint32_t *vertical_map;
  volatile uint32_t *horizontal_map;
  uint8_t boat_is_double = (Boats_Placed >= 3U) ? 1U : 0U;

  if (Placement_Complete || Boats_Placed >= 5U)
    return 0U;

  if (Current_Player == 1U)
  {
    vertical_map = &Player1_Vertical_Map;
    horizontal_map = &Player1_Horizontal_Map;
  }
  else
  {
    vertical_map = &Player2_Vertical_Map;
    horizontal_map = &Player2_Horizontal_Map;
  }

  if (Selected_Map == 0U)
  {
    /* Vertical board: 2 rows x 16 columns.
     * Top row occupies bits 31..16; bottom row occupies bits 15..0.
     */
    if (Cursor_X >= 16U || Cursor_Y >= 2U)
      return 0U;

    if (boat_is_double)
    {
      /* A vertical two-cell boat starts in the top row and fills both rows. */
      if (Cursor_Y != 0U)
        return 0U;

      uint32_t top_bit = (1UL << (16U + Cursor_X));
      uint32_t bottom_bit = (1UL << Cursor_X);
      uint32_t boat_bits = top_bit | bottom_bit;

      if ((*vertical_map & boat_bits) != 0U)
        return 0U;

      *vertical_map |= boat_bits;
    }
    else
    {
      uint32_t bit = (Cursor_Y == 0U)
                   ? (1UL << (16U + Cursor_X))
                   : (1UL << Cursor_X);

      if ((*vertical_map & bit) != 0U)
        return 0U;

      *vertical_map |= bit;
    }
  }
  else
  {
    /* Horizontal board: 3 rows x 8 columns, stored as three bytes. */
    if (Cursor_X >= 8U || Cursor_Y >= 3U)
      return 0U;

    if (boat_is_double)
    {
      /* Two adjacent cells in the same row; column 7 cannot be the start. */
      if (Cursor_X >= 7U)
        return 0U;

      uint32_t first_bit = (1UL << ((8U * Cursor_Y) + Cursor_X));
      uint32_t second_bit = (1UL << ((8U * Cursor_Y) + Cursor_X + 1U));
      uint32_t boat_bits = first_bit | second_bit;

      if ((*horizontal_map & boat_bits) != 0U)
        return 0U;

      *horizontal_map |= boat_bits;
    }
    else
    {
      uint32_t bit = (1UL << ((8U * Cursor_Y) + Cursor_X));

      if ((*horizontal_map & bit) != 0U)
        return 0U;

      *horizontal_map |= bit;
    }
  }

  /* Successful placement: advance to the next boat. */
  Boats_Placed++;

  if (Boats_Placed >= 5U)
  {
    if (Current_Player == 1U)
    {
      /* Player 1 is finished; now begin Player 2's five placements. */
      Current_Player = 2U;
      Boats_Placed = 0U;
      Cursor_X = 0U;
      Cursor_Y = 0U;
    }
    else
    {
      /* Both players have placed all five boats. */
      Placement_Complete = 1U;
    }
  }

  return 1U;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
