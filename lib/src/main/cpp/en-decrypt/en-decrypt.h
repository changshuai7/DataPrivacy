#define BLOCK_SIZE 16

typedef unsigned char BYTE;
typedef unsigned int WORD;

void key_setup(const BYTE key[],
                   WORD w[],
                   int keysize);

void encrypt(const BYTE in[],
                 BYTE out[],
                 const WORD key[],
                 int keysize);

void decrypt(const BYTE in[],
                 BYTE out[],
                 const WORD key[],
                 int keysize);

int encrypt_data(const BYTE in[],
                    size_t in_len,
                    BYTE out[],
                    const WORD key[],
                    int keysize,
                    const BYTE iv[]);

int decrypt_data(const BYTE in[],
                    size_t in_len,
                    BYTE out[],
                    const WORD key[],
                    int keysize,
                    const BYTE iv[]);