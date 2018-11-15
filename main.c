#include"logic.h"

MYSQL *con;

typedef struct{
   GtkWidget *entry;
   GtkWidget *label;
   GtkWidget *view;
   GtkTextBuffer *buff;
} EditSet;



void searchClick(GtkButton *btn, gpointer data){
    EditSet *set = (EditSet*)data;
    const gchar *in = gtk_entry_get_text( GTK_ENTRY(set->entry));
   
    if(in != NULL){
        char *str = g_strchomp(in);
        gchar **words = g_strsplit(str, " ", -1);
        GtkTextIter start, end;
        gint wcounter = 0;
         while(*(words+wcounter)){
            wcounter++;
         }
         
         if(wcounter > 1){
             
             MYSQL_RES *res = get_phrase_info(con, str);
             
             if(res != NULL){
                MYSQL_ROW row = mysql_fetch_row(res);             
                gchar *txt;
                
                asprintf(&txt, "\t%s  - %s", str, row[0]);                
                gtk_text_buffer_set_text(set->buff, txt, -1);
                g_free(txt);
                
                gtk_label_set_text( GTK_LABEL(set->label), "  Phrase  ");
            } else {
                gtk_text_buffer_set_text(set->buff, "\tCan't match your phrase in db!", -1);
            }
            
        } else {
            MYSQL_RES *result = get_word_info(con, words[0]);
            if(result != NULL){
                int num_fields = mysql_num_fields(result);
                MYSQL_ROW row = mysql_fetch_row(result);
                
                gchar *status, *txt;
                asprintf(&status, "Base word: %s \t\t Part of speech: %s", row[0], row[1]);

                asprintf(&txt, "\t( %s ) %s  - %s", row[1], row[0], row[2]);
                gtk_text_buffer_set_text(set->buff, txt, -1);
                
                while(row = mysql_fetch_row(result)){
                    
                    asprintf(&status, "%s | %s", status, row[1]);
                    asprintf(&txt, "\n\n\t( %s ) %s  - %s", row[1], row[0], row[2]);
                    
                    gtk_text_buffer_get_end_iter(set->buff, &end);
                    gtk_text_buffer_insert(set->buff, &end, txt, -1);
                }
                
                gtk_label_set_text(GTK_LABEL(set->label), status);
                g_free(status);
                g_free(txt);
                mysql_free_result(result);
            } else {
                gtk_text_buffer_set_text(set->buff, "\tCan't match your word in db!", -1);
            }
        }
    }
}

static void quit_app(void){
    mysql_close(con);
    g_message("Bye.\n");
    gtk_main_quit();
}


int main(int argc, char** argv){
    
    
    con = create_connect("localhost", "root", "abcde", "multi");
    EditSet set;
    
    
    gtk_init(&argc, &argv);
    
    GtkWidget *win, *lbl, *entry, *btn, *separator, *view, *scrollWin, *hbox, *vbox;
    GtkTextBuffer *buff;
    GtkCssProvider *provider;
    GtkStyleContext *context;
    
    win = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_default_size( GTK_WINDOW(win), 600, 450);
    gtk_window_set_title( GTK_WINDOW(win), "EnVoca" );
    gtk_container_set_border_width (GTK_CONTAINER (win), 5);
    gtk_window_set_position( GTK_WINDOW(win), GTK_WIN_POS_CENTER );
    
    g_signal_connect( G_OBJECT(win), "destroy", G_CALLBACK(quit_app), NULL );
    
    // Status label
    lbl = gtk_label_new( g_strdup_printf("MySQL client version: %s", getVersion()) );
    separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    
    // Input textline
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text( GTK_ENTRY(entry), "Looking for...");
    
    // Button
    btn = gtk_button_new_with_label(" Search ");
    g_signal_connect( G_OBJECT(btn), "clicked", G_CALLBACK(searchClick), &set);
    
    // Text view
    scrollWin = gtk_scrolled_window_new(NULL, NULL);
    view = gtk_text_view_new();
    gtk_text_view_set_editable( GTK_TEXT_VIEW(view), FALSE );
    gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW(view), GTK_WRAP_WORD);
    gtk_text_view_set_left_margin( GTK_TEXT_VIEW(view), 20 );
    gtk_text_view_set_right_margin( GTK_TEXT_VIEW(view), 20 );
    gtk_text_view_set_top_margin( GTK_TEXT_VIEW(view), 10 );
    buff = gtk_text_view_get_buffer( GTK_TEXT_VIEW(view) );
    gtk_text_buffer_set_text (buff, "", -1);
    gtk_container_add( GTK_CONTAINER(scrollWin), view );
    
    provider = gtk_css_provider_new ();
    gtk_css_provider_load_from_data (provider,
                                    "textview {"
                                    " font: 15 serif;"
                                    "}", -1, NULL);
    context = gtk_widget_get_style_context (view);
    gtk_style_context_add_provider (context, GTK_STYLE_PROVIDER (provider),
                                    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    // Grid table
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start( GTK_BOX(hbox), entry, TRUE, TRUE, 5);
    gtk_box_pack_start( GTK_BOX(hbox), btn, FALSE, FALSE, 5);
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start( GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    gtk_box_pack_start( GTK_BOX(vbox), separator, FALSE, FALSE, 8);
    gtk_box_pack_start( GTK_BOX(vbox), lbl, FALSE, FALSE, 2);
    gtk_box_pack_start( GTK_BOX(vbox), scrollWin, TRUE, TRUE, 4);
    gtk_container_add( GTK_CONTAINER(win), vbox );
    
    set.entry = entry;
    set.label = lbl;
    set.view = view;
    set.buff = buff;
    
    gtk_widget_show_all(win);
    
    if(con == NULL){
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "\tCan't connect to MySQL!\nFor more information check system logs....");
        gtk_dialog_run( GTK_DIALOG(dialog) );
        g_signal_connect_swapped (dialog, "response", G_CALLBACK (quit_app), dialog);
    }
    
    gtk_main();
    
    exit(EXIT_SUCCESS);
}
