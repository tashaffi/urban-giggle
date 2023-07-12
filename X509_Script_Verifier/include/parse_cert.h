#ifndef CERTIFICATE_H
#define CERTIFICATE_H

#include <openssl/evp.h>

EVP_PKEY* extract_public_key(char* cert_path);

#endif