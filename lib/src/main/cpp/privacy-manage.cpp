//
// Created by zhangxipeng on 2018/1/5.
//

#include <jni.h>
#include <string.h>
#include <stdlib.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>

#include "en-decrypt/en-decrypt.h"

#define MAX_LEN 1024
#define ENCRYPT 0
#define DECRYPT 1
#define KEY_SIZE 256
#define START_ADDRESS 1080

#define  LOG_TAG    "HengChang"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static unsigned char PRIVACY_IV[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                                        0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

static unsigned char PRIVACY_KEY[32] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71,
                                         0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81, 0x1f, 0x35, 0x2c,
                                         0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf,
                                         0xf4 };

unsigned int bytesToInt(unsigned char* data) {
    unsigned int value = data[0] & 0xFF;
    value |= ((data[1] << 8) & 0xFF00);
    value |= ((data[2] << 16) & 0xFF0000);
    value |= ((data[3] << 24) & 0xFF000000);

    return value;
}

const char* getFileName(JNIEnv *env, jstring filename){
    const char* file_name = env->GetStringUTFChars(filename,0);
    env->ReleaseStringUTFChars(filename,file_name);
    return  file_name;
}

void createFile(const char* file_name, unsigned char* buff, size_t len) {
    FILE* stream = fopen(file_name, "a");

    if (stream == NULL) {
        return;
    } else {
        if (len != fwrite(buff, sizeof(unsigned char),len,stream)){
            LOGE("create file fail!");
        } else {
            fflush(stream);
        }
        fclose(stream);
    }
}

void modifyFile(const char* file_name,unsigned char* modify_buf,size_t modify_len, int offset){
    FILE* stream = fopen(file_name, "r+");

    if (stream == NULL) {
        return;
    } else{
        fseek(stream, offset, SEEK_SET);
        if (modify_len != fwrite(modify_buf, sizeof(unsigned char),modify_len,stream)){
            LOGE("modify file fail!");
        } else {
            fflush(stream);
        }
        fclose(stream);
    }
}

jbyteArray readFile(JNIEnv *env, const char* file_name,int offset) {
    FILE* stream = fopen(file_name, "r");

    if (stream == NULL) {
        return NULL;
    } else {
        fseek(stream, offset, SEEK_SET);
        unsigned char data_length[4];
        if (4 != fread(data_length, sizeof(unsigned char),4,stream)){
            LOGI("read encrypt data length fail!");
            return NULL;
        }

        unsigned int length = bytesToInt(data_length);
        unsigned char* buff =(unsigned char*) malloc(length);
        memset(buff, 0, length);

        if (length != fread(buff, sizeof(unsigned char),length,stream)){
            LOGI("read encrypt data fail!");
        }
        fclose(stream);

        jbyteArray jarray = env->NewByteArray(length);
        env->SetByteArrayRegion(jarray, 0, length, (jbyte*) buff);
        free(buff);

        return jarray;
    }
}

jbyteArray android_crypt(JNIEnv *env,jbyteArray jarray, jint jmode) {
    // 校验数据
    unsigned int len = (unsigned int) (env->GetArrayLength(jarray));
    if (len <= 0 || len >= MAX_LEN) {
        return NULL;
    }

    unsigned char *data = (unsigned char*) env->GetByteArrayElements(jarray, NULL);
    if (!data) {
        return NULL;
    }

    //计算填充长度，当为加密方式且长度不为16的整数倍时，则填充
    unsigned int mode = (unsigned int) jmode;
    unsigned int rest_len = len % BLOCK_SIZE;
    unsigned int padding_len = (
            (ENCRYPT == mode) ? (BLOCK_SIZE - rest_len) : 0);
    unsigned int src_len = len + padding_len;

    //设置输入
    unsigned char *input = (unsigned char *) malloc(src_len);
    memset(input, 0, src_len);
    memcpy(input, data, len);
    if (padding_len > 0) {
        memset(input + len, (unsigned char) padding_len, padding_len);
    }
    //data不再使用
    env->ReleaseByteArrayElements(jarray, (jbyte*)data, 0);

    //设置输出Buffer
    unsigned char * buff = (unsigned char*) malloc(src_len);
    if (!buff) {
        free(input);
        return NULL;
    }
    memset(buff, 0, src_len);

    unsigned int key_schedule[BLOCK_SIZE * 4] = { 0 };
    key_setup(PRIVACY_KEY, key_schedule, KEY_SIZE);

    //执行加解密计算
    if (mode == ENCRYPT) {
        encrypt_data(input, src_len, buff, key_schedule, KEY_SIZE,
                     PRIVACY_IV);
    } else {
        decrypt_data(input, src_len, buff, key_schedule, KEY_SIZE,
                     PRIVACY_IV);
    }

    //解密时计算填充长度
    if (ENCRYPT != mode) {
        unsigned char * ptr = buff;
        ptr += (src_len - 1);
        padding_len = (unsigned int) *ptr;
        if (padding_len > 0 && padding_len <= BLOCK_SIZE) {
            src_len -= padding_len;
        }
        ptr = NULL;
    }

    //设置返回变量
    jbyteArray bytes = env->NewByteArray(src_len);
    env->SetByteArrayRegion(bytes, 0, src_len, (jbyte*) buff);

    //内存释放
    free(input);
    free(buff);

    return bytes;
}


