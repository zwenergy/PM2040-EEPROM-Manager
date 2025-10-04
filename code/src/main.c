//-----------------------------------------------------------------------
//-- Title: EEPROM manager for PM2040 flash cart
//-- Author: zwenergy
//-----------------------------------------------------------------------

#include "pm.h"
#include "print.h"

#include <stdint.h>
#include <string.h>


#define PAGEX 90
#define PAGEY 7
#define indOffset 2

#define CURSORX 1
#define LABELX 8

#define DELAY 100
#define DELAYMAX 255

#define EEPROMSLOTS 3

// Addresses.
// Set EEPROM slot. 0b0100001111000001
#define REG_SLOT 0x43C1

// Set lower address. 0b0100001111000010
#define REG_LOADDR 0x43C2

// Set upper address. 0b0100001111000011
#define REG_HIADDR 0x43C3

// Write byte. 0b0100001111000100
#define REG_BYTE 0x43C4

// Flush temp buffer to RP2040 Flash. 0b0100001111000111
#define REG_STOREFLASH 0x43C7

#define MAXEEPROM 8191

// RP2040 EEPROM temp address.
#define CARTEEPROM 0x4000


volatile uint8_t flag;

const uint8_t readCmd = 0xA1;
const uint8_t writeCmd = 0xA0;

enum menu {
  MAIN,
  BACKUP,
  RESTORE
};

void delay( void ) {
  volatile cnt, dummy;
  
  // Very pretty delay function.
  for ( cnt = 0; cnt < DELAY; ++cnt ) {
    dummy++;
  }
}


void delayLong( void ) {
  volatile cnt, dummy;
  
  // Very pretty delay function.
  for ( cnt = 0; cnt < DELAYMAX; ++cnt ) {
    dummy++;
  }
  
  for ( cnt = 0; cnt < DELAYMAX; ++cnt ) {
    dummy++;
  }
  
  for ( cnt = 0; cnt < DELAYMAX; ++cnt ) {
    dummy++;
  }
  
  for ( cnt = 0; cnt < DELAYMAX; ++cnt ) {
    dummy++;
  }
  
  for ( cnt = 0; cnt < DELAYMAX; ++cnt ) {
    dummy++;
  }
  
  for ( cnt = 0; cnt < DELAYMAX; ++cnt ) {
    dummy++;
  }
}

void EEPROM_SCL_LO ( void ) {
  IO_DATA = ( IO_DATA & 0xF7 );
}

void EEPROM_SCL_HI ( void ) {
  IO_DATA = ( IO_DATA | 0x08 );
}

void EEPROM_SDA_HI ( void ) {
  IO_DATA = ( IO_DATA | 0x04 );
}

void EEPROM_SDA_LO ( void ) {
  IO_DATA = ( IO_DATA & 0xFB );
}


void setSCLOut( void ) {
  IO_DIR = IO_DIR | 0x08;
}
void setSCLIn( void ) {
  IO_DIR = IO_DIR & 0xF7;
}

void writeDir( void ) {
  IO_DIR = IO_DIR | 0x04;
}

void readDir( void ) {
  IO_DIR = IO_DIR & 0xFB;
}

void startCondition( void ) {
  // Start condition
  EEPROM_SDA_HI();
  EEPROM_SCL_HI();
  
  delay();
  
  EEPROM_SDA_LO();
  
  delay();
  
  EEPROM_SCL_LO();
}

void sendBusBit( uint8_t b ) {
  writeDir();
  // We assume the clock is high.
  
  // Set clock low.
  EEPROM_SCL_LO();  
  
  // Set the bit.
  if ( b ) {
    EEPROM_SDA_HI();
  } else {
    EEPROM_SDA_LO();
  }
  
  // Set the clock high.
  EEPROM_SCL_HI();
}


void sendByte( uint8_t b ) {
  int8_t i;
  
  writeDir();
  for( i = 7; i >= 0; --i ) {
    sendBusBit( b & ( 1 << i ) );
  }
  
  // Ack.
  EEPROM_SCL_LO();
  readDir();
  EEPROM_SCL_HI();
}

