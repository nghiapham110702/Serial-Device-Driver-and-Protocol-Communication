/* Template ver 1.0 W2024
 * File:   
 * Author: <Nghia Pham>
 * Brief: 
 * Created on <month> <day>, <year>, <hour> <pm/am>
 * Modified on <month> <day>, <year>, <hour> <pm/am
 */

/*******************************************************************************
 * #INCLUDES                                                                   *
 ******************************************************************************/

#include "uart.h" // The header file for this source file. 
#include "BOARD.h"
#include "Buffer.h"
#include <xc.h>
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MessageIDs.h"
#include "Protocol2.h"

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/


/*******************************************************************************
 * PRIVATE TYPEDEFS                                                            *
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE FUNCTIONS PROTOTYPES                                                 *
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATIONS                                             *
 ******************************************************************************/
//#define TASK1
//#define TASK2
#define TASK3
//#define Test_harness
/*******************************************************************************
 * PRIVATE FUNCTION IMPLEMENTATIONS                                            *
 ******************************************************************************/
/**
 * @Function someFunction(void)
 * @param foo, some value
 * @return TRUE or FALSE
 * @brief 
 * @note 
 * @author <Your Name>
 * @modified <Your Name>, <year>.<month>.<day> <hour> <pm/am> */

// Function to print "Success" or "Fail" based on the result
void printResult(int result) {
    if (result) {
        printf("Success\n");
    } else {
        printf("Fail\n");
    }
}

#ifdef TASK1
int main(void){
    BOARD_Init();
    //printf("hellow");
    Uart_Init(115200);
    
    while(1){
        //printf("hey, im cool");
        // when there is an input detect, put the input into RX register
        // so it will output into cutecom
        if (U1STAbits.URXDA){
            U1TXREG = U1RXREG;
        }
    }
    BOARD_End();
}
#endif

#ifdef TASK2
unsigned char temp;

int main(void) {
    BOARD_Init();
    Uart_Init(115200);
    // Print a string to be sent via UART
    printf("starts\n");
    printf("I love donut and i love eat them and want to fry them\n ");
    printf("I love dog and i love to pet them and want to fry them\n ");
    printf("I love cat and i love to pet them and want to fry them\n ");
    // Print "Hello World!" using PutChar
//    const char helloWorld[] = "Hello World!\n";
//    for (int i = 0; i < strlen(helloWorld); i++) {
//        PutChar(helloWorld[i]);
//    }
//    if (isEmpty(&TXbuffer)) {
//            // Print the "Hello World" string to the TX buffer
//        printf("Hello World!\n");
//    }
    while (1) {
        if (flag_up == 1) {
            flag_up = 0;
            temp = GetChar(&RXbuffer);
            if (temp != ERROR) {
                PutChar(temp);
                }
        }
    }
    BOARD_End();
}
#endif
#ifdef Test_harness
int main() {
    // Initialize protocol
    BOARD_Init();
    Protocol_Init(115200);

    // Test Case 1: Valid packet with correct checksum
    printf("Test Case 1: ");
    rxpADT testPacket1;
    memset(&testPacket1, 0, sizeof(rxpADT));
    int result1 = BuildRxPacket(&testPacket1, HEAD);
    result1 += BuildRxPacket(&testPacket1, 0x03);  // Length = 3
    result1 += BuildRxPacket(&testPacket1, 0x10);  // ID = 0x10
    result1 += BuildRxPacket(&testPacket1, 0xAA);  // Payload byte
    result1 += BuildRxPacket(&testPacket1, 0xBB);  // Payload byte
    result1 += BuildRxPacket(&testPacket1, 0xCC);  // Payload byte
    result1 += BuildRxPacket(&testPacket1, TAIL);
    result1 += BuildRxPacket(&testPacket1, 0x5A);  // Correct checksum
    result1 += BuildRxPacket(&testPacket1, 0x5A);  // Wrap-up
    printResult(result1 == 0);

    // Test Case 2: Packet with incorrect checksum
    printf("Test Case 2: ");
    rxpADT testPacket2;
    memset(&testPacket2, 0, sizeof(rxpADT));
    int result2 = BuildRxPacket(&testPacket2, HEAD);
    result2 += BuildRxPacket(&testPacket2, 0x03);  // Length = 3
    result2 += BuildRxPacket(&testPacket2, 0x10);  // ID = 0x10
    result2 += BuildRxPacket(&testPacket2, 0xAA);  // Payload byte
    result2 += BuildRxPacket(&testPacket2, 0xBB);  // Payload byte
    result2 += BuildRxPacket(&testPacket2, 0xCC);  // Payload byte
    result2 += BuildRxPacket(&testPacket2, TAIL);
    result2 += BuildRxPacket(&testPacket2, 0x5A);  // Incorrect checksum
    result2 += BuildRxPacket(&testPacket2, 0x5A);  // Wrap-up
    printResult(result2 == 0);

    // Test Case 3: Packet with incorrect length
    printf("Test Case 3: ");
    rxpADT testPacket3;
    memset(&testPacket3, 0, sizeof(rxpADT));
    int result3 = BuildRxPacket(&testPacket3, HEAD);
    result3 += BuildRxPacket(&testPacket3, 0x07);  // Invalid length
    result3 += BuildRxPacket(&testPacket3, 0x20);  // ID = 0x20
    result3 += BuildRxPacket(&testPacket3, 0xAA);  // Payload byte
    result3 += BuildRxPacket(&testPacket3, 0xBB);  // Payload byte
    result3 += BuildRxPacket(&testPacket3, 0xCC);  // Payload byte
    result3 += BuildRxPacket(&testPacket3, TAIL);
    result3 += BuildRxPacket(&testPacket3, 0x5A);  // Correct checksum
    result3 += BuildRxPacket(&testPacket3, 0x5A);  // Wrap-up
    printResult(result3 == 0);

    BOARD_End();
}
#endif

