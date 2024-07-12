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
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <dirent.h>

int udp_socket = 0;//udp套接字
struct node *head = NULL;//链表头结点
char download_name[256] = {0};
char file_path[256] = {0};
char people[1024]={0};  
extern char * getweather(char *city);
extern int send_file(char *pathname, char *ip ,int port);
extern int bind_tcp_serv(unsigned short port);
extern int recv_file();
extern int recv_sharefile();
extern int bind_sharetcp_serv(unsigned short port);
char share_path[256] = {0};
struct sockaddr_in dest_addr;
char  all_share_file[1024]; 

/*
udp端口：
tcp客户端端口：
tcp服务器端口：
*/
int udp_port = 9699;
int tcp_client_port = 8686;
int tcp_server_port = 6666;

//线程接收消息函数
void *recv_task(void *arg)
{
        char sign_buf[124] ={0};
        char ip_buf[50] ={0};
        char name_buf[50] ={0};   
        char path[125] ={0};   
         char share_file[4096] ={0};   
        struct sockaddr_in clien_addr;
        int len=sizeof(clien_addr);      

    //等待用户回发数据
    while (1)
    {    
        

         char msg[1024]={0};

         int ret = recvfrom(udp_socket,msg,1024,0,(struct sockaddr*)&clien_addr,&len);
         
          char *ip = inet_ntoa(clien_addr.sin_addr);
          unsigned short port = ntohs(clien_addr.sin_port);

        //2.设置接收者的地址
        struct sockaddr_in dest_addr;
        dest_addr.sin_family = AF_INET; //设置网络协议
        dest_addr.sin_port = htons(port);
        dest_addr.sin_addr.s_addr = inet_addr(ip);          

       // printf("ret%d msg:%s\n",ret,msg);
         if(ret > 0)
         {
            if(strncmp(msg,"##loin",6)==0) //登陆通知 
            {
                sscanf(msg,"##loin %s %s %s %s %s",name_buf,ip_buf,sign_buf,path,share_file);
                printf("[%s]:%s上线了", name_buf, ip_buf);  
                printf("个性签名:%s\n",sign_buf);
                //判断是否存在链表中 不存在则加入链表
                if(FindNodes(head, name_buf) == NULL) 
                {
                    //加入链表中
                    Insert_Node_tail(head,udp_socket,name_buf,ip_buf,sign_buf,share_path,share_file);
                }

                //回发自己的信息
                sendto(udp_socket,people,strlen(people),0,(struct sockaddr*)&dest_addr,len);
                continue;

            }
            if(strncmp(msg,"##reply",7)==0) //回发自己信息 让别人的好友列表有你在！
            {//##reply 小明 192.168.1.211 个性签名:city:揭阳Time:2022-09-0420:55:08weather:多云temp:32.00 ./ ||1.png
              sscanf(msg,"##reply %s %s 个性签名:%s %s %s",name_buf,ip_buf,sign_buf,path,share_file);
              //判断是否存在链表中 不存在则加入链表
              if(FindNodes(head, name_buf) == NULL) 
              {
                  //加入链表中              
                  Insert_Node_tail(head,udp_socket,name_buf,ip_buf,sign_buf,share_path,share_file);
              }
              continue;

            }

            if(strncmp(msg,"##quit",6)==0) //退出通知
            {
                sscanf(msg,"##quit %s %s",name_buf,ip_buf);
                printf(" [%s] %s:下线了\n",ip_buf, name_buf);
                //判断是否存在链表中 存在则删除
                if(FindNodes(head, name_buf) != NULL) 
                {
                    Remove_Node(head, name_buf);
                }  
                continue;          
            }
            
            if(strncmp(msg,"##share",7)==0) //接收共享文件请求 发送文件给好友
            {   
                sscanf(msg,"##share %s",share_path);
                printf("收到[%s]共享文件下载请求\n",ip);
                //tcp发送共享文件
                send_file(share_path,ip,9898);
                continue;
            }

            if(strncmp(msg,"##msg",5) == 0) //接收好友私聊
            {
              char strmsg[1024] = {0};
              sscanf(msg,"##msg %s",strmsg);
              printf("[%s] %d: %s\n",ip, port, strmsg );
              continue;
            }
            if(strncmp(msg,"##all",5) == 0) //接收群聊消息
            {  //##all[小明] 192.168.1.211: 66 
              char strname[128] = {0};
              char msg1[1024] = {0};
              sscanf(msg,"##all %s %s",strname,msg1);
              printf("群聊消息: %s[%s] : %s\n",strname,ip, msg1);
            }            
         }

    }
}

//接收文件线程
void *recv_tcpfile(void *arg)
{

  printf("服务器启动了。。\n");
  while (1)
  {
    int ret =recv_file();//接收文件 端口设置6666
    if(ret == -1)
    {
      break;
    }
  }
  
}

