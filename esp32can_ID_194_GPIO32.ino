
#include <ESP32CAN.h>  // https://github.com/miwagner/ESP32-Arduino-CAN/tree/master
#include <CAN_config.h>
#include <Wire.h>
 

#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;

RTC_DATA_ATTR int bootCount = 0;
unsigned long timout_CAN = 60000;

bool control = 1;

//word filrer[10] = {0x4D2,0x305,0x072,0x228,0x208};
word filter_id = 0xFFFF;

CAN_device_t CAN_cfg; 
const int rx_queue_size = 10;   
unsigned long period = 0;   
word state_pid_399 = 0x00;

#define button_pin 32 // пин для подключения оптопары для дергания кнопри двери багажника

void setup() {
 

Serial.begin(230400),Serial.println(),   ++bootCount; Serial.println("Проснулся " + String(bootCount));
CAN_cfg.speed = CAN_SPEED_125KBPS;
//CAN_cfg.speed = CAN_SPEED_500KBPS;
CAN_cfg.tx_pin_id = GPIO_NUM_33;
CAN_cfg.rx_pin_id = GPIO_NUM_27;

CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));

ESP32Can.CANInit();
pinMode(button_pin, OUTPUT);
SerialBT.begin("ESP32 CAN Stiffer"); SerialBT.println("Hello" + String(bootCount));

}

void loop() {

if (Serial.available())   resp_serial(); 
if (SerialBT.available()) resp_SerialBT(); 

CAN_read ();

                       
                       
if (millis() > timout_CAN + 20000) Serial.println("Засыпаю...."), esp_sleep_enable_ext0_wakeup(GPIO_NUM_27,0), esp_deep_sleep_start();
}




void CAN_read () { CAN_frame_t rx_frame;
 if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) {
 timout_CAN = millis();

 if (rx_frame.MsgID == 0x399) {state_pid_399 = rx_frame.data.u8[0];}
 
 if (rx_frame.MsgID == 0x194) { // 
                               byte data_7 = rx_frame.data.u8[6];  
                               byte out = data_7 << 4;  
Serial.println(out,HEX);
                               if (out == 0x00 && state_pid_399 != 0x00) {// Serial.println("Средняя кнопка нажата и дверь закрыта");
                                                  SerialBT.println("Нажата кнопка закрыть при не закрытой двери");
                                                  delay(1000);
                                                  digitalWrite(button_pin, HIGH);
                                                  delay(800);
                                                  digitalWrite(button_pin, LOW);}
                               
                               if (out == 0x20) {// Serial.println("Средняя кнопка нажата");
                                                  SerialBT.println("Средняя кнопка нажата");
                                                  digitalWrite(button_pin, HIGH);
                                                  delay(800);
                                                  digitalWrite(button_pin, LOW);}
                                                  
                               if (out == 0x10) { //Serial.println("Средняя кнопка нажата");
                                                  SerialBT.println("Нажата кнопка открыть");}                                                  
                              }

/*

for (int i = 0; i < 4; ++i) {   if (rx_frame.MsgID == filrer[i]) {
                                                          printf("%03X: ", rx_frame.MsgID); 
                                                          for (int i = 0; i < rx_frame.FIR.B.DLC; i++) {printf("%02X ", rx_frame.data.u8[i],HEX);}  
                                                          printf("\n");                 }
                                                          } */



                                                    }
                 }


 
void resp_serial (){ String at = ""; int k = 0;
   while (Serial.available()) k = Serial.read(), at += char(k),delay(10); 
   filter_id = strtol(at.c_str(),NULL,16); Serial.print("Filter ID: "), Serial.println(filter_id, HEX); at = "";  }

void resp_SerialBT (){ String at = ""; int k = 0;
   while (SerialBT.available()) k = SerialBT.read(), at += char(k),delay(10); 
   filter_id = strtol(at.c_str(),NULL,16); SerialBT.print("Filter ID: "), SerialBT.println(filter_id, HEX); at = "";  }
