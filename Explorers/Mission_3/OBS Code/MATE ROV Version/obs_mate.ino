
//#include <SoftwareSerial.h>


// Necessary for LSM303
// 
#include <Wire.h>
#include <Math.h>

    float Heading;
    float Pitch;
    float Roll;
    float Xangle;
    float Yangle;
    float Voltage;
    int VoltInput;
    

    float Accx;
    float Accy;
    float Accz;

    float Magx;
    float Magy;
    float Magz;
    float Mag_minx;
    float Mag_miny;
    float Mag_minz;
    float Mag_maxx;
    float Mag_maxy;
    float Mag_maxz;
    int led2 = 8;
    
/*************************************************************************************************************************
 * Tilt and roll calculation
 *
 ************************************************************************
 Send register address and the byte value you want to write the accelerometer and 
loads the destination register with the value you send
*/
void WriteAccRegister(byte data, byte regaddress)
{
    Wire.beginTransmission(0x19);   // Use accelerometer address for regs >=0x20
    Wire.write(regaddress);
    Wire.write(data);  
    Wire.endTransmission();     
}

/*  
Send register address to this function and it returns byte value
for the accelerometer register's contents 
*/
byte ReadAccRegister(byte regaddress)
{
    byte data;
    Wire.beginTransmission(0x19);   // Use accelerometer address for regs >=0x20  
    Wire.write(regaddress);
    Wire.endTransmission();

    delayMicroseconds(100);

    Wire.requestFrom(0x19,1);   // Use accelerometer address for regs >=0x20
    data = Wire.read();
    Wire.endTransmission();   

    delayMicroseconds(100);

    return data;  
}  

/*  
Send register address and the byte value you want to write the magnetometer and 
loads the destination register with the value you send
*/
void WriteMagRegister(byte data, byte regaddress)
{
    Wire.beginTransmission(0x1E);   // Else use magnetometer address
    Wire.write(regaddress);
    Wire.write(data);  
    Wire.endTransmission();     

    delayMicroseconds(100);
}

/*  
Send register address to this function and it returns byte value
for the magnetometer register's contents 
*/
byte ReadMagRegister(byte regaddress)
{
    byte data;
    Wire.beginTransmission(0x1E);   // Else use magnetometer address  
    Wire.write(regaddress);
    Wire.endTransmission();

    delayMicroseconds(100);

    Wire.requestFrom(0x1E,1);   // Else use magnetometer address
    data = Wire.read();
    Wire.endTransmission();   

    delayMicroseconds(100);

    return data;  
}  

void init_Compass(void)
{
    WriteAccRegister(0x67,0x20);  // Enable accelerometer, 200Hz data output

    WriteMagRegister(0x9c,0x00);  // Enable temperature sensor, 220Hz data output
    WriteMagRegister(0x20,0x01);  // set gain to +/-1.3Gauss
    WriteMagRegister(0x00,0x02);  // Enable magnetometer constant conversions
}

/*
Readsthe X,Y,Z axis values from the accelerometer and sends the values to the 
serial monitor.
*/
void get_Accelerometer(void)
{

  // accelerometer values
  byte xh = ReadAccRegister(0x29);
  byte xl = ReadAccRegister(0x28);
  byte yh = ReadAccRegister(0x2B);
  byte yl = ReadAccRegister(0x2A);
  byte zh = ReadAccRegister(0x2D);
  byte zl = ReadAccRegister(0x2C);

  // need to convert the register contents into a righ-justified 16 bit value
  Accx = (xh<<8|xl); 
  Accy = (yh<<8|yl); 
  Accz = (zh<<8|zl); 

}  

