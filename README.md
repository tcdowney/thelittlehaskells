the little haskells
=========

For my [C335 Computer Structures](http://cgi.cs.indiana.edu/~geobrown/c335) course at IU, we developed a suite of "drivers" in C for the STM32VL-Discovery microcontroller in accordance with the course's [lab manual](http://www.cs.indiana.edu/~geobrown/book.pdf).

Our final project was to create a simple game for the board using an [Adafruit 1.8" LCD](https://www.adafruit.com/products/358) as the display, Wii nunchucks for player control, and the chip's onboard DAC capabilities to output audio.

The game's name, "The Little Haskells," was meant to be a tongue-in-cheek reference to The Little Rascals.  The Little Haskells are a crack team of "rockstar programmers" who defend America through the power of buzzwords.  The objective of the game is to pilot a ship through enemy networks and survive long enough to "reverse hack" into their systems.

[Here is a demo video of the game running.](http://www.youtube.com/watch?v=iOoo_a4vqDI)

Components
-----------
The following components are required to play the game:

* [STM32VL-DISCOVERY](http://www.st.com/internet/evalboard/product/250863.jsp) - an ARM Cortex-M3 based microcontroller.
* [Adafruit 1.8" LCD with Micro SD card reader](https://www.adafruit.com/products/358) - a basic lcd screen with a Micro SD card slot.  The Adafruit model was selected because it came with some basic driver code that was translated to C. 
* [Wii Nunchuck Adapter](https://www.adafruit.com/products/345) - although our PCB was designed to have two nunchuck adapters built in, they are essentially the same as the "Nunchucky" adapters on Adafruit.
* Speaker and Potentiometer - we used a cheap speaker to provide audio and a potentiometer to modulate the volume.  Since "The Little Haskells" only plays simple tones, I'm looking into using a piezoelectric buzzer instead.

Additionally, we used the [FatFS file system](http://elm-chan.org/fsw/ff/00index_e.html) to load and read files from the SD card.

Installation
--------------
Coming soon...
