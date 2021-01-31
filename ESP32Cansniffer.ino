
#include <ESP32CAN.h>
#include <CAN_config.h>
#include <Wire.h>
 
#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;

RTC_DATA_ATTR int bootCount = 0;
unsigned long timout_CAN = 200000;

bool control = 1;

word filter_id = 0xFFFF;

CAN_device_t CAN_cfg; 
const int rx_queue_size = 10;   
unsigned long period = 0;   

void setup() {
Serial.begin(230400),Serial.println(),   ++bootCount; Serial.println("Проснулся " + String(bootCount));
//CAN_cfg.speed = CAN_SPEED_125KBPS; // скорость 125 
CAN_cfg.speed = CAN_SPEED_500KBPS;   // скорость 500
CAN_cfg.tx_pin_id = GPIO_NUM_33;
CAN_cfg.rx_pin_id = GPIO_NUM_27;

CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
ESP32Can.CANInit();
SerialBT.begin("ESP32 CAN Stiffer2"); SerialBT.println("Hello" + String(bootCount));
}

void loop() {
if (Serial.available())   resp_serial(); 
if (SerialBT.available()) resp_SerialBT(); 

CAN_read ();
                       
if (millis() > timout_CAN + 200000) Serial.println("Засыпаю...."), esp_sleep_enable_ext0_wakeup(GPIO_NUM_27,0), esp_deep_sleep_start();
}




void CAN_read () { CAN_frame_t rx_frame;
 if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) {
 timout_CAN = millis();
 

     
 
 if (rx_frame.MsgID == filter_id || filter_id == 0xFFFF)  {
                                                           printf("%03X: ", rx_frame.MsgID); 
                                                           SerialBT.print("ID:"), SerialBT.print(rx_frame.MsgID, HEX), SerialBT.print(" DATA: ");
                                                          for (int i = 0; i < rx_frame.FIR.B.DLC; i++) {
                                                                                                        printf("%02X ", rx_frame.data.u8[i],HEX);
                                                                                                        SerialBT.print( rx_frame.data.u8[i],HEX);
                                                                                                        SerialBT.print(" ");
                                                                                                        }  
                                                              
                                                           printf("\n");
                                                           SerialBT.println("");                                                              
                                                           }}
                 }


 
void resp_serial (){ String at = ""; int k = 0;
   while (Serial.available()) k = Serial.read(), at += char(k),delay(10); 
   filter_id = strtol(at.c_str(),NULL,16); Serial.print("Filter ID: "), Serial.println(filter_id, HEX); at = "";  }

void resp_SerialBT (){ String at = ""; int k = 0;
   while (SerialBT.available()) k = SerialBT.read(), at += char(k),delay(10); 
   filter_id = strtol(at.c_str(),NULL,16); SerialBT.print("Filter ID: "), SerialBT.println(filter_id, HEX); at = "";  }


void CAN_Write(word ID, int DLC, word b0, word b1, word b2, word b3, word b4, word b5, word b6, word b7){
CAN_frame_t tx_frame; tx_frame.FIR.B.FF = CAN_frame_std; tx_frame.MsgID = ID; tx_frame.FIR.B.DLC = DLC; 
tx_frame.data.u8[0] = b0; tx_frame.data.u8[1] = b1; tx_frame.data.u8[2] = b2; tx_frame.data.u8[3] = b3;
tx_frame.data.u8[4] = b4; tx_frame.data.u8[5] = b5; tx_frame.data.u8[6] = b6; tx_frame.data.u8[7] = b7;
ESP32Can.CANWriteFrame(&tx_frame);}

