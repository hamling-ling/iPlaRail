#include <IOscMessageParser.h>

#define MTRDRV_IN1 12 // TA7291P pin 6(IN1)
#define MTRDRV_IN2 6 // TA7291P pin 5(IN2)
#define MTRDRV_VRF 5 // TA7291P pin 4(Vref)
#define LED_PIN    13
#define LED_FRONT_BLU  8  // front blue
#define LED_FRONT_RED  9  // front red
#define LED_REAR_BLU  10  // rear blue
#define LED_REAR_RED  11  // rear red

#define OSC_ADDRESS_LED         "/train/led"
#define OSC_ADDRESS_TRAIN_ACCL  "/train/acc"
#define OSC_ADDRESS_TRAIN_MODE  "/train/dir"

#define HIGH 1
#define LOW  0
#define MOTOR_MAX  255
#define MOTOR_MIN  0

using namespace OscMessageParser;

typedef enum {
  MOTOR_STATE_NEUTRAL,
  MOTOR_STATE_BRAKE,
  MOTOR_STATE_FORWARD,
  MOTOR_STATE_BACKWARD,
} MotorState;

typedef enum {
	TRAIN_MODE_OFF,
	TRAIN_MODE_FORWARD,
	TRAIN_MODE_BACKWARD,
} TrainDirection;

class Train {
public:
	Train();
	~Train();
	void SetRunMode(TrainDirection direction);
	void Accel(int accl);
private:
	TrainDirection _direction;
        int _accel;
	void MotorControl(int state, int val);
};

Train::Train(){
	// init pins
	pinMode(LED_PIN,OUTPUT);
	pinMode(LED_FRONT_BLU,OUTPUT);
	pinMode(LED_FRONT_RED,OUTPUT);
	pinMode(LED_REAR_BLU,OUTPUT);
	pinMode(LED_REAR_RED,OUTPUT);
	pinMode(MTRDRV_IN2,OUTPUT);
	pinMode(MTRDRV_IN1,OUTPUT);
	
	// init variables
	SetRunMode(TRAIN_MODE_FORWARD);

        // init variables
        _accel = 3;
}

Train::~Train(){}

void Train::SetRunMode(TrainDirection direction) {
	_direction = direction;
	if(_direction == TRAIN_MODE_FORWARD) {
		digitalWrite(LED_FRONT_BLU, HIGH);
		digitalWrite(LED_FRONT_RED, LOW);
		digitalWrite(LED_REAR_BLU, LOW);
		digitalWrite(LED_REAR_RED, HIGH);
	}
	else if(_direction == TRAIN_MODE_BACKWARD) {
		digitalWrite(LED_FRONT_BLU, LOW);
		digitalWrite(LED_FRONT_RED, HIGH);
		digitalWrite(LED_REAR_BLU, HIGH);
		digitalWrite(LED_REAR_RED, LOW);
	}
	else {
		digitalWrite(LED_FRONT_BLU, LOW);
		digitalWrite(LED_FRONT_RED, LOW);
		digitalWrite(LED_REAR_BLU, LOW);
		digitalWrite(LED_REAR_RED, LOW);
	}
        Accel(_accel);
}

void Train::Accel(int accel) {
        int power = 0;
        MotorState state = MOTOR_STATE_FORWARD;
	if(4 <= accel && accel <= 7) {
                power = MOTOR_MAX/4 * (accel - 3);
                Serial1.print(power,DEC);
                Serial1.println();
		if(_direction == TRAIN_MODE_FORWARD) {
			// forward against standard
			state = MOTOR_STATE_FORWARD;
		}
		else if(_direction == TRAIN_MODE_BACKWARD) {
			// back against standard
			state = MOTOR_STATE_BACKWARD;
		}
	}
	else if(0 <= accel && accel <= 2) {
                power = MOTOR_MAX/3 * (3 - accel);
		// brake
		state = MOTOR_STATE_BRAKE;
	}
	else {
		// newtral
		state = MOTOR_STATE_NEUTRAL;
	}
        MotorControl(state, power);
        _accel = accel;
}

