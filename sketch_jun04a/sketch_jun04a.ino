 
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


bool MSG[64];

bool convertMEMORY(unsigned long *msg)
{
  unsigned MSGI=0;
  bool ret=true;

  for(unsigned i=0; i<128;i+=2)
  {
    //Serial.print("["); Serial.print(i,DEC); Serial.print("]");

    unsigned long mark = msg[i+0];
        
    
    if(mark > 350 && mark < 600)   { MSG[MSGI]=false; /*Serial.print(" 0 ");*/ }
    else
    if(mark > 1250 && mark < 1450) { MSG[MSGI]=true;  /*Serial.print(" 1 ");*/ }
    else
    {
      if(i==0)
      {
        MSGI--;//NOTE: skip fist pair in OUPTUT
        if(mark > 1600 && mark < 1900) { 
        } 
        else
        { 
          ret=false;
          //Serial.print(mark,DEC); Serial.print(" START ERROR "); 
        } 
      }
      //else{ Serial.print(mark); Serial.print(" MARK ERROR "); }
    }

    unsigned long space = msg[i+1];
    if(space>300 && space<500) { /*Serial.print("space");*/} //ok +/- 200 us
    else                       { ret=false; /*Serial.print(space); Serial.print(" SPACE ERROR ");*/ }

    //Serial.println();
    MSGI++;
  }
  return ret;
}


bool HEAD[24]       ={ 1,0,0,0,1,0,0,0, 0,1,0,1,1,0,1,1, 1,1,1,0,1,0,0,1 };
                     
bool TYPE_A[16]     ={ 0,0,0,0,1,1,1,1, 0,0,0,0,0,0,0,0 };
bool TYPE_B[16]     ={ 0,0,0,0,0,0,0,0, 1,1,1,1,0,0,0,1 };

bool BODY_TYPEA[24] ={ 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,1,0,0,1,1,1,0 };
bool BODY_BRIGH[24] ={ 0,0,0,1,1,1,0,0, 0,0,0,0,0,0,0,0, 1,0,0,1,0,0,1,0 };
bool BODY_TIMER[24] ={ 1,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,1,0,1,0,0,0 };
bool BODY_MODE [24] ={ 1,1,1,0,1,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,1,0,1,0,0 };
bool BODY_ANTI [24] ={ 1,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,0, 0,1,0,1,1,0,0,0 };
bool BODY_TURBO[24] ={ 0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,0, 1,0,0,1,1,0,0,0 };
bool BODY_FAN  [24] ={ 0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,0, 1,0,0,0,0,1,0,0 };
bool BODY_AUTO [24] ={ 0,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,0,0,1,0,0,0 };
bool BODY_LOCK [24] ={ 0,0,1,0,1,0,0,0, 0,0,0,0,0,0,0,0, 1,0,1,0,0,1,0,0 };
bool BODY_ONOFF[24] ={ 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,1,0,0,1,0,0,0 };

bool compare(bool *A,bool *B, unsigned startA,unsigned len)
{
   unsigned Bi=0;
   for(unsigned i=startA; i<startA+len; i++,Bi++)
   {
      if(A[i]!=B[Bi])
      {
        /*
        Serial.print("comapre fail");
        Serial.print(" A[");  Serial.print( i);    Serial.print("]="); Serial.print(A[i]);
        Serial.print(" != ");
        Serial.print(" B[");  Serial.print(Bi);    Serial.print("]="); Serial.print(B[i]);
        Serial.print(" !");*/
        return false;   
      }
   }
   return true;
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
      Serial.print("Rec: #");  Serial.print(MEMORYI,DEC);   Serial.println();

      if( MEMORYI==264 && //check len
          (MEMORY[0]>3300 && MEMORY[0]<3600)//check fist
        )
      {
        unsigned long *part1=&MEMORY[1];
        unsigned long *part2=&MEMORY[133];
        // printALl
        Serial.print("head: "); Serial.print(MEMORY[0]); Serial.print(" timeout: "); Serial.print(MEMORY[MEMORYI-1]);   Serial.println();
        convertMEMORY(part1);    for(int i=1;i<64;i++) {Serial.print(MSG[i]);}Serial.print(" ");
        convertMEMORY(part2);    for(int i=1;i<64;i++) {Serial.print(MSG[i]);}Serial.print(" ");
        // ---

        convertMEMORY(part1);
        if(!compare(MSG,HEAD,          0,24))  {Serial.print("errorA1");}
        if(!compare(MSG,TYPE_A,       24,16))  {Serial.print("errorA2");}
        if(!compare(MSG,BODY_TYPEA,   40,24))  {Serial.print("errorA3");}

        convertMEMORY(part2);
        if(!compare(MSG,HEAD,          0,24))  {Serial.print("errorB1");}
        if(!compare(MSG,TYPE_B,       24,16))  {Serial.print("errorB2");}

        if(compare(MSG,BODY_BRIGH,    40,24)) Serial.print("BRIGHT");
        else
        if(compare(MSG,BODY_TIMER,    40,24)) Serial.print("TIMER");
        else
        if(compare(MSG,BODY_MODE,     40,24))  Serial.print("MODE");
        else
        if(compare(MSG,BODY_ANTI,     40,24))  Serial.print("ANTI");
        else
        if(compare(MSG,BODY_TURBO,    40,24)) Serial.print("TURBO");
        else
        if(compare(MSG,BODY_FAN,      40,24))   Serial.print("FAN");
        else
        if(compare(MSG,BODY_AUTO,     40,24))  Serial.print("AUTO");
        else
        if(compare(MSG,BODY_LOCK,     40,24))  Serial.print("LOCK");
        else
        if(compare(MSG,BODY_ONOFF,    40,24)) Serial.print("ONOFF");
        else                                  Serial.print("?????");
        Serial.println();
      }

      reset();
    }
    interrupts();           // disable all interrupts
  }
  
}
