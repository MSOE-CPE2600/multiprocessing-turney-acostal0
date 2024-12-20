/*********************************************
 * Filename: mandel.c
 * Assignment: Lab 12
 * Section: 121
 * Description: This lab generates a Mandelbrot set images using multithreading.
 * Author: Lizbeth Acosta
 * Date: 11/19/24
 * Note: compile with
 *********************************************/
/// 
//  mandel.c
//  Based on example code found here:
//  https://users.cs.fiu.edu/~cpoellab/teaching/cop4610_fall22/project3.html
//
//  Converted to use jpg instead of BMP and other minor changes
//  
///
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "jpegrw.h"
#include <pthread.h>

typedef struct {
    imgRawImage *img; //image being generated, pointer to image structure
    double xmin, xmax, ymin, ymax; //boundries
    int max; //max num of iterations
    int start_row, end_row; //rows of the image that each thread will do
} ThreadData;

// local routines
static int iteration_to_color( int i, int max ); //convert iteration count to color
static int iterations_at_point( double x, double y, int max ); //calculate iteration for a point in the Madelbrot set
static void compute_image( imgRawImage *img, double xmin, double xmax,
									double ymin, double ymax, int max, int num_threads ); //compute Mandelbrot set
static void show_help(); //help message
void* thread_compute(void* arg); //method executes by each thread for computing the Mandelbrot set


int main( int argc, char *argv[] )
{
	char c;

	// These are the default configuration values used
	// if no command line arguments are given.
	const char *outfile = "mandel.jpg";
	double xcenter = 0;
	double ycenter = 0;
	double xscale = 4;
	double yscale = 0; // calc later
	int    image_width = 1000;
	int    image_height = 1000;
	int    max = 1000;
	int num_threads = 1;

	// For each command line argument given,
	// override the appropriate configuration value.

	while((c = getopt(argc,argv,"t:x:y:s:W:H:m:o:h"))!=-1) {
		switch(c) 
		{
			case 't':
				num_threads = atoi(optarg);
            	if (num_threads < 1 || num_threads > 20) {
                	fprintf(stderr, "Error: Number of threads must be between 1 and 20.\n");
                	exit(1);
            	}
            	break;
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				xscale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'h':
				show_help();
				exit(1);
				break;
		}
	}

	// Calculate y scale based on x scale (settable) and image sizes in X and Y (settable)
	yscale = xscale / image_width * image_height;

	// Display the configuration of the image.
	printf("mandel: x=%lf y=%lf xscale=%lf yscale=%1f max=%d outfile=%s\n",xcenter,ycenter,xscale,yscale,max,outfile);

	// Create a raw image of the appropriate size.
	imgRawImage* img = initRawImage(image_width,image_height);

	// Fill it with a black
	setImageCOLOR(img,0);

	// Compute the Mandelbrot image
	//compute_image(img,xcenter-xscale/2,xcenter+xscale/2,ycenter-yscale/2,ycenter+yscale/2,max);
	compute_image(img, xcenter - xscale / 2, xcenter + xscale / 2, ycenter - yscale / 2, ycenter + yscale / 2, max, num_threads);


	// Save the image in the stated file.
	storeJpegImageFile(img,outfile);

	// free the mallocs
	freeRawImage(img);

	return 0;
}




/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iter;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void compute_image(imgRawImage* img, double xmin, double xmax, double ymin, double ymax, int max, int num_threads )
{
	pthread_t threads[num_threads]; //thread Id's
    ThreadData thread_data[num_threads]; //thread data

    int rows_per_thread = img->height / num_threads; //each thread does a specific range of rows
    int remainder_rows = img->height % num_threads; // extra threads if can't divide by the num of threads

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].img = img; //share image among threads
        thread_data[i].xmin = xmin; //left bound for region
        thread_data[i].xmax = xmax; //right bound for region
        thread_data[i].ymin = ymin; //lower bound for region
        thread_data[i].ymax = ymax; //upper bound for region
        thread_data[i].max = max; //max number of iterations for each point

		//assigns a specific range of rows to each thread
        thread_data[i].start_row = i * rows_per_thread;
        thread_data[i].end_row = (i + 1) * rows_per_thread;

        if (i == num_threads - 1) {
            thread_data[i].end_row += remainder_rows; //add remainder rows to the last thread
        }

        pthread_create(&threads[i], NULL, thread_compute, &thread_data[i]); 
		//makes the thread to compute part of the image, passing ThreadData struct to each thread
    }

    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL); //waits for all threads to finish before saving
    }
}

void* thread_compute(void* arg) {
	//calculates the pixel values for the part of the image assigned
    ThreadData* data = (ThreadData*)arg; //cast argument to ThreadData type
    int i, j;
    int width = data->img->width; //image width

	//loops through the assigned rows and columns
    for (j = data->start_row; j < data->end_row; j++) {
        for (i = 0; i < width; i++) {
			//maps pixel coordinates to mandelbrot coordinates
            double x = data->xmin + i * (data->xmax - data->xmin) / width;
            double y = data->ymin + j * (data->ymax - data->ymin) / data->img->height;

			//calculate the number of iterations
            int iters = iterations_at_point(x, y, data->max);
			//set the pixel color based on number of iterations
            setPixelCOLOR(data->img, i, j, iteration_to_color(iters, data->max));
        }
    }

    return NULL; //retrun from thread
}


/*
Convert a iteration number to a color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/
int iteration_to_color( int iters, int max )
{
	int color = 0xFFFFFF*iters/(double)max;
	return color;
}


// Show help message
void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates (X-axis). (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
	printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}