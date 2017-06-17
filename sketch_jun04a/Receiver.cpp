#include <Arduino.h>

class Receiver
{
  /*
   * 8000000 / 1024 = 7812,5 Hz
   * 7812,5 / 50 = 156,26
   * 65536 - 156 = 65380
   * 
   * where:
   * - 8000000 is the main clock frequency in Hz
   * - 1024 is the prescaler that we have set
   * - 50 is the frequency that we want in Hz
   * - 65536 is the maximum timer value
   */
  // Set timer1_counter to the correct value for our interrupt interval
  unsigned timer1_counter   = 65535;// max maximum timer value is 65536.
  //int timer1_counter = 64911;   // preload timer 65536-16MHz/256/100Hz
  //int timer1_counter = 64286;   // preload timer 65536-16MHz/256/50Hz
  //int timer1_counter = 34286;   // preload timer 65536-16MHz/256/2Hz
  //----
  
  unsigned long lastAction=0;//micros
  long maxNoAction=30000;
  
  bool lastState;
  enum eStatus{  stop, idle,  run,  timeout,stop_overflow } Status = stop;
  const int pin;

  #define MEMORYLEN 1000
  unsigned long MEMORY[MEMORYLEN];
  unsigned MEMORYI=0;

public:

  Receiver(const int p_pin):
    pin(p_pin)
  {}

  bool getData(unsigned long **buff, unsigned &len)
  {
    *buff=MEMORY;
    len=MEMORYI;
    return (Status==timeout || Status==stop_overflow);
  }
  
  void begin()
  {
    pinMode(pin, INPUT);

    while(HIGH != digitalRead(pin));//wait for inPin to get high
    {
      delay(1000);
    }
      
    noInterrupts();
    {
      TCCR1A = 0;
      TCCR1B = 0;
      
      TCNT1 = timer1_counter;   //[Timer1 Counter register ] preload timer
      
      TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  
      // set 1024 prescaler
      bitSet(TCCR1B, CS12); //[Timer1 Counter Control Register]
      bitSet(TCCR1B, CS10);
    }
    interrupts();

    reset();
  }

  
  void onTick(bool newAction)
  {
    long timeDelta = abs(micros()-lastAction);
    
    if(newAction) {
      
      MEMORY[MEMORYI++]=timeDelta;
      if(MEMORYI==(MEMORYLEN-1)) {
        Status=stop_overflow;
        return;
      }
          
      lastAction=micros();
      return;
    }
  
    if(timeDelta>maxNoAction){ 
      MEMORY[MEMORYI++]=timeDelta;//save timeout time
      Status=timeout;
    }
  }
  
  void reset()
  {
    MEMORYI=0;
    Status=idle;
  }
  
  
  void tick(){
    TCNT1 = timer1_counter;   // preload timer
    bool IN = digitalRead(pin); 
  
    switch(Status)
    {
      case stop: break;
      
      case idle:     
        if(IN==LOW){
          Status=run;
          lastAction=micros();
          lastState=LOW;
        }
      break;
  
      case run:
      {
         onTick(lastState!=IN);
         lastState=IN;
      }
      break;
  
      case stop_overflow:   return;
      case timeout:         return;
      break;
    }  
  }
};

