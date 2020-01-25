
#include <ESP32CAN.h>
#include <CAN_config.h>
#include <Wire.h>
#include "paj7620.h"

#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;

bool control = 1;

word filter_id = 0xFFFF;

CAN_device_t CAN_cfg; 
const int rx_queue_size = 10;   
unsigned long period = 0;   


void setup() {
Wire.begin(14,26);  // (SDA,SCL)
Serial.begin(230400),Serial.println(), Serial.println("ESP32-Arduino-CAN_Paj7620");
SerialBT.begin("Citroen CAN Brige"); delay(5000); SerialBT.println("ESP32-CAN-Paj7620");
CAN_cfg.speed = CAN_SPEED_125KBPS;
CAN_cfg.tx_pin_id = GPIO_NUM_33;
CAN_cfg.rx_pin_id = GPIO_NUM_27;
CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
ESP32Can.CANInit();

paj7620Init();

uint8_t error = 0;
error = paj7620Init();   
if (error) { SerialBT.print("INIT ERROR,CODE:"), SerialBT.println(error); } else { SerialBT.println("INIT OK");}
}

void loop() {
paj7620_ESP();
if (Serial.available())  resp_serial(); 
CAN_read ();
}




void CAN_read () { CAN_frame_t rx_frame;
 if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) {
 
 if (rx_frame.MsgID == 0x21F && rx_frame.data.u8[0] == 0x88) {Serial.println("OFF"), control = 0, CAN_Write(0x1A1,8,0x80,0xF1,0x80,0x00,0x00,0x00,0x00,0x00), delay(100);}
 if (rx_frame.MsgID == 0x21F && rx_frame.data.u8[0] == 0x48) {Serial.println("ON") , control = 1, CAN_Write(0x1A1,8,0x80,0x92,0x80,0x00,0x00,0x00,0x00,0x00), delay(100);}
 
 if (rx_frame.MsgID == filter_id || filter_id == 0xFFFF)     {printf("%03X: ", rx_frame.MsgID);
                                                              for (int i = 0; i < rx_frame.FIR.B.DLC; i++) {printf("%02X ", rx_frame.data.u8[i]);}  
                                                              printf("\n");}}
                 }

void resp_serial (){ String at = ""; int k = 0;
   while (Serial.available()) k = Serial.read(), at += char(k),delay(10); 
   filter_id = strtol(at.c_str(),NULL,16); Serial.print("Filter ID: "), Serial.println(filter_id, HEX); at = "";  }


void CAN_Write(word ID, int DLC, word b0, word b1, word b2, word b3, word b4, word b5, word b6, word b7){
CAN_frame_t tx_frame; tx_frame.FIR.B.FF = CAN_frame_std; tx_frame.MsgID = ID; tx_frame.FIR.B.DLC = DLC; 
tx_frame.data.u8[0] = b0; tx_frame.data.u8[1] = b1; tx_frame.data.u8[2] = b2; tx_frame.data.u8[3] = b3;
tx_frame.data.u8[4] = b4; tx_frame.data.u8[5] = b5; tx_frame.data.u8[6] = b6; tx_frame.data.u8[7] = b7;
ESP32Can.CANWriteFrame(&tx_frame);}


void paj7620_ESP() { uint8_t data = 0; paj7620ReadReg(0x43, 1, &data);
if (data == GES_UP_FLAG    && control == 1) {Serial.println("(TRACK +) 0x80"),  CAN_Write(0x21F,3,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00);}
if (data == GES_DOWN_FLAG  && control == 1) {Serial.println("(TRACK -) 0x40"),  CAN_Write(0x21F,3,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00);}   

if (data == GES_RIGHT_FLAG && control == 1) {Serial.println("(VOL-), 0x04");
                                             CAN_Write(0x21F,3,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
                                             delay(50);
                                             CAN_Write(0x21F,3,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00);}
                                             
if (data == GES_LEFT_FLAG  && control == 1) {Serial.println("(VOL+), 0x08");
                                             CAN_Write(0x21F,3,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
                                             delay(50);
                                             CAN_Write(0x21F,3,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00);}
}
   
