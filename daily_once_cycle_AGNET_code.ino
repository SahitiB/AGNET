// This is the code to collect water and get the spectrophotometer reading as a part of an open-source, low-cost system for continuous nitrate monitoring in soil and open water
// By Sahiti Bulusu, Basis Independent Fremont, and Elad Levintal, Ben Gurion University
// Basic troubleshooting mechanisms of this code: the suction times per vacuum can vary if the tube length, arch of the tube, or the inner diameter of the tube changes, so be sure to calibrate the system once prior to allowing it to run automated
// More information about the low-cost spectrophotometer, it's code and building instructions can be found here --> https://www.sciencedirect.com/science/article/pii/S246806722030016X
// The current sampling frequency of this code is once a day. This can be changed in 2 ways: 
//       1. You can change the run_no variable to the amount of runs you would like, and that will cause it to go into a loop for that many runs from the second it is connected to power
//       2. You can uncomment the loops that control the device according to the RTC time, and have a set time for the device to run. This would allow you to control the exact times you want the device to run as well. This can be paired with the first option if you would like the runs to start at a certain time and continue looping for a certain number of runs ever since the first time condition was met.





#include <math.h>
#define CHANNELS    288
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include <Wire.h>;
#include <OneWire.h>

/*J26 disabling DS3231
#define DS3231_I2C_ADDR             0x68
#define DS3231_TEMPERATURE_MSB      0x11
#define DS3231_TEMPERATURE_LSB      0x12


RTC_DS3231 rtc;
*/

RTC_DS1307 rtc;

byte temp_msb;
byte temp_lsb;

boolean stop = true;

File myFile;

uint16_t average[CHANNELS];
uint16_t correction[145];

int VIDEO=A0;
int TRIGGER=2;
int CLK=9;
int START=7;


int ledPin = 18;


int liq_level = 0;


int water_sensor = 40;



//Inputs per the driver schematic
int  IN1 =  3;
int  IN2 =  4;
int  IN3 =  5;
int  IN4 =  6;
 
int  IN5 =  10;
int  IN6 =  11;
int  IN7 =  12;
int  IN8 =  13;


String reading = "";

char read=0; 
int i, i_ebc, i_turb, i_col, j, k, r, b, smoothtemp1, smoothtemp2;
int EBC, TURB, COL;
float aveabs = 0;
char numb='0';
//String reading = "";

//SoftwareSerial mySerial(10, 11); //(// RX, TX)
float spec_absor[145];
int spectrum[288];
int n_value = 3;



void GetBaseline(){ //to be multiplied with


  readSpectrometer();
  delay(100);

  //setting fixed baseline so that it doesn't change everytime disconnected from power source (average1 is found in readSpectrometer() so when calibrating the spectrometer for the first time, let it run and if average1 is different from the below baseline, replace it)
  int average1[] = {9314,9433,9401,9410,9431,9428,9440,9412,9496,9421,9391,9414,9473,9416,9339,9436,9451,9418,9407,9442,9453,9464,9440,9402,9457,9450,9472,9393,9485,9493,9443,9450,9449,9437,9465,9459,9490,9471,9454,9484,9420,9454,8695,7994,7355,6487,5396,4275,2724,1646,1612,1311,1310,1349,1262,1326,1354,1473,1595,1841,2010,2458,3398,3824,5025,5293,5698,6114,6076,6290,5978,6032,5414,5874,4824,5303,4701,4135,4559,4242,4187,3865,3479,3541,3766,3020,3147,3285,2955,3145,2853,3335,2914,3664,2735,3524,3121,3202,3213,3564,3345,3725,3578,3977,3735,3584,4263,3615,3791,4003,4285,4294,4300,4360,4818,4732,4816,5186,4981,5533,5322,5573,5568,5786,6057,6018,6160,6467,6641,6756,6652,6919,7033,7249,7231,7277,7622,7492,7787,7638,7841,7799,8044,7953,8142,8272,8387,8503,8640,9023,9206,9315,9337,9361,9339,9465,9449,9450,9440,9552,9459,9407,9473,9516,9510,9414,9410,9440,9510,9422,9418,9462,9440,9487,9393,9458,9481,9460,9401,9430,9471,9391,9441,9432,9427,9420,9447,9460,9476,9451,9419,9393,9460,9460,9430,9413,9480,9451,9489,9480,9510,9437,9380,9502,9496,9423,9477,9446,9466,9470,9464,9441,9451,9439,9415,9509,9431,9434,9470,9392,9450,9350,9461,9389,9401,9363,9374,9312,9311,9296,9390,9299,9302,9354,9346,9430,9454,9398,9423,9500,9518,9420,9400,9416,9420,9444,9516,9503,9434,9427,9475,9420,9488,9458,9452,9380,9484,9481,9463,9458,9474,9461,9484,9375,9342,9136,8944,8723,8617,8407,8279,8210,8130,8105,8119,8119,8112,8068,8107,8086,8148,8108,8091,8133,8035,7966,7918};
  for(int i = 0; i < 287; i++){
    average[i] = average1[i];
  }
  
  
  for(int i = 53; i < 198; i++){
    
  int smooth =(average[i]+average[i-1]+average[i+1])/3;
  
  correction[i-53]=smooth-980;
  if((correction[i-53]<1)||(correction[i-53]>65000)){correction[i-53]=1;}
  
  }
  
  
  
  
  //mySerial.print(" ");

}

