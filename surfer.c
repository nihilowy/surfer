

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <gdk/gdk.h>

#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <gdk/gdkkeysyms.h>
#include <gio/gio.h>
#include <webkit2/webkit2.h>

static gchar *ensure_uri_scheme(const gchar *);
static void destroyWindowCb(GtkWidget *obj, gpointer data);
static void tls_certs(WebKitWebContext *);
static gboolean client_destroy_request(WebKitWebView *, gpointer data);
static gboolean keyboard(GtkWidget *, GdkEvent *, gpointer);
static void changed_title(GObject *, GParamSpec *, gpointer);
static  void client_new( gchar *);
static gboolean decide_policy(WebKitWebView *, WebKitPolicyDecision *, WebKitPolicyDecisionType, gpointer);

static void find(GtkWidget * ,gpointer);
static void openlink(GtkWidget *,gpointer);

static gint clients = 0;

gchar  *home;
gchar  *favpath;
//static WebKitWebContext *web_context;
struct Client {

GtkWidget *main_window;
GtkWidget *webView;
GtkWidget *entry;
GtkWidget *entry_open;
GtkWidget *box;
GtkWidget  *window;
GtkWidget *box_open;
GtkWidget  *window_open;
int f;


};

static void destroyWindowCb(GtkWidget *obj, gpointer data)
{
struct Client *c = (struct Client *)data;
    free(c);
    clients--;

    if (clients == 0)
        gtk_main_quit();
}


gboolean
client_destroy_request( WebKitWebView *webView, gpointer data)
{

         struct Client *c = (struct Client *)data;

    gtk_widget_destroy(c->main_window);

    return TRUE;

}

/*
WebKitWebView *

client_request(WebKitWebView *web_view)
{
    return client_new(NULL);
}

*/


//from lariza browser
gchar *
ensure_uri_scheme(const gchar *t)
{
    gchar *link;

    link = g_ascii_strdown(t, -1);
    if (!g_str_has_prefix(link, "http:") &&
        !g_str_has_prefix(link, "https:") &&
        !g_str_has_prefix(link, "file:") &&
        !g_str_has_prefix(link, "about:"))
    {
        g_free(link);
        link = g_strdup_printf("http://%s", t);
        return link;
    }
    else
        return g_strdup(t);
}


void
 client_new( gchar *uri){
    struct Client *c;
    gchar *link;
    WebKitWebContext *web_context;

 c = malloc(sizeof(struct Client));

  gboolean enabled =  1;

    c->f =0;
    c->main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(c->main_window), 1100, 700);
    c->webView = webkit_web_view_new();
  web_context= webkit_web_view_get_context(WEBKIT_WEB_VIEW(c->webView));


    gtk_container_add(GTK_CONTAINER(c->main_window), GTK_WIDGET(c->webView));

    g_signal_connect(c->main_window, "destroy", G_CALLBACK(destroyWindowCb), c);
  g_signal_connect(c->webView, "notify::title", G_CALLBACK(changed_title), c);
   g_signal_connect(c->webView, "close", G_CALLBACK(client_destroy_request), c	);
   g_signal_connect(c->webView, "key-press-event",G_CALLBACK(keyboard), c);
   //g_signal_connect(c->webView, "create", G_CALLBACK(client_request), NULL);
    g_signal_connect(c->webView, "decide-policy",G_CALLBACK(decide_policy), c);




    gtk_widget_grab_focus(GTK_WIDGET(c->webView));
    gtk_widget_show_all(c->main_window);
     tls_certs(web_context);

    WebKitSettings *settings = webkit_settings_new ();
 //    char *value = "Googlebot/2.1";
//g_object_set (settings, "user-agent", &value, NULL);	
     webkit_web_view_set_settings (WEBKIT_WEB_VIEW(c->webView), settings);
webkit_settings_set_enable_webgl( settings,  enabled);
g_object_set (G_OBJECT(settings), "enable-developer-extras", TRUE, NULL);





 if (uri != NULL)
    {
        link= ensure_uri_scheme(uri);    
     webkit_web_view_load_uri(WEBKIT_WEB_VIEW(c->webView), link);
g_free(link);
}


    c->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(c->window), 300, 70);
    gtk_window_set_title(GTK_WINDOW(c->window), "find");
      c->entry = gtk_entry_new ();

 c->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(c->box), c->entry, TRUE, TRUE, 0);
      gtk_container_add(GTK_CONTAINER(c->window), c->box);
g_signal_connect(G_OBJECT(c->window), "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    g_signal_connect(G_OBJECT(c->entry), "activate",G_CALLBACK(find), c);

     c->window_open = gtk_window_new(GTK_WINDOW_TOPLEVEL);
     gtk_window_set_default_size(GTK_WINDOW(c->window_open), 350, 70);
     gtk_window_set_title(GTK_WINDOW(c->window_open), "open");
      c->entry_open= gtk_entry_new ();

 c->box_open = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(c->box_open),c->entry_open, TRUE, TRUE, 0);
      gtk_container_add(GTK_CONTAINER(c->window_open), c->box_open);
