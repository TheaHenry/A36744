// Header file for new Interface Board
#ifndef __A36744_H
#define __A36744_H


#define FCY_CLK 10000000


#include <xc.h>
#include <libpic30.h>
#include <adc10.h>
#include <timer.h>
#include <pwm.h>

#include "ETM.h"

/*
  Hardware Module Resource Usage

  SPI1   - Used/Configured by LTC265X Module
  Timer3 - 10ms timer 
  Timer1 - Used for repeated arc detection

*/


// ----------------- IO PIN CONFIGURATION -------------------- //
// All unused pins will be set to outputs and logic zero
// LAT values default to 0 at startup so they do not need to be manually set

// Pins to be configured as inputs
/*

  RA6 - Digital Input - Ek O/V Flt
  RA7 - Digital Input - Collector U/V Flt
  RA9 - VREF-
  RA10 - VREF+
  RA12 - Digital Input -  VOLTERRN
  RA13 - INT2 - ARC DETECT
 
  RB0  - ICD - PROGRAM
  RB1  - ICD - PROGRAM
  RB2 - Digital Input - Ek U/V Flt
  RB3  - Analog Input - Ek Resistor Sense
  RB4  - Analog Input - Top Resistor Sense
  RB5 - Digital Input - Over PW Flt
  RB7 - Analog Input - ETM ADC +15V monitor

  RC1  - DAC LDAC  (Configured by DAC module)
  RC4  - HV ON SENSE
  RC14 - Digital Input - Heater Sum Flt
  
  RD1 - Digital Input - Short reset (short power cycle)
  RD4 - Digital Input - Temp SW Flt
  RD5 - Digital Input - Heater O/C Flt
  RD6 - Digital Input - Low Line Flt
  RD7 - Digital Input - Iw O/C Flt
  RD8 - Digital Input - Pretrans
  RD9 - Digital Input - Short heat
  RD10 - Digital Input - Reset
  RD13 - Digital Input - Inverter O/C Flt
  RD15 - Digital Input - Grid U/V Flt
  
  RF2 - UART (not used)
  RF3 - UART (not used)
  RF4 - Digital Input - Grid O/C Flt
  RF5 - Digital Input - Heater U/C Flt
  RF6 - SPI1 (Configured by Pic Module)
  RF7 - SPI1 (Configured by Pic Module)
  RF8 - SPI1 (Configured by Pic Module)
  
  RG6 - Digital Input - TWT over temp Flt
  RG7 - Digital Input - PS over temp Flt
  RG8 - Digital Input - Grid O/V Flt
  RG9 - Digital Input - Over Duty Flt
  RG15  - DAC CS/LD (Configured by DAC module)

*/


#define A36744_TRISA_VALUE 0b0011011011000000 
#define A36744_TRISB_VALUE 0b0000000010111111 
#define A36744_TRISC_VALUE 0b0100000000010010 
#define A36744_TRISD_VALUE 0b1010011111110010
#define A36744_TRISF_VALUE 0b0000000111111100 
#define A36744_TRISG_VALUE 0b1000001111000000


// ------------- PIN DEFINITIONS ------------------- ///

//Control
#define PIN_PIC_ARC_FLT_NOT         	_LATB10
#define PIN_HTR_ENABLE_NOT           	_LATD11
#define PIN_STANDBY			            _LATD0
#define PIN_POR				            _LATD2
#define PIN_PIC_HV_ON				    _LATG0
#define PIN_GRID_ENABLE				    _LATG3
#define PIN_ETM_RESET_DETECT		    _LATG14

//Status
#define PIN_OVERLOAD          			_LATC2
#define PIN_PIC_ERROR        		    _LATB14
#define PIN_HTR_LED				    	_LATG12
#define PIN_PIC_VOLTERRN_NOT			_LATB13

// Test Points
#define PIN_TEST_POINT_A                 _LATB11

// LEDs
#define PIN_LED_OPERATIONAL_GREEN        _LATA14
#define PIN_LED_A_RED                    _LATG8

//Discrete Inputs
#define SHORT_RESET_NOT					_RD1
#define SHORT_HEAT						_RD9
#define HTR_SUM_FLT						_RC14
#define HTR_OC_FLT						_RD5
#define HTR_UC_FLT						_RF5
#define PRETRANS						_RD8


// ---------------- Timing Configuration Values ------------- //
#define ARC_FLT_WINDOW_MIN	        3000        // repeated arcs within defined time will assert an arc fault
#define ARC_FLT_WINDOW_MAX	        5000        // repeated arcs within defined time will assert an arc fault
#define HTR_BACKOFF_WINDOW      5400          // Duration with no pulse to start heater backoff
#define HTR_WARMUP_DEFAULT_DURATION     18000           // Default duration for heater timer delay
#define HV_ON_DELAY             500      // Time allotted for HV turn on during warmup
#define HTR_WARMUP_SHORT_DURATION     6000           // Fast warmup duration in secs for heater timer delay
#define HTR_WARMUP_RECOVERY_DURATION	1000

