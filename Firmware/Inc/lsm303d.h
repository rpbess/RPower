#ifndef __LSM303_H_
#define __LSM303_H_

#define LSM_ADDRESS  0x3A

typedef struct
{
		int X;
		int Y;
		int Z;
} LsmDataTypeDef;

LsmDataTypeDef lsm_read_acc(void);
LsmDataTypeDef lsm_read_mag(void);
char lsm_present(void);
void lsm_init(void);
#endif /* __LSM303_H_ */
