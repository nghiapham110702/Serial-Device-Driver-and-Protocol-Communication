/* Template ver 1.0 W2023
 * File:   Buffer.h
 * Author: <Nghia Pham>
 * Brief: 
 * Created on <01> <29>, <2024>, <10:45> <pm>
 * Modified on <month> <day>, <year>, <hour> <pm/am>
 */

#ifndef BUFFER_H // Header guard
#define	BUFFER_H //

/*******************************************************************************
 * PUBLIC #INCLUDES                                                            *
 ******************************************************************************/

#include "xc.h" 
#include "BOARD.h"
/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/
#define BUFFERSIZE 512

typedef struct
{
    int head;
    int tail;
    unsigned char data[BUFFERSIZE];
    
}buffer;

/*******************************************************************************
 * PUBLIC TYPEDEFS                                                             *
 ******************************************************************************/


/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/

/**
 * @Function someFunction(void)
 * @param foo, some value
 * @return TRUE or FALSE
 * @brief 
 * @note 
 * @author <Your Name>,
 * @modified <Your Name>, <year>.<month>.<day> <hour> <pm/am> */
void Buffer_Init(buffer *buff);

void enqueue(buffer *buff, char ch);

unsigned char dequeue(buffer *buff);

int isFull(buffer *buff);

int isEmpty(buffer *buff);


#endif	/* BUFFER_H */ // End of header guard
