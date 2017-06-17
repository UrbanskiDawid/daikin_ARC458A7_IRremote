#include "Decoder.cpp"
#include "Receiver.cpp"

#define PIN_receive 2

//============================================================================
Receiver receiver(PIN_receive);
ISR(TIMER1_OVF_vect) { receiver.tick(); }   // interrupt service routine 
//============================================================================


void setup()
{
  Serial.begin(9600);  
  receiver.begin();  
}


void loop()
{
  delay(1000);

  unsigned long *buff;
  unsigned len;

  if(receiver.getData(&buff,len))
  {
    noInterrupts();
    {
      Serial.print("Rec: #");  Serial.print(len);   Serial.print(" ");
      Decoder decoder(buff,len);
      if(decoder.OK())
      {
        decoder.printButton();
        decoder.printRaw();
        decoder.printBuffer();
      }else{
        decoder.printBuffer();
      }

      receiver.reset();
    }
    interrupts();
  }
}
