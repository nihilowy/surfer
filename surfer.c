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
#define SURFER_BACK_KEY             GDK_KEY_H
#define SURFER_FORWARD_KEY          GDK_KEY_L
#define SURFER_STOP_KEY             GDK_KEY_Escape
#define SURFER_RELOAD_KEY           GDK_KEY_r
#define SURFER_FIND_KEY             GDK_KEY_slash
#define SURFER_HOME_KEY             GDK_KEY_h
#define SURFER_BOOKMARK_KEY         GDK_KEY_b
#define SURFER_INSPECTOR_KEY        GDK_KEY_i
#define SURFER_ZOOM_IN_KEY          GDK_KEY_equal
#define SURFER_ZOOM_OUT_KEY         GDK_KEY_minus
#define SURFER_FULLSCREEN_KEY       GDK_KEY_F11
#define SURFER_SCROLL_DOWN_KEY      GDK_KEY_j
#define SURFER_SCROLL_UP_KEY        GDK_KEY_k
#define SURFER_SCROLL_PAGE_DOWN_KEY GDK_KEY_D
#define SURFER_SCROLL_PAGE_UP_KEY   GDK_KEY_U
#define SURFER_STYLE_KEY            GDK_KEY_s
#define SURFER_COOKIE_POLICY        WEBKIT_COOKIE_POLICY_ACCEPT_ALWAYS
#define USER_STYLESHEET_FILENAME	"/usr/share/surfer/black.css"

static gchar *ensure_uri_scheme(const gchar *);

static void destroy_window(GtkWidget *obj, gpointer data);

static void tls_certs(WebKitWebContext *);

static gboolean client_destroy_request(WebKitWebView *, gpointer data);

static gboolean keyboard(GtkWidget *, GdkEvent *, gpointer);

static void changed_title(GObject *, GParamSpec *, gpointer);

static void client_new(gchar *);

//static WebKitWebView *create_request(gpointer);
static gboolean decide_policy(WebKitWebView *, WebKitPolicyDecision *, WebKitPolicyDecisionType, gpointer);

static void find(GtkWidget *, gpointer);

static void openlink(GtkWidget *, gpointer);

static void user_style(gpointer);

static gint clients = 0;
gchar *home;
gchar *favpath;


struct Client {
    GtkWidget *main_window;
    GtkWidget *entry;
    GtkWidget *entry_open;
    GtkWidget *box;
    GtkWidget *window;
    GtkWidget *box_open;
    GtkWidget *window_open;
    WebKitWebView *webView;
    int f;
    int s;
};

static void
destroy_window(GtkWidget *obj __attribute__((__unused__)), gpointer data) {
    struct Client *c = (struct Client *) data;
    free(c);
    clients--;

    if (clients == 0)
        gtk_main_quit();
}

gboolean
client_destroy_request(WebKitWebView *webView __attribute__((__unused__)), gpointer data) {
    struct Client *c = (struct Client *) data;

    gtk_widget_destroy(c->main_window);

    return TRUE;
}

//from lariza browser
gchar *
ensure_uri_scheme(const gchar *t) {
    gchar *link;

    link = g_ascii_strdown(t, -1);
    if (!g_str_has_prefix(link, "http:") &&
        !g_str_has_prefix(link, "https:") &&
        !g_str_has_prefix(link, "file:") &&
        !g_str_has_prefix(link, "about:")) {
        g_free(link);
        link = g_strdup_printf("http://%s", t);
        return link;
    } else
        return g_strdup(t);
}

