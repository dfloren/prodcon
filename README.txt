Bounded Buffer Problem
1 producer, multiple consumer threads

Notes:
1. [Possible] memleak (Valgrind detects 0 leaks).
   Iteration testing with two scripts simultaneously:
   Script 1: ./prodcon 1 1 < inputexample
   Script 2: ./prodcon 800 2 < inputexample

   Script 2 segfault's infrequently(about 1 in 400 iterations). The scripts
   write to different log files but use the same file for input. Issues with 
   accessing one file at the same time? The segfault does not seem to occur when 
   testing is not done simultaneously.