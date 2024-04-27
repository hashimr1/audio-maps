#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <PN532_SWHSU.h>
#include <PN532.h>

SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

// SoftwareSerial for PN532
SoftwareSerial SWSerial(2, 3); // RX, TX on digital pins 2 and 3
PN532_SWHSU pn532swhsu(SWSerial);
PN532 nfc(pn532swhsu);


int numTags = 10;
String tagIDs[10]; // Array to store the IDs of the NFC tags
int learnedTags = 0; // Counter for learned tags

void setup()
{
  mySoftwareSerial.begin(9600);
  Serial.begin(115200);
  
  Serial.println();
  Serial.println(F("DFPlayer Mini and NFC Reader Demo"));
  Serial.println(F("Initializing DFPlayer..."));
  
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));
  myDFPlayer.volume(30);  //Set volume value. From 0 to 30

  nfc.begin();
 	uint32_t versiondata = nfc.getFirmwareVersion();
 	if (! versiondata) {
 			Serial.println(F("Didn't Find PN53x Module"));
 			while (1); // Halt
 	}

  Serial.println(F("Found chip PN5"));
  nfc.SAMConfig();
  
  Serial.println(F("Ready! Place a NFC tag near the reader."));
  playAudio(11);
}

void loop()
{
  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }

  if (learnedTags < numTags) {
    String uid = readNfcTag();
    if (uid != "") {
      tagIDs[learnedTags] = uid;
      learnedTags++;
      Serial.print("Learned Tag with ID ");
      Serial.println(uid);
      playAudio(12);
    }
    delay(3000);
  } else if (learnedTags == numTags) {
    playAudio(13);
    Serial.println("Learn phase complete. Ready for play phase.");
    learnedTags++;
    delay(3000);
  } else {
    // Play Phase
    String uid = readNfcTag();
    for (int i = 0; i < numTags; i++) {
      if (tagIDs[i] == uid) {
        playAudio(i + 1);
        break;
      }
    }
    delay(3000);
  }

}

String readNfcTag() {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 }; // Buffer to store the returned UID
  uint8_t uidLength; // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  // Check for an NFC tag
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

  if (success) {
    String uidStr = "";
    for (uint8_t i = 0; i < uidLength; i++) {
      uidStr += String(uid[i], HEX);
    }
    delay(1000); // Debounce delay
    return uidStr;
  } else {
    return ""; // No tag found
  }
}

void playAudio(int fileNumber) {
  myDFPlayer.play(fileNumber);
  Serial.print("Playing file: ");
  Serial.println(fileNumber);
  delay(500); // Delay to prevent continuous playback detection
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}