void
client_new(gchar *uri) {
    struct Client *c;
    gchar *link;
    gchar *cookies_path = g_build_filename(getenv("HOME"), ".cookies", NULL),
          *cookie_file = g_build_filename(cookies_path, "cookie", NULL);
    gchar *cachedir;
    FILE *cookie_file_handler;
    WebKitWebContext *web_context;
    cachedir = g_build_filename(getenv("HOME"), ".cache", NULL);

    web_context = webkit_web_context_new_with_website_data_manager(
            webkit_website_data_manager_new(
                    "base-cache-directory", cachedir,
                    "base-data-directory", cachedir,
                    NULL));
   

    c = malloc(sizeof(struct Client));

    gboolean enabled = 1;

    c->f = 0;
    c->s = 0;
    c->main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(c->main_window), 1100, 700);

    

    c->webView = WEBKIT_WEB_VIEW(webkit_web_view_new_with_user_content_manager(webkit_user_content_manager_new()));

    web_context = webkit_web_view_get_context(WEBKIT_WEB_VIEW(c->webView));

    gtk_container_add(GTK_CONTAINER(c->main_window), GTK_WIDGET(c->webView));

    g_signal_connect(c->main_window, "destroy", G_CALLBACK(destroy_window), c);
    g_signal_connect(c->webView, "notify::title", G_CALLBACK(changed_title), c);
    g_signal_connect(c->webView, "close", G_CALLBACK(client_destroy_request), c);
    g_signal_connect(c->webView, "key-press-event", G_CALLBACK(keyboard), c);
    // g_signal_connect(c->webView, "create", G_CALLBACK(create_request), c);
    g_signal_connect(c->webView, "decide-policy", G_CALLBACK(decide_policy), c);

    gtk_widget_grab_focus(GTK_WIDGET(c->webView));
    gtk_widget_show_all(c->main_window);
    tls_certs(web_context);

   WebKitSettings *settings = webkit_settings_new();
    //char *value = "Googlebot/2.1";
    //g_object_set(settings, "user-agent", &value, NULL);
    webkit_web_view_set_settings(c->webView, settings);
    webkit_settings_set_enable_webgl(settings, enabled);
    g_object_set(G_OBJECT(settings), "enable-developer-extras", TRUE, NULL);

    webkit_web_context_set_tls_errors_policy(web_context, WEBKIT_TLS_ERRORS_POLICY_IGNORE);

    webkit_cookie_manager_set_accept_policy(
            webkit_web_context_get_cookie_manager(web_context),
            SURFER_COOKIE_POLICY);

    //tell webkit where to store cookies
    if (!g_file_test(cookies_path, G_FILE_TEST_EXISTS)) {
        mkdir(cookies_path, 0700);
        cookie_file_handler = fopen(cookie_file, "wb+");
        fclose(cookie_file_handler);
    }

    webkit_cookie_manager_set_persistent_storage(
            webkit_web_context_get_cookie_manager(web_context), cookie_file,
            WEBKIT_COOKIE_PERSISTENT_STORAGE_TEXT); // as mentioned in surf sources only text storage works

    if (uri != NULL) {
        link = ensure_uri_scheme(uri);
        webkit_web_view_load_uri(c->webView, link);
        g_free(link);
    }

    c->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(c->window), 300, 70);
    gtk_window_set_title(GTK_WINDOW(c->window), "find");
    c->entry = gtk_entry_new();

    c->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(c->box), c->entry, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(c->window), c->box);
    g_signal_connect(G_OBJECT(c->window), "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    g_signal_connect(G_OBJECT(c->entry), "activate", G_CALLBACK(find), c);

    c->window_open = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(c->window_open), 350, 70);
    gtk_window_set_title(GTK_WINDOW(c->window_open), "open");
    c->entry_open = gtk_entry_new();

    c->box_open = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(c->box_open), c->entry_open, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(c->window_open), c->box_open);
    g_signal_connect(G_OBJECT(c->window_open), "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    g_signal_connect(G_OBJECT(c->entry_open), "activate", G_CALLBACK(openlink), c);

    clients++;
    //webkit_web_context_set_web_extensions_directory(webkit_web_context_get_default(), c);
    
}




void
user_style(gpointer data){
	struct Client *c = (struct Client *) data;
	gchar *contents;
 
	g_file_get_contents(USER_STYLESHEET_FILENAME,&contents,NULL,NULL);
	webkit_user_content_manager_add_style_sheet(
	    webkit_web_view_get_user_content_manager(c->webView),
	    webkit_user_style_sheet_new(contents,
	    WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
	    WEBKIT_USER_STYLE_LEVEL_USER,
	    NULL, NULL));

	g_free(contents);
}



void
changed_title(GObject *obj __attribute__((__unused__)), GParamSpec *pspec __attribute__((__unused__)), gpointer data) {
    const gchar *t;
    struct Client *c = (struct Client *) data;

    t = webkit_web_view_get_title(c->webView);
    gtk_window_set_title(GTK_WINDOW(c->main_window), t);
}

