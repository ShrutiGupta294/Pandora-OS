#include "library.h"
#include "vga.h"

uint32_t consolerow=0,consolecolumn=0;
uint8_t console_color=0;
uint16_t *vgamem=(uint16_t *)0xb8000, *console_buffer = (uint16_t *)0xb8000;

void console_init()
{
    console_color=make_color(COLOR_WHITE,COLOR_BLACK);
    default_console_color = console_color;
    for(int i=0;i<VGA_HEIGHT;i++)
    {
        for(int j=0;j<VGA_WIDTH;j++)
        {
            vgamem[i*VGA_WIDTH+j]=make_vgaentry(' ', console_color);
        }
    }
}

inline void console_setcolor(uint8_t color)
{
	console_color = color;
}

void console_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	console_buffer[index] = make_vgaentry(c, color);
}

void console_putchar(char c)
{
    char a=c;
    if(c=='\n') c=' ';
    else if(c=='\t') c=' ';
	console_putentryat(c, console_color, consolecolumn, consolerow);
	if ( ++consolecolumn == VGA_WIDTH||a=='\n')
	{
		consolecolumn = 0;
		if ( ++consolerow == VGA_HEIGHT )
		{
		    --consolerow;
		    uint16_t buff[((VGA_HEIGHT+1)*VGA_WIDTH)+VGA_WIDTH+1];
		    for(size_t x=0;x<(((VGA_HEIGHT+1)*VGA_WIDTH)+VGA_WIDTH+1);x++)
            {
                buff[x] = 0;
            }
		    for ( size_t y = 0; y < VGA_HEIGHT; y++ )
            {
                for ( size_t x = 0; x < VGA_WIDTH; x++ )
                {
                    const size_t index = y * VGA_WIDTH + x;
                    buff[index] = console_buffer[index];
                }
            }
            for ( size_t y = 0; y < VGA_HEIGHT; y++ )
            {
                for ( size_t x = 0; x < VGA_WIDTH; x++ )
                {
                    const size_t index = y * VGA_WIDTH + x;
                    const size_t backindex = (y+1) * VGA_WIDTH + x;
                    console_buffer[index] = buff[backindex];
                }
            }
		}
	}
	else if(a=='\t')
    {
        consolecolumn+=5;
    }
}

void console_write(const char* data, size_t size)
{
	for ( size_t i = 0; i < size; i++ )
		console_putchar(data[i]);
}

int putchar(int ic)
{
	char c = (char) ic;
	console_write(&c, sizeof(c));
	return ic;
}

void printint(uint32_t in)
{
  char tmp[16];
	itoa(in, tmp, 10);
	print(tmp, strlen(tmp));
}

void backspace()
{
		--consolecolumn;
		if(!consolecolumn)
		{
			--consolerow;
			consolecolumn = VGA_WIDTH;
		}
		console_putentryat(' ', console_color, consolecolumn, consolerow);
}