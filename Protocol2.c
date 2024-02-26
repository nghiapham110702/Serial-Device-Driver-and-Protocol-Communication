/* 
 * File:   Protocol.c
 * Author: Nghia Pham
 * rewritten for Uart.c 
 */

/*******************************************************************************
 * PUBLIC #DEFINES                                                            *
 ******************************************************************************/

#include "uart.h" // The header file for this source file. 
#include "BOARD.h"
#include "Buffer.h"
#include "Protocol2.h"
#include "MessageIDs.h"
#include <xc.h>
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************************************
 * PUBLIC DATATYPES
 ******************************************************************************/
rxpADT rxPacket;
packet_buffer PKbuffer = {};
static uint8_t payload_counter = 0;
static unsigned char cksum = 0;
int packet_ready;
/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/
typedef enum {
    IDLE,
    GET_LENGTH, 
    GET_ID, 
    GET_PAYLOAD, 
    GET_TAIL,
    WRAP_UP, 
}modes;
static modes STATE = IDLE;
/**
 * @Function Protocol_Init(baudrate)
 * @param Legal Uart baudrate
 * @return SUCCESS (true) or ERROR (false)
 * @brief Initializes Uart1 for stream I/O to the lab PC via a USB 
 *        virtual comm port. Baudrate must be a legal value. 
 * @author  */
//function is primarily intended for initializing the UART and related data structures, not for indicating success or failure. 
//The UART initialization is the main purpose, and it doesn't require returning a success or error status.
void Protocol_Init(unsigned long baudrate){
    Uart_Init(baudrate);
    PKbuffer.head = 0;
    PKbuffer.tail = 0;
    rxPacket.ID = 0;
    rxPacket.len = 0;
    rxPacket.checkSum = 0;
    
}

/**
 * @Function unsigned char Protocol_QueuePacket()
 * @param none
 * @return the buffer full flag: 1 if full
 * @brief Place in the main event loop (or in a timer) to continually check 
 *        for completed incoming packets and then queue them into 
 *        the RX circular buffer. The buffer's size is set by constant
 *        PACKETBUFFERSIZE.
 * @author  */
uint8_t Protocol_QueuePacket (){
    // same idea as enqueue function in buffer.c
    //uses a circular buffer to store received packets. 
    //The circular buffer has a fixed size defined by the constant PACKETBUFFERSIZE.
    
    //if the circular buffer is full. If the next position 
    //after the current tail is equal to the head, it means that the buffer is full, 
    //and there is no space to enqueue another packet.
    if (((PKbuffer.tail + 1) % PACKETBUFFERSIZE) == PKbuffer.head) {
        return 1;  // Buffer is full
    }
    //Copies the contents of the rxPacket (received packet) into the circular buffer at the position indicated by the current tail.
    PKbuffer.data[PKbuffer.tail] = rxPacket;
    PKbuffer.tail = ((PKbuffer.tail + 1) % PACKETBUFFERSIZE);

    
}

/**
 * @Function int Protocol_GetInPacket(uint8_t *type, uint8_t *len, uchar *msg)
 * @param *type, *len, *msg
 * @return SUCCESS (true) or WAITING (false)
 * @brief Reads the next packet from the packet Buffer 
 * @author  */
int Protocol_GetInPacket(uint8_t type, uint8_t len, unsigned char msg[]){
    // Check if the circular buffer is empty (head and tail are equal)
    if(PKbuffer.head == PKbuffer.tail){
        return 0;
    }
    // Retrieve data from the packet at the head of the circular buffer
    type = PKbuffer.data[PKbuffer.head].ID;
    len = PKbuffer.data[PKbuffer.head].len;
    strcpy(msg, PKbuffer.data[PKbuffer.head].payLoad);
    //ove the head of the circular buffer to the next position
    PKbuffer.head = ((PKbuffer.head + 1) % PACKETBUFFERSIZE);
    return SUCCESS;
}

/**
 * @Function int Protocol_SendDebugMessage(char *Message)
 * @param Message, Proper C string to send out
 * @return SUCCESS (true) or ERROR (false)
 * @brief Takes in a proper C-formatted string and sends it out using ID_DEBUG
 * @warning this takes an array, do <b>NOT</b> call sprintf as an argument.
 * @author  */
int Protocol_SendDebugMessage(char Message[]){
    return Protocol_SendPacket(strlen(Message), ID_DEBUG, Message);
}

/**
 * @Function int Protocol_SendPacket(unsigned char len, void *Payload)
 * @param len, length of full <b>Payload</b> variable
 * @param Payload, pointer to data
 * @return SUCCESS (true) or ERROR (false)
 * @brief composes and sends a full packet
 * @author  */
