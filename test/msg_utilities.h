/*************************************************************************
 * Intracom Telecom
 * Project: Control Plane Software (CPS) framework
 * Sub-project: CPS library
 * Filename: msg_utilities.h
 * Description: Message API
 ************************************************************************/


/*******************************************************************/
/*     Below message should be encoded and then decoded            */
/*                                                                 */
/* 0                   1                   2                   3   */
/* 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 */
/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/* |                          Length (52)                        | */
/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/* | Type (RTM_NEWQDISC)           | Flags (NLM_F_EXCL |         | */
/* |                               |NLM_F_CREATE | NLM_F_REQUEST)| */
/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/* |                      Sequence Number(arbitrary number)      | */
/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/* |                        Process ID (0)                       | */
/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/* |Family(AF_INET)|  Reserved1    |         Reserved1           | */
/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/* |                     Interface Index  (4)                    | */
/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/* |                      Qdisc handle  (0x1000001)              | */
/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/* |                     Parent Qdisc   (0x1000000)              | */
/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/* |                        TCM Info  (0)                        | */
/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/* |            Type (TCA_KIND)   |           Length(4)          | */
/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/* |                        Value ("pfifo")                      | */
/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/* |            Type (TCA_OPTIONS) |          Length(4)          | */
/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/* |                        Value (limit=100)                    | */
/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*******************************************************************/

#include "ser_des_api.h"
#define PFIFO 13
#define RTM_NEWQDISC 2
#define NLM_F_EXCL 3
#define NLM_F_CREATE 4
#define NLM_F_REQUEST 5
#define TCA_KIND 123
#define TCA_OPTIONS 124

/* encode a specific message by using serialization functions */
void encode(ser_buf_t **buf);

/* decode a specific message by using deserialization functions */
void decode(char *data);

/* decode TLV */
void tlvdecode(void *tlvtype, ser_buf_t **buf, int *length);