uint8_t readEEPROM( uint16_t a ) {
  uint8_t c, i;
  c = 0;
  
  // Set bus direction.
  IO_DIR = ( IO_DIR | 0x04 | 0x08 );

  // Set start condition.
  startCondition();
  
  // Send write command.
  sendByte( writeCmd );
  
  // Send upper address.
  sendByte( a >> 8 );
  
  // Send lower address.
  sendByte( a );

  EEPROM_SCL_LO();
  writeDir();
  EEPROM_SDA_HI();
  
  EEPROM_SCL_HI();
  
  // Start condition.
  startCondition();
  
  // Send read command.
  sendByte( readCmd );
  
  // Read one byte.
  for ( i = 0; i < 8; ++i ) {
    EEPROM_SCL_LO();
    EEPROM_SCL_HI();
    c = ( c | ( ( ( IO_DATA & 0x04 ) >> 2 ) << ( 7 - i ) ) );
  }
  
  // No ack.
  EEPROM_SCL_LO();
  writeDir();
  EEPROM_SDA_LO();
  EEPROM_SCL_HI();
  
  // Stop condition.
  EEPROM_SDA_HI();
  
  return c;
}

void readEEPROMBytes( uint16_t a, uint8_t* buffer, uint16_t bytes ) {
  uint8_t c, i;
  uint16_t byteCnt;
  
  // Set bus direction.
  IO_DIR = ( IO_DIR | 0x04 | 0x08 );

  // Set start condition.
  startCondition();
  
  // Send write command.
  sendByte( writeCmd );
  
  // Send upper address.
  sendByte( a >> 8 );
  
  // Send lower address.
  sendByte( a );

  EEPROM_SCL_LO();
  writeDir();
  EEPROM_SDA_HI();
  
  EEPROM_SCL_HI();
  
  // Start condition.
  startCondition();
  
  // Send read command.
  sendByte( readCmd );
  
  // Read data.
  for ( byteCnt = 0; byteCnt < bytes; ++byteCnt ) {
    c = 0;
    for ( i = 0; i < 8; ++i ) {
      EEPROM_SCL_LO();
      EEPROM_SCL_HI();
      c = ( c | ( ( ( IO_DATA & 0x04 ) >> 2 ) << ( 7 - i ) ) );
    }
    
    // Store byte.
    buffer[ byteCnt ] = c;
    
    if ( byteCnt != ( bytes - 1 ) ) {
      // Set ack.
      EEPROM_SCL_LO();
      writeDir();
      EEPROM_SDA_LO();
      EEPROM_SCL_HI();
      EEPROM_SCL_LO();
      readDir();
    }
  }
  
  // No ack.
  EEPROM_SCL_LO();
  writeDir();
  EEPROM_SDA_LO();
  EEPROM_SCL_HI();
  
  // Stop condition.
  EEPROM_SDA_HI();
}

void writeEEPROMByte( uint16_t a, uint8_t b ) {
  uint8_t c, i;
  c = 0;
  
  // Set bus direction.
  IO_DIR = ( IO_DIR | 0x04 | 0x08 );

  startCondition();
  
  // Send write command.
  sendByte( writeCmd );
  
  // Send upper address.
  sendByte( a >> 8 );
  
  // Send lower address.
  sendByte( a );
  
  // Send single data byte.
  sendByte( b );
  
  EEPROM_SCL_LO();
  writeDir();
  EEPROM_SDA_LO();
  
  EEPROM_SCL_HI();
  
  delay();
  
  // Stop condition.
  EEPROM_SDA_HI();
}

