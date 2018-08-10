#include "rdm630.h"

rdm630 rfid(12, 14);  //TX-pin of RDM630 connected to Arduino pin 6

void setup()
{
    Serial.begin(9600);  // start serial to PC
    rfid.begin();
}

void loop()
{
    byte data[6];
    byte length;

    if(rfid.available()){
        rfid.getData(data,length);
        Serial.println("Data valid");
        for(int i=0;i<length;i++){
            Serial.print(i);
            Serial.print("    ");
            Serial.println(data[i], HEX);

        }
        unsigned long id;
        unsigned long id2 = 10112960;
        id=data[2]*100000  + (data[3]*256) + data[4];
        Serial.println(id,DEC);
        Serial.println(id, HEX);
        if(id == id2)
        Serial.println("they are the same");
        //id+=data[3]+data
        Serial.println();
        //concatenate the bytes in the data array to one long which can be 
        //rendered as a decimal number
        unsigned long result = 
          ((unsigned long int)data[1]<<24) + 
          ((unsigned long int)data[2]<<16) + 
          ((unsigned long int)data[3]<<8) + 
          data[4];              
        Serial.print("decimal CardID: ");
        Serial.println(result);
    }
}
