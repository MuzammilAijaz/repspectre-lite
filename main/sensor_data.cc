#include "sensor_data.h"
#include "tensorflow/lite/micro/micro_log.h"

// ------------------ MPU ------------------
Quaternion q {};           // [w, x, y, z]         quaternion container
VectorFloat gravity {};    // [x, y, z]            gravity vector
float ypr[3]{};           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector
uint16_t packetSize = 42;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount = 0;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
uint8_t mpuIntStatus = 0;   // holds actual interrupt status byte from MPU
			
// ------------------ MPU data ------------------
float DELTA_T = 1.0f / 200.0f; // this is the default set by the library // TODO : make it more rigid
float velocity[3] = {0.0f, 0.0f, 0.0f}; // xyz // ASSUMPTION : motion starts from rest // TODO : add detection method
float position[3] = {0.0f, 0.0f, 0.0f};
	    
// ------------------ Private Functions Declarations ------------------
void vAFunction(void);

// =====================================================================================
// |                                Reading Data From Sensor
// -------------------------------------------------------------------------------------

void readSensor() {
    mpuIntStatus = mpu.getIntStatus();
    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    // ---- IF the FIFO is full ----
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
      // reset so we can continue cleanly
      mpu.resetFIFO();
    }

    // ---- IF FIFO not full ----
    // otherwise, check for DMP data ready interrupt frequently)
    else if (mpuIntStatus & 0x02) {

      // wait for correct available data length, should be a VERY short wait
      while (fifoCount < packetSize) { 
          fifoCount = mpu.getFIFOCount(); 
          // printf("FIFO count: %d\n", fifoCount);
      }

      // ---- read a packet from FIFO ----
      mpu.getFIFOBytes(fifoBuffer, packetSize);
      mpu.dmpGetQuaternion(&q, fifoBuffer);
      mpu.dmpGetGravity(&gravity, &q);
    }
}

SensorData getQuaternion() {
    // SINCE QUARTERNION IS ALREADY UPDATED INSIDE THE readSensor() function
    // Quaternion stores in : W, X, Y, Z
    // we need in : X, Y, Z, W
    return {
      q.x,
      q.y,
      q.z,
      q.w
    };
}

SensorData getEulerAngles() {
    // Function converts Quaternion into euler angles
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    // printf("%3.1f %3.1f %3.1f\n", ypr[0] * 180/M_PI, ypr[1] * 180/M_PI, ypr[2] * 180/M_PI);
    return {
	static_cast<float>(ypr[1]*180/M_PI), // X - Pitch
	static_cast<float>(ypr[2]*180/M_PI), // Y - Roll
	static_cast<float>(ypr[0]*180/M_PI), // Z - Yaw
        0.0f,
    };
}

SensorData getLinearAccel() {
    // - rate at which the velocity of an object changes in a straight line	    
    // - different from angular velocity which describes rotational motion
    
    VectorInt16 accelRaw;
    VectorInt16 linearAccel;
    VectorInt16 linearAccelInWorld;
    
    // 1. get raw accel, from sensor frame( includes gravity )
    // note : values given are integers ; requires division by LSB, to convert
    mpu.dmpGetAccel(&accelRaw, fifoBuffer); 
    // 2. remove gravity vector from raw accel
    mpu.dmpGetLinearAccel(&linearAccel, &accelRaw, &gravity); 
    // 3. rotate into earths frame
    mpu.dmpGetLinearAccelInWorld(&linearAccelInWorld, &linearAccel, &q); 

    // TODO : get temperature details as well (because readings change with temperature)
    // printf("%3.4f %3.4f %3.4f\n", linearAccelInWorld.x / ACCEL_LSB, linearAccelInWorld.y / ACCEL_LSB, linearAccelInWorld.z / ACCEL_LSB);
    return {linearAccelInWorld.x / ACCEL_LSB, linearAccelInWorld.y / ACCEL_LSB, linearAccelInWorld.z / ACCEL_LSB, 0.0f};
}

SensorData getRawAccel() {
    VectorInt16 accelRaw;
    mpu.dmpGetAccel(&accelRaw, fifoBuffer); 
    return {accelRaw.x / ACCEL_LSB, accelRaw.y / ACCEL_LSB, accelRaw.z / ACCEL_LSB, 0.0f};
}

int printTrue = 1;
float accel[3] = {0.0};
int AVERAGE_COUNT=5;

SensorData getVelocity(SensorData linearAccelInWorld) {
    // Output is linear acceleration
    if (printTrue == AVERAGE_COUNT) {
	for (int i =0;i<3;i++) {accel[i] /= AVERAGE_COUNT;}

	for (int i =0; i<3; i++) {
	    velocity[i] = velocity[i] + (DELTA_T * accel[i]);
	}

	for (int i =0; i<3; i++) {
	    position[i] = position[i] + (DELTA_T * velocity[i]);
	}

	// Output is integrated velocity
	// printf("accel: %3.3f %3.3f %3.3f\n", accel[0], accel[1], accel[2]);
	// printf("velocity: %3.3f %3.3f %3.3f\n", velocity[0], velocity[1], velocity[2]); 
	// printf("position: %3.3f %3.3f %3.3f\n", position[0], position[1], position[2]);

	for(int i=0; i<3;i++) accel[i] = 0; // reset for next iteration
      printTrue = 0;

    } else {
      accel[0] += (float)linearAccelInWorld.x / ACCEL_LSB;
      accel[1] += (float)linearAccelInWorld.y / ACCEL_LSB;
      accel[2] += (float)linearAccelInWorld.z / ACCEL_LSB;

      printTrue++;
    }

    return {accel[0], accel[1], accel[2], 0.0f};
}

// =====================================================================================
// |                                 Private Functions
// -------------------------------------------------------------------------------------

// ----- DEBUG FUNCTIONALITY ------------------------------------
// Tests the MAX amount of RAM a function has used.
void vAFunction( void )
{
    TaskHandle_t xHandle;
    TaskStatus_t xTaskDetails;

    /* Obtain the handle of a task from its name. */
    xHandle = xTaskGetCurrentTaskHandle();

    /* Check the handle is not NULL. */
    configASSERT( xHandle );

    /* Use the handle to obtain further information about the task. */
    vTaskGetInfo( /* The handle of the task being queried. */
                  xHandle,
                  /* The TaskStatus_t structure to complete with information
                     on xTask. */
                  &xTaskDetails,
                  /* Include the stack high water mark value in the
                     TaskStatus_t structure. */
                  pdTRUE,
                  /* Include the task state in the TaskStatus_t structure. */
                  eInvalid );
    
    // configSTACK_DEPTH_TYPE is uint16_t
    configSTACK_DEPTH_TYPE minStackLeft = xTaskDetails.usStackHighWaterMark;
    printf("Min Stack Left: %ld\n", minStackLeft);
}

// =====================================================================================
// |                                 Public Functions/ API
// =====================================================================================

SensorData get_sensor_data(SensorValue val) {
    readSensor(); // update the sensor readings
    
    SensorData data = {};
    switch (val) {
      case SensorValue::ACCELERATION:
          data = getLinearAccel();
          break;
      case SensorValue::RAW_ACCELERATION:
          data = getRawAccel();
          break;
      case SensorValue::VELOCITY:
          data = getLinearAccel();
          data = getVelocity(data);
          break;
      case SensorValue::ANGLE:
          data = getEulerAngles();
          break;
      case SensorValue::QUATERNION:
          data = getQuaternion();
          break;
      default:
          break;
      };

    return data;
}

