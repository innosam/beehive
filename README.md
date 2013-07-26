BEEHIVE:

A Map Reduce framework
https://innosamcodes.wordpress.com/2013/04/07/simple-map-reduce-implementation-without-sorting/

> To test Map Reduce Algorithms on C++

> Learn more about Ashyncronous Network Programming

> Get Familiar with LINUX system calls


To Start Using MapReduce Algorithm with Multiple Clients


1> In MapReduce.cpp

Modify #PATH: Temporary and Output Files are Stored Here

2> Modify the appropriate  Task Name and Input File.

3> Compile the code.


 ./server \<port Number>

 ./tcli \<server address> \<server port> 
  CMD: start wordcount
  It does wordcount for the linux dictionary file
  (The input file is configurable)

 ./client \<server address> \<server port> \<groupId>

 You can run multiple clients either on the same or different Machine.


To add Custom Task:

> Look for sample task(wordcount) in MapReduce.cpp

> Both MAP and REDUCE Functions for a particular task has to be coded.

> Specify the input file, and task name
  

To Do:

1> Make Interface of CLI more user friendly

2> Add more sample task, that can be performed and scaled with this framework

3> Consider the possibility fo Implementing Table Join with the current framework

4> Improve on the current Implementation to support Hadoop like framework in C++


