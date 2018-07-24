#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
//#include "CUnit/Automated.h"
//#include "CUnit/Console.h"

#include "../encoders/protobuf.h"

#include <stdio.h>  // for printf

/* Test Suite setup and cleanup functions: */

int init_suite(void) { return 0; }

int clean_suite(void) { return 0; }

/************* Test case functions ****************/

void test_case_sample(void) {
  CU_ASSERT(CU_TRUE);
  CU_ASSERT_NOT_EQUAL(2, -1);
  CU_ASSERT_STRING_EQUAL("string #1", "string #1");
  CU_ASSERT_STRING_NOT_EQUAL("string #1", "string #2");

  CU_ASSERT(CU_FALSE);
  CU_ASSERT_EQUAL(2, 3);
  CU_ASSERT_STRING_NOT_EQUAL("string #1", "string #1");
  CU_ASSERT_STRING_EQUAL("string #1", "string #2");
}

void test_logline(void) {
  char* logline =
      "{\"t\": \"click\", \"ts\": \"1412235349.190\", \"cid\": \"12334354\"}";
  size_t size;
  char* byte_array = encode_json2pb(logline, strlen(logline), &size);

  const char* result = decode_pb2json(byte_array, size);
  // The field order is sometimes different from the original. Need to fix

  CU_ASSERT_STRING_EQUAL(logline, result);
}

/************* Test Runner Code goes here **************/

int main(void) {
  CU_pSuite pSuite = NULL;

  /* initialize the CUnit test registry */
  if (CUE_SUCCESS != CU_initialize_registry()) return CU_get_error();

  /* add a suite to the registry */
  pSuite = CU_add_suite("logspec_test_suit", init_suite, clean_suite);
  if (NULL == pSuite) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  /* add the tests to the suite */
  if ((NULL == CU_add_test(pSuite, "test_logline", test_logline))) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  // Run all tests using the basic interface
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  printf("\n");
  CU_basic_show_failures(CU_get_failure_list());
  printf("\n\n");
  /*
     // Run all tests using the automated interface
     CU_automated_run_tests();
     CU_list_tests_to_file();

     // Run all tests using the console interface
     CU_console_run_tests();
   */
  /* Clean up registry and return */
  CU_cleanup_registry();
  return CU_get_error();
}
