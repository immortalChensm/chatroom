//
// Created by 1655664358@qq.com on 2019/12/11.
//

#ifndef _BASH_H
#define _BASH_H
#define CLIETN_ID "2ccb8fbd415ac3d38c84"
#define CLIENT_SECRET "642c845f8f405877a852dcd69129fa4ca828deab"
#include "http.h"
#include "fio.c"
#include <netdb.h>
#include <netinet/in.h>
#include <curl/curl.h>
static fio_str_info_s CHAT_CANNEL = {.data = "chat", .len = 4};
typedef struct request{
    http_s *h;
    ws_s *ws;
    fio_str_info_s msg;
    uint8_t is_text;
    char *data;
}request;

struct buffer_struct {
    char *memory;
    size_t size;
};
static void http_route_not_found(http_s *h)
{
    http_send_error(h,404);
}
static void websocket_route_not_found(ws_s *ws)
{
    websocket_write(
            ws, (fio_str_info_s){.data = "routeNotFound!", .len = 14},
            1);
}
static void websocket_response(char *data,int code,char *msg,ws_s *ws)
{
    cJSON *root;
    char *out;
    root=cJSON_CreateObject();
    cJSON_AddNumberToObject(root,"code",code);
    cJSON_AddStringToObject(root,"msg",msg);
    cJSON_AddStringToObject(root,"data",data);
    out=cJSON_Print(root);
    websocket_write(
            ws, (fio_str_info_s){.data = out, .len = strlen(out)},
            1);
    cJSON_Delete(root);
    free(out);

}

static void websocket_publish_response(char *data,int code,char *msg)
{
    cJSON *root;
    char *out;
    root=cJSON_CreateObject();
    cJSON_AddNumberToObject(root,"code",code);
    cJSON_AddStringToObject(root,"msg",msg);
    cJSON_AddStringToObject(root,"data",data);
    out=cJSON_Print(root);
    fio_publish(.channel = CHAT_CANNEL, .message = (fio_str_info_s){.data=out,.len=strlen(out),.capa=sizeof(out)},.is_json=1);
    cJSON_Delete(root);
    free(out);

}

static void http_response(http_s *h,int code,char *message)
{
    http_set_header(h,fiobj_str_new("Content-Type",12),fiobj_str_new("text/json",9));

    cJSON *root;
    char *out;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "code", code);
    cJSON_AddStringToObject(root, "message", message);
    out = cJSON_Print(root);
    http_send_body(h,out,strlen(out));
    http_finish(h);
    cJSON_Delete(root);
    free(out);
}

static void get_domain_ip(char *domain,char *ip)
{
    struct addrinfo *result;
    struct addrinfo *res;
    char buf[128];
    int error;
    error = getaddrinfo(domain, NULL, NULL, &result);
    if (error != 0)
    {
        fprintf(stderr, "error in getaddrinfo: %s/n", strerror(error));
    }
    inet_ntop(AF_INET, &(((struct sockaddr_in *)result->ai_addr)->sin_addr),buf, 128);
    strcpy(ip,buf);
}
static void create_tcp_client(char *host,int port,char *uri,char *method,char *data)
{
    char ip[128];
    get_domain_ip(host,ip);
    if (ip) {
        struct sockaddr_in client;
        bzero(&client, sizeof(client));
        client.sin_family = AF_INET;
        client.sin_port = htons(port);
        inet_pton(AF_INET, ip, &client.sin_addr);
        int sock = socket(PF_INET, SOCK_STREAM, 0);
        int keepalive = 1;
        if (connect(sock, (struct sockaddr *) &client, sizeof(client)) < 0) {
            printf("connect fail %d\n", errno);
        } else {
            size_t len;
            char request[1024 * 4]; char recvbuf[1024*4];
            len = sprintf(request, "%s /%s HTTP/1.1\r\n", method,uri);
            len += sprintf(request + len, "Accept-Language: zh-CN,zh;q=0.9\r\n");
            len += sprintf(request + len, "Accept: */*\r\n");
            len += sprintf(request + len, "Connection: keep-alive\r\n");
            len += sprintf(request + len,
                           "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/73.0.3683.86 Safari/537.36\r\n");
            if (!strcasecmp("POST",method)){
                len += sprintf(request + len, "Content-Length: %d\r\n", strlen(data));
                len += sprintf(request + len, "Content-Type:application/x-www-form-urlencoded\r\n");
                len += sprintf(request + len, "\r\n\r\n%s",data);
            }else{
                len += sprintf(request + len, "Content-Length: %d\r\n", 0);
                len += sprintf(request + len, "\r\n\r\n");
            }
            len = send(sock, request, len, 0);
            len = recv(sock, recvbuf, sizeof(recvbuf) - 1, 0);
            char *temp = recvbuf;

            printf("len=%d,recv=%s\n", len, recvbuf);
            close(sock);

        }
    }else{

    }

}

static size_t write_buffer_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct buffer_struct *mem = (struct buffer_struct *)userp;
    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}
static FIOBJ curl_request(char *uri,char *postdata)
{
    CURL *curl;CURLcode res;struct buffer_struct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    FIOBJ result = fiobj_hash_new();
    if (!curl) {
        fprintf(stderr,"curl init error\n");
        fiobj_hash_set(result,fiobj_str_new("code",4),fiobj_num_new(1));
        return result;
    }
    curl_easy_setopt(curl, CURLOPT_URL, uri);
    struct curl_slist *pList = NULL;
    pList = curl_slist_append(pList,"Accept:application/json, text/javascript, */*; q=0.01");
    pList = curl_slist_append(pList,"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/73.0.3683.86 Safari/537.36");
//    pList = curl_slist_append(pList,"Accept-Language:zh-CN,zh;q=0.8");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pList);

    if (NULL!=postdata) {
//        pList = curl_slist_append(pList,"Content-Type:application/x-www-form-urlencoded; charset=UTF-8");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) strlen(postdata));
        /* Set the expected POST data */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
    }
    //允许重定向
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_buffer_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &chunk);
    curl_easy_setopt (curl,CURLOPT_TIMEOUT, 600L);
    curl_easy_setopt (curl,CURLOPT_CONNECTTIMEOUT, 10L);
    res = curl_easy_perform(curl);
    if(res !=CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
        fiobj_hash_set(result,fiobj_str_new("code",4),fiobj_num_new(1));
        return result;
    }
    //printf("%s\n",chunk.memory);

    curl_easy_cleanup(curl);
    curl_global_cleanup();
   // free(chunk.memory);
   fiobj_hash_set(result,fiobj_str_new("code",4),fiobj_num_new(2));
   fiobj_hash_set(result,fiobj_str_new("data",4),fiobj_str_new(chunk.memory,chunk.size));

    //return chunk.memory;
    return result;

}


#endif