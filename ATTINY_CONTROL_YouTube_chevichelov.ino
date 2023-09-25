#include <TM1650.h>

#define PIN_A2   A2                                                                           //Назначаем пин A2
#define PIN_P1   1                                                                            //Назначаем пин P1
#define PIN_P3   3                                                                            //Назначаем пин P3


float R1_R4                           =  108.9;                                               //Сопротивление резистора R1 + R4 в кОм 98.9 + 10
float R2                              =  9.7;                                                 //Сопротивление резистора R2 в кОм 9.7

float DEFAULT_VOLT                    = 0;                                                    //Напряжение по умолчанию;
float  VOLT_DISPLAY                   = 0;                                                    //Объявляем переменную для хранения значения напряжения
int VOLT                              = 0;                                                    //Объявляем переменную для отображения на дисплее
byte DOT                              = 0;                                                    //Объявляем переменную для хранения позиции точки
unsigned long TIME_BATTERY            = 0;                                                    //Объявляем переменную таймера измерения напряжения батареи
unsigned long TIME                    = 0;                                                    //Объявляем переменную таймера задержки измерений



                                                                                              //Описание из библиотеки
                                                                                              //Define a 4-digit display module. Pin suggestions:
                                                                                              //ESP8266 (Wemos D1): data pin 5 (D1), clock pin 4 (D2)
                                                                                              //ATtiny44A: data pin 9, clock pin 10 (LED_BUILTIN: 8 on ATTinyCore)
TM1650 module(0, 2);                                                                          //data, clock, 4 digits





void setup()
{
  pinMode(PIN_P1, OUTPUT);                                                                      //Пин установлен на выход
  pinMode(PIN_P3, OUTPUT);                                                                      //Пин установлен на выход
  pinMode(PIN_A2, INPUT);                                                                       //Пин установлен на вход
  DEFAULT_VOLT                        = GET_DEFAULT_VOLT();                                     //Измеряем внутреннее напряжение
}


void loop()
{
  if (millis() - TIME <= 200)                                                                   //Добавляем задержку в 200 миллисекунд
    return;
  TIME = millis(); 

  if (millis() - TIME_BATTERY >= 60000)                                                         //Добавляем задержку в 1 минуту для измерения напряжения питания
  {
    DEFAULT_VOLT = GET_DEFAULT_VOLT();                                                          //Измеряем внутреннее напряжение
    TIME_BATTERY = millis(); 
  }
  
  module.setupDisplay(true, 7);                                                                 //иногда шум на линии может изменить уровень интенсивности, поэтому устанавливаем его заного
  VOLT_DISPLAY = analogRead(PIN_A2) * DEFAULT_VOLT / 1024 * (( R1_R4 + R2 ) / R2);              //Рассчитываем значение напряжения
  if (VOLT_DISPLAY >= 0.7)                                                                      //Если напряжение больше 0.7 вольт, значит плюс
  {
    if (digitalRead(PIN_P1) == HIGH)                                                            //Подаём прерывистый звуковой сигнал
      digitalWrite(PIN_P1, LOW);
    else
      digitalWrite(PIN_P1, HIGH);
    digitalWrite(PIN_P3, LOW);                                                                  //Отключаем напряжение на пине P3
  } 
  else if (VOLT_DISPLAY < 0.7 && VOLT_DISPLAY >= 0.3)                                           //Если напряжение в диапазоне от 0.3 до 0.7 вольт, значит масса
  {
    digitalWrite(PIN_P1, HIGH);                                                                 //Подаём непрерывистый звуковой сигнал
    digitalWrite(PIN_P3, HIGH);                                                                 //Подаём напряжение на пин P3
  }
  else if (VOLT_DISPLAY < 0.3)                                                                  //Если напряжение ниже 0.3 вольт
  {
    digitalWrite(PIN_P1, LOW);                                                                  //Отключаем звуковой сигнал
    digitalWrite(PIN_P3, HIGH);                                                                 //Подаём напряжение на пин P3
  }
    
  if (VOLT_DISPLAY >= 10)                                                                       //Напряжение больше 10 вольт
  {
    DOT   = 3;                                                                                  //Устанавливаем точку на 3 позицию
    VOLT  = VOLT_DISPLAY * 100;                                                                 //Сдвигаем значение напряжение на 2 бита влево
  }
  else                                                                                          //Напряжение меньше 10 вольт
  { 
    DOT   = 4;                                                                                  //Устанавливаем точку на 4 позицию
    VOLT  = VOLT_DISPLAY * 1000;                                                                //Сдвигаем значение напряжение на 3 бита влево
  }
  if (VOLT_DISPLAY >= 0.7 || VOLT_DISPLAY < 0.3)                                                //Если напряжение больше 0.7 вольт и меньше 0.3 вольта
  {
    module.setDisplayToDecNumber(VOLT, _BV(DOT));                                               //Отображаем значение на дисплее
  }
  else                                                                                          //Иначе
  {
    module.setDisplayToString("MASS");                                                          //Отображаем на дисплее слово MASS
  }   
}

float GET_DEFAULT_VOLT() {                                                                      //Функция измеряет внутреннее напряжение Arduino
  long RESULT         = 0;                                                                      //Определяем переменную для получения результата.
  byte  COUNT_RESULT  = 100;                                                                    //Определяем сколько значений АЦП требуется получить для усреднения результата.
                                                                                                //Для Arduino Mega, Leonardo и Micro, сбрасываем бит «MUX5» регистра «ADCSRB», так как «MUX[5-0]» должно быть равно 011110 (см. регистр «ADMUX»).
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //Устанавливаем биты регистра «ADMUX»: «REFS»=01 (ИОН=VCC), «ADLAR»=0 (выравнивание результата по правому краю), «MUX[4-0]»=11110 или «MUX[3-0]»=1110 (источником сигнала для АЦП является напряжение ИОН на 1,1 В).   
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif

  for(byte i=0; i<COUNT_RESULT; i++)                                                            //Получаем несколько значений АЦП
  {
    ADCSRA |= _BV(ADSC);                                                                        //Запускаем преобразования АЦП:Устанавливаем биты регистра «ADCSRA»: «ADEN»=1  (вкл АЦП), «ADSC» =1 (запускаем новое преобразование). 
    while (bit_is_set(ADCSRA, ADSC));                                                           //Получаем данные АЦП:
    uint8_t _LOW  = ADCL;
    uint8_t _HIGH = ADCH;
  
    RESULT += (_HIGH << 8) | _LOW;                                                              //Суммируем результат
  }
  RESULT /= COUNT_RESULT;                                                                       //Делим результат «RESULT» на «COUNT_RESULT», так как мы получили его «COUNT_RESULT» раз.    
  return (1.1f/RESULT) * 1024;                                                                  //Рассчитываем напряжение питания:  //  АЦП = (Uвх/Vcc)*1023. Напряжение Uвх мы брали с внутреннего ИОН на 1.1 В, значение которого возвращает функция analogSave_1V1(0).
}
