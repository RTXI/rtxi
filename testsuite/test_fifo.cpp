#include "test_fifo.h"

#include <fifo.h>

CPPUNIT_TEST_SUITE_REGISTRATION(TestFifo);

#define DAT_MAGIC 0xF0F05757
#define BUFFER_SIZE 128

void TestFifo::testSingleReadWrite(void) {
    Fifo fifo(BUFFER_SIZE);
    u_int32_t value1 = DAT_MAGIC;
    u_int32_t value2 = 0;

    CPPUNIT_ASSERT(sizeof(value1) < BUFFER_SIZE);
    CPPUNIT_ASSERT_EQUAL(sizeof(value1),sizeof(value2));
    CPPUNIT_ASSERT_EQUAL(sizeof(value1),fifo.write(&value1,sizeof(value1)));
    CPPUNIT_ASSERT_EQUAL(sizeof(value2),fifo.read(&value2,sizeof(value2)));
    CPPUNIT_ASSERT_EQUAL(value1,value2);
}

#define DATA_SIZE 1024
#define SIZE (DATA_SIZE/sizeof(u_int32_t))

struct fifo_test_info_t {
    Fifo *fifo;
    u_int32_t *data;
};

static void *readFifo(void *p) {
    fifo_test_info_t *info = reinterpret_cast<fifo_test_info_t *>(p);

    size_t n;
    for(size_t i=0;i<SIZE;++i)
        do {
            n = info->fifo->read(info->data+i,sizeof(u_int32_t));
        } while(n != sizeof(u_int32_t));

    return NULL;
}

static void *writeFifo(void *p) {
    fifo_test_info_t *info = reinterpret_cast<fifo_test_info_t *>(p);

    size_t n;
    for(size_t i=0;i<SIZE;++i)
        do {
            n = info->fifo->write(info->data+i,sizeof(u_int32_t));
        } while(n != sizeof(u_int32_t));

    return NULL;
}

void TestFifo::testWrappingReadWrite(void) {
    Fifo fifo(BUFFER_SIZE);

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
        CPPUNIT_ASSERT_EQUAL(value1[i],value2[i]);
}
