#include <stm32f10x.h>
#include <stm32f10x_i2c.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_dma.h>
#include <stm32f10x_rcc.h>
#include <math.h>
#include "bmp.h"
#include "spidma.h"
#include "lcdma.h"
#include "audiodma.h"
#include "ff.h"
#include "i2c.h"

#define NUNCHUCK_ADDRESS 0xA4

//General properties of the game sprites
typedef struct {
    int8_t x;
    int8_t y;
    int8_t height;
    int8_t width;
} sprite;

//Needed for "preloading" the player's ship
//struct bmppixel shipdata[425];

//Receive and transfer buffers
struct bmppixel receive[128] = {0};
uint16_t transfer[128] = {0};

//Needed for bmp loading (structs defined in bmp.h)
struct bmpfile_magic magic = {0};
struct bmpfile_header header = {0};
struct bmppixel pixel24 = {0};
uint16_t pixel16 = 0;
BITMAPINFOHEADER info = {0};

uint16_t r = 0;
uint16_t g = 0;
uint16_t b = 0;


//Shifts the joystick values from the nunchuck into a more manageable range
int joyShift(int joyVal) {
    return (joyVal / 30);
}

//Shifts the joystick values from the nunchuck into a more manageable range
int joyShift2(int joyVal) {
    return (joyVal / 20);
}

void Beep(int length) {
    audioplayerInit(44100);
    audioplayerStart();
    Delay(length);
    audioplayerStop();
}

//Generally used for loading full screen background images
void loadBackground(char *img, int x, int y) {

    //FatFS variables
    FATFS Fatfs;
    FIL Fil;
    FRESULT f;				/* Result code */
    DIR dir;				/* Directory object */
    FILINFO fno;			/* File information object */
    UINT bw, br;

    f =  f_open(&Fil, img, FA_READ);
    f =  f_read(&Fil, &magic, sizeof magic, &br);
    f =  f_read(&Fil, &header, sizeof header, &br);
    f =  f_read(&Fil, &info, sizeof info, &br);

    int i, j;
    for (i = 0; i < info.height; i++) {
        f = f_read(&Fil, &receive,sizeof receive, &br);
        for (j = 0; j < info.width; j++) {
            pixel24 = receive[j];
            r = (pixel24.r >> 3) << 11;
            g = (pixel24.g >> 2) << 5;
            b = (pixel24.b >> 3);
            pixel16 = r + g + b;
            transfer[j] = pixel16;
        }
        imageLine(x, y + i, info.width, transfer);
    }
    f = f_close(&Fil);
}

//Preloads the player's ship
/*void loadShip(char *img) {
  //FatFS variables
	FATFS Fatfs;
	FIL Fil;
  FRESULT f;
  DIR dir;
  FILINFO fno;
  UINT bw, br;

  f =  f_open(&Fil, img, FA_READ);
  f =  f_read(&Fil, &magic, sizeof magic, &br);
  f =  f_read(&Fil, &header, sizeof header, &br);
  f =  f_read(&Fil, &info, sizeof info, &br);
  f = f_read(&Fil, &shipdata,sizeof shipdata, &br);
  f = f_close(&Fil);
}*/

