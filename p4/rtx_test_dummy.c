/*--------------------------------------------------------------------------
 *                      RTX Test Suite 
 *--------------------------------------------------------------------------
 */
/**
 * @file:   rtx_test_dummy.c   
 * @author: Thomas Reidemeister
 * @author: Irene Huang
 * @date:   2010.02.11
 * @brief:  rtx test suite 
 */

#include "rtx_test.h"
#include "dbug.h"
#include "rtx_inc.h"

volatile int testRan = 0;
volatile int testSuccess = 0;

// Test 1 variables
volatile int initVar = 0;
volatile int modVar = 0;

// Test 2 variables
volatile int ranFirst = 0;

void OutputPrefix( CHAR* output ){
    rtx_dbug_outs( (CHAR*)"G01_test: " );
    rtx_dbug_outs( output );
    rtx_dbug_outs( (CHAR*)"\r\n" );
}

// Slave -- Test 1
void test1(){ 
    // Wait for Process 2 to initialize the test
    while ( initVar == 0 ) {
        g_test_fixture.release_processor();
    }
    
    // Test is initialized now; set the modified variable to valid
    modVar = 1;

    // Reset process priority to lower priority; should pre-empt here
    g_test_fixture.set_process_priority( 1, 3 );

    // Pre-emption did not occur; set the modified variable to invalid
    if( initVar ){
        modVar = -1;
    }

    // Loop infinitely now that the test is finished
    while(1){
        g_test_fixture.release_processor();
    }
}

// Master -- Test 1
void test2(){
    int pid;
    void* msg = 0;

    msg = g_test_fixture.receive_message( &pid );

    g_test_fixture.release_memory_block( msg );

    // Set the initialization variable to let test1 run
	initVar = 1;

    // Set priority of process 1 to be higher than process 2; should pre-empt at this point
    g_test_fixture.set_process_priority( 1, 1 );

    // Reset the initialization variable to prevent process 1 from ruining test
    initVar = 0;

    if( modVar == 1 ){
        testSuccess |= 1;
    }

    // Reset modvar
    modVar = 0;

    // Ran Test 1
    testRan |= 1;

    // tell master  process that it's good to go
    msg = g_test_fixture.request_memory_block(); 
    g_test_fixture.send_message( 6, msg );

    // Reset priority level
    g_test_fixture.set_process_priority( 2, 3 );

    // Loop infinitely now that the test is finished
    while (1) {
        g_test_fixture.release_processor();
    }
}

// Master -- Test 2
void test3(){
    int pid;
    void* msg = 0;
    void* msg2 = 0;
    
    msg = g_test_fixture.receive_message( &pid );
    g_test_fixture.release_memory_block( msg );

    ranFirst = 0;

    msg = g_test_fixture.request_memory_block();
    msg2 = g_test_fixture.request_memory_block();

    g_test_fixture.send_message(
            5,
            msg
    );

    g_test_fixture.send_message(
            4,
            msg2 
    );

    while( ranFirst == 0 ){
        g_test_fixture.release_processor();
    }

    if( ranFirst == 4 ){
        testSuccess |= 2;
    }

    testRan |= 2;
    
    msg = g_test_fixture.request_memory_block();
    g_test_fixture.send_message( 6, msg );

    while( 1 ){
        g_test_fixture.release_processor();
    }
}

// Slave -- Test 2
void test4(){
    int pid;
    void* msg = 0;

    msg = g_test_fixture.receive_message( &pid );
    g_test_fixture.release_memory_block( msg );

    if( ranFirst == 0 ){
        ranFirst = 4;
    }

    g_test_fixture.set_process_priority( 4, 3 );

    while( 1 ){
        g_test_fixture.release_processor();
    }
}

// Slave -- Test 2
void test5(){
    int pid;
    void* msg = 0;

    msg = g_test_fixture.receive_message( &pid );
    g_test_fixture.release_memory_block( msg );

    if( ranFirst == 0 ){
        ranFirst = 5;
    }

    g_test_fixture.set_process_priority( 5, 3 );

    while( 1 ){
        g_test_fixture.release_processor();
    }
}

// Master -- Test Suite
void test6(){
    int success = 0;
    int pid = 0;
    void* msg = 0;

    testRan = 0;
    testSuccess = 0;

    msg = g_test_fixture.request_memory_block();
    g_test_fixture.send_message( 2, msg );

    msg = g_test_fixture.request_memory_block();
    g_test_fixture.send_message( 3, msg );

    while( testRan != 3 ) {
        msg = g_test_fixture.receive_message( &pid );
        g_test_fixture.release_memory_block( msg );
    }

    OutputPrefix( "START" );
    OutputPrefix( "2 tests total" );
    
    if( testSuccess & 1 ){
        OutputPrefix( "test 1 OK" );
        success++;
    } else {
        OutputPrefix( "test 2 FAIL" );
    }

    if( testSuccess & 2 ){
        OutputPrefix( "test 2 OK" );
        success++;
    } else {
        OutputPrefix( "test 2 FAIL" );
    }

    switch( success ){
        case 0:
            OutputPrefix( "0/2 tests OK" );
            OutputPrefix( "2/2 tests FAIL" );
            break;
        case 1:
            OutputPrefix( "1/2 tests OK" );
            OutputPrefix( "1/2 tests FAIL" );
            break;
        case 2:
            OutputPrefix( "2/2 tests OK" );
            OutputPrefix( "0/2 tests FAIL" );
            break;
    }

    OutputPrefix( "END" );

    g_test_fixture.set_process_priority( 6, 3 );

    // Done Tests -- go to sleep
    while (1) {
        g_test_fixture.release_processor();
    }
}

/* register the third party test processes with RTX */
void __attribute__ ((section ("__REGISTER_TEST_PROCS__")))register_test_proc()
{
    int i;

    rtx_dbug_outs((CHAR *)"rtx_test: register_test_proc()\r\n");

    for (i =0; i< NUM_TEST_PROCS; i++ ) {
        g_test_proc[i].pid = i + 1;
        g_test_proc[i].priority = 3;
        g_test_proc[i].sz_stack = 2048;
    }

    g_test_proc[0].entry = test1;

    g_test_proc[1].entry = test2;
    g_test_proc[1].priority = 2;

    g_test_proc[2].entry = test3;

    g_test_proc[3].entry = test4;
    g_test_proc[3].priority = 1;

    g_test_proc[4].entry = test5;

    g_test_proc[5].entry = test6;
    g_test_proc[5].priority = 0;
}

/**
 * Main entry point for this program.
 * never get invoked
 */
int main(void)
{
    rtx_dbug_outs((CHAR *)"rtx_test: started\r\n");
    return 0;
}
