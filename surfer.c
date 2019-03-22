#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <gdk/gdk.h>

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#define SURFER_META_MASK            GDK_CONTROL_MASK
#define SURFER_NEW_WINDOW_KEY       GDK_KEY_n
#define SURFER_OPEN_KEY             GDK_KEY_o
#define SURFER_CLOSE_KEY            GDK_KEY_q
#define SURFER_BACK_KEY             GDK_KEY_s
#define SURFER_FORWARD_KEY          GDK_KEY_d
#define SURFER_STOP_KEY             GDK_KEY_Escape
#define SURFER_RELOAD_KEY           GDK_KEY_r
#define SURFER_FIND_KEY             GDK_KEY_slash
#define SURFER_HOME_KEY             GDK_KEY_h
#define SURFER_BOOKMARK_KEY         GDK_KEY_b
#define SURFER_INSPECTOR_KEY        GDK_KEY_i
#define SURFER_ZOOM_IN_KEY          GDK_KEY_equal
#define SURFER_ZOOM_OUT_KEY         GDK_KEY_minus
#define SURFER_FULLSCREEN_KEY       GDK_KEY_F11
#define SURFER_HISTORY_KEY          GDK_KEY_R
#define SURFER_SCROLL_DOWN_KEY      GDK_KEY_j
#define SURFER_SCROLL_UP_KEY        GDK_KEY_k
#define SURFER_SCROLL_PAGE_DOWN_KEY GDK_KEY_D
#define SURFER_SCROLL_PAGE_UP_KEY   GDK_KEY_U
#define SURFER_STYLE_KEY            GDK_KEY_S
#define SURFER_COOKIE_POLICY        WEBKIT_COOKIE_POLICY_ACCEPT_ALWAYS
#define USER_STYLESHEET_FILENAME	"/usr/share/surfer/black.css"
#define WEB_EXTENSIONS_DIRECTORY 	"/usr/lib/surfer"
#define HISTORY_ENABLE	1         //change to 1 for enable history 


typedef struct Client{
    GtkWidget *main_window;
    GtkWidget *entry_find;
    GtkWidget *entry_open;
    GtkWidget *box_find;
 
    GtkWidget *button;
    GtkWidget *box_open;

    GtkWidget *vbox;
   
    WebKitWebView *webView;
//  WebKitPolicyDecision *decision1;
    WebKitFindController *fc;
    
    
    int f;
    int s;
    int o;
	
} Client;

static gint clients = 0;
gchar *home;
gchar *history;
gchar *favpath;
gchar *histpath;
static gchar *fullname = "";

static gboolean isbackforward= 0;


static void destroy_window(GtkWidget* w,Client *rc);

static void tls_certs(WebKitWebContext *);

static gboolean close_request(WebKitWebView *view,Client *c);





static Client *client_new(Client *rc);

static WebKitWebView *clientview(Client *c, WebKitWebView *rv);

static void loadurl(Client *rc, gchar *url);

//static WebKitWebView *create_request(Client *c);
static gboolean decide_policy(WebKitWebView *v,WebKitPolicyDecision *decision,
  WebKitPolicyDecisionType type, Client *c);


static gboolean crashed(WebKitWebView *v, Client *c);

static gboolean keyboard(GtkWidget *widget, GdkEvent *event, Client *c,  gpointer );

static void changed_title(GtkWidget *widget,WebKitWebView *rv,Client *c);

static void changed_url(GtkWidget *widget,WebKitWebView *rv,Client *c );

static void changed_webload(WebKitWebView *webview,WebKitLoadEvent event, Client *c);

static void find(GtkWidget *widget,Client *c);

static void openlink(GtkWidget *widget,Client *c);

static void user_style(Client *c);

static void close_find( Client *c);





static void
destroy_window(GtkWidget* w,Client *c) {
    webkit_web_view_stop_loading(c->webView);
    free(c);
    clients--;

    if (clients == 0)
        gtk_main_quit();
}

gboolean
close_request(WebKitWebView *view,Client *c) {
   

    gtk_widget_destroy(c->main_window);

    return TRUE;
}


gboolean crashed(WebKitWebView *v, Client *c){


webkit_web_view_reload(c->webView);



}




