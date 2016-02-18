/*
// =========================================================================================
//                              BB-8 Padawan Body Code
//                              Version 1.00
// =========================================================================================
//                               Original Developer: danf
//                              Revised  Date: 02/09/15
//   Designed to be used with a second Arduino running the Padawan Dome code
//              EasyTransfer and PS2X_lib libraries by Bill Porter
//
//
//         Set Sabertooth 2x25 Dip Switches 1 and 2 Down, All Others Up
//         For SyRen packetized Serial Set Switches 1 and 2 Down, All Others Up
//         For SyRen Simple Serial Set Switchs 2 & 4 Down, All Others Up
//         Placed a 10K ohm resistor between S1 & GND on the SyRen 10 itself
//
//         This program is free software: you can redistribute it and/or modify it .
//         This program is distributed in the hope that it will be useful,
//         but WITHOUT ANY WARRANTY; without even the implied warranty of
//         MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
*/

// =================== SET AUDIO OPTIONS ===================================================

// NOTE: Sketch currently does NOT support 2 SparkfunMP3 Triggers. 
//       You can use 1 SparkfunMP3 Trigger either with or without a BY8x01-16P module

// Primary Audio Player...
byte AUDIO1 = 2;  // 1=Sparkfun MP3 Trigger
                  // 2=BY8x01-16P Module
                  
//Secondary Audio Player (used for music and background sfx)...
byte AUDIO2 = 0; // 0=No secondary audio player (music will be played by primary)
                  // 1=Sparkfun MP3 Trigger
                  // 2=BY8x01-16P Module
				  
//Start-up Sound Options				  
//#define SNDSTRT 1 //Sound number to play on startup
//#define SNDDRIV 2 //Sound number to play when foot drives enabled
//#define SNDAUTO 3 //Sound number to play when Dome Auto Mode engaged  

//JukeBox Options                    
#define jukeBoxLower 9 //Lowest number track to play in jukeBox mode
#define jukeBoxUpper 12 //Highest number track to play in jukeBox mode



// =================== SET DOME, DRIVE, AND TURN SPEED =====================================

byte drivespeed1 = 100; //set these 3 to whatever speeds work for you. 0-stop, 127-full speed.
byte drivespeed2 = 127; //Recommend beginner: 50 to 75, experienced: 100 to 127, I like 100.
byte drivespeed3 = 0;   //Set to 0 if you only want 2 speeds.

byte drivespeed = drivespeed1;  
                                              
byte turnspeed = 70;  // the higher this number the faster it will spin in place, lower - easier to control. 
                      // Recommend beginner: 40 to 50, experienced: 50 $ up, I like 75   
                                         
byte domespeed = 127; // If using a speed controller for the dome, sets the top speed
                      // Use a number up to 127 for serial                       
                      
byte ramping = 5;     // Ramping- the lower this number the longer R2 will take to speedup or slow down,
                      // change this by incriments of 1               
                          
byte domeRamping = 1; // domeRamping- the lower this number the longer R2's dome will take to speedup or slow down,
                      // change this by incriments of 1                                     

byte domecompensation = 10;  // For controllers that centering problems, causing slight dome drift in one direction

byte drivecompensation = 20; // use the lowest number with no drift

int domeBaudeRate = 9600; // Set the baude rate for the Syren motor controller
                          // for packetized options are: 2400, 9600, 19200 and 38400
                          // for simple use 9600
                          
//#define SYRENSIMPLE // Comment out for packetized serial connection to Syren - Recomended
                      // Un-comment for simple serial - do not use in close contact with people.
                        
                                           
// =================== INCLUDE LIBS AND DECLARE VARIABLES===================================

/*   For Arduino 1.0 and newer, do this:   */
#include <SoftwareSerial.h>
SoftwareSerial domeSerial(8, 5);
SoftwareSerial STSerial(8, 4);
SoftwareSerial SyRSerial(8, 3);
SoftwareSerial MP3Serial1(8, A1); // SoftwareSerial port assignments (RX, TX) to communicate with sound module 1
SoftwareSerial MP3Serial2(8, 6);  // SoftwareSerial port assignments (RX, TX) to communicate with sound module 2

/*   For Arduino 22 and older, do this:   */
//#include <NewSoftSerial.h>
//NewSoftSerial domeSerial(2, 8);//create software serial port
//NewSoftSerial STSerial(2, 7);
//NewSoftSerial SyRSerial(2, 5);