int Protocol_SendPacket(unsigned char len, unsigned char ID, unsigned char Payload[]){
 // Initialize checksum
    unsigned char cksum = 0; 

    // Use last_send to record the success of the last transmission
    int check_send = SUCCESS;

    // Send the HEAD byte
    if (check_send == SUCCESS) {
        check_send = PutChar(HEAD);
    }

    // Send the length byte
    if (check_send == SUCCESS) {
        check_send = PutChar(len);
    }

    // Update checksum with ID and send the ID byte
    cksum = Protocol_CalcIterativeChecksum(ID, cksum);
    if (check_send == SUCCESS) {
        check_send = PutChar(ID);
    }

    // updates the checksum with each byte of the payload and sends them in a loop.
    //If the transmission is successful, last_send is updated to SUCCESS.
    for (int i = 0; i < (len - 1); i++) {
        cksum = Protocol_CalcIterativeChecksum(Payload[i], cksum);
        if (check_send == SUCCESS) {
            check_send = PutChar(Payload[i]);
        }
    }

    // Send the TAIL byte
    if (check_send == SUCCESS) {
        check_send = PutChar(TAIL);
    }

    // Send the checksum, carriage return, and newline characters
    if (check_send == SUCCESS) {
        check_send = PutChar(cksum);
    }
     if (check_send == SUCCESS) {
        check_send = PutChar('\r');
    }
     if (check_send == SUCCESS) {
        check_send = PutChar('\n');
    }

    return check_send;
}

/**
 @Function unsigned char Protocol_ReadNextID(void)
 * @param None
 * @return Reads the ID of the next available Packet
 * @brief Returns ID or 0 if no packets are available
 * @author  */
unsigned char Protocol_ReadNextPacketID(void){
    if (PKbuffer.head == PKbuffer.tail) {
        return 0;  
    }
    //returns the ID of the packet at the head of the buffer (PKbuffer.head).
    return PKbuffer.data[PKbuffer.head].ID;
}

/**
 * @Function flushPacketBuffer()
 * @param none
 * @return none
 * @brief flushes the rx packet circular buffer  
 * @author */
void flushPacketBuffer (){
    memset(PKbuffer.data,0, sizeof(PKbuffer.data));

}
//convert the endianness of the given payload to ensure consistency in byte order
//, especially when dealing with multi-byte data types.
//Suppose you have a payload of length 4 with the bytes [A, B, C, D]. After applying the convertEndian function,
//the payload would be transformed to [D, C, B, A], effectively converting from little-endian to big-endian or vice versa.
unsigned int convertEndian(unsigned char payload[], int len){
    //if the length of the payload is 1. If the length is 1, 
    //there is no need to perform any endian conversion, so the function returns SUCCESS.
    if (len==1){
        return SUCCESS;
    } 
    //iterates through the first half of the payload array. 
    //The loop condition i<(len/2) ensures that the swapping is performed only up to the midpoint of the array.
    unsigned char x;
    for (int i=0; i<(len/2); i++){
        // The current byte at index i of the payload array is stored in the temporary variable x
        x = payload[i];
        //The byte at the opposite end of the array (calculated as len-1-i) is moved to the current position i.
        payload[i] = payload[len-1-i];
        //The originally stored byte (x) is moved to the opposite end of the array.
        payload[len-1-i] = x;
    }
    return SUCCESS;
}
/*******************************************************************************
 * PRIVATE FUNCTIONS
 * Generally these functions would not be exposed but due to the learning nature 
 * of the class some are are noted to help you organize the code internal 
 * to the module. 
 ******************************************************************************/

/* BuildRxPacket() should implement a state machine to build an incoming
 * packet incrementally and return it completed in the called argument packet
 * structure (rxPacket is a pointer to a packet struct). The state machine should
 * progress through discrete states as each incoming byte is processed.
 * 
 * To help you get started, the following ADT is an example of a structure 
 * intended to contain a single rx packet. 
 * typedef struct rxpT {
 *    uint8_t ID;      
 *    uint8_t len;
 *    uint8_t checkSum; 
 *    unsigned char payLoad[MAXPAYLOADLENGTH];
 * }*rxpADT; 
 *   rxpADT rxPacket ...
 * Now consider how to create another structure for use as a circular buffer
 * containing a PACKETBUFFERSIZE number of these rxpT packet structures.
 ******************************************************************************/
