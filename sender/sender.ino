#define IR_SEND_PWM_PIN 11

class Sender
{
  const int pin;

  //PREFIX(3B) + HEADER(2B) + BODY(3B)
  //============================================================================+
  const byte PREFIX[3]      ={ 0b10001000, 0b01011011, 0b11101001 };
  const byte HEADER_BEGIN[2]={ 0b00001111, 0b00000000 };
  const byte HEADER_BODY[2] ={ 0b00000000, 0b11110001 };
  const byte BODY_BEGIN[3] ={ 0b00000000, 0b00000000, 0b01001110 };  
 //============================================================================+

public:
  const byte BODY_BRIGH[3] ={ 0b00011100, 0b00000000, 0b10010010 };
  const byte BODY_TIMER[3] ={ 0b11000000, 0b00000000, 0b00101000 };
  const byte BODY_MODE [3] ={ 0b11101000, 0b00000000, 0b00010100 };
  const byte BODY_ANTI [3] ={ 0b10010000, 0b00000000, 0b01011000 };
  const byte BODY_TURBO[3] ={ 0b00010000, 0b00000000, 0b10011000 };
  const byte BODY_FAN  [3] ={ 0b00001000, 0b00000000, 0b10000100 };
  const byte BODY_AUTO [3] ={ 0b01000000, 0b00000000, 0b11001000 };
  const byte BODY_LOCK [3] ={ 0b00101000, 0b00000000, 0b10100100 };
  const byte BODY_ONOFF[3] ={ 0b10000000, 0b00000000, 0b01001000 };

private:

  void enableIROut()
  {    
    TIMSK2 = 0;
    TCCR2A = _BV(WGM21) | _BV(COM2A0);//18.11.1 TCCR2A – Timer/Counter Control Register A
    TCCR2B =_BV(CS20);                //18.11.2 TCCR2B – Timer/Counter Control Register B
    OCR2A = (F_CPU/(36000L*2L)-1);    //36000L - 36kHz
 }

  void mark(const unsigned int time)
  {
    pinMode(pin, OUTPUT);
    delayMicroseconds(time);
  }

  void space(const unsigned int time)
  {
    pinMode(pin, INPUT);
    delayMicroseconds(time);
  }

  void send(const bool *buff)
  {    
    mark(3600);
    space(1600);
   
    for(int i=0;i<64;i++)
    {
      mark(400);
      space( (buff[i]==true) ? 1300 : 500 );
   }

   mark (400);
   space(0);
  }

  void generateMessages(bool * ret, 
                        const byte *header,
                        const byte * body)
  {    
    int retI=0;

    for(int i=0;i<3;i++) for(int b=7;b>=0;b--)    ret[retI++]=bitRead(PREFIX[i],b);
    for(int i=0;i<2;i++) for(int b=7;b>=0;b--)    ret[retI++]=bitRead(header[i],b);
    for(int i=0;i<3;i++) for(int b=7;b>=0;b--)    ret[retI++]=bitRead(body[i],  b);
  }

public:

  Sender(int outPin):
    pin(outPin)
  {
    pinMode(pin, INPUT);//output dissabled
    digitalWrite(pin, LOW);
  }
  
  void sendCommand(const byte *body)
  { 
    bool message1[64];
    generateMessages(message1,HEADER_BEGIN,BODY_BEGIN);

    bool message2[64];
    generateMessages(message2,HEADER_BODY,body);
       
    enableIROut();
    
    //header
    send(message1);

    //wait less than 30000 usec
    delayMicroseconds(10000);
    delayMicroseconds(10000);
    delayMicroseconds( 9800);
    //--
    
    //payload
    send(message2);

    //create a 30000 usec delay before sending next command
    delayMicroseconds(10000);
    delayMicroseconds(10000);
    delayMicroseconds(10000);
    //--
  }
};
Sender sender(IR_SEND_PWM_PIN);

void setup()
{ 
  sender.sendCommand(sender.BODY_ONOFF);
  sender.sendCommand(sender.BODY_FAN);
  sender.sendCommand(sender.BODY_FAN);
  sender.sendCommand(sender.BODY_BRIGH);
  sender.sendCommand(sender.BODY_BRIGH);
  sender.sendCommand(sender.BODY_TIMER);
  sender.sendCommand(sender.BODY_TIMER);
  sender.sendCommand(sender.BODY_TIMER);
}

void loop(){ delay(100000);}//nothing to do: message will be send on arduino reset!

