#include "A36744.h"
#include "FIRMWARE_VERSION.h"

// This is the firmware for Interface Board


_FOSC(ECIO & CSW_FSCM_OFF); 
_FWDT(WDT_ON & WDTPSA_512 & WDTPSB_8);  // 8 Second watchdog timer 
_FBORPOR(PWRT_OFF & BORV45 & PBOR_ON & MCLR_EN);
_FBS(WR_PROTECT_BOOT_OFF & NO_BOOT_CODE & NO_BOOT_EEPROM & NO_BOOT_RAM);
_FSS(WR_PROT_SEC_OFF & NO_SEC_CODE & NO_SEC_EEPROM & NO_SEC_RAM);
_FGS(GWRP_OFF & GSS_OFF);
_FICD(PGD);


LTC265X U23_LTC2654;

#define EK_VOLTAGE_TABLE_VALUES 50,76,102,128,153,179,204,229,253,277,300,322,344,366,386,406,425,443,460,476,491,505,517,529,540,549,557,564,570,574,577,579,580,579,577,574,570,564,557,549,540,529,517,505,491,476,460,443,425,406,386,366,344,322,300,277,253,229,204,179,153,128,102,76,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50

#define TOP_VOLTAGE_TABLE_VALUES 50,65,79,94,109,123,137,151,165,178,191,204,217,229,240,251,262,272,282,291,299,307,315,321,327,332,337,341,344,347,349,350,350,350,349,347,344,341,337,332,327,321,315,307,299,291,282,272,262,251,240,229,217,204,191,178,165,151,137,123,109,94,79,65,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50


const unsigned int EkReferenceVoltageTable[256] = {EK_VOLTAGE_TABLE_VALUES};  // This table defines the cathode reference voltage per the resistor value
const unsigned int TopReferenceVoltageTable[128]  = {TOP_VOLTAGE_TABLE_VALUES};   // This table defines the top reference voltage per the resistor value


ControlData global_data_A36744;

void DoStateMachine(void);
void InitializeA36744(void);
int CheckHeaterFlt (void);

#define STATE_STARTUP       0x10
#define STATE_WARMUP	    0x20
#define STATE_READY		    0x30
#define STATE_SHUTDOWN      0x40

int main(void) {
  global_data_A36744.control_state = STATE_STARTUP;
  while (1) {
    DoStateMachine();
  }
}