extern "C"
JNIEXPORT jboolean
JNICALL

Java_com_shuai_dataprivacy_PrivacyUtils_savePrivacy(JNIEnv *env, jclass clazz,
                                                                          jstring filename, jbyteArray jarray, jint offset) {
    unsigned int i;

    offset += START_ADDRESS;
    jbyteArray encrypt_jarray = android_crypt(env,jarray,ENCRYPT);
    unsigned int encrypt_len = (unsigned int) (env->GetArrayLength(encrypt_jarray));
    if (encrypt_len <= 0 || encrypt_len >= MAX_LEN) {
        return JNI_FALSE;
    }
    unsigned char* encrypt_data = (unsigned char*) env->GetByteArrayElements(encrypt_jarray, NULL);
    if (!encrypt_data) {
        return JNI_FALSE;
    }


    unsigned char* modify_data = (unsigned char* ) malloc(encrypt_len + 4);

    modify_data[0] = (unsigned char) (0xff & encrypt_len);
    modify_data[1] = (unsigned char) ((0xff00 & encrypt_len) >> 8);
    modify_data[2] = (unsigned char) ((0xff0000 & encrypt_len) >> 16);
    modify_data[3] = (unsigned char) ((0xff000000 & encrypt_len) >> 24);

    for (i = 0; i < encrypt_len; i++) {
        modify_data[i + 4] = encrypt_data[i];
    }

    const char *file_name = getFileName(env,filename);
    modifyFile(file_name,modify_data,encrypt_len + 4,offset);

    //encrypt_data不再使用
    env->ReleaseByteArrayElements(encrypt_jarray, (jbyte*)encrypt_data, 0);
    free(modify_data);

    return JNI_TRUE;
}


extern "C"
JNIEXPORT jbyteArray
JNICALL
Java_com_shuai_dataprivacy_PrivacyUtils_getPrivacy(JNIEnv *env, jclass clazz,
                                                                        jstring filename, jint offset) {
    const char *file_name = getFileName(env,filename);
    offset += START_ADDRESS;
    jbyteArray jarray = readFile(env,file_name,offset);

    return android_crypt(env, jarray, DECRYPT);
}

extern "C"
JNIEXPORT void
JNICALL
Java_com_shuai_dataprivacy_PrivacyUtils_initPrivacy(JNIEnv *env, jclass clazz,
                                                                  jobject assetManager, jstring filename) {

    AAssetManager* am = AAssetManager_fromJava(env, assetManager );
    if(am == NULL)
    {
        LOGE("AAsertManager is NULL");
        return;
    }
    AAsset* asset = AAssetManager_open(am, "privacy_storage.bmp", AASSET_MODE_UNKNOWN);
    if (asset == NULL)
    {
        LOGE("_ASSET_NOT_FOUND_");
    }
    long size = AAsset_getLength(asset);
    unsigned char* buffer = (unsigned char*) malloc (sizeof(unsigned char) * size);
    AAsset_read(asset,buffer,size);

    const char *file_name = getFileName(env,filename);
    createFile(file_name,buffer,size);

    free(buffer);
    AAsset_close(asset);
}

extern "C"
JNIEXPORT jboolean
JNICALL
Java_com_shuai_dataprivacy_PrivacyUtils_isFileExist(JNIEnv *env, jclass clazz,
                                                                         jstring filename) {
    const char *file_name = getFileName(env,filename);

    FILE* stream = fopen(file_name, "r");
    if (stream != NULL) {
        LOGI("======file is exist======");
        fclose(stream);
        return JNI_TRUE;
    }
    return JNI_FALSE;
}







