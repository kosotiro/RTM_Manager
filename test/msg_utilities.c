/*************************************************************************
 * Intracom Telecom
 * Project: Control Plane Software (CPS) framework
 * Sub-project: CPS library
 * Filename: msg_utilities.c
 * Description: Message API
 ************************************************************************/

#include "msg_utilities.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


void encode(ser_buf_t **buf) {
   
   unsigned int length, seqn, pid, reserved, ifIndex, Qdisc, PQdisc, TCMinfo, value;
   short mes_type, flags, tlv_type, tlv_len;
   char family;

   init_ser_buf(buf);

   length = 52;
   ser_data(buf, &length, 4);   /* serialize message length */
   mes_type = RTM_NEWQDISC;
   ser_data(buf, &mes_type, 2); /* serialize message type */
   flags = NLM_F_EXCL | NLM_F_CREATE | NLM_F_REQUEST;
   ser_data(buf, &flags, 2);    /* serialize flags */
   seqn = 1000;
   ser_data(buf, &seqn, 4);     /* serialize sequence number */
   pid = (unsigned int) getpid();
   ser_data(buf, &pid, 4);      /* serialize pid */
   family = AF_INET;
   ser_data(buf, &family, 1);   /* serialize process family */
   reserved = 0;
   ser_data(buf, &reserved, 3); /* serialize reserved field */
   ifIndex = 5;
   ser_data(buf, &ifIndex, 4);  /* serialize ifindex */
   Qdisc = 0x1000001;
   ser_data(buf, &Qdisc, 4);    /* serialize Qdisc */
   PQdisc = 0x1000000;
   ser_data(buf, &PQdisc, 4);   /* serialize PQdisc */
   TCMinfo = 0;
   ser_data(buf, &TCMinfo, 4);  /* serialize TCMinfo */
   tlv_type = TCA_KIND;
   ser_data(buf, &tlv_type, 2); /* serialize first TLV */
   tlv_len = 4;
   ser_data(buf, &tlv_len, 2);  /* serialize TLV length */
   value = PFIFO;
   ser_data(buf, &value, 4);    /* serialize TLV value */
   tlv_type = TCA_OPTIONS;
   ser_data(buf, &tlv_type, 2); /* serialize second TLV */
   ser_data(buf, &tlv_len, 2);  /* serialize TLV length */
   value = 100;
   ser_data(buf, &value, 4);    /* serialize TLV value */
}

void decode(char *data){

   ser_buf_t *buf;
   unsigned int length, pid, reserved, seqn, ifIndex, Qdisc, PQdisc, TCMinfo;
   short msg_type, flags, tlv_type;
   char family;
   
   buf =(ser_buf_t *)malloc(sizeof(ser_buf_t));
   buf->b = (char *)malloc(*((int *)data));
   buf->next = 0;
   buf->size = *((int *)data);

   if (buf != NULL) {
      memcpy(buf->b, data, *((int *)data));
      deser_data(&length, &buf, 4);
      printf("Length field of message is %d %s\n", length, (length == 52) ? "OK" : "NOK");
      length -= 4;

      while (length > 0) {
         deser_data(&msg_type, &buf, 2);
         printf("Message type field is:%d %s\n",msg_type, (msg_type == RTM_NEWQDISC) ? "OK" : "NOK");
         length -= 2;
         deser_data(&flags, &buf, 2);
         printf("Flags field is:%d %s\n",flags, (flags == (NLM_F_EXCL | NLM_F_CREATE | NLM_F_REQUEST)) ? "OK" : "NOK" );
         length -= 2;
         deser_data(&seqn, &buf, 4);
         printf("Sequence number field is:%d %s\n",seqn, (seqn == 1000) ? "OK" : "NOK");
         length -= 4;
         deser_data(&pid, &buf, 4);
         printf("Process id field is:%d\n",pid);
         length -= 4;
         family = 0;
         deser_data(&family, &buf, 1);
         printf("Family field is:%d %s\n", family, (family == AF_INET) ? "OK" : "NOK");
         length -= 1;
         reserved=0;
         deser_data(&reserved, &buf, 3);
         printf("Reserved1 field is:%d %s\n", reserved, (reserved == 0) ? "OK" : "NOK");
         length -= 3;
         deser_data(&ifIndex, &buf, 4);
         printf("Interface Index field is:%d %s\n", ifIndex, (ifIndex == 5) ? "OK" : "NOK");
         length -= 4;
         deser_data(&Qdisc, &buf, 4);
         printf("Qdisc field is:%x %s\n", Qdisc, (Qdisc == 0x1000001) ? "OK" : "NOK");
         length -= 4;
         deser_data(&PQdisc, &buf, 4);
         printf("Parent Qdisk field is:%x %s\n", PQdisc, (PQdisc == 0x1000000) ? "OK" : "NOK");
         length -= 4;
         deser_data(&TCMinfo, &buf, 4);
         printf("TCM info is:%d %s\n", TCMinfo, (TCMinfo == 0) ? "OK" : "NOK");
         length -= 4;
         deser_data(&tlv_type, &buf, 2);
         length -= 2;
         tlvdecode(&tlv_type, &buf, &length);
         deser_data(&tlv_type, &buf, 2);
         length -= 2;
         tlvdecode(&tlv_type, &buf, &length);
      }

      dealloc_ser_buf(&buf); 
   } 
}

void tlvdecode(void *tlvtype, ser_buf_t **buf, int *length) {

   short type,len;
   unsigned int value;

   type = *((short *) tlvtype);
   deser_data(&len, buf, 2);
   deser_data(&value, buf, len);
   *length -= 2;
   *length -= len;

   switch (type){
      case TCA_KIND: 
         printf("TLV type \"TCA_KIND\" with length %d and value %d is %s\n", len, value, ( value == PFIFO ) ? "OK" : "NOK");
         break;
      case TCA_OPTIONS:
         printf("TLV type \"TCA_OPTIONS\" with length %d and value %d is %s\n", len, value, ( value == 100 ) ? "OK" : "NOK");
         break;
      default:
         break;
   }     
}

/*
int main(int argc, char *argv[]){
   
   ser_buf_t *buf;
   int size;

   printf("AF_INET:%d\n",AF_INET);
   printf("unsigned int:%d\n",sizeof(unsigned int));

   encode(&buf);*/
   /* Debug */
/* printf("mes = %d\n",*((int *)buf->b));
   printf("size = %d\n", buf->next);
   decode(buf->b);

   dealloc_ser_buf(&buf);
}*/
