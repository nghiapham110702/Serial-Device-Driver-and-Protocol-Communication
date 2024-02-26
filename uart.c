/***
 * File:    uart.c
 * Author:  Nghia Pham
 * Created: 
 * This library implements a true UART device driver that enforces 
 * I/O stream abstraction between the physical and application layers.
 * All stream accesses are on a per-character or byte basis. 
/*******************************************************************************
 * #INCLUDES                                                                   *
 ******************************************************************************/
#include "uart.h"// The header file for this source file.
#include "BOARD.h"
#include <xc.h>
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Buffer.h"

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
#define FPB 40000000

/*******************************************************************************
 * PRIVATE TYPEDEFS                                                            *
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE FUNCTIONS PROTOTYPES                                                 *
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATIONS                                             *
 ******************************************************************************/
static int indata = 0;
static int collision = 0;

int flag_up = 0;
buffer TXbuffer = {};
buffer RXbuffer = {};
/*******************************************************************************
 * PRIVATE FUNCTION IMPLEMENTATIONS                                            *
 ******************************************************************************/

/**
 * @Function Uart_Init(unsigned long baudrate)
 * @param baudrate
 * @return none
 * @brief  Initializes UART1 to baudrate N81 and creates circ buffers
 * @author nghia pham*/
void Uart_Init(unsigned long baudRate){
    //Clear the control registers for UART 1
    U1MODECLR = 0x0000FFFF;
    U1STACLR = 0x0001FFFF;
    // Calculate the Baud Rate Generator value to generate 115200
    // formula was given in datasheet
    U1BRG = ((FPB / baudRate) / 16) -1;
    //set Uart1 8-N-1
    U1MODEbits.PDSEL = 00;
    // 1 stop bit
    U1MODEbits.STSEL = 0;
    //enable uart1
    U1MODEbits.ON = 1;
    
   //enable TX
    U1STAbits.UTXEN =1;
    // enable RX
    U1STAbits.URXEN = 1;
    //the UART1 transmission interrupt. When a UART transmission completes, this interrupt will be triggered.
    IEC0bits.U1TXIE = 1;
    //configures the UART1 transmission interrupt trigger condition. 
    //Setting it to 3 likely means that the interrupt is triggered when the transmit buffer becomes empty.
    U1STAbits.UTXISEL = 3;
    // initializes the UART1 transmission interrupt flag. It is cleared to start with a clean state.
    IFS0bits.U1TXIF = 0;
    //When data is received, this interrupt will be triggered.
    IEC0bits.U1RXIE = 1;
    //Setting it to 0 likely means that the interrupt is triggered 
    //when the receive buffer has at least one character.
    U1STAbits.URXISEL = 0;
    // initialize the flag
    IFS0bits.U1RXIF = 0;
    Buffer_Init(&TXbuffer);
    Buffer_Init(&RXbuffer);
    //set priority
    IPC6bits.U1IP = 0b110;
    IPC6bits.U1IS = 1;
    
    
    

}

/**
 * @Function int PutChar(char ch)
 * @param ch - the character to be sent out the serial port
 * @return True if successful, else False if the buffer is full or busy.
 * @brief  adds char to the end of the TX circular buffer
 * @author  */
int PutChar(char ch){
    //If the buffer is full, it returns an error code (ERROR). 
    if (isFull(&TXbuffer)){
        return ERROR;
    }
    //sets a flag (indata) to indicate that data is currently being sent. 
    //The purpose of this flag might be to
    //prevent other parts of the program from interfering with the transmission process.
    indata = 1;
    // adds the character ch to the TX circular buffer (TXbuffer) 
    enqueue(&TXbuffer, ch);
    //clears the flag indicating that data is being sent. 
    indata = 0;
    //checks for either a collision conditionor if the UART1 transmitter is empty
    //When U1STAbits.TRMT is equal to 1,
    //it indicates that the transmitter is ready to accept new data to be transmitted.
    if(collision == 1 || U1STAbits.TRMT == 1){
        //sets the UART1 transmit interrupt flag and clears the collision flag.
        IFS0bits.U1TXIF = 1;
        collision = 0;
    }
    //specific value 00 being assigned to is likely setting the interrupt trigger condition.
    U1STAbits.UTXSEL = 00;
    return SUCCESS;

}

/**
 * @Function unsigned char GetChar(void)
 * @param None.
 * @return NULL for error or a Char in the argument.
 * @brief  dequeues a character from the RX buffer,
 * @author */
unsigned char GetChar(buffer *buff){
    if (isEmpty(buff)){
        return ERROR;
    }
    //retrieve and remove the oldest character from the buffer (buff). 
    return dequeue(buff);
}

/**
* Refer to ...\docs\MPLAB C32 Libraries.pdf: 32-Bit Language Tools Library.
* In sec. 2.13.2 helper function _mon_putc() is noted as normally using
* UART2 for STDOUT character data flow. Adding a custom version of your own
* can redirect this to UART1 by calling your putchar() function.
*/
void _mon_putc(char c){
//your code goes here
    while(U1STAbits.UTXBF);
    PutChar(c);
}
/****************************************************************************
* Function: IntUart1Handler
* Parameters: None.
* Returns: None.
* The PIC32 architecture calls a single interrupt vector for both the
* TX and RX state machines. Each IRQ is persistent and can only be cleared
* after "removing the condition that caused it". This function is declared in
* sys/attribs.h.
****************************************************************************/
void __ISR(_UART1_VECTOR) IntUart1Handler(void) {
//your interrupt handler code goes here
    // Check if the UART1 RX interrupt flag is set
    if (IFS0bits.U1RXIF == 1){
        // Clear the UART1 RX interrupt flag
        IFS0bits.U1RXIF = 0;
        // Enqueue the received character to the RX buffer
        enqueue(&RXbuffer, U1RXREG);
        // Set a flag to indicate that new data is available
        flag_up = 1;
    }
    // Check if the UART1 TX interrupt flag is set
    if (IFS0bits.U1TXIF == 1){
        // Clear the UART1 TX interrupt flag
        IFS0bits.U1TXIF = 0;
        // Check if not in the middle of data transmission
        if(!indata){
            // Check if the TX buffer is empty
            if(isEmpty(&TXbuffer)){
                //disable interupt since dont have anything
                U1STAbits.UTXISEL = 3;
            }else{
                // we output if not empt
                U1TXREG = dequeue(&TXbuffer);
            
            }
        }else{
            //we have collition if we are enqueue
            collision = 1;
        }
    }
}


