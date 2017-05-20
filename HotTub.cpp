#include "HotTub.h"
#include "Arduino.h"

unsigned long HotTub::s_hall_count=0;

HotTub::HotTub(int temp_pin, int hall_pin, int valve_pin) {
	_valve_pin=valve_pin;
	_temp_pin=temp_pin;
	_hall_pin=hall_pin;
	pinMode(temp_pin,INPUT);
	pinMode(valve_pin,OUTPUT);
	pinMode(hall_pin,INPUT);
	_state=EMPTY;
	valveOff();
	_capacity = 700; //litres
	_desired_temp = 20; //degrees C
	_flow_rate = 0;
	_litres = 0;
	_wait_time = 20;//mins
	_test_time = 2;//mins
	s_hall_count=0;
	attachInterrupt(_hall_pin-2,HotTub::interruptRoutine,RISING);
	_time1=millis();
	_pin_invert=false;
/*	//tried implementing detector to work out if the thermistor is present - doesn't work.
	int test_temp=0;
	for(int a=0;a!=10;a++){
		test_temp+=getInstantTemp();
		delay(50);
	}
	if(((float)test_temp/10.0)<-30){
		setTestTime(0); //if no thermistor is connected, temp will show very very low
                        //set test time to zero which in turn ensures no test/wait sequence
	}*/

}

HotTub::~HotTub(){
	detachInterrupt(_valve_pin-2);
}

void HotTub::interruptRoutine(){
	s_hall_count++;
}

void HotTub::start() {//starts/restarts the fill
	if (_state!=FULL)
	{
		stateTransition(TESTING);
	}
}

void HotTub::stop() {//only pauses the fill
	stateTransition(STOPPED);
}

void HotTub::reset(){ //sets empty
	stateTransition(EMPTY);
	_litres=0;
	_flow_rate = 0;
	s_hall_count=0;
}

void HotTub::update(){ //updatesthe state machine
	
	if ((millis()-_time1)>10000){
		_litres+=(float)s_hall_count/_flow_const;
		_flow_rate = ((float)s_hall_count/_flow_const)*3600000/(millis()-_time1); //lph
		_time1=millis();
		s_hall_count=0;
	}
	if (_litres>_capacity) {
		stateTransition(FULL);
	}
	

	switch (_state){
		case TESTING:
			if ((long)(millis()-_time2) > (long)(_test_time*60000)) {
				if (getInstantTemp() > _desired_temp){
					stateTransition(FILLING);
				} else {
					stateTransition(WAITING);
				}
			}
			break;
		case WAITING:
			if ((millis()-_time2) > (_wait_time*1000*60)){
				stateTransition(TESTING);
			}
			break;
		case FILLING:
			if (_test_time!=0){
				if (getInstantTemp() < _desired_temp){
					stateTransition(WAITING);
				}
			}
			break;

	};			
}



void HotTub::stateTransition(HTState state){
	_state=state;
	_time2=millis();
	switch (state){
	case EMPTY:
		valveOff();
		break;
	case WAITING:
		valveOff();
		break;
	case TESTING:
		valveOn();
		if(_test_time==0) _state=FILLING;
		break;
	case FILLING:
		valveOn();
		break;
	case STOPPED:
		valveOff();
		break;
	case FULL:
		valveOff();
		break;
	};
}

int HotTub::getTimeOnState(){
	return (int)round((float)(millis()-_time2)/60000);
}

int HotTub::getInstantTemp(){
	//set up for the 10k resistor to be between 5v and Vo and thermister between Vo and gnd
	int Vc=3.28;//adjust accordingly - nb, this should be 5, no idea why it isn't!
	int R1=9970;
	float thermr=15000; //Rref at 25C
	float Vo=Vc*analogRead(_temp_pin)/1024.0;
	float resistance=Vo*R1 / (Vc-Vo);
	float A = 3.354016E-03 ;
	float B = 2.744032E-04;
	float C = 3.666944E-06 ;
	float D = 1.375492E-07;
	float var = log(resistance / thermr);
	float temp = 1 / (A + B*var + C*var*var + D*var*var*var);
	temp = temp - 273.15;  // Convert Kelvin to Celsius                      
	return (int)round(temp);
}

