#include "dac_out.h"
#include "filter.h"
#include "pit.h"

#include "../drivers/fsl_edma.h"
#include "../drivers/fsl_dmamux.h"
#include "../drivers/fsl_common.h"
#include "ping_pong_buffer.h"
#include "../drivers/fsl_common_arm.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define my_DMA         DMA0
#define my_DMAMUX 	   DMAMUX0
#define FILLBUF_CH 	   2
#define PINGPONG_CH	   0

#define TCD_QUEUE_SIZE 2U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void init_dma();
void switch_buffers(ping_pong_buffer_t* ping_pong);
void init_pit();
static void intallTCD(DMA_Type *base, uint32_t channel, edma_tcd_t *tcd);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static edma_handle_t fillbuf_EDMA_Handle;
static int ongoing_transfer = 0;

AT_QUICKACCESS_SECTION_DATA_ALIGN(edma_tcd_t tcdMemoryPoolPtr[TCD_QUEUE_SIZE + 1], sizeof(edma_tcd_t));

edma_handle_t g_EDMA_Handle;

ping_pong_buffer_t music_buffer = {
		.buffer1 = {0},
		.buffer2 = {0},
		.next_buffer =music_buffer.buffer2,
		.current_buffer =music_buffer.buffer1,
		.next_buffer_ready = 0,
		.wait = 0,
		.load_counter = 0,
};

/*******************************************************************************
 * Code
 ******************************************************************************/

/* User callback function for EDMA transfer. */
void EDMA_Callback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
	ongoing_transfer = false;
	dac_out_unpause();
	music_buffer.next_buffer_ready = true;
	return;
}


void EDMA_CallbackPingPong(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
	switch_buffers(&music_buffer);
	if (music_buffer.next_buffer_ready == true) music_buffer.next_buffer_ready = false;
	else dac_out_pause();
    return;
}

void init_dma2(){

    static edma_transfer_config_t transferConfig;
    static edma_config_t userConfig;

	DMAMUX_Init(my_DMAMUX);
    DMAMUX_SetSource(my_DMAMUX, PINGPONG_CH, 62);
    DMAMUX_EnableChannel(my_DMAMUX, PINGPONG_CH);

    EDMA_GetDefaultConfig(&userConfig);
    EDMA_Init(my_DMA, &userConfig);
    EDMA_CreateHandle(&g_EDMA_Handle, my_DMA, PINGPONG_CH);
    EDMA_SetCallback(&g_EDMA_Handle, EDMA_CallbackPingPong, NULL);
    EDMA_ResetChannel(g_EDMA_Handle.base, g_EDMA_Handle.channel);

    /* prepare descriptor 0 */
    EDMA_PrepareTransfer(&transferConfig, music_buffer.current_buffer, sizeof(uint16_t), &(DAC0->DAT[0]), sizeof(uint16_t),
                         sizeof(uint16_t),
                         sizeof(uint16_t) * BUFLEN,
                         kEDMA_MemoryToPeripheral);
    EDMA_TcdSetTransferConfig(tcdMemoryPoolPtr, &transferConfig, &tcdMemoryPoolPtr[1]);
    EDMA_TcdDisableInterrupts(tcdMemoryPoolPtr, kEDMA_HalfInterruptEnable);
    EDMA_TcdEnableInterrupts(tcdMemoryPoolPtr, kEDMA_MajorInterruptEnable);


    /* prepare descriptor 1 */
    EDMA_PrepareTransfer(&transferConfig, music_buffer.next_buffer, sizeof(uint16_t), &(DAC0->DAT[0]), sizeof(uint16_t),
                         sizeof(uint16_t),
                         sizeof(uint16_t) * BUFLEN,
                         kEDMA_MemoryToPeripheral);
    EDMA_TcdSetTransferConfig(&tcdMemoryPoolPtr[1], &transferConfig, tcdMemoryPoolPtr);
    EDMA_TcdDisableInterrupts(&tcdMemoryPoolPtr[1], kEDMA_HalfInterruptEnable);
    EDMA_TcdEnableInterrupts(&tcdMemoryPoolPtr[1], kEDMA_MajorInterruptEnable);

    intallTCD(my_DMA, PINGPONG_CH, tcdMemoryPoolPtr);
    EDMA_StartTransfer(&g_EDMA_Handle);
    DMAMUX_EnablePeriodTrigger(my_DMAMUX, PINGPONG_CH);

}

void dac_out_init(){
    hw_DisableInterrupts();
	init_dac();
	init_pit();
	dac_out_pause();
	init_dma();
	init_dma2();
	hw_EnableInterrupts();
}

void init_pit() {
	PIT_init(1302);
}

void init_dma(){
	static edma_config_t userConfig;

	DMAMUX_Init(my_DMAMUX);
	DMAMUX_SetSource(my_DMAMUX, FILLBUF_CH, 63);
	DMAMUX_EnableChannel(my_DMAMUX, FILLBUF_CH);

	EDMA_GetDefaultConfig(&userConfig);
	EDMA_Init(my_DMA, &userConfig);
	EDMA_CreateHandle(&fillbuf_EDMA_Handle, my_DMA, FILLBUF_CH);
	EDMA_SetCallback(&fillbuf_EDMA_Handle, EDMA_Callback, NULL);
}

int fill_dma_buffer(uint16_t*original_buf) {

	if(music_buffer.next_buffer_ready || ongoing_transfer) return 0; //abortamos, el prox buffer ya está listo
	ongoing_transfer = true;

	static edma_transfer_config_t transferConfig;
	EDMA_PrepareTransfer(&transferConfig, original_buf, sizeof(original_buf[0]), music_buffer.next_buffer,
						sizeof(music_buffer.next_buffer[0]), sizeof(original_buf[0]), sizeof(music_buffer.buffer1[0])*BUFLEN, kEDMA_MemoryToMemory);
	EDMA_SubmitTransfer(&fillbuf_EDMA_Handle, &transferConfig);
	EDMA_StartTransfer(&fillbuf_EDMA_Handle);

	return 1; //se copió el buffer de manera exitosa
	}

void switch_buffers(ping_pong_buffer_t* ping_pong){
	uint16_t * temp = ping_pong->current_buffer;
	ping_pong->current_buffer = ping_pong->next_buffer;
	ping_pong->next_buffer = temp;
}

static void intallTCD(DMA_Type *base, uint32_t channel, edma_tcd_t *tcd)
{
    assert(channel < FSL_FEATURE_EDMA_MODULE_CHANNEL);
    assert(tcd != NULL);
    assert(((uint32_t)tcd & 0x1FU) == 0);

    base->TCD[channel].SADDR = tcd->SADDR;
    base->TCD[channel].SOFF = tcd->SOFF;
    base->TCD[channel].ATTR = tcd->ATTR;
    base->TCD[channel].NBYTES_MLNO = tcd->NBYTES;
    base->TCD[channel].SLAST = tcd->SLAST;
    base->TCD[channel].DADDR = tcd->DADDR;
    base->TCD[channel].DOFF = tcd->DOFF;
    base->TCD[channel].CITER_ELINKNO = tcd->CITER;
    base->TCD[channel].DLAST_SGA = tcd->DLAST_SGA;

    base->TCD[channel].CSR = 0;
    base->TCD[channel].CSR = tcd->CSR;
    base->TCD[channel].BITER_ELINKNO = tcd->BITER;
}


void dac_out_pause(){
	pit_disable_trig();
	DAC_SetData(2047);
	return;
}

void dac_out_unpause(){
	pit_enable_trig();
	return;
}


