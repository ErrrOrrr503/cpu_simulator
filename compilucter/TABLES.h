#ifndef TABLES_H_INCLUDED
#define TABLES_H_INCLUDED

const size_t RAM_SIZE = 65536L;

const size_t NUM_SIZE = sizeof (int64_t);

typedef const uint8_t C_CMD_T;
typedef uint8_t CMD_T;

C_CMD_T PUSH = 0x0E;
C_CMD_T RAM_PUSH = 0xA0;
C_CMD_T RAM_POP = 0xA1;
C_CMD_T SET_RAM_ADDR = 0xA2;
C_CMD_T GET_RAM_ADDR = 0xA3;
C_CMD_T IN = 0x0D;
C_CMD_T OUT = 0x0C;
C_CMD_T POP = 0x1B;
C_CMD_T SUM = 0x10;
C_CMD_T SUB = 0x11;
C_CMD_T MUL = 0x12;
C_CMD_T DIV = 0x13;
C_CMD_T SQRT = 0x21;
C_CMD_T SQR = 0x22;
C_CMD_T IBZ_S = 0x5C;
C_CMD_T IEZ = 0x5E;
C_CMD_T IOZ_NS = 0x59;
C_CMD_T INEZ = 0x57;
C_CMD_T L0 = 0x5B;
C_CMD_T L1 = 0x5D;
C_CMD_T JMP = 0x5F;
C_CMD_T S_S = 0x50;
C_CMD_T CALL = 0x32;


typedef const uint8_t C_FLAG_T;
typedef uint8_t FLAG_T;

C_FLAG_T CMD = 0xFF;
C_FLAG_T INUM = 0xFE;
C_FLAG_T FNUM = 0xFC;
C_FLAG_T REG = 0xFD;
C_FLAG_T LABEL = 0xFA;
C_FLAG_T FUNCTION = 0xF0;
C_FLAG_T NOTHING = 0x00;

typedef const uint8_t C_REG_T;
typedef uint8_t REG_T;

C_REG_T X0 = 0x40;
C_REG_T X1 = 0x41;
C_REG_T X2 = 0x42;
C_REG_T X3 = 0x43;
C_REG_T X4 = 0x44;
C_REG_T X5 = 0x45;
C_REG_T X6 = 0x46;
C_REG_T X7 = 0x47;
C_REG_T X8 = 0x48;
C_REG_T X9 = 0x49;
C_REG_T XA = 0x4A;
C_REG_T XB = 0x4B;
C_REG_T XC = 0x4C;
C_REG_T XD = 0x4D;
C_REG_T XE = 0x4E;
C_REG_T XF = 0x4F;


#endif // TABLES_H_INCLUDED