uint8_t BuildRxPacket (rxpADT *input_Packet, unsigned char reset){
    switch (STATE){
        //This is the initial state where the state machine 
        //is waiting for the start of a new packet (indicated by the HEAD byte).
        case(IDLE):
            //If the incoming byte (reset) is the start of a new packet (HEAD),
            //the function resets all packet-related variables 
            //(length, ID, payload, and checksum) and transitions to the GET_LENGTH state.
            if (reset==HEAD){
            
                input_Packet->len = 0;
                input_Packet->ID = 0;
                memset(input_Packet->payLoad, 0, sizeof(input_Packet->payLoad));
                input_Packet->checkSum = 0;
                STATE = GET_LENGTH;
            }
            break;
        
        
        //reads the length of the incoming packet.
        case(GET_LENGTH):
            //If the length byte is greater than 128, it indicates an error,
            //and the state machine transitions back to the IDLE state after sending a length error debug message.
            if (reset>128) {
                STATE = IDLE;
            }
            //sets the length of the packet and transitions to the GET_ID state.
            input_Packet->len = reset;
            STATE = GET_ID;
            break;
    
            
          //state reads the payload bytes of the packet.            
        case(GET_ID):
            //Sets the packet's ID and updates the checksum with the ID.
            //If the length is 1, it transitions directly to the GET_TAIL state. 
            //Otherwise, it transitions to the GET_PAYLOAD state.
            input_Packet->ID = reset;
            //For each byte (character) received (reset), 
            //the function updates the current checksum (cksum) based on the current state of the checksum and the new byte.
            cksum = Protocol_CalcIterativeChecksum(reset, cksum);
            // a packet with length 1 has no payload
            if (input_Packet->len == 1) {
                STATE = GET_TAIL;
            //indicating that there is payload data to be received.
            }else {
                STATE = GET_PAYLOAD;
            }
            break;
            
         //This state reads the payload bytes of the packet.
        case(GET_PAYLOAD):
            //stores the incoming byte (reset) in the payload array 
            //at the current position indicated by the payload_counter. 
            // reads and stores each payload byte as it arrives.

            input_Packet->payLoad[payload_counter] = reset;
            cksum = Protocol_CalcIterativeChecksum(reset, cksum);
            //increments the payload counter, indicating that the next incoming 
            //byte will be stored in the next position of the payload array
            payload_counter++;
            //checks if all payload bytes have been read. 
            //If the counter (payload_counter) is greater than or equal to the 
            //expected payload length (input_Packet->len - 1), it means all payload bytes are received
            if (payload_counter >= ((input_Packet->len)-1)){
                STATE = GET_TAIL;
                //reset to 0 to prepare for the next packet.
                payload_counter = 0;
            }
            break;
            
            
        case(GET_TAIL):
            //if the incoming byte (reset) is equal to the expected tail byte (TAIL).
            //If they match, it means that the end of the packet is reached and valid. 
            //In this case, it transitions to the next state, which is WRAP_UP
            if (reset==TAIL){
                STATE = WRAP_UP;
            } else{
                STATE = IDLE;
            }
            break;
            
              
        case(WRAP_UP):
            // If the received checksum (reset) matches the calculated checksum (cksum),
            //it means that the packet has been received without errors. 
            //In this case, it queues the packet for further processing (perhaps storing it in a buffer), sets 
            //the packet_ready to 1 (indicating that a valid packet is ready), 
            //and proceeds to the IDLE state to prepare for the next packet.
            if (reset==cksum){
                Protocol_QueuePacket();
                packet_ready = 1;
            }
            cksum = 0;
            STATE = IDLE;
            break;
            
            
         
    }
}

/**
 * @Function char Protocol_CalcIterativeChecksum(unsigned char charIn, unsigned char curChecksum)
 * @param charIn, new char to add to the checksum
 * @param curChecksum, current checksum, most likely the last return of this function, can use 0 to reset
 * @return the new checksum value
 * @brief Returns the BSD checksum of the char stream given the curChecksum and the new char
 * @author  */
unsigned char Protocol_CalcIterativeChecksum(unsigned char charIn, unsigned char curChecksum){
    // Right shift the current checksum by 1 and add the result to the left-shifted current checksum
    curChecksum = (curChecksum >> 1) + (curChecksum << 7);
    // Add the new character to the checksum
    curChecksum += charIn;
    return curChecksum;
}

/**
 * This macro initializes all LEDs for use. It enables the proper pins as outputs and also turns all
 * LEDs off.
 */
#define LEDS_INIT() do {LATECLR = 0xFF; TRISECLR = 0xFF;} while (0)

/**
 * Provides a way to quickly get the status of all 8 LEDs into a uint8, where a bit is 1 if the LED
 * is on and 0 if it's not. The LEDs are ordered such that bit 7 is LED8 and bit 0 is LED0.
 */
#define LEDS_GET() (LATE & 0xFF)

/**
 * This macro sets the LEDs on according to which bits are high in the argument. Bit 0 corresponds
 * to LED0.
 * @param leds Set the LEDs to this value where 1 means on and 0 means off.
 */
#define LEDS_SET(leds) do { LATE = (leds); } while (0)