#ifdef TASK3
int main(void){
   BOARD_Init();
   Protocol_Init(115200);
   LEDS_INIT();
   uint8_t ID = 0;
   uint8_t len = 0;
   unsigned char payload[128];       
    while (1) {
        if (flag_up == 1){
            flag_up = 0;

            // if there's something in RXbuffer
            while (!isEmpty(&RXbuffer)){
                // put it in to the statemachine
                BuildRxPacket(&rxPacket,GetChar(&RXbuffer));
//                    printf("%d",rxPacket.ID);
            }
            // if there is one or more packet in the packet buffer
            if (packet_ready == 1){
                ID = Protocol_ReadNextPacketID();
                // read length and payload
                Protocol_GetInPacket(ID,len,payload);


                if (ID == ID_LEDS_SET) {// Task 1 Set LEDS
                    // Task 1: Set LEDS
                    // Set the LEDs based on the payload data
                    LEDS_SET(payload[0]);
                }else if (ID == ID_LEDS_GET){   // Task 2 Get LEDS status
                    // Task 2: Get LEDS status
                    // Get the current LED status and send it as a response
                    unsigned char message[1];
                    message[0] = LEDS_GET();
                    Protocol_SendPacket(2,ID_LEDS_STATE,message);
                //Check if the received packet has ID equal to ID_PING.
                }else if (ID == ID_PING){       // Task 3 PING PONG
                    // Assuming the initial payload is {0x12, 0x34, 0x56, 0x78}
    
                    // Step 1: Directly construct the 32-bit value in Little-Endian format
                    // without manually converting the payload array.
                    unsigned int value = payload[3] | (payload[2] << 8) | (payload[1] << 16) | (payload[0] << 24);

                    // Step 2: Right-shift the value by 1 (divide by 2)
                    value >>= 1; // value now represents 0x3C2B1A09 in Little-Endian

                    // Step 3: Directly update the payload with the shifted value in Big-Endian order
                    // This step involves breaking down the shifted value into 4 bytes and storing them
                    // in reverse order since the final payload needs to be in Big-Endian.
                    payload[0] = (value >> 24) & 0xFF; // Extracts the highest byte
                    payload[1] = (value >> 16) & 0xFF; // Extracts the second highest byte
                    payload[2] = (value >> 8) & 0xFF;  // Extracts the third highest byte
                    payload[3] = value & 0xFF;         // Extracts the lowest byte
                    //Send a PONG packet 
                   
                    Protocol_SendPacket(5,ID_PONG,payload);
                    //flushPacketBuffer();
                }
            //clear the packet buffer
            //flushPacketBuffer();
            packet_ready = 0;
            }
        }

    }
    BOARD_End();
}
#endif