/*
Reads the X,Y,Z axis values from the magnetometer sends the values to the 
serial monitor.
*/
void get_Magnetometer(void)
{  
  // magnetometer values
  byte xh = ReadMagRegister(0x03);
  byte xl = ReadMagRegister(0x04);
  byte yh = ReadMagRegister(0x07);
  byte yl = ReadMagRegister(0x08);
  byte zh = ReadMagRegister(0x05);
  byte zl = ReadMagRegister(0x06);

  // convert registers to ints
  Magx = (xh<<8|xl); 
  Magy = (yh<<8|yl); 
  Magz = (zh<<8|zl); 
}  

/*
Converts values to a tilt compensated heading in degrees (0 to 360)
*/
void get_TiltHeading(void)
{
  // You can use BM004_Arduino_calibrate to measure max/min magnetometer values and plug them in here.  The values
  // below are for a specific sensor and will not match yours
//   compass.m_min = (LSM303::vector<int16_t>){-497, -606, -745};
//  compass.m_max = (LSM303::vector<int16_t>){+661, +540, +342};
  Mag_minx = -497;
  Mag_miny = -606;
  Mag_minz = -745;
  Mag_maxx = 661;
  Mag_maxy = 540;
  Mag_maxz = 342;

  // use calibration values to shift and scale magnetometer measurements
  Magx = (Magx-Mag_minx)/(Mag_maxx-Mag_minx)*2-1;  
  Magy = (Magy-Mag_miny)/(Mag_maxy-Mag_miny)*2-1;  
  Magz = (Magz-Mag_minz)/(Mag_maxz-Mag_minz)*2-1;  

  // Normalize acceleration measurements so they range from 0 to 1
  float accxnorm = Accx/sqrt(Accx*Accx+Accy*Accy+Accz*Accz);
  float accynorm = Accy/sqrt(Accx*Accx+Accy*Accy+Accz*Accz);

  // calculate pitch and roll
  Pitch = asin(-accxnorm);
  Roll = asin(accynorm/cos(Pitch));

  // tilt compensated magnetic sensor measurements
  float magxcomp = Magx*cos(Pitch)+Magz*sin(Pitch);
  float magycomp = Magx*sin(Roll)*sin(Pitch)+Magy*cos(Roll)-Magz*sin(Roll)*cos(Pitch);

  // arctangent of y/x converted to degrees
  Heading = 180*atan2(magycomp,magxcomp)/PI;

//  if (Heading < 0)
//      Heading +=360;

//    Serial.print("Heading=");
//    Serial.print(Heading);   
    Xangle = (Xangle +((-Pitch*100.0)+3.5))/2.0;  // add or subtract offset for compass as sealed
    Yangle = (Yangle +(Roll*100.0)+4.5)/2.0;
 //   Serial.print("  X=");
//    Serial.print(Xangle);
//    Serial.print("  Y=");
//    Serial.print(Yangle);
     
  }  
    

/*************************************************************************************************************
  Arduino Webserver using ESP8266
  
 *************************************************************************************************************/
 

/*
  Arduino Webserver using ESP8266
  Displays temperature in a webpage

  Arduino Mega has three Serial communication ports,this code works well with 
  Arduino Mega.For UNO users,use Softserial library to connect ESP8266 with
  Arduino UNO 
  If you're unsure about your Arduino model or its pin cofigurations,please check
  the documentation at http://www.arduino.cc
 
  modified August 2016
  By Joyson Chinta and Gladson Kujur
 */
// SoftwareSerial Serial1(5,6); // RX, TX
#define DEBUG true
 