#include <Sabertooth.h>
#include <SyRenSimplified.h>
#include <PS2X_lib.h>  //for v1.7
#include <Servo.h> 
#include <SoftEasyTransfer.h>

int mp3BaudeRate = 9600; // Set the baude rate for the BY8x01-16P sound module

#include <MP3Trigger.h>

#include <BY8x0116Pv2.h>

int busyPin = A0;  // The pin number of the busy pin on BY8x01-16P #1
int busyPin2 = 2;  // The pin number of the busy pin on BY8x01-16P #2


//#if (AUDIO1==2)
BY8x0116Pv2 BY8(MP3Serial1);
//#else
MP3Trigger trigger;
//#endif

//#if (AUDIO2==2)
BY8x0116Pv2 BY82(MP3Serial2);
//#endif


//////////////////////////////////////////////////////////////////
Sabertooth ST(128, STSerial);
#if defined(SYRENSIMPLE)
SyRenSimplified SyR(SyRSerial); // Use SWSerial as the serial port.
#else
Sabertooth SyR(128, SyRSerial);
#endif
SoftEasyTransfer ET;//create object

struct SEND_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to send
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  int snd; //remote sound trigger
  //int hpx; // hp movement
  //int hpy; // hp movement
  int hpl; // hp light
  //int hpa; // hp automation
  //int dsp; // 100=no change, 0=random, 1=alarm, 4=whistle, 5=leia, 6=short circut, 10=EQ, 11=alarm2,
};         // 21=speed1, 22=speed2, 23=speed3, 24=logics+, 25=logics-

SEND_DATA_STRUCTURE domeData;//give a name to the group of data

PS2X ps2x; // create PS2 Controller Class
int error = 0; // part of the ps2x lib
byte type = 0; // part of the ps2x lib
byte vibrate = 0; // part of the ps2x lib

byte BY8Vol = 25; // 30 = full volume, 0 off
byte trigVol = 30;
byte BY8Vol2 = 22; // 30 = full volume, 0 off
byte trigVol2 = 30;

byte drive = 0; // 0 = drive motors off ( right stick disabled )
byte automate = 0;
unsigned long automateMillis = 0;
byte automateDelay = random(3,10);// set this to min and max seconds between sounds
int turnDirection = 20;
byte action = 0;
unsigned long DriveMillis = 0;
int drivenum = 0;
int sticknum = 0;
int domenum = 0;
int domeSticknum = 0;
int turnnum = 0;
byte attempts = 0;
boolean jukeBox = 0;
long jbMillis = 0;
unsigned long timer[1];
byte timerState[1];


// ================= SETUP AND RUN ONCE =====================================================

void setup(){
  SyRSerial.begin(domeBaudeRate);
  MP3Serial1.begin(mp3BaudeRate);
  MP3Serial2.begin(mp3BaudeRate);
    
  //BY8.init(busyPin);  // Init the player with the MP3 BUSY pin connected to Arduino pin defined
  //BY82.init(busyPin2);  // Init the player with the MP3 BUSY pin connected to Arduino pin defined
  
  BY82.setVolume(BY8Vol2);
  BY8.setVolume(BY8Vol);
    
  #if defined(SYRENSIMPLE)
  SyR.motor(0);            
  #else
  SyR.autobaud(); 
  #endif 
    
  STSerial.begin(9600);   // 9600 is the default baud rate for Sabertooth packet serial.
  ST.autobaud();          // Send the autobaud command to the Sabertooth controller(s).
                          // NOTE: *Not all* Sabertooth controllers need this command.
                          //       It doesn't hurt anything, but V2 controllers use an
                          //       EEPROM setting (changeable with the function setBaudRate) to set
                          //       the baud rate instead of detecting with autobaud.
                          //
                          //       If you have a 2x12, 2x25 V2, 2x60 or SyRen 50, you can remove
                          //       the autobaud line and save yourself two seconds of startup delay.
  ST.setTimeout(950);
    
  #if !defined(SYRENSIMPLE)
  SyR.setTimeout(950);
  #endif
    
  ST.setDeadband(drivecompensation);
  ST.drive(0); // The Sabertooth won't act on mixed mode packet serial commands until
  ST.turn(0);  // it has received power levels for BOTH throttle and turning, since it
               // mixes the two together to get diff-drive power levels for both motors.
 trigger.setup();
 
 
while(attempts<25) { //loop around until a PS2 controller is found or we've attempted 25 times
	error = ps2x.config_gamepad(13,11,10,12, true, false);   //setup pins and settings:  GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
	if((error==0)||(error==3)) attempts=26; //PS2 controller was found, or found with pressure disabled so we'll get out of this WHILE loop 
	attempts++;
}
 
 trigger.setVolume(trigVol);//anything with trigger. is for the mp3trigger
 type = ps2x.readType();
    
 domeSerial.begin(57600);//start the library, pass in the data details and the name of the serial port.
 ET.begin(details(domeData), &domeSerial);
    
 BY82.setVolume(BY8Vol2);//Yes, this is a repeat, but somtimes the first one doesn't take :p
 BY8.setVolume(BY8Vol);
 }
 
 // ============== FUNCTIONS TO PLAY SOUND OR MUSIC ==========================================
