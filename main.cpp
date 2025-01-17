/*
 * File:
 *      main.c
 *
 * Description:
 *      Takes constant day duration pictures for creating
 *      long timelapse videos.
 *      Should be run once a day before sunrise.
 *      User can specify as parameters the prefered FPS of
 *      the movie that will be created, as well as the 
 *      total duration of the day in seconds (in the movie).
 *      User should also specify the latitude and longitude.
 */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <wiringPi.h>

// Uses Paul Schlyter's excellent sunriseset code:
#include "sunriset.h"
#include "snapframes.h"

//#define DEBUG

/******************************************************/
/* Converts a broken down time into a calendar time   */
/* which is represented by the seconds since Epoch in */
/* a time_t format                                    */
/******************************************************/
time_t convertToTimeT(int _year, int _month, int _day, double _rsTime) {
	struct tm gmt;

	// Convert the double to hours and minutes
	int hour = (int)_rsTime;
	int minute = (int)((_rsTime - hour) * 60);

	// Fill in the gmt struct
	memset(&gmt, 0, sizeof(gmt));
	gmt.tm_year = _year - 1900;
	gmt.tm_mon = _month - 1;
	gmt.tm_mday = _day;

	gmt.tm_hour = hour;
	gmt.tm_min = minute;

	return timegm(&gmt);
}

/******************************************************/
/* Waits until a sunrise has occured, then returns    */
/* true. In case we are already after the sunrise     */
/* then it returns false immediatly (unless in DEBUG) */
/******************************************************/
bool waitUntilSunrise(time_t _sunrise) {
	time_t now = time(0);
	time_t sleepSeconds;

    // Check if we are after the sunrise for today:
	if (_sunrise < now) {
#ifdef DEBUG
		printf("DEBUG: Skipping waiting for sunrise\n");
		return true;
#else
		return false;
#endif
	}

	printf("Waiting until sunrise...\n");
	printf("Current time:   %s",asctime(localtime(&now)));
	printf("Local sunrise:  %s",asctime(localtime(&_sunrise)));
	fflush(stdout);

	// I do it this way because a single sleep for the time between
	// "now" and "sunrise" could be on the order of several thousand seconds
	// and the sleep timer has proven to be inaccurate at that range.
	do {
		now = time(0);
		sleepSeconds = _sunrise - now;
		if(sleepSeconds > 60)
			sleepSeconds = 60;
		if(sleepSeconds < 0)
			sleepSeconds = 0;
        // Keep on sleeping if needed
		if(sleepSeconds > 0)
			sleep(sleepSeconds);
	}
	while (sleepSeconds > 0);

	printf("Waiting for sunrise completed at %s", asctime(localtime(&now)));
	fflush(stdout);

	return true;
}


/******************************************************/
/* Starts taking pictures on calculated intervals     */
/* based of the FPS, seconcds/day and day length      */
/* throughout the day and then returns                */
/* If dryrun is true, no photos will be taken         */
/* Output dir sets the output directory for the photos*/
/* Raspistill_opt set additional options to raspistill*/
/* cam_led_gpio is the camera LED pin or -1 (not used)*/
/******************************************************/
void snapDayFrames(double _fps, double _sday, double _dayLen,
                   bool _dryrun, char *_outputdir, char *_raspistill_opt,
                   int _cam_led_gpio) {
	// Calculate the number of photos for this day
	int dayFrames = round(_fps * _sday);

	// Calculate delay between photos
	double frameDelay = (_dayLen * 60. * 60.) / dayFrames;

    // Print calculated values
    printf("Photos/day:     %i photos\n", dayFrames);
    printf("Photo interval: %i seconds\n", int(frameDelay));

	// And snap the frames
    printf("=========== Starting captures ==========\n");
	fflush(stdout);
	snapFrames(dayFrames, frameDelay, _dryrun, _outputdir,
               _raspistill_opt, _cam_led_gpio);
}

