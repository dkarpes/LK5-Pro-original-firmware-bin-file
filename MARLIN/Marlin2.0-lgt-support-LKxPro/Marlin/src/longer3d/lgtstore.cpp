#include "lgtstore.h"

#if ENABLED(LGT_LCD_TFT)
#include "w25qxx.h"
#include "lgtsdcard.h"
#include "lgttftlanguage.h"
#include "lgttouch.h"
#include "lgtsdcard.h"

// #include "../../src/libs/crc16.h"
#include "../feature/runout.h"
#include "../feature/powerloss.h"

#define WRITE_VAR(value)        do { FLASH_WRITE_VAR(addr, value); addr += sizeof(value); } while(0)
#define READ_VAR(value)         do { FLASH_READ_VAR(addr, value); addr += sizeof(value); } while(0)

LgtStore lgtStore;

// this coanst text arry must sync with settings struct and settingPointer function
static const char *txt_menu_setts[SETTINGS_MAX_LEN] = {     
	TXT_MENU_SETTS_JERK_X,      // 0
    TXT_MENU_SETTS_JERK_Y,
	TXT_MENU_SETTS_JERK_Z,
	TXT_MENU_SETTS_JERK_E,
	TXT_MENU_SETTS_VMAX_X,
	TXT_MENU_SETTS_VMAX_Y,      // 5
	TXT_MENU_SETTS_VMAX_Z,
	TXT_MENU_SETTS_VMAX_E,
	TXT_MENU_SETTS_VMIN,
	TXT_MENU_SETTS_VTRAVEL,
	TXT_MENU_SETTS_AMAX_X,      // 10
	TXT_MENU_SETTS_AMAX_Y,	
	TXT_MENU_SETTS_AMAX_Z,
	TXT_MENU_SETTS_AMAX_E,
	TXT_MENU_SETTS_ARETRACT,
	TXT_MENU_SETTS_STEP_X,      // 15
	TXT_MENU_SETTS_STEP_Y,
	TXT_MENU_SETTS_STEP_Z,	
	TXT_MENU_SETTS_STEP_E,
    TXT_MENU_SETTS_ACCL,
    TXT_MENU_SETTS_LIST_ORDER,  // 20
	TXT_MENU_SETTS_CHECK_FILA,
    TXT_MENU_SETTS_RECOVERY
};

LgtStore::LgtStore()
{
    memset(reinterpret_cast<void *>(&m_settings), 0, sizeof(m_settings));
    clear();
}

/**
 * @brief save lgt custom settings into spiflash,
 *        and run M500 command or save other settings
 */
void LgtStore::save()
{

    // set version string
    strcpy(m_settings.version, SETTINGS_VERSION);

    // calc crc
    // uint16_t crc = 0;
    // crc16(&crc, reinterpret_cast<void *>(&m_settings.acceleration), sizeof(m_settings) - 4 - 2);
    // SERIAL_ECHOLNPAIR("save crc: ", crc);
    // m_settings.crc = crc;
    
    // save lgt custom settings in spiflash
    uint32_t addr = SPIFLASH_ADDR_SETTINGS;
    WRITE_VAR(m_settings.version);
    WRITE_VAR(m_settings.listOrder);
    SERIAL_ECHOPAIR("settings stored in spiflash(", addr - SPIFLASH_ADDR_SETTINGS);
    SERIAL_ECHOLN(" bytes)");

    //  save other settings in internal flash
    queue.enqueue_now_P("M500"); 
    setModified(false);
}

/**
 * validate if settings is stored in spiflash
 */
bool LgtStore::validate(const char *current, const char*stored)
{
    if (strcmp(current, stored) == 0)
        return true;
    return false;
}

/**
 * @brief load lgt custom settings from spiflash,
 *        spi flash -> settings struct -> memory(apply),
 *        load sequence must be consistent with save sequcence.
 */
bool LgtStore::load()
{
    uint32_t addr = SPIFLASH_ADDR_SETTINGS;

    SERIAL_ECHOLN("-- load settings form spiflash start --");
    READ_VAR(m_settings.version);
    SERIAL_ECHOLNPAIR("stored version: ", m_settings.version);
    SERIAL_ECHOLNPAIR("current version: ", SETTINGS_VERSION);
    if (!validate(SETTINGS_VERSION, m_settings.version)) {
       SERIAL_ECHOLN("load failed, reset settings");
       _reset();
       return false;    
    }

    READ_VAR(m_settings.listOrder);
    SERIAL_ECHOLNPAIR("listOrder: ", m_settings.listOrder);
    lgtCard.setListOrder(m_settings.listOrder);

    SERIAL_ECHOLN("-- load settings form spiflash end --");

    return true;
}