Client *client_new(Client *rc) {
      
    Client *c;
 
// c = malloc(sizeof(Client));
    c = calloc(1, sizeof(Client));
    
    c->o = 0;
    c->f = 0;
    c->s = 0;
    c->main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_default_size(GTK_WINDOW(c->main_window), 1100, 700);

 //   c->webView = clientview(c);
   c->webView = clientview(c, rc ? rc->webView : NULL);

     

    c->vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
 
    c->box_find = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    c->box_open = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    c->button = gtk_button_new_with_label ("Close");
    c->entry_find = gtk_entry_new();
    c->entry_open= gtk_entry_new();
  

    gtk_box_pack_start(GTK_BOX(c->box_find), c->entry_find, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(c->box_find), c->button,TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(c->box_open),c->entry_open, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX(c->vbox),c->box_find, FALSE, FALSE, 0);    
    gtk_box_pack_start(GTK_BOX (c->vbox),  c->box_open, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(c->vbox),GTK_WIDGET(c->webView), TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(c->main_window), GTK_WIDGET(c->vbox));
   
    

    //tls_certs(wc);

   
    
    g_signal_connect(G_OBJECT(c->box_open), "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    g_signal_connect(G_OBJECT(c->entry_open), "activate", G_CALLBACK(openlink), c);
    g_signal_connect(G_OBJECT(c->entry_find), "activate", G_CALLBACK(find), c);
    g_signal_connect_swapped (G_OBJECT (c->button), "clicked",G_CALLBACK (close_find),c); 
    g_signal_connect(G_OBJECT(c->main_window), "key-press-event", G_CALLBACK(keyboard),c);
    g_signal_connect(G_OBJECT(c->main_window), "destroy", G_CALLBACK(destroy_window), c);
  //  g_signal_connect(G_OBJECT(c->webView), "decide-policy", G_CALLBACK(decide_policy), c);
//    g_signal_connect(G_OBJECT(c->webView), "web-process-crashed",G_CALLBACK(crashed), c);

//g_signal_connect(G_OBJECT(c->webView), "notify::url", G_CALLBACK(changed_url), c);    


    gtk_widget_show_all(c->main_window);
    gtk_widget_grab_focus(GTK_WIDGET(c->webView));
    gtk_widget_hide(c->box_find);
    gtk_widget_hide(c->box_open);
    
    
    clients++;
 return c;
}



WebKitWebView *clientview(Client *c,WebKitWebView *rv)
{
WebKitWebView *view;
WebKitSettings *settings;
WebKitUserContentManager *contentmanager;
WebKitWebsiteDataManager *mgr;
WebKitCookieManager *cookiemgr;
WebKitWebContext *wc;

gboolean enabled = 1;
FILE *cookie_file_handler;
gchar *cookies_path = g_build_filename(getenv("HOME"), ".cookies", NULL);
gchar *cookie_file = g_build_filename(cookies_path, "cookie", NULL); 
gchar *datadir  = g_build_filename(g_get_user_data_dir() , fullname, NULL);
gchar *cachedir = g_build_filename(g_get_user_cache_dir(), fullname, NULL);

if (rv != NULL) {
               //  c->webView=WEBKIT_WEB_VIEW(rv);
		view = WEBKIT_WEB_VIEW(
		    webkit_web_view_new_with_related_view(rv));
                 //  printf("related\n");
	} 


else {

   


settings = webkit_settings_new();

contentmanager = webkit_user_content_manager_new(); 


  

//mgr = webkit_website_data_manager_new("base-data-directory" , datadir,"base-cache-directory", cachedir,NULL);


//wc = webkit_web_context_new_with_website_data_manager(mgr);
  
wc= webkit_web_context_get_default();
   // view= WEBKIT_WEB_VIEW(webkit_web_view_new_with_context(wc));

    
    //char *value = "Googlebot/2.1";
    //g_object_set(settings, "user-agent", &value, NULL);

    g_object_set(G_OBJECT(settings), "enable-developer-extras", TRUE, NULL);
    g_object_set(G_OBJECT(settings), "enable-webgl", TRUE, NULL);


tls_certs(wc);

   
webkit_web_context_set_process_model(wc,WEBKIT_PROCESS_MODEL_MULTIPLE_SECONDARY_PROCESSES);

webkit_web_context_set_web_extensions_directory(wc, WEB_EXTENSIONS_DIRECTORY);



 //tell webkit where to store cookies
    if (!g_file_test(cookies_path, G_FILE_TEST_EXISTS)) {
        mkdir(cookies_path, 0700);
        cookie_file_handler = fopen(cookie_file, "wb+");
        fclose(cookie_file_handler);
    }


 // cookiemgr = webkit_website_data_manager_get_cookie_manager(mgr);
  cookiemgr = webkit_web_context_get_cookie_manager(wc);

    webkit_cookie_manager_set_persistent_storage(
            cookiemgr, cookie_file,
          //  WEBKIT_COOKIE_PERSISTENT_STORAGE_TEXT); // as mentioned in surf sources only text storage works
WEBKIT_COOKIE_PERSISTENT_STORAGE_SQLITE);
   webkit_cookie_manager_set_accept_policy(
            cookiemgr,
            SURFER_COOKIE_POLICY);

   
   webkit_web_context_set_tls_errors_policy(wc, WEBKIT_TLS_ERRORS_POLICY_IGNORE);
   


   

view = g_object_new(WEBKIT_TYPE_WEB_VIEW,
		    "settings", settings,
		    "user-content-manager", contentmanager,
		    "web-context",wc );

// printf("new\n");
}

g_object_connect(
        G_OBJECT(view),"signal::load-changed", G_CALLBACK(changed_webload),c,
                       "signal::notify::title", G_CALLBACK(changed_title),c ,
                       "signal::notify::url", G_CALLBACK(changed_url), c,
                       "signal::decide-policy", G_CALLBACK(decide_policy),c,
                       "signal::close", G_CALLBACK(close_request), c,
                       "signal::web-process-crashed",G_CALLBACK(crashed), c,
    NULL
    );

 //   g_signal_connect(G_OBJECT(view), "create", G_CALLBACK(create_request), rc);
//g_signal_connect(G_OBJECT(c->main_window), "key-press-event", G_CALLBACK(keyboard),c);
   


return view;
 }


void
loadurl(Client *c, gchar *url)
{
    gchar *link;

    link = g_ascii_strdown(url, -1);
    if (!g_str_has_prefix(link, "http:") &&
        !g_str_has_prefix(link, "https:") &&
        !g_str_has_prefix(link, "file:") &&
        !g_str_has_prefix(link, "about:")) {
        g_free(link);
        link = g_strdup_printf("http://%s", url);
    } else

       link = g_strdup(url);
    


//link = soup_uri_normalize(url,NULL);
 //printf("%s/n",link);   
     webkit_web_view_load_uri(WEBKIT_WEB_VIEW(c->webView), link);
  
    g_free(link);



}


void
user_style(Client *c){
	
	gchar *contents;
 
	g_file_get_contents(USER_STYLESHEET_FILENAME,&contents,NULL,NULL);
	webkit_user_content_manager_add_style_sheet(
	    webkit_web_view_get_user_content_manager(WEBKIT_WEB_VIEW(c->webView)),
	    webkit_user_style_sheet_new(contents,
	    WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
	    WEBKIT_USER_STYLE_LEVEL_USER,
	    NULL, NULL));

	g_free(contents);
}


void
close_find(Client *rc) {
   
 
    gtk_widget_hide(rc->box_find);
    gtk_widget_grab_focus(GTK_WIDGET(rc->webView));

}






void
changed_title(GtkWidget *widget,WebKitWebView *rv,Client *c) {
    const gchar *title;
    const gchar *url;

    
    

    title = webkit_web_view_get_title(WEBKIT_WEB_VIEW(c->webView));
    url = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(c->webView));
   
    gtk_window_set_title(GTK_WINDOW(c->main_window), title);
    url = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(c->webView));

   gtk_entry_set_text(GTK_ENTRY(c->entry_open), url);
    
   
    
    
    
    

 
}