void playSound(int soundNumber) {
  if (AUDIO1 == 1)
    trigger.trigger(soundNumber);
  else  
    BY8.playFile(soundNumber);
  //endif  
}

void playMusic(int musicNumber) {
  if (AUDIO2 == 1)
    trigger.trigger(musicNumber);
  else if (AUDIO2 == 2)
    BY82.playFile(musicNumber);
  else  
   playSound(musicNumber);
  //endif 
}

void setVol1(byte BY8Vol1, byte trigVol1) {
  if (AUDIO1 == 2)
     BY8.setVolume(BY8Vol1);
   else
     trigger.setVolume(trigVol1);}
     
void setVol2(byte  BY8Vol2, byte trigVol2) {
  if (AUDIO2 == 2)
     BY82.setVolume( BY8Vol2);
   else
     trigger.setVolume(trigVol2);}     


// ================ DELAY WITHOUT DELAY =======================================================
int delayMilliSeconds(int timerNumber,unsigned long delaytime){
  unsigned long timeTaken;
  if (timerState[timerNumber]==0){    //If the timer has been reset (which means timer (state ==0) then save millis() to the same number timer, 
    timer[timerNumber]=millis();
    timerState[timerNumber]=1;      //now we want mark this timer "not reset" so that next time through it doesn't get changed.
  }
  if (millis()> timer[timerNumber]){
    timeTaken=millis()+1-timer[timerNumber];    //here we see how much time has passed
  }
  else{
    timeTaken=millis()+2+(4294967295-timer[timerNumber]);    //if the timer rolled over (more than 48 days passed)then this line accounts for that
  }
  if (timeTaken>=delaytime) {          //here we make it easy to wrap the code we want to time in an "IF" statement, if not then it isn't and so doesn't get run.
     timerState[timerNumber]=0;  //once enough time has passed the timer is marked reset.
     return 1;                          //if enough time has passed the "IF" statement is true
  }
  else {                               //if enough time has not passed then the "if" statement will not be true.
    return 0;
  }
}