void readSpectrometer(){
 //  uint16_t save[5];
    unsigned long accum = 0; 

   memset(average,0,sizeof(average));
  for(long j = 0; j < 10; j++){

  int delayTime = 1; // delay time

  // Start rtc cycle and set start pulse to signal start
  digitalWrite(CLK, LOW);
  delayMicroseconds(delayTime);
  digitalWrite(CLK, HIGH);
  delayMicroseconds(delayTime);
  digitalWrite(CLK, LOW);
  digitalWrite(START, HIGH);
  delayMicroseconds(delayTime);

  digitalWrite(START, HIGH);
  
  //Sample for a period of time
  for(int i = 0; i < 15; i++){

      digitalWrite(CLK, HIGH);
      delayMicroseconds(delayTime);
      delay(14);
      
    //  delay(1);
    //  delayMicroseconds(100);
      digitalWrite(CLK, LOW);
      delayMicroseconds(delayTime); 
      
  }

  //Set SPEC_ST to low
  digitalWrite(START, LOW);

  //Sample for a period of time
  for(int i = 0; i < 87; i++){

      digitalWrite(CLK, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(CLK, LOW);
      delayMicroseconds(delayTime); 
      
  }

  //One more rtc pulse before the actual read
 /* digitalWrite(CLK, HIGH);
  delayMicroseconds(delayTime);
  digitalWrite(CLK, LOW);
  delayMicroseconds(delayTime);*/



  //Read from SPEC_VIDEO

  for(int i = 0; i < CHANNELS; i++){

      accum=(average[i]*(j) + analogRead(VIDEO)*10)/(j+1);
      average[i]=accum;
      
      //average[i] += analogRead(VIDEO);
      
      digitalWrite(CLK, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(CLK, LOW);
      delayMicroseconds(delayTime);
        
  }

  //Set SPEC_ST to high


  //Sample for a small amount of time
  for(int i = 0; i < 7; i++){
    
      digitalWrite(CLK, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(CLK, LOW);
      delayMicroseconds(delayTime);
    
  }

  digitalWrite(CLK, HIGH);
  delayMicroseconds(delayTime);
  delay(100);
}
 for(int i = 0; i < CHANNELS; i++){
//  average[i] = average[i]/10;
  Serial.print(average[i]);
  Serial.print(",");}
//  Serial.print(accum);
  Serial.print("\n");

}




void resetallvaccum(){

  digitalWrite(IN1,LOW);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,LOW);
  digitalWrite(IN5,LOW);
  digitalWrite(IN6,LOW);
  digitalWrite(IN7,LOW);
  digitalWrite(IN8,LOW);


}



//turns on the first vacuum(that pulls water from soil) 
void first_vacuum_regular_turnon(int pin){
  //J26 digitalWrite(pin, LOW);
  digitalWrite(pin, HIGH);
  
}


//turns off the first vacuum(that pulls water from soil) 
void first_vacuum_regular_turnoff(){
  resetallvaccum();
  
}


void runvaccum(int pin, int runfor){

  delay(1000);
 //J26 digitalWrite(pin, LOW);
  digitalWrite(pin, HIGH);
  delay(runfor);
  resetallvaccum();
}

void runvaccumtwopins(int lowpin1, int highpin2, int runfor){
  delay(1000);
  digitalWrite(lowpin1, LOW);
  //J26 delay(1000);
  digitalWrite(highpin2,HIGH);
  delay(runfor);
  resetallvaccum();
}




//takes the spectrometer reading and saves it to the sd card file
void Capture(){ // this captures the absorption spectrum when called
  aveabs=0;
  String spacing = String(", "); //the application reads data in "nm, intensity" format, so a comma and a space is needed
  
  readSpectrometer();
  delay(100);// reads the spectrometer again 
  
  
  String tot = ""; 
  smoothtemp1;
  smoothtemp2=average[52];
  
  for(i = 53; i < 198; i++){
    smoothtemp1 = smoothtemp2;  
    smoothtemp2 = average[i];
    average[i]=(average[i]+smoothtemp1+average[i+1])/3;
    
    average[i]=average[i]-980; // removes background. background values vary a bit for every measurement, 96 is an average of about 10 measurements
    if((average[i]<1)||(average[i]>65000)){average[i]=1;}// corr=6500;} // this makes sure that there is no overflow or division by zero (usually happens when spectra outside of the LED spectrum is registered)
    
    float m = correction[i-53]/(float)average[i];
    aveabs += log10(m);
  
  }
  
  
  aveabs=aveabs/(198-53);
  
  for(i = 53; i < 198; i++){//reads from position 53 to position 198, which corresponds to approximately 450-750nm
  uint16_t nm[145];
  int corr;
  //nm[i-53] = (3.103932661*pow(10,2)+2.683934106*i-1.098262279*pow(10,-3)*pow(i,2)-7.817392551*pow(10,-6)*pow(i,3)+9.609636190*pow(10,-9)*pow(i,4)+4.681760466*pow(10,-12)*pow(i,5))*10;
  nm[i-53] = (3.038924085*pow(10,2)+2.715277913*i-1.245962432*pow(10,-3)*pow(i,2)-7.391530247*pow(10,-6)*pow(i,3)+9.130177259*pow(10,-9)*pow(i,4)+4.718390529*pow(10,-12)*pow(i,5))*10;
  //nm[i] calculates the corresponding wavelength to each pixel - the parameters are included in the C12880MA manual and are unique, so the above line needs to be corrected accordingly
  /*in order to just register and check the spectrum of the light source
  comment out lines starting from the "corr=correction[i]" to "float n = log10(m)"
  line (included) and substitute "String(n,4)" in the "String val =" with "data[i]"*/
  
  //corr=correction[i]; // this copies the baseline data, so it could be modified without modifying the actual baseline data
  //average[i]=average[i]-980; // removes background. background values vary a bit for every measurement, 96 is an average of about 10 measurements
  //if((average[i]<1)||(average[i]>65000)){average[i]=1;}// corr=6500;} // this makes sure that there is no overflow or division by zero (usually happens when spectra outside of the LED spectrum is registered)
  
  float m = correction[i-53]/(float)average[i]; //calculates the baseline LED spectrum divided by the spectrum with an inserted sample
  float n = log10(m)+0.0556-0.0767*aveabs; // calculates absorbance
  
  float integ = nm[i-53]/10.0; //this is due to the multiplication by 10 at the end of nm[i] calculation. This is used to avoid a float massive, which takes up more space than an int massive - this means that we get for example a calculated position of 550.12345*10 =5501.2345 (which is 5501 as an int) and then we divide it back as a float and get the value of 550.1 nm
  String temp = String(integ,1); //turns it into a sting with 1 decimal place
  String val = String(n,4); //turns into a string with 4 decimal places
  
  
         tot += temp; //adds the resulting nm to a string
         tot += ", "; //adds a comma
         tot += val; //adds the resulting absorbance to a string
         tot+="\n"; // adds a new line
         if (myFile) {
          //myFile.println(run_no);
          myFile.println(tot);
         }
         //spec_absor[i-53]=val;
         //spec_absor[i-53][0] = temp;
         //spec_absor[i-53][1] = val;
       //  Serial.print(tot);
         //mySerial.print(tot); //prints the "nm, absorbance" string
         tot = ""; //erases the contents of the "nm, absorbance" string
  
           }
      //     mySerial.print(tot);
      //     Serial.print(tot);
      //     tot = "";
  delay(200);
  /*for(int i = 2; i < CHANNELS; i++){
    float t = correction[i]/(float)average[i];
   // average[i] = average[i]/10;
    Serial.print(t);
    Serial.print(",\n");}*/


}


  
  

//turns on led for spectrometer
void CalibrateLED(){
j=0;

/*for(int f = 2; f < 200; f++){ //LED brightness
analogWrite(ledPin, f);

for(int i = 0; i < CHANNELS; i++){
  if(data[i]>950){j=f-1; break;}}
if(j>0){break;}
}*/

  // J26 analogWrite(ledPin, 2);
  //analogWrite(5,2);
  digitalWrite(ledPin,HIGH);
  delay(100);
  //mySerial.print(" ");
  delay(100);



}
//correction = baseline


void specificWavelength(){

  aveabs=0;
  int singlenm, singlei;
  readSpectrometer();
  delay(100);
  
  
  int wlnumber=reading.toInt();
  
  for(i = 53; i < 198; i++){
  //singlenm=(3.103932661*pow(10,2)+2.683934106*i-1.098262279*pow(10,-3)*pow(i,2)-7.817392551*pow(10,-6)*pow(i,3)+9.609636190*pow(10,-9)*pow(i,4)+4.681760466*pow(10,-12)*pow(i,5));
  singlenm = (3.103932661*pow(10,2)+2.683934106*i-1.098262279*pow(10,-3)*pow(i,2)-7.817392551*pow(10,-6)*pow(i,3)+9.609636190*pow(10,-9)*pow(i,4)+4.681760466*pow(10,-12)*pow(i,5))*10;
  if(singlenm>wlnumber){singlei=i; break;}}
  
  smoothtemp2=average[52];
  for(i = 53; i < 198; i++){
    smoothtemp1 = smoothtemp2;  
    smoothtemp2 = average[i];
    average[i]=(average[i]+smoothtemp1+average[i+1])/3;
    
    average[i]=average[i]-980; // removes background. background values vary a bit for every measurement, 96 is an average of about 10 measurements
    if((average[i]<1)||(average[i]>65000)){average[i]=1;}// corr=6500;} // this makes sure that there is no overflow or division by zero (usually happens when spectra outside of the LED spectrum is registered)
    
    float m = correction[i-53]/(float)average[i];
    aveabs += log10(m);
  }
  aveabs=aveabs/(198-53);
  
  float m = correction[singlei-53]/(float)average[singlei]; //calculates the baseline LED spectrum divided by the spectrum with an inserted sample
  m = log10(m)+0.0556-0.0767*aveabs;
  
  m =m*100;
  m=m/100;
  //mySerial.print(m);
}


void setup() {

  pinMode(water_sensor, INPUT);
  
  pinMode(CLK, OUTPUT);
  pinMode(START, OUTPUT);
  //J26 -- NOT SURE WHAT IS A0
  pinMode(A0, INPUT);
  pinMode(ledPin, INPUT);


  
  
  digitalWrite(CLK, HIGH);
  digitalWrite(START, LOW);
  rtc.begin();
  

   // analogWrite(ledPin, 6);
  digitalWrite(ledPin, HIGH);

  
  // Relay code
 resetallvaccum();


  pinMode(IN1, OUTPUT);   
  pinMode(IN2 , OUTPUT);   
  pinMode(IN3, OUTPUT);   
  pinMode(IN4 , OUTPUT);
  pinMode(IN5, OUTPUT);   
  pinMode(IN6 , OUTPUT);   
  pinMode(IN7, OUTPUT);   
  pinMode(IN8 , OUTPUT); 
  


  Wire.begin();
  //sensors.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  
  if (!SD.begin(53)) {
  Serial.println("initialization failed!");
  }
  Serial.println("initialization done.");


  
} 

void loop() {
  digitalWrite(ledPin, LOW);
  DateTime time1 = rtc.now();
    String date = String(time1.month()) + String(time1.day()) +"n.txt";
  if(SD.exists(date)){
      myFile = SD.open("x.txt", FILE_WRITE);
      myFile.println("came inside sd check and so stopped the loop");
      myFile.close();
      stop = false;
        }
// terminates the loop after the nth run
stop=true;
  if(stop){
    
    int run_no = 0;
    

    Serial.println("in loop");   
    
    
    
    

    
    // starts system loop at chosen time everyday (if following option 2 of controlling the sampling frequency, this is the loop to be uncommented)
    //if( time.hour()==10 && time.minute()==30){
    
      
      
      //the run number increments every time the loop starts
      run_no = run_no + 1;

      //if it is the nth(2nd) run (2nd days run), don't discard the sample back into the soil (can change n according to what you would prefer)
      
        myFile = SD.open("y.txt", FILE_WRITE);
        
    
        // if the file opened okay, write to it:
        if (myFile) {
          Serial.print("Writing to txt...");
          
          myFile.println(String(time1.month())+"/" + String(time1.day()) + "/" + String(time1.year())+ " " + String(time1.hour()) + ":" + String(time1.minute()) + ":" + String(time1.second()));
          //J26 myFile.println("Temperature: " + String(temp));
          
        }


  
        GetBaseline();
        
        for(i=0; i<288; i++){
          spectrum[i] = average[i];
        }
        
//        uses water sensor to know when to stop pulling water from the soil
        liq_level=digitalRead(water_sensor);
        while(liq_level ==0){
        first_vacuum_regular_turnon(IN5);
        delay(1000);
        liq_level=digitalRead(water_sensor);
        }
        first_vacuum_regular_turnoff();
        myFile.println("first vacuum pulled all the water and is now off");
        time1 = rtc.now();
        myFile.println(String(time1.hour()) + ":" + String(time1.minute()) + ":" + String(time1.second()));
        Serial.println("first vacuum pulled all the water and is now off" );
        delay(5000);

        myFile.flush();
      
        //second vacuum regular
        runvaccumtwopins(IN2, IN1,5000);
        myFile.println("second vacuum deposited the sample into the other test tube");
        time1 = rtc.now();
         myFile.println(String(time1.hour()) + ":" + String(time1.minute()) + ":" + String(time1.second()));
        Serial.println("second vacuum deposited the sample into the other test tube");
        myFile.flush();
        delay(5000);
        //second vacuum reverse
        runvaccumtwopins(IN1, IN2,7000);
        myFile.println("second vacuum reversed and flushed its tube");
        myFile.flush();
        time1 = rtc.now();
         myFile.println(String(time1.hour()) + ":" + String(time1.minute()) + ":" + String(time1.second()));
        Serial.println("second vacuum reversed and flushed its tube");
        delay(5000);
        
        //pulls the reagent required for the sample, fourth vaccum
        runvaccumtwopins(IN4, IN3,4000);
        myFile.println("reagent vacuum ran and deposited the reagent");
        //myFile.flush();
        time1 = rtc.now();
        myFile.println(String(time1.hour()) + ":" + String(time1.minute()) + ":" + String(time1.second()));
        Serial.println("reagent vacuum ran and deposited the reagent");
        delay(5000);
        //clears the reagent vacuums tubes using reverse flush
        runvaccumtwopins(IN3, IN4,10000);
        myFile.println("reverse flushed the reagent vacuum and emptied out that tube");
        myFile.flush();
        time1 = rtc.now();
         myFile.println(String(time1.hour()) + ":" + String(time1.minute()) + ":" + String(time1.second()));
        Serial.println("reverse flushed the reagent vacuum and emptied out that tube");
        delay(5000);
        //puts it all into the cuvette
        //J26 runvaccum(IN6,29000);
        runvaccum(IN6,9000);
        myFile.println("third vacuum ran and put all the solution into the cuvette");
        myFile.flush();
        time1 = rtc.now();
         myFile.println(String(time1.hour()) + ":" + String(time1.minute()) + ":" + String(time1.second()));
        Serial.println("third vacuum ran and put all the solution into the cuvette");
        delay(10000);
    
        
        
        
        //4 hrs delay now
        CalibrateLED();
        // if temperature curve comes, put in the correlation metrics according to measured temperature
        delay(28800000);

        //takes the spectrometer reading and saves it in the sd card file
        Capture();
        myFile.flush();
        //clears the cuvette
        
       //J26  runvaccum(IN7,25000);
       runvaccum(IN7,15000);
        myFile.println("rinse vacuum ran and emptied out the cuvette");
        time1 = rtc.now();
        myFile.println(String(time1.hour()) + ":" + String(time1.minute()) + ":" + String(time1.second()));
        Serial.println("rinse vacuum ran and emptied out the cuvette");
        myFile.flush();
        delay(5000);

        //checks nth value
        if(run_no!=n_value){
        //puts it back into the soil
          runvaccum(IN8,60000);
          myFile.println("last vacuum ran emptying out the entire test tube into the soil");
          myFile.flush();
          time1 = rtc.now();
           myFile.println(String(time1.hour()) + ":" + String(time1.minute()) + ":" + String(time1.second()));
          Serial.println("last vacuum ran emptying out the entire test tube into the soil");
          //pick how long you want the delay to be
          delay(18800000);
          myFile.println("new run begins now");
         }
         else{
        stop = false;
        myFile.close();
         }
  
 //}
 
}
}