void
changed_url(GtkWidget *widget,WebKitWebView *rv,Client *c) {

   const gchar *url;

   url = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(c->webView));
                 
   gtk_entry_set_text(GTK_ENTRY(c->entry_open), url);
   
}

static void changed_webload(WebKitWebView *webview,
        WebKitLoadEvent event, Client *c)
{
    GTlsCertificateFlags tlsflags;
    const gchar *title;
    const gchar *url =NULL;
    FILE *File;
    
    title = webkit_web_view_get_title(WEBKIT_WEB_VIEW(c->webView));
    url = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(c->webView));

    switch (event) {
        case WEBKIT_LOAD_STARTED:
             break;
           

        case WEBKIT_LOAD_REDIRECTED:
  if(HISTORY_ENABLE==1 && isbackforward==0 && title!=NULL){
    File = fopen(histpath, "a");                
                   
    fprintf(File, "<a href=\"%s\" >%.60s</a>\n",url,title );
                   
    fclose(File);
   
     
    }   
     isbackforward= 0;
            break;

        case WEBKIT_LOAD_COMMITTED:
         
            break;

        case WEBKIT_LOAD_FINISHED:
      if(HISTORY_ENABLE==1 && isbackforward==0 && title!=NULL){
    

    File = fopen(histpath, "a");                
                   
    fprintf(File, "<a href=\"%s\" >%.60s</a>\n",url,title );
                   
    fclose(File);
   
     
    }   
     isbackforward= 0;
        
            break;
 } 
}