//捕捉ctrl+c信号 
void catch_sigint(int arg)
{

        //退出 
        printf("触发键盘信号函数\n");
        //拼接退出通知协议 
        char quit[1024]={0};  
        char name_buf[512] = {0};
         char ip_buf[128] = {0};
        sscanf(people,"##reply %s %s 个性签名:",name_buf,ip_buf);
        sprintf(quit,"%s %s %s\n","##quit",name_buf,ip_buf);
        //设计退出通知协议   quit 姓名  IP 
        //发送一个UDP 广播数据报
        int ret = sendto(udp_socket,quit,strlen(quit),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr));  
        if(ret < 0 )
        {
          perror("");
        } 
        close(udp_socket);
        system("killall -9 main");//有时退出的时候进程会还在 得关了否则会出现udp绑定失败
           
    
}

//广播消息
int udp_broadcast(char const*argv[])
{

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

    dest_addr.sin_family=AF_INET; //设置网络协议 
    dest_addr.sin_port=htons(udp_port); //所有处于192.168.64网段且，端口为 6666 的进程都可以收到数据 
    dest_addr.sin_addr.s_addr=inet_addr("192.168.1.255"); //设置广播地址 

    //绑定IP地址信息 
    struct sockaddr_in server_addr;
    server_addr.sin_family=AF_INET; //设置网络协议 
    server_addr.sin_port=htons(udp_port); //所有处于192.168.64网段且，端口为 6666 的进程都可以收到数据 
    server_addr.sin_addr.s_addr = INADDR_ANY;  
    ret=bind(udp_socket,(struct sockaddr *)&server_addr,sizeof(server_addr)); 

      if(ret  < 0)
      {
        perror("");
       // return  -1;
      }  else{
        printf("绑定udp成功\n");
      }   

      bind_tcp_serv(8989); //绑定接收文件的tcp服务器
      bind_sharetcp_serv(9898); //绑定接收文件的tcp服务器9898
    //创建一个线程接收udp数据
    pthread_t tid;
    pthread_create(&tid, NULL,recv_task, NULL);  
    //创建一个线程接收tcp发送的文件
    pthread_t tid1;
   pthread_create(&tid1, NULL,recv_tcpfile, NULL);   
    //拼接回发通知协议 
  char *weather_str = getweather((char *)argv[3]);  //(char *)argv[3]
    if(weather_str == NULL)
    {
      printf("http请求获取签名失败 可能网络问题 请重新尝试\n");
      //return -1;
      exit(0);
    }
    //printf("你的个性签名：%s\n", weather_str);
    sprintf(people,"%s %s %s 个性签名:%s %s %s\n","##reply",argv[1],argv[2],weather_str,share_path,all_share_file);

    //拼接登陆通知协议 

    char loin[1024]={0};  
    sprintf(loin,"%s %s %s %s %s %s\n","##loin",argv[1],argv[2],weather_str,share_path,all_share_file);


    //设计登陆通知协议   loin  姓名  IP 
    //发送一个UDP 广播数据报
    sendto(udp_socket,loin,strlen(loin),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr)); 

  
}


//检索文件函数
void serch_dir(char *path)
{
  int i = 0;
  //1.打开目录
 DIR * dp =  opendir(path);
 if(dp==NULL)
 {
  perror("");
  return;
 }
 //2.读取源目录的文件 
     printf("检索共享文件夹下的文件中。\n");
    while (1)
    {
        //读取源目录的文件 
        struct dirent *msg=readdir(dp);
        if(msg == NULL)
        {
            break;
        }

        if(msg->d_name[0] == '.')  //跳过隐藏文件
        {
            continue;
        }
        //判断是否为普通文件 
        if(msg->d_type == DT_REG)
        {
            //进行目录拼接 

            sprintf(all_share_file,"%s||%s",all_share_file,msg->d_name);
            //strcpy(all_share_file[i],all1);
            //printf("%s\n",all1);
            //i++;

        }

    }

   // sprintf(all_share_file,"");//拼接总的共享文件


}


