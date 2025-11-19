# CS-214---P3

Project 3: My Shell

Author: Thai Nguyen 
NetID: tbn22

Author: Zachary Adam
NetID: zea6

-----------------------------------------------------------------------------------------------------

Test Plan: 

Test: Running a basic batchfile.

The my shell program should execute the commands of a batch file without interactive mode behavior such as: prompts, welcome message, and good-bye message.

Included is a file called batch1.txt. This file only contains the commands: 

echo hello
echo world
exit

and if run with the myshell like so:

./mysh batch1.txt

The output should be:

echo
world

no other included messages or prompts. 

---------------------------------------------------------------------------------------------------

Test: Testing die command in batch, and making sure nothing is ran after the die command (the shell exits from die).

In the file batch-die-test.txt the only commands contained are:

die some problem, finished
echo this-should-not-run

When run with the my shell program the same way as the first batch file was:

./mysh bath-die-test.txt

the output should be:

some problem, finished

and the shell should exit with exit code (1) indicating that die worked properly. There should not be any other message display. The echo command should not be reached in this batch file. 

----------------------------------------------------------------------------------------------------

Test: Running built in commands in interactive mode.

when the program is ran without any additional arguments it should run in interactive mode and initially display the welcome message and prompt:

Welcome to my shell!
mysh> 

When inputting a command such as exit or die that exits the shell it should print either the exit message from die and exit with exit code (1) or solely a goodbye message with no other message:

mysh> exit
Exiting my shell.

or 

mysh> die example message

example message 
Exiting my shell.

-----------------------------------------------------------------------------------------------------