void copyToEEPROM() {
  uint8_t page, i, c;
  uint16_t addr;
  c = 0;
  i = 0;
  page = 0;
  addr = 0;
  
  // Go over page-wise.
  while( 1 ) {
    // Set bus direction.
    IO_DIR = ( IO_DIR | 0x04 | 0x08 );

    startCondition();
    
    // Send write command.
    sendByte( writeCmd );
    
    // Send upper address.
    sendByte( addr >> 8 );
    
    // Send lower address.
    sendByte( addr );
    
    // Send 32 bytes.
    for ( i = 0; i < 32; ++i ) {
      // Read byte from RP.
      c =  *( (uint8_t *) ( CARTEEPROM + addr ) );
      ++addr;
      
      sendByte( c );
    }
    
    EEPROM_SCL_LO();
    writeDir();
    EEPROM_SDA_LO();
    
    EEPROM_SCL_HI();
    
    delay();
    
    // Stop condition.
    EEPROM_SDA_HI();
    
    // Wait for write. This takes up to 5 ms.
    delayLong();
    
    if ( page == 255 ) {
      break;
    }
    ++page;
  }
}

void waitForFlash() {
  volatile cnt, cnt2, dummy;
  
  // Transfer to Flash.
  *( (uint8_t *) REG_STOREFLASH ) = 1;
  
  // Yes, very pretty. Very much calculated.
  for ( cnt = 0; cnt < DELAYMAX; ++cnt ) {
    dummy++;
    
    for ( cnt2 = 0; cnt2 < DELAYMAX; ++cnt2 ) {
      dummy++;
    }
  }
}

static uint8_t keyScan( void ) {
  uint8_t k = KEY_PAD;
  
  return k;
}

_interrupt( 2 ) void prc_frame_copy_irq(void)
{
  flag = 1;
  IRQ_ACT1 = IRQ1_PRC_COMPLETE;
}

enum menu curMenu = MAIN;

void drawMenu() {
  uint8_t i;
  
  // Empty screen.
  memset( 0x1000, 0, 96 * 8 );

  if ( curMenu == MAIN ) {
   print( 9, 0, "PM2040 EEPROM" );
   print( 8, 2, "Backup" );
   print( 8, 3, "Restore" );
   
  } else if ( curMenu == BACKUP ) {
    print( 5, 0, "Backup to slot" );
    
    for ( i = indOffset; i < EEPROMSLOTS + indOffset; ++i ) {
      printDigit( 8, i, ( i - indOffset ) );
    }
    
  } else if ( curMenu == RESTORE ) {
    print( 0, 0, "Restore slot?" );
    
    for ( i = indOffset; i < EEPROMSLOTS + indOffset; ++i ) {
      printDigit( 8, i, ( i - indOffset ) );
    }
    
  }
}

void waitForButton( void ) {
  
  while( !( ~keyScan() ) ) {
    
  }
}

uint8_t readBuffer[ 32 ];