g_signal_connect(G_OBJECT(c->window_open), "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    g_signal_connect(G_OBJECT(c->entry_open), "activate",G_CALLBACK(openlink), c);


  clients++;
  //  webkit_web_context_set_web_extensions_directory(webkit_web_context_get_default(), c);
// return WEBKIT_WEB_VIEW(c->webView);
}



void
changed_title(GObject *obj, GParamSpec *pspec, gpointer data)
{
    const gchar *t;
    struct Client *c = (struct Client *)data;


    t = webkit_web_view_get_title(WEBKIT_WEB_VIEW(c->webView));


    gtk_window_set_title(GTK_WINDOW(c->main_window), t);
}





gboolean
keyboard( GtkWidget *widget, GdkEvent *event, gpointer data)
{
    struct Client *c = (struct Client *)data;
  
 //WebKitWebContext *web_context = webkit_web_view_get_context(WEBKIT_WEB_VIEW(c->webView));

WebKitWebInspector *inspector;

   FILE *File;
  char buffer[256]= "</body></html>";

 //  gchar *link;
  const gchar * url;
const gchar* tmp;  

if (event->type == GDK_KEY_PRESS)
  {
        if (((GdkEventKey *)event)->state & GDK_MOD1_MASK)
        {
            switch (((GdkEventKey *)event)->keyval)
            {
                case GDK_KEY_q:  
                    gtk_widget_destroy(c->main_window);
                    return TRUE;


                 case GDK_KEY_Left:
                 webkit_web_view_go_back(WEBKIT_WEB_VIEW(c->webView));
                        return TRUE;

                case GDK_KEY_Right:
                webkit_web_view_go_forward(WEBKIT_WEB_VIEW(c->webView));
                        return TRUE;

         case GDK_KEY_s:
         inspector = webkit_web_view_get_inspector (WEBKIT_WEB_VIEW(c->webView));
webkit_web_inspector_show (inspector);

                        return TRUE;


                    case GDK_KEY_o:
              gtk_widget_show_all(c->window_open);

              url = webkit_web_view_get_uri (WEBKIT_WEB_VIEW(c->webView));
             gtk_entry_set_text(GTK_ENTRY(c->entry_open),url);
		//g_free(url);
                        return TRUE;
                   case GDK_KEY_n:
                      client_new(home);
                        return TRUE;
                 case GDK_KEY_h:
                       webkit_web_view_load_uri(WEBKIT_WEB_VIEW(c->webView), home);
			return TRUE;
 		case GDK_KEY_r:
                       webkit_web_view_reload(WEBKIT_WEB_VIEW(c->webView));
                        return TRUE;
                    case GDK_KEY_f:
                        gtk_widget_show_all(c->window);
                       return TRUE;
                    case GDK_KEY_b:

                 File = fopen(favpath, "a");
                 //if(File== NULL)
                 tmp=webkit_web_view_get_uri(WEBKIT_WEB_VIEW(c->webView));
                 fprintf(File, "<a href=\"%s\" >%s</a><br>", (char *) tmp,(char *) tmp);
                  fprintf(File, "%s\n",buffer);
                  fclose(File);
			          return TRUE;

            }
        }
	  else if (((GdkEventKey *)event)->keyval == GDK_KEY_F11)
	  {
	      if(c->f  == 0)
	      {
	        gtk_window_fullscreen(GTK_WINDOW(c->main_window));
	        c->f = 1;
	      }
	     else 
	      {
			             gtk_window_unfullscreen(GTK_WINDOW(c->main_window));
	                      c->f = 0;
	      }
           return TRUE;
       }
       else if (((GdkEventKey *)event)->keyval == GDK_KEY_F2)
       {
		    webkit_web_view_go_back(WEBKIT_WEB_VIEW(c->webView));
           return TRUE;
        }
         else if (((GdkEventKey *)event)->keyval ==GDK_KEY_F3)
       {
		  webkit_web_view_go_forward(WEBKIT_WEB_VIEW(c->webView));
           return TRUE;
        }
            else if (((GdkEventKey *)event)->keyval == GDK_KEY_Escape)
        {
            webkit_web_view_stop_loading(WEBKIT_WEB_VIEW(c->webView));
           return TRUE;
        }
  }
  return FALSE;
}

