#ifndef TEST_IMPL_DEF_H
#define TEST_IMPL_DEF_H


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <elf.h>
#include <execinfo.h>
#include <limits.h>


/* -- USER API -- */

/* define a package name above all your test code */
#define TEST_PACKAGE(PACKAGE_NAME)

/* use TEST(PACKAGE) instead of defining a testing function */
#define TEST(PACKAGE_NAME) 

/* after all your test code was defined, put this macro in the end of your test code to run the test */
#define START_TEST(PACKAGE_NAME) 

/* call this macro to log something */
#define TEST_LOG(fmt, ...)

/* assert */
#define TEST_ASSERT(exp)


/* -- UNDEFINE USER API -- */


#undef TEST_PACKAGE

#undef TEST

#undef START_TEST

#undef TEST_LOG

#undef TEST_ASSERT


/* -- STATIC MACROS -- */

#define MAX_LOG_LEN 1024

#if ULONG_MAX == UINT64_MAX
#define OSBITS_64_
#else 
#define OSBITS_32_
#endif

#ifdef OSBITS_64_
#define ehdr_ Elf64_Ehdr
#define shdr_ Elf64_Shdr
#endif

#ifdef OSBITS_32_ 
#define ehdr_ Elf32_Ehdr
#define shdr_ Elf32_Shdr
#endif

#define TEST_VAR_SECTION "TEST_SEC"

/* -- TYPE DECLARTION -- */

struct test_case_ins;
struct test_package_ins;
typedef int (*callback_) (struct test_case_ins * ins, struct test_package_ins *package); 

/* -- FUNCTION DECRLARTIONS -- */

void test_log_raw_(char *str);
void test_log_prefix_(char *str, char *f, int l);
void test_log_(char *f, int l, char *fmt, ...);
void test_run_all_test(struct test_package_ins *ins);
int test_assert(int res, char *file, int line, char *expr);
void test_print_stack();
void test_print_summary(struct test_package_ins *pkg);
struct test_package_ins* test_get_packages_(void);

/* -- TYPE DEFINATION -- */

struct test_case_ins {
    char *name;
    char *package;
    
    callback_ handle;

    int failed;
};


struct test_package_ins {
    int numtests;
    struct test_case_ins **ins;
    const char *name;

    int succeed;
    int failed;
};

/* -- FUNCTINAL MACROS -- */
#define TEST_PROTO_SUFFIX(TESTNAME, SUFFIX_)                                \
     test_func_proto_ ## TESTNAME ## _ ##SUFFIX_

#define TEST_PROTOTYPE(TESTNAME)                                            \
    TEST_PROTO_SUFFIX(TESTNAME, impl)

#define TEST_WRAPPER(TESTNAME)                                              \
    TEST_PROTO_SUFFIX(TESTNAME, wrapper)

#define TEST_DEFINE_WARPPER(NAME_)                                          \
    static int TEST_WRAPPER(NAME_) {                                        \
        void (*fp) (void) = TEST_PROTOTYPE(NAME_) ;                         \
        fp();                                                               \
    }

