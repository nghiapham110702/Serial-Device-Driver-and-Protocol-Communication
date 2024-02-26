/* Template ver 1.0 W2024
 * File:   Buffer.c
 * Author: <Nghia Pham>
 * Brief: 
 * Created on <01> <30>, <2024>, <5:05> <pm>
 * Modified on <month> <day>, <year>, <hour> <pm/am>
 */

/*******************************************************************************
 * #INCLUDES                                                                   *
 ******************************************************************************/

#include "Buffer.h" // The header file for this source file. 

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

/*******************************************************************************
 * PRIVATE FUNCTION IMPLEMENTATIONS                                            *
 ******************************************************************************/

/**
 * @Function someFunction(void)
 * @param foo, some value
 * @return TRUE or FALSE
 * @brief 
 * @note 
 * @author <Your Name>,
 * @modified <Your Name>, <year>.<month>.<day> <hour> <pm/am> */
void Buffer_Init(buffer *buff){
    buff->head = 0;
    buff->tail = 0;

}
void enqueue(buffer *buff, char ch){
    //checks whether the buffer is not full.
    if (!isFull(buff)){
        //If the buffer is not full, the character ch is added to the buffer at the position indicated by buff->tail. 
        //buff->tail is the index where the next element will be added. 
        //buff->data is the array that holds the actual data in the buffer.
        buff->data[buff->tail] = ch;
        //updated to point to the next position in the buffer. 
        //The expression ((buff->tail+1) % BUFFERSIZE) calculates the next index, 
        //and the modulo operator (%)
        //ensures that the index wraps around to 0 when it reaches
        //the end of the buffer (BUFFERSIZE), creating a circular buffer.
        buff->tail = ((buff->tail+1) % BUFFERSIZE);
    }
}

unsigned char dequeue(buffer *buff){
    //declares a temporary variable x and initializes it to 0. This variable will store the character being dequeued.
    unsigned char x = 0;
    //checks whether the buffer is not empty
    if (!isEmpty(buff)){
        // the character at the head of the buffer (buff->head) is read into the temporary variable x.
        x = buff->data[buff->head];
        buff->head = ((buff->head+1) % BUFFERSIZE);
        return x;    
    }
}
int isFull(buffer *buff){
    return (buff->head == ((buff->tail + 1) % BUFFERSIZE));
}

int isEmpty(buffer *buff){
    return (buff->head == buff->tail);

}


