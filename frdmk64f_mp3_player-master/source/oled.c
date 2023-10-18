
#include <stdlib.h>
#include "oled.h"  
#include "glcd.h"
#include <stdint.h>
#include <fsl_i2c.h>
#include "fsl_dmamux.h"
#include "fsl_edma.h"
#include "glcd.h"
#include "fsl_i2c_edma.h"

extern i2c_master_handle_t i2cmh;
extern volatile uint8_t i2c_transfer_done;
extern volatile uint8_t i2c_dma_complete;
extern volatile uint8_t i2c_dma_running;

extern uint8_t glcd_buffer[GLCD_LCD_WIDTH * GLCD_LCD_HEIGHT / 8];
extern volatile  uint8_t i2c_bulk_push;
extern i2c_master_edma_handle_t g_m_dma_handle;
extern edma_handle_t edmaHandle;
  
__attribute__((packed)) struct {
    uint8_t init[15];
    uint8_t fb[128*4];
  } fb_f;
    
  const uint8_t inits[] = {
    0x80,SSD1306_COLUMNADDR,\
      0x80,0,\
        0x80,127,\
          0x80,SSD1306_PAGEADDR,\
            0x80,0,\
              0x80,3,\
                0x80,0xb0,\
                  0x40
  };

    

void oled_init(void)
{
   #if defined SSD1306_128_32
    // Init sequence for 128x32 OLED module
    ssd1306_command(SSD1306_DISPLAYOFF);                    // 0xAE
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
    ssd1306_command(0x80);                                  // the suggested ratio 0x80
    ssd1306_command(SSD1306_SETMULTIPLEX);                  // 0xA8
    ssd1306_command(0x1F);
    ssd1306_command(SSD1306_SETDISPLAYOFFSET);              // 0xD3
    ssd1306_command(0x0);                                   // no offset
    ssd1306_command(SSD1306_SETSTARTLINE | 0x0);            // line #0
    ssd1306_command(SSD1306_CHARGEPUMP);                    // 0x8D
   // if (vccstate == SSD1306_EXTERNALVCC) 
  //    { ssd1306_command(0x10); }
   // else 
      { ssd1306_command(0x14); }
    ssd1306_command(SSD1306_MEMORYMODE);                    // 0x20
    ssd1306_command(0x00);                                  // 0x0 act like ks0108
	ssd1306_command(SSD1306_SEGREMAP | 0x1);
    ssd1306_command(SSD1306_COMSCANDEC);
    ssd1306_command(SSD1306_SETCOMPINS);                    // 0xDA
    ssd1306_command(0x02);
    ssd1306_command(SSD1306_SETCONTRAST);                   // 0x81
    ssd1306_command(0x8F);
    ssd1306_command(SSD1306_SETPRECHARGE);                  // 0xd9
 //   if (vccstate == SSD1306_EXTERNALVCC) 
      { ssd1306_command(0x22); }
 //   else 
 //     { ssd1306_command(0xF1); }
    ssd1306_command(SSD1306_SETVCOMDETECT);                 // 0xDB
    ssd1306_command(0x40);
    ssd1306_command(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
    ssd1306_command(SSD1306_NORMALDISPLAY);                 // 0xA6
  #endif

  #if defined SSD1306_128_64
    // Init sequence for 128x64 OLED module
    ssd1306_command(SSD1306_DISPLAYOFF);                    // 0xAE
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
    ssd1306_command(0x80);                                  // the suggested ratio 0x80
    ssd1306_command(SSD1306_SETMULTIPLEX);                  // 0xA8
    ssd1306_command(0x3F);
    ssd1306_command(SSD1306_SETDISPLAYOFFSET);              // 0xD3
    ssd1306_command(0x0);                                   // no offset
    ssd1306_command(SSD1306_SETSTARTLINE | 0x0);            // line #0
    ssd1306_command(SSD1306_CHARGEPUMP);                    // 0x8D
    if (vccstate == SSD1306_EXTERNALVCC) 
      { ssd1306_command(0x10); }
    else 
      { ssd1306_command(0x14); }
    ssd1306_command(SSD1306_MEMORYMODE);                    // 0x20
    ssd1306_command(0x00);                                  // 0x0 act like ks0108
    ssd1306_command(SSD1306_SEGREMAP | 0x1);
    ssd1306_command(SSD1306_COMSCANDEC);
    ssd1306_command(SSD1306_SETCOMPINS);                    // 0xDA
    ssd1306_command(0x12);
    ssd1306_command(SSD1306_SETCONTRAST);                   // 0x81
    if (vccstate == SSD1306_EXTERNALVCC) 
      { ssd1306_command(0x9F); }
    else 
      { ssd1306_command(0xCF); }
    ssd1306_command(SSD1306_SETPRECHARGE);                  // 0xd9
    if (vccstate == SSD1306_EXTERNALVCC) 
      { ssd1306_command(0x22); }
    else 
      { ssd1306_command(0xF1); }
    ssd1306_command(SSD1306_SETVCOMDETECT);                 // 0xDB
    ssd1306_command(0x40);
    ssd1306_command(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
    ssd1306_command(SSD1306_NORMALDISPLAY);                 // 0xA6
  #endif
  
  ssd1306_command(SSD1306_DISPLAYON);//--turn on oled panel
  
  
  glcd_bbox.x_min = 0;
  glcd_bbox.y_min = 0;
  glcd_bbox.x_max = 128;
  glcd_bbox.y_max = 32;

  
  //**( vinod
  memcpy(fb_f.init, inits, 15);
  glcd_select_screen(fb_f.fb,  &glcd_bbox);
  //**)
}


// Invert so black text on white background
void  invertDisplay(uint8_t i) {
  if (i) {
    ssd1306_command(SSD1306_INVERTDISPLAY);
  } else {
    ssd1306_command(SSD1306_NORMALDISPLAY);
  }
} // invertDisplay()


void  ssd1306_command(uint8_t c) { 
    uint8_t control = 0x00;   // Co = 0, D/C = 0
    //I2c.write( _i2caddr, control, c);
    
    uint8_t data[2];
    data[0] = control;
    data[1] = c;
    i2cmh.transfer.data = data;
    i2cmh.transfer.dataSize = 2;
    i2cmh.transfer.direction = kI2C_Write;
    i2cmh.transfer.slaveAddress = 0x3c;
    I2C_MasterTransferBlocking(I2C0, &(i2cmh.transfer));
//    while(!i2c_transfer_done);
//    i2c_transfer_done = 0;
} // ssd1306_command()


// startscrollright
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F) 
void  startscrollright(uint8_t start, uint8_t stop){
	ssd1306_command(SSD1306_RIGHT_HORIZONTAL_SCROLL);
	ssd1306_command(0X00);
	ssd1306_command(start);
	ssd1306_command(0X00);
	ssd1306_command(stop);
	ssd1306_command(0X01);
	ssd1306_command(0XFF);
	ssd1306_command(SSD1306_ACTIVATE_SCROLL);
} // startscrollright()


// startscrollleft
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F) 
void  startscrollleft(uint8_t start, uint8_t stop){
	ssd1306_command(SSD1306_LEFT_HORIZONTAL_SCROLL);
	ssd1306_command(0X00);
	ssd1306_command(start);
	ssd1306_command(0X00);
	ssd1306_command(stop);
	ssd1306_command(0X01);
	ssd1306_command(0XFF);
	ssd1306_command(SSD1306_ACTIVATE_SCROLL);
} // startscrollleft()


