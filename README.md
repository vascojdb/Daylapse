# Daylapse
A Raspberry Pi timelapse creator for very long timelapse projects.  
Based on Jon Bennett's Daylapse project available [here](https://github.com/jondbennett/Daylapse). Kudos for him to put this project up and running!  
An example timelapse movie from Jon Bennett can be watched [here on YouTube](https://www.youtube.com/watch?v=xY_Os_A_1po).

## Why should you use it?
**The scenario:**  
Imagine this: Your friend asks you to make a 1 year timelapse of his/her house being build. You think it is an awesome idea so you agree and go straight to work, but before you can scream with joy you find some problems on the way...

**The problems:**
 - A regular timelapse does not take into consideration the **day and night cycles** recording both during the day and the night, and you do not want a timelapse that becomes stroboscopic when showing 1 year in 1 minute, right?
 - You do not want to **manually delete all the dark/night frames**, that's out of the question.
 - And even if you do, what will happen during summer and winter? Will your timelapse video be **slower in summer and faster in winter**? *(summer days are longer than winter ones)*, well that does not sound right...  

**Solution:**
The solution is not that complicated, bare with me and keep reading :)

## How does it work?
The way Daylapse works is that it takes from you the following critical information:
 - Your location *(latitude and longitude)*
 - The desired number of FPS of your timelapse video *(usually 25, 30 or 60fps)*
 - The duration you want each day to have on the video *(for example 4 seconds)*

Daylapse then calculates the proper number of photos to take during that day, which will be evenly distributed throughout the day, between sunrise and sunset.  
The time between photos is longer in the summer when days are longer, and it decreases with the shorter days of winter. 

Daylapse needs to be started every day at an early hour *(for example 3:00AM)*, then it will wait until the sunrise so it can start taking photos.  
During this time you can optionally use a GPIO to control the camera LED, which in some cameras is connected to an onboard IR-cut filter.  
After taking all of the photos for a given day the program exits.  
In order to lean how to setup autostart every day, see the relative section below.