#define TEST_LOG(fmt, ...)                                                  \
    test_log_(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_AND_EXIT(fmt, ...) do {                                         \
    TEST_LOG(fmt, ##__VA_ARGS__) ;                                          \
    exit(1);                                                                \
    }while(0)


/* -- INTERNAL VARIABLES -- */
extern const char *unit_package_name_;

/* -- CODE INJECTION -- */

/* platfrom detection */
#if  defined(__GNUC__)

#define TEST_SECTION(SECTION) __attribute__((section(SECTION)))

#define TEST_INJECT_GET_PACKAGES(PACKAGE_NAME)                                                      \
    struct test_package_ins *test_get_packages_(void) {                                             \
                                                                                                    \
        static struct test_package_ins ins;                                                         \
        ins.name = unit_package_name_;                                                              \
        ins.succeed = 0;                                                                            \
        ins.failed = 0;                                                                             \
                                                                                                    \
        struct test_package_ins *ret = NULL;                                                        \
        int fd;                                                                                     \
        struct stat st;                                                                             \
                                                                                                    \
        fd = open("/proc/self/exe", O_RDONLY);                                                      \
        if (fstat(fd, &st) == -1) {                                                                 \
            LOG_AND_EXIT("STAT FILE FAILED [%s]", strerror(errno));                                 \
        }                                                                                           \
                                                                                                    \
        void *ptr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);                          \
        if (ptr == MAP_FAILED) {                                                                    \
            LOG_AND_EXIT("MMAP FAILED [%s]", strerror(errno));                                      \
        }                                                                                           \
                                                                                                    \
        const  ehdr_ *elf = (ehdr_ *)ptr;                                                           \
        const shdr_ *sechdr = (void *) ((char *)elf + elf->e_shoff);                                \
        const shdr_ *str_sechdr = sechdr + elf->e_shstrndx;                                         \
                                                                                                    \
        int i ;                                                                                     \
        for (i = 0; i != elf->e_shnum; i++) {                                                       \
            char *name = (char *)elf + str_sechdr->sh_offset + sechdr[i].sh_name;                   \
            if (!strcmp(name, TEST_VAR_SECTION)) {                                                  \
                ins.ins = (void *)ptr + sechdr[i].sh_offset;                                        \
                if (sechdr[i].sh_size % 8 != 0) {                                                   \
                    LOG_AND_EXIT("SECTION HAS INVALID SIZE [%d]", sechdr[i].sh_size);               \
                }                                                                                   \
                ins.numtests = sechdr[i].sh_size / 8;                                               \
                ret = &ins;                                                                         \
                break;                                                                              \
            }                                                                                       \
        }                                                                                           \
        return ret;                                                                                 \
    }

#define TEST_INJECT_RUN_ALL_TEST(PACKAGE_NAME)                                                      \
    void test_run_all_test(struct test_package_ins *pkg) {                                          \
        int i ;                                                                                     \
                                                                                                    \
        TEST_LOG("TESTING PACKAGE [%s] "                                                            \
                 "WITH [%d] "                                                                       \
                 "CASES ", unit_package_name_, pkg->numtests);                                      \
                                                                                                    \
        for (i = 0; i != pkg->numtests; i++) {                                                      \
            struct test_case_ins *ins = pkg->ins[i];                                                \
            if (ins->handle(ins, pkg) ) break;                                                      \
        }                                                                                           \
    }
#endif //end of platfrom detection


#define TEST_INJECT_TEST_LOG(PACKAGE_NAME)                                                          \
    void test_log_raw_(char *str) {                                                                 \
        printf(str);                                                                                \
        printf("\n");                                                                               \
        fflush(stdout);                                                                             \
    }                                                                                               \
    void test_log_prefix_(char *str, char *f, int l) {                                              \
        char msg[MAX_LOG_LEN];                                                                      \
        snprintf(msg, sizeof(msg), "[%s : %d] :  %s", f, l, str);                                   \
                                                                                                    \
        test_log_raw_(msg);                                                                         \
    }                                                                                               \
    void test_log_(char *f, int l, char *fmt, ...) {                                                \
        va_list ap;                                                                                 \
        char msg[MAX_LOG_LEN];                                                                      \
                                                                                                    \
        va_start(ap, fmt);                                                                          \
        vsnprintf(msg, sizeof(msg), fmt, ap);                                                       \
        va_end(ap);                                                                                 \
        test_log_prefix_(msg, f, l);                                                                \
    }

#define TEST_INJECT_PRINT_STACK(PACKAGE_NAME)                                                       \
    void test_print_stack(void) {                                                                   \
        int trace_size;                                                                             \
        void *trace[100];                                                                           \
        char **messages = NULL;                                                                     \
        int i;                                                                                      \
                                                                                                    \
        trace_size = backtrace(trace, 100);                                                         \
                                                                                                    \
        messages = backtrace_symbols(trace, trace_size);                                            \
        test_log_raw_("-- STACK START --");                                                         \
        for (i = 0; i != trace_size; i++) {                                                         \
            test_log_raw_(messages[i]);                                                             \
        }                                                                                           \
        test_log_raw_("-- STACK END --");                                                           \
    }

#define TEST_INJECT_WRAPPER(NAME_)                                                                  \
    static int TEST_WRAPPER(NAME_) (struct test_case_ins *ins, struct test_package_ins *package);   \
    static int TEST_WRAPPER(NAME_) (struct test_case_ins *ins, struct test_package_ins *package) {  \
        TEST_LOG("TESTING [%s]", ins->name);                                                        \
        TEST_PROTOTYPE(NAME_)(ins, package);                                                        \
        if (!ins->failed) {                                                                         \
            TEST_LOG("<PASSED>  [%s] " ,ins->name);                                                 \
            package->succeed ++;                                                                    \
        } else {                                                                                    \
            TEST_LOG("<FAILED> "                                                                    \
                    "[%s]  "                                                                        \
                    "IN PACKAGE [%s]",                                                              \
                    ins->name, package->name);                                                      \
            package->failed ++;                                                                     \
            return 1;                                                                               \
        }                                                                                           \
        return 0;                                                                                   \
    } 

#define TEST_ASSERT(EXPR) do {                                                                      \
        if (test_assert((EXPR), __FILE__, __LINE__, #EXPR)) {                                       \
            ins->failed = 1;                                                                        \
            return 0;                                                                               \
        }                                                                                           \
    } while (0)

#define TEST_INJECT_ASSERT(PACKAGE_NAME)                                                            \
    int test_assert(int res, char *file, int line, char *expr) {                                    \
        if (res == 0) {                                                                             \
            char msg[MAX_LOG_LEN];                                                                  \
            snprintf(msg, sizeof(msg), "ASSERT FAIED AT FILE: [%s]"                                 \
                    " LINE: [%d] "                                                                  \
                    "EXPRESSION : [ %s ]", file, line, expr);                                       \
            test_log_(file, line, msg);                                                             \
            test_print_stack();                                                                     \
            return 1;                                                                               \
        }                                                                                           \
        return 0;                                                                                   \
    }

#define TEST_INJECT_PRINT_SUMMARY(PACKAGE_NAME)                                                     \
    void test_print_summary(struct test_package_ins *pkg) {                                         \
        TEST_LOG("TEST FINISHED [%d] PASSED , "                                                     \
                "[%d] FAILED",                                                                      \
                pkg->succeed, pkg->failed);                                                         \
    }                                                                                               \

#define TEST_INJECT_PRINT_GREETING(PACKAGE_NAME) \
    void test_print_greeting(void) {                                                                \
        /* nothing to be greeted about once u dancing with evil */                                  \
    }

#define TEST_INJECT_FUNCTIONS(PACKAGE_NAME)                                                         \
    TEST_INJECT_TEST_LOG(PACKAGE_NAME)                                                              \
    TEST_INJECT_RUN_ALL_TEST(PACKAGE_NAME)                                                          \
    TEST_INJECT_PRINT_STACK(PACKAGE_NAME)                                                           \
    TEST_INJECT_ASSERT(PACKAGE_NAME)                                                                \
    TEST_INJECT_PRINT_GREETING(PACKAGE_NAME)                                                        \
    TEST_INJECT_PRINT_SUMMARY(PACKAGE_NAME)                                                         \
    TEST_INJECT_GET_PACKAGES(PACKAGE_NAME)

#define TEST_INJECT_VARIABLES(PACKAGE_NAME)                                                         \
    const char *unit_package_name_ = #PACKAGE_NAME;

#define TEST_INJECT_PROTOTYPE(TESTNAME) \
    int TEST_PROTOTYPE(TESTNAME) (struct test_case_ins *ins, struct test_package_ins *package) ;    \


#define TEST_PACKAGE(PACKAGE_NAME)                                                                  \
        TEST_INJECT_VARIABLES(PACKAGE_NAME)                                                         \
        TEST_INJECT_FUNCTIONS(PACKAGE_NAME)

#define TEST(TESTNAME)                                                                              \
    TEST_INJECT_PROTOTYPE(TESTNAME)                                                                 \
    TEST_INJECT_WRAPPER(TESTNAME)                                                                   \
    struct test_case_ins TEST_PROTO_SUFFIX(TESTNAME, info) = {                                      \
       .name = #TESTNAME ,                                                                          \
       .handle = TEST_WRAPPER(TESTNAME),                                                            \
       .failed = 0,                                                                                 \
    };                                                                                              \
    TEST_SECTION(TEST_VAR_SECTION)                                                                  \
    struct test_case_ins *TEST_PROTO_SUFFIX(TESTNAME, ptr) = &TEST_PROTO_SUFFIX(TESTNAME, info);    \
    int TEST_PROTOTYPE(TESTNAME) (struct test_case_ins *ins, struct test_package_ins *package) 



#define START_TEST(PACKAGE_NAME)                                                                    \
    int main (int argc, char **argv) {                                                              \
        test_print_greeting();                                                                      \
        struct test_package_ins *ins = test_get_packages_();                                        \
        if (ins == NULL) {                                                                          \
            LOG_AND_EXIT("GET PACKAGES FAILED");                                                    \
        }                                                                                           \
        test_run_all_test(ins);                                                                     \
        test_print_summary(ins);                                                                    \
        return 0;                                                                                   \
    }


#endif // TEST_IMPL_DEF_H