// startscrolldiagright
// Activate a diagonal scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F) 
void  startscrolldiagright(uint8_t start, uint8_t stop){
	ssd1306_command(SSD1306_SET_VERTICAL_SCROLL_AREA);	
	ssd1306_command(0X00);
	ssd1306_command(SSD1306_LCDHEIGHT);
	ssd1306_command(SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
	ssd1306_command(0X00);
	ssd1306_command(start);
	ssd1306_command(0X00);
	ssd1306_command(stop);
	ssd1306_command(0X01);
	ssd1306_command(SSD1306_ACTIVATE_SCROLL);
} // startscrolldiagright()


// startscrolldiagleft
// Activate a diagonal scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F) 
void  startscrolldiagleft(uint8_t start, uint8_t stop){
	ssd1306_command(SSD1306_SET_VERTICAL_SCROLL_AREA);	
	ssd1306_command(0X00);
	ssd1306_command(SSD1306_LCDHEIGHT);
	ssd1306_command(SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
	ssd1306_command(0X00);
	ssd1306_command(start);
	ssd1306_command(0X00);
	ssd1306_command(stop);
	ssd1306_command(0X01);
	ssd1306_command(SSD1306_ACTIVATE_SCROLL);
} // startscrolldiagleft()


void  stopscroll(void){
	ssd1306_command(SSD1306_DEACTIVATE_SCROLL);
} // stopscroll()_


void  ssd1306_data(uint8_t c) {
    uint8_t control = 0x40;   // Co = 0, D/C = 1
    //I2c.write(_i2caddr, control, c);
    uint8_t data[2];
    data[0] = control;
    data[1] = c;
    i2cmh.transfer.data = data;
    i2cmh.transfer.dataSize = 2;
    i2cmh.transfer.direction = kI2C_Write;
    i2cmh.transfer.slaveAddress = 0x3c;
    I2C_MasterTransferBlocking(I2C0, &(i2cmh.transfer));
    //while(!i2c_transfer_done);
    //i2c_transfer_done = 0;
}  // ssd1306_data()


void  glcd_write(void) {
//  ssd1306_command(SSD1306_COLUMNADDR);
//  ssd1306_command(0);                // Column start address (0 = reset)
//  ssd1306_command(127);              // Column end address (127 = reset)
//  ssd1306_command(SSD1306_PAGEADDR);
//  ssd1306_command(0);                // Page start address (0 = reset)
//  ssd1306_command((SSD1306_LCDHEIGHT == 64) ? 7 : 3); // Page end address
//  
//  
//  // save I2C bitrate
////  uint8_t twbrbackup = TWBR;
////  TWBR = 12; // upgrade to 400KHz!
//
//  uint8_t control = 0x40;  // Co bit = 0 which means following info will be data, 
//                           // D/C bit = 1,   tells LCD the following data bytes should be stored at the GDDRAM. 
//                           // The GDDRAM column address pointer will be increased by one automatically after each data write. 
//  // Send buffer to LCD
//////  for (uint16_t i = 0; i < (SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8); i++) {
//////    //    I2c.write(_i2caddr, control, buffer[i]);
//////    uint8_t data[2];
//////    data[0] = control;
//////    data[1] = glcd_buffer[i];
//////    i2cmh.transfer.data = data;
//////    i2cmh.transfer.dataSize = 2;
//////    i2cmh.transfer.direction = kI2C_Write;
//////    i2cmh.transfer.slaveAddress = 0x3c;
//////    I2C_MasterTransferNonBlocking(I2C0, &i2cmh, &(i2cmh.transfer));    
//////    while(!i2c_transfer_done);
//////    i2c_transfer_done = 0;
//////  }
//
//
//////  for (uint16_t i = 0; i < ( SSD1306_LCDHEIGHT ); i++) {
//////    uint8_t data[SSD1306_LCDWIDTH/8 + 1];
//////  
//////    data[0] = control;
//////    memcpy((void *)&data[1], &glcd_buffer[i * SSD1306_LCDWIDTH/8 ], SSD1306_LCDWIDTH/8);
//////    i2cmh.transfer.data = data;
//////    i2cmh.transfer.dataSize = sizeof(data);
//////    i2cmh.transfer.direction = kI2C_Write;
//////    i2cmh.transfer.slaveAddress = 0x3c;
//////    I2C_MasterTransferNonBlocking(I2C0, &i2cmh, &(i2cmh.transfer));    
//////    while(!i2c_transfer_done);
//////    i2c_transfer_done = 0;
//////  }
//
//  //for (uint16_t i = 0; i < ( SSD1306_LCDHEIGHT ); i++) {
//    
//    
//  
// __packed struct {
//    uint8_t control;
//    uint8_t data[16*32];
//  } data_buff;
//  
//  
//    
//    
//    //for(int i = 0; i < SSD1306_LCDHEIGHT; i++) {
//      data_buff.control = control;
//      memcpy(data_buff.data, &glcd_buffer, 16*32);
//    //}
//    
//    
////    for(int i = 0; i < 32; i++) {
//      i2cmh.transfer.data = (void *)&data_buff;
//      i2cmh.transfer.dataSize = 16*32 + 1;
//      i2cmh.transfer.direction = kI2C_Write;
//      i2cmh.transfer.slaveAddress = 0x3c;
//      I2C_MasterTransferNonBlocking(I2C0, &i2cmh, &(i2cmh.transfer));    
//      while(!i2c_transfer_done);
//      i2c_transfer_done = 0;
////    }
//    //}
//  
//
//  
//  ssd1306_command(SSD1306_COLUMNADDR);
//  ssd1306_command(0);                // Column start address (0 = reset)
//  ssd1306_command(127);              // Column end address (127 = reset)
//  ssd1306_command(SSD1306_PAGEADDR);
  //  ssd1306_command(0);                // Page start address (0 = reset)
  //  ssd1306_command((SSD1306_LCDHEIGHT == 64) ? 7 : 3); // Page end address
//  }
  
  

   //while(i2c_bulk_push);
 //  i2c_transfer_done = 0;

  
  
  i2cmh.transfer.data = (void *)&fb_f;
  i2cmh.transfer.dataSize = sizeof(fb_f);
  i2cmh.transfer.direction = kI2C_Write;
  i2cmh.transfer.slaveAddress = 0x3c; 
  while(i2c_bulk_push);
  I2C_MasterTransferNonBlocking(I2C0, &i2cmh, &(i2cmh.transfer));    
  i2c_bulk_push = 1;    
// I2C_MasterTransferBlocking(I2C0, &(i2cmh.transfer));    



  
//  while(i2c_dma_running);
// I2C_MasterTransferEDMA( I2C0, &g_m_dma_handle, &(i2cmh.transfer));
//  i2c_dma_running = 1;

  
  
  //  while(!i2c_dma_complete);
//  i2c_dma_complete = 0;
  
  
  // i2c_bulk_push = 1;
 // while(!i2c_transfer_done);

  
  
  
//  TWBR = twbrbackup;
} // display()

//
//// clear everything
//void  clearDisplay(void) {
//  me6mset(glcd_buffer, 0, (SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/8));
//}


