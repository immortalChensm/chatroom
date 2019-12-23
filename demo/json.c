//
// Created by 1655664358@qq.com on 2019/12/3.
//

#include "http.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


void on_request(http_s *request);


FIOBJ HTTP_HEADER_X_DATA;



int main(void) {

    HTTP_HEADER_X_DATA = fiobj_str_new("X-Data", 6);

    http_listen("2347", NULL, .on_request = on_request, .max_clients=10240,.log = 1,.public_folder="../web");

    fio_start(.threads = 2,.workers=3);

    fiobj_free(HTTP_HEADER_X_DATA);
}


void on_request(http_s *request) {
    FIOBJ method;

    const char *data = "{\n\t\"id\":1,\n"
                       "\t// comments are ignored.\n"
                       "\t\"number\":42,\n"
                       "\t\"float\":42.42,\n"
                       "\t\"string\":\"\\uD834\\uDD1E oh yeah...\",\n"
                       "\t\"hash\":{\n"
                       "\t\t\"nested\":true\n"
                       "\t},\n"
                       "\t\"symbols\":[\"id\","
                       "\"number\",\"float\",\"string\",\"hash\",\"symbols\"]\n}";
    printf("%s\n",data);

    //字符串转换为json 对象后，可以以hash方式访问结点
    FIOBJ json;
    size_t len = fiobj_json2obj(&json,data,strlen(data));
    printf("len=%d\n",len);

    FIOBJ key = fiobj_str_new("number",6);
    if (FIOBJ_TYPE_IS(json,FIOBJ_T_HASH)&&fiobj_hash_haskey(json,key)){
        //printf("有\n");

        FIOBJ number = fiobj_hash_get(json,key);
        printf("%s=%d\n",fiobj_obj2cstr(key).data,fiobj_obj2num(number));
    }
    fiobj_free(key);
    FIOBJ key_str = fiobj_str_new("string",6);
    if (FIOBJ_TYPE_IS(json,FIOBJ_T_HASH)&&fiobj_hash_haskey(json,key_str)){
        FIOBJ str = fiobj_hash_get(json,key_str);
        printf("%s=%s\n",fiobj_obj2cstr(key_str).data,fiobj_obj2cstr(str).data);
    }
    fiobj_free(key_str);

    FIOBJ key_arr = fiobj_str_new("symbols",7);
    if (FIOBJ_TYPE_IS(json,FIOBJ_T_HASH)&&fiobj_hash_haskey(json,key_arr)){
        FIOBJ arr = fiobj_hash_get(json,key_arr);
        //fiobj_ary_shift(arr);
        //printf("symbols=%s\n",fiobj_type_name(arr));
        printf("arr[0]=%s\n",fiobj_obj2cstr(fiobj_ary_shift(arr)).data);
        printf("arr[1]=%s\n",fiobj_obj2cstr(fiobj_ary_shift(arr)).data);
    }
    fiobj_free(key_arr);

    //对象转json字符串对象
    FIOBJ str_data = fiobj_obj2json(json,1);

    printf("%d,%s\n",fiobj_obj2cstr(str_data).len,fiobj_obj2cstr(str_data).data);


    http_set_cookie(request, .name = "my_cookie", .name_len = 9, .value = "data",
                    .value_len = 4);
    http_set_header(request, HTTP_HEADER_CONTENT_TYPE,
                    fiobj_str_new("application/json",16));
    http_set_header(request, HTTP_HEADER_X_DATA, fiobj_str_new("my data", 7));
    http_send_body(request, fiobj_obj2cstr(str_data).data,fiobj_obj2cstr(str_data).len);
}