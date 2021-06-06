#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include <EEPROM.h>
#include <PZEM004Tv30.h>
#include <NTPtimeESP.h>
#include <FirebaseArduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);


#define FIREBASE_HOST "energy-4c629-default-rtdb.firebaseio.com"                         
#define FIREBASE_AUTH "yiposlSK4v3bqSnGEUOxaz0XRZX5khqcTf6b18qt"  

strDateTime dateTime;
#define Tx 14    //--->Rx pzem
#define Rx 12     //--->Tx pzem
PZEM004Tv30 pzem(Rx, Tx);
#define button 0
#define led 2
float voltage=0;
float current=0;
float power=0;
float energy=0;
float frequency=0;
float pf;

float csHomqua, csHomtruoc, csThangroi, csThangtruoc;
int ngayChotSo=1;
int gioChotSo=23, phutChotSo=59;giayChotso=50;

int gia =2000;

boolean resetE = false;
unsigned long times = millis();
//----------------------------
NTPtime NTPch("ch.pool.ntp.org");
char ssid[] = "Matwifiroi"; //Tên wifi
char pass[] = "kiduynhat"; //Mật khẩu
const char auth[] = "Jr42Z6unNvlN6W3IQdunEfESE4fYXnvW";

WidgetRTC rtc;
WidgetLED led_connect(V0);
BlynkTimer timer;
boolean savedata = false;

byte customChar[] = {
  B00010,
  B00111,
  B00110,
  B00011,
  B00011,
  B00110,
  B00010,
  B00000
};

void setup() {
  lcd.begin();
  lcd.backlight();
  lcd.createChar(0, customChar);
  lcd.setCursor(3,0);
  lcd.print("DIEN NANG");
  lcd.setCursor(1,1);
  lcd.print("DESIGN BY DDQ");
  delay(3000);
  lcd.clear();
  Serial.begin(115200);
  pinMode(button,INPUT_PULLUP);
  pinMode(led,OUTPUT);
  digitalWrite(led,HIGH);
  attachInterrupt(digitalPinToInterrupt(button),resetEnergy,FALLING);//hàm ngắt ,pin đi từ cao xuống thấp

  Blynk.begin(auth, ssid, pass);
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  EEPROM.begin(512);
  delay(10);
  readChiSo();
  setSyncInterval(60*60);
  timer.setInterval(60000L,saveData);//1 phút lưu giá trị 1 lần

  timer.setInterval(1000L,blinkled);
  float sum = Firebase.getFloat("user-posts/diennang/r8PN6ox87kOSk10CwUlSuqfjrXk1/Tổng");

}

