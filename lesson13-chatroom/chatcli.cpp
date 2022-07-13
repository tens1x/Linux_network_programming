#include "Pub.h"
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <error.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>

using namespace std;

char username[16];
USER_LIST client_list;

#define ERR_EXIT(m) \
        do \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while (0);


void do_someone_login(MESSAGE msg)
{
    // puts("do_someone_login\n");
    printf("msgbody : %s\n", msg.body);
    USER_INFO *user = (USER_INFO*)msg.body;//给userinfo.username赋值
    in_addr tmp;
    tmp.s_addr = user->ip;
    printf("%s <-> %s : %d has logined server\n", user->username, inet_ntoa(tmp), ntohs(user->port));
    client_list.push_back(*user);
}

void do_someone_logout(MESSAGE msg)
{
    // puts("do_someone_logout\n");
    USER_LIST::iterator it;
    for (it = client_list.begin(); it != client_list.end(); ++it)
    {
        if (strcmp(it->username, msg.body) == 0)
        {
            break;
        }
    }

    if (it != client_list.end())
    {
        client_list.erase(it);
    }
    printf("user %s has logout\n", msg.body);
}

void do_getlist(int sock)
{
    // puts("do_getlist\n");
    int count;
    recvfrom(sock, &count, sizeof(int), 0, NULL, NULL);
    printf("has %d user logined server\n", ntohs(count));
    client_list.clear();

    int n = ntohs(count);
    for (int i = 0; i < n; ++i)
    {
        USER_INFO user;
        recvfrom(sock, &user, sizeof (USER_INFO), 0, NULL, NULL);
        client_list.push_back(user);
        in_addr tmp;
        tmp.s_addr = user.ip;

        printf("%s <-> %s : %d\n", user.username, inet_ntoa(tmp), ntohs(user.port));
    }
}

char* trim(char* cmdline)
{
    char* s = cmdline;
    while (*s == ' ')
    {
        s++;
    }

    char* e = s;
    int len = 0;
    while ((strcmp(e, "") != 0) && *e != ' ')
    {
        len++;
        e++;
    }
    char* res = new char[len];
    memcpy(res, s, len);
    return res;
}

vector<char*> getcmd(char* cmdline) {
    vector<char*> res;
    char *s = trim(cmdline);
    if (strcmp(s, "") == 0) {
        return res;
    }
    int len = 0;
    char *temp;
    char *tmp ;
    int count = 3;
    while (count--) {
        s = trim(cmdline);
        if (strcmp(s, "") == 0) {
            return res;
        }
        len = 0;
        temp = s;
        tmp = new char[500];
        while ((strcmp(temp, "") != 0) && *temp != ' ') {
            len++;
            temp++;
        }
        memcpy(tmp, s, len);
        res.push_back(tmp);
        cmdline = cmdline + len;
        while ((strcmp(cmdline, "") != 0) && *cmdline == ' ')
        {
            cmdline++;
        }
    }
    return res;
}

bool sendmsgto(int sock, char* name, char* msg)
{
    //cout << "sendmsgto" << endl;
//    if (strcmp(name, username) == 0)
//    {
//        printf("can't send message to self\n");
//        return false;
//    }

    USER_LIST::iterator it;
    for (it = client_list.begin(); it != client_list.end(); ++it)
    {
        if (strcmp(it->username, name) == 0)
        {
            break;
        }
    }

    if (it == client_list.end())
    {
        printf("user %s has not logined server\n", name);
        return false;
    }

    //发送的命令
    MESSAGE m;
    memset(&m, 0, sizeof(m));
    m.cmd = htons(C2C_CHAT);
    
    //发送的用户名和内容
    CHAT_MSG cm;
    strcpy(cm.username, username);
    strcpy(cm.msg, msg);

    memcpy(m.body, &cm, sizeof(MESSAGE));

    struct sockaddr_in peeraddr;
    memset(&peeraddr, 0, sizeof peeraddr);
    peeraddr.sin_family = AF_INET;
    peeraddr.sin_addr.s_addr = it->ip;
    peeraddr.sin_port = it->port;

    in_addr tmp;
    tmp.s_addr = it->ip;

    printf("sending message [%s] to user [%s] <-> %s:%d\n", msg, it->username, inet_ntoa(peeraddr.sin_addr), it->port);
    sendto(sock, &m, sizeof(m), 0, (struct sockaddr*)&peeraddr, sizeof peeraddr);   // 直接发送给客户
    return true;
}

bool send_msg_to_all(int sock, char* msg)
{
    USER_LIST::iterator it;

    //发送的命令
    MESSAGE m;
    memset(&m, 0, sizeof(m));
    m.cmd = htons(C2C_CHAT_ALL);
    
    //发送的用户名和内容
    CHAT_MSG cm;
    strcpy(cm.username, username);
    strcpy(cm.msg, msg);

    //将名字和内容都整合为msgbody
    memcpy(m.body, &cm, sizeof(MESSAGE));

    // 向所有用户广播数据
    //TODO:可以用组播加入特定的一个群组
    for (it = client_list.begin(); it != client_list.end(); ++it)
    {   
        if(strcmp(it->username, cm.username) == 0)
            continue;
        struct sockaddr_in peeraddr;
        memset(&peeraddr, 0, sizeof peeraddr);
        peeraddr.sin_family = AF_INET;
        peeraddr.sin_addr.s_addr = it->ip;
        peeraddr.sin_port = it->port;

        if (sendto(sock, &m, sizeof(MESSAGE), 0, (struct sockaddr*)&peeraddr, sizeof(peeraddr)) < 0)
        {
            ERR_EXIT("sendtoall");
        }
    }
    return true;
}