void main() {

    //Start system core clock
    if (SysTick_Config(SystemCoreClock / 1000))
        while (1);

    //Nunchuck instructions
    const uint8_t inst1[] = {0xf0, 0x55};
    const uint8_t inst2[] = {0xfb, 0x00};
    const uint8_t reset[] = {0};

    //X and Y positions of the joysticks
    int x1 = ST7735_width / 2 - 10;
    int y1 = ST7735_height / 2 - 15;
    int x2 = ST7735_width / 2;
    int y2 = ST7735_height / 2;

    //Game state
    uint8_t crashes = 0;
    uint16_t shield = 0;
    uint16_t sfull = 50;
    uint8_t maxCrashes = 7;
    uint16_t score = 0;
    uint8_t haxblox = 0;
    uint8_t victory = 0;

    //Sprite data
    sprite ship = {0};
    sprite reticule = {0};
    sprite damage = {0};
    sprite hax = {0};
    sprite ob1 = {0};
    sprite ob2 = {0};
    sprite ob3 = {0};
    sprite ob4 = {0};

    //Initialize the data for the falling obstacles
    ob1.x = 25;
    ob1.y = 0;
    ob1.height = 8;
    ob1.width = 5;

    ob2.x = 55;
    ob2.y = 0;
    ob2.height = 8;
    ob2.width = 5;

    ob3.x = 75;
    ob3.y = 0;
    ob3.height = 8;
    ob3.width = 5;

    ob4.x = 115;
    ob4.y = 0;
    ob4.height = 8;
    ob4.width = 5;

    //Initialize the data for the player's ship
    ship.x = x1;
    ship.y = x2;
    ship.height = 30;
    ship.width = 20;

    //Initialize the data for the damage blocks
    damage.x = 3;
    damage.y = 5;
    damage.height = 12;
    damage.width = 6;

    //Initialize the data for the hax blocks
    hax.x = 10;
    hax.y = 5;
    hax.height = 12;
    hax.width = 6;

    //Initialize the data for the targeting reticule
    reticule.x = x2;
    reticule.y = x2;
    reticule.height = 5;
    reticule.width = 5;

    //X and Y values of the joysticks
    int8_t jx1 = 0;
    int smoothjx1 = 0;
    int8_t jy1 = 0;
    int smoothjy1 = 0;

    int8_t jx2 = 0;
    int smoothjx2 = 0;
    int8_t jy2 = 0;
    int smoothjy2 = 0;

    //Nunchuck button statuses
    int zNotPressed1 = 0;
    int cNotPressed1 = 0;
    int zNotPressed2 = 0;
    int cNotPressed2 = 0;

    //Nunchuck data arrays
    uint8_t data1[6] = {0};
    uint8_t data2[6] = {0};

    //Initialize some counters
    int smooth = 0;
    int image = 0;
    int k = 0;
    char *strs[6] = {"splash.bmp", "intro.bmp", "imprt.bmp", "message.bmp", "instruct.bmp"};


    // Initialize I2C busses
    I2C_LowLevel_Init(I2C1, 10000, NUNCHUCK_ADDRESS);
    I2C_LowLevel_Init(I2C2, 10000, NUNCHUCK_ADDRESS);

    // Initialize the nunchucks
    I2C_Write(I2C1, inst1, 2, NUNCHUCK_ADDRESS);
    I2C_Write(I2C1, inst2, 2, NUNCHUCK_ADDRESS);

    I2C_Write(I2C2, inst1, 2, NUNCHUCK_ADDRESS);
    I2C_Write(I2C2, inst2, 2, NUNCHUCK_ADDRESS);

    //Initialize SPI and the LCD
    spiInit(SPI2);
    ST7735_init();

    //Fill the screen with black
    fillScreen(0x0000);

    //Mount the file system
    FATFS Fatfs;
    f_mount(0, &Fatfs);

    //Intro screens
    int i;
    for (i = 0; i < 5; i++) {
        //Clear nunchuck data
        I2C_Write(I2C1, reset, 1, NUNCHUCK_ADDRESS);
        I2C_Write(I2C2, reset, 1, NUNCHUCK_ADDRESS);

        loadBackground(strs[i], 0 , 0);
        Delay(200);
        while (1) {
            Delay(100);
            //Read nunchuck data
            I2C_Write(I2C1, reset, 1, NUNCHUCK_ADDRESS);
            Delay(100);
            I2C_Read(I2C1, data1, 6, NUNCHUCK_ADDRESS);

            // Update the screen based on the button presses
            cNotPressed1 = ((data1[5] >> 1) & 0x1);

            if (!cNotPressed1) {
                break;
            }
        }
    }

    uint8_t loadingX = 60;
    uint8_t loadingY = 50;
    loadBackground("game.bmp", 0 , 0);
    writeCharDMA(GREEN, BLACK, '3', loadingX, loadingY);
    Delay(1000);
    writeCharDMA(BLACK, BLACK, '3', loadingX, loadingY);
    writeCharDMA(GREEN, BLACK, '2', loadingX, loadingY);
    Delay(1000);
    writeCharDMA(BLACK, BLACK, '2', loadingX, loadingY);
    writeCharDMA(GREEN, BLACK, '1', loadingX, loadingY);
    Delay(1000);
    writeCharDMA(BLACK, BLACK, '1', loadingX, loadingY);
    //loadShip("spijet.bmp");


    //Start the game!
    while(1) {
        smooth++;


        //THis shield is only activated when a player crashes
        if (shield > 0) {
            shield--;
        }

        // Read the nunchuck to get the accelerometer, joystick, and button data
        I2C_Write(I2C1, reset, 1, NUNCHUCK_ADDRESS);
        I2C_Read(I2C1, data1, 6, NUNCHUCK_ADDRESS);

        I2C_Write(I2C2, reset, 1, NUNCHUCK_ADDRESS);
        I2C_Read(I2C2, data2, 6, NUNCHUCK_ADDRESS);

        /* Interpret the data and collect it over several iterations to average them later*/

        // Get JX
        jx1 = (data1[0]-128);
        smoothjx1 += jx1;

        jx2 = (data2[0]-128);
        smoothjx2 += jx2;

        // Get JY
        jy1 = (data1[1]-128);
        smoothjy1 += jy1;

        jy2 = (data2[1]-128);
        smoothjy2 += jy2;

        // Get C and Z button status
        /*cNotPressed1 = ((data1[5] >> 1) & 0x1);
        zNotPressed1 = data1[5] & 0x1;

        	cNotPressed2 = ((data2[5] >> 1) & 0x1);
        zNotPressed2 = data2[5] & 0x1;*/

        if (smooth % 5 == 0) {

            //Manipulate the score
            if (score % 100 == 0) {
                haxblox++;
                fillBlockColor(hax.x, ST7735_height - (haxblox * 20), hax.height, hax.width, GREEN);
            }

            //Clear the ship
            fillBlockColor(ship.x, ship.y, ship.height, ship.width, BLACK);

            //Clear the reticule
            fillBlockColor(x2, y2, 5, 5, BLACK);

            //Clear falling objects
            writeCharDMA(BLACK, BLACK, '0', ob1.x, ob1.y);
            writeCharDMA(BLACK, BLACK, '1', ob2.x, ob2.y);
            writeCharDMA(BLACK, BLACK, '0', ob3.x, ob3.y);
            writeCharDMA(BLACK, BLACK, '1', ob4.x, ob4.y);

            //Calculate pitch and roll by averaging the values of the previous iterations
            smoothjx1 = smoothjx1 / smooth;
            smoothjy1 = smoothjy1 / smooth;

            smoothjx2 = smoothjx2 / smooth;
            smoothjy2 = smoothjy2 / smooth;

            //Joystick position
            x1 += joyShift(smoothjx1);
            y1 += joyShift(smoothjy1);
            ship.x = x1;
            ship.y = y1;

            x2 += joyShift2(smoothjx2);
            y2 += joyShift2(smoothjy2);
            reticule.x = x2;
            reticule.y = y2;

            //Modify falling object positions
            ob1.y += 4;
            ob2.y += 2;
            ob3.y += 3;
            ob4.y += 5;


            // Boundary checking for the ship (x1,y1)

            //Screen border checking
            if (ship.x < 20) {
                x1 = 20;
                ship.x = x1;
            } else if ((x1 + 20) > ST7735_width - 1) {
                x1 = ST7735_width - 20;
                ship.x = x1;
            }
            if (y1 < 0) {
                y1 = 0;
                ship.y = y1;
            } else if ((y1 + 30) > ST7735_height - 1) {
                y1 = ST7735_height - 30;
                ship.y = y1;
            }

            //Collision detection for the ship with various objects
            //Certain things need to be inverted to account for reverse loading of BMP vs regular graphics
            if ((ship.x + ship.width > ob1.x) && (ship.x < (ob1.x + ob1.width))
                    && (ship.y + ship.height > 160 - ob1.y) && (ship.y < 160 - (ob1.y + ob1.height))) {
                //writeString(RED, BLACK, "CRASH 1", 0, 20);
                if (shield == 0) {
                    crashes++;
                    fillBlockColor(damage.x, ST7735_height - (crashes * 20), damage.height, damage.width, RED);
                    shield = sfull;
                    Beep(250);
                }
            } else //writeString(BLACK, BLACK, "CRASH 1", 0, 20);

                if ((ship.x + ship.width > ob2.x) && (ship.x < (ob2.x + ob2.width))
                        && (ship.y + ship.height > 160 - ob2.y) && (ship.y < 160 - (ob2.y + ob2.height))) {
                    //writeString(RED, BLACK, "CRASH 2", 0, 40);
                    if (shield == 0) {
                        crashes++;
                        fillBlockColor(damage.x, ST7735_height - (crashes * 20), damage.height, damage.width, RED);
                        shield = sfull;
                        Beep(250);
                    }
                } else //writeString(BLACK, BLACK, "CRASH 2", 0, 40);

                    if ((ship.x + ship.width > ob3.x) && (ship.x < (ob3.x + ob3.width))
                            && (ship.y + ship.height > 160 - ob3.y) && (ship.y < 160 - (ob3.y + ob3.height))) {
                        //writeString(RED, BLACK, "CRASH 3", 0, 60);
                        if (shield == 0) {
                            crashes++;
                            fillBlockColor(damage.x, ST7735_height - (crashes * 20), damage.height, damage.width, RED);
                            shield = sfull;
                            Beep(250);
                        }
                    } else //writeString(BLACK, BLACK, "CRASH 3", 0, 60);

                        if ((ship.x + ship.width > ob4.x) && (ship.x < (ob4.x + ob4.width))
                                && (ship.y + ship.height > 160 - ob4.y) && (ship.y < 160 - (ob4.y + ob4.height))) {
                            //writeString(RED, BLACK, "CRASH 4", 0, 60);
                            if (shield == 0) {
                                crashes++;
                                fillBlockColor(damage.x, ST7735_height - (crashes * 20), damage.height, damage.width, RED);
                                shield = sfull;
                                Beep(250);
                            }
                        } else //writeString(BLACK, BLACK, "CRASH 4", 0, 80);

                            /*//Reticule collisions
                            if ((reticule.x + reticule.width > ob1.x) && (reticule.x < (ob1.x + ob1.width))
                            		&& (reticule.y + reticule.height > ob1.y) && (reticule.y < (ob1.y + ob1.height))) {
                            	writeString(RED, BLACK, "CRASH 1", 60, 20);
                            }
                            else writeString(BLACK, BLACK, "CRASH 1", 60, 20);

                            if ((reticule.x + reticule.width > ob2.x) && (reticule.x < (ob2.x + ob2.width))
                            		&& (reticule.y + reticule.height > ob2.y) && (reticule.y < (ob2.y + ob2.height))) {
                            	writeString(RED, BLACK, "CRASH 2", 60, 40);
                            }
                            else writeString(BLACK, BLACK, "CRASH 2", 60, 40);

                            if ((reticule.x + reticule.width > ob3.x) && (reticule.x < (ob3.x + ob3.width))
                            		&& (reticule.y + reticule.height > ob3.y) && (reticule.y < (ob3.y + ob3.height))) {
                            	writeString(RED, BLACK, "CRASH 3", 60, 60);
                            }
                            else writeString(BLACK, BLACK, "CRASH 3", 60, 60);

                            if ((reticule.x + reticule.width > ob4.x) && (reticule.x < (ob4.x + ob4.width))
                            		&& (reticule.y + reticule.height > ob4.y) && (reticule.y < (ob4.y + ob4.height))) {
                            	writeString(RED, BLACK, "CRASH 4", 60, 60);
                            }
                            else writeString(BLACK, BLACK, "CRASH 4", 60, 80);*/

                            if ((reticule.x + reticule.width > ship.x) && (reticule.x < (ship.x + ship.width))
                                    && (reticule.y + reticule.height > ship.y) && (reticule.y < (ship.y + ship.height))) {
                                //writeString(RED, BLACK, "CRASH Ret", 0, 0);
                            } else //writeString(BLACK, BLACK, "CRASH Ret", 0, 0);

                                //Boundary checking for the reticule (x2,y2)
                                if (x2 < 20) {
                                    x2 = 20;
                                    reticule.x = x2;
                                } else if ((x2 + 5) > ST7735_width - 1) {
                                    x2 = ST7735_width - 5;
                                    reticule.x = x2;
                                }
            if (y2 < 0) {
                y2 = 0;
                reticule.y = y2;
            } else if ((y2 + 17) > ST7735_height - 1) {
                y2 = ST7735_height - 17;
                reticule.y = y2;
            }

            //Boundary checking for falling objects
            if ((ob1.y + ob1.height) > ST7735_height - 1) {
                ob1.y = 0;
                //ob1.x = x1;
            }

            if ((ob2.y + ob2.height) > ST7735_height - 1) {
                ob2.y = 0;
                //ob2.x = x2;
            }

            if ((ob3.y + ob3.height) > ST7735_height - 1) {
                ob3.y = 0;
                //ob3.x = x2 - x1;
            }

            if ((ob4.y + ob4.height) > ST7735_height - 1) {
                ob4.y = 0;
                //ob4.x = x2 - x1;
            }


            //Game state checks
            if (crashes > maxCrashes) {
                victory = 0;
                break;
            }
            if (haxblox > 7) {
                victory = 1;
                break;
            }

            //Now we draw the objects to the screen

            //Place the ship
            //placeImage(shipdata, ship.x, ship.y, ship.height, ship.width);
            fillBlockColor(ship.x, ship.y, ship.height, ship.width, CYAN);

            //Place the targeting reticule
            fillBlockColor(reticule.x, reticule.y, reticule.height, reticule.width, BLUE);

            //Place the falling objects
            writeCharDMA(GREEN, BLACK, '0', ob1.x, ob1.y);
            writeCharDMA(GREEN, BLACK, '1', ob2.x, ob2.y);
            writeCharDMA(GREEN, BLACK, '0', ob3.x, ob3.y);
            writeCharDMA(GREEN, BLACK, '1', ob4.x, ob4.y);

            //Reset the iteration counter and the smoothed values; continue to the
            //next iteration
            smooth = 0;
            score++;
            smoothjx1 = 0;
            smoothjy1 = 0;
            smoothjx2 = 0;
            smoothjy2 = 0;
        }
    }

    if (victory) {
        loadBackground("victory.bmp", 0 , 0);
    }

    else {
        loadBackground("gameover.bmp", 0 , 0);
    }
    Beep(1000);

}

DWORD get_fattime (void) {
    return	  ((DWORD)(2012 - 1980) << 25)	/* Year = 2012 */
              | ((DWORD)1 << 21)      /* Month = 1 */
              | ((DWORD)1 << 16)	/* Day_m = 1*/
              | ((DWORD)0 << 11)	/* Hour = 0 */
              | ((DWORD)0 << 5)       /* Min = 0 */
              | ((DWORD)0 >> 1);      /* Sec = 0 */
}
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) {
    while(1);
}
#endif