gboolean
keyboard(GtkWidget *widget,GdkEvent *event, Client *c,  gpointer data) {
  
 //   Client *c = (Client *)data;
    WebKitWebInspector *inspector;
    FILE *File;
    char buffer[256] = "</body></html>";
    Client *rc;
    const gchar *url;
    const gchar *tmp;
    gdouble z;
    guint meta_key_pressed;
    int key_pressed;
//   GdkEvent *event = c->eventt; 

    if (event->type == GDK_KEY_PRESS) {
        key_pressed = ((GdkEventKey *) event)->keyval;
        meta_key_pressed = ((GdkEventKey *) event)->state & SURFER_META_MASK;

        if (meta_key_pressed) {
            switch (key_pressed) {
                case SURFER_SCROLL_DOWN_KEY:
                    event->key.keyval = GDK_KEY_Down;
                    gdk_event_put(event);
                    return TRUE;

                case SURFER_SCROLL_UP_KEY:
                    event->key.keyval = GDK_KEY_Up;
                    gdk_event_put(event);
                    return TRUE;

                case SURFER_SCROLL_PAGE_UP_KEY:
                    event->key.keyval = GDK_KEY_Page_Up;
                    gdk_event_put(event);
                    return TRUE;

                case SURFER_SCROLL_PAGE_DOWN_KEY:
                    event->key.keyval = GDK_KEY_Page_Down;
                    gdk_event_put(event);
                    return TRUE;

                case SURFER_CLOSE_KEY:
                    //gtk_widget_destroy(c->main_window);
                    close_request(c->webView,c);
                    return TRUE;

                case SURFER_BACK_KEY:
                    webkit_web_view_go_back(WEBKIT_WEB_VIEW(c->webView));
                    isbackforward= 1;
                    return TRUE;

                case SURFER_FORWARD_KEY:
                    webkit_web_view_go_forward(WEBKIT_WEB_VIEW(c->webView));
                    isbackforward= 1;
                    return TRUE;

                case SURFER_INSPECTOR_KEY:
                    inspector = webkit_web_view_get_inspector(WEBKIT_WEB_VIEW(c->webView));
                    webkit_web_inspector_show(inspector);
                    return TRUE;

                case SURFER_OPEN_KEY:
                  if (c->o == 0) {
                 gtk_widget_show_all(c->box_open);
                 gtk_widget_grab_focus(c->entry_open);
                    url = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(c->webView));
                    gtk_entry_set_text(GTK_ENTRY(c->entry_open), url);
                        c->o = 1;
		
                    } else {
                     gtk_widget_hide(c->box_open);
                       c->o = 0;
                     }

                    return TRUE;

		case SURFER_HISTORY_KEY:
                    webkit_web_view_load_uri(c->webView, history);
                    return TRUE;
                        

                case SURFER_NEW_WINDOW_KEY:     
                    rc = client_new(c);
                    loadurl(rc,home);
                    return TRUE;

                case SURFER_HOME_KEY:
                    webkit_web_view_load_uri(c->webView, home);
                    return TRUE;

                case SURFER_RELOAD_KEY:
                    webkit_web_view_reload(c->webView);
                    return TRUE;

                case SURFER_FIND_KEY:
                    gtk_widget_show_all(c->box_find);
		    gtk_widget_grab_focus(c->entry_find);
                    return TRUE;

                case SURFER_BOOKMARK_KEY:
                    File = fopen(favpath, "a");
                    //if(File== NULL)
                    tmp = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(c->webView));
                    fprintf(File, "<a href=\"%s\" >%.110s</a><br>", (char *) tmp, (char *) tmp);
                    fprintf(File, "%s\n", buffer);
                    fclose(File);
                    return TRUE;

                case SURFER_ZOOM_OUT_KEY:
                    z = webkit_web_view_get_zoom_level(WEBKIT_WEB_VIEW(c->webView));
                    webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(c->webView), z - 0.1);
                    return TRUE;

                case SURFER_ZOOM_IN_KEY:
                    z = webkit_web_view_get_zoom_level(WEBKIT_WEB_VIEW(c->webView));
                    webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(c->webView), z + 0.1);
                    return TRUE;
		case SURFER_STYLE_KEY:
                   
		if (c->s == 0) {
                 user_style(c);
                        c->s = 1;
                    } else {
                      webkit_user_content_manager_remove_all_style_sheets(
			    webkit_web_view_get_user_content_manager(WEBKIT_WEB_VIEW(c->webView)));
                        c->s = 0;
                    }
                    return TRUE;

                default:
                    return FALSE;
            }
        } else {
            switch (key_pressed) {
                case SURFER_FULLSCREEN_KEY:
                    if (c->f == 0) {
                        gtk_window_fullscreen(GTK_WINDOW(c->main_window));
                        c->f = 1;
                    } else {
                        gtk_window_unfullscreen(GTK_WINDOW(c->main_window));
                        c->f = 0;
                    }
                    return TRUE;

                case SURFER_STOP_KEY:
                    webkit_web_view_stop_loading(WEBKIT_WEB_VIEW(c->webView));
                    return TRUE;
               

                default:
                    return FALSE;
            }
        }
    }
    return FALSE;
}

