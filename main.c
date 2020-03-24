#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#define I2CBUS "/dev/i2c-0"

void get_i2cbus(char *i2cbus, char *i2caddr);
int readOneByte(int in_adr);
int detectMagnet();
int getMagnetStrength();
uint16_t readTwoBytes(int in_adr_hi, int in_adr_lo);
float getAngle();

int i2cfd;
int _stat = 0x0b;
int _raw_ang_hi = 0x0c;
int _raw_ang_lo = 0x0d;

int main(int argc, char *argv[]){
  char senaddr[256] = "0x36";
  char i2c_bus[256] = I2CBUS;

  get_i2cbus(i2c_bus, senaddr);

  while(1){
    if(detectMagnet() == 1){
      //printf("Detect magnet, current magnitude: %d.\n", getMagnetStrength());
      printf("Encoder Angle: %f\n", getAngle());
    }
    else{
      printf("Can not detect magnet\n");
    }
  }
}

void get_i2cbus(char *i2cbus, char *i2caddr){
   if((i2cfd = open(i2cbus, O_RDWR)) < 0) {
      printf("Error failed to open I2C bus [%s].\n", i2cbus);
      exit(-1);
   }
   /* set I2C device */
   int addr = (int)strtol(i2caddr, NULL, 16);

   if(ioctl(i2cfd, I2C_SLAVE, addr) != 0) {
      printf("Error can't find sensor at address [0x%02X].\n", addr);
      exit(-1);
   }
   /* I2C communication test */
   char reg = 0x00;
   if(write(i2cfd, &reg, 1) != 1) {
      printf("Error: I2C write failure register [0x%02X], sensor addr [0x%02X]?\n", reg, addr);
      exit(-1);
   }
}

int readOneByte(int in_adr){
  int retVal = -1;
  if(write(i2cfd, &in_adr, 1) != 1) {
      printf("Error: Request one byte\n");
      return(-1);
    }
  if(read(i2cfd, &retVal, 1) != 1) {
      printf("Error: Receive one byte\n");
      return(-1);
    }
  return retVal;
}

int detectMagnet(){
  int magStatus;
  int retVal = 0;

  magStatus = readOneByte(_stat);
  if(magStatus & 0x20)
    retVal = 1; 
  
  return retVal;
}

int getMagnetStrength(){
  int magStatus;
  int retVal = 0;
  /*0 0 MD ML MH 0 0 0*/
  /* MD high = AGC minimum overflow, Magnet to strong */
  /* ML high = AGC Maximum overflow, magnet to weak*/ 
  /* MH high = magnet detected*/ 
  magStatus = readOneByte(_stat);
  if(detectMagnet() ==1){
      retVal = 2; /*just right */
      if(magStatus & 0x10)
        retVal = 1; /*to weak */
      else if(magStatus & 0x08)
        retVal = 3; /*to strong */
  }
  
  return retVal;
}

uint16_t readTwoBytes(int in_adr_hi, int in_adr_lo){
  uint16_t retVal = -1;

  /* read low byte */
  uint8_t low = readOneByte(in_adr_lo);

  /* read high byte */
  uint16_t high = readOneByte(in_adr_hi);
  high = high << 8;

  retVal = high | low;

  return retVal;
}

float getAngle(){
  uint16_t rawAngle = readTwoBytes(_raw_ang_hi, _raw_ang_lo);
  float angle = rawAngle * 0.087;
  return angle;
}