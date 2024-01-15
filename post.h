#ifndef POST_H
#define POST_H

unsigned int probeRam(unsigned int start, unsigned int stride, int print_message);
unsigned int probeAll(unsigned int stride, unsigned int top, int print_message);
int probeSpeed();
void doPost();

#endif