gboolean
decide_policy( WebKitWebView *v,WebKitPolicyDecision *decision,
              WebKitPolicyDecisionType type,Client *c) {
    WebKitResponsePolicyDecision *r;
    WebKitNavigationType navigation_type;
    WebKitNavigationPolicyDecision *navigationDecision;
    WebKitNavigationAction *navigation_action;
    WebKitURIRequest *request;
    guint button, mods;
    gchar *link;
     gchar *t;
    Client *rc;
    


    switch (type) {
        case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION:
            navigationDecision = WEBKIT_NAVIGATION_POLICY_DECISION(WEBKIT_POLICY_DECISION(decision));
            navigation_action = webkit_navigation_policy_decision_get_navigation_action(navigationDecision);
            request = webkit_navigation_action_get_request(navigation_action);
            navigation_type = webkit_navigation_action_get_navigation_type(navigation_action);  

            if (navigation_type == WEBKIT_NAVIGATION_TYPE_LINK_CLICKED) {
                mods = webkit_navigation_action_get_modifiers(navigation_action);
                button = webkit_navigation_action_get_mouse_button(navigation_action);

                if (button == 1 && mods & GDK_CONTROL_MASK) {
                    t = (gchar *) webkit_uri_request_get_uri(request);
                    
                    rc = client_new(c);
                   
                    loadurl(rc,t);
                    webkit_policy_decision_ignore(decision);
                    return TRUE;
                } else webkit_policy_decision_use(decision);
                return TRUE;
            }
            break;

        case WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION:
            navigationDecision = WEBKIT_NAVIGATION_POLICY_DECISION(decision);
            navigation_action = webkit_navigation_policy_decision_get_navigation_action(navigationDecision);
            request = webkit_navigation_action_get_request(navigation_action);
            navigation_type = webkit_navigation_action_get_navigation_type(navigation_action);  
            if (navigation_type == WEBKIT_NAVIGATION_TYPE_LINK_CLICKED) {

                t = (gchar *) webkit_uri_request_get_uri(request);
                rc = client_new(c);
                    loadurl(rc,t);
                return TRUE;
            }
              webkit_policy_decision_ignore(decision);
              return TRUE;
            break;

        case WEBKIT_POLICY_DECISION_TYPE_RESPONSE:
            r = WEBKIT_RESPONSE_POLICY_DECISION(decision);
            if (!webkit_response_policy_decision_is_mime_type_supported(r))
                webkit_policy_decision_download(decision);
            else
                webkit_policy_decision_use(decision);
            break;

        default:
            webkit_policy_decision_use(decision);
            return FALSE;
    }
    return TRUE;
}

