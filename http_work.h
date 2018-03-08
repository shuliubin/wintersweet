#ifndef HTTPWORK_H_
#define HTTPWORK_H_

int socket_create(int port);
void *http_thread(void *argc);
void save_log(char *buf);

#endif /* PUB_H_ */
