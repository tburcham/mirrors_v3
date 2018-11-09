#include <PacketSerial.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>




// By default, PacketSerial automatically wraps the built-in `Serial` object.
// While it is still possible to use the Serial object directly, it is
// recommended that the user let the PacketSerial object manage all serial
// communication. Thus the user should not call Serial.write(), Serial.print(),
// etc. Additionally the user should not use the serialEvent() framework.
//
// By default, PacketSerial uses COBS encoding and has a 256 byte receive
// buffer. This can be adjusted by the user by replacing `PacketSerial` with
// a variation of the `PacketSerial_<COBS, 0, BufferSize>` template found in
// PacketSerial.h.
PacketSerial myPacketSerial;

int ledPin1 = LED_BUILTIN;

// called this way, it uses the default address 0x40
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver pwm2 = Adafruit_PWMServoDriver(0x41);

// Depending on your servo make, the pulse width min and max may vary, you 
// want these to be as small/large as possible without hitting the hard stop
// for max range. You'll have to tweak them as necessary to match the servos you
// have!
#define SERVOMIN  75 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // this is the 'maximum' pulse length count (out of 4096)

// our servo # counter
uint8_t servonum = 0;
uint8_t totalservos = 4;

void setup()
{
  // We begin communication with our PacketSerial object by setting the
  // communication speed in bits / second (baud).
  myPacketSerial.begin(115200);

  // If we want to receive packets, we must specify a packet handler function.
  // The packet handler is a custom function with a signature like the
  // onPacketReceived function below.
  myPacketSerial.setPacketHandler(&onPacketReceived);

  pwm.begin();
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
  delay(10);

  pwm2.begin();
  pwm2.setPWMFreq(60);
  delay(10);
  
}

// you can use this function if you'd like to set the pulse length in seconds
// e.g. setServoPulse(0, 0.001) is a ~1 millisecond pulse width. its not precise!
void setServoPulse(uint8_t n, double pulse) {
  double pulselength;
  
  pulselength = 1000000;   // 1,000,000 us per second
  pulselength /= 60;   // 60 Hz
  Serial.print(pulselength); Serial.println(" us per period"); 
  pulselength /= 4096;  // 12 bits of resolution
  Serial.print(pulselength); Serial.println(" us per bit"); 
  pulse *= 1000000;  // convert to us
  pulse /= pulselength;
  Serial.println(pulse);
  pwm.setPWM(n, 0, pulse);
}


void setServoAngle(uint8_t n, int degrees) {

  Adafruit_PWMServoDriver _pwm;
  if (n < 16) {
    _pwm = pwm;
  } else {
    _pwm = pwm2;
    n -= 16;
  }
  
  double pulselength = map(degrees, 0, 180, SERVOMIN, SERVOMAX);
  _pwm.setPWM(n, 0, pulselength);
  delay(10);
  /*pwm2.setPWM(n, 0, pulselength);
  delay(10);*/
}



void loop()
{
  // Do your program-specific loop() work here as usual.

  // The PacketSerial::update() method attempts to read in any incoming serial
  // data and emits received and decoded packets via the packet handler
  // function specified by the user in the void setup() function.
  //
  // The PacketSerial::update() method should be called once per loop(). Failure
  // to call the PacketSerial::update() frequently enough may result in buffer
  // serial overflows.
  myPacketSerial.update();
}

// This is our handler callback function.
// When an encoded packet is received and decoded, it will be delivered here.
// The `buffer` is a pointer to the decoded byte array. `size` is the number of
// bytes in the `buffer`.
void onPacketReceived(const uint8_t* buffer, size_t size)
{
  // In this example, we will simply reverse the contents of the array and send
  // it back to the sender.

  // Make a temporary buffer.
  uint8_t tempBuffer[size];

  // Copy the packet into our temporary buffer.
  memcpy(tempBuffer, buffer, size);

  // Reverse our temporaray buffer.
  //reverse(tempBuffer, size);

  // Send the reversed buffer back to the sender. The send() method will encode
  // the whole buffer as as single packet, set packet markers, etc.
  // The `tempBuffer` is a pointer to the `tempBuffer` array and `size` is the
  // number of bytes to send in the `tempBuffer`.
  myPacketSerial.send(tempBuffer, size);

  //(char*) stackHolder

  char* input = (char*) tempBuffer;
  //String msg = String(cmsg);

  // Read each command pair 
  char* command = strtok(input, "&");
  while (command != 0)
  {
      // Split the command in two values
      char* separator = strchr(command, ':');
      if (separator != 0)
      {
          // Actually split the string in 2: replace ':' with 0
          *separator = 0;
          int servoId = atoi(command);
          ++separator;
          int position = atoi(separator);
  
          // Do something with servoId and position

          

          setServoAngle((uint8_t)servoId, position);
          
      }
      // Find the next command in input string
      command = strtok(0, "&");
  }

}

// This function takes a byte buffer and reverses it.
void reverse(uint8_t* buffer, size_t size)
{
  uint8_t tmp;

  for (size_t i = 0; i < size / 2; i++)
  {
    tmp = buffer[i];
    buffer[i] = buffer[size - i - 1];
    buffer[size - i - 1] = tmp;
  }
}
