//
// Created by 1655664358@qq.com on 2019/12/12.
//

#include "fio.h"
#include "fio_tls.h"


#define MAX_BYTES_READ_PER_CYCLE 4096


/* *****************************************************************************
TCP/IP / Unix Socket Client
***************************************************************************** */

static void on_data(intptr_t uuid, fio_protocol_s *protocol) {
    ssize_t ret = 0;
    char buffer[MAX_BYTES_READ_PER_CYCLE + 1];
    ret = fio_read(uuid, buffer, MAX_BYTES_READ_PER_CYCLE);
    while (ret > 0) {
        FIO_LOG_DEBUG("Recieved %zu bytes", ret);
        buffer[ret] = 0;
        fwrite(buffer, ret, 1, stdout); /* NUL bytes on binary streams are normal */
        fflush(stdout);
        ret = fio_read(uuid, buffer, MAX_BYTES_READ_PER_CYCLE);
    }
    //fio_stop();
    fio_close(uuid);
    (void)protocol; /* we ignore the protocol object, we don't use it */
}

/* Called during server shutdown */
static uint8_t on_shutdown(intptr_t uuid, fio_protocol_s *protocol) {
    FIO_LOG_INFO("Disconnecting.\n");
    /* don't print a message on protocol closure */
    protocol->on_close = NULL;
    return 0;   /* close immediately, don't wait */
    (void)uuid; /*we ignore the uuid object, we don't use it*/
}

/** Called when the connection was closed, but will not run concurrently */
static void on_close(intptr_t uuid, fio_protocol_s *protocol) {
    FIO_LOG_INFO("Remote connection lost.\n");
    //kill(0, SIGINT); /* signal facil.io to stop */
    fio_close(uuid);
    (void)protocol;  /* we ignore the protocol object, we don't use it */
    (void)uuid;      /* we ignore the uuid object, we don't use it */
}

/** Timeout handling. To ignore timeouts, we constantly "touch" the socket */
static void ping(intptr_t uuid, fio_protocol_s *protocol) {
    fio_touch(uuid);
    (void)protocol; /* we ignore the protocol object, we don't use it */
}

static fio_protocol_s client_protocol = {
        .on_data = on_data,
        .on_shutdown = on_shutdown,
        .on_close = on_close,
        //.ping = ping,
};


static void on_connect(intptr_t uuid, void *udata) {
    if (udata) // TLS support, udata is the TLS context.
        fio_tls_connect(uuid, udata, NULL);

    fio_attach(uuid, &client_protocol);
    fio_write(uuid,"GET / HTTP/1.1\n",16);
    (void)udata; /* we ignore the udata pointer, we don't use it here */
}

static void on_fail(intptr_t uuid, void *udata) {
    FIO_LOG_ERROR("Connection failed\n");
    //kill(0, SIGINT); /* signal facil.io to stop */
    fio_close(uuid);
    (void)uuid;      /* we ignore the uuid object, we don't use it */
    (void)udata;     /* we ignore the udata object, we don't use it */
}

void tcp_tls_client(char *address,char *port,int tls_flag) {

    intptr_t uuid;
    fio_tls_s *tls = NULL;
    if (tls_flag) {
        /* Manage TLS */
        tls = fio_tls_new(NULL, NULL, NULL, NULL);
        uuid =
                fio_connect(.address = address, .port = port,
                            .on_connect = on_connect, .on_fail = on_fail, .udata = tls);
    }else{
        uuid =
                fio_connect(.address = address, .port = port,
                            .on_connect = on_connect, .on_fail = on_fail);
    }

    if (uuid == -1)
        FIO_LOG_ERROR("Connection can't be established");
    else
        //fio_start(.threads = 1);
    if (tls_flag&&tls){
        fio_tls_destroy(tls);
    }

}