The tool uses `raspistill` to take the photos, so be sure you have a Raspberry Pi camera connected and enabled via `raspi-config`.  
You will need the WiringPi library from [here](http://wiringpi.com/) to control the camera LED, which sometimes is connectd to an IR-cut filter. By using this feature, the LED or IR-Filter stays active from sunrise till sunset. Remember to disable the automatic camera LED control on your Raspberry Pi `/boot/config.txt` file with `disable_camera_led=1`.

*NOTE: You need to have **proper time and timezone set** on the Raspberry Pi, either through NTP via internet or via an external RTC for the sunrise and sunset calculations to work.*

## Installation
First you will need to update your system and get get and wiringpi:
```
sudo apt update
sudo apt upgrade
sudo apt install git wiringpi
```

Then clone the repository to your Raspberry Pi (under your home directory for example):
```
cd ~
git clone https://github.com/vascojdb/Daylapse.git
```

Now you can compile the code:
```
cd Daylapse
make
```

You can now install the application:
```
sudo make install
# You can also uninstall it later using "sudo make uninstall"
```

NOTE: You can also clean the compiled files with `make clean` or uninstall the application using `sudo make uninstall`

## Usage
After the application is installed you can call it with the `-h` option to see the help:
```
$ daylapse -h
daylapse: Takes constant day duration pictures for
          creating long timelapse videos.
          Should be run once a day before sunrise.
Usage: daylapse [options]
Where options are:
  -f <fps>      Desired FPS on the final video
                > Default: 25
  -d/-s <s/day> Day duration in seconds/day in the
                video at the desired FPS. Used to
                calculate the interval between
                pictures taken.
                > Default: 2.00
  -y <lat>      Your latitude to calculate sunrise
                and sunset times.
                > Default: 50.80
  -x <lon>      Your longitude to calculate sunrise
                and sunset times.
                > Default is 19.80
  -o <dir>      Directory to save the pictures to.
                > Default: current directory
  -O "<opts>"   Use these options in raspistill.
                Do not use -t and -o options here.
                > Default: "-n -w 1920 -h 1080 -q 90"
  -p <gpio>     GPIO pin number to turn ON the
                camera LED or enable IR-cut filter.
                Turns ON at sunrise, OFF at sunset.
                -GPIO control disabled:    -1
                -Raspberry Pi Model A/B:   5
                -Raspberry Pi Model B+:    32
                -Raspberry Pi Zero/Zero W: 40
                GPIO control does not work on Rpi 3
                > Default: -1
  -D            Dry run, does everything but does
                not take any pictures
  -S            Skip checking the sunrise time to
                start taking pictures immediately
  -h            Shows this help
If no options are provided. Defaults will be used.
Photos are saved as: YYYY_MM_DD_HH_MM_SS_mmm.jpg
Y=Year, M=Month, D=Day, H=Hour, M=Min, S=Sec, m=Milis
```

## Examples
### Example 1
Take pictures from the Eiffel Tower in Paris, to make a timelapse video at 60fps, where each day will be 4 seconds long on the video. Save the pictures on the current directory:
```
$ daylapse -f 60 -d 4 -y 48.85 -x 2.29
=========== Starting daylapse ===========
Video FPS:      60 fps
1 Day on video: 4.00 seconds at 60 fps
Latitude:       48.85
Longitude:      2.29
=========== Calculated values ===========
Local Sunrise:  Thu Sep  5 07:12:00 2019
Local Sunset:   Thu Sep  5 20:26:00 2019
Day length:     13.22h
Photos/day:     240 photos
Photo interval: 198 seconds
=========== Starting captures ==========
Taking photo 1 of 240
(...)
```

### Example 2
Take 640x480 night pictures from the Statue of Liberty in NYC, to make a timelapse video at 25fps where each day will be 10 seconds long in the video. Save the pictures in /media/USBSTICK/images/. Use Raspberry Pi Zero with a Pi camera with a GPIO controlled IR-cut filter on the LED pin:
```
$ daylapse -d 10 -y 40.68 -x -74.04 -o /media/USBSTICK/images/ -O "-w 640 -h 480 -ex night" -p 40
```

### Example 3
Just run the application from Krakow in Poland and test the camera by taking 640x480 night pictures on a Raspberry Pi Zero W with a Pi camera with IR-cut filter assigned to the camera LED pin. Force skipping the sunrise check as well.
```
$ daylapse -y 50.06 -x 19.94 -O "-w 640 -h 480 -ex night" -p 40 -S
```

### Example 4
Just run the application from Krakow in Poland without taking pictures. Force skipping the sunrise check as well.
```
$ daylapse -y 50.06 -x 19.94 -D -S
```

## Setting up to run every day
The best and easiest way to use Daylapse is to **setup a daily cron job** for some very early time in the morning *(like 3:00 AM)* that calls the application with your defined parameters. Here is an example from my instance:  
Open crontab for the user *pi* using:
```
crontab -e
```
Add the following line in the end of your crontab file to trigger the job every day at 03:00AM:
```
0 3 * * * /usr/local/bin/daylapse -f 30 -d 2 -o /home/pi/daylapse -p 40 -O "-n -ex auto -drc high -w 2592 -h 1944 -q 90" >> /home/pi/daylapse.log 2>&1
```

**NOTE:** The part with `>> /home/pi/daylapse.log 2>&1` is used to keep a log file of the application and it is optional.

It might make sense to connect to the Raspberry Pi and **download the images from time to time** in order to keep the SD card from filling up.  
In my case my pictures had about 3.5MB each *(at native 5MPix)*, with a rate of 60 pictures/day *(30fps * 2s)* a month of pictures will occupy about 6.3GB.

## Post processing:
After you have some or all the photos taken, you need to post-process them. To do this the steps are:
1. **Extract the photos from the Raspberry Pi into a folder on a Linux machine.**  
   If you do not have one, you may set up a virtual machine using VirtualBox and Debian, Lubuntu, Ubuntu or any other Linux distribution. You can extract the files directly via SD card or remotely via SCP-SSH, WinSCP or any other method.  
   Lets assume the folder where you place the photos is called `daylapse`.

2. **Place the scripts on the folder.**  
   Copy the scripts inside `Tools` folder on this repository to the `daylapse` folder, where the photos are located.

3. **Run a deflicker tool to average the brighness of the pictures.**  
   This will average the picture brightness to make sure there are no hard transitions on the final video. I am using Vangelis [timelapse-deflicker](https://github.com/cyberang3l/timelapse-deflicker/) Perl script.  
   Run the Perl script as follows, note that this may take some time:
   ```
   cd daylapse
   ./timelapse-deflicker.pl -w 200 -p 2
   ```
   
4. Renumber the photos from their original name to a more fliendly timelapse format (00000.jpg to 99999.jpg) using the script renumberJPG with the folder containing the files to rename as parameter:
   ```
   ./renumberJPG Deflickered
   ```

5. Open the photos on a batch editing software *(for example Adobe [Lighroom](https://lightroom.adobe.com/) or [DarkTable](http://www.darktable.org/))* and edit the pictures as you please.

6. Use a video editor to generate the timelapse *(for example [OpenShot](https://www.openshot.org/), [Adobe Premiere](https://www.adobe.com/products/premiere.html) or any other that will allow creation of timelapse from a photo sequence)*