// ================== LOOP RUN OVER AND OVER =================================================
void loop(){
    
 if(error == 1) //skip loop if no controller found
 
  return; 
 
  if(ps2x.Analog(PSS_RX) ==255 && ps2x.Analog(PSS_RY) ==255 && ps2x.Analog(PSS_LX)==255 &&ps2x.Analog(PSS_LY)==255)
 { 
  ST.drive(0);
  ST.turn(0);
  SyR.motor(1,0);
  return; 
 }
  ps2x.read_gamepad();   //read controller and set large motor to spin at 'vibrate' speed


// =================== ENABLE / DISABLE RIGHT STICK & PLAY SOUND =============================
 if(ps2x.ButtonPressed(PSB_START)) 
{if (drive<1)
    {drive = 1; playSound(52);}
 else {drive = 0; playSound(53);}
}

/*
// ================== TURN HP AUTOMATION OR AUTOMATE ON & OFF ================================
 if(ps2x.ButtonPressed(PSB_SELECT))
  {if(ps2x.Button(PSB_R2))
   {if(domeData.hpa == 1)
    {domeData.hpa = 0; domeData.dsp = 100; ET.sendData();}//if hp automation is off, turn it on
    else
    {domeData.hpa = 1; domeData.dsp = 100; ET.sendData();}//turn hp automation off
  } 

else
{if (automate<1)
{automate = 1; playSound(52);}
else {automate = 0; action = 0; playSound(53);}
}}
*/
    
// ==================== AUTOMATE =============================================================
if (automate == 1)
{ if (ps2x.Analog(PSS_LX)!=128)
  { automateMillis = millis();}
  unsigned long currentMillis = millis();
   if (currentMillis - automateMillis > (automateDelay*1000))
 {
  automateMillis = millis();
  action = random(1,5);
  if (action>1)
  {
   (playSound(random(32,52)));
  }}
  if (action<4)
  {
   #if defined(SYRENSIMPLE) 
   SyR.motor(turnDirection);
   #else
   SyR.motor(1,turnDirection);
   #endif
  
  if (delayMilliSeconds(0,750))
  {
  action =5;  
  #if defined(SYRENSIMPLE) 
  SyR.motor(0);
  #else
  SyR.motor(1,0);
  #endif
  
   if (turnDirection>0)
      {turnDirection = -48;}
   else 
      {turnDirection = 45;}
  
  automateDelay = random(3,10);// set this to min and max seconds between sounds
}}}


// ================= VOLUME CONTROL ==========================================================
 if(ps2x.ButtonPressed(PSB_PAD_UP)) 
 {
  if(ps2x.Button(PSB_R1))
   { if (BY8Vol<30)
   {BY8Vol++;
    trigVol-=15;
    setVol1(BY8Vol,trigVol);} //volume up
   }  
 }
  if(ps2x.ButtonPressed(PSB_PAD_DOWN)) 
  {
  if(ps2x.Button(PSB_R1))
   { if (BY8Vol>0)
   {BY8Vol--;
    trigVol+=15;
    setVol1(BY8Vol, trigVol);} //volume down   
   } 
 }
 if(ps2x.ButtonPressed(PSB_PAD_RIGHT)) 
 {
  if(ps2x.Button(PSB_R1))
   { if (BY8Vol2<30)
   {BY8Vol2++;
    trigVol2-=15;
   setVol2(BY8Vol2, trigVol2);} //volume up
   }  
 }
  if(ps2x.ButtonPressed(PSB_PAD_LEFT)) 
  {
  if(ps2x.Button(PSB_R1))
   { if (BY8Vol2>0)
   {BY8Vol2--;
    trigVol2+=15;
   setVol2(BY8Vol2, trigVol2);} //volume down   
   } 
 }
 
    
// ================== STOP ALL SOUNDS AND MUSIC ==============================================
 if(ps2x.Button(PSB_R1)&&ps2x.Button(PSB_R2)&&ps2x.Button(PSB_L1)&&ps2x.Button(PSB_L2))
 {
   if (AUDIO1 == 2)
     {BY8.stopPlay();}//; domeData.dsp = 0; ET.sendData();}
   if (AUDIO2 == 2)  
     {BY82.stopPlay();}//; domeData.dsp = 0; ET.sendData();}
   if (AUDIO1 == 1)  
     {trigger.stop();}//; domeData.dsp = 0; ET.sendData();}
   if (AUDIO2 == 1)  
     {trigger.stop();}//; domeData.dsp = 0; ET.sendData();}
 }
 
    
// =================== LOGIC DISPLAY BRIGHTNESS ==============================================
 if(ps2x.ButtonPressed(PSB_PAD_UP)) 
 {
  //if(ps2x.Button(PSB_L1))
   //{
     //domeData.dsp = 24; ET.sendData();
   //}  
 }
  if(ps2x.ButtonPressed(PSB_PAD_DOWN)) 
  {
  //if(ps2x.Button(PSB_L1))
   //{
     //domeData.dsp = 25; ET.sendData();
   //} 
 }
 
/*
// ==================== HP MOVEMENT ============================================================
if(ps2x.ButtonPressed(PSB_PAD_UP))
{if (!((ps2x.Button(PSB_R1))||(ps2x.Button(PSB_L1))))
 {
   domeData.hpy = 30; domeData.dsp = 100; ET.sendData();
 }}
 if(ps2x.ButtonPressed(PSB_PAD_DOWN))
{if (!((ps2x.Button(PSB_R1))||(ps2x.Button(PSB_L1))))
 {
   domeData.hpy = 150; domeData.dsp = 100; ET.sendData();
 }}
 if(ps2x.ButtonReleased(PSB_PAD_UP)||ps2x.ButtonReleased(PSB_PAD_DOWN))
 {domeData.hpy = 0; domeData.dsp = 100; ET.sendData();}
  
 if(ps2x.ButtonPressed(PSB_PAD_RIGHT))
 {
   domeData.hpx = 150; domeData.dsp = 100; ET.sendData();
 }
 if(ps2x.ButtonPressed(PSB_PAD_LEFT))
 {
   domeData.hpx = 30; domeData.dsp = 100; ET.sendData();
 }
 if(ps2x.ButtonReleased(PSB_PAD_RIGHT)||ps2x.ButtonReleased(PSB_PAD_LEFT))
 {domeData.hpx = 0; domeData.dsp = 100; ET.sendData();}
 */
 
// ================= PLAY SOUNDS AND CHANGE DISPLAY ==========================================
  if(ps2x.ButtonPressed(PSB_GREEN))//triangle top
   {if(ps2x.Button(PSB_L1))
      {(playSound(8));}
    else if(ps2x.Button(PSB_L2))
      {(playSound(2));}
    else if(ps2x.Button(PSB_R1))
      {(playMusic(9));}//; domeData.dsp = 0; ET.sendData();}
     else 
      {(playSound(random(13,17)));}}
  if(ps2x.ButtonPressed(PSB_BLUE))//x bottom
   {if(ps2x.Button(PSB_L1))
      {(playSound(6));}//; domeData.dsp = 6; ET.sendData();}
    else if(ps2x.Button(PSB_L2))
      {(playSound(1));}//; domeData.dsp = 1; ET.sendData(); domeData.dsp = 0;}
    else if(ps2x.Button(PSB_R1))
      {(playMusic(11));}//; domeData.dsp = 11; ET.sendData(); domeData.dsp = 0;}
     else 
      {(playSound(random(17,25)));}}
  if(ps2x.ButtonPressed(PSB_RED))//circle right
   {if(ps2x.Button(PSB_L1))
      {(playSound(7));}
    else if(ps2x.Button(PSB_L2))
      {(playSound(3));}
    else if(ps2x.Button(PSB_R1))
      {(playMusic(10));}//; domeData.dsp = 10; ET.sendData();}
     else 
      {(playSound(random(32,52)));}}
  if(ps2x.ButtonPressed(PSB_PINK))//square left
   {if(ps2x.Button(PSB_L1))
      {(playSound(5));}//; domeData.dsp = 5; ET.sendData(); domeData.dsp = 0;}
    else if(ps2x.Button(PSB_L2))
      {(playSound(4));}//; domeData.dsp = 4; ET.sendData();}
    else if(ps2x.Button(PSB_R1))
      {(playMusic(12));}//; domeData.dsp = 0; ET.sendData();}
     else 
      {(playSound(random(25,32)));}}
      

// ========= turn hp light on & off  or change sound system  or turn jukebox on / off ======= 
  if(ps2x.ButtonPressed(PSB_L3)) //left joystick
{if(ps2x.Button(PSB_R1))
   {
     if (AUDIO1 == 2)
     {
       AUDIO1 = 1;
       AUDIO2 = 0;
       playSound(53);
     }
     else
     {
       AUDIO1 = 2;
       AUDIO2 = 2;
       playSound(52);
     }
   }
   else if(ps2x.Button(PSB_L1))
   {
     jukeBox = !jukeBox;
   }
  else
  {
    if(domeData.hpl == 1)
    {domeData.hpl = 0;}//; domeData.dsp = 100; ET.sendData();}//if hp light is on, turn it off
    else
    {domeData.hpl = 1;}//; domeData.dsp = 100; ET.sendData();}//turn hp light on
  }   
}


// ==================== JUKEBOX ==============================================================
if (jukeBox ==1)
{
  if (AUDIO2 == 2 || (AUDIO2 == 0 && AUDIO1 == 2))
{
unsigned long currentMillis = millis(); 
  if(currentMillis - jbMillis > 1000) 
  {
    jbMillis = currentMillis;   
  if(digitalRead(busyPin2)==LOW){
    playMusic(random(jukeBoxLower,jukeBoxUpper+1));
  }
}}

/*else if (AUDIO2 == 1 || (AUDIO2 == 0 && AUDIO1 == 1))
  {
  unsigned long currentMillis = millis(); 
  if(currentMillis - jbMillis > 3000) 
  {
    jbMillis = currentMillis;
  char playing = Serial.read();
 if (playing == 'x' || playing == 'X' || playing == 'E')
 {
   playing = 0;
  playMusic(random(jukeBoxLower,jukeBoxUpper+1));
}
  }
}*/
}

// ================= CHANGE DRIVESPEED ======================================================
if(ps2x.ButtonPressed(PSB_R3)) //right joystick
{
if(drivespeed == drivespeed1)//if in lowest speed
{drivespeed = drivespeed2; playSound(53);}//;domeData.dsp = 22; ET.sendData(); domeData.dsp = 0;}//change to medium speed and play sound 3-tone
else if(drivespeed == drivespeed2 && (drivespeed3!=0))//if in medium speed
{drivespeed = drivespeed3; playSound(1);}//;domeData.dsp = 23; ET.sendData(); domeData.dsp = 0;}//change to high speed and play sound scream
else////////////////////////////////////////we must be in high speed
{drivespeed = drivespeed1; playSound(52);}//;domeData.dsp = 21; ET.sendData(); domeData.dsp = 0;}//change to low speed and play sound 2-tone
} 

   
// ================== FOOT DRIVES ============================================================
/////////////////new stuff//////////////////
sticknum = (map(ps2x.Analog(PSS_RY), 0, 255, -drivespeed, drivespeed));
  
   if (drivenum < sticknum)
   {
     if (sticknum-drivenum<(ramping+1))
     drivenum+=ramping;
     else
     drivenum = sticknum;

   }
     
   else if (drivenum > sticknum)
   {
     if (drivenum-sticknum<(ramping+1))
     drivenum-=ramping;
     else
     drivenum = sticknum;

   }
 
 
 turnnum = (ps2x.Analog(PSS_RX));   
 if (turnnum <= 200 && turnnum >= 54)
  turnnum = (map(ps2x.Analog(PSS_RX), 54, 200, -(turnspeed/3), (turnspeed/3)));
 else if (turnnum > 200)
  turnnum = (map(ps2x.Analog(PSS_RX), 201, 255, turnspeed/3, turnspeed));
 else if (turnnum < 54)
  turnnum = (map(ps2x.Analog(PSS_RX), 0, 53, -turnspeed, -(turnspeed/3))); 

////////////////////////////////// 
  if (drive == 1) //right stick (drive)
{  
  ST.turn(turnnum);
  ST.drive(drivenum);
}

    
// ================== DOME DRIVE ===================================================
if (timerState[0]==0) //Dome is not turning automatically
{
domeSticknum = (map(ps2x.Analog(PSS_LX), 0, 255, -domespeed, domespeed));
if (domeSticknum > -domecompensation && domeSticknum < domecompensation)
  domeSticknum = 0;
  
if (domenum < domeSticknum)
   {
     if (domeSticknum-domenum<(domeRamping+1))
     domenum+=domeRamping;
     else
     domenum = domeSticknum;

   }
     
   else if (domenum > domeSticknum)
   {
     if (domenum-domeSticknum<(domeRamping+1))
     domenum-=domeRamping;
     else
     domenum = domeSticknum;

   }  
  
#if defined(SYRENSIMPLE)  
SyR.motor(domenum);
#else
SyR.motor(1,domenum);
#endif
}
////////////
 delay(50);
}
////////End Loop///////////////////////////////////////////

