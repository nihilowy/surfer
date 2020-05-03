#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <gdk/gdk.h>

#include <sys/file.h>

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>


#define SURFER_META_MASK            GDK_CONTROL_MASK
#define SURFER_NEW_WINDOW_KEY       GDK_KEY_n
#define SURFER_OPEN_KEY             GDK_KEY_o
#define SURFER_CLOSE_KEY            GDK_KEY_q
#define SURFER_BACK_KEY             GDK_KEY_b
#define SURFER_FORWARD_KEY          GDK_KEY_f
#define SURFER_STOP_KEY             GDK_KEY_Escape
#define SURFER_RELOAD_KEY           GDK_KEY_r
#define SURFER_FIND_KEY             GDK_KEY_slash
#define SURFER_HOME_KEY             GDK_KEY_h
#define SURFER_BOOKMARK_KEY         GDK_KEY_B
#define SURFER_INSPECTOR_KEY        GDK_KEY_i
#define SURFER_ZOOM_IN_KEY          GDK_KEY_equal
#define SURFER_ZOOM_OUT_KEY         GDK_KEY_minus
#define SURFER_FULLSCREEN_KEY       GDK_KEY_F11
#define SURFER_HISTORY_KEY          GDK_KEY_H
#define SURFER_SCROLL_DOWN_KEY      GDK_KEY_Down
#define SURFER_SCROLL_UP_KEY        GDK_KEY_Up
#define SURFER_SCROLL_PAGE_DOWN_KEY GDK_KEY_s
#define SURFER_SCROLL_PAGE_UP_KEY   GDK_KEY_w
#define SURFER_STYLE_KEY            GDK_KEY_S
#define SURFER_COOKIE_POLICY        WEBKIT_COOKIE_POLICY_ACCEPT_ALWAYS
//WEBKIT_COOKIE_POLICY_ACCEPT_ALWAYS -Accept all cookies unconditionally.
//WEBKIT_COOKIE_POLICY_ACCEPT_NEVER -Reject all cookies unconditionally.
//WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY -Accept only cookies set by the main document loaded

#define USER_STYLESHEET_FILENAME	"/usr/share/surfer/black.css"  //change to your style file
#define DEFAULT_STYLE_ENABLE 0 //change to 1 to enable default style

#define WEB_EXTENSIONS_DIRECTORY 	"/usr/lib/surfer"
#define HISTORY_ENABLE	0         //change to 1 for enable history 

#define SURFER_DIR	".surfer"                 // upper directory(s) must exist
#define SURFER_DOWNLOADS "downloads"


typedef struct Client{
    GtkWidget *main_window;
    GtkWidget *entry_find;
    GtkWidget *entry_open;
    GtkWidget *box_find;

    GtkWidget *button;
    GtkWidget *button_dm;
    GtkWidget *button_js;
    GtkWidget *button_find_back;
    

    GtkWidget *box_open;

    GtkWidget *vbox;

    WebKitWebView *webView;
//  WebKitPolicyDecision *decision1;
    WebKitFindController *fc;
    WebKitSettings *settings;

    int f;
    int s;
    int o;


  /* TLS information. */
  GTlsCertificate *certificate;
  GTlsCertificateFlags tls_errors;

  gboolean bypass_safe_browsing;
  gboolean loading_error_page;
  char *tls_error_failing_uri;




} Client;

static GtkWidget *menuitem1;
static GtkWidget *menu;
//static gchar *download_dir = "/var/tmp";

static gint clients = 0,downloads = 0;
gchar *home;
const gchar *history;
gchar *favpath;
gchar *histpath;
gchar *downloads_dir;
gchar *surfer_dir;


static gchar *fullname = "";

gboolean enablejs=1;
static gboolean isbackforward= 0,dl_win_show=FALSE;
static gboolean wc_setup_done = FALSE;


static void destroy_window(GtkWidget* w,Client *rc);

//static void tls_certs(WebKitWebContext *wc);

static void allow_tls_cert (Client *c);

static void display_webview( WebKitWebView *rc,Client *c);