void DoStateMachine(void) {
  switch (global_data_A36744.control_state) {

  case STATE_STARTUP:
    InitializeA36744();
    global_data_A36744.heater_warmup_timer = HTR_WARMUP_DEFAULT_DURATION;
	if SHORT_RESET_NOT == 0 //checking power interrupt duration
	{
		global_data_A36744.heater_warmup_timer = HTR_WARMUP_RECOVERY_DURATION;
	}
	else
	{
		Convert ADC Ek resistor
		Convert ADC Top resistor
		while (conversion incomplete)
		Get Top and Ek reference voltages.
		output ek and top voltages.
		output htr voltage.
		while (conversion incomplete)
	}
    global_data_A36744.control_state = STATE_WARMUP;
    break;

	
  case STATE_WARMUP:
    int flash_LED_timer = 10;
	PIN_STANDBY = 1;
	PIN_HTR_ENABLE_NOT = 0;
	while (global_data_A36744.heater_warmup_timer > 0)
	{
		output heater set voltage.
		if (_T3IF == 1)
		{
			_T3IF = 0;
			global_data_A36744.heater_warmup_timer--;
			flash_LED_timer--;
		}
		if (flash_LED_timer==0)
		{
			flash_LED_timer = 50;
			PIN_HTR_LED =^ 1;
		}
		if (SHORT_HEAT == 1 && global_data_A36744.heater_set_voltage != HEATER_FAST_WARMUP_VOLTAGE) 
		{
			global_data_A36744.heater_set_voltage = HEATER_FAST_WARMUP_VOLTAGE;
			
			if global_data_A36744.heater_warmup_timer > HTR_WARMUP_SHORT_DURATION
				global_data_A36744.heater_warmup_timer = HTR_WARMUP_SHORT_DURATION;
		}		
		if global_data_A36744.heater_warmup_timer <= HV_ON_DELAY
			PIN_PIC_HV_ON = 1 ;
		if (_INT1IF == 1) {
			global_data_A36744.control_state = STATE_SHUTDOWN;
		}
		if (CheckHeaterFlt)
			PIN_HTR_LED = 1;
		else
			PIN_HTR_LED = 0;
	
	}
    global_data_A36744.heater_set_voltage = HEATER_DEFAULT_VOLTAGE;
	global_data_A36744.heater_warmup_timer = HTR_WARMUP_DEFAULT_DURATION;
	PIN_STANDBY = 0;
	PIN_ETM_RESET_DETECT = 1;
	global_data_A36744.control_state = STATE_READY;
    break;

  case STATE_READY:
  /* 
  During the ready state, the controller checks for the following conditions:
	1. Heater Backoff - if there is no pulse input (pretrans) for a given amount of time (heater backoff window), 
		then the heater voltage will be backed off to heater backoff voltage.	
		Implementation notes - backoff counter is always counting back with the 10ms timer. pretrans is used as a latched input (external interrupt), 
		which asserts its flag on every rising edge. As long as a rising edge is detected, the backoff counter is reset. 
		If there is no rising edge for long enough, the counter will reach zero and lower the heater voltage.
	2. Arc fault - if a number of repeated arcs (arcs repeated) is detected within a given 
		amount of time (arc fault window min-max), then an arc fault is generated.
	3. Check for Volterrn condition- once asserted, the power supply needs to be shut down. When cleared, 
		the power supply needs to be brought up again and start the heater warmup sequence.
  */
  
	_T3IF = 0;
	_INT4IF = 0;
	_INT0IE = 1;
    while (global_data_A36744.control_state == STATE_READY) {
		
		if _INT4IF == 1 // keep heater at its default values as long as pulses are coming in.
		{
			_INT4IF = 0;
			global_data_A36744.heater_backoff_time_counter = HTR_BACKOFF_WINDOW;
			global_data_A36744.heater_set_voltage = HEATER_DEFAULT_VOLTAGE;
		}		
			
			
      if (_T3IF == 1)
		{
			_T3IF = 0;
			global_data_A36744.heater_backoff_time_counter--; 
			if global_data_A36744.heater_backoff_time_counter == 0 // change heater voltage to backoff value if no pulses came in for x amount of time.
			{
				global_data_A36744.heater_set_voltage = HEATER_BACKOFF_VOLTAGE;
			}
			if (global_data_A36744.arc_counter != 0) // what happens if arc detected in this line?
			{
				global_data_A36744.arc_timer++;
			}					
			else{
				global_data_A36744.arc_timer = 0; // does this make sense??
			}
		}
	   output htr voltage.
	  
	  if (global_data_A36744.arc_timer >= ARC_FLT_WINDOW_MIN)
	  {
		  _INT0IE = 0;
		  if (global_data_A36744.arc_counter >= ARCS_REPEATED)
		  {
			  PIN_PIC_ARC_FLT_NOT = 0;
			  _delay_ms(10);
			  PIN_PIC_ARC_FLT_NOT = 1;
			  _INT0IF = 0;
		  }
	
		 if ( global_data_A36744.arc_timer >= ARC_FLT_WINDOW_MAX)
		 {
			 global_data_A36744.arc_counter = 0;
		 }
		  
		  _INT0IE = 1; 
	  } 
	  
	  if (_INT1IF == 1) {
		global_data_A36744.control_state = STATE_SHUTDOWN;
	}
		if (CheckHeaterFlt)
			PIN_HTR_LED = 1;
		else
			PIN_HTR_LED = 0; 
    }
    break;
     
    
 case STATE_SHUTDOWN:
    _INT1EP = 0; //change interrupt to detect rising edge (removal of the volterrn condition)
	_INT1IF = 0;
	PIN_PIC_HV_ON = 0;
	PIN_HTR_ENABLE_NOT = 1;
	_delay_ms (500);
	PIN_GRID_ENABLE = 0;
	
	while (global_data_A36744.control_state == STATE_SHUTDOWN) {
     	 if (_INT1IF == 1)
		{
		 _INT1IF = 0;
		 PIN_GRID_ENABLE = 1;
		 _delay_ms (100);
		 global_data_A36744.heater_set_voltage = HEATER_DEFAULT_VOLTAGE;
		 global_data_A36744.control_state = STATE_WARMUP;
		 _INT1EP = 1 ;
		}
		if (CheckHeaterFlt)
			PIN_HTR_LED = 1;
		else
			PIN_HTR_LED = 0;
	 }
	break;

	
  default:
    global_data_A36744.control_state = STATE_READY;
    break;

  }
}


