#include "akinator.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main()
{
    void* akinator = NULL;
    while (1) {
        printf("1: new game\n");
        if (akinator) printf("2: play game\n");
        printf("3: load game\n");
        if (akinator) printf("4: store game\n");
        printf("5: exit\n");

        char number = 0;
        scanf(" %c%*[^\r\n]", &number);

        if (number == '1') {
            free(akinator);
            akinator = akinator_create();
            LOG("%p", akinator)
            printf("number = 1\n");
        }
        if (number == '2' && akinator) {
            akinator_play(&akinator);
        }
        char buffer[256];
        if (number == '3') {
            free(akinator);
            printf("print file name:\n");
            scanf("%250s%*[^\r\n]", buffer);
            akinator = akinator_load(buffer);
        }
        if (number == '4' && akinator) {
            printf("print file name:\n");
            scanf("%250s%*[^\r\n]", buffer);
            akinator_store(akinator, buffer);
        }
        if (number == '5') {
            free(akinator);
            return 0;
        }
    }
}
