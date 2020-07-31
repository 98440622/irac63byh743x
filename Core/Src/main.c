/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lwip.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "snmp_mib2.h"
#include "snmp_scalar.h"
#include "snmp.h"
#include "bzQueue.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
	emLED_RED		= 0,
	emLED_YELLOW,
	emLED_GREEN,
	emLED_ID_NUM,
} LEDId;

typedef enum {
	emLED_ON		= 0,
	emLED_OFF,
	emLED_ST_NUM,
} LEDStatus;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_CMD_LEN	(32)

#define MAX_DHCP_TRIES  4
#define DHCP_OFF                   (uint8_t) 0
#define DHCP_START                 (uint8_t) 1
#define DHCP_WAIT_ADDRESS          (uint8_t) 2
#define DHCP_ADDRESS_ASSIGNED      (uint8_t) 3
#define DHCP_TIMEOUT               (uint8_t) 4
#define DHCP_LINK_DOWN             (uint8_t) 5

/*Static IP ADDRESS*/
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   0
#define IP_ADDR3   10

/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   0
#define GW_ADDR3   1

#define MAX_ALINK_IDS	32

#define SNMP_STR_MAX	16
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

FDCAN_HandleTypeDef hfdcan1;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
extern struct netif gnetif;
uint8_t  is_ip_addr_acquired = 0;
uint8_t  usart3_1char[1];
uint8_t  cmdLen = 0;
uint8_t  cmdBuf[MAX_CMD_LEN];
bzSimpleQueue_new(int, cmdQue, MAX_CMD_LEN);

struct netif *netif = (struct netif *)&gnetif;
ip_addr_t ipaddr;
ip_addr_t netmask;
ip_addr_t gw;
struct dhcp *dhcp;
uint8_t DHCP_state = DHCP_START;

FDCAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[12];

struct {
	uint32_t id;
	uint32_t counts;
} alinks[MAX_ALINK_IDS];
int nbr_of_alinks = 0;

//Functions prototypes
static s16_t get_switches_value(struct snmp_node_instance* instance, void* value);
static s16_t get_leds_value(struct snmp_node_instance* instance, void* value);
static snmp_err_t set_leds_value(struct snmp_node_instance* instance, u16_t len, void *value);
/*----------------------------------------------------------------------------*/
/* led .1.3.6.1.4.1.26381.1.1 */
//static const struct snmp_scalar_node leds_node = SNMP_SCALAR_CREATE_NODE_READONLY( 1, SNMP_ASN1_TYPE_INTEGER, get_leds_value);
/* led .1.3.6.1.4.1.26381.1.1 */
//#define SNMP_SCALAR_CREATE_NODE(oid, access, asn1_type, get_value_method, set_test_method, set_value_method)
static const struct snmp_scalar_node leds_node = SNMP_SCALAR_CREATE_NODE(
1, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_INTEGER, get_leds_value, NULL, set_leds_value);
/* led .1.3.6.1.4.1.26381.1.2 */
static const struct snmp_scalar_node switches_node = SNMP_SCALAR_CREATE_NODE_READONLY( 2, SNMP_ASN1_TYPE_INTEGER, get_switches_value);
/* led nodes  .1.3.6.1.4.1.26381.1 */
static const struct snmp_node* const gpio_nodes[] = { &switches_node.node.node, &leds_node.node.node };
static const struct snmp_tree_node gpio_tree_node = SNMP_CREATE_TREE_NODE(1, gpio_nodes);
static const u32_t my_base_oid[] = { 1, 3, 6, 1, 4, 1, 26381, 1 };
const struct snmp_mib gpio_mib = SNMP_MIB_CREATE(my_base_oid, &gpio_tree_node.node);

