#include <FileIO.h>

unsigned long time;
unsigned long wait3start;
unsigned long puffstart;
unsigned long blockedstart;
unsigned long wait2start;
unsigned long pufftime;
unsigned long wait3time;
unsigned long pumptime;
unsigned long wait2time;
unsigned long waterprob;
unsigned long touch;
unsigned long lastwrite;
unsigned long npump;
unsigned long wait1start;
unsigned long wait1time;
unsigned long loopdelay;
char printdata[45][23];
unsigned long datacounter;
#define relayPinPump 8
#define relayPinPuff 7
#define relayPinFake 2
#define relayPinFakeTwo 10

#define SENSORPIN 4
#define LICKPIN 11

int temp;
//const int savePin = 5;
//String filename;
String filenameWeb;
String cleaneddatetime;
boolean recording;
String dataString;
double runtime;
boolean real;

//-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-------------------------------------------------------------------------------------
void setup() {


  npump = 0; //set number of pumps at 0
  wait3start = 0; //set pausestart at 0
  puffstart = 0; //set puffstart at 0
  blockedstart = 0; //set blockedstart at 0
  wait2start = 0; //set firstpausestart at 0
  wait1start = 0; //set wait1start at 0
  wait1time = 0; //set wait1time at 0
  loopdelay = 100; //delay for arduino so its not too fast
  pufftime = 500; //pufftime lasts for x milliseconds
  wait2time = 500; //firstpausetime lasts for x milliseconds
  wait3time = 925; //pausetime lasts for x milliseconds
  pumptime = 75; //pumptime lasts for x milliseconds
  waterprob = 80; //probablity of watering (a percent)
  datacounter = 0;
  // initialize the sensor pin as an input:
  pinMode(SENSORPIN, INPUT);
  digitalWrite(SENSORPIN, HIGH); // turn on the pullup
  pinMode(relayPinPump, OUTPUT);
  pinMode(relayPinPuff, OUTPUT);
  pinMode(relayPinFake, OUTPUT);
  pinMode(relayPinFakeTwo, OUTPUT);
  pinMode(LICKPIN, INPUT);

  Bridge.begin();
  FileSystem.begin();
  FileSystem.remove("/mnt/sda1/arduino/www/progressfile.txt");
  File dataFile = FileSystem.open("/mnt/sda1/arduino/www/progressfile.txt", FILE_APPEND); //open the file
  if (dataFile) { //if the file is available, write to it
    dataFile.println("Starting");
    dataFile.close();
  }

  randomSeed(analogRead(A0)); //randomizes number for delay

  while (digitalRead(LICKPIN)) { //if the lick pin is on when the arduino is on then delay so voltage can stabilize
    temp = Printprogressfile("initializing");
    delay(1000);
  }

  //pinMode(savePin, INPUT); //switch is an input
  //recording = false; //no recording

  cleaneddatetime += getTimeStamp(); //the cleaneddatetime is the date and time
  cleaneddatetime.replace("/", "_"); //replace with an underscore
  cleaneddatetime.replace("-", "~T~"); //replace with a T
  cleaneddatetime.replace(":", "_"); //replace with an underscore
  filenameWeb += "/mnt/sda1/arduino/www/temptest.txt" + cleaneddatetime += ".txt";

}

//------------------------------------------------------------------------------

int Printprogressfile(String progressfiledata) {
  FileSystem.remove("/mnt/sda1/arduino/www/progressfile.txt");
  File dataFile = FileSystem.open("/mnt/sda1/arduino/www/progressfile.txt", FILE_APPEND); //open the file
  if (dataFile) { //if the file is available, write to it
    /*    dataFile.println("Puff Time in ms");
        dataFile.println(String(pufftime));
        dataFile.println("First Pause Time in ms");
        dataFile.println(String(wait2time));
        dataFile.println("Pause Time in ms");
        dataFile.println(String(wait3time));
        dataFile.println("Pump Time in ms");
        dataFile.println(String(pumptime));
        dataFile.println("Water Probability in %");
        dataFile.println(String(waterprob));
    */    dataFile.println(progressfiledata);
    dataFile.close();
  }
  return 0;

}



//------------------------------------------------------------------------------

String getTimeStamp() {
  String result;
  Process time;
  // date is a command line utility to get the date and the time
  // in different formats depending on the additional parameter
  time.begin("date");
  time.addParameter("+%D-%T");  // parameters: D for the complete date mm/dd/yy
  //             T for the time hh:mm:ss
  time.run();  // run the command

  // read the output of the command
  while (time.available() > 0) {
    char c = time.read();
    if (c != '\n') {
      result += c;
    }
  }
  return result;
}


