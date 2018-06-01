/************************************************************************************
 *
 * 	Name    : Sleep_n0m1.h
 * 	Author  : Noah Shibley / NoMi Design
 * 	Date    : July 10th 2011
 * 	Version : 0.1
 * 	Notes   : Some of this code comes from "Cloudy" on the arduino forum
 *			  http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1292898715
 *
 * 		    Sleep_n0m1 is free software: you can redistribute it and/or modify
 * 		    it under the terms of the GNU General Public License as published by
 * 		    the Free Software Foundation, either version 3 of the License, or
 * 		    (at your option) any later version.
 *
 * 		    Sleep_n0m1 is distributed in the hope that it will be useful,
 * 		    but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 		    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 		    GNU General Public License for more details.
 *
 * 		    You should have received a copy of the GNU General Public License
 * 		    along with Sleep_n0m1.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***********************************************************************************/

#include "Sleep_n0m.h"

Sleep* Sleep::pSleep = 0;

Sleep::Sleep()
{
	pSleep = this;	//the ptr points to this object
	timeSleep = 0;  // total time due to sleep
	calibv = 1.0; // ratio of real clock with WDT clock
	byte isrcalled = 0;  // WDT vector flag
	sleepCycleCount = 0;
	sleepCycleInterval = 100;

}

/********************************************************************
*
*	setSleepMode
*
********************************************************************/
void Sleep::setSleepMode(int mode)
{
  sleepMode_ = mode;
}


/********************************************************************
*
*	sleepPinInterrupt
*
********************************************************************/
void Sleep::sleepPinInterrupt(int interruptPin,int mode) {

	int intNum = digitalPinToInterrupt(interruptPin);

	if(mode == FALLING || mode == LOW)
	{
	   pinMode (interruptPin, INPUT);
	   digitalWrite (interruptPin, HIGH);
	}

	set_sleep_mode(sleepMode_);
	sleep_enable();
	attachInterrupt(intNum,sleepHandler,mode);
	sei(); //make sure interrupts are on!
	sleep_mode();
	 //----------------------------- ZZZZZZ sleeping here----------------------
	sleep_disable(); //disable sleep, awake now
	detachInterrupt(intNum);
}


/********************************************************************
*
*	sleepHandler ISR
*
********************************************************************/
void sleepHandler(void)
{


}