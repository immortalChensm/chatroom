
#include "http.h"
#include "vendor/json/cJSON.h"
#include "app/httpControllers/users.c"
#include "app/httpControllers/groups.c"
#include "app/httpControllers/base.h"

#include "app/websocketControllers/users.c"

static void on_http_request(http_s *h);
static void on_http_upgrade(http_s *h, char *requested_protocol, size_t len);
static void ws_on_open(ws_s *ws);
static void ws_on_message(ws_s *ws, fio_str_info_s msg, uint8_t is_text);
static void ws_on_shutdown(ws_s *ws);
static void ws_on_close(intptr_t uuid, void *udata);
//static fio_str_info_s CHAT_CANNEL = {.data = "chat", .len = 4};
static FIOBJ HTTP_HEADER_SERVER;
static FIOBJ HTTP_VALUE_SERVER;
/* *****************************************************************************
Routing
***************************************************************************** */

typedef void (*fio_router_handler_fn)(request *);
//定义结构体的名称
#define FIO_SET_NAME fio_router
//定义结构体成员value的数据类型
#define FIO_SET_OBJ_TYPE fio_router_handler_fn
//定义结构体成员key数据类型
#define FIO_SET_KEY_TYPE fio_str_s
#define FIO_SET_KEY_COPY(dest, obj) fio_str_concat(&(dest), &(obj))
#define FIO_SET_KEY_DESTROY(obj) fio_str_free(&(obj))
#define FIO_SET_KEY_COMPARE(k1, k2) fio_str_iseq(&(k1), &k2)
#define FIO_INCLUDE_STR
#define FIO_STR_NO_REF
#include <fio.h>
/* the router is a simple hash map */
static fio_router_s http_routes;//定义新结构体名称

/* adds a route to our simple router */
static void route_add(char *path, void (*fio_router_handler_fn)(request *)) {
    /* add handler to the hash map */
    //赋值操作
    fio_str_s tmp = FIO_STR_INIT_STATIC(path);//作为key
    /* fio hash maps support up to 96 full collisions, we can use len as hash */
    fio_router_insert(&http_routes, fio_str_len(&tmp), tmp, fio_router_handler_fn, NULL);
}

/* routes a request to the correct handler */
static void route_perform(http_s *h) {
    /* add required Serevr header */
    http_set_header(h, HTTP_HEADER_SERVER, fiobj_dup(HTTP_VALUE_SERVER));
    /* collect path from hash map */
    FIOBJ request_uri = fiobj_str_tmp();
    fiobj_str_concat(request_uri,h->method);
    fiobj_str_concat(request_uri,h->path);
    fio_str_info_s tmp_i = fiobj_obj2cstr(request_uri);
    //赋值操作 FIO_STR_INIT_EXISTING =fio_str_s
    fio_str_s tmp = FIO_STR_INIT_EXISTING(tmp_i.data, tmp_i.len, 0);//作为key
    fio_router_handler_fn handler = fio_router_find(&http_routes, tmp_i.len, tmp);
    /* forward request or send error */
    request *req = malloc(sizeof(*req));
    *req = (request){.h=h};
    if (handler) {
        handler(req);
        return;
    }
    http_send_error(h, 404);
}

static void websocket_route_perform(ws_s *ws, fio_str_info_s msg, uint8_t is_text) {

    cJSON *data;
    data=cJSON_Parse(msg.data);
    if (!data) {printf("websocket data format is error: [%s]\n",cJSON_GetErrorPtr());}
    else
    {
        cJSON *uri = cJSON_GetObjectItem(data,"uri");
        if (uri->valuestring){
            FIOBJ request_uri = fiobj_str_new(uri->valuestring,strlen(uri->valuestring));
            fio_str_info_s tmp_i = fiobj_obj2cstr(request_uri);
            fio_str_s tmp = FIO_STR_INIT_EXISTING(tmp_i.data, tmp_i.len, 0);
            fio_router_handler_fn handler = fio_router_find(&http_routes, tmp_i.len, tmp);
            /* forward request or send error */
            request *req = malloc(sizeof(*req));
            *req = (request){.ws=ws,.is_text=is_text,.msg=msg};
            if (handler) {
                handler(req);
                return;
            }
            websocket_route_not_found(ws);
        }else{
            websocket_route_not_found(ws);
        }
    }


}
/* cleanup for our router */
static void route_clear(void) { fio_router_free(&http_routes); }

