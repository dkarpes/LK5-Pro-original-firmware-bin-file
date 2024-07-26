#pragma once

#include "../inc/MarlinConfigPre.h"

#if ENABLED(LGT_LCD_DW)

#include "../MarlinCore.h"
// #include "lgtdwdef.h"
// #include "Marlin.h"
// #include "cardreader.h"
// #include "printcounter.h"
// #include "temperature.h"
// #include "duration_t.h"
// #include"LGT_MACRO.h"
// #include "power_loss_recovery.h"
// #include "planner.h"
// #include "parser.h"
// #include "stopwatch.h"
// #include "macros.h"
// #include "endstops.h"
// #include "configuration_store.h"

// extern uint8_t target_extruder;
// extern int ii_setup;
// #if ENABLED(SDSUPPORT)
// 	extern CardReader card;
// #endif
// extern void DWIN_MAIN_FUNCTIONS();
// extern bool enqueue_and_echo_command(const char* cmd);
// extern bool check_recovery;

typedef struct Data_Buffer
{
	unsigned char head[2];
	unsigned char cmd;
	unsigned long addr;
	unsigned long datalen;
	unsigned int data[30];
	char data_num[6];
}DATA;

enum PRINTER_STATUS
{
	PRINTER_SETUP,
	PRINTER_STANDBY,
	PRINTER_HEAT,
	PRINTER_PRINTING,
	PRINTER_PAUSE,
	PRINTER_PRINTING_F
};

enum PRINTER_KILL_STATUS
{
	PRINTER_NORMAL = 0,
	E_TEMP_KILL,
	B_TEMP_KILL,
	M112_KILL,
	SDCARD_KILL,    //trying to call sub - gcode files with too many levels
	HOME_KILL,       //homing failed
	TIMEOUT_KILL,     //KILL caused by too much inactive time 
	EXTRUDER_KILL,      //Invalid extruder number !
	DRIVER_KILL,        //driver kill
	E_MINTEMP_KILL,
	B_MINTEMP_KILL,
	E_MAXTEMP_KILL,
	B_MAXTEMP_KILL,
	E_RUNAWAY_KILL,
	B_RUNAWAY_KILL
};

enum ScreenModel {
	SCREEN_DWIN_T5,
	SCREEN_DWIN_T5L,
	SCREEN_JX
};

class LGT_SCR_DW
{
public:
	LGT_SCR_DW();

    void begin();
	void processButton();
	void hideButtonsBeforeHeating();
	void showButtonsAfterHeating();
	void LGT_Pause_Move();
	void goFinishPage();
	void saveFinishTime();
    void LGT_LCD_startup_settings();

	// void LED_Bright_State(uint8_t LED, uint16_t per, uint8_t mod);

	void LGT_MAC_Send_Filename(uint16_t Addr, uint16_t i);
	void LGT_Print_Cause_Of_Kill(const char* error, const char *component);
	void LGT_Get_MYSERIAL1_Cmd();
	void LGT_Analysis_DWIN_Screen_Cmd();
	void LGT_Send_Data_To_Screen(uint16_t Addr, int16_t Num);
	void LGT_Send_Data_To_Screen(unsigned int addr, float num,char axis);
	void LGT_Send_Data_To_Screen(unsigned int addr,char* buf);
	void LGT_Send_Data_To_Screen1(unsigned int addr,const char* buf);
	void LGT_Main_Function();

	void LGT_Display_Filename();
	void LGT_Clean_DW_Display_Data(unsigned int addr);
	void LGT_SDCard_Status_Update();
	void LGT_Change_Page(unsigned int pageid);
	void LGT_Power_Loss_Recovery_Resume();
	void LGT_Disable_Enable_Screen_Button(unsigned int pageid, unsigned int buttonid, unsigned int sta);
	void LGT_Screen_System_Reset();
	void LGT_Stop_Printing();
	void LGT_Exit_Print_Page();
	int LGT_Get_Extrude_Temp();
	void LGT_Save_Recovery_Filename(unsigned char cmd, unsigned char sys_cmd, /*unsigned int sys_addr,*/unsigned int addr, unsigned int length);
	// void LGT_Printer_Status_Light();
	// void LGT_Printer_Light_Update();
	void LGT_Printer_Data_Updata();
	void LGT_DW_Setup();
	void LGT_Change_Filament(int fila_len);
public:
	void readScreenModel();	
	static void test();
	inline bool hasDwScreen() { return ((_screenModel == SCREEN_DWIN_T5) || (_screenModel == SCREEN_DWIN_T5L)); }
	inline bool hasJxScreen() { return (_screenModel == SCREEN_JX); }
	void pausePrint();

private:
	void writeData(uint16_t addr, const uint8_t *data, uint8_t size, bool isRead=false);
	inline void writeData(uint16_t addr, uint8_t byteData, bool isRead=false)
	{
		writeData(addr, &byteData, 1, isRead);
	}
	inline void writeData(uint16_t addr, uint16_t wordData, bool isRead=false)
	{
		uint8_t *byte = (uint8_t *)(&wordData);	// reinterpret word to byte
		uint8_t data[2] = {byte[1], byte[0]};	// little-endian to big-endian
		writeData(addr, data, 2, isRead);

	}
	
	void readData(uint16_t addr, uint8_t *data, uint8_t size);

private:
	ScreenModel _screenModel;
	// button enable for JX screen
	bool _btnPauseEnabled;
	bool _btnFilamentEnabled1;
	bool _btnFilamentEnabled2;
};

#define CHANGE_TXT_COLOR(addr,color)	LGT_Send_Data_To_Screen((uint16_t)addr,(int16_t)color)
#define SP_COLOR_SEL_FILE_NAME			(SP_COLOR_TXT_PRINT_FILE_ITEM_0 + sel_fileid*LEN_FILE_NAME)
#define HILIGHT_FILE_NAME()				CHANGE_TXT_COLOR(SP_COLOR_SEL_FILE_NAME, COLOR_LIGHT_RED)
#define DEHILIGHT_FILE_NAME()			CHANGE_TXT_COLOR(SP_COLOR_SEL_FILE_NAME, COLOR_WHITE)

extern LGT_SCR_DW lgtLcdDw;     // extern interface
extern bool LGT_is_printing;
extern char leveling_sta; 
extern bool check_recovery; // for recovery dialog
extern bool is_abort_recovery_resume; // for abort recovery resume
extern bool is_recovery_resuming; // recovery resume
#endif // LGT_LCD_DW