/**
 * reset lgt custom variables
 */
void LgtStore::_reset()
{
    lgtCard.setListOrder(true);

}

/**
 * reset all variables
 */
void LgtStore::reset()
{
    float tmp1[] = DEFAULT_AXIS_STEPS_PER_UNIT;
    float tmp2[] = DEFAULT_MAX_FEEDRATE;
    long tmp3[] = DEFAULT_MAX_ACCELERATION;

    LOOP_XYZE_N(i) {
        planner.settings.axis_steps_per_mm[i]          = tmp1[i];
        planner.settings.max_feedrate_mm_s[i]          = tmp2[i];
        planner.settings.max_acceleration_mm_per_s2[i] = tmp3[i];
    }
    planner.refresh_positioning();
    planner.settings.acceleration = DEFAULT_ACCELERATION;
    planner.settings.retract_acceleration = DEFAULT_RETRACT_ACCELERATION;;
    planner.settings.min_feedrate_mm_s =   DEFAULT_MINIMUMFEEDRATE;
    planner.settings.min_travel_feedrate_mm_s = DEFAULT_MINTRAVELFEEDRATE;
    planner.max_jerk[X_AXIS] = DEFAULT_XJERK;
    planner.max_jerk[Y_AXIS] = DEFAULT_YJERK;
    planner.max_jerk[Z_AXIS] = DEFAULT_ZJERK;
    #if DISABLED(JUNCTION_DEVIATION) || DISABLED(LIN_ADVANCE)
      planner.max_jerk[E_AXIS] =  DEFAULT_EJERK;
    #endif

    runout.enabled = true;
    recovery.enable(PLR_ENABLED_DEFAULT);

    _reset();
}

/**
 * apply settings struct to variables
 */
void LgtStore::applySettings()
{
    LOOP_XYZE_N(i) {
        planner.settings.axis_steps_per_mm[i]          = m_settings.axis_steps_per_unit[i];
        planner.settings.max_feedrate_mm_s[i]          = m_settings.max_feedrate[i];
        planner.settings.max_acceleration_mm_per_s2[i] = m_settings.max_acceleration_units_per_sq_second[i];
    }
    planner.refresh_positioning();
    planner.settings.acceleration = m_settings.acceleration;
    planner.settings.retract_acceleration = m_settings.retract_acceleration;;
    planner.settings.min_feedrate_mm_s =   m_settings.minimumfeedrate;
    planner.settings.min_travel_feedrate_mm_s = m_settings.mintravelfeedrate;
    planner.max_jerk[X_AXIS] = m_settings.max_x_jerk;
    planner.max_jerk[Y_AXIS] = m_settings.max_y_jerk;
    planner.max_jerk[Z_AXIS] =  m_settings.max_z_jerk;
    #if DISABLED(JUNCTION_DEVIATION) || DISABLED(LIN_ADVANCE)
      planner.max_jerk[E_AXIS] =  m_settings.max_e_jerk;
    #endif

    lgtCard.setListOrder(m_settings.listOrder);
    runout.enabled = m_settings.enabledRunout;
    recovery.enable(m_settings.enabledPowerloss);
}

/**
 * sync variables to settings struct
 */
void LgtStore::syncSettings()
{
    LOOP_XYZE_N(i) {
       m_settings.axis_steps_per_unit[i] = planner.settings.axis_steps_per_mm[i];
        m_settings.max_feedrate[i] = planner.settings.max_feedrate_mm_s[i];
       m_settings.max_acceleration_units_per_sq_second[i] = planner.settings.max_acceleration_mm_per_s2[i];
    }
    // planner.refresh_positioning();
    m_settings.acceleration = planner.settings.acceleration;
    m_settings.retract_acceleration = planner.settings.retract_acceleration;
    m_settings.minimumfeedrate = planner.settings.min_feedrate_mm_s;
    m_settings.mintravelfeedrate = planner.settings.min_travel_feedrate_mm_s;
    m_settings.max_x_jerk = planner.max_jerk[X_AXIS];
    m_settings.max_y_jerk = planner.max_jerk[Y_AXIS];
    m_settings.max_z_jerk = planner.max_jerk[Z_AXIS];
    #if DISABLED(JUNCTION_DEVIATION) || DISABLED(LIN_ADVANCE)
      m_settings.max_e_jerk = planner.max_jerk[E_AXIS];
    #endif
    m_settings.listOrder = lgtCard.isReverseList();
    m_settings.enabledRunout = runout.enabled;
    m_settings.enabledPowerloss = recovery.enabled;
}

