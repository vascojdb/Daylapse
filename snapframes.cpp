/*
 * File:
 *      snapframes.cpp
 *
 * Description:
 *      C library that takes N pictures with raspistill
 *      with T interval in between each picture
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <sys/time.h>

/******************************************************/
/* Returns current time in seconds + fractional second*/
/******************************************************/
double currentTimeMs() {
    struct timeval tp;
    double seconds = 0.;
    double milliseconds = 0.;

	gettimeofday(&tp, 0);
	seconds = (double)tp.tv_sec;
	milliseconds = (double)(tp.tv_usec / 1000);

	return (seconds + (milliseconds / 1000));
}

/******************************************************/
/* Starts taking _frameCount pictures with _frameDelay*/
/* intervals in between them. This function will call */
/* raspistill to take the pictures                    */
/* If dryrun is true, no photos will be taken         */
/******************************************************/
void snapFrames(unsigned int _frameCount, double _frameDelay, bool _dryrun, char *_outputdir, char *_raspistill_opt) {
	// Sanity check
	if(_frameDelay < 0) return;
    if(_frameCount == 0) return;

	// Set up the first frame time
	double frame_time = currentTimeMs();
	double next_frame_time = 0.;
	char snapCommand[PATH_MAX + FILENAME_MAX];

	// Main loop
	for (unsigned int frame = 0; frame < _frameCount; frame++) {
		// Figure timing for the next frame
		next_frame_time = frame_time + _frameDelay;

		// Figure latency
		int latency = (int)((_frameDelay * 1000) - 500);
		if(latency < 1000) latency = 1000;
		if(latency > 5000) latency = 5000;

		// Now work up the file name in GMT to avoid daylight saving time issues
		time_t frame_time_int = (int)frame_time;
		int frame_time_decimal = (int)((frame_time - frame_time_int) * 1000);
		struct tm* gmt = gmtime(&frame_time_int);

		// Create snapshot command
		sprintf(snapCommand,"raspistill %s -t %d -o %s/%04d_%02d_%02d_%02d_%02d_%02d_%03d.jpg",
				_raspistill_opt,
                latency,
                _outputdir,
                gmt->tm_year + 1900,gmt->tm_mon + 1,gmt->tm_mday,
                gmt->tm_hour,gmt->tm_min,gmt->tm_sec, frame_time_decimal);

		{
			time_t now = time(0);
            printf("Taking photo %u of %u at %s", (frame+1), _frameCount, asctime(localtime(&now)));
            //printf("Command: %s\n", snapCommand);
			fflush(stdout);
		}

		// Execute snapshot command
		if (_dryrun) printf("Dry run: No photo was taken.\n");
        else {
            // Kill any running instance of raspistill that may be running:
            char snapKillCommand[50];
            strcpy(snapKillCommand, "killall raspistill >>/dev/null 2>>/dev/null");
            system(snapKillCommand);
            system(snapCommand);
        }

		// Use sleep if we have more than a second to wait
		int delay;
		delay = (int)(next_frame_time - frame_time);
		if(delay > 1)
			sleep(delay - 1);

		// Less than a second to go, so start a tight
		// loop waiting for the time to change
		while(currentTimeMs() < next_frame_time);
        frame_time = next_frame_time;
	}
}
