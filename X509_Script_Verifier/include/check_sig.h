#ifndef CHECK_SIGNATURE_H
#define CHECK_SIGNATURE_H

#include <openssl/bn.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include "parse_cert.h"
#include "file_reader.h"


void printBN(const char *msg, BIGNUM * a);
int check_signature(char* script_start, char* signature_str, char* cert_file);
int check_signature_with_all_certs(const char *script_start, const char *signature_str, const char *certs_dir);

#endif // CHECK_SIGNATURE_H