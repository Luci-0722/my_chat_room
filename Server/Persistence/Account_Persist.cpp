/*************************************************************************
	>    File Name: Account_Persist.c
	>       Author: fujie
	>         Mail: fujie.me@qq.com
	> Created Time: 2017年08月08日 星期二 15时42分05秒
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Account_Persist.h"
#include "MySQL.h"
#include "../Service/Friends_Srv.h"
#include "../Common/CGImysql/sql_connection_pool.h"
extern connection_pool* m_connPool;
int Account_Perst_ChIsOnline(int uid ,int is_online){
    char SQL[100];
    sprintf(SQL ,"UPDATE account SET is_online = '%d' WHERE uid = '%d'" ,is_online ,uid);
    MYSQL * mysql = m_connPool->GetConnection();
    if(mysql_real_query(mysql , SQL , strlen(SQL))){
        printf("%s\n",mysql_error(mysql));
        m_connPool->ReleaseConnection(mysql);
        return 0;
    }
    m_connPool->ReleaseConnection(mysql);
    return 1;
}


int Account_Perst_IsUserName(const char * name){
    char SQL[100];
    MYSQL_RES * res;
    MYSQL_ROW row;
    int rtn = 0;
    sprintf(SQL,"SELECT uid FROM account WHERE name = '%s'",name);
    printf("正在查询用户 %s\n", name, SQL);
    MYSQL * mysql = m_connPool->GetConnection();
    if(mysql_real_query(mysql , SQL , strlen(SQL))){
        m_connPool->ReleaseConnection(mysql);
        printf("用户名不存在\n");
        return 0;
    }    

    res = mysql_store_result(mysql);
    row = mysql_fetch_row(res);
    if(row) rtn = atoi(row[0]);
    mysql_free_result(res);    
    m_connPool->ReleaseConnection(mysql);
    return rtn;
}

int Account_Perst_AddUser(const char *name ,int sex , const char *password){
    char SQL[100];
    MYSQL * mysql = m_connPool->GetConnection();

    sprintf(SQL,"INSERT INTO account VALUES (NULL , '%s' ,'%d' , 0, 0 , md5('%s'))", name ,sex , password);
    if(mysql_real_query(mysql , SQL , strlen(SQL))){
        m_connPool->ReleaseConnection(mysql);
        printf("%s\n",mysql_error(mysql));
        return 0;
    }    
    m_connPool->ReleaseConnection(mysql);
    return 1;

}


int Account_Perst_MatchUserAndPassword(int uid , const char * password){
    char SQL[100];
    MYSQL_RES * res;
    MYSQL_ROW row;
    int rtn;
    sprintf(SQL,"SELECT * FROM account WHERE (uid = '%d' AND password = md5('%s'))" 
            , uid , password);
    MYSQL * mysql = m_connPool->GetConnection();

    if(mysql_real_query(mysql , SQL ,strlen(SQL))){
        m_connPool->ReleaseConnection(mysql);
        return 0;
    }
    res = mysql_store_result(mysql);
    row = mysql_fetch_row(res);
    if(row) rtn = 1;
    else rtn = 0;
    mysql_free_result(res);    
    m_connPool->ReleaseConnection(mysql);
    return rtn;
}

char * Account_Perst_GetUserNameFromUid(int uid){
    MYSQL_RES *res;
    MYSQL_ROW row;
    char * rtn = NULL;
    char SQL[100];
    MYSQL * mysql = m_connPool->GetConnection();

    sprintf(SQL, "SELECT name FROM account WHERE uid = '%d'" ,uid);
    if(mysql_real_query(mysql , SQL ,strlen(SQL))){
        m_connPool->ReleaseConnection(mysql);
        printf("%s\n" ,mysql_error(mysql));
        return 0;
    }
    res = mysql_store_result(mysql);
    row = mysql_fetch_row(res);
    if(row){
        rtn = (char *)malloc(sizeof(char) * 30);
        strcpy(rtn ,row[0]);
    }
    mysql_free_result(res);    
    m_connPool->ReleaseConnection(mysql);
    return rtn;
}

