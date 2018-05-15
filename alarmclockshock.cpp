#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <time.h>
#include <string>
#include <sstream>
#include <fstream>
#include <math.h>

#include <ugpio/ugpio.h>


bool gpioRequest(int pin);//function declaration


int writefile(bool lightOne, bool lightTwo, bool lightThree)
{
	time_t today = time(NULL);
	tm* timePtrToToday = localtime(&today);


	std::ofstream myfile;
	myfile.open("stats.stat",std::ios_base::app);

	myfile << lightOne << ',';
	myfile << lightTwo << ',';
	myfile << lightThree << ',';
	myfile << timePtrToToday->tm_hour << ',';
	myfile << timePtrToToday->tm_min << ',';
	myfile << timePtrToToday->tm_sec << ',';
	myfile << timePtrToToday->tm_mday << ',';
	myfile << timePtrToToday->tm_mon+1 << "\n";

	myfile.close();
}


int main(int argc, char **argv, char **envp)
{
	int value;
	if (argc < 2) {
		printf("Usage: newg <gpio>\n\n");
		printf("Reads the input value of the specified GPIO input pins (even numbered pins) once every two seconds. \n");
		printf("Outputs to specified GPIO output pins (odd numbered pins) once every two seconds. \n");
		exit(-1);
	}
	int size = argc - 1;
	int gpioArray[size];

	if (size %2 == 1)
	{
		printf("Warning: use an even number of arguments alternating input and output.\n\n");
	}

	//Fills the gpio array with the gpios in use
	for (int i = 1; i < argc; i++)
	{
		gpioArray[i-1] = atoi(argv[i]);
		if(!gpioRequest(gpioArray[i-1]))
		{
			printf("ERROR REQUESTING GPIO");
			return EXIT_FAILURE;
		}
	}

	////////////////////////this pin is used to end the main loop, set it to input
	gpioRequest(8);
	gpio_direction_input(8);
	////////////////////////



	// set directions of input/output alternating.
	// starts with input and oscillates
	for (int i = 0; i < size; i++)
	{
		if(i%2 == 0)
		{
			printf("PGIO%d is an input pin. \n",gpioArray[i]);
			gpio_direction_input( gpioArray[i] );
		}
		else
		{
			printf("PGIO%d is an output pin. \n",gpioArray[i]);
			gpio_direction_output( gpioArray[i] , 0 );
		}
	}
	
	for (int i = 0; i < size; i++)
	{
		if (i%2==0)
			printf(">>> begin reading GPIO%d\n",gpioArray[i]);
	}
	//initialize all the pin variables
	bool pin1 = 0;
	bool pin2 = 0;
	bool pin3 = 0;
	bool oldPin1 = 0;
	bool oldPin2 = 0;
	bool oldPin3 = 0;

	// read the gpio pins until pin 8 recieves input, this is how you end the program
	while (gpio_get_value(8) == 0) 
	{ 
		for (int j = 0; j < size; j++)
		{
			//read only the input gpios, aka the even numbered ones
			if (j%2==0)
			{
				value = gpio_get_value(gpioArray[j]);
				printf("  > Read GPIO%d: value '%d'\n", gpioArray[j], value);
				gpio_direction_output(gpioArray[j+1],value);
				pin1 = gpio_get_value(gpioArray[0]);
				pin2 = gpio_get_value(gpioArray[2]);
				pin3 = gpio_get_value(gpioArray[4]);
			}
		}

		// do checks for which pins have changed since last check
		if( ( pin1 != oldPin1 ) || ( pin2 != oldPin2 ) || ( pin3 != oldPin3 ) )
		{
			writefile(gpio_get_value(gpioArray[0]),gpio_get_value(gpioArray[2]),gpio_get_value(gpioArray[4]));
		}


		//divide between each read.
		printf("<|><><><><><><><><><><><><><><|>\n");

		oldPin1 = pin1;
		oldPin2 = pin2;
		oldPin3 = pin3;
		//sleep between each read
		sleep(5);
	}


	// unexport the gpio
	for (int i = 0; i < size; i++)
	{
		gpio_free(gpioArray[i]);
	}
	return 0;
}


bool gpioRequest(int pin)
{
	if (gpio_is_requested(pin) < 0)
	{
		perror("error");
		return false;
	}
	// request and export the pin GPIO
	if (!gpio_is_requested(pin))
	{
		printf("requesting GPIO %d \n", pin );
		if (gpio_request(pin, NULL) < 0)
		{
			perror("error");
			return false;
		}
	}
	return true;
}
