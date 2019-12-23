//
// Created by 1655664358@qq.com on 2019/12/14.
//

#ifndef _DATABASE_H
#define _DATABASE_H
int gbk_strlen(const char* str)
{
    const char* p = str;		//p用于后面遍历

    while(*p)					//若是结束符0，则结束循环
    {
        if(*p < 0 && (*(p+1)<0 || *(p+1) < 63))			//中文汉字情况
        {
            str++;				//str移动一位，p移动移动2位，因此长度加1
            p += 2;
        }
        else
        {
            p++;				//str不动，p移动一位，长度加1
        }
    }
    return p-str;				//返回地址之差
}
#endif //CHAT_DATABASE_H
