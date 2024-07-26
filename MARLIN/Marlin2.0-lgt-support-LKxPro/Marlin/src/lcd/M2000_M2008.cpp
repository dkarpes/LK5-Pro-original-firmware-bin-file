#include "../inc/MarlinConfig.h"
#include "../gcode/gcode.h"

#if ENABLED(LGT_LCD_DW)
  #include "lgtdwlcd.h"
  #include "lgtdwdef.h"
  #include "../module/motion.h"
  #include "../feature/runout.h"
  extern E_MENU_TYPE menu_type;
  extern PRINTER_STATUS status_type;
  extern char menu_move_dis_chk;
  // #include "../../module/stepper.h"
  // #include "../../module/endstops.h"

  // #define DEBUG_M2000

  #define DEBUG_OUT ENABLED(DEBUG_M2000)
  #include "../core/debug_out.h"

  // abort printing and return to home menu
  void GcodeSuite::M2000()
  {
      DEBUG_ECHOLNPGM("run M2000");
      M18_M84();
		  if (leveling_sta!=2)
		  {
			  lgtLcdDw.LGT_Change_Page(ID_MENU_HOME);
		  }
		  else
		  {
			  leveling_sta=0;
		  }
		  runout.reset();
  }

  // wait for printing pausing
  void GcodeSuite::M2001()
  {
      DEBUG_ECHOLNPGM("run M2001");
      lgtLcdDw.LGT_Pause_Move();
		  lgtLcdDw.LGT_Change_Page(ID_MENU_PRINT_HOME_PAUSE);
  }

  #if ENABLED(U20_PLUS)
  // wait for auto-levelling measuring 
    void GcodeSuite::M2002()
    {
		  planner.synchronize();
		  lgtLcdDw.LGT_Change_Page(ID_MENU_MEASU_S1 + 1);
    }
  #endif

  // save position and filament runout  move
  void GcodeSuite::M2003()
  {
      DEBUG_ECHOLNPGM("run M2003");
      lgtLcdDw.LGT_Change_Page(ID_DIALOG_NO_FILA);
      status_type = PRINTER_PAUSE;
      // if(all_axes_known()) {
        
        lgtLcdDw.LGT_Pause_Move();
      // }
  }

  // load filament
  void GcodeSuite::M2004()
  {
    DEBUG_ECHOLNPGM("run M2004");
    lgtLcdDw.LGT_Change_Filament(LOAD_FILA_LEN);

  }

  // unload filament
  void GcodeSuite::M2005()
  {
    DEBUG_ECHOLNPGM("run M2005");
    lgtLcdDw.LGT_Change_Filament(UNLOAD_FILA_LEN);

  }

  // filament change in printing
  void GcodeSuite::M2006()
  {

      DEBUG_ECHOLNPGM("run M2006");
		  lgtLcdDw.LGT_Pause_Move();
		  lgtLcdDw.LGT_Change_Page(ID_MENU_HOME_FILA_0);
		  menu_type = eMENU_HOME_FILA;

  }

  void GcodeSuite::M2007()
  {
      // if (parser.seen('Z'))
		  // {
			//   current_position[Z_AXIS] = parser.value_linear_units();
			//   LGT_Line_To_Current(Z_AXIS);
			//   planner.set_z_position_mm((destination[Z_AXIS] = current_position[Z_AXIS] = 0));
		  // }
		  // else if (parser.seen('E'))
		  // {
			//   current_position[E_AXIS] = parser.value_linear_units();
			//   LGT_Line_To_Current(E_AXIS);
			//   planner.set_e_position_mm((destination[E_AXIS] = current_position[E_AXIS] = 0));
		  // }
  }

// wait for homing in MOVE menu
  void GcodeSuite::M2008()
  {
		if(menu_move_dis_chk==0)
			lgtLcdDw.LGT_Change_Page(ID_MENU_MOVE_0);
		else if(menu_move_dis_chk==1)
			lgtLcdDw.LGT_Change_Page(ID_MENU_MOVE_1);
		else
			lgtLcdDw.LGT_Change_Page(ID_MENU_MOVE_1+1);
  } 

#endif // LGT_LCD_DW

