#include "CB2.h"

class CB2 : public IterativeRobot
{
public:
	CB2(void)	{
		
		frontLeft		=		new Victor(2);
		backLeft		=		new Victor(3);
		frontRight		=		new Victor(4);
		backRight		=		new Victor(5);
		spinMotor		=		new Victor(6);
		winchMotor		=		new Victor(7);
		pickupMotor		=		new Victor(8);
		flapperMotor	=		new Victor(9);

		drive			=		new RobotDrive(frontLeft, backLeft, frontRight, backRight);

		ds				=		DriverStation::GetInstance();
		dsLCD			=		DriverStationLCD::GetInstance();
		priorPacketNumber =	0;
		dsPacketsReceivedInCurrentSecond = 0;

		rightEncoder	=		new AugmentedEncoder(3, 4, d_p_t_drive, false);
		leftEncoder		=		new AugmentedEncoder(5, 6, d_p_t_drive, true);
		winchEncoder	=		new AugmentedEncoder(7, 8, d_p_t_winch, true);
		pickupEncoder	=		new AugmentedEncoder(9, 10, d_p_t_pickup, false);	
		flapEncoder		=		new AnalogChannel(1, 1);
		
		pumpAir			=		new Compressor(1,1);
		hookRelease		=		new DoubleSolenoid(1,2);
		
		leftStick		=		new Joystick(1);
		rightStick		=		new Joystick(2);
		pilotPad		=		new LogitechGamepad(3);
		
		autotimer		=		new Timer();
		teletimer		=		new Timer();	
		winchSwitch		=		new DigitalInput(2);

		autoPeriodicLoops = 0;
		disabledPeriodicLoops = 0;
		telePeriodicLoops = 0;

		pickupD		=		0;
		leftD		=		0;
		rightD		=		0;
		winchD		=		0;
		pickupA		=		0;
		winchS		=		1;
		
		leftEncoder->Start();
		rightEncoder->Start();
		pickupEncoder->Start();
		winchEncoder->Start();
		
		pumpAir->Start();

		GetWatchdog().SetExpiration(50);
	}
	
	
	/********************************** Init Routines *************************************/

	void RobotInit(void) {
		Write2LCD();
	}
	
	void DisabledInit(void) {
		disabledPeriodicLoops = 0;
	}

	void AutonomousInit(void) {
		autoPeriodicLoops = 0;
		
		autotimer->Reset();
		
		leftEncoder->Reset();
		rightEncoder->Reset();
		pickupEncoder->Reset();
		winchEncoder->Reset();
	
		autonFlap = false;
		aTimer = 0;
		
		shootBall = false;
		shootRetract = false;
		winchDown = false;
		autonMode		=		(int) (rightStick->GetThrottle() + 2);
		
		shootDelay		=		leftStick->GetThrottle() + 1;
	}

	void TeleopInit(void) {
		telePeriodicLoops = 0;
		
		flapA			=		(45 * (flapEncoder->GetAverageVoltage() - 0.5));
		
		pickupEncoder->Reset();
		winchEncoder->Reset();
		leftEncoder->Reset();
		rightEncoder->Reset();
		
		flap = flapA;

		dsPacketsReceivedInCurrentSecond = 0;
	}

	/********************************** Periodic Routines *************************************/
	
	void DisabledPeriodic(void)  {
		GetWatchdog().Feed();
		
		autonMode		=		(rightStick->GetThrottle() + 2);
		
		shootDelay		=		leftStick->GetThrottle() + 1;
		
		disabledPeriodicLoops++;

		Write2LCD();
	}