static gboolean close_request(WebKitWebView *view,Client *c);

static void find_back(GtkWidget * widget,Client *c);

static void enablejs_cb(GtkWidget * widget,Client *c);


static Client *client_new(Client *rc);

static WebKitWebView *clientview(Client *c, WebKitWebView *rv);

static void loadurl(Client *rc, gchar *url);

static WebKitWebView *create_request(WebKitWebView *rv,WebKitNavigationAction *navact, Client *c);


static gboolean decide_policy(WebKitWebView *v,WebKitPolicyDecision *decision,WebKitPolicyDecisionType type, Client *c);
static void decide_navaction(WebKitPolicyDecision *decision,Client *c);
static void decide_newwindow(WebKitPolicyDecision *decision,Client *c);
static void decide_response(WebKitPolicyDecision *decision,Client *c);

static gboolean button_press( Client *c);
static gboolean download_handle(WebKitDownload *, gchar *, gpointer);
static void download_handle_start(WebKitWebView *, WebKitDownload *, gpointer);
static void download_cancel( GtkWidget *,WebKitDownload *download);
static void download_handle_finished(WebKitDownload *download, gpointer data);
static void download_progress( WebKitDownload *download,GParamSpec *pspec,    GtkWidget *tb);


static gboolean crashed(WebKitWebView *v, Client *c);

static gboolean keyboard(GtkWidget *widget, GdkEvent *event, Client *c,  gpointer );

static void changed_title(GtkWidget *widget,WebKitWebView *rv,Client *c);
static void changed_url(GtkWidget *widget,WebKitWebView *rv,Client *c );
static void changed_webload(WebKitWebView *webview,WebKitLoadEvent event, Client *c);

static void find(GtkWidget *widget,Client *c);
static void openlink(GtkWidget *widget,Client *c);
static void user_style(Client *c);
static void close_find( Client *c);
static void goback(Client *c);
static void goforward(Client *c);

static gboolean setup();



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

return TRUE;

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


    c->fc= webkit_web_view_get_find_controller(c->webView);




   

    c->vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    c->box_find = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    c->box_open = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    c->button_dm = gtk_button_new_with_label("[...]");
    c->button_js = gtk_button_new_with_label("->js");

    gtk_widget_show_all (menuitem1);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem1);

    c->button = gtk_button_new_with_label ("Close");
    c->button_find_back = gtk_button_new_with_label ("Find Back");
    c->entry_find = gtk_entry_new();
    c->entry_open= gtk_entry_new();


    gtk_box_pack_start(GTK_BOX(c->box_find), c->entry_find, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(c->box_find), c->button_find_back, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(c->box_find), c->button,TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(c->box_open), c->button_dm,FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(c->box_open),c->entry_open, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(c->box_open),c->button_js, FALSE, FALSE, 0);

    
    gtk_box_pack_start(GTK_BOX (c->vbox),  c->box_open, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(c->vbox),GTK_WIDGET(c->webView), TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX(c->vbox),c->box_find, FALSE, FALSE, 0);

    //gtk_container_add(GTK_CONTAINER(c->button_dm), GTK_WIDGET(menu));

    gtk_container_add(GTK_CONTAINER(c->main_window), GTK_WIDGET(c->vbox));



    g_signal_connect(G_OBJECT(c->box_open), "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    g_signal_connect(G_OBJECT(c->entry_open), "activate", G_CALLBACK(openlink), c);
    g_signal_connect(G_OBJECT(c->entry_find), "activate", G_CALLBACK(find), c);
    g_signal_connect(G_OBJECT(c->button_find_back), "clicked", G_CALLBACK(find_back), c);
    g_signal_connect(G_OBJECT(c->button_dm), "clicked", G_CALLBACK(button_press), c);
    g_signal_connect(G_OBJECT(c->button_js), "clicked", G_CALLBACK(enablejs_cb), c);
    g_signal_connect_swapped (G_OBJECT (c->button), "clicked",G_CALLBACK (close_find),c);
    g_signal_connect(G_OBJECT(c->main_window), "key-press-event", G_CALLBACK(keyboard),c);
    g_signal_connect(G_OBJECT(c->main_window), "destroy", G_CALLBACK(destroy_window), c);

   // g_signal_connect_object (G_OBJECT(c->main_window), "allow-tls-certificate",G_CALLBACK (allow_tls_cert),c);

    gtk_widget_show_all(c->main_window);
//    gtk_widget_grab_focus(GTK_WIDGET(c->webView));

    display_webview(NULL, c);

 if (DEFAULT_STYLE_ENABLE == 1){
    user_style(c);
 }
 

    clients++;
 return c;
}



