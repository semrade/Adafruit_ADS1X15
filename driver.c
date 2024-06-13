#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "driver.h"



/*
** Function prototypes.
*/ 
static int  I2C_Read(unsigned char *out_buf, unsigned int len );
static int  I2C_Write(unsigned char *buf, unsigned int len );


/*
** Variable to store Line Number and Cursor Position.
*/ 
static uint8_t SSD1306_LineNum   = 0;
static uint8_t SSD1306_CursorPos = 0;
static uint8_t SSD1306_FontSize  = SSD1306_DEF_FONT_SIZE;

int i2c_fd= -1;;
int add = 0x3c;
int speed = 400000; // 200kHz (Fast Mode)

 
/*
** This function writes the data into the I2C client
**
**  Arguments:
**      buff -> buffer to be sent
**      len  -> Length of the data
**   
*/
static int I2C_Write(unsigned char *buf, unsigned int len)
{
  /*
  ** Sending Start condition, Slave address with R/W bit, 
  ** ACK/NACK and Stop condtions will be handled internally.
  */ 
  
    if (i2c_fd < 0) {
        perror("Error opening I2C device");
        exit(1);
    }
    else
    {
      if(write(i2c_fd, buf, len)!=len)
      {
        perror("Failed to write data");
      }
    }
  return 1;
}
 
/*
** This function reads one byte of the data from the I2C client
**
**  Arguments:
**      out_buff -> buffer wherer the data to be copied
**      len      -> Length of the data to be read
** 
*/
static int I2C_Read(unsigned char *out_buf, unsigned int len)
{
  /*
  ** Sending Start condition, Slave address with R/W bit, 
  ** ACK/NACK and Stop condtions will be handled internally.
  */ 
  //int ret = i2c_master_recv(etx_i2c_client_oled, out_buf, len);
  
  return 0;
}



/*
** This function is specific to the SSD_1306 OLED.
** This function sends the command/data to the OLED.
**
**  Arguments:
**      is_cmd -> true = command, flase = data
**      data   -> data to be written
** 
*/
void SSD1306_Write(bool is_cmd, unsigned char data)
{
  unsigned char buf[2] = {0};
  int ret;
  
  /*
  ** First byte is always control byte. Data is followed after that.
  **
  ** There are two types of data in SSD_1306 OLED.
  ** 1. Command
  ** 2. Data
  **
  ** Control byte decides that the next byte is, command or data.
  **
  ** -------------------------------------------------------                        
  ** |              Control byte's | 6th bit  |   7th bit  |
  ** |-----------------------------|----------|------------|    
  ** |   Command                   |   0      |     0      |
  ** |-----------------------------|----------|------------|
  ** |   data                      |   1      |     0      |
  ** |-----------------------------|----------|------------|
  ** 
  ** Please refer the datasheet for more information. 
  **    
  */ 
  if( is_cmd == true )
  {
      buf[0] = 0x00;
  }
  else
  {
      buf[0] = 0x40;
  }
  
  buf[1] = data;
  
  ret = I2C_Write(buf, 2);
}

/*
** This function is specific to the SSD_1306 OLED.
**
**  Arguments:
**      lineNo    -> Line Number
**      cursorPos -> Cursor Position
**   
*/
void SSD1306_SetCursor( uint8_t lineNo, uint8_t cursorPos )
{
  /* Move the Cursor to specified position only if it is in range */
  if((lineNo <= SSD1306_MAX_LINE) && (cursorPos < SSD1306_MAX_SEG))
  {
    SSD1306_LineNum   = lineNo;             // Save the specified line number
    SSD1306_CursorPos = cursorPos;          // Save the specified cursor position

    SSD1306_Write(true, 0x21);              // cmd for the column start and end address
    SSD1306_Write(true, cursorPos);         // column start addr
    SSD1306_Write(true, SSD1306_MAX_SEG-1); // column end addr

    SSD1306_Write(true, 0x22);              // cmd for the page start and end address
    SSD1306_Write(true, lineNo);            // page start addr
    SSD1306_Write(true, SSD1306_MAX_LINE);  // page end addr
  }
}

/*
** This function is specific to the SSD_1306 OLED.
** This function move the cursor to the next line.
**
**  Arguments:
**      none
** 
*/
void  SSD1306_GoToNextLine( void )
{
  /*
  ** Increment the current line number.
  ** roll it back to first line, if it exceeds the limit. 
  */
  SSD1306_LineNum++;
  SSD1306_LineNum = (SSD1306_LineNum & SSD1306_MAX_LINE);

  SSD1306_SetCursor(SSD1306_LineNum,0); /* Finally move it to next line */
}

