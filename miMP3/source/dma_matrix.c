/***************************************************************************//**
  @file     dma.h
  @brief    Driver for Dynamic Memory Access
  @author   TEAM OREO
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "hardware.h"
#include <stddef.h>
#include "dma_matrix.h"
#include "PWM.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define RED  {0,255,0}
#define BLUE {0,0,255}
#define GREEN {255,0,0}
#define CYAN {255,0,255}
#define ORANGE {165,255,0}
#define YELLOW {255,255,0}
#define WHITE {255,255,255}
#define NO_COLOR {0,0,0}

#define ROW_NLEDS 8
#define SOURCE_OFFSET 2
#define DESTINATION_OFFSET 0
#define TRANSFER_ATTRIBUTES DMA_ATTR_SSIZE(0x1) | DMA_ATTR_DSIZE(0x1);
#define MINORLOOP 0x02

uint8_t fullRow[8][3] = {GREEN,GREEN,GREEN,YELLOW,YELLOW,ORANGE,RED,RED};
uint8_t newFullRow[8][3];
uint8_t noColorRow[3]=NO_COLOR;

/* Structure with the TCD fields. */
typedef struct
{
	uint32_t SADDR;
	uint16_t SOFF;
	uint16_t ATTR;
	union
	{
		uint32_t NBYTES_MLNO;
		uint32_t NBYTES_MLOFFNO;
		uint32_t NBYTES_MLOFFYES;
	};
	uint32_t SLAST;
	uint32_t DADDR;
	uint16_t DOFF;
	union
	{
		uint16_t CITER_ELINKNO;
		uint16_t CITER_ELINKYES;
	};
	uint32_t DLASTSGA;
	uint16_t CSR;
	union
	{
		uint16_t BITER_ELINKNO;
		uint16_t BITER_ELINKYES;
	};
}TCD_t;

