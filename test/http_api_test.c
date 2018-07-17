#include <unistd.h>
#include "../include/globals.h"
#include "../include/http_api.h"

// Test command curl -i -X POST http://localhost:8080/4

int main(void)
{
    init_http_api(8080);
    for(int i = 0 ; i<10; i++)
    {
        sleep(1);
        process_http_api();
    }
    close_http_api();
    return 0;
}

