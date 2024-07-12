#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/select.h>
/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//添加json 库文件 
#include "cJSON.h"

char * getweather(char *city);

double get_double(cJSON *json, char *key)
{
	cJSON *value = cJSON_GetObjectItem(json,key);
	return cJSON_GetNumberValue(value);
}


char *get_string(cJSON *json, char *key)
{
	cJSON *value = cJSON_GetObjectItem(json,key);
	return cJSON_GetStringValue(value);
}


char * find_weather(char *city)
{
    //创建TCP 通信socket 
    int   tcp_socket = socket(AF_INET, SOCK_STREAM, 0);


    //2.连接服务器  (重点)
            struct sockaddr_in addr; 
            addr.sin_family = AF_INET; 
            addr.sin_port   = htons(80) ; //所有HTTP服务器的端口都是 80 
            addr.sin_addr.s_addr =  inet_addr("124.243.226.2");  //青云客服务器 
    int ret=connect(tcp_socket,(struct sockaddr *)&addr,sizeof(addr));
        if(ret < 0)
        {   
            perror("连接服务器失败\n");
        }else
        {
            printf("连接服务器成功\n");
        }


    //3.往服务器中发送请求协议 
//char *http = "GET /api/weather/city/101280101 HTTP/1.1\r\nHost:t.weather.sojson.com\r\n\r\n";
char http[1024] = {0};
sprintf(http, "GET /csp/api/v2.1/weather?openId=aiuicus&clientType=android&sign=android&city=%s&needMoreData=true&pageNo=1&pageSize=7 HTTP/1.1\r\nHost:autodev.openspeech.cn\r\n\r\n",city);


  write(tcp_socket,http,strlen(http));
    
    //4.读取服务器的应答数据 
    char  data[4096*2]={0};
    read(tcp_socket,data,4096*2);

        printf("请求成功\n");

        //获取响应报体       
       char * p = strstr(data,"{"); 
        //p = strstr(data,"content"); 

        char *p1 = strrchr(p,'}');
        *(p1+1)='\0';
       // printf("%s\n",p); 
        return p;

}


//查找天气并返回json后的数据
char * getweather(char *city)
{

    char *string = find_weather(city);

   //解析JSON 数据包 
   cJSON *json = cJSON_Parse(string);
   if(json == NULL)
   {
       const char *err =  cJSON_GetErrorPtr();  
       fprintf(stderr,"cJSON_Parse fail:%s\n",err);
   }else 
   {
     printf("解析成功\n");
   }

   cJSON *data = cJSON_GetObjectItem( json,"data");
  
		//printf("sourceName:%s\n",get_string(data, "sourceName")) ;


    char bufcity[1024] = {0};
    sprintf(bufcity,"sourceName:%s",get_string(data, "sourceName"));

   cJSON *list = cJSON_GetObjectItem(data,"list");

   int n = cJSON_GetArraySize(list);

   for(int i = 0 ; i < n; i++)
   {
     char bufdate[1024] = {0};
       cJSON * json_str = cJSON_GetArrayItem(list, i);
       // cJSON * json_str= cJSON_Parse(str);
        // printf("city:%s\n",get_string(json_str, "city"));
        // printf("lastUpdateTime:%s\n",get_string(json_str, "lastUpdateTime"));
        // printf("weather:%s\n",get_string(json_str, "weather"));
        // printf("temp:%.2f\n",get_double(json_str, "temp"));
        // printf("==========\n");
        sprintf(bufdate,"city:%sweather:%stemp:%.2f",
        get_string(json_str, "city"),
        get_string(json_str, "weather"),
        get_double(json_str, "temp"));
        char * p = bufdate;
       // printf("bufdate%s\n", p);
        return p; 

   }
}