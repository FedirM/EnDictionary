#include<my_global.h>
#include<mysql.h>
#include<stdlib.h>
#include<gtk/gtk.h>

const gchar* getVersion(void){
    return mysql_get_client_info();
}

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  //exit(1);        
}

MYSQL* create_connect(gchar* host, gchar* user, gchar* pass, gchar* def_db){
    MYSQL *con = mysql_init(NULL);
  
  if (con == NULL)
  {
      fprintf(stderr, "mysql_init() failed\n");
      return NULL;
  }  
  
  if (mysql_real_connect(con, host, user, pass, def_db, 0, NULL, 0) == NULL) 
  {
      finish_with_error(con);
      return NULL;
  }
  
  if (!mysql_set_character_set(con, "utf8"))
  {
      g_message("New client character set: %s\n", mysql_character_set_name(con));
  }
  return con;
}

MYSQL_RES* query(MYSQL *con, gchar* query){
    if (mysql_query(con, query)) 
    {
        finish_with_error(con);
        return NULL;
    }
    
    MYSQL_RES *result = mysql_store_result(con);
    
    if (result == NULL) 
    {
        finish_with_error(con);
        return NULL;
    }
    return result;
}


MYSQL_RES * get_word_info(MYSQL *con, gchar *word){
//     g_message("WORD: %s", word);
    MYSQL_RES *result;
    
    char* buffer;
    char query[] = {"SELECT en_root, en_speech, mw_ru FROM multiw W JOIN en_root R ON W.mw_en=R.en_root JOIN en_root_form RF ON R.en_root_id=RF.en_root_id JOIN en_form F ON F.en_form_id=RF.en_form_id WHERE F.en_form ='%s'"};
    
    g_message("Query: %s", query);
    asprintf(&buffer, query, word);
    g_message("Buffer: %s", buffer);
    
    if (mysql_query(con, buffer)) 
    {
        finish_with_error(con);
        return NULL;
    }
    
    result = mysql_store_result(con);
    
    if (result == NULL) 
    {
        finish_with_error(con);
        return NULL;
    }
    g_free(buffer);
    return result;
}

MYSQL_RES * get_phrase_info(MYSQL *con, gchar* ph){
    MYSQL_RES *res;
    
    gchar * callsub;
    asprintf( &callsub, "CALL check_phrase('%s', @ph_ru);", ph);
    
    if(mysql_query(con, callsub)){
        finish_with_error(con);
        return NULL;
    }
    
    if(mysql_query(con, "SELECT @ph_ru")){
        finish_with_error(con);
        return NULL;
    }
    g_free(callsub);
    
    res = mysql_store_result(con);
    return res;
}
