#include <SoftwareSerial.h> 
#include "DHT.h"
SoftwareSerial sim800l(8, 9); // 8 - RX Arduino (TX SIM800L), 9 - TX Arduino (RX SIM800L)
int piezoPin = 5;
#define flamePin A1
int flame;
#define DHTPIN 2
DHT dht(DHTPIN, DHT11);
int quantity = 0; // для того, чтобы смс не отправлялось дважды
unsigned long last_time;//переменная для хранения времени
char incomingByte;//переменная для считывания данных из смс
String inputString;//переменная для записи данных смс
#define analogSignal A0 //подключение аналогового сигнального пина датчика газа
int digitalSignal = 8; //подключение цифрового сигнального пина датчика газа

int noGas;
float gasValue;
float t;
float p;
float h;

void SendSMS()
{
  Serial.println("Sending SMS...");                // Печать текста
  sim800l.print("AT+CMGF=1\r");                   // Выбирает формат SMS
  delay(100);
  sim800l.print("AT+CMGS=\"+79235671896\"\r");  // Отправка СМС на указанный номер +792100000000"
  delay(300);
  sim800l.println("Attention, Emergency situation!\n");
  sim800l.print("Humidity: "); 
  sim800l.print(dht.readHumidity());
  sim800l.print(" %"); 
  sim800l.println("\n");       // Тест сообщения
  sim800l.print("Temperature: "); 
  sim800l.print(dht.readTemperature());
  sim800l.print(" *C"); 
  delay(300);
  sim800l.print((char)26);// (требуется в соответствии с таблицей данных)
  delay(300);
  sim800l.println();
  Serial.println("Text Sent.");
  delay(500);
}

void sms(String text, String phone) {  // Процедура Отправка SMS
  Serial.println("SMS send started");
  sim800l.println("AT+CMGS=\"" + phone + "\"");
  delay(500);
  sim800l.print(text);
  delay(500);
  sim800l.print((char)26);
  delay(500);
  Serial.println("SMS send complete");
  delay(2000);
}

void setup()
{
  pinMode(flamePin, INPUT);
  pinMode(piezoPin, OUTPUT);
  pinMode(digitalSignal, INPUT); //установка режима пина датчика газа
  dht.begin();
  sim800l.begin(9600);   //Инициализация последовательной связи с Arduino и SIM800L
  Serial.begin(9600);   // Инициализация последовательной связи с Arduino и Arduino IDE (Serial Monitor)

  while(!sim800l.available()){             // Зацикливаем и ждем инициализацию SIM800L
   sim800l.println("AT");                  // Отправка команды AT
   delay(1000);                             // Пауза
   Serial.println("Connecting...");         // Печатаем текст
   }
   Serial.println("Connected!");            // Печатаем текст
   sim800l.println("AT+CMGF=1");           // Отправка команды AT+CMGF=1
   delay(1000);                             // Пауза
   sim800l.println("AT+CNMI=1,2,0,0,0");   // Отправка команды AT+CNMI=1,2,0,0,0
   delay(1000);                             // Пауза
   sim800l.println("AT+CMGL=\"REC UNREAD\"");
}


void loop() {
  noGas = digitalRead(digitalSignal); //считываем значение о присутствии газа
  gasValue = analogRead(analogSignal); // и о его количестве
  Serial.println(noGas);
  Serial.println(gasValue);
  flame = digitalRead(flamePin);
  Serial.print("Flame Sensor - ");
  Serial.println(flame);
  Serial.println(dht.readTemperature());

  if(sim800l.available()){                       // Проверяем, если есть доступные данные
      delay(100);                                 // Пауза
      
      while(sim800l.available()){                // Проверяем, есть ли еще данные.
      incomingByte = sim800l.read();             // Считываем байт и записываем в переменную incomingByte
      inputString += incomingByte;                // Записываем считанный байт в массив inputString
      }
   
      delay(10);                                  // Пауза      
      Serial.println(inputString);                // Отправка в "Мониторинг порта" считанные данные
      inputString.toUpperCase();                  // Меняем все буквы на заглавные
 
      if (inputString.indexOf("DATA") > -1){      // Проверяем полученные данные
        t = dht.readTemperature();
        h = dht.readHumidity();
        sms(String("Temperature: " + String(t) + " *C " + " Humidity: " + String(h) + " % "), String("+79235671896")); // Отправка SMS  
        }     
        
        delay(50);
      if (inputString.indexOf("OK") == -1){
          sim800l.println("AT+CMGDA=\"DEL ALL\"");
          delay(1000);}
    
          inputString = "";
  }
    
  if (flame == HIGH) { // Если огня не обнаружено
    Serial.println("GOOD");
    delay(100);
  }

  if (flame == LOW || gasValue > 449) { // Если огонь обнаружен
    SendSMS();
    Serial.println("POZHAR");
    analogWrite(piezoPin, 1);
    delay(1000); // Сколько работает пищалка
    analogWrite(piezoPin, 0);
  }
  delay(1000);
}