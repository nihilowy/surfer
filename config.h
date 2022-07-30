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
#define SURFER_INSPECTOR_KEY        GDK_KEY_I
#define SURFER_ZOOM_IN_KEY          GDK_KEY_equal
#define SURFER_ZOOM_OUT_KEY         GDK_KEY_minus
#define SURFER_FULLSCREEN_KEY       GDK_KEY_F11
#define SURFER_HISTORY_KEY          GDK_KEY_H
#define SURFER_SCROLL_DOWN_KEY      GDK_KEY_Down
#define SURFER_SCROLL_UP_KEY        GDK_KEY_Up
#define SURFER_SCROLL_PAGE_DOWN_KEY GDK_KEY_s
#define SURFER_SCROLL_PAGE_UP_KEY   GDK_KEY_w
#define SURFER_STYLE_KEY            GDK_KEY_S

#define SURFER_COOKIE_POLICY        WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY

//WEBKIT_COOKIE_POLICY_ACCEPT_ALWAYS -Accept all cookies unconditionally.
//WEBKIT_COOKIE_POLICY_ACCEPT_NEVER -Reject all cookies unconditionally.
//WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY -Accept only cookies set by the main document loaded

#define SURFER_ACCELERATION_2DCANVAS FALSE


#define SURFER_ACCELERATION_POLICY WEBKIT_HARDWARE_ACCELERATION_POLICY_ON_DEMAND
/*
WEBKIT_HARDWARE_ACCELERATION_POLICY_ON_DEMAND
Hardware acceleration is enabled/disabled as request by web contents.
 
WEBKIT_HARDWARE_ACCELERATION_POLICY_ALWAYS
Hardware acceleration is always enabled, even for websites not requesting it.
 
WEBKIT_HARDWARE_ACCELERATION_POLICY_NEVER
Hardware acceleration is always disabled, even for websites requesting it.
*/

#define SURFER_SPATIAL_NAVIGATION 	FALSE//TRUE to enable 
#define SURFER_WINDOW_WIDTH	800
#define SURFER_WINDOW_HEIGHT	600

#define SURFER_THUMBNAIL_WIDTH 80
#define SURFER_THUMBNAIL_HEIGHT 80
#define SURFER_SMOOTH_SCROLLING FALSE
#define SURFER_RESIZABLE_TEXT	TRUE
#define SURFER_ZOOM_LEVEL 1

#define FONT_MIN_SIZE	12

// tablecss.txt in SURFER_DIR for custom css per site
#define USER_STYLESHEET_FILENAME	"/usr/share/surfer/black.css"  //change to your style file
#define DEFAULT_STYLE_ENABLE 0 					       //change to 1 to enable default style
#define ADBLOCK_JSON_FILE		"adblock.json"

#define WEB_EXTENSIONS_DIRECTORY 	"/usr/lib/surfer"

//#define DONT_WAIT_FOR_SITE_FULLLOAD //just uncomment if you want so !

#define HISTORY_ENABLE 0					       //change to 1 to enable history

#define SURFER_DIR	".surfer"                                      // upper directory(s) must exist
#define SURFER_DOWNLOADS "downloads"
#define SURFER_TMPDOWNLOADS "/tmp"
#define SURFER_BIN	"/usr/bin"                                      // upper directory(s) must exist

#define SURFER_PLAYER	"/usr/bin/mpv"                                          // best with youtube-dl on supported sites
#define SURFER_SEARCH_SITE  "https://translate.google.com/#auto/en/"

//"https://www.google.com/search?q="