/*

MP3 / Sound Reference

1SCREAM2.mp3
2CHORTLE.mp3
3DOODOO.mp3
4WOLFWSTL.mp3
5LEIA.mp3
6SHORTCKT.mp3
7PATROL1.mp3
8ANNOYED.mp3
9Theme.mp3
10Cantina.mp3
11Emperor.mp3
12Chorus.mp3
13ALARM3.mp3
14ALARM5.mp3
15ALARM7.mp3
16ALARM8.mp3
17MISC3.mp3
18MISC7.mp3
19MISC14.mp3
20MISC16.mp3
21MISC17.mp3
22MISC25.mp3
23MISC30.mp3
24MISC34.mp3
25OOH1.mp3
26OOH2.mp3
27OOH3.mp3
28OOH4.mp3
29OOH5.mp3
30OOH6.mp3
31OOH7.mp3
32SENT1.mp3
33SENT2.mp3
34SENT3.mp3
35SENT4.mp3
36SENT5.mp3
37SENT6.mp3
38SENT7.mp3
39SENT8.mp3
40SENT9.mp3
41SENT10.mp3
42SENT11.mp3
43SENT12.mp3
44SENT13.mp3
45SENT14.mp3
46SENT15.mp3
47SENT16.mp3
48SENT17.mp3
49SENT18.mp3
50SENT19.mp3
51SENT20.mp3
52HUM19.mp3
53HUM20.mp3

*/