int main(void)
{
  uint8_t keys, keysPrev, n;
  uint8_t cnt = 0;
  uint8_t i = 0;
  uint16_t addr = 0;
  char c;

  // Key interrupts priority
  PRI_KEY(0x03);

  // Enable interrupts for keys (only power)
  IRQ_ENA3 = IRQ3_KEYPOWER;

  // PRC interrupt priority
  PRI_PRC(0x01);

  // Enable PRC IRQ
  IRQ_ENA1 = IRQ1_PRC_COMPLETE;

  n = indOffset;

  drawMenu();
  print( CURSORX, n, ">" );
  
  for ( ;; ) {
    keysPrev = keys;
    keys = keyScan();

    
    if ( curMenu == MAIN ) {
      if ( !( keys & KEY_DOWN ) && ( keys != keysPrev  ) ) {
        if ( n < indOffset + 1 ) {
          print( CURSORX, n, " " );
          ++n;
          print( CURSORX, n, ">" );
        }
      }
      
      if ( !( keys & KEY_UP ) && ( keys != keysPrev  ) ) {
        if ( n > indOffset ) {
          print( CURSORX, n, " " );
          --n;
          print( CURSORX, n, ">" );
        }
      }
      
      if ( !( keys & KEY_A ) && ( keys != keysPrev  ) ) {
        if ( n == indOffset ) {
          curMenu = BACKUP;
        } else {
          curMenu = RESTORE;
        }
        
        n = indOffset;
        
        drawMenu();
        print( CURSORX, n, ">" );
      }
      
      
    } else if ( curMenu == BACKUP ) {
      if ( !( keys & KEY_DOWN ) && ( keys != keysPrev  ) ) {
        if ( n < EEPROMSLOTS + 1 ) {
          print( CURSORX, n, " " );
          ++n;
          print( CURSORX, n, ">" );
        }
      }
      
      if ( !( keys & KEY_UP ) && ( keys != keysPrev  ) ) {
        if ( n > indOffset ) {
          print( CURSORX, n, " " );
          --n;
          print( CURSORX, n, ">" );
        }
      }
      
      if ( !( keys & KEY_B ) && ( keys != keysPrev  ) ) {
        curMenu = MAIN;
        n = indOffset;
        drawMenu();
        print( CURSORX, n, ">" );
      }
      
      if ( !( keys & KEY_A ) && ( keys != keysPrev  ) ) {
        // Do backup.
        memset( 0x1000, 0, 96 * 8 );
        print( 8, 5, "DOING BACKUP" );
        
        // Set the slot.
        *( (uint8_t *) REG_SLOT ) = ( n - indOffset );
        
        // Copy bytes over. 32 bytes per read.
        addr = 0;
        while ( 1 ) {
          // Read 32 bytes.
          readEEPROMBytes( addr, readBuffer, 32 );
          
          // Transfer the read bytes.
          for ( i = 0; i < 32; ++i ) {
            // Set lower addr.
            *( (uint8_t *) REG_LOADDR ) = ( addr & 0xFF );
            
            // Set upper addr.
            *( (uint8_t *) REG_HIADDR ) = ( addr >> 8 );
            
            // Transfer byte.
            *( (uint8_t *) REG_BYTE ) = readBuffer[ i ];
            
            ++addr;
          }

          
          if ( ( addr - 1 ) == MAXEEPROM ) {
            break;
          }
          
        }
        
        // Transfer to Flash.
        *( (uint8_t *) REG_STOREFLASH ) = 1;
        
        // Show done.
        print( 8, 5, "DOING BACKUP... DONE" );
        
        // Wait for button press.
        waitForButton();
        
        curMenu = MAIN;
        n = indOffset;
        drawMenu();
        print( CURSORX, n, ">" );
        
      }
      
    } else if ( curMenu == RESTORE ) {
      if ( !( keys & KEY_DOWN ) && ( keys != keysPrev  ) ) {
        if ( n < EEPROMSLOTS + 1 ) {
          print( CURSORX, n, " " );
          ++n;
          print( CURSORX, n, ">" );
        }
      }
      
      if ( !( keys & KEY_UP ) && ( keys != keysPrev  ) ) {
        if ( n > indOffset ) {
          print( CURSORX, n, " " );
          --n;
          print( CURSORX, n, ">" );
        }
      }
      
      if ( !( keys & KEY_B ) && ( keys != keysPrev  ) ) {
        curMenu = MAIN;
        n = indOffset;
        drawMenu();
        print( CURSORX, n, ">" );
      }
      
      if ( !( keys & KEY_A ) && ( keys != keysPrev  ) ) {
        // Do restore.
        memset( 0x1000, 0, 96 * 8 );
        print( 0, 5, "DOING RESTORE" );
        
        // Set the slot.
        *( (uint8_t *) REG_SLOT ) = ( n - indOffset );
        
        // Copy bytes over.
        addr = 0;
        
        copyToEEPROM();
        
        // Show done.
        print( 0, 5, "DOING RESTORE... DONE" );
        
        waitForButton();
        
        curMenu = MAIN;
        n = indOffset;
        drawMenu();
        print( CURSORX, n, ">" );
      }
    }
    
  }
}
