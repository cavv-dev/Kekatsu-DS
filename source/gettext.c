// Based off https://github.com/dborth/libwiigui/blob/master/source/gettext.cpp

#include "gettext.h"

#include "en_lang.h"
#include "it_lang.h"
#include <calico/types.h>
#include <stdlib.h>
#include <string.h>

typedef struct _MSG {
    unsigned long id;
    char* msgstr;
    struct _MSG* next;
} MSG;

static MSG* baseMSG = 0;

#define HASHWORDBITS 32

static inline unsigned long hashString(const char* strParam)
{
    unsigned long hval, g;
    const char* str = strParam;

    hval = 0;
    while (*str != '\0') {
        hval <<= 4;
        hval += (unsigned char)*str++;
        g = hval & ((unsigned long)0xf << (HASHWORDBITS - 4));
        if (g != 0) {
            hval ^= g >> (HASHWORDBITS - 8);
            hval ^= g;
        }
    }
    return hval;
}

static char* expandEscape(const char* str)
{
    char *retval, *rp;
    const char* cp = str;

    retval = (char*)malloc(strlen(str) + 1);
    if (retval == NULL)
        return NULL;
    rp = retval;

    while (cp[0] != '\0' && cp[0] != '\\')
        *rp++ = *cp++;
    if (cp[0] != '\0') {
        do {
            switch (*++cp) {
            case '\"':
                *rp++ = '\"';
                ++cp;
                break;
            case 'a':
                *rp++ = '\a';
                ++cp;
                break;
            case 'b':
                *rp++ = '\b';
                ++cp;
                break;
            case 'f':
                *rp++ = '\f';
                ++cp;
                break;
            case 'n':
                *rp++ = '\n';
                ++cp;
                break;
            case 'r':
                *rp++ = '\r';
                ++cp;
                break;
            case 't':
                *rp++ = '\t';
                ++cp;
                break;
            case 'v':
                *rp++ = '\v';
                ++cp;
                break;
            case '\\':
                *rp = '\\';
                ++cp;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7': {
                size_t ch = *cp++ - '0';
                if (*cp >= '0' && *cp <= '7') {
                    ch *= 8;
                    ch += *cp++ - '0';
                    if (*cp >= '0' && *cp <= '7') {
                        ch *= 8;
                        ch += *cp++ - '0';
                    }
                }
                *rp = ch;
            } break;
            default:
                *rp = '\\';
                break;
            }
            while (cp[0] != '\0' && cp[0] != '\\')
                *rp++ = *cp++;
        } while (cp[0] != '\0');
    }

    *rp = '\0';
    return retval;
}

static MSG* findMSG(unsigned long id)
{
    MSG* msg;
    for (msg = baseMSG; msg; msg = msg->next) {
        if (msg->id == id)
            return msg;
    }
    return NULL;
}

static MSG* setMSG(const char* msgid, const char* msgstr)
{
    unsigned long id = hashString(msgid);
    MSG* msg = findMSG(id);
    if (!msg) {
        msg = (MSG*)malloc(sizeof(MSG));
        msg->id = id;
        msg->msgstr = NULL;
        msg->next = baseMSG;
        baseMSG = msg;
    }
    if (msg) {
        if (msgstr) {
            if (msg->msgstr)
                free(msg->msgstr);
            msg->msgstr = expandEscape(msgstr);
        }
        return msg;
    }
    return NULL;
}

static void gettextCleanUp(void)
{
    while (baseMSG) {
        MSG* nextMsg = baseMSG->next;
        free(baseMSG->msgstr);
        free(baseMSG);
        baseMSG = nextMsg;
    }
}

static char* memfgets(char* dst, size_t maxlen, char* src)
{
    if (!src || !dst || maxlen <= 0)
        return NULL;

    char* newline = strchr(src, '\n');
    if (newline == NULL)
        return NULL;

    memcpy(dst, src, (newline - src));
    dst[(newline - src)] = 0;
    return ++newline;
}

static bool loadLanguageFromMemory(const u8* lang_data, size_t lang_size)
{
    char line[256];
    char* lastID = NULL;

    const char *file, *eof;

    file = (const char*)lang_data;
    eof = file + lang_size;

    gettextCleanUp();

    while (file && file < eof) {
        file = memfgets(line, sizeof(line), (char*)file);

        if (!file)
            break;

        if (line[0] == '#')
            continue;

        if (strncmp(line, "msgid \"", 7) == 0) {
            char *msgid, *end;
            if (lastID) {
                free(lastID);
                lastID = NULL;
            }
            msgid = &line[7];
            end = strrchr(msgid, '"');
            if (end && end - msgid > 1) {
                *end = 0;
                lastID = strdup(msgid);
            }
        } else if (strncmp(line, "msgstr \"", 8) == 0) {
            char *msgstr, *end;

            if (lastID == NULL)
                continue;

            msgstr = &line[8];
            end = strrchr(msgstr, '"');
            if (end && end - msgstr > 1) {
                *end = 0;
                setMSG(lastID, msgstr);
            }
            free(lastID);
            lastID = NULL;
        }
    }
    return true;
}

void loadLanguage(LanguageEnum lang)
{
    switch (lang) {
    case LANG_IT:
        loadLanguageFromMemory(it_lang, it_lang_size);
        break;
    default:
        loadLanguageFromMemory(en_lang, en_lang_size);
        break;
    }
}

const char* gettext(const char* msgid)
{
    MSG* msg = findMSG(hashString(msgid));
    if (msg && msg->msgstr)
        return msg->msgstr;

    return msgid;
}