//主函数
int main(int argc, char const*argv[])
{


    signal(2, catch_sigint);//捕捉ctrl+c信号
    if(argc != 4)
    {
        printf("请输入姓名和IP  ./main 姓名 IP 城市\n");
        return -1; 
    }  
    printf("请输入共享文件路径\n");  
    scanf("%s",share_path);
    serch_dir(share_path);
    printf("%s||%s\n",share_path,all_share_file);
    //0.创建单链表头结点 设置共享
    head = malloc(sizeof(struct node));

    udp_broadcast(argv);//开启广播

    while (1)
    {
      int n;
      printf("======网络聊天系统====\n");
      printf("1.获取在线好友 2.指定好友聊天 3.群聊 4.发送文件 5.共享文件  6.退出系统\n");
      printf("======================\n\n");      
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
        char all_msg[1024] = {0};
        //指定好友发送消息
        printf("请输入好友的IP\n");
        scanf("%s", buf_ip);

          //获取 好友 的 ip 和端口
          struct sockaddr_in dest_addr;
          dest_addr.sin_family=AF_INET; //设置网络协议 
          dest_addr.sin_port=htons(udp_port);  //atoi(argv[4])
          dest_addr.sin_addr.s_addr=inet_addr(buf_ip);  
          printf("进入聊天成功！按quit退出\n");
          while (1)
          {
          
            printf("请输入发送的信息\n");  
            scanf("%s", buf_msg);  
            sprintf(all_msg,"##msg %s",buf_msg);  
            if(strcmp(buf_msg,"quit")== 0)
            {
              break;
            }  
            sendto(udp_socket,all_msg,strlen(all_msg),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr));   
          }
               

      }else if( n == 3)
      {
        //群聊 组播
        char group[124] = {0};
        printf("请输入群聊的组播号：\n");
        scanf("%s", group);
        //1.加入组播
        struct ip_mreq a;
        bzero(&a, sizeof(a));
        a.imr_interface.s_addr = INADDR_ANY; //所有网卡地址加入组播 
        a.imr_multiaddr.s_addr = inet_addr(group); // 指定多播地址 
        //开启组播功能 
        int ret = setsockopt(udp_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &a, sizeof(a));    
        if(ret < 0)
        {
          perror("");
        }else{
          printf("开启组播成功\n");
        }
          printf("进入群聊成功！按quit退出\n");
          while (1)
          {
            //4.往组播地址中发送数据 
            struct sockaddr_in arry_addr;
            arry_addr.sin_family=AF_INET;
            arry_addr.sin_port=htons(udp_port);
            arry_addr.sin_addr.s_addr=inet_addr(group); //所有网卡地址 

            printf("请输入内容\n");
            char buf[512]={0};
            char msgbuf[1024]={0};
            scanf("%s",buf);
            sprintf(msgbuf,"##all %s %s", argv[1], buf);//拼接群里消息标志##all
            if(strncmp(buf,"quit",4) == 0)
            {
              break;
            }
            sendto(udp_socket,msgbuf,strlen(msgbuf),0,(struct sockaddr *)&arry_addr,sizeof(arry_addr));
          }
          

      }else if( n == 4)
      {
        //发送文件
        char file_name[124] = {0};
        char ip[124] = {0};

        printf("发送文件中。。|请输入文件名\n");
        scanf("%s",file_name);
        printf("请输入好友的ip\n");
        scanf("%s",ip);
        send_file(file_name,ip,8989); ////发送文件 端口设置8989    
      
      }else if( n == 5){

        //文件共享  共享自己的文件路径  如果别人进入点进来下载 你再发送这个文件给他 他接收 即可
          //1.查看好友的共享文件
        char ip[125] = {0};
        char pathname[512] = {0};
        char msgbuf[1024] = {0};
        char file[256] = {0};
        Traver_sharepath_Nodes(head);
        printf("请输入好友的ip\n");
        scanf("%s",ip);
       struct node *p= Find_IP_Nodes(head,ip);//利用ip查找该节点
       printf("%s\n",p->share_file);//打印节点的共享文件信息
        printf("请输入你要下载文件好友的文件\n");
        scanf("%s",file);
        sprintf(pathname,"%s%s",p->pathname,file);
        //发送udp数据告诉好友 需要下载
          //获取 好友 的 ip 和端口
          struct sockaddr_in dest_addr;
          dest_addr.sin_family=AF_INET; //设置网络协议 
          dest_addr.sin_port=htons(udp_port);  //
          dest_addr.sin_addr.s_addr=inet_addr(ip);  
         sprintf(msgbuf,"##share %s", pathname);//拼接群里消息标志##all
         sendto(udp_socket,msgbuf,strlen(msgbuf),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr));
          recv_sharefile();//接收共享文件

      }else if( n == 6)
      {
        //退出 
        //拼接退出通知协议 
        char quit[1024]={0};  
        sprintf(quit,"%s %s %s\n","##quit",argv[1],argv[2]);
        //设计退出通知协议   quit 姓名  IP 
        //发送一个UDP 广播数据报
        int ret = sendto(udp_socket,quit,strlen(quit),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr));  
        if(ret < 0 )
        {
          perror("");
        } 
        close(udp_socket);
      
        system("killall -9 main");//有时退出的时候进程会还在 得关了否则会出现udp绑定失败
        return -1;      
      }

    }
    
    return 0;
}

