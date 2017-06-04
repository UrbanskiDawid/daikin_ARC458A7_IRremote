 
#define inPin  2

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

#define MEMORYLEN 1000
unsigned long MEMORY[MEMORYLEN];
unsigned MEMORYI=0;

enum eStatus{  stop, idle,  run,  timeout,stop_overflow } Status = stop;

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

bool lastState;
ISR(TIMER1_OVF_vect)        // interrupt service routine 
{
  TCNT1 = timer1_counter;   // preload timer
  bool IN = digitalRead(inPin); 

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

// microseconds per clock interrupt tick
#define USECPERTICK    50
#define TIMER_COUNT_TOP  (SYSCLOCK * USECPERTICK / 1000000)

void setup()
{
  pinMode(inPin, INPUT);      // sets the digital pin 7 as input

  Serial.begin(9600);  

  //Serial.print("Freq: ");Serial.println(F_CPU);

  {
    do
    {
      //Serial.print("waiting for inPin ");      Serial.print(inPin);       Serial.println(" to go high");
      delay(1000);
    }while(HIGH != digitalRead(inPin));
  }

  // initialize timer1
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


void printMsg(unsigned long *msg)
{
  for(unsigned i=1; i<128;i+=2)
  {
    Serial.print("[");
    Serial.print(i,DEC);
    Serial.print("]");
    
    unsigned long mark = msg[i+0];
    if(mark > 350 && mark < 600)   { Serial.print(" 0 "); } //ok +/- 200 us
    else
    if(mark > 1250 && mark < 1450) { Serial.print(" 1 "); } //ok +/- 200 us
    else                           { Serial.print(mark); Serial.print(" MARK ERROR ");}

    unsigned long space = msg[i+1];
    if(space>300 && space <500) { } //ok +/- 200 us
    else                        { Serial.print(space); Serial.print(" SPACE ERROR "); }

    Serial.println();
  }
}

void loop()
{
  /*
  Serial.print("status:");
  Serial.print((int)Status);
  Serial.println();
  */
  delay(1000);
  if(Status==timeout)
  {
    noInterrupts();           // disable all interrupts
    {
      Serial.print("Rec: #");
      Serial.print(MEMORYI,DEC);
      Serial.println();

      Serial.println(MEMORY[0]);
/*
      if(MEMORYI==264)
      {
        unsigned long *part1=&MEMORY[1];
        unsigned long *part2=&MEMORY[133];
        printMsg(part1);
        printMsg(part2);      
      }
*/
      for(unsigned i=1; i<MEMORYI-1;i+=2)
      {
        Serial.print("[");
        Serial.print(i,DEC);
        Serial.print("]");
        
        unsigned long mark = MEMORY[i+0];
        if(mark > 350 && mark < 600)   { Serial.print(" 0 "); } //ok +/- 200 us
        else
        if(mark > 1250 && mark < 1450) { Serial.print(" 1 "); } //ok +/- 200 us
        else                           { Serial.print(mark); Serial.print(" MARK ERROR ");}
        
        
        unsigned long space = MEMORY[i+1];
        if(space>300 && space <500) { } //ok +/- 200 us
        else                        { Serial.print(space); Serial.print(" SPACE ERROR "); }

        Serial.println();
      }
      
      Serial.println(MEMORY[MEMORYI-1]);

      reset();
    }
    interrupts();           // disable all interrupts
  }
  
}