//-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP--------------------------------------------------------------------------------------
void loop() {
  dataString = ""; //helps to only print the date once

  if  (!digitalRead(SENSORPIN)) { //if sensor is blocked
    if ((puffstart == 0) && (wait3start == 0) &&  (wait2start == 0) && (wait1start == 0)) { //if nothing is on
      blockedstart = millis(); //start blocked start
    }
    else {
      blockedstart = 0;
    }
  }

  //-BLOCKED--------------------------------------------------------------------------------------

  if (blockedstart > 0) { //what happens if sensor is blocked
    blockedstart = 0;
    if (random(100) < waterprob) { //randomizes when the pump/puff turns on
      real = true; //goes into real phase
    }
    else {
      real = false; //goes into fake phase
    }
    wait1time = random(200, 801); //pick a random number between 200 and 800
    wait1start = millis(); //start the first wait time
  }

  //-WAIT1START--------------------------------------------------------------------------------------

  if (wait1start > 0) { //if the first wait time has started
    if ((wait1time) < (millis() - wait1start)) { //if the first wait time is over
      wait1start = 0; //set wait1start to 0
      wait1time = 0; //set wait1start to 0
      puffstart = millis(); //start the puff stage
      if (real) { //if actually puffing
        digitalWrite(relayPinPuff, HIGH); //turn puff on
      }
      else { //if not puffing
        digitalWrite(relayPinFake, HIGH); //turn fake puff on
      }
    }
  }

  //-PUFFSTART--------------------------------------------------------------------------------------

  if (puffstart > 0) { //if the puff stage has started
    if ((pufftime) < (millis() - puffstart)) { //if the puff stage is over
      puffstart = 0; //puffstart goes back to 0
      if (real) {
        digitalWrite(relayPinPuff, LOW); //turn puff off
      }
      else {
        digitalWrite(relayPinFake, LOW); //turn fake puff off
      }
      wait2start = millis(); //start second wait time
    }
  }

  //-WAIT2START--------------------------------------------------------------------------------------

  if (wait2start > 0) { //if the second wait time has started
    if ((wait2time) < (millis() - wait2start)) { //if the second wait time is over
      wait2start = 0; //set wait2start back to 0
      if (real) { //if actually pumping
        npump = npump + 1; //adding one to the total number of times pumped
        digitalWrite(relayPinPump, HIGH); //turn pump on
        delay(pumptime); //length of a pump
        digitalWrite(relayPinPump, LOW); //turn pump low
      }
      else { //if not actually pumping
        digitalWrite(relayPinFakeTwo, HIGH); //turn fake pump on
        delay(pumptime); //length of a pump
        digitalWrite(relayPinFakeTwo, LOW); //turn fake pump off
      }
      wait3start = millis(); //starts third wait time
    }
  }

  //-WAIT3START--------------------------------------------------------------------------------------
  if (wait3start > 0) { //if the third wait time has started
    if (((wait3time - 200) < (millis() - wait3start)) && (digitalRead(SENSORPIN))) { //indicator that the third wait time ended
      wait3start = 0; //wait3start goes back to 0
      File dataFileWeb = FileSystem.open(filenameWeb.c_str(), FILE_APPEND); //open the file
      if (dataFileWeb) { //if the file is available, write to it
        for (int x = 0; x < datacounter; x++) {
          String tempString = String(printdata[x]);
          dataFileWeb.println(tempString);
          for (int y = 0; y < 23; y++) {
            printdata[x][y] = (char)0;
          }
        }
          String tempString2 = String(runtime);
          runtime = float(millis()) / 1000; //calculate seconds
          dataFileWeb.println(tempString2 + ",0,0,7,0");

        dataFileWeb.close();
        datacounter = 0;
      }

    }
  }




  //}


  //-DATA-STRING-WRITING--------------------------------------------------------------------------------------

  if (!(blockedstart == 0) || !(puffstart == 0) || !(wait1start == 0) || !(wait2start == 0) || !(wait3start == 0)) { //if anything is triggered
    // dataString += getTimeStamp(); //start the date
    // dataString += ","; //separate with a comma
    runtime = float(millis()) / 1000; //calculate seconds
    dataString += runtime; //start the number of seconds
    dataString += ","; //separate with a comma
    if (!digitalRead(SENSORPIN)) { //if the sensor is blocked
      dataString += "1"; //write a 1
    }
    else {
      dataString += "0"; //otherwise write a 0
    }
    dataString += ","; //separate with a comma
    if (digitalRead(LICKPIN) != 0) { //if licked
      dataString += "2"; //write a 2
    }
    else {
      dataString += "0"; //otherwise write a 0
    }
    dataString += ","; //separate with a comma


    if (real) {
      if (wait1start > 0) { //if wait1start has started
        dataString += "3"; //write a 3
      }
      if (puffstart > 0) { //if puffstart has started
        dataString += "4"; //write a 4
      }
      if (wait2start > 0) { //if wait2start has started
        dataString += "5"; //write a 5
      }
      if (wait3start > 0) { //if wait3start has started
        dataString += "7"; //write a 7
      }
    }
    else {
      dataString += "9"; //9 is fake run
    }

    dataString += ","; //separate with a comma
    dataString += wait1time;



    char charBuf[23];
    dataString.toCharArray(charBuf, 23);
    strcpy(printdata[datacounter], charBuf);
    datacounter = datacounter + 1;
    if (datacounter == 40) {
      datadump();
    }

    delay(loopdelay);
  }



}

//- END LOOP-ENDLOOP-END LOOP-END LOOP-END LOOP-END LOOP-END LOOP-END LOOP------------------------------------------------------------
//------------------------------------------------------------------------------

void datadump() {

  File dataFileWeb = FileSystem.open(filenameWeb.c_str(), FILE_APPEND); //open the file
  if (dataFileWeb) { //if the file is available, write to it
    for (int x = 0; x < datacounter; x++) {
      String tempString = String(printdata[x]);
      dataFileWeb.println(tempString);
      for (int y = 0; y < 23; y++) {
        printdata[x][y] = (char)0;
      }
    }
    dataFileWeb.close();
    datacounter = 0;
  }
}