/*
** This function is specific to the SSD_1306 OLED.
** This function sends the single char to the OLED.
**
**  Arguments:
**      c   -> character to be written
** 
*/
void SSD1306_PrintChar(unsigned char c)
{
  uint8_t data_byte;
  uint8_t temp = 0;

  /*
  ** If we character is greater than segment len or we got new line charcter
  ** then move the cursor to the new line
  */ 
  if( (( SSD1306_CursorPos + SSD1306_FontSize ) >= SSD1306_MAX_SEG ) ||
      ( c == '\n' )
  )
  {
    SSD1306_GoToNextLine();
  }

  // print charcters other than new line
  if( c != '\n' )
  {
  
    /*
    ** In our font array (SSD1306_font), space starts in 0th index.
    ** But in ASCII table, Space starts from 32 (0x20).
    ** So we need to match the ASCII table with our font table.
    ** We can subtract 32 (0x20) in order to match with our font table.
    */
    c -= 0x20;  //or c -= ' ';

    do
    {
      data_byte= SSD1306_font[c][temp]; // Get the data to be displayed from LookUptable

      SSD1306_Write(false, data_byte);  // write data to the OLED
      SSD1306_CursorPos++;
      
      temp++;
      
    } while ( temp < SSD1306_FontSize);
    SSD1306_Write(false, 0x00);         //Display the data
    SSD1306_CursorPos++;
  }
}

/*
** This function is specific to the SSD_1306 OLED.
** This function sends the string to the OLED.
**
**  Arguments:
**      str   -> string to be written
** 
*/
void SSD1306_String(unsigned char *str)
{
  while(*str)
  {
    SSD1306_PrintChar(*str++);
  }
}

/*
** This function is specific to the SSD_1306 OLED.
** This function inverts the display.
**
**  Arguments:
**      need_to_invert   -> true  - invert display
**                          false - normal display        
** 
*/
void SSD1306_InvertDisplay(bool need_to_invert)
{
  if(need_to_invert)
  {
    SSD1306_Write(true, 0xA7); // Invert the display
  }
  else
  {
    SSD1306_Write(true, 0xA6); // Normal display
  }
}

/*
** This function is specific to the SSD_1306 OLED.
** This function sets the brightness of  the display.
**
**  Arguments:
**      brightnessValue   -> true  - invert display  
** 
*/
void SSD1306_SetBrightness(uint8_t brightnessValue)
{
    SSD1306_Write(true, 0x81); // Contrast command
    SSD1306_Write(true, brightnessValue); // Contrast value (default value = 0x7F)
}

/*
** This function is specific to the SSD_1306 OLED.
** This function Scrolls the data right/left in horizontally.
**
**  Arguments:
**      is_left_scroll   -> true  - left horizontal scroll
                            false - right horizontal scroll
        start_line_no    -> Start address of the line to scroll 
        end_line_no      -> End address of the line to scroll                 
** 
*/
void SSD1306_StartScrollHorizontal( bool is_left_scroll,
                                           uint8_t start_line_no,
                                           uint8_t end_line_no
                                         )
{
  if(is_left_scroll)
  {
    // left horizontal scroll
    SSD1306_Write(true, 0x27);
  }
  else
  {
    // right horizontal scroll 
    SSD1306_Write(true, 0x26);
  }
  
  SSD1306_Write(true, 0x00);            // Dummy byte (dont change)
  SSD1306_Write(true, start_line_no);   // Start page address
  SSD1306_Write(true, 0x00);            // 5 frames interval
  SSD1306_Write(true, end_line_no);     // End page address
  SSD1306_Write(true, 0x00);            // Dummy byte (dont change)
  SSD1306_Write(true, 0xFF);            // Dummy byte (dont change)
  SSD1306_Write(true, 0x2F);            // activate scroll
}

