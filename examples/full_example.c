#include <stdio.h>
#include <string.h>


// Tuning emJSON.h library. You can see them on the top of emJSON.h.
#define EMJSON_INIT_BUF_SIZE 128
#define EMJSON_INIT_TABLE_SIZE 2
#include "emJSON.h"

int main()
{
    char sz[] = "Hello, World!\n";  /* Hover mouse over "sz" while debugging to see its contents */
    printf("%s", sz);
    fflush(stdout);
    printf("%d\n", json_hash("a") & 7);
    printf("%d\n", json_hash("i") & 7);
    printf("%d\n", json_hash("q") & 7);
    printf("%d\n", json_hash("TestH") & 7);
    fflush(stdout);

    json_t test = emJSON_init();

    // Addition, deletion, retrieval test
    emJSON_insert_str(&test, "a", "JSON");
    emJSON_insert_str(&test, "i", "IS");
    emJSON_insert_str(&test, "q", "Cool");
    emJSON_insert_str(&test, "TestJ", "Should not appear");
    emJSON_insert_str(&test, "TestH", "Test Good?");
    emJSON_delete(&test, "TestJ");
    // Serialize
    printf("==JSON Serialize Test==\n");
    char *str = emJSON_string(&test);
    printf("%d\n", (int)json_strlen(&test));
    printf("%d\n", (int)strlen(str));
    printf("%s\n", str);
    free(str);
    emJSON_insert_str(&test, "TestI", "Really??");
    emJSON_insert_str(&test, "TestK", "Really? again???");
    printf("%s\n", emJSON_get_str(&test, "a"));
    printf("%s\n", emJSON_get_str(&test, "i"));
    printf("%s\n", emJSON_get_str(&test, "q"));
    printf("%s\n", emJSON_get_str(&test, "TestH"));
    printf("%s\n", emJSON_get_str(&test, "TestI"));
    fflush(stdout);
    printf("%s\n", (NULL == emJSON_get_str(&test, "TestJ"))?"NULL":emJSON_get_str(&test, "TestJ"));
    fflush(stdout);
    printf("%s\n", emJSON_get_str(&test, "TestK"));
    fflush(stdout);

    // clear test
    emJSON_clear(&test);
    printf("%s\n", (NULL == emJSON_get_str(&test, "a"))?"NULL":emJSON_get_str(&test, "a"));
    printf("%s\n", (NULL == emJSON_get_str(&test, "i"))?"NULL":emJSON_get_str(&test, "i"));
    printf("%s\n", (NULL == emJSON_get_str(&test, "q"))?"NULL":emJSON_get_str(&test, "q"));
    printf("%s\n", (NULL == emJSON_get_str(&test, "TestH"))?"NULL":emJSON_get_str(&test, "TestH"));
    printf("%s\n", (NULL == emJSON_get_str(&test, "TestI"))?"NULL":emJSON_get_str(&test, "TestI"));
    printf("%s\n", (NULL == emJSON_get_str(&test, "TestJ"))?"NULL":emJSON_get_str(&test, "TestJ"));
    printf("%s\n", (NULL == emJSON_get_str(&test, "TestK"))?"NULL":emJSON_get_str(&test, "TestK"));

    // More types test.
    emJSON_insert_float(&test, "float1", 4.9);
    emJSON_insert_float(&test, "float2", 0.023);
    emJSON_set_float(&test, "float2", 999.999);
    emJSON_insert_float(&test, "float3", -0.00911);

    emJSON_insert_str(&test, "a", "JSON");
    emJSON_insert_str(&test, "i", "IS");
    emJSON_set_str(&test, "i", "IS...!!~~!~!~!~!~!!~!!!?");
    emJSON_insert_str(&test, "q", "Cool");

    emJSON_insert_int(&test, "int1", 654);
    emJSON_set_int(&test, "int1", 85989);
    emJSON_insert_int(&test, "int2", 17);
    emJSON_insert_int(&test, "int3", -68759);


    printf("%f\n", emJSON_get_float(&test, "float1"));
    printf("%f\n", emJSON_get_float(&test, "float2"));
    printf("%f\n", emJSON_get_float(&test, "float3"));
    printf("%s\n", emJSON_get_str(&test, "a"));
    printf("%s\n", emJSON_get_str(&test, "i"));
    printf("%s\n", emJSON_get_str(&test, "q"));
    printf("%d\n", emJSON_get_int(&test, "int1"));
    printf("%d\n", emJSON_get_int(&test, "int2"));
    printf("%d\n", emJSON_get_int(&test, "int3"));

    // Non-exist keys
    printf("%d\n", emJSON_get_int(&test, "int?3"));
    printf("%f\n", emJSON_get_float(&test, "str??3"));
    printf("%s\n", (NULL == emJSON_get_str(&test, "3int?"))?"NULL":emJSON_get_str(&test, "3int?"));

    // Serialize
    printf("==JSON Serialize Test==\n");
    str = emJSON_string(&test);
    printf("%d\n", (int)json_strlen(&test));
    printf("%d\n", (int)strlen(str));
    printf("%s\n", str);
    free(str);

    // parsing test
    emJSON_clear(&test);
    char str_input[] = "{\"sensor1\":0.00468,\"message\":\"JSON Is Cool\",\"sensor2\":-1423}";
    emJSON_parse(&test, str_input);
    // Serialize
    printf("==JSON Serialize Test==\n");
    str = emJSON_string(&test);
    printf("%d\n", (int)json_strlen(&test));
    printf("%d\n", (int)strlen(str));
    printf("%s\n", str);
    free(str);
    // Finished
    emJSON_free(&test);
    printf("%s\n", "Done, You're Good!\n");
    return 0;
}