gboolean
keyboard(GtkWidget *widget __attribute__((__unused__)), GdkEvent *event, gpointer data) {
    struct Client *c = (struct Client *) data;
    WebKitWebInspector *inspector;
    FILE *File;
    char buffer[256] = "</body></html>";
    //gchar *link;
    const gchar *url;
    const gchar *tmp;
    gdouble z;
    guint meta_key_pressed;
    int key_pressed;

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
                    gtk_widget_destroy(c->main_window);
                    return TRUE;

                case SURFER_BACK_KEY:
                    webkit_web_view_go_back(c->webView);
                    return TRUE;

                case SURFER_FORWARD_KEY:
                    webkit_web_view_go_forward(c->webView);
                    return TRUE;

                case SURFER_INSPECTOR_KEY:
                    inspector = webkit_web_view_get_inspector(c->webView);
                    webkit_web_inspector_show(inspector);
                    return TRUE;

                case SURFER_OPEN_KEY:
                    gtk_widget_show_all(c->window_open);
                    url = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(c->webView));
                    gtk_entry_set_text(GTK_ENTRY(c->entry_open), url);
                    //g_free(url);
                    return TRUE;

                case SURFER_NEW_WINDOW_KEY:
                    client_new(home);
                    return TRUE;

                case SURFER_HOME_KEY:
                    webkit_web_view_load_uri(c->webView, home);
                    return TRUE;

                case SURFER_RELOAD_KEY:
                    webkit_web_view_reload(c->webView);
                    return TRUE;

                case SURFER_FIND_KEY:
                    gtk_widget_show_all(c->window);
                    return TRUE;

                case SURFER_BOOKMARK_KEY:
                    File = fopen(favpath, "a");
                    //if(File== NULL)
                    tmp = webkit_web_view_get_uri(c->webView);
                    fprintf(File, "<a href=\"%s\" >%.110s</a><br>", (char *) tmp, (char *) tmp);
                    fprintf(File, "%s\n", buffer);
                    fclose(File);
                    return TRUE;

                case SURFER_ZOOM_OUT_KEY:
                    z = webkit_web_view_get_zoom_level(c->webView);
                    webkit_web_view_set_zoom_level(c->webView, z - 0.1);
                    return TRUE;

                case SURFER_ZOOM_IN_KEY:
                    z = webkit_web_view_get_zoom_level(c->webView);
                    webkit_web_view_set_zoom_level(c->webView, z + 0.1);
                    return TRUE;
		case SURFER_STYLE_KEY:
                   
		if (c->s == 0) {
                 user_style(c);
                        c->s = 1;
                    } else {
                      webkit_user_content_manager_remove_all_style_sheets(
			    webkit_web_view_get_user_content_manager(c->webView));
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
                    webkit_web_view_stop_loading(c->webView);
                    return TRUE;

                default:
                    return FALSE;
            }
        }
    }
    return FALSE;
}

gboolean
decide_policy(WebKitWebView *webView __attribute__((__unused__)), WebKitPolicyDecision *decision,
              WebKitPolicyDecisionType type, gpointer data __attribute__((__unused__)) ) {
    WebKitResponsePolicyDecision *r;
    WebKitNavigationType navigation_type;
    WebKitNavigationPolicyDecision *navigationDecision;
    WebKitNavigationAction *navigation_action;
    WebKitURIRequest *request;
    guint button, mods;
    gchar *link;
    const gchar *t;

    switch (type) {
        case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION:
            navigationDecision = WEBKIT_NAVIGATION_POLICY_DECISION(decision);
            navigation_action = webkit_navigation_policy_decision_get_navigation_action(navigationDecision);
            request = webkit_navigation_action_get_request(navigation_action);
            navigation_type = webkit_navigation_action_get_navigation_type(navigation_action);

            if (navigation_type == WEBKIT_NAVIGATION_TYPE_LINK_CLICKED) {
                mods = webkit_navigation_action_get_modifiers(navigation_action);
                button = webkit_navigation_action_get_mouse_button(navigation_action);

                if (button == 1 && mods & GDK_CONTROL_MASK) {
                    t = (gchar *) webkit_uri_request_get_uri(request);
                    link = ensure_uri_scheme(t);
                    client_new(link);
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

void
openlink(GtkWidget *widget __attribute__((__unused__)), gpointer data) {
    struct Client *c = (struct Client *) data;
    gchar *link;
    const gchar *p;

    p = gtk_entry_get_text(GTK_ENTRY(c->entry_open));
    link = ensure_uri_scheme(p);
    webkit_web_view_load_uri(c->webView, link);
    g_free(link);
    gtk_widget_hide(c->window_open);
}

void
find(GtkWidget *widget __attribute__((__unused__)), gpointer data) {
    struct Client *c = (struct Client *) data;
    static gchar *search_text;
    const gchar *p;

    WebKitWebView *web_View = c->webView;
    WebKitFindController *fc = webkit_web_view_get_find_controller(web_View);

    p = gtk_entry_get_text(GTK_ENTRY(c->entry));

    gtk_widget_grab_focus(GTK_WIDGET(c->webView));

    if (search_text != NULL)
        g_free(search_text);

    search_text = g_strdup(p);
    webkit_find_controller_search(fc, search_text,
                                  WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE | WEBKIT_FIND_OPTIONS_WRAP_AROUND, G_MAXUINT);
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
    int i;
    gchar *favfilename;
    FILE *File;
    char buffer[256] = "<html><head></head><body bgcolor=black>";
    gchar *link;

    gtk_init(&argc, &argv);

    webkit_web_context_set_process_model(webkit_web_context_get_default(),
                                         WEBKIT_PROCESS_MODEL_MULTIPLE_SECONDARY_PROCESSES);

    favfilename = g_strdup_printf("%s", ".fav");
    favpath = g_build_filename(getenv("HOME"), favfilename, NULL);
    g_free(favfilename);

    if (!g_file_test(favpath, G_FILE_TEST_EXISTS)) {
        File = fopen(favpath, "wb+");
        fprintf(File, "%s", buffer);
        fclose(File);
    }

    home = g_build_filename("file:/", favpath, NULL);

    if (argc > 1) {
        for (i = 1; i < argc; i++) {
            link = argv[i];
            client_new(link);
        }
    } else
        client_new(home);

    gtk_main();
    return 0;
}
