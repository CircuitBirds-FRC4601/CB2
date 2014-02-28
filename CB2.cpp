#include "CB2.h"

class CB2 : public IterativeRobot
{
public:
	CB2(void)	{
		
		frontLeft		=		new Victor(2);											//Call motors
		backLeft		=		new Victor(3);
		frontRight		=		new Victor(4);
		backRight		=		new Victor(5);
		spinMotor		=		new Victor(6);
		winchMotor		=		new Victor(7);
		pickupMotor		=		new Victor(8);
		flapperMotor	=		new Victor(9);
		
		drive			=		new RobotDrive(frontLeft, backLeft, frontRight, backRight);

		ds				=		DriverStation::GetInstance();							//Creates instance of Driver Station
		dsLCD			=		DriverStationLCD::GetInstance();
		
		rightEncoder	=		new AugmentedEncoder(3, 4, d_p_t_drive, false);			//Call Encoders
		leftEncoder		=		new AugmentedEncoder(5, 6, d_p_t_drive, true);
		winchEncoder	=		new AugmentedEncoder(7, 8, d_p_t_winch, true);
		pickupEncoder	=		new AugmentedEncoder(9, 10, d_p_t_pickup, false);
		flapEncoder		=		new AnalogChannel(1, 1);
		
		leftPID			=		new SimPID(DRIVEKP, DRIVEKI, DRIVEKD, 2);				//Call PID Controllers
		rightPID		=		new SimPID(DRIVEKP, DRIVEKI, DRIVEKD, 2);
		flapperPID		=		new SimPID(FLAPPERKP, FLAPPERKI, FLAPPERKD, 2);
		pickupPID		=		new SimPID(PICKUPKP, PICKUPKI, PICKUPKD, 2);
		leftPID->setMaxOutput(1.0);
		rightPID->setMaxOutput(1.0);
		flapperPID->setMaxOutput(1.0);
		pickupPID->setMaxOutput(1.0);
		
		pumpAir			=		new Compressor(1,1);									//Call Pnuematics
		hookRelease		=		new DoubleSolenoid(1,2);
		
		leftStick		=		new Joystick(1);										//Call Joysticks
		rightStick		=		new Joystick(2);
		pilotPad		=		new LogitechGamepad(3);

		autotimer		=		new Timer();											//Call Timers
		teletimer		=		new Timer();
		
		winchSwitch		=		new DigitalInput(2);									//Call Limits

		autoPeriodicLoops = 0;															//Variables to 0
		telePeriodicLoops = 0;

		leftD			=		0;
		rightD			=		0;
		winchD			=		0;
		pickupA			=		0;
		
		leftEncoder->Start();															//Start Encoders and Compressor
		rightEncoder->Start();
		pickupEncoder->Start();
		winchEncoder->Start();
		
		pumpAir->Start();

		GetWatchdog().SetExpiration(50);
	}
	
	
	/********************************** Init Routines *************************************/

	void RobotInit(void) {																//Initial LCD Update (at power on)
		Write2LCD();
	}
	
	void DisabledInit(void) {
	}

	void AutonomousInit(void) {															//Setting and Resetting
		autoPeriodicLoops = 0;
		
		autotimer->Reset();
		leftEncoder->Reset();
		rightEncoder->Reset();
		pickupEncoder->Reset();
		winchEncoder->Reset();

		aTimer			=		0;
	}

	void TeleopInit(void) {																//Setting and Resetting
		telePeriodicLoops = 0;
	
		flapA			=		(45 * (flapEncoder->GetAverageVoltage() - 0.5));		//Gets Flapper Angle
		
		pickupEncoder->Reset();
		winchEncoder->Reset();
		leftEncoder->Reset();
		rightEncoder->Reset();
		
		flap = flapA;																	//Stores Angle (prevents a bug that
		winchS = 1;																		//made the flapper move when enabled)
	}

	/********************************** Periodic Routines *************************************/
	
	void DisabledPeriodic(void)  {														//Used to set delay
		GetWatchdog().Feed();
		
		GetControllerIO();
		
		Write2LCD();
	}

	void AutonomousPeriodic(void) {
		GetWatchdog().Feed();
		
		flapA			=		(45 * (flapEncoder->GetAverageVoltage() - 0.5));		//Get all encoder values
		leftD			=		leftEncoder->GetDistance();
		rightD			=		rightEncoder->GetDistance();
		winchD			=		winchEncoder->GetDistance();
		pickupA			=		(1 / d_p_t_pickup) * (pickupEncoder->GetDistance());

		Auton();																		//Auton function (See Below)

		autoPeriodicLoops++;

		Write2LCD();
	}