gboolean
decide_policy( WebKitWebView *webView, WebKitPolicyDecision *decision,
              WebKitPolicyDecisionType type, gpointer data)
{
    WebKitResponsePolicyDecision *r;
    WebKitNavigationType navigation_type;
	WebKitNavigationPolicyDecision *navigationDecision;
   WebKitNavigationAction *navigation_action;
     WebKitURIRequest *request;
    	guint button, mods;
   gchar *link;
   const gchar *t;
 //struct Client *c = (struct Client *)data;

    switch (type)
    {
		case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION: 
	 navigationDecision= WEBKIT_NAVIGATION_POLICY_DECISION(decision);
	  navigation_action = webkit_navigation_policy_decision_get_navigation_action (navigationDecision);
     request = webkit_navigation_action_get_request (navigation_action);
     navigation_type = webkit_navigation_action_get_navigation_type (navigation_action);
      if ( navigation_type== WEBKIT_NAVIGATION_TYPE_LINK_CLICKED){
          mods = webkit_navigation_action_get_modifiers(navigation_action);
          button = webkit_navigation_action_get_mouse_button(navigation_action);

if (  button == 1 &&  mods & GDK_CONTROL_MASK ) {
    t= (gchar*) webkit_uri_request_get_uri (request);
            link = ensure_uri_scheme(t);
           client_new(link);
          webkit_policy_decision_ignore(decision);
              return TRUE;
	   }
else	   webkit_policy_decision_use(decision);
	         return TRUE;
   }
 
		 break;

      case  WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION:
       navigationDecision= WEBKIT_NAVIGATION_POLICY_DECISION(decision);
	  navigation_action = webkit_navigation_policy_decision_get_navigation_action (navigationDecision);
     request = webkit_navigation_action_get_request (navigation_action);
     navigation_type = webkit_navigation_action_get_navigation_type (navigation_action);
      if ( navigation_type== WEBKIT_NAVIGATION_TYPE_LINK_CLICKED){

    t= (gchar*) webkit_uri_request_get_uri (request);
            link = ensure_uri_scheme(t);
           client_new(link);
          webkit_policy_decision_ignore(decision);
}
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


void openlink(GtkWidget *widget,gpointer data)
{
  struct Client *c = (struct Client *)data;
gchar *link;
const gchar *p;
p= gtk_entry_get_text(GTK_ENTRY(c->entry_open));

  link= ensure_uri_scheme(p);
     webkit_web_view_load_uri(WEBKIT_WEB_VIEW(c->webView), link);
g_free(link);

gtk_widget_hide (c->window_open);
}


void find( GtkWidget *widget, gpointer data)
{

  struct Client *c = (struct Client *)data;

 static gchar *search_text;
const gchar *p;

   WebKitWebView *web_View= WEBKIT_WEB_VIEW(c->webView);
WebKitFindController *fc = webkit_web_view_get_find_controller(web_View);

p= gtk_entry_get_text(GTK_ENTRY(c->entry));
 //?   gtk_entry_set_text(GTK_ENTRY(entry),p);
            gtk_widget_grab_focus((c->webView));
                     if (search_text != NULL)
                           g_free(search_text);
                      search_text = g_strdup(p );
webkit_find_controller_search(fc, search_text,WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE | WEBKIT_FIND_OPTIONS_WRAP_AROUND,G_MAXUINT);

}

	void
tls_certs(WebKitWebContext *wc)
{
    GDir *directory;
    GTlsCertificate *certificate;
    const gchar *basedir,  *keyfile;
   

   
 basedir = g_build_filename(g_get_user_config_dir(),  "/usr/bin/bro", "certs",NULL);

   directory = g_dir_open(basedir, 0, NULL);
    if (directory != NULL)
    {

	keyfile = g_build_filename(g_get_user_config_dir(), "/usr/bin/bro", "certs",NULL);
	
	certificate = g_tls_certificate_new_from_file(keyfile,NULL);
	 webkit_web_context_allow_tls_certificate_for_host(wc, certificate, keyfile);
       g_dir_close(directory);
    }
}

int main(int argc, char *argv[])

{
   int i;
    gchar *cookiefilename,*cookiepath,*Cookie;  
    gchar *favfilename;
FILE *File,*File1;
  char buffer[256]= "<html><head></head><body bgcolor=black>";

    gtk_init(&argc, &argv);
WebKitWebContext *web_context;
 web_context = webkit_web_context_get_default();

webkit_cookie_manager_set_accept_policy(
			webkit_web_context_get_cookie_manager(web_context),
			WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY);
//mkdir (".cookies");
	//tell webkit where to store cookies
 cookiefilename=g_strdup_printf("%s", ".cookies");
cookiepath = g_build_filename(getenv("HOME"), cookiefilename, NULL);
    g_free(cookiefilename);
    if (!g_file_test(cookiepath, G_FILE_TEST_EXISTS))
{        
mkdir(cookiepath, 0700);

cookiefilename=g_strdup_printf("%s", "cookie");
Cookie = g_build_filename(cookiepath,cookiefilename, NULL);
File1 = fopen(Cookie,"wb+");
                  fclose(File1);
 g_free(cookiefilename);
}

webkit_cookie_manager_set_persistent_storage(webkit_web_context_get_cookie_manager(web_context),".cookies/cookie",WEBKIT_COOKIE_PERSISTENT_STORAGE_SQLITE);

favfilename=g_strdup_printf("%s", ".fav");
favpath = g_build_filename(getenv("HOME"), favfilename, NULL);
    g_free(favfilename);

    if (!g_file_test(favpath, G_FILE_TEST_EXISTS))
     //   mkdir(favpath, 0600);
{

File = fopen(favpath,"wb+");
     fprintf(File, "%s",buffer);
                   // g_free(&tmp);   
                  fclose(File);
}

home = g_build_filename("file:/", favpath, NULL);
gchar *link;
      
if (argc > 1)
{
    for (i = 1; i < argc; i++)
{

link = argv[i];

client_new(link);
}
}
else
client_new(home);

    gtk_main();

  return 0;

}