/*******************************************************************************/

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/
static void DMARefreshMatrix(void);
static TCD_t mainTCD;
static bool change = true;
static uint16_t mainMatrix[2*SIZEMAINTABLE]; //Aca ya estan los bits uno por uno en PWM Format
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void DMAmatrixInit()
{

		SIM->SCGC7 |= SIM_SCGC7_DMA_MASK;
		SIM->SCGC6 |= SIM_SCGC6_DMAMUX_MASK;

		DMAMUX->CHCFG[1] |= DMAMUX_CHCFG_ENBL_MASK |DMAMUX_CHCFG_SOURCE(30); // El 30 ftm2
		/*****************************************/
		NVIC_ClearPendingIRQ(DMA1_IRQn);
		NVIC_EnableIRQ(DMA1_IRQn);
		pwmInit();
		/*************************************************/
		mainTCD.SADDR = (uint32_t)((uint16_t*)mainMatrix + 1); //El +1 es porque si no, el primer push lo hace de 9 bits xd
		mainTCD.DADDR = (uint32_t)(&(FTM2->CONTROLS[0].CnV));
		mainTCD.SOFF = SOURCE_OFFSET;
		mainTCD.DOFF = DESTINATION_OFFSET;
		mainTCD.ATTR = TRANSFER_ATTRIBUTES;
		mainTCD.NBYTES_MLNO = MINORLOOP;
		mainTCD.CITER_ELINKNO = DMA_CITER_ELINKNO_CITER(SIZEMAINTABLE);
		mainTCD.BITER_ELINKNO = DMA_BITER_ELINKNO_BITER(SIZEMAINTABLE);
		/*************************************************/
		for(int i =0; i<2*SIZEMAINTABLE; i++)
		{
			mainMatrix[i] = ZERO2PWMDUTTY;
		}
		DMARefreshMatrix();

}
void DMAChangeRow(uint8_t newRow[8][3], uint8_t row,uint8_t attenuation,bool refresh)
{
	if(row>7)
	{
		return;
	}
	else
	{
		if(!change)
		{
			for(uint8_t c = 0; c<8 ; c++)
			{
				for(uint8_t j = 0; j<3; j++)
				{
					uint8_t tmp = (uint8_t)newRow[c][j]/attenuation;

					for(uint8_t k = 0; k<8; k++)
					{
						if((tmp>>k)%2)
						{
							mainMatrix[(c*BITSPERLED*8)+(row*BITSPERLED)+(j*8)+(7-k)] = ONE2PWMDUTTY;
						}
						else
						{
							mainMatrix[(c*BITSPERLED*8)+(row*BITSPERLED)+(j*8)+(7-k)] = ZERO2PWMDUTTY;
						}
					}
				}
			}
			if(refresh)
			{
				change = true;
				DMARefreshMatrix();
			}
		}
	}

}
void DMAChangeCol(uint8_t newCol[8][3], uint8_t col,uint8_t attenuation,bool refresh)
{
	if(col>7)
	{
		return;
	}
	else
	{
		if(!change)
		{
			for(uint8_t r = 0; r<8 ; r++)
			{
				for(uint8_t j = 0; j<3; j++)
				{
					uint8_t tmp = (uint8_t)newCol[r][j]/attenuation;

					for(uint8_t k = 0; k<8; k++)
					{
						if((tmp>>k)%2)
						{
							mainMatrix[(col*BITSPERLED*8)+(r*BITSPERLED)+(j*8)+(7-k)] = ONE2PWMDUTTY;
						}
						else
						{
							mainMatrix[(col*BITSPERLED*8)+(r*BITSPERLED)+(j*8)+(7-k)] = ZERO2PWMDUTTY;
						}
					}
				}
			}
			if(refresh)
			{
				change = true;
				DMARefreshMatrix();
			}
		}
	}

}
void DMAChangeAllMatrix(uint8_t newMatrix[CANTLEDS][3],uint8_t attenuation)
{
	if(!change)
	{
		for(uint8_t i=0; i<CANTLEDS;i++)
		{
			for(uint8_t j = 0; j<3; j++)
			{
				uint8_t tmp = (uint8_t)newMatrix[i][j]/attenuation;
				for(uint8_t k = 0; k<8; k++)
				{
					if((tmp>>k)%2)
					{
						mainMatrix[(i*BITSPERLED)+(j*8)+(7-k)] = ONE2PWMDUTTY;
					}
					else
					{
						mainMatrix[(i*BITSPERLED)+(j*8)+(7-k)] = ZERO2PWMDUTTY;
					}
				}
			}
		}
		
		change = true;
		DMARefreshMatrix();
		
	}
}
void DMAChangeLed(uint8_t led[3],uint8_t x, uint8_t y,uint8_t attenuation,bool refresh)
{
	if(attenuation == 0 || x>7 || y>7)
	{
		return;
	}
	if(!change)
	{
		for(uint8_t j = 0; j<3; j++)
		{
			uint8_t tmp = (uint8_t)led[j]/attenuation;

			for(uint8_t k = 0; k<8; k++)
			{
				if((tmp>>k)%2)
				{
					mainMatrix[(y*BITSPERLED*8)+(x*BITSPERLED)+(j*8)+(7-k)] = ONE2PWMDUTTY;
				}
				else
				{
					mainMatrix[(y*BITSPERLED*8)+(x*BITSPERLED)+(j*8)+(7-k)] = ZERO2PWMDUTTY;
				}
			}
		}
		if(refresh)
		{
			change = true;
			DMARefreshMatrix();
		}
	}
}
static void DMARefreshMatrix(void)
{
	if(change)
	{
		DMA0->TCD[1].SADDR = mainTCD.SADDR;
		DMA0->TCD[1].DADDR = mainTCD.DADDR;

		DMA0->TCD[1].SOFF = mainTCD.SOFF;//0x01;
		DMA0->TCD[1].DOFF = mainTCD.DOFF;
		DMA0->TCD[1].ATTR = mainTCD.ATTR;

		DMA0->TCD[1].NBYTES_MLNO =mainTCD.NBYTES_MLNO; //0x02;

		DMA0->TCD[1].CITER_ELINKNO = mainTCD.CITER_ELINKNO;
		DMA0->TCD[1].BITER_ELINKNO = mainTCD.BITER_ELINKNO;

		DMA0->TCD[1].SLAST = 0;//-config.source_len;

		DMA0->TCD[1].DLAST_SGA = 0;

		//DMA0->TCD[channel].CSR |= DMA_CSR_INTMAJOR_MASK;
		//dma_callback[channel] = config.dma_callback;
		DMA0->ERQ |= DMA_ERQ_ERQ1_MASK;
		//DMA0->SERQ = DMA_SERQ_SERQ(0);
		DMA0->TCD[1].CSR = DMA_CSR_INTMAJOR_MASK;
		//DMA0->TCD[channel].CSR |= DMA_CSR_START_MASK;
		FTM_StartClock(FTM2);
//		DMA0->TCD[1].CSR |= DMA_CSR_START_MASK;
	}
}

/**
 * @brief Modify a column in the led Matrix.
 * @param column Number of the column you change (0-7)
 * @param level Level of the maximum led (1-8)
 * @param refresh True if you want to refresh matrix
 */
void Matrix_ColByLevel(uint8_t column, uint8_t level, bool refresh){

	int i=0;
	for(i=0;i<ROW_NLEDS;i++){
		if(i<level){
			newFullRow[i][0]=fullRow[i][0];
			newFullRow[i][1]=fullRow[i][1];
			newFullRow[i][2]=fullRow[i][2];
		}
		else{
			newFullRow[i][0]=noColorRow[0];
			newFullRow[i][1]=noColorRow[1];
			newFullRow[i][2]=noColorRow[2];
		}
	}
	DMAChangeCol(newFullRow,column,1,refresh);

}

void Matrix_TurnOff()
{
	if(!change){
		for(uint8_t i=0; i<CANTLEDS;i++)
		{
			for(uint8_t j = 0; j<3; j++)
			{
				uint8_t tmp = 0;
				for(uint8_t k = 0; k<8; k++)
				{
					if((tmp>>k)%2)
					{
						mainMatrix[(i*BITSPERLED)+(j*8)+(7-k)] = ONE2PWMDUTTY;
					}
					else
					{
						mainMatrix[(i*BITSPERLED)+(j*8)+(7-k)] = ZERO2PWMDUTTY;
					}
				}
			}
		}

		change = true;
		DMARefreshMatrix();
	}
	
}

bool Matrix_TransferInProcess(){
	return change;
}

void DMA1_IRQHandler(){
	FTM_StopClock(FTM2);
	DMA0->INT = DMA_INT_INT1_MASK;
	FTM2->CNT = 0X00;
	change = false;
}