/*
** This function is specific to the SSD_1306 OLED.
** This function Scrolls the data in vertically and right/left horizontally(Diagonally).
**
**  Arguments:
**      is_vertical_left_scroll   -> true  - vertical and left horizontal scroll
**                                   false - vertical and right horizontal scroll
**      start_line_no             -> Start address of the line to scroll 
**      end_line_no               -> End address of the line to scroll 
**      vertical_area             -> Area for vertical scroll (0-63)
**      rows                      -> Number of rows to scroll vertically             
** 
*/
void SSD1306_StartScrollVerticalHorizontal( bool is_vertical_left_scroll,
                                                   uint8_t start_line_no,
                                                   uint8_t end_line_no,
                                                   uint8_t vertical_area,
                                                   uint8_t rows
                                                 )
{
  
  SSD1306_Write(true, 0xA3);            // Set Vertical Scroll Area
  SSD1306_Write(true, 0x00);            // Check datasheet
  SSD1306_Write(true, vertical_area);   // area for vertical scroll
  
  if(is_vertical_left_scroll)
  {
    // vertical and left horizontal scroll
    SSD1306_Write(true, 0x2A);
  }
  else
  {
    // vertical and right horizontal scroll 
    SSD1306_Write(true, 0x29);
  }
  
  SSD1306_Write(true, 0x00);            // Dummy byte (dont change)
  SSD1306_Write(true, start_line_no);   // Start page address
  SSD1306_Write(true, 0x00);            // 5 frames interval
  SSD1306_Write(true, end_line_no);     // End page address
  SSD1306_Write(true, rows);            // Vertical scrolling offset
  SSD1306_Write(true, 0x2F);            // activate scroll
}
 
 
/*
** This function sends the commands that need to used to Initialize the OLED.
**
**  Arguments:
**      none
** 
*/
int SSD1306_DisplayInit(void)
{
  // Open the I2C devive
  if ((i2c_fd = open (I2C_DEVICE, O_RDWR))<0)
  {
    perror("Failed to open i2c bus");
    return 1;
  }

  // Sepcify the I2C slave address
  if (ioctl(i2c_fd, I2C_SLAVE, add)<0)
  {
    perror("Faild to aquire bus access and/or talk to slave");
    close(i2c_fd);
    return 1;
  }
  
  sleep(0.1);               // delay

  /*
  ** Commands to initialize the SSD_1306 OLED Display
  */
  SSD1306_Write(true, 0xAE); // Entire Display OFF
  SSD1306_Write(true, 0xD5); // Set Display Clock Divide Ratio and Oscillator Frequency
  SSD1306_Write(true, 0x80); // Default Setting for Display Clock Divide Ratio and Oscillator Frequency that is recommended
  SSD1306_Write(true, 0xA8); // Set Multiplex Ratio
  SSD1306_Write(true, 0x3F); // 64 COM lines
  SSD1306_Write(true, 0xD3); // Set display offset
  SSD1306_Write(true, 0x00); // 0 offset
  SSD1306_Write(true, 0x40); // Set first line as the start line of the display
  SSD1306_Write(true, 0x8D); // Charge pump
  SSD1306_Write(true, 0x14); // Enable charge dump during display on
  SSD1306_Write(true, 0x20); // Set memory addressing mode
  SSD1306_Write(true, 0x00); // Horizontal addressing mode
  SSD1306_Write(true, 0xA1); // Set segment remap with column address 127 mapped to segment 0
  SSD1306_Write(true, 0xC8); // Set com output scan direction, scan from com63 to com 0
  SSD1306_Write(true, 0xDA); // Set com pins hardware configuration
  SSD1306_Write(true, 0x12); // Alternative com pin configuration, disable com left/right remap
  SSD1306_Write(true, 0x81); // Set contrast control
  SSD1306_Write(true, 0x80); // Set Contrast to 128
  SSD1306_Write(true, 0xD9); // Set pre-charge period
  SSD1306_Write(true, 0xF1); // Phase 1 period of 15 DCLK, Phase 2 period of 1 DCLK
  SSD1306_Write(true, 0xDB); // Set Vcomh deselect level
  SSD1306_Write(true, 0x20); // Vcomh deselect level ~ 0.77 Vcc
  SSD1306_Write(true, 0xA4); // Entire display ON, resume to RAM content display
  SSD1306_Write(true, 0xA6); // Set Display in Normal Mode, 1 = ON, 0 = OFF
  SSD1306_Write(true, 0x2E); // Deactivate scroll
  SSD1306_Write(true, 0xAF); // Display ON in normal mode
  
  //Clear the display
  SSD1306_Fill(0x00);
  return 0;
}
 
/*
** This function Fills the complete OLED with this data byte.
**
**  Arguments:
**      data  -> Data to be filled in the OLED
** 
*/
void SSD1306_Fill(unsigned char data)
{
  unsigned int total  = 128 * 8;  // 8 pages x 128 segments x 8 bits of data
  unsigned int i      = 0;
  
  //Fill the Display
  for(i = 0; i < total; i++)
  {
      SSD1306_Write(false, data);
  }
}
 
 
/*
** This function getting called when the slave has been removed
** Note : This will be called only once when we unload the driver.
*/
int SSD1306_exitLcd( void )
{
  //Set cursor
  SSD1306_SetCursor(7,0);  
  
  //Write String to OLED
  //SSD1306_String("ThanK YoU!!!");
  
  sleep(1);
  
  //Set cursor
  SSD1306_SetCursor(0,0);  
  //clear the display
  SSD1306_Fill(0x00);
  
  SSD1306_Write(true, 0xAE); // Entire Display OFF

  close(i2c_fd);
  
  return 0;
}

