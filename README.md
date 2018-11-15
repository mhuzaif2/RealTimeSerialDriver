# RealTimeSerialDriver
This was a course project required for the course SYEN 7306 - Real Time Systems. The project comprises receiver and sender end code written in C++.

Project Objective:
To achieve a real time serial communication between a simulated sensor device and a receiver system. The sensor has a given data sending rate and a specific data pattern. We want a pre-emption free mechanism for collecting the data at receiver. For that we have developed a real time thread that can collect a specific amount of data without being pre-empted by any other process and notify a user space application when the data is ready for collection.

Sensor:

As a part of the study, we simulated the sensor in another computer system and achieved following simulated sensor parameters;

- 181 elements per packet: each element containing two data bytes.
- A header containing four ASCI characters, 'A', 'S', 'C' and 'I'
- A tail consisting of one byte, '0x01'
- Sending Rate: 20 Packets per second

Kernel Module at Receiver End:

At the receiver end, we defined a real time kernel module file for receiving the data from the serial port. We could have used the traditional applications for receiving the data but we chose this kernel module because:
- We don't want data reception to hinder any other processor activities
- We want to collect the data in the form of batches, each batch containing 5 data packets

Following are the main parts of the Kernel Module for receiver:
- Defining a real time thread for the receiving function.
- Opening Serial Port for receiving the data
- Opening a FIFO for putting the received data (for user space application to use henceforth)
- Opening a FIFO for updating the status of reception to the user-space application.
- Reading the Serial Port; Checking if the header and tail match with the sensor specifications
- Put the Packet in FIFO if it is correct (Has the desired header and tail)
- When five such packets are received and put in the FIFO, update the status for user-space by putting two status characters in the status FIFO ('O','K' in our case)
- perform this real time task periodically with the period calculated for 20 Packets /second.

User Space Application at Receiver End:
User space application at the receiver end is like a normal non real time application. It's main purposes are:
- Pick the data from FIFO after inter-process communication.
- Separate the header and tail from all of the packets.
- Compute the average of the rest of the data.

Inter-Process Communication:

Inter-process communication means the communication between kernel modules and user-space applications. In our case, this is achieved using the FIFOs. As explained above, in the receiver, the receiver kernel module receives the data from the serial port and intimates the user-space application when 5 packages are received through the FIFO. The user-space application in turn responds in the following way:

- Keep checking the status FIFO for status characters.
- In case the correct status characters turn up, load all the 5 packets of data from the data FIFO and do further processing.

Problems Faced in the project:

- Finding right version of RTAI (Real Time flavor of Linux that we chose) that could run with the systems we had at our disposal.
- Inserting or removing module, at times, caused the computer to hang. Proper consideration of timing and wait condition at the end of real time thread solved the problem.
- Most of the time, inserted module did not start responding (e.g. start receiving the data) immediately. Sometimes, even repeated removal and then insertion of module was done to solve this problem.


Steps for running the files:

We have two sets of kernel codes; One for the simulated sensor computer and the other for the receiver computer.

1.	Make sure you have real time patch for linux installed in both systems (simulated sensor and the receiver). Make sure you have the following feature:
  1.	Add-ons: Serial Line Driver (Enabled)
  2.	After patch has been made, you can test if the real time patch is working or not by running the latency test “run“ shell script that is available in usr/realtime/testsuite/kern/latency/
  3.	Include the rtai modules that are available in usr/realtime/modules. (You can do this by accessing the folder and individually inserting the modules in kernel using “insmod” or you can make a shell script to do this for all the modules in a for loop statement)
  4.	Now define rtf0 and rtf1 devices to be used as FIFO in the receiver computer. You can do this by using the following command:
  “ mknod -m 666 /dev/rtf0 c 150 0 ”
  “ mknod -m 666 /dev/rtf1 c 150 1 ”
  5.	Now make the kernel module files “.ko” from the kernel module “.c” files using “Makefile”. Write the “Makefile” for both the kernel modules (for simulated sensor and the receiver) in the following format:
“KDIR 	:= /usr/src/linux		/* Defines location of your linux kernel files */
PWD	:= $(shell pwd) 	               /* Your current directory */
EXTRA_CFLAGS := -I/usr/realtime/include -O2           /* Path for RTAI libraries */
obj-m	:= sensor.o          /* Target kernel module file (must be the same as the .c file name) */
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions”
6.	use “make” command in the terminal to run the “Makefile” for each kernel module file. “.ko” files will be generated. Now use “insmod <kernel_module_name>.ko” to insert the module in the kernel. Do this for both, simulated sensor and the receiver. 
7.	You can use “dmesg” command in the terminal to see the kernel module outputs of the kernel module files. This can be used to see if there are any errors popping up and for debugging.
8.	Now, for running the user-space application we have to compile the “.c” file of the user-space application and make an executable. We can do that in the following manner:
“gcc -o <executable_name> <user_application_file_name>.c”
 	
As a result <executable_name> executable is generated. It can be run using “./<executable>” in 	the terminal.

REFERENCES:

1.	Tutorial for installing RTAI: https://www.rtai.org/userfiles/downloads/RTAILAB/RTAI-TARGET-HOWTO.txt
2.	Serial Communication in Linux: http://ulisse.elettra.trieste.it/services/doc/serial/basics.html
3.	Interprocess Communication Tutorial: http://tldp.org/LDP/lpg/node7.html

