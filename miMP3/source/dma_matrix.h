/***************************************************************************//**
  @file     dma.h
  @brief    Driver for Dynamic Memory Access
  @author   TEAM OREO
 ******************************************************************************/
#ifndef DMA_MATRIX_H_
#define DMA_MATRIX_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>

#define ONE2PWMDUTTY 10
#define ZERO2PWMDUTTY 18
#define CANTLEDS 64
#define BITSPERLED 24
#define CANTBITS BITSPERLED * CANTLEDS // 64 led * 24bits/led
#define SIZEMAINTABLE CANTBITS
#define MIN_ATT 1
#define MAX_ATT 255
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef void (*dma_callback_t)(void);


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

void DMAmatrixInit();
void DMAChangeAllMatrix(uint8_t newMatrix[CANTLEDS][3],uint8_t attenuation);
void DMAChangeRow(uint8_t newRow[8][3], uint8_t row,uint8_t attenuation,bool refresh);
void DMAChangeCol(uint8_t newCol[8][3], uint8_t col,uint8_t attenuation,bool refresh);
void DMAChangeLed(uint8_t led[3],uint8_t x, uint8_t y,uint8_t attenuation,bool refresh);

void Matrix_ColByLevel(uint8_t column, uint8_t level,bool refresh);
void Matrix_TurnOff();
bool Matrix_TransferInProcess();
#endif /* DMA_MATRIX_H_ */