/* *****************************************************************************
Cleanup
***************************************************************************** */

/* cleanup any leftovers */
static void cleanup(void) {

    fiobj_free(HTTP_HEADER_SERVER);
    fiobj_free(HTTP_VALUE_SERVER);
//    fiobj_free(JSON_KEY);
//    fiobj_free(JSON_VALUE);

    route_clear();
}

FIOBJ client;

typedef void (*handler)(http_s *);
static void config_init()
{
    FILE *f;long len;char *data;
    char *route_file = "../config/http_route.json";
    if(access(route_file,F_OK)==-1){
        fprintf(stderr,"http_route.json is not existing,please create it and add some route^_^\nfrom 勺颠颠");
        exit(0);
    }
    f=fopen(route_file,"rb");
    if (f){
        fseek(f,0,SEEK_END);
        len=ftell(f);
        fseek(f,0,SEEK_SET);
        data=(char*)malloc(len+1);
        fread(data,1,len,f);
        if (data){
            //http_route=cJSON_Parse(data);
            //if (!http_route) {printf("Error before: [%s]\n",cJSON_GetErrorPtr());}
        }
    }else{
        fprintf(stderr,"http_route.json can not read,please create it and add some route^_^\nfrom 勺颠颠");
        exit(0);
    }

    fclose(f);
}
static void http_router_init()
{

    route_add("GET/api/users/index",indexAction);
    route_add("GET/api/git/login",gitLoginAction);
    route_add("POST/api/users/test",testAction);
    route_add("POST/api/users/list",getAllUsers);
//    route_add("POST/api/group/index",groupsAction);
//    route_add("POST/api/group/add",addGroupAction);

}
static void websocket_router_init()
{
    route_add("websocket/users/recent",recentTalkHandler);
    route_add("websocket/users/talk",talkHandler);
    route_add("websocket/users/heartbeat",heartBeatHandler);
}


int main(int argc, char const *argv[]) {

    http_router_init();
    websocket_router_init();
    websocket_optimize4broadcasts(WEBSOCKET_OPTIMIZE_PUBSUB, 1);
    http_listen("2347", NULL,
                .on_request = on_http_request,
                .on_upgrade = on_http_upgrade,
                .max_clients=10240,
                .log = 0,
                .public_folder="../web"
    );
    fio_start(.threads = 1, .workers = 1);

    return 0;
}


static void on_http_request(http_s *h) {
    route_perform(h);
}


static void on_http_upgrade(http_s *h, char *requested_protocol, size_t len) {
    FIOBJ nickname;
    http_upgrade2ws(h, .on_message = ws_on_message, .on_open = ws_on_open,
                    .on_shutdown = ws_on_shutdown, .on_close = ws_on_close,
                    .udata = (void *)nickname);

}

static void ws_on_message(ws_s *ws, fio_str_info_s msg, uint8_t is_text) {

    websocket_route_perform(ws,msg,is_text);
}
static void ws_on_open(ws_s *ws) {

            websocket_subscribe(ws, .channel = CHAT_CANNEL);
//
//            FIOBJ tmp = fiobj_str_copy((FIOBJ)websocket_udata_get(ws));
//            fiobj_str_write(tmp, "welcome.", 16);
//            fio_publish(.channel = CHAT_CANNEL, .message = fiobj_obj2cstr(tmp));

//            websocket_write(
//            ws, (fio_str_info_s){.data = "welcome", .len = 30},
//            1);
    //websocket_response("欢迎光临",5,"请求成功",ws);

    websocket_publish_response("有个吊毛进聊天室了，你们准备好攻击",403,"");
}
static void ws_on_shutdown(ws_s *ws) {
//    websocket_write(
//            ws, (fio_str_info_s){.data = "Server shutting down, goodbye.", .len = 30},
//            1);

    websocket_response("服务器大哥睡觉了...",402,"请求成功",ws);
}

static void ws_on_close(intptr_t uuid, void *udata) {
    //fiobj_str_write((FIOBJ)udata, " left the chat.", 15);
    //fio_publish(.channel = CHAT_CANNEL, .message = fiobj_obj2cstr((FIOBJ)udata));
    websocket_publish_response("有个吊毛不爽的离开了聊天室还装逼的走了",404,"");
}


