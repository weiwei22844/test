#include "../zmqDemo/include/zmq.h"
#include "../zmqDemo/include/zmq_utils.h"
#include <string.h>
#include <stdio.h>
#include "CommLinphone.h"

int main () {
    int message_size;
    int message_count = 20;
    void *s;
    void *ctx;
    int rc;
    int i;
    zmq_msg_t msg;
    BellPkg bellpkg;

    ctx = zmq_init(1);
    if(!ctx){
        printf ("error in zmq_init: %s\n", zmq_strerror (errno));
        return -1;
    }

    s = zmq_socket (ctx, ZMQ_PUSH);
    if (!s) {
        printf ("error in zmq_socket: %s\n", zmq_strerror (errno));
        return -1;
    }

    //rc = zmq_connect (s, "tcp://192.168.1.222:5555");
    rc = zmq_connect (s, "tcp://192.168.1.173:5555");
    if (rc != 0) {
        printf ("error in zmq_connect: %s\n", zmq_strerror (errno));
        return -1;
    }

    bellpkg.header.ucVersion = 0;
    bellpkg.header.ucCode = 1;
    bellpkg.header.HdLen = sizeof(BellPkgHead);
    bellpkg.header.Reserved = 0;
    bellpkg.header.uiTotalLen = sizeof(BellPkgHead)+sizeof(TLV);
    bellpkg.tlvs->type = 3;
    bellpkg.tlvs->len = 1;
    bellpkg.tlvs->value[0] = 'A';
    message_size = bellpkg.header.uiTotalLen;
    for (i = 0; i != message_count; i++) {
        rc = zmq_msg_init_size (&msg, message_size);
        if (rc != 0) {
            printf ("error in zmq_msg_init_size: %s\n", zmq_strerror (errno));
            return -1;
        }

        printf("will send msg%d\n", i);
        memcpy (zmq_msg_data (&msg), &bellpkg, sizeof(bellpkg));

        rc = zmq_sendmsg (s, &msg, 0);
        if (rc < 0) {
            printf ("error in zmq_sendmsg: %s\n", zmq_strerror (errno));
            return -1;
        }
        rc = zmq_msg_close (&msg);
        if (rc != 0) {
            printf ("error in zmq_msg_close: %s\n", zmq_strerror (errno));
            return -1;
        }
        bellpkg.tlvs->value[0] += 1;
    }

    printf("client will exit\n");
    rc = zmq_close (s);
    if (rc != 0) {
        printf ("error in zmq_close: %s\n", zmq_strerror (errno));
        return -1;
    }

    printf("client will exit2\n");
    rc = zmq_term (ctx);
    if (rc != 0) {
        printf ("error in zmq_term: %s\n", zmq_strerror (errno));
        return -1;
    }
    printf("client will exit3\n");

    return 0;
}