WebKitWebView *clientview(Client *c,WebKitWebView *rv)
{
WebKitWebView *view;

WebKitUserContentManager *contentmanager;
WebKitWebsiteDataManager *mgr;
WebKitCookieManager *cookiemgr;
WebKitWebContext *wc;

//gboolean enabled = 1;



FILE *File;

//gchar *cookies_path = g_build_filename(getenv("HOME"), surfer_dir, NULL);
gchar *cookie_file = g_build_filename(surfer_dir, "cookie", NULL);

gchar *datadir  = g_build_filename(g_get_user_data_dir() , fullname, NULL);
gchar *cachedir = g_build_filename(g_get_user_cache_dir(), fullname, NULL);



if (rv != NULL) {
               //  c->webView=WEBKIT_WEB_VIEW(rv);
		view = WEBKIT_WEB_VIEW(
		    webkit_web_view_new_with_related_view(rv));
                 //  printf("related\n");
	}


else {



c->settings = webkit_settings_new();

contentmanager = webkit_user_content_manager_new();


//mgr = webkit_website_data_manager_new("base-data-directory" , datadir,"base-cache-directory", cachedir,NULL);
//wc = webkit_web_context_new_with_website_data_manager(mgr);
   wc= webkit_web_context_get_default();

   // view= WEBKIT_WEB_VIEW(webkit_web_view_new_with_context(wc));
 //   char *value = "Mozilla/5.0";
 //   g_object_set(settings, "user-agent", &value, NULL);

    if (!wc_setup_done)
    {

    g_signal_connect(G_OBJECT(wc), "download-started", G_CALLBACK(download_handle_start), NULL);
    wc_setup_done = TRUE;
    }


    g_object_set(G_OBJECT(c->settings), "enable-developer-extras", TRUE, NULL);
    g_object_set(G_OBJECT(c->settings), "enable-webgl", TRUE, NULL);

    g_object_set(G_OBJECT(c->settings), "enable-mediasource", TRUE, NULL);
   //_object_set(G_OBJECT(c->settings),"enable-javascript", FALSE, NULL);

//allow_tls_cert(c,wc);


webkit_web_context_set_process_model(wc,WEBKIT_PROCESS_MODEL_MULTIPLE_SECONDARY_PROCESSES);

webkit_web_context_set_web_extensions_directory(wc, WEB_EXTENSIONS_DIRECTORY);

//webkit_web_context_set_sandbox_enabled(wc,enabled);

 //tell webkit where to store cookies

    if (!g_file_test(cookie_file, G_FILE_TEST_EXISTS)) {
        //mkdir(cookies_path, 0700);
        File = fopen(cookie_file, "wb+");
        
        fclose(File);
    }

 


 // cookiemgr = webkit_website_data_manager_get_cookie_manager(mgr);

  cookiemgr = webkit_web_context_get_cookie_manager(wc);

   webkit_cookie_manager_set_persistent_storage(cookiemgr, cookie_file,WEBKIT_COOKIE_PERSISTENT_STORAGE_SQLITE);
   webkit_cookie_manager_set_accept_policy(cookiemgr,SURFER_COOKIE_POLICY);


   webkit_web_context_set_tls_errors_policy(wc, WEBKIT_TLS_ERRORS_POLICY_IGNORE);



view = g_object_new(WEBKIT_TYPE_WEB_VIEW,
		    "settings", c->settings,
		    "user-content-manager", contentmanager,
		    "web-context",wc );

 //printf("new\n");
}

g_object_connect(
        G_OBJECT(view),"signal::load-changed", G_CALLBACK(changed_webload),c,
                       "signal::notify::title", G_CALLBACK(changed_title),c ,
                       "signal::notify::url", G_CALLBACK(changed_url), c,
                       "signal::decide-policy", G_CALLBACK(decide_policy),c,
                       "signal::close", G_CALLBACK(close_request), c,
                       "signal::ready-to-show",G_CALLBACK(display_webview), c,
                       "signal::create",G_CALLBACK(create_request), c,
                       "signal::web-process-crashed",G_CALLBACK(crashed), c,
//                       "signal::load-failed-with-tls-errors", G_CALLBACK(allow_tls_cert), c,
    NULL
    );





return view;
}


