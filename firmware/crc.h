/******************************************************************************
* CRC Module                                                                  *
*                                                                             *
*                                                                             *
******************************************************************************/

/******************************************************************************
* CRC Module                                                                  *
*                                                                             *
* Contains functions and definitions required for RBDS CRC generation         *
*                                                                             *
*                                                                             *
* (uint16_t) uartInit(rbds_t, uint8_t) Function computes checksum of rbds     *
*                                      data structure with designated group   *
*                                      offset.                                *
*                                                                             *
******************************************************************************/

extern uint16_t crcChecksum(rbds_t *rbds, uint8_t offset);