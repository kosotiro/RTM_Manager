/*************************************************************************
 * Intracom Telecom
 * Project: Control Plane Software (CPS) framework
 * Sub-project: CPS library
 * Filename: ser_des_api.h
 * Description: SERIALIZE AND DESERIALIZE API
 ************************************************************************/

#define MAX_BUF_SIZE 32

typedef struct serialize_buffer{
   char *b;  /* buffer data */
   int size; /* total size of buffer */
   int next; /* next available position to write data */
}ser_buf_t;

/* initialize the serial buffer data */
void init_ser_buf(ser_buf_t **buf);

/* serialize "data" of size "nbytes" */
void ser_data(ser_buf_t **buf, void *data, int nbytes);

/* deserializing data of size nbytes */
void deser_data(void *dest, ser_buf_t **buf, int nbytes);

/* deallocate data structure used for TLV */
void dealloc_ser_buf(ser_buf_t **buf);
