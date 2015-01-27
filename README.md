# Zumo Sonar Library 
**Sonar library for Project Lab**  
![Zumol](http://i.snag.gy/f6lHj.jpg)  
[Doxygen Documentation](http://maslows.github.io/Zumo---Sonar/index.html)
## Features
* Control of HC-SR04 Sonar
* Control of Power HD 1160A Servo
* Many work modes
	* Constant measurement at fixed position 
	* Constant measurement in two different sweep modes
	* Single measurement at given angles using interupts and polling
* Support for averaging of measurements
* Pseudo feedback for servo. 
* Labview interface 

## Setup - hardware
Connect sonar and servo pins as follows:

**Sonar:**
* PTE20	<-> Sonar	Echo pin
* PTE21	<-> Sonar	Trigger pin
* 5V    <-> Sonar Vcc
* Gnd   <-> Sonar Gnd

We recommend using voltage divider on Echo pin   
![Divider](http://i.snag.gy/JsRG8.jpg)

**Servo:**
* PTE22	<-> Servo	Yellow cable
* 5V    <-> Servo Red Cable
* Gnd   <-> Servo Brown Cable

We recommend connecting 470uF capacitor near Servo power connectors.

## Setup - software
1. Copy `sonar.h, sonar.c, servo.c` and `servo.h` files to your project.
2. Include both headers in your main.c file  

   ```c
   #include "sonar.h"
   #include "servo.h"
   ```
   
3. In your main function initialize Sonar anf Servo with proper options.

  ```c
  int main(void){
  	Sonar_init(SINGLE);
  	Servo_init(MANUAL, SCAN_AND_GO);
  	(...)
  }
   ```
   Check documentation for details: [Servo_init()](http://maslows.github.io/Zumo---Sonar/servo_8c.html#ab944078572fad93e2090509e3957005b) 
   and [Sonar_init()](http://maslows.github.io/Zumo---Sonar/sonar_8c.html#a11b02ebbeab065aabcab82dd386024ea).

4. Decide how you want to retrieve measurements results
	* Via Interupt (Both Single and Sweep measurements modes)
		In `sonar.c` file at the bottom, there is a function called [SonarDistHandler](http://maslows.github.io/Zumo---Sonar/sonar_8c.html#ae81e58eda1f172e0f29250cb6ac5ed7a).
		This function is called when Sonar obtains some usable data. For example if you want to send the results you can use it like this:
		To trigger measurement you can use CONTINUOUS Sonar mode or [SonarStartMeas](http://maslows.github.io/Zumo---Sonar/sonar_8c.html#af0c249436f7ef0657389cc40941da5cf) function.
		
		```c
		void SonarDistHandler(uint16_t distance_cm, int32_t angle){	
			/* Your code here */
			char buffor[12];
			sprintf(buffor, "%04d,%04hu\n",angle,distance_cm);
			bt_sendStr(buffor);
		}	
		```
	* Via Pooling
		If you just want to easly get single measurement, use [SonarGetDistance](http://maslows.github.io/Zumo---Sonar/sonar_8c.html#acbfc04f4efdc8692cf46217b1331bfd4) function somewhere in your code.
		For example, to get distance measurement at angle 45deg use
		
		```c
		int result = SonarGetDistance(new_deg);
		```
5. Change Sonar and Servo work modes during runtime if needed. You should only use these functions for this purpose.
**Do NOT modify the content of any Servo or Sonar variables by hand!**
	 * [ServoChangeMode](http://maslows.github.io/Zumo---Sonar/servo_8c.html#a1c1e027654d29aef3848af9f2e118688)
	 * [ServoChangeSweepMode](http://maslows.github.io/Zumo---Sonar/servo_8c.html#a1865a21f6683bb41e8a935c33caa3363)
	 * [ServoChangeLockRange](http://maslows.github.io/Zumo---Sonar/servo_8c.html#addddfc1dc1dee155e313d00b0f660075)
	 * [SonarChangeMode](http://maslows.github.io/Zumo---Sonar/sonar_8c.html#a7b4e1f5dddc103f0cb52c133156c241e)
	 
6. In `sonar.h` and `servo.h` there are few parameters which can be changed, depending on needs. For example, how many
samples to average or measurement interval in continous mode. **Before you try and modyfy any of them, read the comments!**

## Work modes
There are sevral work modes available.

### Single measurements 
The easiest way to get distance is to use [SonarGetDistance](http://maslows.github.io/Zumo---Sonar/sonar_8c.html#acbfc04f4efdc8692cf46217b1331bfd4)
or [SonarStartMeas](http://maslows.github.io/Zumo---Sonar/sonar_8c.html#af0c249436f7ef0657389cc40941da5cf) function.
Difference between those two funtions is simple: The first one will constantly pool the Sonar until it retrieves the result. Not very efficient, but very simple to use.
The second one will trigger a measurement and exit. When Sonar gets the result, it will trigger [SonarDistHandler](http://maslows.github.io/Zumo---Sonar/sonar_8c.html#ae81e58eda1f172e0f29250cb6ac5ed7a).

### Continuous measurement
If you want to get a steady stream of measurements, use Sonar in CONTINUOUS mode. Use [SonarDistHandler](http://maslows.github.io/Zumo---Sonar/sonar_8c.html#ae81e58eda1f172e0f29250cb6ac5ed7a) to retrieve data.
If you want to change sonar position, use [ServoMoveByDegree](http://maslows.github.io/Zumo---Sonar/servo_8c.html#a0efffe5db4190a16376b48ab8ae0ac2c).
Remember to set Servo to manual mode first!

### Sweep modes
There are two [Sweep modes](http://maslows.github.io/Zumo---Sonar/servo_8h.html#a9b2b8b593017fd9289c835aebc4e643b) available:
* Sweep and Go
	Sonar will move constantly by angle defined by [SERVO_STEP_DEG](http://maslows.github.io/Zumo---Sonar/servo_8h.html#a3a036039969fe1374ce619c5be838d61).
	At each step it will try to measure the distance and return the result via  [SonarDistHandler](http://maslows.github.io/Zumo---Sonar/sonar_8c.html#ae81e58eda1f172e0f29250cb6ac5ed7a).
* Sweep and Lock
	Sonar will move until it finds something closer than [ServoLockRange](http://maslows.github.io/Zumo---Sonar/servo_8c.html#aeaa7cfdb4429db0c616191749d831110).
	You can change this value using [ServoChangeLockRange](http://maslows.github.io/Zumo---Sonar/servo_8c.html#addddfc1dc1dee155e313d00b0f660075) function.
	When the lock is lost, servo will resume sweep until it locks again.

## Labview interface
We created an intreface for testing sonar. [LINK - EXE](https://www.dropbox.com/s/oskoljoqz5cncie/zumo_control.zip?dl=0).  
![Control Panel](http://i.snag.gy/A61br.jpg)  
**Note: It requires bluetooth module to work.
If you need labview files, send us an email.



	 
	
