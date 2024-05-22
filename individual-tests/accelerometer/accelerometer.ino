#include <Wire.h>
#include <MechaQMC5883.h>	

MechaQMC5883 qmc;

float RateRoll, RatePitch, RateYaw;
float RateCalibrationRoll, RateCalibrationPitch, RateCalibrationYaw;
int RateCalibrationNumber;
int AccCalibrationNumber;
float AccX, AccY, AccZ;
float AccXCalibration, AccYCalibration, AccZCalibration;
float AngleRoll, AnglePitch, AngleYaw;
float elapsedTime, currentTime, previousTime;
uint32_t LoopTimer;
float KalmanAngleRoll=0, KalmanUncertaintyAngleRoll=4*4;
float KalmanAnglePitch=0, KalmanUncertaintyAnglePitch=4*4;
float KalmanAngleYaw=-60, KalmanUncertaintyAngleYaw=4*4;
float Kalman1DOutput[]={0,0};
int mx_bias=-287;
int my_bias=239;
int mz_bias=275;

float Yaw;
bool calibrated = false;

void kalman_1d(float KalmanState, float KalmanUncertainty, float KalmanInput, float KalmanMeasurement) {
  KalmanState=KalmanState+0.004*KalmanInput;
  KalmanUncertainty=KalmanUncertainty + 0.004 * 0.004 * 4 * 4;
  float KalmanGain=KalmanUncertainty * 1/(1*KalmanUncertainty + 3 * 3);
  KalmanState=KalmanState+KalmanGain * (KalmanMeasurement-KalmanState);
  KalmanUncertainty=(1-KalmanGain) * KalmanUncertainty;
  Kalman1DOutput[0]=KalmanState; 
  Kalman1DOutput[1]=KalmanUncertainty;
}
void gyro_signals(void) {
  Wire.beginTransmission(0x68);
  Wire.write(0x1A);
  Wire.write(0x05);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x1C);
  Wire.write(0x10);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(); 
  Wire.requestFrom(0x68,6);
  int16_t AccXLSB = Wire.read() << 8 | Wire.read();
  int16_t AccYLSB = Wire.read() << 8 | Wire.read();
  int16_t AccZLSB = Wire.read() << 8 | Wire.read();
  Wire.beginTransmission(0x68);
  Wire.write(0x1B); 
  Wire.write(0x8);
  Wire.endTransmission();     
  Wire.beginTransmission(0x68);
  Wire.write(0x43);
  Wire.endTransmission();
  Wire.requestFrom(0x68,6);
  int16_t GyroX=Wire.read()<<8 | Wire.read();
  int16_t GyroY=Wire.read()<<8 | Wire.read();
  int16_t GyroZ=Wire.read()<<8 | Wire.read();
  RateRoll=(float)GyroX/65.5;
  RatePitch=(float)GyroY/65.5;
  RateYaw=(float)GyroZ/65.5;
  AccX=(float)AccXLSB/4096;//+0.01;
  AccY=(float)AccYLSB/4096;//-0.01;
  AccZ=(float)AccZLSB/4096;//-0.12;
  AngleRoll=atan(AccY/sqrt(AccX*AccX+AccZ*AccZ))*1/(3.142/180);
  AnglePitch=-atan(AccX/sqrt(AccY*AccY+AccZ*AccZ))*1/(3.142/180);
}

void gyro_calibration(void){
  for (RateCalibrationNumber=0; RateCalibrationNumber<4000; RateCalibrationNumber ++) {
    gyro_signals();
    RateCalibrationRoll+=RateRoll;
    RateCalibrationPitch+=RatePitch;
    RateCalibrationYaw+=RateYaw;
    AccXCalibration+=AccX;
    AccYCalibration+=AccY;
    AccZCalibration+=AccZ;
    delay(1);
  }
  RateCalibrationRoll/=4000;
  RateCalibrationPitch/=4000;
  RateCalibrationYaw/=4000;
  AccXCalibration/=4000;
  AccYCalibration/=4000;
  AccZCalibration/=4000;
}


void setup() {
  pinMode(4,OUTPUT);
  Serial.begin(115200);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  Wire.setClock(400000);
  Wire.begin();
  delay(250);
  Wire.beginTransmission(0x68); 
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();
  LoopTimer=micros();
}


void loop() {

  uint16_t dataSerial = 0;
  uint16_t dataValue = 0;
  uint8_t dataCode = 0;

  if (Serial.available() > 0){
    dataSerial = Serial.parseInt();  // Returns a Zero after reading
    if (Serial.read() == '\n'){} // kills 1 second wait
    // Works for 5 digit values. Can be used with String(value).length().
    if (dataSerial > 99){
      dataCode = dataSerial / 1000;
      dataValue = dataSerial % 1000;
    } else {
      dataCode = dataSerial;
    }
  }

  switch(dataCode){
    case 11:
      gyro_calibration();
      calibrated = true;
    default:
      gyro_signals();
      int mx,my,mz,acimut;
      qmc.read(&mx,&my,&mz,&acimut);
      mx -= mx_bias;
      my -= my_bias;
      mz -= mz_bias;
      AngleYaw = atan2(my,mx)*(180/3.1415);
      if (calibrated){
        RateRoll-=RateCalibrationRoll;
        RatePitch-=RateCalibrationPitch;
        RateYaw-=RateCalibrationYaw;
        AccX-=AccXCalibration;
        AccY-=AccYCalibration;
        AccZ = AccZ - AccZCalibration +1;
      }
      kalman_1d(KalmanAngleRoll, KalmanUncertaintyAngleRoll, RateRoll, AngleRoll);
      KalmanAngleRoll=Kalman1DOutput[0]; 
      KalmanUncertaintyAngleRoll=Kalman1DOutput[1];
      kalman_1d(KalmanAnglePitch, KalmanUncertaintyAnglePitch, RatePitch, AnglePitch);
      KalmanAnglePitch=Kalman1DOutput[0]; 
      KalmanUncertaintyAnglePitch=Kalman1DOutput[1];
      kalman_1d(KalmanAngleYaw, KalmanUncertaintyAngleYaw, RateYaw, AngleYaw);
      KalmanAngleYaw=Kalman1DOutput[0]; 
      KalmanUncertaintyAngleYaw=Kalman1DOutput[1];
      //Serial.print("Roll [deg]: "); 
      Serial.print(KalmanAngleRoll);Serial.print(",");
      //Serial.print("Pitch [deg]: "); 
      Serial.print(KalmanAnglePitch);Serial.print(",");
      //Serial.print("Yaw [deg]: "); 
      Serial.println(KalmanAngleYaw);//Serial.println(",");


      while (micros() - LoopTimer < 4000);
      LoopTimer=micros();
  }
}

float degTorad(float angle){
  float rad_angle = angle * (3.1415/180);
  return rad_angle;
}