	void TeleopPeriodic(void) {
		GetWatchdog().Feed();

		drive->TankDrive(leftAxisY, rightAxisY, true);									//Drive Setup
		drive->SetExpiration(50);
		drive->SetSafetyEnabled(true);

		flapA			=		(45 * (flapEncoder->GetAverageVoltage() - 0.5));		//Get all encoder values
		leftD			=		leftEncoder->GetDistance();
		rightD			=		rightEncoder->GetDistance();
		winchD			=		winchEncoder->GetDistance();
		pickupA			=		(1 / d_p_t_pickup) * (pickupEncoder->GetDistance());	//Math for Distance->Angle

		GetControllerIO();																//Get Driver Input
		BallControls();																	//Functions to use the Input
		FlapperAngle();
		WinchDistance();
		PickupRotator();
		
		drive->TankDrive(leftAxisY, rightAxisY, true);									//Drive Setup
		drive->SetExpiration(50);
		drive->SetSafetyEnabled(true);

		Write2LCD();																	//Update LCD
		telePeriodicLoops++;
	}

	/********************************** Miscellaneous Routines *************************************/

	void BallControls() {
		if (shootBall == true) {														//To shoot or not to shoot
			hookRelease->Set(OUT);														//That is the question
		}
		
		else if (shootRetract == true && ((winchS == 1) || (winchS == 0))) {
			hookRelease->Set(IN);
		}
		
		else if (shootBall == true && winchS == false) {
			hookRelease->Set(OUT);
		}
		
		else {
		}
		
		if (leftPadY >= 0.1) {															//Arm Tilt Control
			pickupMotor->Set(leftPadY);
		}
			
		else if (leftPadY <= -0.1) {
			pickupMotor->Set(leftPadY);
		}
		
		else {
		}

	}
	
	void FlapperAngle() {																//Sets Flapper Angle
		if (flapA > (flapStore + 2)) {
			flapperMotor->Set(-0.4);
		}

		else if (flapA < (flapStore - 2)) {
			flapperMotor->Set(0.4);
		}

		else {
			flapperMotor->Set(0.0);
			flapDone = true;
		}
	}

	void WinchDistance() {																//Sets Winch
		if (winchD > (winchStore + .5)) {
			winchMotor->Set(-1.0);
		}
		
		else if (winchD < (winchStore - .5)) {
			winchMotor->Set(1.0);
		}
		
		else {
			winchMotor->Set(0.0);
		}
	}
	
	void PickupRotator() {																//Sets Pickup Arm
		if (pickupStore > 0) {
			signStore = 1;
		}
		
		else if (pickupStore < 0) {
			signStore = -1;
		}
		
		else {
			signStore = 0;
		}

		if (signStore == -1) {
			if (pickupA > (pickupStore - 2)) {
				pickupMotor->Set(-0.3);
			}
			
			else if (pickupA < (pickupStore + 2)) {
				pickupMotor->Set(0.2);
			}
			
			else {
				pickupMotor->Set(0.0);
			}
		}
		
		else if (signStore == 1) {
			if (pickupA < (pickupStore - 2)) {
				pickupMotor->Set(0.2);
			}
			
			else if (pickupA > (pickupStore + 2)) {
				pickupMotor->Set(-0.3);
			}
			
			else {
				pickupMotor->Set(0.0);
			}
		}
		
		else {
			pickupMotor->Set(0.0);
		}
	}
	
	void FlapperControlsAuton() {														//Flapper for Auton
		if (autonFlap == true) {
			if (flapA >= 84.5) {
				flapperMotor->Set(-0.2);
			}
			
			else if(flapA <= 82.5) {
				flapperMotor->Set(0.2);
			}
					
			else {
				flapperMotor->Set(0.0);
			}		

			flap = flapA;
		}
			
		else {																			//Keep in Position
			if ((flapA) <= (flap - 1.5)) {
				flapperMotor->Set(0.1);
			}
					
			else if ((flapA) >= (flap + 1.5)) {
				flapperMotor->Set(-0.1);
			}
					
			else {
				flapperMotor->Set(0.0);
			}
		}
	}
	
	void Auton() {																		//Auton Code
		if ((rightD < 48) && (leftD < 48)) {											//Replace with PID
			driveDone = false;
		}

		else if ((rightD > 50) && (leftD > 50)) {
			driveDone = false;
		}

		else {
			driveDone = true;
			autonFlap = true;
			FlapperControlsAuton();
		}

		PickupRotatorAuton(90);

		if ((driveDone == true) && ((flapA >= 82.5) && (flapA <= 84.5)) && ((pickupA >= 88.5) && (pickupA <= 92.5))) {
			autotimer->Start();

			autonFlap = false;

			aTimer			=		(int) autotimer->Get();

			if (aTimer == (1.0 + shootDelay)) {
				hookRelease->Set(IN);
			}

			else if (aTimer > (1.0 + shootDelay)) {
				hookRelease->Set(OUT);
				PickupRotatorAuton(135);
			}

			else {
			}
		}

		else {
		}
	}