void
openlink(GtkWidget * widget,Client *c){


    gchar *link;
   const gchar *p;

   p = gtk_entry_get_text(GTK_ENTRY(c->entry_open));
   link = g_strdup(p);
   loadurl(c, link);
   g_free(link);
   gtk_widget_hide(c->box_open);
}

void
find(GtkWidget * widget,Client *c) {

    static gchar *search_text;
    const gchar *p;

    c->fc= webkit_web_view_get_find_controller(c->webView);

    

p = gtk_entry_get_text(GTK_ENTRY(c->entry_find));
    search_text = g_strdup(p);

    if(search_text != NULL){
       
    
    webkit_find_controller_search(c->fc, search_text,
                                  WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE | WEBKIT_FIND_OPTIONS_WRAP_AROUND, G_MAXUINT);
     g_free(search_text);

    }
    
}



void
tls_certs(WebKitWebContext *wc) {
    GDir *directory;
    GTlsCertificate *certificate;
    const gchar *basedir, *keyfile;

    basedir = g_build_filename(g_get_user_config_dir(), "/usr/bin/surfer", "certs", NULL);
    directory = g_dir_open(basedir, 0, NULL);

    if (directory != NULL) {
        keyfile = g_build_filename(g_get_user_config_dir(), "/usr/bin/surfer", "certs", NULL);

        certificate = g_tls_certificate_new_from_file(keyfile, NULL);
        webkit_web_context_allow_tls_certificate_for_host(wc, certificate, keyfile);
        g_dir_close(directory);
    }
}

int main(int argc, char *argv[]) {
    Client *c;
    int i;
    gchar *favfilename;
    gchar *histfilename;
    FILE *File;
    char buffer[256] = "<!DOCTYPE html><html><head><meta charset=utf8><style>body {background-color: #000009;}p    {color: yellow;} a:link { color: #00e900; }</style><p>";
    gchar *link;
    char textdate[100]; 
    gtk_init(&argc, &argv);
    
    
     //
    
   

    favfilename = g_strdup_printf("%s", ".fav");
    favpath = g_build_filename(getenv("HOME"), favfilename, NULL);
    g_free(favfilename);

    if (!g_file_test(favpath, G_FILE_TEST_EXISTS)) {
        File = fopen(favpath, "wb+");
        fprintf(File, "%s", buffer);
        fclose(File);
        g_free(File);
    }

    histfilename = g_strdup_printf("%s", ".hist");
    histpath = g_build_filename(getenv("HOME"), histfilename, NULL);
    g_free(histfilename);

    if (!g_file_test(histpath, G_FILE_TEST_EXISTS)) {
        File = fopen(histpath, "wb+");
        fprintf(File, "%s", buffer);
        fclose(File);
        g_free(File);
}    
     
     if(HISTORY_ENABLE==1 ){
        time_t now = time(NULL);
    struct tm *t = localtime(&now);       

    strftime(textdate, sizeof(textdate)-1, "%d %m %Y %H:%M", t);

    File = fopen(histpath, "a");
                   
                   
    fprintf(File, "%s",textdate);


                   
    fclose(File);
}

  
    //home =  g_strdup(favpath);

    home = (gchar *) g_strdup_printf("file://%s", favpath); 
    history = (gchar *) g_strdup_printf("file://%s", histpath); 

    c = client_new(NULL);

    if (argc > 1) {
        for (i = 1; i < argc; i++) {
            link = (gchar *) argv[i];
            loadurl(c,link);
        }
    } 
   
  else    loadurl(c,home);


   gtk_main();

    return 0;
}