static const struct snmp_mib *my_snmp_mibs[] = { &mib2, &gpio_mib };
u8_t * syscontact_str = (u8_t*) "admin";
u16_t syscontact_len = sizeof("admin");
u8_t * syslocation_str = (u8_t*) "BZ none RTOS LWIP SNMP";
u16_t syslocation_len = sizeof("BZ none RTOS LWIP SNMP");
u8_t * sysname_str = (u8_t*) "botong";
u16_t sysname_len = sizeof("botong");
u16_t bufsize = 64;
const u8_t * sysdescr = (u8_t*) "irac36 snmp demo";
const u16_t sysdescr_len = sizeof("irac36 snmp demo");
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == &huart3) {
		//isOneCharReceived = 1;

		HAL_UART_Receive_IT(&huart3, usart3_1char, 1);

		bzSimpleQueue_push(int, cmdQue, usart3_1char[0]);
	} else ;
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
	if (hfdcan == &hfdcan1) {
		if (HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK) {
			int i = 0;
			for (; i < nbr_of_alinks; i++)
				if (alinks[i].id == RxHeader.Identifier)
					break;
		    if (nbr_of_alinks == 0 || i >= nbr_of_alinks) {
		    	alinks[nbr_of_alinks].id = RxHeader.Identifier;
		    	alinks[nbr_of_alinks].counts = 1;
		    	nbr_of_alinks++;
		    } else {
		    	alinks[i].counts++;
		    }
		}

		/* Activate Rx FIFO 0 watermark notification */
		HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_WATERMARK, 0);
	}
}

//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
//	if (huart == &huart2) {
//	} else if (huart == &huart3) {
//		// pass by
//	} else {
//		// pass by
//	}
//}

int __io_putchar(int ch) {
	return HAL_UART_Transmit(&huart3, (uint8_t*)&ch, 1, 5);
}

void show_splash_window(void) {
	printf("\033[2J");
	printf("\r\n******************************");
	printf("\r\n*                            *");
	printf("\r\n*  STM32H743x Tutorial       *");
	printf("\r\n*                            *");
	printf("\r\n* - LWIP      2.0.3          *");
	printf("\r\n* - BZoops    1.0.0          *");
	printf("\r\n*                            *");
	printf("\r\n* 98440622@163.com copyright *");
	printf("\r\n*   HAPPY COVID-19 YEAR!     *");
	printf("\r\n*                            *");
	printf("\r\n******************************\r\n\r\n");
	HAL_UART_Transmit(&huart3, (uint8_t*)"->", 2, 10);
}

void toggle_led(LEDId id) {
	switch (id) {
	case emLED_RED:
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
		break;
	case emLED_GREEN:
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
		break;
	case emLED_YELLOW:
		HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_1);
		break;
	default:
		break;
	}
}

void set_led(LEDId id, LEDStatus st) {
	switch (id) {
	case emLED_RED:
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, !st
				? GPIO_PIN_SET : GPIO_PIN_RESET);
		break;
	case emLED_GREEN:
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, !st
				? GPIO_PIN_SET : GPIO_PIN_RESET);
		break;
	case emLED_YELLOW:
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1, !st
				? GPIO_PIN_SET : GPIO_PIN_RESET);
		break;
	default:
		break;
	}
}

