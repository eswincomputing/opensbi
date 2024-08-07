// SPDX-License-Identifier: BSD-2-Clause
/*
 * The header file of eic770x_uart.c.
 *
 * Copyright 2024 Beijing ESWIN Computing Technology Co., Ltd.
 *
 * Authors:
 *   HuangYifeng <huangyifeng@eswincomputing.com>
 *
 */
#ifndef _EIC770X_UART_H_
#define _EIC770X_UART_H_

#define EIC770X_DIV_ROUND_CLOSEST(x, divisor)(			\
{							\
	typeof(x) __x = x;				\
	typeof(divisor) __d = divisor;			\
	(((typeof(x))-1) > 0 ||				\
	 ((typeof(divisor))-1) > 0 ||			\
	 (((__x) > 0) == ((__d) > 0))) ?		\
		(((__x) + ((__d) / 2)) / (__d)) :	\
		(((__x) - ((__d) / 2)) / (__d));	\
}							\
)

#define EIC770X_UART_RBR_OFFSET		0	/* In:  Recieve Buffer Register */
#define EIC770X_UART_THR_OFFSET		0	/* Out: Transmitter Holding Register */
#define EIC770X_UART_DLL_OFFSET		0	/* Out: Divisor Latch Low */
#define EIC770X_UART_IER_OFFSET		1	/* I/O: Interrupt Enable Register */
#define EIC770X_UART_DLM_OFFSET		1	/* Out: Divisor Latch High */
#define EIC770X_UART_FCR_OFFSET		2	/* Out: FIFO Control Register */
#define EIC770X_UART_IIR_OFFSET		2	/* I/O: Interrupt Identification Register */
#define EIC770X_UART_LCR_OFFSET		3	/* Out: Line Control Register */
#define EIC770X_UART_MCR_OFFSET		4	/* Out: Modem Control Register */
#define EIC770X_UART_LSR_OFFSET		5	/* In:  Line Status Register */
#define EIC770X_UART_MSR_OFFSET		6	/* In:  Modem Status Register */
#define EIC770X_UART_SCR_OFFSET		7	/* I/O: Scratch Register */
#define EIC770X_UART_MDR1_OFFSET	8	/* I/O:  Mode Register */
#define EIC770X_UART_DLF_OFFSET		48	/* I/O: Divisor Latch Fraction Register */

#define EIC770X_UART_LSR_FIFOE		0x80	/* Fifo error */
#define EIC770X_UART_LSR_TEMT		0x40	/* Transmitter empty */
#define EIC770X_UART_LSR_THRE		0x20	/* Transmit-hold-register empty */
#define EIC770X_UART_LSR_BI		0x10	/* Break interrupt indicator */
#define EIC770X_UART_LSR_FE		0x08	/* Frame error indicator */
#define EIC770X_UART_LSR_PE		0x04	/* Parity error indicator */
#define EIC770X_UART_LSR_OE		0x02	/* Overrun error indicator */
#define EIC770X_UART_LSR_DR		0x01	/* Receiver data ready */
#define EIC770X_UART_LSR_BRK_ERROR_BITS	0x1E	/* BI, FE, PE, OE bits */

/* U84 and stm32 communication definition */
#define FRAME_HEADER    0xA55AAA55
#define FRAME_TAIL      0xBDBABDBA

#define FRAME_DATA_MAX 250


// Message structure
typedef struct {
	uint32_t header;	// Frame heade
	uint32_t xTaskToNotify; // id
	uint8_t msg_type; 	// Message type
	uint8_t cmd_type; 	// Command type
	uint8_t cmd_result;  // command result
	uint8_t data_len; 	// Data length
	uint8_t data[FRAME_DATA_MAX]; // Data
	uint8_t checksum; 	// Checksum
	uint32_t tail;			// Frame tail
} __attribute__((packed)) Message;

// Define command types
typedef enum {
	MSG_REQUEST = 0x01,
	MSG_REPLY,
	MSG_NOTIFLY,
} MsgType;

// Define command types
typedef enum {
	CMD_POWER_OFF = 0x01,
	CMD_REBOOT,
	CMD_READ_BOARD_INFO,
	CMD_CONTROL_LED,
	CMD_PVT_INFO,
	CMD_BOARD_STATUS,
	CMD_POWER_INFO,
	CMD_RESTART,    //cold reboot with power off/on
	// You can continue adding other command types
} CommandType;


int eic770x_uart8250_init(unsigned long base, u32 in_freq, u32 baudrate, u32 reg_shift,
		  u32 reg_width);
int transmit_message(Message *msg);

#endif
