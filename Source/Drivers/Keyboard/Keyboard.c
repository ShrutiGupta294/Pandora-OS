#include "sys.h"
#include "keyboard.h"
#include "scancodes.h"
#include "common.h"
#include "library.h"


int keyboard_scancodes(int key)
{
  if (_capslock || _shift)
    if (key >= 'a' && key <= 'z')
      key -= 32;

  if (_capslock && _shift)
    if (key >= 'A' && key <= 'Z')
      key += 32;

		if (_shift)
			{
				if (key >= '0' && key <= '9')
				switch (key)
        {

					case '0':
						key = KEY_RIGHTPARENTHESIS;
						break;
					case '1':
						key = KEY_EXCLAMATION;
						break;
					case '2':
						key = KEY_AT;
						break;
					case '3':
						key = KEY_HASH;
						break;
					case '4':
						key = KEY_DOLLAR;
						break;
					case '5':
						key = KEY_PERCENT;
						break;
					case '6':
						key = KEY_CARRET;
						break;
					case '7':
						key = KEY_AMPERSAND;
						break;
					case '8':
						key = KEY_ASTERISK;
						break;
					case '9':
						key = KEY_LEFTPARENTHESIS;
						break;
				}
			else
      {

				switch (key) {
					case KEY_COMMA:
						key = KEY_LESS;
						break;

					case KEY_DOT:
						key = KEY_GREATER;
						break;

					case KEY_SLASH:
						key = KEY_QUESTION;
						break;

					case KEY_SEMICOLON:
						key = KEY_COLON;
						break;

					case KEY_QUOTE:
						key = KEY_QUOTEDOUBLE;
						break;

					case KEY_LEFTBRACKET :
						key = KEY_LEFTCURL;
						break;

					case KEY_RIGHTBRACKET :
						key = KEY_RIGHTCURL;
						break;

					case KEY_GRAVE:
						key = KEY_TILDE;
						break;

					case KEY_MINUS:
						key = KEY_UNDERSCORE;
						break;

					case KEY_EQUAL:
						key = KEY_PLUS;
						break;

					case KEY_BACKSLASH:
						key = KEY_BAR;
						break;
          case KEY_RETURN:
            key = '\n';
            break;
				}
			}
		}
    return key;
}

void Assign_Scode(int scode, int (*func)(int))
{
    MainScodes->scodes[scode].func = func;
}

void Create_ScodeTables()
{
    scancodes = Main_key_codes;
    int maxCodes = 0x60 + 0x80;
    MainScodes = (ScodeTable_t*)phy_alloc4K();//kmalloc(sizeof(ScodeTable_t) + (sizeof(KeyCode_t)*maxCodes));
    MainScodes->entries = maxCodes;
    MainScodes->type = 1;
    strcpy(MainScodes->layout, "US");

    for(int i = 0; i < 0x59; i++)       // Make Code
    {
        if(scancodes[i])
        {
            MainScodes->scodes[i].code = scancodes[i];
            MainScodes->scodes[i].type = SCODE_ORDINARY;
        }
        else  
        {
            MainScodes->scodes[i].code = Special_key_codes[i];
            MainScodes->scodes[i].type = SCODE_SPECIAL;
        }
        MainScodes->scodes[i].func = &Scode_OrdinaryMakeHandle;
    }

    for(int i = 0; i < 0x59; i++)       // Break Code
    {
        if(scancodes[i])
        {
            MainScodes->scodes[i+0x80].code = scancodes[i];
            MainScodes->scodes[i+0x80].type = SCODE_ORDINARY;
        }
        else  
        {
            MainScodes->scodes[i+0x80].code = Special_key_codes[i];
            MainScodes->scodes[i+0x80].type = SCODE_SPECIAL;
        }
        MainScodes->scodes[i+0x80].func = &Scode_OrdinaryBreakHandle;
    }
    
    Assign_Scode(0xe, &Scode_BackSpaceHandle);
    Assign_Scode(0x1c, &Scode_ReturnHandle);
    Assign_Scode(0x1d, &Scode_MAKE_LCtrl);
   // Assign_Scode(, &Scode_MAKE_RCtrl);
    Assign_Scode(0x2a, &Scode_MAKE_LShift);
    Assign_Scode(0x36, &Scode_MAKE_RShift);
   // Assign_Scode(, &Scode_MAKE_LAlt);
    Assign_Scode(0x38, &Scode_MAKE_RAlt);
    Assign_Scode(0x3a, &Scode_MAKE_CapsOn);
    Assign_Scode(0x45, &Scode_MAKE_NumOn);
    Assign_Scode(0x46, &Scode_MAKE_ScrollOn);
    Assign_Scode(0x49, &Scode_MAKE_Pup);
    Assign_Scode(0x51, &Scode_MAKE_Pdown);
    Assign_Scode(0x48, &Scode_MAKE_Aup);
    Assign_Scode(0x50, &Scode_MAKE_Adown);

    Assign_Scode(0x1d + 0x80, &Scode_BREAK_LCtrl);
   // Assign_Scode( + 0x80, &Scode_BREAK_RCtrl);
    Assign_Scode(0x2a + 0x80, &Scode_BREAK_LShift);
    Assign_Scode(0x36 + 0x80, &Scode_BREAK_RShift);
   // Assign_Scode( + 0x80, &Scode_BREAK_LAlt);
    Assign_Scode(0x38 + 0x80, &Scode_BREAK_RAlt);
   // Assign_Scode(0x49 + 0x80, &Scode_BREAK_Pup);
   // Assign_Scode(0x51 + 0x80, &Scode_BREAK_Pdown);
    Assign_Scode(0x48 + 0x80, &Scode_BREAK_Aup);
    Assign_Scode(0x50 + 0x80, &Scode_BREAK_Adown);
}