/******************************************************/
/* This is the main function that is executed when    */
/* the binary is executed.                            */
/* The binary should be executed before sunrise, then */
/* it will wait for the sunrise and call the function */
/* snapDayFrames to take the pictures according to    */
/* the calculated intervals during that day.          */
/* Arguments use the defaut values, unless specified  */
/* on the terminal. See -h option for more details    */
/******************************************************/
int main(int argc, char *argv[]) {
    int opt;
    bool dryrun = false;
    bool skipSunriseCheck = false;
    
    // Default options
    double lat = 50.80;
	double lon = 19.80;
    double fps = 25.0;
    double sday = 2.0;
    char outputdir[PATH_MAX] = "./";
    char raspistill_opt[MAX_INPUT] = "-n -w 1920 -h 1080 -q 90";
    int cam_led_gpio = -1;
    
    // Loop through all arguments:
    while ((opt = getopt(argc, argv, "f:d:s:y:x:o:O:p:DSh")) != -1) {
        switch (opt) {
            case 'f':
                fps = strtod(optarg, NULL);
                break;
            case 'd':
            case 's':
                sday = strtod(optarg, NULL);
                break;
            case 'x':
                lon = strtod(optarg, NULL);
                break;
            case 'y':
                lat = strtod(optarg, NULL);
                break;
            case 'o':
                snprintf(outputdir, PATH_MAX-1, "%s", optarg);
                break;
            case 'O':
                snprintf(raspistill_opt, MAX_INPUT-1, "%s", optarg);
                break;
            case 'p':
                cam_led_gpio = int(strtod(optarg, NULL));
                break;
            case 'h':
                printf("daylapse: Takes constant day duration pictures for\n");
                printf("          creating long timelapse videos.\n");
                printf("          Should be run once a day before sunrise.\n");
                printf("Usage: daylapse [options]\n");
                printf("Where options are:\n");
                printf("  -f <fps>      Desired FPS on the final video\n");
                printf("                > Default: %.0f\n", fps);
                printf("  -d/-s <s/day> Day duration in seconds/day in the\n");
                printf("                video at the desired FPS. Used to\n");
                printf("                calculate the interval between\n");
                printf("                pictures taken.\n");
                printf("                > Default: %.2f\n", sday);
                printf("  -y <lat>      Your latitude to calculate sunrise\n");
                printf("                and sunset times.\n");
                printf("                > Default: %.2f\n", lat);
                printf("  -x <lon>      Your longitude to calculate sunrise\n");
                printf("                and sunset times.\n");
                printf("                > Default is %.2f\n", lon);
                printf("  -o <dir>      Directory to save the pictures to.\n");
                printf("                > Default: current directory\n");
                printf("  -O \"<opts>\"   Use these options in raspistill.\n");
                printf("                Do not use -t and -o options here.\n");
                printf("                > Default: \"%s\"\n", raspistill_opt);
                printf("  -p <gpio>     GPIO pin number to turn ON the\n");
                printf("                camera LED or enable IR-cut filter.\n");
                printf("                Turns ON at sunrise, OFF at sunset.\n");
                printf("                -GPIO control disabled:    -1\n");
                printf("                -Raspberry Pi Model A/B:   5\n");
                printf("                -Raspberry Pi Model B+:    32\n");
                printf("                -Raspberry Pi Zero/Zero W: 40\n");
                printf("                GPIO control does not work on Rpi 3\n");
                printf("                > Default: %i\n", cam_led_gpio);
                printf("  -D            Dry run, does everything but does\n");
                printf("                not take any pictures\n");
                printf("  -S            Skip checking the sunrise time to\n");
                printf("                start taking pictures immediately\n");
                printf("  -h            Shows this help\n");
                printf("If no options are provided. Defaults will be used.\n");
                printf("Photos are saved as: YYYY_MM_DD_HH_MM_SS_mmm\n");
                printf("Y=Year, M=Month, D=Day, H=Hour, M=Min, S=Sec, m=Milis\n");
                return 0;
                break;
            case 'D':
                dryrun = true;
                break;
            case 'S':
                skipSunriseCheck = true;
                break;
            case ':':
                fprintf(stderr, "Option provided needs a value\n");
                exit(1);
                break;
            case '?':
                fprintf(stderr, "Unknown option: %c. Try '-h' for more information.\n", optopt);
                exit(1);
                break;
        }
    }
    
    printf("=========== Starting daylapse ===========\n");
    fflush(stdout);
    
	// Current time
	time_t now = time(0);
	struct tm tmNow = *gmtime(&now);

	// Date
	int year = tmNow.tm_year + 1900;
	int month = tmNow.tm_mon + 1;
	int day = tmNow.tm_mday;

	// Output
	double rise, set;
	time_t riseTime;
	time_t setTime;

    // Compute rise and set times
	int rs = sun_rise_set(year, month, day, lon, lat, &rise, &set );
	riseTime = convertToTimeT(year, month, day, rise);
	setTime = convertToTimeT(year, month, day, set);

    // Compute the day length
	double daylen  = day_length(year,month,day,lon,lat);

    // Print some information about preferences
    printf("Video FPS:      %.0f fps\n", fps);
    printf("1 Day on video: %.2f seconds at %.0f fps\n", sday, fps);
    printf("Latitude:       %.2f\n", lat);
    printf("Longitude:      %.2f\n", lon);
    if (dryrun) printf("Dry run:        Enabled\n");
    if (skipSunriseCheck) printf("Sunrise check:  Disabled (skip)\n");
    printf("Output dir:     %s\n", outputdir);
    printf("Raspistill opt: %s\n", raspistill_opt);
    if (cam_led_gpio >= 0) printf("Camera GPIO:    %i\n", cam_led_gpio);
    fflush(stdout);

    // Take action based on the result of sun_rise_set
	switch (rs) {
        case 0:
            printf("=========== Calculated values ===========\n");
            printf("Local Sunrise:  %s", asctime(localtime(&riseTime)));
            printf("Local Sunset:   %s", asctime(localtime(&setTime)));
            printf("Day length:     %5.2fh\n", daylen);
            fflush(stdout);

            if (!skipSunriseCheck) {
                // Wait for sunrise today or exit if we missed it
                if(!waitUntilSunrise(riseTime)) {
                    printf("ERROR: Sunrise for today has already passed!\n");
                    exit(1);
                }
            }
            else printf("Skipping sunrise check!\n");

            snapDayFrames(fps, sday, daylen, dryrun, outputdir, raspistill_opt, cam_led_gpio);
            break;
        
        case +1:
            printf("ERROR: Sun is above the horizon, no sunset today!\n");
            break;
        
        case -1:
            printf("ERROR: Sun is below the horizon, no sunrise today!\n");
            break;
	}

	{
		time_t now = time(0);
		printf("Timelapse for today completed at %s", asctime(localtime(&now)));
		fflush(stdout);
	}
	return 0;
}





