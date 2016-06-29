#ifndef RECEIVER_H
#define RECEIVER_H

void handle_error(char *message);
void evaluate(struct timeval before, struct timeval after, int msg_size, int amt, int expected, uint crc);

#endif /* RECEIVER_H */
