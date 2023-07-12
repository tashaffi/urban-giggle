#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <string.h>

void printLastError(char *msg)
{
    char *err = malloc(130);
    ERR_error_string(ERR_get_error(), err);
    printf("%s ERROR: %s\n", msg, err);
    free(err);
}

EVP_PKEY* extract_public_key(const char* cert_path) {
    FILE* fp = fopen(cert_path, "r");
    if (fp == NULL) {
        printf("Unable to open certificate file\n");
        exit(1);
    }
    X509* cert = PEM_read_X509(fp, NULL, NULL, NULL);
    fclose(fp);
    if (cert == NULL) {
        printf("Unable to read certificate\n");
        exit(1);
    }
    EVP_PKEY* pkey = X509_get_pubkey(cert);
    X509_free(cert);
    return pkey;
}

int main()
{
    EVP_PKEY *pkey = NULL;
    FILE *fp = fopen("../scripts/private_key1.pem", "r");
    if (fp == NULL) {
        printf("Unable to open private key file\n");
        exit(1);
    }
    pkey = PEM_read_PrivateKey(fp, &pkey, NULL, NULL);
    fclose(fp);

    // Get the public key from the certificate
    EVP_PKEY* pub_pkey = extract_public_key("../certs/c1.pem");
    if (pub_pkey == NULL) {
        // Handle error...
    }

    // Test data
    unsigned char plaintext[]="This is a test.";
    unsigned char encrypted[4098]={};
    unsigned char decrypted[4098]={};

    // Encrypt the data with the public key
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pub_pkey, NULL);
    EVP_PKEY_encrypt_init(ctx);
    EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING);
    size_t outlen = sizeof(encrypted);
    EVP_PKEY_encrypt(ctx, encrypted, &outlen, plaintext, strlen(plaintext)+1);
    EVP_PKEY_CTX_free(ctx);

    // Decrypt the data with the private key
    ctx = EVP_PKEY_CTX_new(pkey, NULL);
    EVP_PKEY_decrypt_init(ctx);
    EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING);
    size_t decryptedlen = sizeof(decrypted);
    EVP_PKEY_decrypt(ctx, decrypted, &decryptedlen, encrypted, outlen);
    EVP_PKEY_CTX_free(ctx);

    // Check if the decrypted data matches the original data
    if (strcmp((char*)plaintext, (char*)decrypted) == 0) {
        printf("The keys are a valid pair!\n");
    } else {
        printf("The keys are NOT a valid pair!\n");
    }

    return 0;
}
