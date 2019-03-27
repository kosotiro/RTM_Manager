typedef enum {
   CREATE,
   UPDATE,
   DELETE,
   NOOPT} OPCODE;


typedef struct _data {
   char destination[16]; /* this is our key */
   short mask;
   char gateway_ip[16];
   char oif[32];
} data;


typedef struct sync_msg {
   OPCODE op_code;
   data msg_body;
} sync_msg_t;