/**
 * @brief  get settting string for lcd
 */
void LgtStore::settingString(uint8_t i, char* str)
{
    char p[10] = {0};
	if (i >= SETTINGS_MAX_LEN) { /* error index */
		return;
	} else if (i > 20) {  	    /* bool type: off/on */				
        #ifndef Chinese
            const char * format = "%8s";
        #else
            const char * format = "%5s";
        #endif
        if(*reinterpret_cast<bool *>(settingPointer(i)))
            sprintf(p, format, TXT_MENU_SETTS_VALUE_ON);
        else
            sprintf(p, format, TXT_MENU_SETTS_VALUE_OFF);
	} else if (i == 20) {       // bool type: inverse/forward
        #ifndef Chinese
            const char * format = "%8s";
        #else
            const char * format = "%5s";
        #endif
        if(*reinterpret_cast<bool *>(settingPointer(i)))
            sprintf(p, format, TXT_MENU_SETTS_VALUE_INVERSE);
        else
            sprintf(p, format, TXT_MENU_SETTS_VALUE_FORWARD);
    } else if (i >= 10 && i<= 13) { /* uint32 type */		
		#ifndef Chinese 				
			sprintf(p,"%8lu", *reinterpret_cast<uint32_t *>(settingPointer(i)));			
		#else
			sprintf(p,"%6lu",  *reinterpret_cast<uint32_t *>(settingPointer(i)));
        #endif
	} else { /* float type */
        sprintf(p,"%8.2f", *reinterpret_cast<float *>(settingPointer(i)));
	}

    sprintf(str, "%-20s%s", txt_menu_setts[i], p);

}

void *LgtStore::settingPointer(uint8_t i)
{
	switch(i)
	{
        // float type
		case 0:
			return &m_settings.max_x_jerk;	
		case 1:
			return &m_settings.max_y_jerk;		
		case 2:
			return &m_settings.max_z_jerk;			
		case 3:
			return &m_settings.max_e_jerk;		
		case 4:
			return &m_settings.max_feedrate[X_AXIS];			
		case 5: 
			return &m_settings.max_feedrate[Y_AXIS];
		case 6:
			return &m_settings.max_feedrate[Z_AXIS];			
		case 7:
			return &m_settings.max_feedrate[E_AXIS];
		case 8: 
			return &m_settings.minimumfeedrate;
		case 9:
			return &m_settings.mintravelfeedrate;
        // uint32 type
		case 10:
			return &m_settings.max_acceleration_units_per_sq_second[X_AXIS];
		case 11: 
			return &m_settings.max_acceleration_units_per_sq_second[Y_AXIS];
		case 12:
			return &m_settings.max_acceleration_units_per_sq_second[Z_AXIS];
		case 13:
			return &m_settings.max_acceleration_units_per_sq_second[E_AXIS];
        // float type
		case 14: 
			return &m_settings.retract_acceleration;
		case 15:
			return &m_settings.axis_steps_per_unit[X_AXIS];
		case 16:
			return &m_settings.axis_steps_per_unit[Y_AXIS];
		case 17: 
			return &m_settings.axis_steps_per_unit[Z_AXIS];
		case 18:
			return &m_settings.axis_steps_per_unit[E_AXIS];
        case 19:
            return &m_settings.acceleration;
        // bool type
        case 20:
            return &m_settings.listOrder;        
		case 21:
			return &m_settings.enabledRunout;
        case 22:
            return &m_settings.enabledPowerloss;
		default: return 0;
	}
}

float LgtStore::distanceMultiplier(uint8_t i)
{
	switch(i){
		case 2: case 15: case 16: case 17: case 18:	
			return 0.1;
		case 0: case 1:	case 3:case 4: case 5: case 6: case 7:
		case 8: case 9: case 12:
			return 1.0;
		case 10: case 11: case 13: case 14: case 19:
			return 100.0;
        default:
            return 0.0; 
	}
}

/**
 * @brief  change setting value
 * @param  i setting index
 * @param  distance change value
 * @retval None
 */
