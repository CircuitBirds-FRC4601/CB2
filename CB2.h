#include "WPILib.h"
#include "LogitechGamepad.h"
#include "TractionControl.h"
#include "Math.h"
#include "SimPID.h"

/*
 * Defines
 */
#define OUT			DoubleSolenoid::kForward
#define IN			DoubleSolenoid::kReverse
#define OFF			DoubleSolenoid::kOff

#define DRIVEKP		0.070						//Test at Buckeye to find
#define DRIVEKI		0.000
#define DRIVEKD		0.000
float leftM;
float rightM;

#define FLAPPERKP	0.007						//Test at Buckeye to find
#define FLAPPERKI	0.000
#define FLAPPERKD	0.000
float flapperM;

#define PICKUPKP	0.035						//Test at Buckeye to find
#define PICKUPKI	0.000
#define PICKUPKD	0.000
float pickupM;

/*
 * Drive Train
 */
RobotDrive *drive;

/*
 * PID
 */
SimPID *leftPID;
SimPID *rightPID;
SimPID *flapperPID;
SimPID *pickupPID;

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

/*
 * Driver Station
 */
DriverStation *ds;
DriverStationLCD *dsLCD;

/*
 * Controls
 */
Joystick *rightStick;
Joystick *leftStick;
LogitechGamepad *pilotPad;

float leftAxisY;
float rightAxisY;
float leftPadY;

bool flapUp;
bool flapDown;
bool highGoalShot;
bool lowGoalShot;
bool trussShot;
bool passBall;
bool lineDrive;
bool loadBall;
bool shootBall;
bool shootRetract;
bool spinIn;
bool spinOut;

bool autonFlap;

float shootDelay;

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
float leftD;
float pickupA;
float winchD;
float flap;
float flapA;
int flapStore;
int winchStore;
int pickupStore;

bool driveDone;
bool flapDone;
bool winchDone;

DigitalInput *winchSwitch;
int winchS;

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
UINT32 telePeriodicLoops;	