gboolean
button_press( Client *c )
{

    gtk_menu_popup_at_pointer (GTK_MENU (menu), NULL);

    return FALSE;
}


void
download_handle_start(WebKitWebView *web_view, WebKitDownload *download,
                      gpointer data)
{
    g_signal_connect(G_OBJECT(download), "decide-destination",
                     G_CALLBACK(download_handle), data);
}


gboolean
download_handle(WebKitDownload *download, gchar *suggested_filename, gpointer data)
{
    gchar *sug, *path, *path2 = NULL, *basename;
    GtkWidget *tb;
    int suffix = 1;
    size_t i;
    gchar *uri = NULL;
    const gchar *download_uri;

if (!suggested_filename || !*suggested_filename) {
        download_uri = webkit_uri_request_get_uri(webkit_download_get_request(download));
        uri  = soup_uri_decode(download_uri);
        basename     = g_filename_display_basename(uri);
        g_free(uri);

        suggested_filename = basename;
    }

    sug = g_strdup(suggested_filename);



    for (i = 0; i < strlen(sug); i++)
        if (sug[i] == G_DIR_SEPARATOR )
            sug[i] = '_';

    path = g_build_filename(downloads_dir, sug, NULL);
    path2 = g_strdup(path);

    while (g_file_test(path2, G_FILE_TEST_EXISTS) && suffix < 100)
    {
        g_free(path2);

        path2 = g_strdup_printf("%s.%d", path, suffix);
        suffix++;
    }

    if (suffix == 100)
    {
        fprintf(stderr, "Surfer : Suffix reached limit for download.\n");
        webkit_download_cancel(download);
    }
    else
    {
        uri = g_filename_to_uri(path2, NULL, NULL);
        webkit_download_set_destination(download, uri);
        g_free(uri);
        downloads++;

        tb = gtk_menu_item_new_with_label(sug);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu),tb);
        gtk_widget_show_all (GTK_WIDGET(menu));


        g_signal_connect(G_OBJECT(download), "notify::estimated-progress",G_CALLBACK(download_progress), tb);

        g_signal_connect(G_OBJECT(download), "finished",G_CALLBACK(download_handle_finished), NULL);

        g_object_ref(download);
        g_signal_connect(G_OBJECT(tb), "activate",G_CALLBACK(download_cancel), download);


    }

    g_free(sug);
    g_free(path);
    g_free(path2);

    return FALSE;
}

void
download_cancel( GtkWidget *tb,WebKitDownload *download)
{

    webkit_download_cancel(download);
    g_object_unref(download);

    gtk_widget_destroy(GTK_WIDGET(tb));
}

void
download_handle_finished(WebKitDownload *download, gpointer data)
{
    downloads--;
}


void
download_progress( WebKitDownload *download,GParamSpec *pspec,    GtkWidget *tb)
{
    WebKitURIResponse *resp;

    const gchar *uri;
    gchar *t, *filename, *base;
    gdouble p,size_mb;
    int b;
    p = webkit_download_get_estimated_progress(download);
    p = p > 1 ? 1 : p;
    p = p < 0 ? 0 : p;
    p *= 100;
//    b = (int) p;



    resp = webkit_download_get_response(download);
    size_mb = webkit_uri_response_get_content_length(resp) / 1e6;

    uri = webkit_download_get_destination(download);
    filename = g_filename_from_uri(uri, NULL, NULL);

    base = g_path_get_basename(filename);
        t = g_strdup_printf("%s (%.0f%% of %.1f MB)", base, p, size_mb);
        g_free(filename);
        g_free(base);

    gtk_menu_item_set_label(GTK_MENU_ITEM(tb), t);
    g_free(t);



}