	void AutonomousPeriodic(void) {
		GetWatchdog().Feed();

		flapA			=		(45 * (flapEncoder->GetAverageVoltage() - 0.5));
	
		pickupD		=		pickupEncoder->GetDistance();
		leftD		=		leftEncoder->GetDistance();
		rightD		=		rightEncoder->GetDistance();
		winchD		=		winchEncoder->GetDistance();
		pickupA		=		(1 / d_p_t_pickup) * pickupD;
		
		if (autonMode == 1) {				//Left or Right
			if ((rightD < 48) && (leftD < 48)) {
				DriveAuton(1.0);
				driveDone = false;
			}
			
			else if ((rightD > 50) && (leftD > 50)) {
				DriveAuton(-.75);
				driveDone = false;
			}
			
			else {
				DriveAuton(0.0);
				driveDone = true;
				autonFlap = true;
				FlapperControlsAuton();
			}

			PickupRotator(90);
			
			if ((driveDone == true) && ((flapA >= 82.5) && (flapA <= 84.5)) && ((pickupA >= 88.5) && (pickupA <= 92.5))) {
				autotimer->Start();
				
				aTimer			=		(int) autotimer->Get();
				
				if (aTimer == (1.0 + shootDelay)) {
					hookRelease->Set(IN);
				}
				
				else if (aTimer > (1.0 + shootDelay)) {
					hookRelease->Set(OUT);
					PickupRotator(135);
				}
				
				else {
				}
			}
			
			else {
			}
		}
		
		else if (autonMode == 2) {			//Middle Left
			
		}
		
		else if (autonMode == 3) {			//Middle Right
			
		}
		
		else {								//Anywhere
			
		}
		
		autoPeriodicLoops++;
		
		Write2LCD();
	}

	void TeleopPeriodic(void) {
		GetWatchdog().Feed();
		
		telePeriodicLoops++;			 
		
		flapA			=		(45 * (flapEncoder->GetAverageVoltage() - 0.5));
		
		pickupD		=		pickupEncoder->GetDistance();
		leftD		=		leftEncoder->GetDistance();
		rightD		=		rightEncoder->GetDistance();
		winchD		=		winchEncoder->GetDistance();
		pickupA		=		(1 / d_p_t_pickup) * pickupD;
		
		GetControllerIO();

		TeleopControls();
		
		ShootBall();
		
		dsPacketsReceivedInCurrentSecond++;

		drive->TankDrive(leftAxisY, rightAxisY, true);
		drive->SetExpiration(50);
		drive->SetSafetyEnabled(true);
		
		Write2LCD();
	}

	/********************************** Miscellaneous Routines *************************************/
	
	void Write2LCD() {	
		dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Flap Angle: %f", flapA);
				
		dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "Winch Switch: %f", winchS);
				
		dsLCD->Printf(DriverStationLCD::kUser_Line3, 1, "Winch Distance: %f", winchD);
		
		dsLCD->Printf(DriverStationLCD::kUser_Line4, 1, "Right Distance: %f", rightD);
		
		dsLCD->Printf(DriverStationLCD::kUser_Line5, 1, "Left Distance: %f", leftD);
		
		dsLCD->Printf(DriverStationLCD::kUser_Line6, 1, "Pickup Angle %f", pickupA);		
		