void loop() {
 dateTime = NTPch.getNTPtime(7.0, 0); // Chọn múi giờ +7, tắt chế độ daylight
  if(millis() - times> 5000){
    times = millis();
    readPzem();
    Blynk.virtualWrite(V1, voltage);//Điện áp
    Blynk.virtualWrite(V2, current);//dòng điện
    Blynk.virtualWrite(V3, power);//Công suất 
    Blynk.virtualWrite(V4, frequency);//Tần số
    Blynk.virtualWrite(V5,energy-csHomqua); //hôm nay
    Blynk.virtualWrite(V6,csHomqua-csHomtruoc);//hôm qua
    Blynk.virtualWrite(V7,energy-csThangroi);//tháng này
    Blynk.virtualWrite(V8,csThangroi-csThangtruoc);//tháng trước
    Blynk.virtualWrite(V9,energy); //Tổng
 
  }
   
   String curr = String(energy-csHomqua) + String(" Wh"); //hôm nay
    String yest = String(csHomqua-csHomtruoc) + String(" Wh"); //hôm qua
    String mont = String(energy-csThangroi) + String(" Wh"); //tháng này
    String montyt = String(csThangroi-csThangtruoc) + String(" Wh"); //tháng trước
    String montyte = String(energy-csThangroi-csThangtruoc) + String(" Wh"); //tháng trước nữa
    float sum =  float(energy) ; //tổng số điện
    float mon = sum *2000;
    
      if(hour()==gioChotSo && minute() == phutChotSo && second() == giayChotso { //đến 23h59p thì lưu số điện ngày hôm đó
    Firebase.pushString("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Ngày/Chi tiết",String(energy-csHomqua)+ ' ' + String(dateTime.day) + "/" + String(dateTime.month) + "/" + String(dateTime.year) );
     }
    Firebase.setString("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Ngày/Hiện tại",curr + ' ' + String(dateTime.day) + "/" + String(dateTime.month) + "/" + String(dateTime.year) );
    Firebase.setString("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Tổng/Số điện", "Điện: " + String(sum) + " Wh");
    Firebase.setString("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Tổng/Số tiền", "Tiền: " + String(mon) + " vnđ");
    Firebase.setString("user-posts/diennang/r8PN6ox87kOSk10CwUlSuqfjrXk1/Ngày/Hiện tại",curr + ' ' + String(dateTime.day) + "/" + String(dateTime.month) + "/" + String(dateTime.year) );
    Firebase.setString("user-posts/diennang/r8PN6ox87kOSk10CwUlSuqfjrXk1/Tổng/Số điện", "Điện: " + String(sum) + " Wh");
    Firebase.setString("user-posts/diennang/r8PN6ox87kOSk10CwUlSuqfjrXk1/Tổng/Số tiền", "Tiền: " + String(mon) + " vnđ");
  
   // Tháng
            if (dateTime.month == 1)
                Firebase.setString("user-posts/diennang/r8PN6ox87kOSk10CwUlSuqfjrXk1/Tháng ","Tháng 11: " +String(Firebase.getFloat("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Tháng/thang2")) + " Wh\nTháng 12: " + String(Firebase.getFloat("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Tháng/thang1")) + " Wh\nTháng 1: " + String(energy) + " Wh");
            else if (dateTime.month == 2)Firebase.setString("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Tháng ","Tháng 12: " + String(Firebase.getFloat("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Tháng/thang2")) + " Wh\nTháng 1: " + String(Firebase.getFloat("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Tháng/thang1")) + " Wh\nTháng 2: " + String(energy) + " Wh");
            else
                Firebase.setString("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Tháng/Tháng trước nữa ","Tháng " +String(dateTime.month - 2) + ": " + String(Firebase.getFloat("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Tháng/thang2"))  +" Wh");
                Firebase.setString("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Tháng/Tháng trước ","Tháng " + String(dateTime.month - 1) + ": " +String(Firebase.getFloat("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Tháng/thang1")) + " Wh") ;
                Firebase.setString("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Tháng/Tháng này","Tháng " +  String(dateTime.month) + ": " + String(energy) + " Wh");
 //reset
            if (dateTime.day == 1)
              {
                        // Reset data, Dien
                        Firebase.setFloat("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Tháng/thang2", Firebase.getFloat("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Tháng/thang1"));
                        Firebase.setFloat("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Tháng/thang1", Firebase.getFloat("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Tháng/thang"));
                        Firebase.setFloat("user-posts/diennang/K2EdVHea7Qh6O7SbgqmUOZ0wvXw1/Tháng/thang", 0);
                        Firebase.setFloat("user-posts/diennang/r8PN6ox87kOSk10CwUlSuqfjrXk1/Tổng/Số điện", 0);                                
                }
    lcd.setCursor(0,0);
    lcd.print("Energy :");
    lcd.print(energy);
    lcd.print(" Kw");
    lcd.setCursor(0,1);
    lcd.write(0);
    lcd.print(mon);
    lcd.print("vnd");
  if(resetE==true){
    resetE = false;
    pzem.resetEnergy();
    for (int i = 0; i < 16; ++i) {
    EEPROM.write(i, 0);//EEPROM.write(adress,value)   
      delay(10);
      digitalWrite(led,!digitalRead(led));
    }
    EEPROM.commit();
    readChiSo();
  }
  timer.run();
  if(savedata==true){
    savedata=false;
    writeChiSo();
  }
  Blynk.run();

}
//==============Chương trình con======================//
void readPzem(){
    voltage = pzem.voltage();
    if( !isnan(voltage) ){
        Serial.print("Voltage: "); Serial.print(voltage); Serial.println("V");
    }
    current = pzem.current();
    if( !isnan(current) ){
        Serial.print("Current: "); Serial.print(current); Serial.println("A");
    }
    power = pzem.power();
    if( !isnan(power) ){
        //Serial.print("Power: "); Serial.print(power); Serial.println("W");
    }
    energy = pzem.energy();
    if( !isnan(energy) ){
        Serial.print("Energy: "); Serial.print(energy,3); Serial.println("kWh");
    } else {
        Serial.println("Error reading energy");
    }

    frequency = pzem.frequency();
    if( !isnan(frequency) ){
        Serial.print("Frequency: "); Serial.print(frequency, 1); Serial.println("Hz");
    }
    pf = pzem.pf();
    if( !isnan(pf) ){
       Serial.print("PF: "); Serial.println(pf);
    }
    Serial.println();
}
ICACHE_RAM_ATTR void resetEnergy(){
  resetE = true;
  Serial.println("Reset energy!");
}
//-------------BLYNK----------------------
BLYNK_CONNECTED() {
  rtc.begin();
  Blynk.syncVirtual(V10);
}
BLYNK_WRITE(V10){
  ngayChotSo = param.asInt();
}
//-------------Ghi dữ liệu kiểu float vào bộ nhớ EEPROM----------------------//
float readFloat(unsigned int addr){
  union{
    byte b[4];
    float f;
  }data;
  for(int i=0; i<4; i++){
    data.b[i]=EEPROM.read(addr+i);
  }
  return data.f;
}
void writeFloat(unsigned int addr, float x){
  union{//kiểu union sẽ lấy kích thước tối thiểu bằng kích thước của phần tử lớn nhất
    byte b[4];
    float f;
  }data;
  data.f=x;
  for(int i=0; i<4;i++){
    EEPROM.write(addr+i,data.b[i]);
  }
}

void readChiSo(){
  csHomqua = readFloat(0);
  csHomtruoc = readFloat(4);
  csThangroi = readFloat(8);
  csThangtruoc = readFloat(12);
  Serial.print("Chỉ số hôm qua: ");Serial.println(csHomqua);
  Serial.print("Chỉ số hôm trước: ");Serial.println(csHomtruoc);
  Serial.print("Chỉ số tháng rồi: ");Serial.println(csThangroi);
  Serial.print("Chỉ số tháng trước: ");Serial.println(csThangtruoc);
}
void saveData(){
  savedata = true;
}
void writeChiSo(){
  if(hour()==gioChotSo && minute() == phutChotSo){
    Serial.println("Ghi số ngày mới!");
    writeFloat(4,csHomqua);
    writeFloat(0,energy);
    if(day()==ngayChotSo){
      Serial.println("Ghi số tháng mới!");
      writeFloat(12,csThangroi);
      writeFloat(8,energy);
    }
    EEPROM.commit();
    readChiSo();
  }
}
void blinkled(){
  if(led_connect.getValue()){
    led_connect.off();
  }else{
    led_connect.on();
  }
}
