
#include "fio.h"
#include "http.h"


/* *****************************************************************************
The main function
***************************************************************************** */


static void on_http_request(http_s *h);

static void on_http_upgrade(http_s *h, char *requested_protocol, size_t len);

FIOBJ client;

int main(int argc, char const *argv[]) {

    client = fiobj_hash_new();
    http_listen("2347", NULL,
            .on_request = on_http_request,
            .on_upgrade = on_http_upgrade,
            .max_clients=10240,
            .log = 1,
            .public_folder="../web"
                    );
    fio_start(.threads = 1, .workers = 1);

    return 0;
}

/* *****************************************************************************
HTTP Request / Response Handling
***************************************************************************** */

static void on_http_request(http_s *h) {
    /* set a response and send it (finnish vs. destroy). */
    http_send_body(h, "Hello World!", 12);
}

/* *****************************************************************************
HTTP Upgrade Handling
***************************************************************************** */

/* WebSocket Handlers */
static void ws_on_open(ws_s *ws);
static void ws_on_message(ws_s *ws, fio_str_info_s msg, uint8_t is_text);
static void ws_on_shutdown(ws_s *ws);
static void ws_on_close(intptr_t uuid, void *udata);

/* HTTP upgrade callback */
static void on_http_upgrade(http_s *h, char *requested_protocol, size_t len) {
    /* Upgrade to SSE or WebSockets and set the request path as a nickname. */
    FIOBJ nickname;
    http_upgrade2ws(h, .on_message = ws_on_message, .on_open = ws_on_open,
                        .on_shutdown = ws_on_shutdown, .on_close = ws_on_close,
                        .udata = (void *)nickname);

}


/* *****************************************************************************
WebSockets Callbacks
***************************************************************************** */

static void ws_on_message(ws_s *ws, fio_str_info_s msg, uint8_t is_text) {

    printf("%s\n",msg.data);

    websocket_write(
            ws, (fio_str_info_s){.data = msg.data, .len = msg.len},
            1);
}
static void ws_on_open(ws_s *ws) {

    //if (fiobj_hash_haskey(client,));
    ws.
    websocket_write(
            ws, (fio_str_info_s){.data = "welcome", .len = 30},
            1);
}
static void ws_on_shutdown(ws_s *ws) {
    websocket_write(
            ws, (fio_str_info_s){.data = "Server shutting down, goodbye.", .len = 30},
            1);
}

static void ws_on_close(intptr_t uuid, void *udata) {
    printf("客户端已经关闭\n");
}


