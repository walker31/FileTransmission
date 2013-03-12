/*
* This file defines common formats for Linpop, a LAN chatting program
* Linpop follows the IP Messenger communication protocol (Draft-9)
* you can obtain a copy of Linpop at
*
*	http://code.google.com/p/linpop
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the Apache 2.0 License.
*/
#ifndef ENCODE_H
#define ENCODE_H

#include <unistd.h>

extern int code_convert(char *to_charset, char *from_charset,
                        char *inbuf, int inlen, char *outbuf, int outlen);
extern int u2g(char *inbuf, int inlen, char *outbuf, int outlen);
extern int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen);

#endif //ENCODE_H