int HotTub::getTemp(){ //not written yet
	return 0;
}

int HotTub::getVolume(){
	return (int)_litres;
}

int HotTub::getCapacity(){
	return _capacity;
}


float HotTub::getFlowRate(){
	return _flow_rate;
}

void HotTub::setCapacity(int litres){
	if (litres<0) {
		_capacity=0;
	} else if (litres>1000)
		{ _capacity=1000;
	} else {
		_capacity=litres;
	}	
}

void HotTub::setVolume(int litres){
	if (litres<0) {
		_litres=0;
	} else if (litres>_capacity)
		{ _litres=_capacity;
	} else {
		_litres=litres;
	}	
}

void HotTub::setTempTarget(int desired_temp){
	_desired_temp=desired_temp;
}

int HotTub::getTempTarget(){
	return _desired_temp;
}

void HotTub::setWaitTime(int wait_time){ //mins - no more than 1 hour
	if (wait_time<0) {
		_wait_time=0;
	} else if (wait_time>60) {
		_wait_time=60;
	} else {
		_wait_time = wait_time;
	}
}

void HotTub::setTestTime(int test_time){ //mins no more than 5 mins - to ensure that it doesn't continually flow
	if (test_time<0) {
		_test_time=0;
	} else if (test_time>5) {
		_test_time=5;
	} else {
		_test_time = test_time;
	}
}

int HotTub::getFillTime(){
	switch(_state){
	case EMPTY:
	case STOPPED:
	case WAITING:
	case FULL:
		return 0;
		break;
	case TESTING:
	case FILLING:
		if (_flow_rate<1){
			return 0; //ensure we don't get a number outside the range of an int or indeed infinity
		} else {
			return (int)(_capacity-_litres)/(_flow_rate);
		}
		break;
	};
}

int HotTub::getState(){
	return _state;
}

int HotTub::getState(String *state_string){
	switch (_state){
	case EMPTY:
		*state_string = "Empty";
		break;
	case TESTING:
		*state_string="Testing";
		break;
	case FILLING:
		*state_string="Filling";
		break;
	case WAITING:
		*state_string="Waiting";
		break;
	case FULL:
		*state_string="Full";
		break;
	case STOPPED:
		*state_string="Paused";
		break;
	};
	return _state;
}

void HotTub::valvePinInvert(bool pin_invert){
	_pin_invert=pin_invert;
}

void HotTub::valveOn(){
	digitalWrite(_valve_pin,!_pin_invert);
}

void HotTub::valveOff(){
	digitalWrite(_valve_pin,_pin_invert);
}

void HotTub::processCommand(HTCommand command, long data){
	switch(command){
	case START:
		start();
		break;
	case STOP:
		stop();
		break;
	case RESET:
		reset();
		break;
	case VOL:
		setVolume(data); 
		break;
	case CAP:
		setCapacity(data);
		break;
	case TEMP_TARG:
		setTempTarget(data);
		break;
	case WAIT_TIME:
		setWaitTime(data);
		break;
	case TEST_TIME:
		setTestTime(data);
		break;
	};
}

void HotTub::getData(HTCommand command, long& data){
	switch(command){
	case VOL:
		data=getVolume();
		break;
	case CAP:
		data=getCapacity();
		break;
	case TEMP:
		data=getTemp();
		break;
	case INST_TEMP:
		data=getInstantTemp();
		break;
	case FLOW:
		data=getFlowRate();
		break;
	case TEMP_TARG:
		data=getTempTarget();
		break;
	case WAIT_TIME:
		data=_wait_time;
		break;
	case TEST_TIME:
		data=_test_time;
		break;
	case STATE:
		data=_state;
		break;
	case TIME_ON_STATE:
		data=getTimeOnState();
		break;
	case TIME_TO_FILL:
		data=getFillTime();
		break;
	};
}

