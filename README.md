# Temperature-Processes
A program with 7 processes, each managing its own temperature. The processes are set up as a binary tree, with Processes 1 &amp; 2 being the child processes of Process 0, and Processes 3&amp;4 and 5&amp;6 being the children of Processes 2 &amp; 3 respectively. When a process receives a temperature from its parent it will calculate its value according to a special equation and transmit this number to its children. When a process receives a temperature from its children it will calculate its value according to a special equation and transmit this number to its parent. When the calculated temperature is less than 1/100th degree different than the previous temperature for Process 0, the program ends and all temperatures are outputted. The processes communicate with each other through the POSIX message queue.
