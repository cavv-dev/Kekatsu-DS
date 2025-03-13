#include "strings.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

void humanizeSize(char* sizeStr, size_t bufferSize, u64 sizeInBytes)
{
    const char* suffix[] = { "B", "KiB", "MiB", "GiB" };
    u8 suffixesCount = 4;

    u8 i = 0;
    double dblBytes = sizeInBytes;
    if (sizeInBytes > 1024) {
        for (i = 0; (sizeInBytes / 1024) > 0 && i < suffixesCount - 1; i++, sizeInBytes /= 1024)
            dblBytes = (double)sizeInBytes / 1024;
    }

    snprintf(sizeStr, bufferSize, "%.01lf %s", dblBytes, suffix[i]);
}

void lowerStr(char* str)
{
    for (size_t i = 0; str[i] != '\0'; i++)
        str[i] = tolower(str[i]);
}

void removeAccentsStr(char* str)
{
    const char normalizedCharMap[] = {
        (char)0, (char)1, (char)2, (char)3, (char)4, (char)5, (char)6, (char)7, (char)8, (char)9, (char)10, (char)11, (char)12, (char)13, (char)14, (char)15, (char)16, (char)17, (char)18, (char)19, (char)20, (char)21, (char)22, (char)23, (char)24, (char)25, (char)26, (char)27, (char)28, (char)29, (char)30, (char)31, (char)32, (char)33, (char)34, (char)35, (char)36, (char)37, (char)38, (char)39, (char)40, (char)41, (char)42, (char)43, (char)44, (char)45, (char)46, (char)47, (char)48, (char)49, (char)50, (char)51, (char)52, (char)53, (char)54, (char)55, (char)56, (char)57, (char)58, (char)59, (char)60, (char)61, (char)62, (char)63, (char)64, (char)65, (char)66, (char)67, (char)68, (char)69, (char)70, (char)71, (char)72, (char)73, (char)74, (char)75, (char)76, (char)77, (char)78, (char)79, (char)80, (char)81, (char)82, (char)83, (char)84, (char)85, (char)86, (char)87, (char)88, (char)89, (char)90, (char)91, (char)92, (char)93, (char)94, (char)95, (char)96, (char)97, (char)98, (char)99, (char)100, (char)101, (char)102, (char)103, (char)104, (char)105, (char)106, (char)107, (char)108, (char)109, (char)110, (char)111, (char)112, (char)113, (char)114, (char)115, (char)116, (char)117, (char)118, (char)119, (char)120, (char)121, (char)122, (char)123, (char)124, (char)125, (char)126, (char)127, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)63, (char)32, (char)33, (char)63, (char)63, (char)63, (char)63, (char)124, (char)63, (char)34, (char)63, (char)97, (char)63, (char)33, (char)63, (char)63, (char)45, (char)63, (char)63, (char)50, (char)51, (char)39, (char)117, (char)80, (char)42, (char)44, (char)49, (char)111, (char)63, (char)63, (char)63, (char)63, (char)63, (char)65, (char)65, (char)65, (char)65, (char)65, (char)65, (char)63, (char)67, (char)69, (char)69, (char)69, (char)69, (char)73, (char)73, (char)73, (char)73, (char)68, (char)78, (char)79, (char)79, (char)79, (char)79, (char)79, (char)120, (char)79, (char)85, (char)85, (char)85, (char)85, (char)89, (char)63, (char)63, (char)97, (char)97, (char)97, (char)97, (char)97, (char)97, (char)63, (char)99, (char)101, (char)101, (char)101, (char)101, (char)105, (char)105, (char)105, (char)105, (char)100, (char)110, (char)111, (char)111, (char)111, (char)111, (char)111, (char)47, (char)111, (char)117, (char)117, (char)117, (char)117, (char)121, (char)63, (char)121
    };

    // Replace each non-ASCII character with its corresponding in map
    size_t i = 0, j = 0;
    while (str[i] != '\0') {
        u8 c = (u8)str[i];
        if (c < 128)
            str[j++] = str[i];
        else
            str[j++] = normalizedCharMap[c];

        i++;
    }

    str[j] = '\0';
}

void safeStr(char* str)
{
    removeAccentsStr(str);

    // Replace characters not in 'a-zA-Z0-9 ' with '_'
    size_t i, j;
    for (i = 0, j = 0; str[i]; i++) {
        if ((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= '0' && str[i] <= '9') || (str[i] == ' '))
            str[j++] = str[i];
        else
            str[j++] = '_';
    }

    str[j] = '\0';
}

void joinPath(char* joinedPath, const char* path1, const char* path2)
{
    strcpy(joinedPath, path1);

    if (joinedPath[strlen(joinedPath) - 1] != '/')
        strcat(joinedPath, "/");

    if (path2[0] == '/')
        strcat(joinedPath, path2 + 1);
    else
        strcat(joinedPath, path2);
}
