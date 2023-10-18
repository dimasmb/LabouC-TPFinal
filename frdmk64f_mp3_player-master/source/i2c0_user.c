#include "i2c0_user.h"

#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_i2c.h"
#include "fsl_port.h"
#include "fsl_i2c_edma.h"

#include "clock_config.h"
#include "pin_mux.h"
#define EXAMPLE_I2C_MASTER_BASEADDR I2C0
#define I2C_MASTER_CLK_SRC I2C0_CLK_SRC
#define I2C_MASTER_CLK_FREQ CLOCK_GetFreq(I2C0_CLK_SRC)

#define DMA_REQUEST_SRC kDmaRequestMux0I2C0
#define I2C_DMA_CHANNEL 0U
#define EXAMPLE_I2C_DMAMUX_BASEADDR DMAMUX0
#define EXAMPLE_I2C_DMA_BASEADDR DMA0
#define I2C_DATA_LENGTH (32) /* MAX is 256 */
#define I2C_MASTER_SLAVE_ADDR_7BIT (0x3CU)
#define I2C_BAUDRATE (1000000) /* 100K */


i2c_master_edma_handle_t g_m_dma_handle;
edma_handle_t edmaHandle;

volatile uint8_t i2c_dma_complete;
volatile uint8_t i2c_dma_running;
volatile uint8_t i2c_transfer_done;
volatile uint8_t i2c_bulk_push;

i2c_master_handle_t i2cmh;


i2c_master_config_t masterConfig;

uint32_t sourceClock;
edma_config_t config;



void  i2c_master_transfer_callback (I2C_Type *base,i2c_master_handle_t *handle,status_t status,void *userData) 
{
  if(status == 0) {
    i2c_transfer_done = 1;
    
    if(i2c_bulk_push)
      i2c_bulk_push = 0;
  }
}
void i2c_master_edma_transfer_callback (I2C_Type *base,i2c_master_edma_handle_t *handle,status_t status,void *userData)
{
  if(status == 0) {
    i2c_dma_complete = 1;
    i2c_transfer_done = 1;
    if(i2c_dma_running) {
      i2c_dma_running = 0;
    }
  }
}



static void i2c_release_bus_delay(void)
{
    uint32_t i = 0;
    for (i = 0; i < I2C_RELEASE_BUS_COUNT; i++)
    {
        __NOP();
    }
}

void I2C_ReleaseBus(void)
{
    uint8_t i = 0;
    gpio_pin_config_t pin_config;
    port_pin_config_t i2c_pin_config = {0};

    /* Config pin mux as gpio */
    i2c_pin_config.pullSelect = kPORT_PullUp;
    i2c_pin_config.mux = kPORT_MuxAsGpio;

    pin_config.pinDirection = kGPIO_DigitalOutput;
    pin_config.outputLogic = 1U;
    CLOCK_EnableClock(kCLOCK_PortE);
    PORT_SetPinConfig(I2C_RELEASE_SCL_PORT, I2C_RELEASE_SCL_PIN, &i2c_pin_config);
    PORT_SetPinConfig(I2C_RELEASE_SCL_PORT, I2C_RELEASE_SDA_PIN, &i2c_pin_config);

    GPIO_PinInit(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, &pin_config);
    GPIO_PinInit(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, &pin_config);

    /* Drive SDA low first to simulate a start */
    GPIO_WritePinOutput(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 0U);
    i2c_release_bus_delay();

    /* Send 9 pulses on SCL and keep SDA low */
    for (i = 0; i < 9; i++)
    {
        GPIO_WritePinOutput(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 0U);
        i2c_release_bus_delay();

        GPIO_WritePinOutput(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 1U);
        i2c_release_bus_delay();

        GPIO_WritePinOutput(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 1U);
        i2c_release_bus_delay();
        i2c_release_bus_delay();
    }

    /* Send stop */
    GPIO_WritePinOutput(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 0U);
    i2c_release_bus_delay();

    GPIO_WritePinOutput(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 0U);
    i2c_release_bus_delay();

    GPIO_WritePinOutput(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 1U);
    i2c_release_bus_delay();

    GPIO_WritePinOutput(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 1U);
    i2c_release_bus_delay();
}

void i2c0_user_init(void)
{
  
  
    EnableIRQ(I2C0_IRQn);
    NVIC_SetPriority(I2C0_IRQn, 0);

    /*Init EDMA for example*/
    DMAMUX_Init(EXAMPLE_I2C_DMAMUX_BASEADDR);
    EDMA_GetDefaultConfig(&config);
    EDMA_Init(EXAMPLE_I2C_DMA_BASEADDR, &config);

    /*  set master priority lower than slave */
    NVIC_EnableIRQ(DMA0_IRQn);
    NVIC_SetPriority(DMA0_IRQn, 2);
    
    I2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Bps = I2C_BAUDRATE;

    sourceClock = I2C_MASTER_CLK_FREQ;

    I2C_MasterInit(EXAMPLE_I2C_MASTER_BASEADDR, &masterConfig, sourceClock);

     I2C_MasterTransferCreateHandle(I2C0, &i2cmh, i2c_master_transfer_callback, "MOK");
//
    //memset(&g_m_dma_handle, 0, sizeof(g_m_dma_handle));


    //DMAMUX_SetSource(EXAMPLE_I2C_DMAMUX_BASEADDR, I2C_DMA_CHANNEL, DMA_REQUEST_SRC);
    //DMAMUX_EnableChannel(EXAMPLE_I2C_DMAMUX_BASEADDR, I2C_DMA_CHANNEL);
  //  EDMA_CreateHandle(&edmaHandle, EXAMPLE_I2C_DMA_BASEADDR, I2C_DMA_CHANNEL);
    
    //I2C_MasterCreateEDMAHandle(EXAMPLE_I2C_MASTER_BASEADDR, &g_m_dma_handle, i2c_master_edma_transfer_callback, NULL, &edmaHandle);
}