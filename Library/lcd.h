#define LOW 0
#define HIGH 1

#define LCD_C LOW
#define LCD_D HIGH

#define LCD_PORT GPIOC
#define LCD_PORT_BKL GPIOA
#define SPILCD SPI2
#define LCDSPEED SPI_FAST

#define GPIO_PIN_BKL GPIO_Pin_1
#define GPIO_PIN_SCE GPIO_Pin_0
#define GPIO_PIN_RST GPIO_Pin_1
#define GPIO_PIN_DC GPIO_Pin_2

#define ST7735_CASET 0x2A
#define ST7735_RASET 0x2B
#define ST7735_MADCTL 0x36
#define ST7735_RAMWR 0x2C
#define ST7735_RAMRD 0x2E
#define ST7735_COLMOD 0x3A

#define MADCTLGRAPHICS 0x6
#define MADCTLBMP 0x2

#define ST7735_width 128
#define ST7735_height 160

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define MADVAL(x) (((x) << 5) | 8)
static uint8_t madctlcurrent = MADVAL(MADCTLGRAPHICS);

void Delay(uint32_t nTime);

void SysTick_Handler(void);

int abs(int x);

void swap(int *x, int *y);

void ST7735_setAddrWindow(uint16_t x0, uint16_t y0,
			  uint16_t x1, uint16_t y1,
			  uint8_t madctl);

void ST7735_pushColor(uint16_t *color, int cnt);

void fillScreen(uint16_t color);

void writePixel(int x, int y, uint16_t color);

void drawCircle(int x0, int y0, int r, uint16_t color);

void drawLine(int x0, int y0,int x1, int y1,uint16_t color);

void writeChar(uint16_t fg, uint16_t bg, char c, int row, int col);

void clearChar(uint16_t bg, int x, int y);

void writeString(uint16_t fg, uint16_t bg, char *str, int x, int y);

void ST7735_init();

void ST7735_backLight(uint8_t on);

static void LcdWrite(char dc, const char *data, int nbytes);

static void LcdWrite16(char dc, const uint16_t *data, int cnt);

static void ST7735_writeCmd(uint8_t c);