void Keyboard_init()
{

}


int Scode_OrdinaryMakeHandle(int key)
{
    char call = keyboard_scancodes(key);//KeyPlexer(key);
    *(Istream_ptr) = call;
    ++Istream_ptr;
    ++kb_buff;
    putchar(call);
   /* if((uint32_t)Istream_ptr == Istream_end)
     Istream_ptr = (char*)Input_stream;   //Reset*/
}

int Scode_OrdinaryBreakHandle(int key)
{
    return key;    // Do Nothing
}

int Scode_SpecialHandle(int key)
{
    printf("{SCODE NOT ASSIGNED FOR %d %c}", key, key);
}

int Scode_BackSpaceHandle(int key)
{
    if(kb_buff)
    {
      --Istream_ptr;
      *Istream_ptr = 0;
      --kb_buff;
      backspace();
    }
    return key;
}

int Scode_ReturnHandle(int key)
{
    enter_pressed = 1;
   /* printf("\nEnter");
    Shell_SFexecute("proc -l Shell");*/
    return key;
}

int Scode_MAKE_LCtrl(int key)
{
    _ctrl = 1;
    return key;
}

int Scode_MAKE_RCtrl(int key)
{
    _ctrl = 2;
    return key;
}

int Scode_MAKE_LShift(int key)
{
    _shift = 1;
    return key;
}

int Scode_MAKE_RShift(int key)
{
    _shift = 2;
    return key;
}

int Scode_MAKE_LAlt(int key)
{
    _alt = 1;
    return key;
}

int Scode_MAKE_RAlt(int key)
{
    _alt = 2;
    return key;
}

int Scode_MAKE_CapsOn(int key)
{
    _capslock = (_capslock) ? false : true;
    kkybrd_set_leds (_numlock, _capslock, _scrolllock);
    return key;
}

int Scode_MAKE_NumOn(int key)
{
    _numlock = (_numlock) ? false : true;
    kkybrd_set_leds (_numlock, _capslock, _scrolllock);
    return key;
}

int Scode_MAKE_ScrollOn(int key)
{
    _scrolllock = (_scrolllock) ? false : true;
    kkybrd_set_leds (_numlock, _capslock, _scrolllock);
    return key;
}

int Scode_MAKE_Pup(int key)
{
    return key;
}

int Scode_MAKE_Pdown(int key)
{
    return key;
}

int Scode_MAKE_Aup(int key)
{
 //   _arror_up = 1;
    return key;
}

int Scode_MAKE_Adown(int key)
{
    _arrow_down = 1;
    return key;
}

/**************** BREAK ****************/

int Scode_BREAK_LCtrl(int key)
{
    _ctrl = 0;
    return key;
}

int Scode_BREAK_RCtrl(int key)
{
    _ctrl = 0;
    return key;
}

int Scode_BREAK_LShift(int key)
{
    _shift = 0;
    return key;
}

int Scode_BREAK_RShift(int key)
{
    _shift = 0;
    return key;
}

int Scode_BREAK_LAlt(int key)
{
    _alt = 0;
    return key;
}

int Scode_BREAK_RAlt(int key)
{
    _alt = 0;
    return key;
}

int Scode_BREAK_Aup(int key)
{
 //   _arrow_up = 0;
    return key;
}

int Scode_BREAK_Adown(int key)
{
    _arrow_down = 0;
    return key;
}