void setup()
{
     Wire.begin();
    init_Compass();
  Serial.begin(9600);    ///////For Serial monitor 
 
  Serial1.begin(115200); ///////ESP Baud rate
  pinMode(8,OUTPUT);    /////used if connecting a LED to pin 11
  digitalWrite(8,LOW);
 
  sendData("AT+RST\r\n",2000,DEBUG); // reset module
  sendData("AT+CWMODE=2\r\n",1000,DEBUG); // configure as access point
  sendData("AT+CIFSR\r\n",1000,DEBUG); // get ip address
  sendData("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
  sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // turn on server on port 80
}


float sensetemp() ///////function to sense temperature.
 {
  int val = analogRead(A0);
  float mv = ( val/1024.0)*5000; 
  float celcius = mv/10;
  return(celcius);
 }
 
int LoopCount = 0;
int connectionId;
void loop()
{
      // Get the Compass data at all times
    get_Accelerometer();
    get_Magnetometer();
    get_TiltHeading();
    // Get the Voltage input from A1 for the Inductive Coupling
    VoltInput = analogRead(A1);           // read the input
    Voltage = 5.0 *( VoltInput / 1024.0); // scale 0.0 to 5.0 volts
    if (Voltage > 3.0) {
            digitalWrite(8,LOW);   
    }
    else {
            digitalWrite(8,HIGH);   
    }
    Serial.print (" Volts= ");
    Serial.print(Voltage);
    Serial.print("  X=");
    Serial.print(Xangle);
    Serial.print("  Y=");
    Serial.println(Yangle);
    
  if(Serial1.available())
  {
    /////////////////////Recieving from web browser to toggle led
    if(Serial1.find("+IPD,"))
    {
     delay(300);
     connectionId = Serial1.read()-48;
     if(Serial1.find("pin="))
     { 
     Serial.println("recieving data from web browser");
     int pinNumber = (Serial1.read()-48)*10; 
     pinNumber += (Serial1.read()-48); 
     digitalWrite(pinNumber, !digitalRead(pinNumber));
     }
   
    /////////////////////Sending data to browser
    else
    {
      String webpage = "<h1>MATE 2018 SEISMOGRAPH STATUS</h1>";
      espsend(webpage);
     }
     
     String add1="Voltage=";
     String two =  String(Voltage, 3);
     add1+= two;
    
     if(Voltage > 4.0)
     {
      Serial.println("Connected to WiFi");
      two = " Xangle=";
      add1+= two;
      two =  String(Xangle, 3);
      add1+= two;
      two = " Yangle=";
      add1+= two;
      two =  String(Yangle, 3);
      add1+= two;
      two = " Count=";
      add1+= two;
      two =  String(LoopCount, 3);
      add1+= two;
      espsend(add1);
      LoopCount +=1;
      if (abs(Xangle) < 5.0) {
        if (abs(Yangle) < 5.0) {
              add1 = "</p>DATA: 0, 4.4, -4.3, 0, 0, 6.8, -7, 0, 0, 1.6, -1.6, 0, 0, 6.3, -6.4, 0";
              espsend(add1);
              Serial.println("DATA: 0, 4.4, -4.3, 0, 0, 6.8, -7, 0, 0, 1.6, -1.6, 0, 0, 6.3, -6.4, 0");
  
        }
      }
     }
     else {
      espsend(add1);  

     }
     String closeCommand = "AT+CIPCLOSE=";  ////////////////close the socket connection////esp command 
     closeCommand+=connectionId; // append connection id
     closeCommand+="\r\n";
     sendData(closeCommand,3000,DEBUG);
    }
  }
}
 
  //////////////////////////////sends data from ESP to webpage///////////////////////////
 
 void espsend(String d)
         {
             String cipSend = " AT+CIPSEND=";
             cipSend += connectionId; 
             cipSend += ",";
             cipSend +=d.length();
             cipSend +="\r\n";
             sendData(cipSend,1000,DEBUG);
             sendData(d,1000,DEBUG); 
         }

//////////////gets the data from esp and displays in serial monitor///////////////////////         
String sendData(String command, const int timeout, boolean debug)
            {
                String response = "";
                Serial1.print(command);
                long int time = millis();
                while( (time+timeout) > millis())
                {
                   while(Serial1.available())
                      {
                         char c = Serial1.read(); // read the next character.
                         response+=c;
                      }  
                }
                
                if(debug)
                     {
                     Serial.print(response); //displays the esp response messages in arduino Serial monitor
                     }
                return response;
            }