void LgtStore::changeSetting(uint8_t i, int8_t distance)
{
	if(i >= SETTINGS_MAX_LEN){			/* error index */
		return;
	}
	else if(i >= 20)  	/* bool type */
	{				
		*(bool *)settingPointer(i) = !(*(bool *)settingPointer(i));
	}
	else if(i >= 10 && i<= 13)  /* unsigned long type */
	{		
		*(unsigned long *)settingPointer(i) = *(unsigned long *)settingPointer(i) +
			distance * (unsigned long)distanceMultiplier(i);
		if((long)*(unsigned long *)settingPointer(i) < 0)
            *(unsigned long *)settingPointer(i) = 0;	//minimum value
	} 
    else /* float type */
	{								
		*(float *)settingPointer(i) = *(float *)settingPointer(i) + distance * distanceMultiplier(i);
		if(*(float *)settingPointer(i) < 0.0) 
            *(float *)settingPointer(i) = 0.0;	//minimum value
	}
    setModified(true);
}

 bool LgtStore::selectSetting(uint16_t item)
 {
    if (item < LIST_ITEM_MAX) {
        uint16_t n = m_currentPage * LIST_ITEM_MAX + item;
        if (n < SETTINGS_MAX_LEN) {
            m_currentItem = item;
            m_currentSetting = n;
            m_isSelectSetting = true;
            return false;   // success to set
        }
    }
    return true;   // fail to set
 }

/**
 * @brief save touch calibration data into spiflash
 */
void LgtStore::saveTouch()
{
    TouchCalibration &touch = lgtTouch.calibrationData();
    // set version string
    strcpy(touch.version, TOUCH_VERSION);
    // save calibration data in spiflash
    uint32_t addr = SPIFLASH_ADDR_TOUCH;
    WRITE_VAR(touch);
    SERIAL_ECHOPAIR("touch data stored in spiflash(", addr - SPIFLASH_ADDR_TOUCH);
    SERIAL_ECHOLN(" bytes)");
}

/**
 * @brief load touch calibration data from spiflash
 */
bool LgtStore::loadTouch()
{
    SERIAL_ECHOLN("-- load touch data form spiflash start --");
    uint32_t addr = SPIFLASH_ADDR_TOUCH;
    TouchCalibration &touch = lgtTouch.calibrationData();
    READ_VAR(touch.version);
    SERIAL_ECHOLNPAIR("stored version: ", touch.version);
    SERIAL_ECHOLNPAIR("current version: ", TOUCH_VERSION);
    if (!validate(TOUCH_VERSION, touch.version)) {
        SERIAL_ECHOLN("load failed. calibrate touch screen");
        // lgtTouch.resetCalibration();
        lgtTouch.calibrate(false);
        return false;    
    }
    READ_VAR(touch.xCalibration);
    SERIAL_ECHOLNPAIR("xCali: ", touch.xCalibration);

    READ_VAR(touch.yCalibration);
    SERIAL_ECHOLNPAIR("yCali: ", touch.yCalibration);

    READ_VAR(touch.xOffset);
    SERIAL_ECHOLNPAIR("xOffset: ", touch.xOffset);

    READ_VAR(touch.yOffset);
    SERIAL_ECHOLNPAIR("yOffset: ", touch.yOffset);

    SERIAL_ECHOLN("-- load touch data form spiflash end --");
    return true;
}

/**
 * @brief save data of powerloss into spiflash
 */
void LgtStore::saveRecovery()
{

    // save recovery data in spiflash
    uint32_t addr = SPIFLASH_ADDR_RECOVERY;
    // SERIAL_ECHOLNPAIR("save filename: ", card.longFilename);
    WRITE_VAR(card.longFilename);
    WRITE_VAR(lgtCard.printTime());
    SERIAL_ECHOPAIR("recovery data stored in spiflash(", addr - SPIFLASH_ADDR_RECOVERY);
    SERIAL_ECHOLN(" bytes)");
}

/**
 * @brief load data of powerloss into spiflash
 */
bool LgtStore::loadRecovery()
{
    SERIAL_ECHOLN("-- load recovery data form spiflash start --");
    uint32_t addr = SPIFLASH_ADDR_RECOVERY;

    READ_VAR(card.longFilename);
    SERIAL_ECHOLNPAIR("longfilename: ", card.longFilename);

    READ_VAR(lgtCard.printTime());
    SERIAL_ECHOLNPAIR("printTime: ", lgtCard.printTime());  

    SERIAL_ECHOLN("-- load recovery data from spiflash end --");
    return true;
}

/**
 * @brief clear verison value of settings stored in spiflash
 */
void LgtStore::clearSettings()
{
    char tmp[4] = {0};
    FLASH_WRITE_VAR(SPIFLASH_ADDR_SETTINGS, tmp);
    SERIAL_ECHOLN("settings in spiflash has been cleared");
}

/**
 * @brief clear version value of touch calibration stored in spiflash
 */
void LgtStore::clearTouch()
{
    char tmp[4] = {0};
    FLASH_WRITE_VAR(SPIFLASH_ADDR_TOUCH, tmp);
    SERIAL_ECHOLN("touch data in spiflash has been cleared");

}

#endif