/*********************************************
 * Filename: mandelmovie.c
 * Assignment: Lab 11
 * Section: 121
 * Description: This lab generates a Mandelbrot set images using multiprocessing.
 * Author: Lizbeth Acosta
 * Date: 11/19/24
 * Note: compile with
 * $ make
 * $ ./mandelmovie -p <num processes> -f <num frames>
 * 
 *********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

void print_usage(const char *name);

int main(int argc, char *argv[]) {
    int processes = 1;  //default to 1 process
    int frames = 50;    //default to 50 frames
    int opt;

    //parse command-line args
    while ((opt = getopt(argc, argv, "p:f:")) != -1) {
        switch (opt) {
            case 'p':
                processes = atoi(optarg);
                if (processes < 1) {
                    fprintf(stderr, "Number of processes must be at least 1.\n");
                    print_usage(argv[0]);
                }
                break;
            case 'f':
                frames = atoi(optarg);
                if (frames < 1) {
                    fprintf(stderr, "Number of frames must be at least 1.\n");
                    print_usage(argv[0]);
                }
                break;
            default:
                print_usage(argv[0]);
        }
    }

    //make sure parameters are provided
    if (argc < 3) {
        print_usage(argv[0]);
    }

    printf("Generating %d frames using %d process(es)...\n", frames, processes);

    //manage child processes
    int active_processes = 0;
    for (int i = 0; i < frames; i++) {
        if (active_processes >= processes) {
            wait(NULL);  //wait for child process to finish
            active_processes--;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            //pid == 0 : child process
            char frame_name[50];
            char scale[20];
            double scale_value = 1.0 / (1.0 + (i * 0.1));  //zoom progression

            snprintf(frame_name, sizeof(frame_name), "frame%d.jpg", i);
            snprintf(scale, sizeof(scale), "%f", scale_value);

            //execlp to run the mandel program
            execlp("./mandel", "./mandel", "-x", "-0.4", "-y", "-0.60", "-W", "800",
                   "-H", "800", "-s", scale, "-o", frame_name, (char *)NULL);

            //if execlp fails
            perror("execlp");
            exit(EXIT_FAILURE);
        } else {
            //anything other than 0 is parent process
            active_processes++;
        }
    }

    //wait for child processes to finish
    while (active_processes > 0) {
        wait(NULL);
        active_processes--;
    }

    printf("Frames generated successfully.\n");
    return 0;
}

void print_usage(const char *name) {
    fprintf(stderr, "Usage: %s -p <num_processes> -f <num_frames>\n", name);
    exit(EXIT_FAILURE);
}
