#ifndef CB2_H_
#define CB2_H_

#include "WPILib.h"
#include "LogitechGamepad.h"
#include "TractionControl.h"
#include "Math.h"

/*
 * Drive Train
 */
RobotDrive *drive;

/*
 * Pneumatics
 */
Compressor *pumpAir;
DoubleSolenoid *hookRelease;

/*
 * Motors and Controllers
 */
Victor *frontLeft;
Victor *backLeft;
Victor *frontRight;
Victor *backRight;
Victor *spinMotor;
Victor *winchMotor;
Victor *pickupMotor;
Victor *flapperMotor;

#define OUT			DoubleSolenoid::kForward
#define IN			DoubleSolenoid::kReverse
#define OFF			DoubleSolenoid::kOff

/*
 * Driver Station
 */
DriverStation *ds;						// driver station object
DriverStationLCD *dsLCD;
UINT32 priorPacketNumber;					// keep track of the most recent packet number from the DS
UINT8 dsPacketsReceivedInCurrentSecond;	// keep track of the ds packets received in the current second

/*
 * Controls
 */
Joystick *rightStick;
Joystick *leftStick;
LogitechGamepad *pilotPad;
float leftAxisY;
float rightAxisY;
float rightPadY;
float leftPadY;

bool flapUp;
bool flapDown;

bool preset1;
bool preset2;
bool preset3;
bool preset4;

bool shootBall;
bool shootRetract;
bool shoot;

bool winchDown;
bool winchUp;

bool driveDone;
bool autonFlap;

float shootDelay;
float autonMode;

/*
 * Encoders
 */
AugmentedEncoder *leftEncoder;
AugmentedEncoder *rightEncoder;
AugmentedEncoder *winchEncoder;
AugmentedEncoder *pickupEncoder;
AnalogChannel *flapEncoder;

double d_p_t_drive = (4 * 4 * atan(1)) / 360;
double d_p_t_winch = (1.1 * 4 * atan(1)) / 360;
double d_p_t_pickup = (44 * 4 * atan(1)) / 360;

int signStore;

float rightD;
float rightV;
float leftD;
float leftV;
float pickupD;
float pickupV;
float pickupA;
float winchD;
float winchV;

float flapA;
float flap;


/*
 * Sensors
 */
DigitalInput *winchSwitch;
int winchS;
bool winchR;

/*
 * Timers
 */
Timer *autotimer;
Timer *teletimer;
float aTimer;
float tTimer;

/*
 * Miscellaneous
 */
UINT32 autoPeriodicLoops;
UINT32 disabledPeriodicLoops;
UINT32 telePeriodicLoops;	


#endif