void InitializeA36744(void) {

  TRISA = A36744_TRISA_VALUE;
  TRISB = A36744_TRISB_VALUE;
  TRISC = A36744_TRISC_VALUE;
  TRISD = A36744_TRISD_VALUE;
  TRISF = A36744_TRISF_VALUE;
  TRISG = A36744_TRISG_VALUE;

  PIN_PIC_ARC_FLT_NOT = 1;
  PIN_HTR_ENABLE_NOT = 1;

  PIN_STANDBY   = 0;
  PIN_POR   = 0;
  PIN_PIC_HV_ON   = 0;
  PIN_GRID_ENABLE   = 1;
  //PIN_ETM_RESET_DETECT
  PIN_OVERLOAD = 0;
  PIN_PIC_ERROR = 0;
  PIN_HTR_LED = 0;
  PIN_PIC_VOLTERRN_NOT = 1;
  //PIN_TEST_POINT_A =0;
  PIN_LED_OPERATIONAL_GREEN = 1;
  //PIN_LED_A_RED = 0;


  //Timer1 setup
 // PR1 = PR1_SETTING;
 // _T1IF = 0; 		// Clear Timer1 interrupt flag
 // _T1IP = 5; 	// Set Timer1 interrupt priority
 // _T1IE = 1; 		//Enable Timer1 Interrupt
 // T1CON = T1CON_SETTING;

  //Timer3 setup
  PR3 = PR3_VALUE_10_MILLISECONDS;
  T3CON = T3CON_VALUE;
  _T3IF = 0;
  
//ADC setup
  ADCON2 = ADCON2_SETTING;
  ADCON3 = ADCON3_SETTING;
  ADCHS  = ADCHS_SETTING;
  ADPCFG = ADPCFG_SETTING;
  ADCSSL = ADCSSL_SETTING;
  ADCON1 = ADCON1_SETTING;
  
// additional interrupt set-up
  _INT4IF = 0;
  _INT4IP = 1;
  _INT4IE = 0;

  _INT0IF = 0;
  _INT0IP = 4;

  _INT1IF = 0;
  _INT1IP = 5;
  _INT1IE = 0;
  _INT1EP = 1;

  // Initialize LTC DAC
  //SetupLTC265X(&U23_LTC2654, ETM_SPI_PORT_1, FCY_CLK, LTC265X_SPI_2_5_M_BIT, _PIN_RG15, _PIN_RC1);
  SetupLTC265X(&U23_LTC2654, ETM_SPI_PORT_1, FCY_CLK, LTC265X_SPI_2_5_M_BIT, _PIN_RC1, _PIN_RC3);

//#define AFT_CONTROL_VOLTAGE_MAX_PROGRAM  12000
//#define AFT_CONTROL_VOLTAGE_MIN_PROGRAM  1000

  ETMAnalogInitializeOutput(&global_data_A36744.heater_set_voltage,
			    MACRO_DEC_TO_SCALE_FACTOR_16(3.15),
			    OFFSET_ZERO,
			    ANALOG_OUTPUT_0,
			    AFT_CONTROL_VOLTAGE_MAX_PROGRAM,
			    AFT_CONTROL_VOLTAGE_MIN_PROGRAM,
			    0);

  ETMAnalogInitializeOutput(&global_data_A36744.aft_control_voltage,
			    MACRO_DEC_TO_SCALE_FACTOR_16(3.98799),
			    OFFSET_ZERO,
			    ANALOG_OUTPUT_0,
			    AFT_CONTROL_VOLTAGE_MAX_PROGRAM,
			    AFT_CONTROL_VOLTAGE_MIN_PROGRAM,
			    0);

  ETMAnalogInitializeOutput(&global_data_A36744.aft_control_voltage,
			    MACRO_DEC_TO_SCALE_FACTOR_16(3.98799),
			    OFFSET_ZERO,
			    ANALOG_OUTPUT_0,
			    AFT_CONTROL_VOLTAGE_MAX_PROGRAM,
			    AFT_CONTROL_VOLTAGE_MIN_PROGRAM,
			    0);
  
}

int CheckHeaterFlt (void) {
	return (HTR_SUM_FLT || HTR_OC_FLT || HTR_UC_FLT);
}

void __attribute__((interrupt, no_auto_psv)) _INT0Interrupt(void) {
  /*
    External interrupt 0 indicates an arc was detected. each interrupt will increase the arc counter by 1. 
  */
	global_data_A36744.arc_counter++;
    _T1IF = 0;
}
