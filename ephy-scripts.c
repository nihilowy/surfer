//most code form ephy-scripts on github.com + my fixes

#include <jsc/jsc.h>
#include <webkit2/webkit-web-extension.h>
#include <libgen.h>
#include <string.h>
struct script_info {
	gchar *str;
	gsize len;
	gchar *src;
};

static gchar *universal;

static GArray *script_infos;

static void add_file(const gchar *dirname, const gchar *filename)
{
	gchar *path = g_build_filename(dirname, filename, NULL);
	struct script_info s;
	if (g_file_get_contents(path, &s.str, &s.len, NULL)) {
		s.src = g_filename_to_uri(path, NULL, NULL);
		g_array_append_val(script_infos, s);
	}
	g_free(path);
}

static void add_path(const gchar *path)
{
	gchar *dirname = g_build_filename(path, "surfer", NULL);
	GDir *dir = g_dir_open(dirname, 0, NULL);
	if (dir != NULL) {
		const gchar *filename;
		while ((filename = g_dir_read_name(dir)) != NULL)
			add_file(dirname, filename);
		g_dir_close(dir);
	}
	g_free(dirname);
}

static void on_document_loaded(WebKitWebPage *page, gpointer user_data)
{

	(void) user_data;
	JSCContext *ctx = webkit_frame_get_js_context(webkit_web_page_get_main_frame(page));
	JSCValue *hostname = jsc_context_evaluate(ctx,"window.location.hostname;",-1);
	gchar *url = jsc_value_to_string(hostname);
	for (gsize i = 0; i < script_infos->len; i++) {
		const struct script_info *s = &g_array_index(script_infos, struct script_info, i);

        gchar *tmp = g_strdup(s->src);
        gchar *last = basename(tmp);

		if( g_strcmp0(url,last) == 0 || g_strcmp0(universal,last) == 0){

		g_object_unref(jsc_context_evaluate_with_source_uri(ctx, s->str, s->len, s->src, 0));

	}

	}
}


static void on_page_created(WebKitWebExtension *extension, WebKitWebPage *page, gpointer user_data)
{
	(void) extension;
	(void) user_data;
	g_signal_connect(page, "document-loaded", G_CALLBACK(on_document_loaded), NULL);
}

G_MODULE_EXPORT void webkit_web_extension_initialize(WebKitWebExtension *extension)
{
	universal = g_strdup_printf("universal.js");
	script_infos = g_array_new(FALSE, FALSE, sizeof(struct script_info));
	const gchar * const *paths = g_get_system_data_dirs();
	while (*paths != NULL)
		add_path(*(paths++));
	add_path(g_get_user_data_dir());
	g_signal_connect(extension, "page-created", G_CALLBACK(on_page_created), NULL);
}
