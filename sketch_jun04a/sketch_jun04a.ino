 
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

//============================================================================+
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
//============================================================================+

class Decoder
{
  bool isOK;
  unsigned long *MEMORY;
  
  bool convertMEMORY(unsigned long *msg,bool *OUT)
  {
    unsigned outI=0;
    bool ret=true;      
  
    for(unsigned i=0; i<129;i+=2)
    {
      unsigned long mark = msg[i+0];
      if(mark >  350 && mark <  600) { OUT[outI]=false; }
      else
      if(mark > 1250 && mark < 1450) { OUT[outI]=true;  }
      else
      {
        if(i==0)
        {
          outI--;//NOTE: skip fist pair in OUPTUT
          if(mark > 1600 && mark < 1900) { } 
          else { ret=false; } 
        }
      }
  
      unsigned long space = msg[i+1];
      if(space>300 && space<500) { }
      else                       { ret=false; }
  
      outI++;
    }
    return ret;
  }
  
  bool compare(bool *A,bool *B, unsigned startA,unsigned len)
  {
     unsigned Bi=0;
     for(unsigned i=startA; i<startA+len; i++,Bi++)
     {
        if(A[i]!=B[Bi]) return false;        
     }
     return true;
  }

public:
  enum eButton{
    unknown,
    BRIGH,
    TIMER,
    MODE ,
    ANTI ,
    TURBO,
    FAN  ,
    AUTO ,
    LOCK ,
    ONOFF
  }button;

  Decoder(unsigned long *buffer,unsigned len)
  {
      MEMORY=buffer;
    
      if( len==264 && //check len
          (buffer[0]>3300 && buffer[0]<3600)//check fist  
        )
      {
        unsigned long *part1=&buffer[1];
        unsigned long *part2=&buffer[133];
        
        bool MSG[64];
  
        convertMEMORY(part1,MSG);
        if(!compare(MSG,HEAD,          0,24))  {Serial.print("errorA1"); isOK=false;}
        if(!compare(MSG,TYPE_A,       24,16))  {Serial.print("errorA2"); isOK=false;}
        if(!compare(MSG,BODY_TYPEA,24+16,24))  {Serial.print("errorA3"); isOK=false;}
        
        convertMEMORY(part2,MSG);
        if(!compare(MSG,HEAD,          0,24))  {Serial.print("errorB1"); isOK=false;}
        if(!compare(MSG,TYPE_B,       24,16))  {Serial.print("errorB2"); isOK=false;}

        if(compare(MSG,BODY_BRIGH,    40,24)) {button=BRIGH;}
        else
        if(compare(MSG,BODY_TIMER,    40,24)) {button=TIMER;}
        else
        if(compare(MSG,BODY_MODE,     40,24)) {button=MODE;}
        else
        if(compare(MSG,BODY_ANTI,     40,24)) {button=ANTI;}
        else
        if(compare(MSG,BODY_TURBO,    40,24)) {button=TURBO;}
        else
        if(compare(MSG,BODY_FAN,      40,24)) {button=FAN;}
        else
        if(compare(MSG,BODY_AUTO,     40,24)) {button=AUTO;}
        else
        if(compare(MSG,BODY_LOCK,     40,24)) {button=LOCK;}
        else
        if(compare(MSG,BODY_ONOFF,    40,24)) {button=ONOFF;}
        else                                  {button=unknown; isOK=false;}
    
        isOK=true;
        return;
      }else{
        isOK=false;
        Serial.print("Decode fail");
      }      
  }
  
  void printButton()
  {
    switch(button)//TODO: array of string
    {
      case BRIGH: Serial.println("BRIGHT"); break;
      case TIMER: Serial.println("TIMER");  break;
      case MODE:  Serial.println("MODE");   break;
      case ANTI:  Serial.println("ANTI");   break;
      case TURBO: Serial.println("TURBO");  break;
      case FAN:   Serial.println("FAN");    break;
      case AUTO:  Serial.println("AUTO");   break;
      case LOCK:  Serial.println("LOCK");   break;
      case ONOFF: Serial.println("ONOFF");  break;
      default:    Serial.println("?????");  break;
    }
  }

  void printRaw()
  {    
    Serial.print("head: "); Serial.print(MEMORY[0]); Serial.print(" timeout: "); Serial.print(MEMORY[MEMORYI-1]);   Serial.println();
    unsigned long *part1=&MEMORY[1];
    unsigned long *part2=&MEMORY[133];

    bool MSG[64];
    convertMEMORY(part1,MSG);    Serial.print("|"); for(int i=0;i<64;i++) { if(i==24 || i==24+16) {Serial.print("|");} Serial.print(MSG[i]);}Serial.println("| ");  
    convertMEMORY(part2,MSG);    Serial.print("|"); for(int i=0;i<64;i++) { if(i==24 || i==24+16) {Serial.print("|");} Serial.print(MSG[i]);}Serial.println("| ");
  }

};

void loop()
{
  delay(1000);

  if(Status==timeout)
  {
    noInterrupts();
    {
      Decoder decoder(MEMORY,MEMORYI);
      Serial.print("Rec: #");  Serial.print(MEMORYI,DEC);   Serial.print(" ");
      decoder.printButton();
      decoder.printRaw();

      reset();
    }
    interrupts();
  }
  
}
