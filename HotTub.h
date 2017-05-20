/* class to control hot tub object

ensure hall pin is on a pin with an interrupt - ie, pin 2 or 3

*/


#ifndef __HOTTUB_H_INCLUDED__
#define __HOTTUB_H_INCLUDED__

#include "Arduino.h"
#include <math.h>

typedef enum HTCommand{
	START, // 0
	STOP,  // 1 
	RESET, // 2
	WAIT_TIME, // 3 
	TEST_TIME, // 4
	VOL, // 5
	CAP, // 6
	TEMP, // 7 
	INST_TEMP, // 8
	FLOW, // 9
	TEMP_TARG, // 10 
	STATE, // 11
	TIME_ON_STATE, // 12
	TIME_TO_FILL // 13
} HTCommand;

typedef enum HTState{
		EMPTY,
		TESTING,
		FILLING,
		WAITING,
		FULL,
		STOPPED
	} HTState;

class HotTub{
public:
	HotTub(int temp_pin, int hall_pin, int valve_pin);
	~HotTub();

	void start(); //starts/restarts the fill - can be done whilst in any state
	void stop();  //only pauses the fill
	void reset(); //sets empty

	void update(); //updates

	int getVolume(); //litres filled
	void setVolume(int litres); //set if we are not starting at zero

	float getFlowRate(); //lph
	
	int getCapacity(); //litres max
	void setCapacity(int litres);

	void setTempTarget(int desired_temp);
	int getTempTarget(); 
	int getTemp(); //integrated and estimated temp of total volume of water so far
	int getInstantTemp(); // current temp sensor on inflow pipe
	void setWaitTime(int wait_time); //mins to allow boiler to refill hot tank
	void setTestTime(int test_time); //mins to allow water to flow before starting to measure the temp
	int getState(String *state_string);
	int getState();
	int getTimeOnState();

	void valvePinInvert(bool pin_invert); //false means valve on is pin high, true means valve on is pin low

	int getFillTime(); //predicted number of minutes left to fill

	static void interruptRoutine();

	void processCommand(HTCommand command, long data);
	void getData(HTCommand command, long& data);

private:

	static unsigned long s_hall_count;

	HTState _state;
	int _valve_pin;
	int _temp_pin;
	int _hall_pin;
	int _capacity; //litres
	int _desired_temp; //degrees C
	
	unsigned long _time1;
	unsigned long _time2;
	int _flow_rate; //lph
	float _litres;
	
	bool _pin_invert; //false means valve on is pin high, true means valve on is pin low

	void valveOn();
	void valveOff();
	int _wait_time; //mins
	int _test_time; //mins

	static const int _flow_const=475;
	//this value seems to be temperature dependant.  when cold, it is about 450
	//if it is warm this increases to 485.  475 seems to be a happy medium with +-4% error

	void stateTransition(HTState state);

};

#endif // __HOTTUB_H_INCLUDED__ 