
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <openssl/bn.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <dirent.h>

#include "../include/parse_cert.h"
#include "../include/file_reader.h"

#define STATUS_SUCCESS 1
#define STATUS_FAILURE -1


void printBN(char *msg, BIGNUM * a)
{
   /* Use BN_bn2hex(a) for hex string
    * Use BN_bn2dec(a) for decimal string */
   char * number_str = BN_bn2hex(a);
   printf("%s %s\n", msg, number_str);
   OPENSSL_free(number_str);
}


int check_signature(char* script_start, char* signature_str, char* cert_file){
    // printf("Checking signature %s for Process ID: %d\n", cert_file, getpid());
    // Extract public key from certificate
    EVP_PKEY* pkey = extract_public_key(cert_file);
    if (pkey == NULL) {
        return STATUS_FAILURE; //Certificate does not exist or does not contain code signing extension
    }

    BIGNUM *n = NULL;
    if (EVP_PKEY_get_bn_param(pkey, "n", &n) <= 0) {
        fprintf(stderr, "Unable to parse certificate\n");
        return STATUS_FAILURE;
    }

    BIGNUM *e = NULL;
    if (EVP_PKEY_get_bn_param(pkey, "e", &e) <= 0) {
        fprintf(stderr, "Unable to parse certificate\n");
        return STATUS_FAILURE;
    }
    // printBN("n: ", n);
    // printBN("e: ", e);

    // Convert signature to BIGNUM
    BIGNUM *signature = BN_new();
    BN_hex2bn(&signature, signature_str);  // Use BN_dec2bn if your signature is in decimal format

    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *res = BN_new();
    BN_mod_exp(res, signature, e, n, ctx);

    // Compute the SHA-256 hash of the script
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)script_start, strlen(script_start), hash);

    // Convert the hash to a BIGNUM
    BIGNUM *hash_bn = BN_bin2bn(hash, SHA256_DIGEST_LENGTH, NULL);

    // Convert the res BIGNUM back to a byte array
    unsigned char *res_bytes = OPENSSL_malloc(BN_num_bytes(res));
    BN_bn2bin(res, res_bytes);
    // The hash is at the end of the res_bytes array, after the padding and ASN.1 structure
    unsigned char *res_hash = res_bytes + BN_num_bytes(res) - SHA256_DIGEST_LENGTH;
    // Convert the res_hash byte array to a BIGNUM
    BIGNUM *res_hash_bn = BN_bin2bn(res_hash, SHA256_DIGEST_LENGTH, NULL);
    
    int status_code = STATUS_SUCCESS;
    if (BN_cmp(res_hash_bn, hash_bn) == 0) {
        status_code = STATUS_SUCCESS;
    } else {
        status_code = STATUS_FAILURE;
    }

    //cleanup
    BN_free(n);
    BN_free(e);
    BN_free(signature);
    OPENSSL_free(res_bytes);
    BN_free(hash_bn);
    BN_free(res_hash_bn);

    return status_code;
}


// Function to check signature against all certificates in a directory
int check_signature_with_all_certs(const char *script_start, const char *signature_str, const char *certs_dir) {
    DIR *dir;
    struct dirent *ent;
    char cert_path[1024];

    if ((dir = opendir(certs_dir)) != NULL) {
        // Iterate over all the files in the directory
        while ((ent = readdir(dir)) != NULL) {
            // Skip directories
            if (ent->d_type == DT_DIR) {
                continue;
            }

            // Construct the full path to the certificate file
            snprintf(cert_path, sizeof(cert_path), "%s/%s", certs_dir, ent->d_name);

            // Check the signature with the current certificate
            int status_code = check_signature(script_start, signature_str, cert_path);
            if (status_code == STATUS_SUCCESS) {
                printf("The signature for script received from Process ID %d is valid with certificate: %s\n", getpid(), cert_path);
                closedir(dir);
                return STATUS_SUCCESS;
            }
        }
        closedir(dir);
    } else {
        // Could not open directory
        perror("Could not open certificates directory");
        return STATUS_FAILURE;
    }

    return STATUS_FAILURE;
}