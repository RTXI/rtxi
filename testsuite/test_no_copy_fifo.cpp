#include "test_no_copy_fifo.h"

#include <no_copy_fifo.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

CPPUNIT_TEST_SUITE_REGISTRATION(TestNoCopyFifo);

#define DAT_MAGIC 0xF0F05757
#define BUFFER_SIZE 127

void TestNoCopyFifo::testSingleReadWrite(void) {
    NoCopyFifo fifo(BUFFER_SIZE);
    uint32_t value1 = DAT_MAGIC;
    uint32_t value2 = 0;
    uint32_t *ptr;

    CPPUNIT_ASSERT(sizeof(value1) < BUFFER_SIZE);
    CPPUNIT_ASSERT_EQUAL(sizeof(value1),sizeof(value2));

    CPPUNIT_ASSERT((ptr = reinterpret_cast<uint32_t *>(fifo.write(sizeof(value1)))) != NULL);
    memcpy(ptr,&value1,sizeof(value1));
    fifo.writeDone();

    CPPUNIT_ASSERT((ptr = reinterpret_cast<uint32_t *>(fifo.read(sizeof(value2)))) != NULL);
    memcpy(&value2,ptr,sizeof(value2));
    fifo.readDone();

    CPPUNIT_ASSERT_EQUAL(value1,value2);
}

#define DATA_SIZE 1021
#define SIZE (DATA_SIZE/sizeof(uint32_t))

struct fifo_test_info_t {
    NoCopyFifo *fifo;
    uint32_t *data;
};

static void *readFifo(void *p) {
    fifo_test_info_t *info = reinterpret_cast<fifo_test_info_t *>(p);
    uint32_t *ptr;

    for(size_t i=0;i<SIZE;++i) {
        do {
            ptr = reinterpret_cast<uint32_t *>(info->fifo->read(sizeof(uint32_t)));
        } while(ptr == NULL);
        memcpy(info->data+i,ptr,sizeof(uint32_t));
        info->fifo->readDone();
    }

    return NULL;
}

static void *writeFifo(void *p) {
    fifo_test_info_t *info = reinterpret_cast<fifo_test_info_t *>(p);
    uint32_t *ptr;

    for(size_t i=0;i<SIZE;++i) {
        do {
            ptr = reinterpret_cast<uint32_t *>(info->fifo->write(sizeof(uint32_t)));
        } while(ptr == NULL);
        memcpy(ptr,info->data+i,sizeof(uint32_t));
        info->fifo->writeDone();
    }

    return NULL;
}

void TestNoCopyFifo::testWrappingReadWrite(void) {
    NoCopyFifo fifo(BUFFER_SIZE);

    uint32_t value1[SIZE];
    uint32_t value2[SIZE];

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
        CPPUNIT_ASSERT_EQUAL(value1[i],value2[i]);
}