void parse_cmd(char *cmdline, int sock, struct sockaddr_in server_addr)
{
    vector<char*> cmd = getcmd(cmdline);    // [0]为命令 [1]为发送对方 [2]为发送消息
    if (cmd.size() == 0 )
    {
        printf("bad command\n");
        printf("\n Commands are:\n");
        printf("send username msg\n");
        printf("all\n");
        printf("list\n");
        printf("exit\n");
        printf("\n");
    }
    else if (strcmp(cmd[0], "exit") == 0)
    {
        MESSAGE msg;
        memset(&msg, 0, sizeof(msg));
        msg.cmd = htons(C2S_LOGOUT);
        strcpy(msg.body, username);

        if (sendto(sock, &msg, sizeof(MESSAGE), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            ERR_EXIT("sendto");
        }
        printf("user %s has logout server\n", username);
        exit(EXIT_SUCCESS);
    }
    else if (strcmp(cmd[0], "send") == 0)
    {
        char* peername = new char[16];
        char* msg = new char[MSG_LEN];

        peername = cmd[1];
        msg = cmd[2];
        /* send user msg */
        /*      p    p2  */
        sendmsgto(sock, peername, msg);
    }
    else if (strcmp(cmd[0], "all") == 0)
    {
        char* msg = new char[MSG_LEN];

        msg = cmd[1];
        send_msg_to_all(sock, msg);
    }
    else if (strcmp(cmd[0], "list") == 0)
    {
        MESSAGE msg;
        memset(&msg, 0, sizeof(MESSAGE));
        msg.cmd = htons(C2S_ONLINE_USER);

        if (sendto(sock, &msg, sizeof(MESSAGE), 0, (struct sockaddr*)&server_addr, sizeof server_addr) < 0)
        {
            ERR_EXIT("sendto");
        }
    }
    else
    {
        printf("bad command\n");
        printf("\n Commands are:\n");
        printf("send username msg\n");
        printf("all\n");
        printf("list\n");
        printf("exit\n");
        printf("\n");
    }
};

void do_chat(const MESSAGE& msg)
{
    // cout << "do_chat" << endl;
    CHAT_MSG *cm = (CHAT_MSG*)msg.body;
    printf("recv a msg [%s] from [%s]\n", cm->msg, cm->username);
}

void chat_cli(int sock)
{
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6666);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    MESSAGE msg;
    int ret;
    while (1)
    {
        memset(&msg, 0, sizeof(msg));
        cout << "please input your name:";
        fflush(stdout);
        scanf("%s", username);

        memset(&msg, 0, sizeof(msg));
        msg.cmd = ntohs(C2S_LOGIN);
        strcpy(msg.body, username);

        sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, sizeof server_addr);

        memset(&msg, 0, sizeof(msg));
        ret = recvfrom(sock, &msg, sizeof(msg), 0, NULL, NULL);
        if (ret == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            ERR_EXIT("recvfrom")
        }

        int cmd = ntohs(msg.cmd);
        if (cmd == S2C_ALREADY_LOGINED)
        {
            printf("user %s already logined server, please use another user\n", msg.body);
        }
        else if (cmd == S2C_LOGIN_OK)
        {
            printf("user %s has logined server\n", username);
            break;
        }
    }

    int count = 0;
    recvfrom(sock, &count, sizeof(int), 0, NULL, NULL);

    int n = ntohs(count);
    printf("has %d users logined server\n", n);

    for (int i = 0; i < n; ++i)
    {
        USER_INFO user;
        recvfrom(sock, &user, sizeof(USER_INFO), 0, NULL, NULL);
        client_list.push_back(user);
        in_addr tmp;
        tmp.s_addr = user.ip;

        printf("%d %s <-> %s:%d\n", i, user.username, inet_ntoa(tmp), user.port);
    }
    printf("\n Commands are:\n");
    printf("send username msg\n");
    printf("list\n");
    printf("all\n");
    printf("exit\n");
    printf("\n");

    fd_set rset;
    FD_ZERO(&rset);
    int nready;
    socklen_t peerlen;
    struct sockaddr_in peeraddr;
    while (1)
    {
        FD_SET(STDIN_FILENO, &rset);
        FD_SET(sock, &rset);
        nready = select(sock+1, &rset, NULL, NULL, NULL);
        if (nready == -1)
        {
            ERR_EXIT("select");
        }
        if (nready == 0)
        {
            continue;
        }
        if (FD_ISSET(sock, &rset))
        {
            peerlen = sizeof(peeraddr);
            memset(&msg, 0, sizeof(msg));
            recvfrom(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&peeraddr, &peerlen);
            int cmd = ntohs(msg.cmd);
            switch (cmd)
            {
                case S2C_SOMEONE_LOGIN:
                    do_someone_login(msg);
                    break;
                case S2C_SOMEONE_LOGOUT:
                    do_someone_logout(msg);
                    break;
                case S2C_ONLINE_USER:
                    do_getlist(sock);
                    break;
                case C2C_CHAT:
                    do_chat(msg);
                    break;
                case C2C_CHAT_ALL:
                    do_chat(msg);
                    break; 
                default:
                    break;
            }
        }
        if (FD_ISSET(STDIN_FILENO, &rset))
        {
            char cmdline[100] = {0};
            if (fgets(cmdline, sizeof(cmdline), stdin) == NULL)
            {
                break;
            }
            if (cmdline[0] == '\n')
            {
                continue;
            }
            cmdline[strlen(cmdline) - 1] = '\0';
            parse_cmd(cmdline, sock, server_addr);
        };
    }
}

int main(void)
{
    int sock;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        ERR_EXIT("socket");
    }
    chat_cli(sock);

    return 0;
}