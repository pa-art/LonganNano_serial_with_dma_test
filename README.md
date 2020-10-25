# LonganNano USART receive test with DMA transfer
This source file is to receive a character via USART with DMA transfer of Longan Nano and echo back the character to the termainal. 

## Requirement
- systick.c
- lcd/lcd.c

These files are required to compile main.c, obtained from [https://github.com/sipeed/Longan_GD32VF_examples].

## Usage
1. Connect Longan Nano to your PC via USB-serial converter.
2. Compile and load the hex file to Longan Nano via USB.
3. Run serial termainal on your PC and connect to the port of USB-serial converter.
4. Type some texts on the terminal, then these characters appears on Longan Nano's OLED and the terminal when 16 characters are typed.

## Memo
- To receive via USART, the USART interrupt should be enabled, I think. (really ??)
- DMA usage is correct ??
- Some bug(?) are in the source file perhaps. The last character of previous type on the terminal appears as the first character of next DMA transfer.