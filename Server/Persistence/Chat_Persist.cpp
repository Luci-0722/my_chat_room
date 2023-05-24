/*************************************************************************
	>    File Name: Chat_Persist.c
	>       Author: fujie
	>         Mail: fujie.me@qq.com
	> Created Time: 2017年08月16日 星期三 09时56分26秒
 ************************************************************************/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"./Chat_Persist.h"
#include<mysql/mysql.h>
#include "../Common/CGImysql/sql_connection_pool.h"
extern connection_pool* m_connPool;
int Chat_Perst_Private(int from_uid ,int to_uid ,const char *msg, int is_offline){
    MYSQL * mysql = m_connPool->GetConnection();
    printf("获取数据库连接成功\n");
    printf("剩余连接池数量%d\n", m_connPool->GetFreeConn());
    m_connPool->ReleaseConnection(mysql);
    char SQL[100 + strlen(msg)];
    sprintf(SQL ,
            "INSERT INTO private_rec VALUES('%d' ,'%d' ,'%s','%d')" ,
            from_uid ,to_uid ,msg  ,is_offline);
    if(mysql_real_query(mysql ,SQL ,strlen(SQL))){
        printf("%s",mysql_error(mysql));
        m_connPool->ReleaseConnection(mysql);
        return 0;
    }
    m_connPool->ReleaseConnection(mysql);
    printf("释放连接池，剩余数量%d\n", m_connPool->GetFreeConn());
    return 1;
}
int Chat_Perst_Group(int uid ,int gid ,const char *msg, const char *offlist){
    MYSQL * mysql = m_connPool->GetConnection();
    char SQL[100 + strlen(msg)];
    sprintf(SQL ,
            "INSERT INTO group_rec VALUES('%d' ,'%d' ,'%s','%s')" ,
            gid ,uid ,msg ,offlist);
    if(mysql_real_query(mysql ,SQL ,strlen(SQL))){
        printf("%s",mysql_error(mysql));
        m_connPool->ReleaseConnection(mysql);
        return 0;
    }
    m_connPool->ReleaseConnection(mysql);
    return 1;
}
//int Chat_Perst_GetOffline(ugid)
MYSQL_RES * Chat_Perst_GetOfflinePrivateMsg(int uid){
    MYSQL * mysql = m_connPool->GetConnection();
    char SQL[100];
    MYSQL_RES *res;
    sprintf(SQL,"SELECT msg FROM private_rec WHERE (to_uid = '%d' AND is_offline = 1)" ,uid);
    if(mysql_real_query(mysql ,SQL ,strlen(SQL))){
        printf("%s\n",mysql_error(mysql));
        m_connPool->ReleaseConnection(mysql);
        return NULL;
    }
    res = mysql_store_result(mysql);
    sprintf(SQL, "UPDATE private_rec set is_offline = '0' WHERE to_uid = '%d'" ,uid);
    if(mysql_real_query(mysql ,SQL ,strlen(SQL))){
        printf("%s\n",mysql_error(mysql));
        m_connPool->ReleaseConnection(mysql);
        return NULL;
    }
    m_connPool->ReleaseConnection(mysql);
    return res;
}

MYSQL_RES *Chat_Perst_GetPrivateRec(int uid ,int fuid){
    MYSQL * mysql = m_connPool->GetConnection();
    char SQL[200];
    sprintf(SQL ,"SELECT msg FROM private_rec WHERE ((to_uid = '%d' AND from_uid = '%d')"
            " OR (to_uid = '%d' AND from_uid = '%d'))" ,uid ,fuid, fuid ,uid);
    if(mysql_real_query(mysql ,SQL ,strlen(SQL))){
        printf("%s\n",mysql_error(mysql));
        m_connPool->ReleaseConnection(mysql);
        return NULL;
    }
    m_connPool->ReleaseConnection(mysql);
    return mysql_store_result(mysql);
}



