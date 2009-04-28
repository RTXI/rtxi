#include "test_no_copy_fifo.h"

#include <no_copy_fifo.h>

CPPUNIT_TEST_SUITE_REGISTRATION(TestNoCopyFifo);

#define DAT_MAGIC 0xF0F05757
#define BUFFER_SIZE 128

void TestNoCopyFifo::testSingleReadWrite(void) {
    NoCopyFifo fifo(BUFFER_SIZE);
    u_int32_t value1 = DAT_MAGIC;
    u_int32_t value2 = 0;
    u_int32_t *ptr;

    CPPUNIT_ASSERT(sizeof(value1) < BUFFER_SIZE);
    CPPUNIT_ASSERT(sizeof(value1) == sizeof(value2));

    CPPUNIT_ASSERT((ptr = reinterpret_cast<u_int32_t *>(fifo.write(sizeof(value1)))) != NULL);
    memcpy(ptr,&value1,sizeof(value1));
    fifo.writeDone();

    CPPUNIT_ASSERT((ptr = reinterpret_cast<u_int32_t *>(fifo.read(sizeof(value2)))) != NULL);
    memcpy(&value2,ptr,sizeof(value2));
    fifo.readDone();

    CPPUNIT_ASSERT(value1 == value2);
}

#define DATA_SIZE 1024
#define SIZE (DATA_SIZE/sizeof(u_int32_t))

struct fifo_test_info_t {
    NoCopyFifo *fifo;
    u_int32_t *data;
};

static void *readFifo(void *p) {
    fifo_test_info_t *info = reinterpret_cast<fifo_test_info_t *>(p);
    u_int32_t *ptr;

    for(size_t i=0;i<SIZE;++i) {
        do {
            ptr = reinterpret_cast<u_int32_t *>(info->fifo->read(sizeof(u_int32_t)));
        } while(ptr == NULL);
        memcpy(info->data+i,ptr,sizeof(u_int32_t));
        info->fifo->readDone();
    }

    return NULL;
}

static void *writeFifo(void *p) {
    fifo_test_info_t *info = reinterpret_cast<fifo_test_info_t *>(p);
    u_int32_t *ptr;

    for(size_t i=0;i<SIZE;++i) {
        do {
            ptr = reinterpret_cast<u_int32_t *>(info->fifo->write(sizeof(u_int32_t)));
        } while(ptr == NULL);
        memcpy(ptr,info->data+i,sizeof(u_int32_t));
        info->fifo->writeDone();
    }

    return NULL;
}

void TestNoCopyFifo::testWrappingReadWrite(void) {
    NoCopyFifo fifo(BUFFER_SIZE);

    u_int32_t value1[SIZE];
    u_int32_t value2[SIZE];

    for(size_t i=0;i<SIZE;++i)
        value1[i] = rand();

    pthread_t reader, writer;
    fifo_test_info_t read_info = {
        &fifo,
        value2,
    };
    fifo_test_info_t write_info = {
        &fifo,
        value1,
    };

    pthread_create(&reader,NULL,readFifo,&read_info);
    pthread_create(&writer,NULL,writeFifo,&write_info);

    pthread_join(writer,NULL);
    pthread_join(reader,NULL);

    for(size_t i=0;i<SIZE;++i)
        CPPUNIT_ASSERT(value1[i] == value2[i]);
}
