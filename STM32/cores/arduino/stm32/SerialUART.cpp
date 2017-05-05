/**
 * TODO: Check if txBuffer is NULL in every method
 * TODO: generate different BUFFER_SIZE values for different boards based on available memory
 * TODO: add alternate pin selection functions
 * TODO: add constructor with custom buffer parameter
 */
#include "SerialUART.h"
#include "stm32_gpio_af.h"

/**
 * Set the underlying UART instance.
 */
SerialUART::SerialUART(USART_TypeDef *instance) {
    this->instance = instance;
}

/**
 * Arduino always instantiates the Serial object.
 *
 * To save memory, this implementation will:
 * - not use any memory if begin() is never called
 * - use statically allocated memory, if begin() is called exactly in one SerialUARTs.
 * - use statically allocated memory for the first, and malloc() for any subsequent calls to begin() on DIFFERENT SerialUARTs.
 */

void SerialUART::begin(const uint32_t baud) {
  if (txBuffer == NULL) {
	static uint8_t tx[BUFFER_SIZE];
	static uint8_t static_tx_used = 0;

	if (!static_tx_used) {
	  txBuffer = (uint8_t*)tx;
		static_tx_used = true;
	} else {
	  txBuffer = (uint8_t*)malloc(BUFFER_SIZE);
	}
  }

  if (rxBuffer == NULL) {
  	static uint8_t rx[BUFFER_SIZE];
  	static uint8_t static_rx_used = 0;

  	if (!static_rx_used) {
  	  rxBuffer = (uint8_t*)rx;
  		static_rx_used = true;
  	} else {
  	  rxBuffer = (uint8_t*)malloc(BUFFER_SIZE);
  	}
  }

  if (handle == NULL) {
  	static UART_HandleTypeDef h = {};
  	static uint8_t static_handle_used = 0;

  	if (!static_handle_used) {
  		handle = &h;
  	static_handle_used = true;
  	} else {
  		handle = (UART_HandleTypeDef*)malloc(sizeof(UART_HandleTypeDef));
  	}
  }

  handle->Instance = instance;
  
  #ifdef USART1
  if (handle->Instance == USART1) {
    __HAL_RCC_USART1_CLK_ENABLE();
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0); 
    HAL_NVIC_EnableIRQ(USART1_IRQn); 
  }
  #endif
  
  #ifdef USART2
  if (handle->Instance == USART2) {
    __HAL_RCC_USART2_CLK_ENABLE();
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0); 
    HAL_NVIC_EnableIRQ(USART2_IRQn); 
  }
  #endif
  
  #ifdef USART3
  if (handle->Instance == USART3) {
    __HAL_RCC_USART3_CLK_ENABLE();
    HAL_NVIC_SetPriority(USART3_IRQn, 0, 0); 
    HAL_NVIC_EnableIRQ(USART3_IRQn); 
  }
  #endif
  
  #ifdef USART4
  if (handle->Instance == USART4) {
    __HAL_RCC_USART4_CLK_ENABLE();
    HAL_NVIC_SetPriority(USART4_IRQn, 0, 0); 
    HAL_NVIC_EnableIRQ(USART4_IRQn); 
  }
  #endif
  
  stm32AfUARTInit(instance, rxPort, rxPin, txPort, txPin);
  
  handle->Init.BaudRate = baud; 
  handle->Init.WordLength = UART_WORDLENGTH_8B;
  handle->Init.StopBits = UART_STOPBITS_1; 
  handle->Init.Parity = UART_PARITY_NONE; 
  handle->Init.Mode = UART_MODE_TX_RX; 
  handle->Init.HwFlowCtl = UART_HWCONTROL_NONE; 
  handle->Init.OverSampling = UART_OVERSAMPLING_16; 
  HAL_UART_Init(handle);

  HAL_UART_Receive_IT(handle, &receive_buffer, 1);
    
}

int SerialUART::available() {
    return rxEnd != rxStart;
}
int SerialUART::peek() {
    if (available()) {
    return rxBuffer[rxStart % BUFFER_SIZE];
  } else {
    return -1;
  }
}

void SerialUART::flush() {
    
}

int SerialUART::read() {
  if (available()) {
    return rxBuffer[rxStart++ % BUFFER_SIZE];
  } else {
    return -1;
  }
}

size_t SerialUART::write(const uint8_t c) {
  while((txEnd + 1) % BUFFER_SIZE == txStart % BUFFER_SIZE);
  
  txBuffer[txEnd % BUFFER_SIZE] = c;
  txEnd++;
  if (txEnd % BUFFER_SIZE == (txStart + 1) % BUFFER_SIZE) {
    HAL_UART_Transmit_IT(handle, &txBuffer[txStart % BUFFER_SIZE], 1);
  }
  return 1;
}

void SerialUART::stm32SetRX(uint8_t rx) {
    rxPort = variant_pin_list[rx].port;
    rxPin = variant_pin_list[rx].pin_mask;
}

void SerialUART::stm32SetTX(uint8_t tx) {
    txPort = variant_pin_list[tx].port;
    txPin = variant_pin_list[tx].pin_mask;
}

//// Interrupt

SerialUART *interruptUART;

#ifdef USART1
extern "C" void USART1_IRQHandler(void) {
  interruptUART = &SerialUART1;
  HAL_UART_IRQHandler(interruptUART->handle);
}
SerialUART SerialUART1(USART1);
#endif

#ifdef USART2
extern "C" void USART2_IRQHandler(void) {
  interruptUART = &SerialUART2;
  HAL_UART_IRQHandler(interruptUART->handle);
}
SerialUART SerialUART2(USART2);
#endif

#ifdef USART3
extern "C" void USART3_IRQHandler(void) {
  interruptUART = &SerialUART3;
  HAL_UART_IRQHandler(interruptUART->handle);
}
SerialUART SerialUART3(USART3);
#endif

#ifdef USART4
extern "C" void USART4_IRQHandler(void) {
  interruptUART = &SerialUART4;
  HAL_UART_IRQHandler(interruptUART->handle);
}
SerialUART SerialUART4(USART4);
#endif


extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  interruptUART->txStart++;
  if (interruptUART->txStart != interruptUART->txEnd) {
    HAL_UART_Transmit_IT(interruptUART->handle, &interruptUART->txBuffer[interruptUART->txStart % BUFFER_SIZE], 1);
  }
}


extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  interruptUART->rxBuffer[interruptUART->rxEnd % BUFFER_SIZE] = interruptUART->receive_buffer;
  interruptUART->rxEnd++;
  HAL_UART_Receive_IT(interruptUART->handle, &interruptUART->receive_buffer, 1);
}

