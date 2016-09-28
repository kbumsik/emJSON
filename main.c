#include <stdio.h>
#include <string.h>
#include "emJSON.h"

int main(int argc, char *argv[])
{
    char sz[] = "Hello, World!\n";  /* Hover mouse over "sz" while debugging to see its contents */
    printf("%s", sz);   
    fflush(stdout); /* <============== Put a breakpoint here */
    printf("%d\n", json_hash("a") & 7);
    printf("%d\n", json_hash("i") & 7);
    printf("%d\n", json_hash("q") & 7);
    printf("%d\n", json_hash("TestH") & 7);
    json_t test = emJSON_init();
    emJSON_insert(&test, "a", "JSON");
    emJSON_insert(&test, "i", "IS");
    emJSON_insert(&test, "q", "Cool");
    emJSON_insert(&test, "TestH", "Test Good?");
    
    printf("%s\n", emJSON_get(&test, "a"));
    printf("%s\n", emJSON_get(&test, "i"));
    printf("%s\n", emJSON_get(&test, "q"));
    printf("%s\n", emJSON_get(&test, "TestH"));
    char *str = emJSON_string(&test);
    printf("%d\n", json_strlen(&test));
    printf("%d\n", strlen(str));
    
    printf("%s\n", str);
    free(str);
    emJSON_free(&test);
    return 0;
}

/**
{
    "a":"JSON",
    "i":"IS",
    "q":"Cool"
}
*/
	