#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include <string.h>


//初始化节点
void Insert_Node(struct node *head, int socket)
{
	struct node *new_node = malloc(sizeof(struct node));
	
	new_node->socket = socket;
	new_node->next = NULL;
	
	new_node->next = head->next;
	head->next = new_node;
	
}

//尾插
void Insert_Node_tail(struct node *head, int socket, char *name, char *ip, char *sign,char *share_path, char *share_file)
{
	//创建新节点
	struct node *new_node = malloc(sizeof(struct node));
	//初始化新节点
	new_node->socket = socket;
	strcpy(new_node->name, name);
	strcpy(new_node->ip, ip);	
	strcpy(new_node->signature, sign);	
	strcpy(new_node->pathname, share_path);		
	strcpy(new_node->share_file, share_file);		
	new_node->next = NULL;
	
	//循环遍历找到最后一个数据的尾巴  这里注意：如果使用	while(pos->= NULL) 是直接指向最后的空的位置 
	struct node *pos = head;
	while(pos->next != NULL)
	{
		pos=pos->next;
	}

	//插入新节点
	pos->next = new_node;
	
}

//利用名字查找到节点
char * FindNodes(struct  node *pos, char *name)
{
	
     while (pos != NULL)
     {
        

		if(strcmp(pos->name,name) == 0)
		{
			//printf("get socket success!pos->socket=%d\n",pos->socket);
			return pos->ip;
			
		}
		pos=pos->next;
     }
	 
	//printf("get socket fail!\n");	 
	 return NULL;
		
	
}
//利用ip查找到 返回节点 用于查找共享文件
struct node* Find_IP_Nodes(struct  node *pos, char *IP)
{
	
     while (pos != NULL)
     {
        

		if(strcmp(pos->ip,IP) == 0)
		{
			printf("ip:%s\n",pos->ip);
			return pos; 
			
		}
		pos=pos->next;
     }
	 
	 return NULL;
		
	
}

//遍历好友的信息
void Traver_Nodes(struct  node *head)
{
	printf("==========好友列表==============\n");	 	
	
	struct  node *pos = head->next;
     while (pos != NULL)
     {
        
		printf("[%s] %s  个性签名:%s\n",pos->ip, pos->name ,pos->signature);
        pos=pos->next;
     }
	printf("========================\n\n");	 
	
} 

//遍历好友的共享文件夹
void Traver_sharepath_Nodes(struct  node *head)
{
	printf("==========共享文件夹==============\n");	 	
	struct  node *pos = head->next;
     while (pos != NULL)
     {
        
        printf("[%s] %s  共享路径:%s\n",pos->ip, pos->name ,pos->pathname);
        pos=pos->next;
     }
	printf("========================\n\n");	 
	
}

//删除节点
int Remove_Node(struct  node *head, char *name)
{
	struct  node *pos = head->next;//定义pos指针遍历
	struct  node *prev = head;//定义prev指针遍历
	
	
     while (pos != NULL)
     {

		if(  strcmp(pos->name,name) == 0)
		{
			//重新连线 
			prev->next = pos->next;
			pos->next = NULL;
			free(pos);//释放节点
			//printf("节点删除成功！\n");	
			pos = prev;//回家
		}
		prev = pos;
		pos=pos->next;

     }
	 
	 
	 return 0;
		
	
	
}




	
