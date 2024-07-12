#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <stdio.h>
#include <string.h>
#include "list.h"
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/types.h>          /* See NOTES */

int udp_socket = 0;
char people[1024]={0};  
extern int recv_file();
extern int send_file(char *ip, char *file_path);
extern char * getweather(char *city);

void *recv_task(void *arg)
{
    //等待用户回发数据 (开个线程接收)
    while (1)
    {
        
         struct sockaddr_in clien_addr;
         int len=sizeof(clien_addr);

         char msg[1024]={0};
         recvfrom(udp_socket,msg,1024,0,(struct sockaddr*)&clien_addr,&len);

         if(strncmp(msg,"loin",4)==0) //登陆通知 
         {
            char buf[124] ={0};
            char ip_buf[50] ={0};
            char name_buf[50] ={0};
            sscanf(msg,"loin %s %s",name_buf,ip_buf);
            //sprintf(buf,"[%s]:%s上线了", ip_buf, name_buf);
            printf("[%s]:%s上线了\n", ip_buf, name_buf);      
            //插入链表：
            //Insert_Node_tail(head,udp_socket,)    
            //回发自己的信息
            sendto(udp_socket,people,strlen(people),0,(struct sockaddr*)&clien_addr,len);
         }
         if(strncmp(msg,"quit",4)==0) //退出通知
         {
            printf("%s:下线了\n",msg);
         }
        
         if(strncmp(msg,"people",6)==0) //回发信息people 
         {
            printf("%s\n",msg);
         }

         if(strncmp(msg,"name",4)==0) //接收文件信息
         {
            recv_file();
         }
    }
}

int main(int argc, char const *argv[])
{
    if(argc != 3)
    {
        printf("请输入姓名和IP  ./main 姓名 IP\n");
        return -1; 
    }    
    //0.创建单链表头结点 设置共享
    struct node *head = malloc(sizeof(struct node));

    //1.创建对象
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_socket<0)
    {
        perror("");
        return -1;
    }
    //开启广播功能 
    int on=1; //开启
    int ret =  setsockopt(udp_socket,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on)); 
        if(ret < 0)
        {
            perror("广播开启失败:");
            return -1; 
        }else 
        {
            printf("广播开启成功\n");
        }  
    //设置广播地址 
    struct sockaddr_in dest_addr;
    dest_addr.sin_family=AF_INET; //设置网络协议 
    dest_addr.sin_port=htons(6666); //所有处于192.168.64网段且，端口为 6666 的进程都可以收到数据 
    dest_addr.sin_addr.s_addr=inet_addr("192.168.1.255"); //设置广播地址 

    //绑定IP地址信息 
    struct sockaddr_in server_addr;
    server_addr.sin_family=AF_INET; //设置网络协议 
    server_addr.sin_port=htons(6666); //所有处于192.168.64网段且，端口为 6666 的进程都可以收到数据 
    server_addr.sin_addr.s_addr=inet_addr(argv[2]);  
    ret=bind(udp_socket,(struct sockaddr *)&server_addr,sizeof(server_addr)); 
      if(ret  < 0)
      {
        perror("");
        return  -1;
      }
    pthread_t tid;
    pthread_create(&tid, NULL,recv_task, NULL);

    //拼接登陆通知协议 
    char loin[1024]={0};  //11
    sprintf(loin,"%s %s %s\n","loin",argv[1],argv[2]);

    //设计登陆通知协议   loin  姓名  IP 
    //发送一个UDP 广播数据报
    sendto(udp_socket,loin,strlen(loin),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr)); 

    //拼接回发通知协议 
    sprintf(people,"%s %s %s \n","people",argv[1],argv[2]);
    //加入链表
    Insert_Node_tail(head, udp_socket,(char *)argv[1], (char *)argv[2]);


    while (1)
    {
      int n;
      printf("======网络聊天系统====\n");
      printf("1.获取在线好友 2.指定好友聊天 3.群聊 4.发送文件 5.共享文件 6.获取个性签名 7.退出系统\n");
      scanf("%d", &n);
      if(n == 1)
      {
        //遍历链表
        Traver_Nodes(head);
      }else if( n == 2)
      {
        char buf_name[256] = {0};
        char buf_ip[124] = {0};    
        char buf_msg[1024] = {0};
        //指定好友发送消息
        printf("请输入好友的名字\n");
        scanf("%s", buf_name);
        //查找节点socket 
        strcmp(buf_ip, FindNodes(head, buf_name));
        if(buf_ip== NULL)
        {
          printf("查找失败\n");
        }else{
          //获取 好友 的 ip 和端口
          struct sockaddr_in dest_addr;
          dest_addr.sin_family=AF_INET; //设置网络协议 
          dest_addr.sin_port=htons(6666);
          dest_addr.sin_addr.s_addr=inet_addr(buf_ip);  
          printf("请输入发送的信息\n");  
          scanf("%s", buf_msg);      
          sendto(udp_socket,buf_msg,strlen(buf_msg),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr));           
        }

      }else if( n == 3)
      {
        //群聊 组播
        //1.加入组播
        struct ip_mreq a;
        bzero(&a, sizeof(a));
        a.imr_interface.s_addr = INADDR_ANY; //所有网卡地址加入组播 
        a.imr_multiaddr.s_addr = inet_addr("224.10.10.10"); // 指定多播地址 230.90.76.1
        //开启组播功能 
        int ret = setsockopt(udp_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &a, sizeof(a));    
        if(ret < 0)
        {
          perror("");
        }else{
          printf("开启组播成功\n");
        }
          //开启一个线程 发送群聊的消息
          // pthread_t tid; 
          // pthread_create(&tid,NULL,recv_task,NULL);
          while (1)
          {
            //4.往组播地址中发送数据 
             printf("进入群聊成功！按1退出\n");
            struct sockaddr_in arry_addr;
            arry_addr.sin_family=AF_INET;
            arry_addr.sin_port=htons(6666);
            arry_addr.sin_addr.s_addr=inet_addr("224.10.10.10"); //所有网卡地址 

            printf("请输入内容\n");
            char buf[1024]={0};
            scanf("%s",buf);
            printf("buf:%s\n",buf);
            if(strncmp(buf,"1",1) == 0)
            {
              break;
            }
            sendto(udp_socket,buf,strlen(buf),0,(struct sockaddr *)&arry_addr,sizeof(arry_addr));
          }
          

      }else if( n == 4)
      {
        //发送文件
        char filepath[512] = {0};
        char name[124] = {0};
        char ip[124] = {0};

        printf("发送文件中。。|请输入文件名\n");
        scanf("%s",filepath);
        printf("请输入好友的名字\n");
        scanf("%s",name);
        strcpy(ip, FindNodes(head, name));
        if( ip != NULL)
        {
          //获取ip成功
         send_file(ip,filepath);

        }else{
          break;
        }
        

      
      }else if( n == 5){

        //文件共享



      }else if( n == 6){
        //获取个性签名 http
        char city[124] = {0};
        printf("获取个性签名 请输入你的城市\n");
        scanf("%s", city);
        char *str = getweather(city);
        printf("你的个性签名：%s\n", str);
        //数据 加入链表中



      }else if( n == 7)
      {
        //退出 
        //拼接退出通知协议 
        char quit[1024]={0};  
        sprintf(quit,"%s %s %s\n","quit",argv[1],argv[2]);

        //设计退出通知协议   quit 姓名  IP 
        //发送一个UDP 广播数据报
        sendto(udp_socket,quit,strlen(quit),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr));   
        return -1;      
      }

    }
    

    return 0;
}

