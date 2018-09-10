#include "keyboard.h"
#include "scancodes.h"
#include "Arch/x86.h"
#include "Arch/x86/custom_defs.h"
#include "Shell/Shell.h"
#include "stdio.h"

void keyboard_init()
{
  //kkybrd_set_leds(1,1,1);
  scancodes=Main_key_codes;
  Create_ScodeTables();
}


void Assign_Scode(int scode, int (*func)(int))
{
    MainScodes->scodes[scode].func = func;
}

void Assign_Scode_cs(int scode, int (*func)(int), KeyCode_t* scodes)
{
    scodes[scode].func = func;
}

void Assign_Scode_ccs(int scode, int (*func)(int), KeyCode_t* scodes, uint32_t type, int code)
{
    scodes[scode].func = func;
    scodes[scode].type = type;
    scodes[scode].code = code;
}

void Assign_Scode_tm(int scode, func_t func, uintptr_t* map)
{
    map[scode] = (uintptr_t)func;
}

uintptr_t Get_ctrlHandle(int key)
{
    return Ctrl_Handles[key];
}

void CtrlGeneral_MAKE_KeyHandle()
{
}

void CtrlGeneral_BREAK_KeyHandle()
{
}

void Scode_ctrlNormalKeys(int scancode)
{
    func_t handle = (func_t)Get_ctrlHandle(scancode);
    if(handle)
    {
        handle();
    }
    else 
    {
    }
}

void KeyboardHandler(int scancode)
{
    int key;
    if(_shift)
    {
      key = MainScodes->scodes[scancode + 0x60 + 0x80].code;  // Shifted keys
      
      if (key >= 'a' && key <= 'z')
        key -= 32;
    }
    else
    {
      key = MainScodes->scodes[scancode].code;
    }
    if (_capslock)
    {
      if(_shift)
      {
        if (key >= 'A' && key <= 'Z')
          key += 32;
      }
      else
      {
        if (key >= 'a' && key <= 'z')
          key -= 32;
      }
    }
      
    MainScodes->scodes[scancode].func(key);
}


void KeyboardCTRLhandler(int scancode)
{
    int key;
    key = MainScodes->scodes[scancode].code;
    CtrlScodes->scodes[scancode].func(key);
}

void Create_CtrlScodes()
{
    for(int i = 0; i < 0x59; i++)       // Make Default Controls for all keys so no problem/faults occur
    {
        if(scancodes[i])
        {
            CtrlScodes->scodes[i].code = scancodes[i];
            CtrlScodes->scodes[i].type = SCODE_ORDINARY;
            CtrlScodes->scodes[i + 0x80].code = scancodes[i];
            CtrlScodes->scodes[i + 0x80].type = SCODE_ORDINARY;
        }
        CtrlScodes->scodes[i].func = (intfunc1_t)&CtrlGeneral_MAKE_KeyHandle;
        CtrlScodes->scodes[i+0x80].func = (intfunc1_t)&CtrlGeneral_BREAK_KeyHandle;
    }
    Assign_Scode_ccs(0x1d, &Scode_MAKE_LCtrl, CtrlScodes->scodes, SCODE_SPECIAL_COPY, scancodes[0x1d]);    // For CTRL Itself
    Assign_Scode_ccs(0x1d + 0x80, &Scode_BREAK_LCtrl, CtrlScodes->scodes, SCODE_SPECIAL_COPY, scancodes[0x1d + 0x80]);    // For CTRL Itself
    Assign_Scode_ccs(0x2e, &Scode_CTRL_MAKE_C, CtrlScodes->scodes, SCODE_SPECIAL_COPY, scancodes[0x2e]);    // For CTRL + C MAKE
    Assign_Scode_ccs(0x2e + 0x80, &Scode_CTRL_BREAK_C, CtrlScodes->scodes, SCODE_SPECIAL_COPY, scancodes[0x2e + 0x80]);    // For CTRL + C BREAK
}