#define HEATER_DEFAULT_VOLTAGE		6300 		//mV
#define HEATER_FAST_WARMUP_VOLTAGE	7000		//mV
#define HEATER_BACKOFF_VOLTAGE		6150		//mV
#define ARCS_REPEATED				3 

#define HEATER_SET_VOLTAGE_MAX_PROGRAM  7000 	//mV
#define HEATER_SET_VOLTAGE_MIN_PROGRAM  6000 	//mV
#define CATHODE_SET_VOLTAGE_MAX_PROGRAM  10300	//V
#define CATHODE_SET_VOLTAGE_MIN_PROGRAM  11500	//V
#define TOP_SET_VOLTAGE_MAX_PROGRAM  150		//V
#define TOP_SET_VOLTAGE_MIN_PROGRAM  0			//V

// --------------------- T1 Configuration -----
// With 1:8 prescale 

#define T1CON_SETTING     (T1_ON & T1_IDLE_CON & T1_GATE_OFF & T1_PS_1_256 & T1_SYNC_EXT_OFF & T1_SOURCE_INT)
#define PR1_SETTING  (unsigned int)(FCY_CLK / 32 / 8 / ARC_FLT_WINDOW)


/* 
   TMR3 Configuration
   Timer3 - Used for 10msTicToc
   Period should be set to 10mS
   With 10Mhz Clock, x8 multiplier will yield max period of 17.7mS, 2.71uS per tick
*/

#define T3CON_VALUE                    (T3_ON & T3_IDLE_CON & T3_GATE_OFF & T3_PS_1_8 & T3_SOURCE_INT)
#define PR3_PERIOD_US                  10000   // 10mS
#define PR3_VALUE_10_MILLISECONDS      (unsigned int)((FCY_CLK / 1000000)*PR3_PERIOD_US/8)




// -------------------  ADC CONFIGURATION ----------------- //
//Still needs to be defined for application
#define ADCON1_SETTING   (ADC_MODULE_ON & ADC_IDLE_STOP & ADC_FORMAT_INTG & ADC_CLK_MANUAL & ADC_SAMPLE_SIMULTANEOUS & ADC_AUTO_SAMPLING_ON)
#define ADCON2_SETTING   (ADC_VREF_EXT_EXT & ADC_SCAN_OFF & ADC_CONVERT_CH_0ABC & ADC_SAMPLES_PER_INT_1 & ADC_ALT_BUF_ON & ADC_ALT_INPUT_OFF)
#define ADCON3_SETTING   (ADC_SAMPLE_TIME_10 & ADC_CONV_CLK_SYSTEM & ADC_CONV_CLK_2Tcy)
#define ADCHS_SETTING    (ADC_CHX_POS_SAMPLEA_AN3AN4AN5 & ADC_CHX_NEG_SAMPLEA_VREFN & ADC_CH0_POS_SAMPLEA_AN8 & ADC_CH0_NEG_SAMPLEA_VREFN & ADC_CHX_POS_SAMPLEB_AN3AN4AN5 & ADC_CHX_NEG_SAMPLEB_VREFN & ADC_CH0_POS_SAMPLEB_AN8 & ADC_CH0_NEG_SAMPLEB_VREFN)
#define ADPCFG_SETTING   (ENABLE_AN2_ANA & ENABLE_AN3_ANA & ENABLE_AN4_ANA & ENABLE_AN5_ANA & ENABLE_AN8_ANA)
#define ADCSSL_SETTING   (SKIP_SCAN_AN0 & SKIP_SCAN_AN1 & SKIP_SCAN_AN2 & SKIP_SCAN_AN6 & SKIP_SCAN_AN7 & SKIP_SCAN_AN9 & SKIP_SCAN_AN10 & SKIP_SCAN_AN11 & SKIP_SCAN_AN12 & SKIP_SCAN_AN13 & SKIP_SCAN_AN14 & SKIP_SCAN_AN15)

typedef struct {
  unsigned int control_state;
  AnalogOutput heater_set_voltage;
  unsigned int cathode_resistor;
  unsigned int top_resistor;
  AnalogOutput cathode_set_voltage;
  AnalogOutput top_set_voltage;
  unsigned int arc_counter; //consider adding volatile
  unsigned int arc_timer;
  unsigned int heater_warmup_timer;
  unsigned int heater_backoff_time_counter;
  unsigned int volterrn_time_counter;

} ControlData;

extern ControlData global_data_A36744;

#endif
