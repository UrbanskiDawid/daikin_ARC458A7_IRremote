#define IR_SEND_PWM_PIN 11
#define IR_CLOCK_RATE 36000L

class Sender
{
  const int pin;
  
public:
  Sender(int outPin):
    pin(outPin)
  {
    pinMode(pin, INPUT);//output dissabled
    digitalWrite(pin, LOW);
  }

  void enableIROut(unsigned char khz)
  {    
    TIMSK2 = 0;
    TCCR2A = _BV(WGM21) | _BV(COM2A0);//18.11.1 TCCR2A – Timer/Counter Control Register A
    TCCR2B =_BV(CS20);                //18.11.2 TCCR2B – Timer/Counter Control Register B
    OCR2A = (F_CPU/(IR_CLOCK_RATE*2L)-1);
 }

  void mark(unsigned int time)
  {
    pinMode(pin, OUTPUT);
    delayMicroseconds(time);
  }

  void space(unsigned int time)
  {
    pinMode(pin, INPUT);
    delayMicroseconds(time);
  }

  void send(bool *buff)
  {
    mark(3600);
    space(1600);

    for(int i=0;i<64;i++)
    {
      mark(400);
      space( (buff[i]==true) ? 1300 : 500 );
   }

   mark (400);
   space(29900U);
  }

  void sendOnOff()
  {    
    enableIROut(36);
        
    //header
    bool PART1[64]       ={ 1,0,0,0,1,0,0,0, 0,1,0,1,1,0,1,1, 1,1,1,0,1,0,0,1, /*TYPE_A[16]*/ 0,0,0,0,1,1,1,1, 0,0,0,0,0,0,0,0, /*BODY_TYPEA*/ 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,1,0,0,1,1,1,0 };
    send(PART1);

    //payload
    bool PART2[64]       ={ 1,0,0,0,1,0,0,0, 0,1,0,1,1,0,1,1, 1,1,1,0,1,0,0,1, /*TYPE_B[16]*/ 0,0,0,0,0,0,0,0, 1,1,1,1,0,0,0,1, /*BODY_ONOFF*/ 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,1,0,0,1,0,0,0 };
    send(PART2);
  }
};
Sender sender(IR_SEND_PWM_PIN);

void setup()
{ 
  sender.sendOnOff();
}

void loop(){ delay(100000); }//nothing to do: message will be send on arduino reset!

