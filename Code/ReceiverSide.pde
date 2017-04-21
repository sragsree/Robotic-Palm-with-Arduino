include<Servo.h >

Servo myservo1; // create servo object to control a servo
Servo myservo2;
Servo myservo3;
Servo myservo4;
Servo myservo5;

void setup()
{
	Serial.begin(9600);
	myservo1.attach(2); // attaches the servo on pin 9 to the servo object
	myservo2.attach(3);
	myservo3.attach(4);
	myservo4.attach(5);
	myservo5.attach(6);
	void loop()
	{
		if(Serial.available() >=5)
		byte servoAng1 = Serial.read();
		byte servoAng2 = Serial.read();
		byte servoAng3 = Serial.read();
		byte servoAng4 = Serial.read();
		byte servoAng5 = Serial.read();
		myservo1.write(servoAng1);
		myservo2.write(servoAng2);	
		myservo3.write(servoAng3);
		myservo4.write(servoAng4);
		myservo5.write(servoAng5);
	}
}	