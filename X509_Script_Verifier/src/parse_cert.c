#include <stdio.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/evp.h>


EVP_PKEY* extract_public_key(char* cert_path) {
    FILE *fp;
    X509 *cert = NULL;
    EVP_PKEY *pkey = NULL;

    // Open the certificate file
    fp = fopen(cert_path, "r");
    if (fp == NULL) {
        perror("Unable to open certificate file");
        return NULL;
    }

    // Read the certificate from the file
    cert = PEM_read_X509(fp, NULL, NULL, NULL);
    fclose(fp);
    if (cert == NULL) {
        fprintf(stderr, "Unable to parse certificate\n");
        return NULL;
    }

    int ext_key_usage_nid = NID_ext_key_usage;
    int index = X509_get_ext_by_NID(cert, ext_key_usage_nid, -1);
    if (index < 0) {
        // fprintf(stderr, "Certificate does not contain Code Signing extension\n");
        return NULL;
    }

    X509_EXTENSION *ext = X509_get_ext(cert, index);
    ASN1_STRING *eku = (ASN1_STRING *)X509_get_ext_d2i(cert, NID_ext_key_usage, NULL, NULL);

    int code_signing_nid = NID_code_sign;
    int is_code_signing = -1;
    for (int i = 0; i < sk_ASN1_OBJECT_num(eku); i++) {
        ASN1_OBJECT *obj = sk_ASN1_OBJECT_value(eku, i);
        if (OBJ_obj2nid(obj) == code_signing_nid) {
            is_code_signing = 1;
            break;
        }
    }

    if (is_code_signing < 0) {
        // fprintf(stderr, "Certificate does not contain Code Signing extension\n");
        return NULL;
    }

    // Extract the public key from the certificate
    pkey = X509_get_pubkey(cert);
    if (pkey == NULL) {
        fprintf(stderr, "Unable to extract public key from certificate\n");
        return NULL;
    }

    //cleanup
    X509_free(cert);
    ASN1_STRING_free(eku);

    return pkey;
}