 /******************************************************************************
 *
 * Module: Ultrasonic
 *
 * File Name: ultrasonic.c
 *
 * Description: Source file for the Ultrasonic driver
 *
 * Author: Alaa Elsayed
 *
 *******************************************************************************/

#include <util/delay.h> /* To use delay function */
#include "ultrasonic.h"
#include "gpio.h"
#include "icu.h"

/*******************************************************************************
 *                           Global Variables                                  *
 *******************************************************************************/

/* Global variable to store the values to be used in calculation in generating distance */
uint8 g_edgeCount = 0;
uint16 g_timeHigh = 0;
uint16 g_timePeriod = 0;
uint16 g_timePeriodPlusHigh = 0;

/* Global variable to store the distance in */
uint16 g_sensorDistance = 0;

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/

/*
 * Description :
 * Initialize the Ultrasonic:
 * 1. Initialize the ICU driver as required.
 * 2. Setup the ICU call back function.
 * 3. Setup the direction for the trigger pin as output pin through the GPIO driver.
 */
void Ultrasonic_init(void)
{
	/***************** ICU Initialization Steps *****************/

	Icu_ConfigType Icu_config = { F_CPU_8, RISING }; /* Create configuration structure for ICU Driver initialization */
	Icu_setCallBack( Ultrasonic_edgeProcessing ); /* Set the call back function pointer for ICU Driver */
	Icu_init( &Icu_config ); /* Initialize ICU for edge detection */


	/***************** Trigger Pin Setup *****************/

	GPIO_setupPinDirection(ULTRASONIC_SENSOR_TRIGGER_PORT_ID, ULTRASONIC_SENSOR_TRIGGER_PIN_ID, PIN_OUTPUT); /* Setup the trigger pin ( PB5 ) as output pin */

} /* End Ultrasonic_init Function */




/*
 * Description: Function to Send the Trigger pulse to the ultrasonic.
 */
void Ultrasonic_Trigger(void)
{
	/* Give an input pulse to the trigger pin to start ranging */
	GPIO_writePin(ULTRASONIC_SENSOR_TRIGGER_PORT_ID, ULTRASONIC_SENSOR_TRIGGER_PIN_ID, LOGIC_HIGH);

	_delay_us(10); /* Delay to get a pulse of ( 10 us ) or more */

	/* Disable the input pulse as we get the required duration to start ranging */
	GPIO_writePin(ULTRASONIC_SENSOR_TRIGGER_PORT_ID, ULTRASONIC_SENSOR_TRIGGER_PIN_ID, LOGIC_LOW);

} /* End Ultrasonic_Trigger Function */




/*
 * Description :
 * Get the distance from the sensor:
 * 1. Send the trigger pulse by using Ultrasonic_Trigger function.
 * 2. Start the measurements by the ICU from this moment.
 */
uint16 Ultrasonic_readDistance(void)
{
	Ultrasonic_Trigger(); /* Send the trigger pulse by using Ultrasonic_Trigger function */

	/* Make sure not to make calculation before measuring all needed time periods */
	if( g_edgeCount == 4 )
	{
		g_edgeCount = 0; /* Reset the counter of the edges */

		g_sensorDistance = ( g_timePeriodPlusHigh - g_timePeriod ) / DIVISION_FACTOR; /* Calculate the distance ( in cm ) from the pulse width time */
	} /* End if */

	return g_sensorDistance; /* Return the distance */
} /* End Ultrasonic_readDistance Function */





/*
 * Description :
 * 1. This is the call back function called by the ICU driver.
 * 2. This is used to calculate the high time (pulse time) generated by the ultrasonic sensor.
 */
void Ultrasonic_edgeProcessing(void)
{
	g_edgeCount++; /* increment the counter of the edges */

	/* When we detect first edge:
	 * 1. Reset the timer to start measurements from the first detected raising edge
	 * 2. Start the ICU again with falling edge configuration
	 */
	if( g_edgeCount == 1 )
	{
		Icu_clearTimerValue();
		Icu_setEdgeDetectionType(FALLING);
	}
	/* When we detect second edge:
	 * 1. Store the High time in global variable
	 * 2. Start the ICU again with raising edge configuration
	 */
	else if( g_edgeCount == 2 )
	{
		g_timeHigh = Icu_getInputCaptureValue();
		Icu_setEdgeDetectionType(RISING);
	}
	/* When we detect third edge:
	 * 1. Store the Period time in global variable
	 * 2. Start the ICU again with falling edge configuration
	 */
	else if( g_edgeCount == 3 )
	{
		g_timePeriod = Icu_getInputCaptureValue();
		Icu_setEdgeDetectionType(FALLING);
	}
	/* When we detect fourth edge:
	 * 1. Store the High + Period times in global variable
	 * 2. Reset the timer to start measurements from the first detected falling edge
	 * Start the ICU again with raising edge configuration
	 */
	else if( g_edgeCount == 4 )
	{
		g_timePeriodPlusHigh = Icu_getInputCaptureValue();
		Icu_clearTimerValue();
		Icu_setEdgeDetectionType(RISING);
	}

} /* End Ultrasonic_edgeProcessing Function */