/* switches value .1.3.6.1.4.1.26381.1.2.0 */
static s16_t get_switches_value(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = 0;
    return sizeof(*uint_ptr);
}
/* leds value .1.3.6.1.4.1.26381.1.1.0 */
static s16_t get_leds_value(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = 1;
    return sizeof(*uint_ptr);
}
static snmp_err_t set_leds_value(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    if (val)
    	set_led(emLED_YELLOW, emLED_ON);
    else
    	set_led(emLED_YELLOW, emLED_OFF);
    return SNMP_ERR_NOERROR;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* Enable D2 domain SRAM3 Clock (0x30040000 AXI)*/
  __HAL_RCC_D2SRAM3_CLK_ENABLE();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FDCAN1_Init();
  MX_USART3_UART_Init();
  MX_LWIP_Init();
  /* USER CODE BEGIN 2 */
  {
	  /* Configure Rx FIFO 0 watermark to 2 */
	  HAL_FDCAN_ConfigFifoWatermark(&hfdcan1, FDCAN_CFG_RX_FIFO0, 2);

	  /* Activate Rx FIFO 0 watermark notification */
	  HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_WATERMARK, 0);

	  HAL_FDCAN_Start(&hfdcan1);

	  HAL_UART_Receive_IT(&huart3, usart3_1char, 1);
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  show_splash_window();
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	{
		static uint32_t pre_tick 					= 0;
		static uint32_t _1s_counter_by_systick 		= 1000;
		static uint32_t _500ms_counter_by_systick	= 500;
		static uint32_t _1m_counter_by_systick		= 60000;
		uint32_t 		cur_tick 					= HAL_GetTick();

		if (pre_tick != cur_tick) {
			if (_1s_counter_by_systick == 0) {
				// 1 second timer fired

				// reload counter
				_1s_counter_by_systick = 1000;
			} else ;

			_1s_counter_by_systick = (_1s_counter_by_systick > 0) ?
					_1s_counter_by_systick - 1 : 0;

			if (_500ms_counter_by_systick == 0) {
				// .5 second timer fired
				dhcp_fine_tmr();
				toggle_led(emLED_GREEN);

				{
				    switch (DHCP_state)
				    {
				    case DHCP_START:
				      {
				        ip_addr_set_zero_ip4(&netif->ip_addr);
				        ip_addr_set_zero_ip4(&netif->netmask);
				        ip_addr_set_zero_ip4(&netif->gw);
				        DHCP_state = DHCP_WAIT_ADDRESS;

				        dhcp_start(netif);
				      }
				      break;
				    case DHCP_WAIT_ADDRESS:
				      {
				        if (dhcp_supplied_address(netif))
				        {
				          DHCP_state = DHCP_ADDRESS_ASSIGNED;
				          printf("\r\n" "\033[0;32m" "IP is acquired." "\033[0m" "\r\n");

				          {
				        	  // start SNMP agent
				        	  //proto: void	snmp_mib2_set_syscontact(u8_t *ocstr, u16_t *ocstrlen, u16_t bufsize)
				        	  snmp_mib2_set_syscontact(syscontact_str, &syscontact_len, bufsize);
				        	  //proto: void	snmp_mib2_set_syslocation(u8_t *ocstr, u16_t *ocstrlen, u16_t bufsize)
				        	  snmp_mib2_set_syslocation(syslocation_str, &syslocation_len, bufsize);
				        	  //proto: void snmp_set_auth_traps_enabled(u8_t enable)
				        	  //set to 0 when disabled, >0 when enabled
				        	  snmp_set_auth_traps_enabled(ENABLE);
				        	  //proto: void snmp_mib2_set_sysdescr(const u8_t *str, const u16_t *len)
				        	  snmp_mib2_set_sysdescr(sysdescr, &sysdescr_len);
				        	  //proto: void snmp_mib2_set_sysname(u8_t *ocstr, u16_t *ocstrlen, u16_t bufsize)
				        	  snmp_mib2_set_sysname(sysname_str, &sysname_len, bufsize);
				        	  //proto: void snmp_trap_dst_ip_set(u8_t dst_idx, const ip_addr_t *dst)
				        	  snmp_trap_dst_ip_set(0, &netif->gw);
				        	  snmp_trap_dst_enable(0, ENABLE);
				        	  snmp_set_mibs(my_snmp_mibs, LWIP_ARRAYSIZE(my_snmp_mibs));
				        	  snmp_init(); //Just after udp_init

				        	  printf("\033[0;32mSNMP initialization completed.\033[0m\r\n;");
				          }
				        }
				        else
				        {
				          dhcp = (struct dhcp *)netif_get_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);

				          /* DHCP timeout */
				          if (dhcp->tries > MAX_DHCP_TRIES)
				          {
				            DHCP_state = DHCP_TIMEOUT;

				            /* Stop DHCP */
				            dhcp_stop(netif);

				            /* Static address used */
				            IP_ADDR4(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
				            IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
				            IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
				            netif_set_addr(netif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask), ip_2_ip4(&gw));

				            printf("\r\nDHCP timeout.\r\n");
				          }
				        }
				      }
				      break;
				  case DHCP_LINK_DOWN:
				    {
				      /* Stop DHCP */
				      dhcp_stop(netif);
				      DHCP_state = DHCP_OFF;
				    }
				    break;
				    default: break;
				    }
				}

				// reload counter
				_500ms_counter_by_systick = 500;
			} else ;

			_500ms_counter_by_systick = (_500ms_counter_by_systick > 0) ?
					_500ms_counter_by_systick - 1 : 0;

			if (_1m_counter_by_systick == 0) {
				// 1 minute timer fired
				dhcp_coarse_tmr();

				// reload counter
				_1m_counter_by_systick = 60000;
			} else ;

			_1m_counter_by_systick = (_1m_counter_by_systick > 0) ?
					_1m_counter_by_systick - 1 : 0;
		} else ;

		pre_tick = cur_tick;
	}

	MX_LWIP_Process();

	{
		if (!bzSimpleQueue_isEmpty(cmdQue)) {
			uint8_t ch = 0;
			bzSimpleQueue_pop(uint8_t, cmdQue, ch);
			switch (ch) {
			case '\r':
				if (cmdLen > 0) {
					cmdBuf[cmdLen] = '\0';
					printf("\r\n");

					// TODO : user commands
					if (!strcmp((char*)cmdBuf, "ifconfig")) {
						if (netif_is_link_up(&gnetif)) {
							int i = 0;
							struct dhcp* dhcp_info = netif_dhcp_data(&gnetif);

							printf("MAC : ");
							for (i = 0; i < 6; i++) {
								printf("%02x", gnetif.hwaddr[i]);
								if (i < 5)
									printf(":");
							}
							printf("\r\n");

							printf("IP  : ");
							for (i = 0; i < 4; i++) {
								printf("%d", (int)((dhcp_info->offered_ip_addr.addr >> (8 * i)) & 0x000000FF));
								if (i < 3)
									printf(".");
							}
							printf("\r\n");

							printf("SNSK: ");
							for (i = 0; i < 4; i++) {
								printf("%d", (int)((dhcp_info->offered_sn_mask.addr >> (8 * i)) & 0x000000FF));
								if (i < 3)
									printf(".");
							}
							printf("\r\n");

							printf("GTWY: ");
							for (i = 0; i < 4; i++) {
								printf("%d", (int)((dhcp_info->offered_gw_addr.addr >> (8 * i)) & 0x000000FF));
								if (i < 3)
									printf(".");
							}
							printf("\r\n");
						} else {
							printf("network is not connected\r\n");
						}
						printf("\r\n");
					} else if (!strcmp((char*)cmdBuf, "alink")) {
						int i = 0;
						printf("alink message statistic:\r\n");
						for (; i < nbr_of_alinks; i++) {
							printf("0x%08x %d\r\n", (unsigned int)alinks[i].id, (int)alinks[i].counts);
						}
					} else {
						printf("%s\r\n", cmdBuf);
						HAL_UART_Transmit(&huart3, (uint8_t*)"->", 2, 10);
					}
				} else {
					HAL_UART_Transmit(&huart3, (uint8_t*)"\r\n->", 4, 10);
				}

				cmdLen = 0;
				break;
			case '\b':
				if (cmdLen > 0) {
					HAL_UART_Transmit(&huart3, (uint8_t*)"\b \b", 3, 10);
					cmdLen--;
				}
				break;
			default:
				if (ch >= ' ' && ch <= '~') {
					cmdBuf[cmdLen++] = ch;
					HAL_UART_Transmit(&huart3, &ch, 1, 10);
					if (cmdLen == MAX_CMD_LEN - 1) {
						cmdLen = 0;
					}
				}
				break;
			}

		} else ;
	}
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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOMEDIUM;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_FDCAN;
  PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = ENABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = ENABLE;
  hfdcan1.Init.NominalPrescaler = 32;
  hfdcan1.Init.NominalSyncJumpWidth = 8;
  hfdcan1.Init.NominalTimeSeg1 = 31;
  hfdcan1.Init.NominalTimeSeg2 = 8;
  hfdcan1.Init.DataPrescaler = 32;
  hfdcan1.Init.DataSyncJumpWidth = 8;
  hfdcan1.Init.DataTimeSeg1 = 31;
  hfdcan1.Init.DataTimeSeg2 = 8;
  hfdcan1.Init.MessageRAMOffset = 0;
  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.RxFifo0ElmtsNbr = 2;
  hfdcan1.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxFifo1ElmtsNbr = 2;
  hfdcan1.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxBuffersNbr = 1;
  hfdcan1.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.TxEventsNbr = 2;
  hfdcan1.Init.TxBuffersNbr = 1;
  hfdcan1.Init.TxFifoQueueElmtsNbr = 2;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|RED_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(YELLOW_LED_GPIO_Port, YELLOW_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB0 RED_LED_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_0|RED_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : YELLOW_LED_Pin */
  GPIO_InitStruct.Pin = YELLOW_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(YELLOW_LED_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();
  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x30040000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x30044000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
