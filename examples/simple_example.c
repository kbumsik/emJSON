#include "json.h"
#include "emJSON.h"
#include <stdio.h>

void json_example(void)
{
	/**
	 * json.h
	 */
	char str_input[] = "{\"sensor1\":0.045600,\"message\":\"JSON Is Cool\",\"sensor2\":142}";

	char buffer[256];
	json_t test = json_init(buffer, 256, 4);  // 4 is table size, this MUST be power of 2.
	json_parse(&test, str_input);

	float sensor1 = json_get_float(&test, "sensor1"); // 0.0456
	int   sensor2 = json_get_int(&test, "sensor2");   // 142
	char *message = json_get_str(&test, "message");   // "JSON Is Cool"

	json_insert_int(&test, "intInput", 999); // insert

	char str[80];
	json_strcpy(str, &test);
	printf("%s\n", str);
	// {"sensor1":0.045600,"intInput":999,"message":"JSON Is Cool","sensor2":142}

	printf("%f\n", sensor1);
	printf("%d\n", sensor2);
	printf("%s\n", message);
}

void emJSON_example(void)
{
	/**
	 * emJSON.h
	 */
	char str_input[] = "{\"sensor1\":0.045600,\"message\":\"JSON Is Cool\",\"sensor2\":142}";

	// The difference to json.h is emJSON does not require declaring buffer.
	json_t test = emJSON_init();  // 8 is table size
	emJSON_parse(&test, str_input);

	float sensor1 = emJSON_get_float(&test, "sensor1"); // 0.0456
	int   sensor2 = emJSON_get_int(&test, "sensor2");   // 142
	char *message = emJSON_get_str(&test, "message");   // "JSON Is Cool"

	emJSON_insert_int(&test, "intInput", 999); // insert

	char *str = emJSON_string(&test);
	printf("%s\n", str);
	// {"sensor1":0.045600,"intInput":999,"message":"JSON Is Cool","sensor2":142}

	free(str);

	printf("%f\n", sensor1);
	printf("%d\n", sensor2);
	printf("%s\n", message);

	// emJSON.h objects needs to be freed after used instead.
	emJSON_free(&test);
}

int main()
{
	json_example();
	emJSON_example();
	return 0;
}
