/*
   Project: Real Time Embedded Systems 
   User Space Code for Receiver End
   
   Description: 
	1- Reads the Status FIFO for any message from the Kernel Module
	2- In case, "OK" data appears on the Status FIFO, pick up the data from Serial_FIFO_NUM
	3- Display one of the 5 Packages of Data received
	4- Compute Average of all the 5 Data Packages received
   Author: Muhammad Umer Huzaifa
   Date: 05/12/2014
*/
#include <stdio.h>
#include <unistd.h>		/* open() */
#include <fcntl.h>		/* O_RDONLY */
#include "common.h"		/* declarations for our command and status*/ 
int main()
{
  int data_fd;
  int status_fd;
  int i = 0;
  unsigned char status[2];  
  int k;  
  int retval;
  int pkg_size=1835;   /* Number of Bytes in 5 packages
  int data_size = 367;
  int data_set_size = 181;
  unsigned char unit_data[data_size];
  unsigned char pkg_data[pkg_size];
  unsigned short data[data_set_size];
  int count = 0;
  int average_count = 5;
  int sum[5];
  int average=0;
  if ((data_fd = open(Serial_FIFO_DEV, O_RDONLY)) < 0) 
  {
    fprintf(stderr, "error opening %s\n", Serial_FIFO_DEV);
    return 1;
  }
  if ((status_fd = open(Status_FIFO_DEV, O_RDONLY)) < 0) {
    fprintf(stderr, "error opening %s\n", Status_FIFO_DEV);
    return 1;
  }
  retval = 0;
  printf("Data Received.\n");
  for( i = 0; i < data_set_size; i++)
  {
	data[i] = 0;
  }
  for( i = 0; i < 5; i++)
  {
	sum[i] = 0;
  }
/* Loop for Reading From FIFO */
  while(1)
  { if (sizeof(status) == read(status_fd, &status, sizeof(status)))   /* Inter Process Communication  */  {			/* Reading the Serial Data Containing FIFO only when Status FIFO sends "OK" */										
           if (status[0]=='O'&&status[1]=='K')
	{
    if (pkg_size == read(data_fd, pkg_data, pkg_size))    /* Only if 5 packages are there in the FIFO */
       {
            printf("5 packages received \n Displaying One Package \n");
 	for (i=0; i<data_size;i++)			/* Displaying one of the 5 packages */
		{
		printf("Data[%d]=%d \n",i,pkg_data[i]);	}

	 	}
	  printf("Computing Average of Data Excluding Header and Tail \n");
		for (count=0;count<5;count++)
		{
		for (i=4+count*367;i<(count+1)*data_size-1;i++)
			{
				sum[count]+=pkg_data[i];		
			} 	   
			average+=sum[count];
			sum[count]=0;
		}	
		average=average/1810;
		printf("Average of 5 Datasets is %d",average);
	}
	average=0; 
	}	
   }
    close(data_fd);
 return retval;
}