void Create_ScodeTables()
{
    int maxCodes = 0x60 + 0x80;
    MainScodes = (ScodeTable_t*)kmalloc(sizeof(ScodeTable_t) + (2*sizeof(KeyCode_t)*maxCodes));
    MainScodes->entries = maxCodes;
    MainScodes->type = 1;
    strcpy(MainScodes->layout, "US");
    Ctrl_Handles = (uintptr_t*)phy_alloc4K();

    CtrlScodes = (ScodeTable_t*)kmalloc(sizeof(ScodeTable_t) + (2*sizeof(KeyCode_t)*maxCodes));
    CtrlScodes->entries = maxCodes;
    CtrlScodes->type = 2;
    strcpy(CtrlScodes->layout, "US");

    for(int i = 0; i < 0x59; i++)       // Ordinary Keys
    {
        if(scancodes[i])
        {
            MainScodes->scodes[i].code = scancodes[i];
            MainScodes->scodes[i].type = SCODE_ORDINARY;
            MainScodes->scodes[i + 0x80].code = scancodes[i];
            MainScodes->scodes[i + 0x80].type = SCODE_ORDINARY;
        }
        MainScodes->scodes[i].func = &Scode_OrdinaryMakeHandle;
        MainScodes->scodes[i+0x80].func = &Scode_OrdinaryBreakHandle;
        //Ctrl_Handles[i] = (uintptr_t)&CtrlGeneral_MAKE_KeyHandle;       // Just to fill in the table
        //Ctrl_Handles[i+0x80] = (uintptr_t)&CtrlGeneral_BREAK_KeyHandle;
    }

    for(int i = 0; i < 0x59; i++)       // Shift keys
    {
        if(scancodes[i])
        {
            MainScodes->scodes[i + maxCodes].code = Shift_key_codes[i];
            MainScodes->scodes[i + maxCodes].type = SCODE_SHIFTON;
            MainScodes->scodes[i + maxCodes + 0x80].code = Shift_key_codes[i];
            MainScodes->scodes[i + maxCodes + 0x80].type = SCODE_SHIFTON;
        }
        MainScodes->scodes[i + maxCodes].func = &Scode_OrdinaryMakeHandle;
        MainScodes->scodes[i + maxCodes + 0x80].func = &Scode_OrdinaryBreakHandle;
    }

    Assign_Scode(0xe, &Scode_BackSpaceHandle);
    Assign_Scode(0x1c, &Scode_ReturnHandle);
    Assign_Scode(0x1d, &Scode_MAKE_LCtrl);
    Assign_Scode_tm(0x1d, (func_t)&Scode_MAKE_LCtrl, Ctrl_Handles);
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
    Assign_Scode_tm(0x1d + 0x80, (func_t)&Scode_BREAK_LCtrl, Ctrl_Handles);
   // Assign_Scode( + 0x80, &Scode_BREAK_RCtrl);
    Assign_Scode(0x2a + 0x80, &Scode_BREAK_LShift);
    Assign_Scode(0x36 + 0x80, &Scode_BREAK_RShift);
   // Assign_Scode( + 0x80, &Scode_BREAK_LAlt);
    Assign_Scode(0x38 + 0x80, &Scode_BREAK_RAlt);
   // Assign_Scode(0x49 + 0x80, &Scode_BREAK_Pup);
   // Assign_Scode(0x51 + 0x80, &Scode_BREAK_Pdown);
    Assign_Scode(0x48 + 0x80, &Scode_BREAK_Aup);
    Assign_Scode(0x50 + 0x80, &Scode_BREAK_Adown);

    Create_CtrlScodes(CtrlScodes);

    NormalKey_Func = &KeyboardHandler;
    std_in[0] = 8;
    std_in[1] = 0;
}

int __attribute__((flatten)) Scode_NormalKeys(int key)
{
    char call = key;//keyboard_scancodes(key);//KeyPlexer(key);
    *(Istream_ptr) = call;
    ++Istream_ptr;
    ++kb_buff;
    if(Istream_ptr == Istream_end)
     Istream_ptr = (char*)Input_stream;   //Reset
    putchar((int)call);
    return key;
}

int Scode_OrdinaryMakeHandle(int key)
{
    return Scode_NormalKeys(key);
}

int Scode_OrdinaryBreakHandle(int key)
{
    return key;    // Do Nothing
}

int Scode_SpecialHandle(int key)
{
    printf("{SCODE NOT ASSIGNED FOR %d %c}", key, key);
    return key;
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
    char* tmp = (char*)((uint32_t)Istream_ptr - (uint32_t)kb_buff);
    (*Istream_ptr) = '\0';
    ++Istream_ptr;
    strcpy(&((char*)std_in)[std_in[0]], tmp);
    //printf("\n[%s], [%s]", &((char*)std_in)[std_in[0]], tmp);
    std_in[1] = kb_buff;
    kb_buff = 0;
    
    return key;
}

int Scode_MAKE_LCtrl(int key)
{
    _ctrl = 1;
    NormalKey_Func = &KeyboardCTRLhandler;//Scode_ctrlNormalKeys;
    return key;
}

int Scode_MAKE_RCtrl(int key)
{
    _ctrl = 2;
    NormalKey_Func = &KeyboardCTRLhandler;//Scode_ctrlNormalKeys;
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
    //kkybrd_set_leds (_numlock, _capslock, _scrolllock);
    return key;
}

int Scode_MAKE_NumOn(int key)
{
    _numlock = (_numlock) ? false : true;
    //kkybrd_set_leds (_numlock, _capslock, _scrolllock);
    return key;
}

int Scode_MAKE_ScrollOn(int key)
{
    _scrolllock = (_scrolllock) ? false : true;
    //kkybrd_set_leds (_numlock, _capslock, _scrolllock);
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
    _arrow_up = 1;
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
    NormalKey_Func = &KeyboardHandler;
    return key;
}

int Scode_BREAK_RCtrl(int key)
{
    _ctrl = 0;
    NormalKey_Func = &KeyboardHandler;
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

/***************************** CTRL POLY KEY CODES ********************************/

int Scode_CTRL_MAKE_C(int key)
{
    printf("\tCTRL+C presses");
    // Kill the Currently running task which is using the shell ostream
	Task_wakeup((task_t*)Shell_task);
	shell_awake = 1;
    return key;
}

int Scode_CTRL_BREAK_C(int key)
{
    _ctrl_C_pressed = 1;

    return key;
}