		dsLCD->UpdateLCD();
	}

	void ShootBall() {						//DONE
		if (shootBall == true) {
			hookRelease->Set(OUT);
		}
		
		else if (shootRetract == true && ((winchS == 1) || (winchS == 0))) {
			hookRelease->Set(IN);
		}
		
		else if (shootBall == true && winchS == 1) {
			hookRelease->Set(OUT);
		}
		
		else {
		}
		
		if (winchDown == true && winchS == 1) {
			winchMotor->Set(-1.0);
			
			winchS = winchSwitch->Get();
		}
		
		else if ((winchUp == true) && ((winchS == 1) || (winchS == 0))) {
			winchMotor->Set(1.0);
		}
		
		else if (winchS == 0 && winchDown == true) {
			winchMotor->Set(0.0);
			hookRelease->Set(OUT);
			
			winchEncoder->Reset();
			winchD = 0;
		}
		
		else{
			winchMotor->Set(0.0);
			winchS = 1;
		}
	}
	
	void TeleopControls() {
		if (rightPadY >= 0.1) {					//Arm Wheel Control
			spinMotor->Set(rightPadY);
		}
		
		else if (rightPadY <= -0.1) {
			spinMotor->Set(rightPadY);
		}
		
		else {
			spinMotor->Set(0.0);
		}

		if (leftPadY >= 0.1) {					//Arm Tilt Control
			pickupMotor->Set(leftPadY);
		}
			
		else if (leftPadY <= -0.1) {
			pickupMotor->Set(leftPadY);
		}
		
		else {
			pickupMotor->Set(0.0);
		}


		if (flapUp == true) {					//MANUAL
			flapperMotor->Set(-0.4);
			flap = flapA;
		}
		
		else if (flapDown == true) {
			flapperMotor->Set(0.4);
			flap = flapA;
		}
		
		else if (preset1 == true) {			//HOME		
			PickupRotator(135);
		}
		
		else if (preset2 == true) {			//GOAL SHOT
			if (flapA >= 83.5) {
				flapperMotor->Set(-0.4);
			}
			
			else if(flapA <= 81.5) {
				flapperMotor->Set(0.4);
			}
			
			else {
				flapperMotor->Set(0.0);
			}
			
			flap = flapA;
		}
	
		else if (preset3 == true) {			//OVER TRUSS
			if (flapA >= 117.5) {
				flapperMotor->Set(-0.4);
			}
			
			else if(flapA <= 115.5) {
				flapperMotor->Set(0.4);
			}
			
			else {
				flapperMotor->Set(0.0);
			}
			
			flap = flapA;
		}
		
		else if (preset4 == true) {			//LOAD
			if (flapA >= 134.5) {
				flapperMotor->Set(-0.4);
			}
			
			else if(flapA <= 132.5) {
				flapperMotor->Set(0.4);
			}
			
			else {
				flapperMotor->Set(0.0);
			}
			
			flap = flapA;
		}
		
		else {									//Keep in Position
			if ((flapA) <= (flap - 1.5)) {
				flapperMotor->Set(0.2);
			}
			
			else if ((flapA) >= (flap + 1.5)) {
				flapperMotor->Set(-0.2);
			}
			
			else {
				flapperMotor->Set(0.0);
			}
		}
	}
	
	void DriveAuton(float x) {			//Insert Motor Speeds
		frontLeft->Set(x);
		backLeft->Set(x);
		frontRight->Set(-.75 * x);
		backLeft->Set(-.75 * x);
	}
	
	void PickupRotator(int y) {			//Insert Angle
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
	
	void FlapperControlsAuton() {
		if (autonFlap == true) {			//LOAD
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
			
		else {									//Keep in Position
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
	
	void GetControllerIO(void){
		
		/*
		 * Left Joystick
		 */

		leftAxisY		=		-leftStick->GetY();
		
		/*
		 * Right Joystick
		 */

		rightAxisY		=		-rightStick->GetY();
		
		/*
		 * Copilot Pad
		 */
		
		rightPadY		=		pilotPad->GetRightY();
		leftPadY		=		pilotPad->GetLeftY();
		flapUp			=		pilotPad->GetNumberedButton(4);
		flapDown		=		pilotPad->GetNumberedButton(2);
		shootBall		=		pilotPad->GetNumberedButton(10);
		shootRetract	=		pilotPad->GetNumberedButton(9);
		winchDown		=		pilotPad->GetNumberedButton(1);
		winchUp			=		pilotPad->GetNumberedButton(3);
		preset1			=		pilotPad->GetNumberedButton(5);
		preset2			=		pilotPad->GetNumberedButton(6);
		preset3			=		pilotPad->GetNumberedButton(7);
		preset4			=		pilotPad->GetNumberedButton(8);

	}

};

START_ROBOT_CLASS(CB2);
