## Introduction

This project is a server-client application that uses OpenSSL for secure script execution. The server receives a bash script from a client, verifies its signature against a set of certificates, and if valid, executes the script. The server can also handle multiple concurrent client connections. The project uses OpenSSL for generating private keys, hashes, and signatures, and TCP/IP sockets for communication between the server and clients.

## Development Environment
This program was developed and tested on MacOS with OpenSSL version 1.1.1u. Please note that instructions and program behavior may vary depending on your operating system and OpenSSL version.

## Usage

Server code for processing a single request and multiple requests at a time are in two different files. Running ```make``` will generate all the necessary binaries.

To test the server without concurrency:

```bash
cd src
# Run the two binaries from separate terminals
./server
./process <your_script_name>
```

To test the server with concurrency:

```bash
cd src
./server_concurr
./../test/test_concurr
```

The ```./../test/test_concurr``` will run processes for all the scripts in the ```scripts``` folder. Alternatively, the processes can be run from different terminals. The server will keep running unless it is terminated using ```CTRL + C```.

## Communication Protocol

The server communicates with clients using a TCP/IP socket. The server listens for incoming connections on a specified port. When a client connects, it sends a bash script to the server. The first line of the script is the signature of the rest of the file. The server verifies this signature using a locally stored X.509 certificate. If the signature is valid, the server executes the script.



## Status Codes

The server prints a status message to stdout indicating whether the signature is valid or not. If the signature is valid, the server prints *Congratulations! The script is safe to execute!* and executes the script. If the signature is not valid, the server prints *NO valid certificate found for this script! Do not execute!* and does not execute the script.

If the server encounters an error (such as a problem reading the certificate file or a network error), it will print an error message to stderr and exit with a non-zero status code.


## Resources

I have used the ```OpenSSL``` Library to generate the private keys, hashes, and signatures. Below are the commands I used:
- Private keys: ```openssl genpkey -algorithm RSA -out private_key.pem -pkeyopt rsa_keygen_bits:2048```
- Certificates with code signing extension: ```openssl req -new -x509 -key private_key.pem -out certificate.crt -days 365 -config codesign.cnf -extensions ext```
- Certificates without code signing extension: ```openssl req -new -x509 -key private_key.pem -out certificate.pem -days 365```
- Signatures: ```openssl dgst -sha256 -sign private_key.pem -out signature.sig script.sh```, followed by ```xxd -p -c 256 signature.sig > signature.txt``` which was pasted at the start of the script

All the private keys and scripts are in the ```scripts``` folder. The certificates are in the ```certs``` folder.

## Test Cases

I have generated the test cases below using the commands specified in the section above:

- Valid certificate: ```script2.sh``` has a valid signature. The public key is in certificate ```c2.pem```, and the corresponding private key is ```private_key1.pem```. The server should successfully verify the signature and execute the script.
- Invalid certificate without the code signing extension: ```script1.sh``` has a valid signature. The signature results in the hash of the content of the script if decrypted with the public key found in ```c1.pem```. However, ```c1.pem``` does not have a code signing extension, therefore the server should fail to verify the signature and should not execute the script. Note that the signature will not be further verified if the extension is not found.
- Invalid signature: ```script3.sh``` has an invalid signature. It was created with a signature corresponding to ```c3.pem```'s public key but the content of the script has been changed after creating the signature. The server should fail to verify the signature and should not execute the script.
- Invalid signature: ```script4.sh``` has a signature that cannot be decrypted with any of the certificates. The server should fail to verify the signature. The server should fail to verify the signature and should not execute the script.

Only ```script2.sh``` is supposed to execute in both the single-request and concurrent cases.


## File Descriptions

- ```server.c```: This server-side application receives a script from a client, verifies its signature against a set of certificates, executes the script if valid, and sends a confirmation message to the client.
- ```process.c```: This is the client-side application. It sends a specified script to a server and receives a response, which it then prints out. It handles socket creation, connection, data transmission, and cleanup.
- ```parse_cert.c```: This file contains a function that reads a certificate, verifies the presence of the Code Signing extension, and extracts the public key from the certificate.
- ```check_sig.c```: This file contains functions for checking the digital signature of a script against a set of certificates. It includes the function ```check_signature```, which extracts the public key from a given certificate and uses it to verify the signature of a script. It also includes the function ```check_signature_with_all_certs```, which iterates over all certificates in a given directory and checks the signature against each one until a valid certificate is found. If no valid certificate is found, it returns a failure status.
- ```file_reader.c```: This file provides a function to read an entire file into a dynamically allocated buffer, handling any errors and cleanup as necessary.
- ```server_helper.c```: The ```server_helper.c``` file contains functions for setting up a server socket and handling client data. It creates a unique file for each client's data, verifies the data, and manages the communication between the server and client.
- ```server_concurr.c```: This is exactly like ```server.c``` but it process multiple requests concurrently.


## Possible Improvements

The current implementation checks for a matching signature and the presence of a code signing extension. More comprehensive validation, such as checking the certificate's validity period or issuer, could be implemented. The application uses a multi-process model for concurrent requests. Alternative models like event-driven or multi-threading could be explored for better performance and scalability. More robust error handling could be implemented to handle potential issues such as network errors, file system errors, or malformed input.