void Train::MotorControl(int state, int val){
  if(state == MOTOR_STATE_FORWARD) {
    Serial1.println("MOTOR_STATE_FORWARD");
    //HIGH,LOWでデジタル出力
    digitalWrite(MTRDRV_IN2,HIGH);
    digitalWrite(MTRDRV_IN1,LOW);
  }
  else if(state == MOTOR_STATE_BACKWARD) {
    Serial1.println("MOTOR_STATE_BACKWARD");
    digitalWrite(MTRDRV_IN2,LOW);
    digitalWrite(MTRDRV_IN1,HIGH);
  }
  else if(state == MOTOR_STATE_BRAKE) {
    Serial1.println("MOTOR_STATE_BRAKE");
    digitalWrite(MTRDRV_IN2,HIGH);
    digitalWrite(MTRDRV_IN1,HIGH);
  }
  else {
    Serial1.println("MOTOR_STATE_STOP");
    digitalWrite(MTRDRV_IN2,LOW);
    digitalWrite(MTRDRV_IN1,LOW);
    val = 0;
  }
  //valが大きいほど出力値も大きくなる
  analogWrite(MTRDRV_VRF,val); //出力値:1~255
}

IOscMessageParser *pPsr;
Train *pTrain;

void setup() {
  Serial1.begin(9600);
  pPsr = IOscMessageParser::Create();
  pTrain = new Train();
}

void loop(){
  if(SerialReceive() == HIGH) {
    Serial1.println("Message Proceeded");
  }
}

void printHex(char c) {
  char buf[4];
  itoa(c, buf, 16);
  if(buf[1]=='\0') {
    buf[2]='\0';
    buf[1]=buf[0];
    buf[0]='0';
  }
  Serial1.print(buf);
}

void LedControl(bool isOn) {
  if(isOn) {
    digitalWrite(LED_PIN,HIGH);
  }
  else {
    digitalWrite(LED_PIN,LOW);
  }
}

/**
 use param as void* for WA of issue 188
 */
void ProcessOscMessage(void* param) {
  OscMessage* pMsg = (OscMessage*)param;
  if(strcmp(pMsg->Address,OSC_ADDRESS_TRAIN_ACCL) == 0) {
    int val = 0;
    if(pMsg->ParamNum >= 1) {
      val = pMsg->Params[0].Value.IntValue;
      val = min(val,8);
      val = max(val,0);
      Serial1.print("pTrain->Accel:");
      Serial1.println(val);
      pTrain->Accel(val);
    }
    char buf[8];
    sprintf(buf,"val=%d",val);
    Serial1.println(buf);
  }
  else if(strcmp(pMsg->Address,OSC_ADDRESS_TRAIN_MODE) == 0) {
    if(pMsg->ParamNum >= 1) {
      switch(pMsg->Params[0].Value.IntValue) {
        case 1:
          pTrain->SetRunMode(TRAIN_MODE_FORWARD);
          Serial1.println("runmode:TRAIN_MODE_FORWARD");
          break;
        case 2:
          pTrain->SetRunMode(TRAIN_MODE_BACKWARD);
          Serial1.println("runmode:TRAIN_MODE_BACKWARD");
          break;
        default:
          pTrain->SetRunMode(TRAIN_MODE_OFF);
          Serial1.println("runmode:TRAIN_MODE_OFF");
          break;
      }
    }
  }
  else if(strcmp(pMsg->Address,OSC_ADDRESS_LED) == 0) {
    if(pMsg->ParamNum >= 1 && pMsg->Params[0].Value.IntValue != 0) {
      Serial1.println("LED on");
      LedControl(true);
    }
    else {
      Serial1.println("LED off");
      LedControl(false);
    }
  }
  else {
    Serial1.print("ununderstandable:");
    Serial1.println(pMsg->Address);
  }
}

int SerialReceive() {
  int ret = LOW;
  if(!Serial1.available()) {
    return ret;
  }
  
  char c = '\0';
  while(Serial1.available()) {
    c = Serial1.read();
    printHex(c);
    if(pPsr->Push(c)) {
      OscMessage msg;
      pPsr->Read(&msg);
      Serial1.println();
      Serial1.println(msg.Address);
      ProcessOscMessage(&msg);
    }
  }
  Serial1.println("--");
  Serial1.flush();
  return ret;
}



