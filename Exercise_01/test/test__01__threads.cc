#include <catch2/catch_test_macros.hpp>

///////////////////////////
//                       //
// BEGIN MODIFYABLE CODE //
//                       //
///////////////////////////

#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>

using namespace std;

TEST_CASE("each thread output appears exactly once and intact")
{
  //
  // Launch N_threads threads and collect their outputs.
  //
  mutex t_lock;
  int32_t const N_threads = 10;
  vector<thread> threads;
  
  ostringstream ostr;
  for (int32_t idx=0; idx<N_threads; ++idx)
  {
    threads.push_back(
      thread(
        [&ostr, &t_lock, idx] ()  {
          t_lock.lock();
          ostr << "This is the output for thread idx=" << idx << endl;
          t_lock.unlock();
        }
      )
    );
  }

  //
  // LOOP A
  //
  for (auto & t : threads) {
    t.join();
  }
  /*
  This loop iterates over the threads vector using the iterator t. 
  Each vector element in threads is a thread, and t references one element at a time.
  t.join() joins the selected thread. Joining a thread means that once it completes its 
  tasks, it will then return to the calling thread. 
  Using join() blocks the calling thread preventing it from proceeding until all threads 
  have completed their work.
  */

  string program_output_str = ostr.str();
  INFO("The test failed because the output strings were not intact and/or\n"
          "the thread index occurred more than once.");
  INFO("\nThe output strings are:");
  INFO(program_output_str);
  /////////////////////////
  //                     //
  // END MODIFYABLE CODE //
  //                     //
  /////////////////////////

  //
  // Verify that the output for each thread appears
  // exactly once and intact.
  //
  for (int32_t idx=0; idx<N_threads; ++idx)
  {
    ostringstream target_ostr;
    target_ostr << "This is the output for thread idx=" << idx << endl;
    auto target_str = target_ostr.str();

    //
    // LOOP B
    //
    int32_t N_matches = 0;
    size_t pos = program_output_str.find(target_str, 0);
    while (pos != string::npos) {
      ++N_matches;
      pos = program_output_str.find(target_str, pos+1);
    }  /*
       A few things I had to lookup to understand this loop:
       string::npos --> this is the max value for size_t
       str.find() --> the first parameter is the string to search for, 
       the 2nd parameter is the position of the first character to begin 
       the search. This returns the position of the first character of 
       the first match, unless no match is found in which case string::npos is returned.
       
       Given the above info, what loop B is doing is comparing the strings 
       created by the threads with the current string created by the main thread on 
       each iteration of the for loop. If the strings match, pos will not equal 
       string::npos (since pos will be some relatively small position value and 
       string::pos will be some  very large value) and will then enter the loop.
       N_matches will increment and the starting position of find() will increment. 
       Since the strings will no longer  match once pos is incremented, find() should 
       now return string::npos and when the loop condition is checked again, 
       pos will now equal string::npos and the loop will terminate.
       */

    CHECK(N_matches == 1); // LINE C
    /*
    Line C confirms that the string created by the thread occurs only once. I think
    I kind of explained a bit of this on loop B, but if N_matches is 0 that means that
    the target string was not found. If N_matches is greater than 1, this means that
    the string occurs more than once. So, N_matches should equal exactly one to meet
    the criteria that each thread output occurs only once and is intact.
    */
  }
}

/*
  Why this code works as expected:
  I made 2 changes: I added a mutex to ensure exclusive write access to the ostr
  variable (and captured the mutex by reference into the lambda), and I changed 
  the lambda's idx capture to be by value. Two things were causing tests to fail:
  writes to the ostr buffer were occasionally being interrupted by another thread,
  and the idx variable was being accessed simultaneously by multiple threads. When 
  the ostr buffer was interrupted, the target_str could not be found in 
  program_output_str, and when the same idx value was accessed by more than 1 thread,
  the same string was written more than once.

*/