void
loadurl(Client *c,  gchar *url)
{
    gchar *link;
    gchar *nospaces;

    nospaces = g_strchug(url);
    link = g_ascii_strdown(nospaces, -1);

    if (!g_str_has_prefix(link, "http:") &&
        !g_str_has_prefix(link, "https:") &&
        !g_str_has_prefix(link, "file:") &&
        !g_str_has_prefix(link, "about:")) {
        g_free(link);
        link = g_strdup_printf("http://%s", url);
    } else

       link = g_strdup(url);



  // link = soup_uri_normalize(url,NULL);
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
close_find(Client *c) {

   gtk_entry_set_text(GTK_ENTRY(c->entry_find), "");
 find(c->entry_find,c);
    gtk_widget_hide(c->box_find);
    gtk_widget_grab_focus(GTK_WIDGET(c->webView));

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
          	     goback(c);

                    isbackforward= 1;
                    return TRUE;

                case SURFER_FORWARD_KEY:
                     goforward(c);

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
                   }
                   else {

                   gtk_widget_hide(c->box_open);
                   gtk_widget_grab_focus(GTK_WIDGET(c->webView));
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

void
goback(Client *c){
 
    const gchar *back_uri;
    WebKitBackForwardList *history;
    WebKitBackForwardListItem *back_item;

   if(webkit_web_view_can_go_back(c->webView)){
   //webkit_web_view_go_back(WEBKIT_WEB_VIEW(c->webView));

    history = webkit_web_view_get_back_forward_list(c->webView);
    back_item = webkit_back_forward_list_get_back_item(history);
    //back_uri = webkit_back_forward_list_item_get_original_uri(back_item);	
      
    webkit_web_view_go_to_back_forward_list_item(c->webView,back_item);             
    //webkit_web_view_load_uri (c->webView, back_uri);

   
   // printf("%s\n",back_uri);
    }
    else;
 
}



void
goforward(Client *c){
 
    const gchar *forward_uri;
    WebKitBackForwardList *history;
    WebKitBackForwardListItem *forward_item;

    if(webkit_web_view_can_go_forward(c->webView)){
 // webkit_web_view_go_forward(WEBKIT_WEB_VIEW(c->webView));


    history = webkit_web_view_get_back_forward_list (c->webView);
    forward_item = webkit_back_forward_list_get_forward_item (history);
    //forward_uri = webkit_back_forward_list_item_get_original_uri (forward_item);
      
    webkit_web_view_go_to_back_forward_list_item(c->webView,forward_item);     
    //webkit_web_view_load_uri (c->webView, forward_uri);

 }
  else;
}



gboolean
decide_policy( WebKitWebView *v,WebKitPolicyDecision *decision, WebKitPolicyDecisionType type,Client *c) {


    switch (type) {
        case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION:
             decide_navaction(decision,c);
            break;

        case WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION:
           decide_newwindow(decision,c);
            break;

        case WEBKIT_POLICY_DECISION_TYPE_RESPONSE:
           decide_response(decision,c);
            break;

        default:
            webkit_policy_decision_ignore(decision);
           break;
    }
    return TRUE;
}

void decide_navaction(WebKitPolicyDecision *decision,Client *c) {

    WebKitNavigationType navigation_type;
    WebKitNavigationPolicyDecision *navigationDecision;
    WebKitNavigationAction *navigation_action;
    WebKitURIRequest *request;
    guint button, mods;
    gchar *link;
    gchar *t;
    Client *rc;


    navigation_action = webkit_navigation_policy_decision_get_navigation_action(WEBKIT_NAVIGATION_POLICY_DECISION(decision));
    request = webkit_navigation_action_get_request(navigation_action);
    navigation_type = webkit_navigation_action_get_navigation_type(navigation_action);

    t =  ( gchar *) webkit_uri_request_get_uri(request);
    mods = webkit_navigation_action_get_modifiers(navigation_action);
    button = webkit_navigation_action_get_mouse_button(navigation_action);



    if (navigation_type == WEBKIT_NAVIGATION_TYPE_LINK_CLICKED && button == 1 && mods & SURFER_META_MASK) {

       webkit_policy_decision_ignore(decision);
   //                 printf("new\n");
       rc = client_new(c);

       loadurl(rc,t);
      // g_free(t);
     }
    else webkit_policy_decision_use(decision);
 //    printf("no\n");
}

void decide_newwindow(WebKitPolicyDecision *decision,Client *c)
{

    WebKitNavigationType navigation_type;
    WebKitNavigationAction *navigation_action;
    WebKitURIRequest *request;
    gchar *t;
    Client *rc;

    navigation_action = webkit_navigation_policy_decision_get_navigation_action(WEBKIT_NAVIGATION_POLICY_DECISION(decision));
    request = webkit_navigation_action_get_request(navigation_action);
    navigation_type =webkit_navigation_action_get_navigation_type(navigation_action);

    switch (navigation_type) {
        case WEBKIT_NAVIGATION_TYPE_LINK_CLICKED:

             t = (gchar *) webkit_uri_request_get_uri(request);
             rc = client_new(c);
             loadurl(rc,t);
        case WEBKIT_NAVIGATION_TYPE_FORM_SUBMITTED: 
        case WEBKIT_NAVIGATION_TYPE_BACK_FORWARD:
        case WEBKIT_NAVIGATION_TYPE_RELOAD:
        case WEBKIT_NAVIGATION_TYPE_FORM_RESUBMITTED:
        case WEBKIT_NAVIGATION_TYPE_OTHER:
        default:

           break;
     }
    webkit_policy_decision_ignore(decision);

}

void decide_response(WebKitPolicyDecision *decision,Client *c){

   guint status;
   WebKitURIResponse *response;

   response = webkit_response_policy_decision_get_response(WEBKIT_RESPONSE_POLICY_DECISION(decision));
   status = webkit_uri_response_get_status_code(response);


   if (webkit_response_policy_decision_is_mime_type_supported(WEBKIT_RESPONSE_POLICY_DECISION(decision)))
        webkit_policy_decision_use(decision);
   else if (SOUP_STATUS_IS_SUCCESSFUL(status) || status == SOUP_STATUS_NONE)
        webkit_policy_decision_download(decision);
   else
        webkit_policy_decision_ignore(decision);

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


    p = gtk_entry_get_text(GTK_ENTRY(c->entry_find));
    search_text = g_strdup(p);

    if(search_text != NULL){

    webkit_find_controller_search(c->fc, search_text,WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE | WEBKIT_FIND_OPTIONS_WRAP_AROUND, G_MAXUINT);
    g_free(search_text);

    }

}


void
find_back(GtkWidget * widget,Client *c){


webkit_find_controller_search_previous(c->fc);


}


void
enablejs_cb(GtkWidget * widget,Client *c){

   //GdkColor color;

   if (enablejs==1){
   g_object_set(G_OBJECT(c->settings),"enable-javascript", FALSE, NULL);
   enablejs=0;
  // gdk_color_parse ("red", &color);
  // gtk_widget_modify_bg ( GTK_WIDGET(c->button_js), GTK_STATE_NORMAL, &color);
   

   }
   else{
   //gdk_color_parse ("red", &color);
   //gtk_widget_modify_bg ( GTK_WIDGET(c->button_js), GTK_STATE_NORMAL, &color);
 

   g_object_set(G_OBJECT(c->settings),"enable-javascript", TRUE, NULL);
   enablejs=1;
   }
}



void
allow_tls_cert(Client *c)
{
  SoupURI *uri;
  gchar *url;
  gchar *tls_message;

  //if (webkit_web_view_get_page_id (WEBKIT_WEB_VIEW (c->webView)) != page_id)
    //return;

 g_assert (G_IS_TLS_CERTIFICATE (c->certificate));
  g_assert (c->tls_error_failing_uri != NULL);



  uri = soup_uri_new (c->tls_error_failing_uri);
  webkit_web_context_allow_tls_certificate_for_host (webkit_web_view_get_context(c->webView),c->certificate,uri->host);

//  url = (gchar *)webkit_web_view_get_uri(WEBKIT_WEB_VIEW(c->webView));
/*
tls_message = g_strdup_printf("tls cert error for site ignored");


gtk_entry_set_text(GTK_ENTRY(c->entry_open), tls_message);

gtk_widget_show_all(c->box_open);
*/

  //loadurl(c, url);
//  soup_uri_free (uri);
//  g_free(url);
}



void
display_webview(WebKitWebView *rc, Client *c)
{
 //   printf("new\n");
    gtk_widget_show_all(c->main_window);
    gtk_widget_grab_focus(GTK_WIDGET(c->webView));
    gtk_widget_hide(c->box_find);
    gtk_widget_hide(c->box_open);
}

WebKitWebView *create_request(WebKitWebView *rv,WebKitNavigationAction *navact, Client *c)
{
    Client *rc;

     rc = client_new(c);
    // rc->webView = rv;
    return rc->webView;
}


gboolean setup(){


  
    gchar *downloadsfilename,*surferdirfilename;

    FILE *File,*File1;
    char buffer[256] = "<!DOCTYPE html><html><head><meta charset=utf8><style>body {background-color: #000009;}p\
    {color: yellow;} a:link { color: #00e900; }</style></head><body><p>";


   

    downloadsfilename = g_strdup_printf("%s", SURFER_DOWNLOADS);

    downloads_dir = g_build_filename(getenv("HOME"), downloadsfilename, NULL);
    g_free(downloadsfilename);


    surferdirfilename = g_strdup_printf("%s", SURFER_DIR);

    surfer_dir = g_build_filename(getenv("HOME"), surferdirfilename, NULL);
    g_free(surferdirfilename);



  if (!g_file_test(downloads_dir, G_FILE_TEST_EXISTS)) {
        mkdir(downloads_dir, 0700);

    }

  if (!g_file_test(surfer_dir, G_FILE_TEST_EXISTS)) {
        mkdir(surfer_dir, 0700);

    }    

 
    favpath = g_build_filename(surfer_dir,"fav", NULL);
 
    histpath = g_build_filename(surfer_dir, "hist", NULL);
   

    if (!g_file_test(favpath, G_FILE_TEST_EXISTS)) {
        File = fopen(favpath, "wb+");
        fprintf(File, "%s", buffer);
        fclose(File);
            
    }


  //if (HISTORY_ENABLE == 1)

    if (!g_file_test(histpath, G_FILE_TEST_EXISTS)) {
        File1 = fopen(histpath, "wb+");
        fprintf(File1, "%s", buffer);
        fclose(File1);
        
    }


    history = (gchar *) g_strdup_printf("file://%s", histpath);

    home = (gchar *) g_strdup_printf("file://%s", favpath);


return TRUE;
}


int main(int argc, char *argv[]) {
 
    Client *c;
    int i;
    FILE *File1;
    gchar *link;
    char textdate[100];

    gtk_init(&argc, &argv);


    menu =gtk_menu_new();
    menuitem1 = gtk_menu_item_new_with_label("Click to cancel");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem1);
    //gtk_widget_show_all(menu);

    setup();
    c = client_new(NULL);

     if (argc > 1){

         for (i = 1; i < argc; i++) {
         link = (gchar *) argv[i];
         loadurl(c,link);
         }
     }

     else

     loadurl(c,home);


     if (HISTORY_ENABLE == 1){

     time_t now = time(NULL);
     struct tm *t = localtime(&now);

     strftime(textdate, sizeof(textdate)-1, "%d %m %Y %H:%M", t);

     File1 = fopen(histpath, "a");
     fprintf(File1, "%s",textdate);
     fclose(File1);


     }


    gtk_main();

    return 0;
}

