#ifndef RSA_H
#define RSA_H

int initialize_rsa(void);

void cleanup_rsa(void);

int rsa_encrypt(int x);

#endif /* !RSA_H */
