#include <cstdlib>
#include <rtfile.h>
#include <string.h>

#include "test_rtfile.h"

#define DATA_SIZE 1024

static const char file_name_template[] = "rtxi.test_rtfile.XXXXXX";
static const size_t file_name_template_size = sizeof(file_name_template)+1;

CPPUNIT_TEST_SUITE_REGISTRATION(TestRTFile);

void TestRTFile::setUp(void) {
    srand(time(NULL));
}

void TestRTFile::tearDown(void) {
}

void TestRTFile::testSingleWrite(void) {
    RTFile file;
    u_int32_t value1 = rand();
    u_int32_t value2;
    char file_name[file_name_template_size];
    int fd;

    strcpy(file_name,file_name_template);
    CPPUNIT_ASSERT((fd = mkstemp(file_name)) >= 0);

    CPPUNIT_ASSERT(file.open(file_name,O_CREAT|O_WRONLY|O_TRUNC));
    CPPUNIT_ASSERT_EQUAL(sizeof(value1),file.write(&value1,sizeof(value1)));
    file.close();

    close(fd);

    CPPUNIT_ASSERT((fd = open(file_name,O_RDONLY)) >= 0);
    CPPUNIT_ASSERT_EQUAL(sizeof(value2),static_cast<size_t>(read(fd,&value2,sizeof(value2))));
    close(fd);

    CPPUNIT_ASSERT_EQUAL(value1,value2);

    unlink(file_name);
}

void TestRTFile::testLargeWrite(void) {
    RTFile file;
    u_int32_t value1[DATA_SIZE];
    u_int32_t value2[DATA_SIZE];
    char file_name[file_name_template_size];
    int fd;

    strcpy(file_name,file_name_template);
    CPPUNIT_ASSERT((fd = mkstemp(file_name)) >= 0);

    CPPUNIT_ASSERT(file.open(file_name,O_CREAT|O_WRONLY|O_TRUNC));
    for(size_t i=0;i<DATA_SIZE;++i) {
        value1[i] = rand();
        CPPUNIT_ASSERT_EQUAL(sizeof(value1[0]),file.write(value1+i,sizeof(value1[0])));
    }
    file.close();

    // Verify that the data was written to file
    struct stat fstats;
    fstat(fd,&fstats);
    CPPUNIT_ASSERT_EQUAL(sizeof(value1),static_cast<size_t>(fstats.st_size));

    close(fd);

    CPPUNIT_ASSERT((fd = open(file_name,O_RDONLY)) >= 0);
    for(size_t i=0;i<DATA_SIZE;++i)
        CPPUNIT_ASSERT_EQUAL(sizeof(value2[0]),static_cast<size_t>(read(fd,value2+i,sizeof(value2[0]))));
    close(fd);

    for(size_t i=0;i<DATA_SIZE;++i)
        CPPUNIT_ASSERT_EQUAL(value1[i],value2[i]);

    unlink(file_name);
}

void TestRTFile::testSingleRead(void) {
    RTFile file;
    u_int32_t value1 = rand();
    u_int32_t value2;
    char file_name[file_name_template_size];
    int fd;

    strcpy(file_name,file_name_template);
    CPPUNIT_ASSERT((fd = mkstemp(file_name)) >= 0);
    CPPUNIT_ASSERT_EQUAL(sizeof(value1),static_cast<size_t>(write(fd,&value1,sizeof(value1))));
    close(fd);

    CPPUNIT_ASSERT(file.open(file_name,O_RDONLY));
    while(file.read(&value2,sizeof(value2)) != sizeof(value2));
    file.close();

    CPPUNIT_ASSERT_EQUAL(value1,value2);

    unlink(file_name);
}

void TestRTFile::testLargeRead(void) {
    RTFile file;
    u_int32_t value1[DATA_SIZE];
    u_int32_t value2[DATA_SIZE];
    char file_name[file_name_template_size];
    int fd;

    strcpy(file_name,file_name_template);
    CPPUNIT_ASSERT((fd = mkstemp(file_name)) >= 0);
    for(size_t i=0;i<DATA_SIZE;++i) {
        value1[i] = rand();
        CPPUNIT_ASSERT_EQUAL(sizeof(value1[0]),static_cast<size_t>(write(fd,value1+i,sizeof(value1[0]))));
    }
    fsync(fd);

    // Verify that the data was written to file
    struct stat fstats;
    fstat(fd,&fstats);
    CPPUNIT_ASSERT_EQUAL(sizeof(value1),static_cast<size_t>(fstats.st_size));

    close(fd);

    CPPUNIT_ASSERT(file.open(file_name,O_RDONLY));
    for(size_t i=0;i<DATA_SIZE;++i)
        CPPUNIT_ASSERT_EQUAL(sizeof(value2[0]),file.read(value2+i,sizeof(value2[0])));
    file.close();

    for(size_t i=0;i<DATA_SIZE;++i)
        CPPUNIT_ASSERT_EQUAL(value1[i],value2[i]);

    unlink(file_name);
}
