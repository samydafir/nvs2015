#ifndef SENDER_H
#define SENDER_H

void handleError(char *message);
void evaluate(struct timeval before, struct timeval after, int msg_size, int amt);
char* createMsg(int length);
#endif /* SENDER_H */