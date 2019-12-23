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


//    //创建一个数组
//    FIOBJ array = fiobj_ary_new();
//    printf("数组地址=%d\n",array);
//
//    //给数据添加元素
//    char *string = "哈哈";
//    FIOBJ data = fiobj_str_new(string,strlen(string));
//    fiobj_ary_push(array,data);
//
//    fiobj_ary_push(array,request->method);
//
//    //弹出最后的数据
//    FIOBJ pop = fiobj_ary_pop(array);
//    printf("pop=%s\n",fiobj_obj2cstr(pop).data);
//
//    //移出前面的数据
//    fiobj_ary_push(array,request->path);
//    printf("shitf=%s\n",fiobj_obj2cstr(fiobj_ary_shift(array)).data);
//    printf("shitf=%s\n",fiobj_obj2cstr(fiobj_ary_shift(array)).data);
//    printf("shitf=%s\n",fiobj_obj2cstr(fiobj_ary_shift(array)).data);
//
//    fiobj_ary_push(array,request->version);
//    fiobj_ary_push(array,request->path);
//    //查找元素位置
//    int64_t index = fiobj_ary_find(array,request->path);
//    printf("index=%d\n",index);
//
//    //替换数组元素的内容
//    FIOBJ oldElem = fiobj_ary_replace(array,request->body,index);
//    printf("oldElem=%s\n",fiobj_obj2cstr(oldElem).data);
//
//    //printf("pop=%s\n",fiobj_obj2cstr(fiobj_ary_pop(array)).data);
//
//    //删除数组某个下标的元素
//    int ret = fiobj_ary_remove(array,index);
//    printf("remove result=%d\n",ret);
//    printf("pop=%s\n",fiobj_obj2cstr(fiobj_ary_pop(array)).data);

//    FIOBJ  array = fiobj_ary_new2(4);
//    fiobj_ary_push(array,request->path);
//    fiobj_ary_push(array,request->version);
//    int count = fiobj_ary_count(array);
//    printf("%d\n",count);
//    fiobj_ary_push(array,request->body);
//    fiobj_ary_push(array,request->method);
//    fiobj_ary_push(array,request->params);
//    count = fiobj_ary_count(array);
//    printf("%d\n",count);
//    int k=0;
//    for(k;k<count;k++){
//        printf("%d=%s\n",k,fiobj_obj2cstr(fiobj_ary_shift(array)).data);
//    }

    //创建一个2维数组
//    FIOBJ data1 = fiobj_ary_new();
//    FIOBJ data2 = fiobj_ary_new2(2);
//
//    fiobj_ary_push(data1,request->method);
//    fiobj_ary_push(data1,request->path);
//
//    fiobj_ary_push(data2,data1);
//
//    if (fiobj_type_is(data2,FIOBJ_T_ARRAY)){
//        FIOBJ one = fiobj_ary_pop(data2);
//
//        printf("method=%s->%s\n",fiobj_obj2cstr(fiobj_ary_shift(one)).data,fiobj_obj2cstr(fiobj_ary_shift(one)).data);
//    }else{
//        printf("不是数组\n");
//    }

    //创建一个数字对象
//    FIOBJ num = fiobj_num_new(100);
//    printf("%d\n",fiobj_obj2num(num));
//    num = fiobj_num_tmp(1999);
//    printf("%d\n",fiobj_obj2num(num));
//
//    num = fiobj_float_new(1324.2352);
//    printf("%f\n",fiobj_obj2float(num));
//
//    num = fiobj_float_tmp(234.432);
//    printf("%f\n",fiobj_obj2float(num));
//
//    fiobj_float_set(num,88.88);
//    printf("%f\n",fiobj_obj2float(num));

    //Hash 表创建
//    FIOBJ hash = fiobj_hash_new();
//    printf("%d\n",hash);
//
//    //给hash表添加key,value
//    if(fiobj_hash_set(hash,request->method,request->path)==0){
//        printf("hash table add success\n");
//    }
//    //hash弹出栈
//    FIOBJ elem = fiobj_hash_pop(hash,&request->method);
//    printf("%s\n",elem);
//
//    size_t si = fiobj_hash_count(hash);
//    printf("si=%d\n",si);
//
//    elem = fiobj_hash_get(hash,request->method);
//    printf("hash_get=%d\n",elem);
//    printf("%s\n",fiobj_obj2cstr(elem).data);
//
//    int key = fiobj_hash_haskey(hash,request->method);
//    printf("是否存在%d\n",key);
//
//    if(fiobj_hash_delete(hash,request->method)==0){
//        printf("hash键删除成功");
//    }
    FIOBJ hash = fiobj_hash_new();
    fiobj_hash_set(hash,request->method,request->path);
    int k = fiobj_hash_haskey(hash,request->method);
    printf("k=%d\n",k);
    FIOBJ path = fiobj_hash_get(hash,request->method);
    printf("path=%s\n",fiobj_obj2cstr(path).data);

    fiobj_hash_replace(hash,request->method,request->body);
    path = fiobj_hash_get2(hash,fiobj_str_hash(request->method));
    printf("path=%s\n",fiobj_obj2cstr(path).data);
    FIOBJ name;
    name = fiobj_str_buf(20);
    fiobj_str_write(name,"name",10);
    fiobj_hash_set(hash,name,request->params);

    size_t count = fiobj_hash_count(hash);
    printf("%d\n",count);

    size_t cap = fiobj_hash_capa(hash);
    printf("hash 容量:%d\n",cap);

    FIOBJ params = fiobj_hash_get2(hash,fiobj_str_hash(name));
    printf("%s=%s\n",fiobj_obj2cstr(name).data,fiobj_obj2cstr(params).data);


    http_set_cookie(request, .name = "my_cookie", .name_len = 9, .value = "data",
    .value_len = 4);
    http_set_header(request, HTTP_HEADER_CONTENT_TYPE,
                    http_mimetype_find("txt", 3));
    http_set_header(request, HTTP_HEADER_X_DATA, fiobj_str_new("my data", 7));

    printf("query=%s\n",fiobj_obj2cstr(request->query).data);
    http_send_body(request, "Hello World!\r\n", 14);
}