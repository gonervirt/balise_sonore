#include <CC1101_ESP_Arduino.h>

//ESP32 PINs
const int SPI_SCK = 18; 			//board or mcu specific
const int SPI_MISO = 16; 			//board or mcu specific
const int SPI_MOSI = 19; 			//board or mcu specific
const int SPI_CS = 17; 				//select any pin
const int RADIO_INPUT_PIN = 7; 	    //select any pin, this is the TX-PIN
const int RADIO_OUTPUT_PIN = 4; 	//select any pin, this is the RX-PIN

CC1101 cc1101(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS, RADIO_INPUT_PIN, RADIO_OUTPUT_PIN);

volatile long last_micros;

const float ERROR_RATE = 0.3;
const float ERROR_RATE_MIN = 1 - ERROR_RATE;
const float ERROR_RATE_MAX = 1 + ERROR_RATE;
const float data_timings[46] = {625, 312.5, 312.5, 207.5, 207.5, 500, 500, 250, 250, 250, 250, 500, 500, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 500, 250, 250, 500, 250, 250, 500, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250};

const int BUFFER_LEN = 800;
int buff[BUFFER_LEN];
int vals[BUFFER_LEN];

int id = 0;

void radioHandlerOnChange() {
	int delta_micros = micros() - last_micros;
	
	bool input = digitalRead(RADIO_OUTPUT_PIN);
	if(input == 1){
    buff[id] = delta_micros;
    vals[id] = 0;
    id++;
	} else {
    buff[id] = delta_micros;
    vals[id] = 1;
    id++; 
	}
  if (id % 50==0){
    Serial.print(" .. ");
  }
	if (id >= BUFFER_LEN){
    cc1101.setIdle();
    id=0;
    Serial.println("checking...");
    detectFrame(vals, buff);
    cc1101.setRx();
  }
	last_micros = micros();
}

void detectFrame(int values[], int timings[]){
  bool detected_frame = false;
  int i = 0;
  int values_len = BUFFER_LEN;
  //int values_len = sizeof(values)/sizeof(values[0]);
  
  while (i < values_len) {
    // Check the presence of a syncword
    bool data = true;
    if (values[i] == 1){
      int j = i;
      
      for (float timing : data_timings){
        if (timings[j] >= (timing * ERROR_RATE_MIN) &&
          timings[j] <= (timing * ERROR_RATE_MAX) &&
          j < values_len){
          j += 1;
        }
        else{
          data = false;
          break;
        }
      }
      if (data) {
        detected_frame = true;
        break;
      }
    }
    i += 1;
  }
  if (detected_frame){
    Serial.println("Trame complète détectée");
  }
  else{
    Serial.println("Pas de trame détectée");
    i = 0;
  }
}

void setup(){
	Serial.begin(9600);
  delay(5000);
	
	cc1101.init();
	cc1101.setMHZ(868.3);
	cc1101.setTXPwr(TX_PLUS_10_DBM);
	cc1101.setDataRate(4000);
	cc1101.setModulation(ASK_OOK);

	cc1101.setRx();
  Serial.println("Prêt à écouter !");

  cc1101.spiWriteReg(0x1B,0x03) ; // AGCCTRL2 ; gain 
  Serial.println("AGCCTRL2 : ");Serial.println(cc1101.spiReadReg(0x1B)); // check gain

  cc1101.spiWriteReg(0x1C,0x00) ; // AGCCTRL1
  Serial.println("AGCCTRL1 : ");Serial.println(cc1101.spiReadReg(0x1C)); // check

  cc1101.spiWriteReg(0x1D,0x91) ; // AGCCTRL0
  Serial.println("AGCCTRL0 : ");Serial.println(cc1101.spiReadReg(0x1D)); // check

  Serial.println("MDMCFG4 : ");
  Serial.print(cc1101.spiReadReg(0x10));
  
  attachInterrupt(RADIO_OUTPUT_PIN, radioHandlerOnChange, CHANGE);
}

volatile long last_millis;

void loop(){
}
