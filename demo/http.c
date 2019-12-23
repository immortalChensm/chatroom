//
// Created by 1655664358@qq.com on 2019/12/3.
//

#include "http.h"

void on_request(http_s *request);

FIOBJ HTTP_HEADER_X_DATA;

int main(void) {

    HTTP_HEADER_X_DATA = fiobj_str_new("X-Data", 6);

    http_listen("2347", NULL, .on_request = on_request, .max_clients=10240,.log = 1,.public_folder="../web");

    fio_start(.threads = 2,.workers=3);

    fiobj_free(HTTP_HEADER_X_DATA);
}


void on_request(http_s *request) {


    http_set_cookie(request, .name = "my_cookie", .name_len = 9, .value = "data",
    .value_len = 4);
    http_set_header(request, HTTP_HEADER_CONTENT_TYPE,
                    http_mimetype_find("html", 4));
    http_set_header(request, HTTP_HEADER_X_DATA, fiobj_str_new("my data", 7));
    const char *word = "你好中国";
    FIOBJ content = fiobj_str_tmp();
    size_t len = fiobj_str_write(content,word, strlen(word));
    http_send_body(request, fiobj_obj2cstr(content).data,fiobj_obj2cstr(content).len);
}