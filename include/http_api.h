/**
 * @file http_api.c
 * @author Francisco José Juan Quintanilla
 * @date May 2018
 * @brief Http Api that allows in-place adjusting of the video coding
 * @see https://github.com/jjaranda13/LHE_Pi
 */

/**
 * @brief Initializes the Http Api
 *
 * This function creates the socket and binds it to the requested port. The
 * socket is created reusable and non-bloquing in order to allow the execution
 * while coding frames. This function must be called before doing anything else
 * with the HTTP api. The function returns -1 in case of error and logs to the
 * stderr.
 *
 * @param port Port to bind the server.
 * @return Zero if everything is allright, -1 in case of error.
 */
int init_http_api(int port);

/**
 * @brief Processes pending requets
 *
 * This function processes the pending requests in the socket. It retrieves the
 * message in case the format is not correct it will return a 400 error code. If
 * the message is correct it sends back a 200 code.
 *
 * @return Zero if everything is allright, -1 in case of error.
 */
int process_http_api();
/**
 * @brief Closes the Api
 *
 * Just closes the Api.
 *
 * @return Zero if everything is allright wich is allways.
 */
int close_http_api();

/************
 * PRIVATE
 ************/

void send_HTTP_400(int socket);
void send_HTTP_200(int socket);