	void PickupRotatorAuton(int y) {													//Set Pickup Arm (auton)
		if (y > 0) {
			signStore = 1;
		}
		
		else if (y < 0) {
			signStore = -1;
		}
		
		else {
			signStore = 0;
		}

		if (signStore == -1) {
			if (pickupA > (y - 2)) {
				pickupMotor->Set(-0.3);
			}
			
			else if (pickupA < (y + 2)) {
				pickupMotor->Set(0.2);
			}
			
			else {
				pickupMotor->Set(0.0);
			}
		}
		
		else if (signStore == 1) {
			if (pickupA < (y - 2)) {
				pickupMotor->Set(0.2);
			}
			
			else if (pickupA > (y + 2)) {
				pickupMotor->Set(-0.3);
			}
			
			else {
				pickupMotor->Set(0.0);
			}
		}
		
		else {
			pickupMotor->Set(0.0);
		}
	}
	
	void Write2LCD() {																	//LCD Update Commands
		dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Flap Angle: %f", flapA);
				
		dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "SPARE");
				
		dsLCD->Printf(DriverStationLCD::kUser_Line3, 1, "Shoot Delay: %f", shootDelay);
		
		dsLCD->Printf(DriverStationLCD::kUser_Line4, 1, "Command Finished: %f", aTimer);
		
		dsLCD->Printf(DriverStationLCD::kUser_Line5, 1, "Ready to Fire: %f", winchD);
		
		dsLCD->Printf(DriverStationLCD::kUser_Line6, 1, "Pickup Angle: %f", pickupA);		
		
		dsLCD->UpdateLCD();
	}
	
	void GetControllerIO(void){															//Get Driver Input
		
		/*
		 * Left Joystick
		 */

		leftAxisY		=		-leftStick->GetY();
		shootDelay		=		leftStick->GetThrottle() + 1;
		spinOut			=		leftStick->GetTrigger();
		
		/*
		 * Right Joystick
		 */

		rightAxisY		=		-rightStick->GetY();
		spinIn			=		rightStick->GetTrigger();
		
		if (spinIn) {
			spinMotor->Set(1.0);
		}
		
		else if (spinOut) {
			spinMotor->Set(-1.0);
		}
		
		else {
			spinMotor->Set(0.0);
		}
		
		/*
		 * Copilot Pad
		 */
		
		leftPadY		=		pilotPad->GetLeftY();									//Pickup Motor
		highGoalShot	=		pilotPad->GetNumberedButton(4);							//Right Side UP
		lowGoalShot		=		pilotPad->GetNumberedButton(2);							//Right Side DOWN
		shootBall		=		pilotPad->GetNumberedButton(10);						//Start
		shootRetract	=		pilotPad->GetNumberedButton(9);							//Select
		trussShot		=		pilotPad->GetNumberedButton(1);							//Right Side RIGHT
		loadBall		=		pilotPad->GetNumberedButton(3);							//Right Side LEFT
		passBall		=		pilotPad->GetNumberedButton(5);							//L1
		lineDrive		=		pilotPad->GetNumberedButton(6);							//R1
		flapDown		=		pilotPad->GetNumberedButton(7);							//L2
		flapUp			=		pilotPad->GetNumberedButton(8);							//R2

		if (flapUp) {																	//Store Flapper, Winch, and Pickup Arm stuff
			flapStore		=		(int) (flapA - 2);
		}
		
		else if (flapDown) {
			flapStore		=		(int) (flapA + 2);
		}
		
		else if (loadBall) {
			flapStore		=		125;
			winchStore		=		0;
			pickupStore		=		120;
		}
		
		else if (highGoalShot) {
			flapStore		=		87;
			winchStore		=		15;
			pickupStore		=		90;
		}
		
		else if (trussShot) {
			flapStore		=		91;
			winchStore		=		15;
			pickupStore		=		90;
		}
		
		else if (lowGoalShot) {
			flapStore		=		92;
		}
		
		else if (passBall) {
			flapStore		=		80;
		}
		
		else if (lineDrive) {
			flapStore		=		50;
			winchStore		=		15;
			pickupStore		=		90;
		}
		
		else {
			flapStore		=		flapStore;											//May need to remove
			winchStore		=		winchStore;											//May need to remove
			pickupStore		=		pickupStore;										//May need to remove
		}	
	}

};

START_ROBOT_CLASS(CB2);
