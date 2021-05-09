MAP THIS! GPS MAP VIEWER FOR PSP
--------------------------------

DISCLAIMER
----------
This software is distributed under GPL license and provided for education purposes only.
It is not fully tested and therefore should be used with caution.
I shell not be responsible for any damages and/or legal problems which may arise from using this 
application & underlying hardware setup.
This software acquires image data from Google Maps online service. Please read and understand Google Maps' "Terms of Use"
agreement before using this application.

NEW IN v0.5.20
- Generic serial GPS receivers support (Holux gpslim236-240, M1000-1200 and others) for slim psp
- One version of the program (FW3.XX) supports both PSP-290 and Holux receivers.
  Type of receiver can now be selected via configuration screen: 
  ENABLESERIALPORT=1 - Holux / serial port gps devices
  ENABLESERIALPORT=0 - PSP-290 support.
  Note that due to FW limitations the version for 1.0-1.5 firmware does not have support for PSP-290 receiver.
- WIFI upload should now work in all versions of the program. (must be enabled with LOADWIFI=1 flag)
- Variable map scroll speed (controlled via CURSORSPEED=..)
- Zoom in/out animation (toggled with SMOOTHZOOM=1)
- Smooth transition between 2D and bird-view modes
- Sound output controlled programatically in FW3.xx version. (applies to serial port gps devices only)
  If during program starup the audio/serial connector is not plugged in, the sound is routed to internal speakers.
- Rotating POI icons, better rendering of compass arrows, etc
- Numerous rendering optiomizations (as of v05.05)
- Misc bug fixes
- Code cleanup
- Source code included

NEW IN v0.5 (1 year anniversary edition:)
-----------
- Birds Eye mode
- Inverted colors (night) mode
- Voice Prompts / Alerts support. (* see notes below) 
- Missing zoom layer rendering (only for gpsfs maps). MapThis! attempts to render a missing zoom layer with shrinked
  tiles fromthe next zoom level.
- Screenshots are saved in ms0:/PSP/PHOTO now
- Icons/sounds can now be located in either a particular map directory or system (system/sounds or system/icons) directory.  
- POI and WAYPOINT files can now also be looked up from ms0:/PSP/COMMON
- Geodata can now be shared between maps if geodata.dat file is placed in the same directory as EBOOT.PBP.
  The program will attempt to load geodata.dat file in the current map directory first.
- New tiger line geodata for USA (2006se from 03/2007) compiled and available through gmdl18a and higher versions. 
- Edit functionality added to "Configuration" screen. While in Configuration screen, scroll down untill the line that needs to be
  edited is on top. Press [X] to enter the edit mode. You can use up/down arrows to increment/decrement current character by 1.
  Use [START] to save or [SELECT] to cancel the edit.
- New current position prediction logic, hopefully resulting in smoother scrolling.
- A few rendering bug fixes in graphics routines.
- New parameters added to configuration file: STARTUPFREQUENCY, STARTUPSCREENMODE, NIGHTMODE, SPEEDLIMIT.
  SATINFOFREQUENCY defines cpu frequency on Satellite Info screen (PSP-290 only)
- Timezone parameter removed from configuration, timezone is determined based on your psp date/time settings.
- Localization support (language packs).
- PSP-290 version does not require prx files to run in non-gps mode.
- Attached sorce code adopted to the latest verison of PSPSDK

KNOWN BUGS PROBLEMS
-----------------
- WIFI MAP UPLOAD does not work for PSP-290 version
- Infrequent crashes (still under investigation)



VOICE PROMPTS / ALERTS EXPLANATION
---------------------------------
Starting with MapThis! 0.5 it is possible to assign sound alerts to any POI or waypoint.
For instance,  if you have a speed trap POI list for your area and you'd like to have a 
sound alert when approaching a speed trap you'll need to do following:
1) Save your alert sound as 128bps/44Khz mp3 file as system/messageXXX.mp3 where XXX is any 3 digit number.
2) Edit your speed trap POI file to add that message number to the end of every poi definition line
40.666758,-73.992049,SPEED CAMERA,CAMERA1,XXX
40.666901,-73.988001,SPEED TRAP,TROOPER,XXX
........

The latest gmdl tool has the ability to match waypoint descriptions to a predefined dictionary of voice 
prompts. For example if it detects a word "left" in the description it can assign "Left turn ahead" 
pre-recorded message. The dictionary in gmdl is configurable via sounds.txt file, in which certain words
or phrases are linked to mp3 messages. MapThis! allows up to 1000 different messages per route or POI file.



INSTALLATION
------------

To get gps functionality working:
generic gps receiver version must be ran under 1.5 kernel (install in GAME150 folder under custom FW)
PSP-290 gps receiver version must be ran under 3.XX kernel (install in GAME3XX folder under custom FW)
PSP-290 version requires usbacc.prx and usbgps.prx files for GPS functionality to be working properly.
These files must be extracted from your firmware version and placed in the same directory as EBOOT.PBP.
Visit MapThis! FAQ threads in DCEMU forums (http://www.dcemu.co.uk/vbulletin/forumdisplay.php?f=170) for details.




NEW IN v0.4
-----------
- GPSFS format support. Map tiles are stored in a custom format for much faster transfer and access rate.
  Zip support for maps is discontinued (since it was too slow for large maps)
- Added logic for low baud GPS devices (by Zero Altitude). To activate put SLOW_GPS=1 in config.txt
- Straight and path distance calculation in map mode
- Ability to add POI. ([X] in Map Mode)
- POI search / display. ([O] from Attraction selector screen). List the names of up to 100 closest 
  points of interest, which match your search criteria together with distances from current location
- Variable read-speed for "GPS from file" mode (FAKEFEED=1). While re-playing your pre-recorded GPS data,
  press [X] to toggle re-play speed between x1,x3,x5,x7 speeds
- Last position recall. The application memorizes the latest position and zoom settings for each map.
- Improved map rotation logic for TRACK-UP mode. Use TURNSPEED variable in config.txt to specify the minimum
  speed at which screen rotation takes place. This allows to reduce screen rotation due to deviation in 
  GPS coordinates while the device is not moving or moving slowly.  
- Google maps version updated to most recent in WIFI download mode.
- "No UMD" bug is fixed. GPS read functionality does not depend on UMD in drive, wifi switch or 
  hold button position.
- STARTUPMAP=... option in config.txt allows to go directly to a specified map and enables the GPS mode.
- Improved and hopefully mode informative help screen ([START] button)
- New (faster) garbage collection logic (old logic used to crash application on large maps)
- More accurate distance calculation in GPS mode.
- Code cleanup (source code attached)
- New version of Windows Map Download Tool by in7ane (GPSFS conversion/support, new map vendors and much more)



NEW IN v0.3
-----------
- Multiple POI icons support - a few types of POI can now be merged in one file for simultaneous display.
- POI files: removed icon size definition (the size is read from the image itself, this should be backwards compatible with prev definition though..).
- speed up in menu responses
- some new parameters in config file:
###########################################
# THIS VARIABLE IS INTRODUCED TO SMOOTH # 
# THE MAP MOVEMENT BETWEEN GPS UPDATES #
# IE THE MAP WILL CONTINUE MOVEMENT IN #
# THE SAME DIRECTION WITH A SLIGHT # 
# DECREASE OF SPEED FOR EACH NEXT #
# FRAME RENDERING # 
###########################################
SPEEDFACTOR=0.97
##########################################
## LOAD WIFI MODULE ##
##########################################
LOADWIFI=0

- NMEA buffer increased to 1024 - this may help some GPS devices
- map size bug fixed. Now it should support maps up to 1024x1024
- Basic trip computer in gps mode. Display of elapsed time, distance and average speed.
- Set mark. In gps mode, displays distance & direction to mark.
- Scrollable menus in map/poi selectors: ability to display/scroll through up to 100 items.
- Numerous improvements in gmdl tool by in7ane: new map/poi sources added, POI converter, custom map import,
  routing tool, etc.
- Ability to disable top info bar and scale for maps with custom scales and projections: just remove coords.txt


NEW IN v0.2
-----------
- compass bug fix
- CPU clock speed toggle between 222 & 333Mhz
- battery life display
- improved POI support (displays up to 400 closest POI on screen / works in "track up" mode too now) 
- basic waypoint support
- new configuration paramaters (READTIME, WARNINGDISTANCE, etc)
  People with slow baud GPS devices should try to adjust NMEA read timing (READTIME >1)
- some graphics tweaks and new sample map.


MAIN FEATURES
-------------
- Scroll through large (up to 65536x65536 pixels) maps.
- Zoom in/out. The number of zoom levels depends of on the size of the map.
- Large coverage. The program uses imagery from Google Maps, which currently have pretty good coverage of
  North America, Western Europe, Australia, Japan. Visit Google maps, to find out if they provide required 
  coverage for your location.
- Customizable map size, level of detail and coverage.
- Ability to display zipped maps (currently buggy)
- Ability to display satellite and hybrid maps from Google.
- WIFI map retrieval: ability to acquire and store map data to memory stick over WIFI
- Linux/Cygwin script is also provided to generate a map of given size and detail for a given location.
- GPS support: ability to read and interpret NMEA sentences from a GPS receiver communicating through PSP's serial port.
  Rather simple DYI hook up with GPSlim 236 receiver from Holux is explained in following thread:
  http://www.dcemu.co.uk/vbulletin/showthread.php?t=30035
  In GPS mode there are following features:
     - Speed, direction, altitude, latitude, longitude, number of satellites in view.
     - NORTH UP mode: map is displayed with NORTH always on top. The arrow in the middle turns to shows current direction.
     - TRACK UP mode: map rotates so that direction of movement is always from top down.
     - Record GPS data (captures NMEA sentences in gps.txt file for later replay)
- POI/Attractions data support. Ability to select and display different types of attractions.
- Basic track/waypoint support. Ability to import/display directions instructions.
- PSPSDK compatible source code provided.
  
INSTALLATION
------------

1.5  users: copy contents of 1.5 folder to ms:/PSP/GAME
1.0  use EBOOT.PBP from 1.0 folder with data files from 1.5 folder
2.0+ try using EBOOT.PBP from NOGPS folder with data files from 1.5 folder
2.7+ generate maps of up to 32x32 size with provided cygwin script or "gmdl" windows program and html files, 
     copy them on MS and view through the PSP's browser.

CONTROLS
--------

MENUS:

[ANALOG PAD]                    MOVE UP/DOWN; SELECT LETTER ON DANZEFF KEYBOARD
[UP/DOWN]                       MOVE UP/DOWN
[LEFT/RIGHT]                    MOVE CURSOR IN TEXT ENTRY MODE/ TOGGLE CPU CLOCK SPEED WHILE IN MENUS
[X]                             CONFIRM SELECTION
[SELECT]                        EXIT
[START]                         DISPLAY HELP SCREEN / START MAP DOWNLOAD ON WIFI SCREEN
[L-TRIGGER]                     TOGGLE DIGITS IN TEXT ENTRY MODE
[R-TRIGGER]                     TOGGLE UPPER CASE IN TEXT ENTRY MODE

MAPS:
 
[ANALOG PAD]                    MOVE CURSOR; TURNS OFF GPS MODES
[UP/DOWN]                       ZOOM IN/OUT
[LEFT]                          TOGGLE GPS DATA RECORDING (ONLY IN GPS MODE)
[RIGHT]                         TOGGLE NORTH UP/TRACK UP MODES  (ONLY IN GPS MODE)
[SQUARE]                        TURN ON GPS MODE (THE GPS MODE TURNS OFF WHEN YOU MOVE THE CURSOR OR EXIT THE MAP)
[CIRCLE]                        DISPLAY POI/ATTRACTIONS MENU
[TRIANGLE]                      TURN ON/OFF POI DISPLAY (TURNS OFF IF ZOOM LEVEL IS CHANGED)
[START]				  DISPLAY HELP SCREEN
[SELECT]                        EXIT THE MAP



DETAILS
-------

GETTING THE MAPS:

The easiest way to get the maps, POIs and routing data is via "GMDL" program, created by in7ane.
Unpack the attached gmdlXX.zip file and navigate to .bin/debug subfolder.
You'll need to install Microsoft .net 2 from http://www.microsoft.com/downloads/ (~22Mb)


Getting maps using WIFI:
First go to Google maps and check if they cover the desired area @ at resolution that you want.
You'll need following information in order to get the maps for your area:
Latitude/Longitude of some location inside the desired map - these must be in decimal format.
Base zoom: 0-15; 0 being the most detailed. 
Size of a side of the map in 256pixel chunks. The map size must be equals to powers of 2, ie 4,8,16,32,64,128,256...
You map will always be square, so for example size=16 will generate 4096x4096 map at the base zoom with 4 zoom in bars.

To get your lat/lon coordinates, select _GLOBAL_MAP.zip in the main menu. Zoom in and point the
 cursor at your location on the map. The coordinates will be displayed on top. Alternatively, you
 can plug in your GPS receiver and turn on the GPS mode to find out your precise location.

NOTE: WIFI SWITCH MUST BE OFF IN ORDER FOR GPS COMMUNICATION TO WORK.

Now if you have WIFI, exit the WORLD map with [SELECT] and go to === WIFI MAP UPLOAD === option
 on the main menu. The WIFI entry form should "remember" your last position.
While choosing the size of your map, keep in mind that it may take significant amount of time to download and
 a lot of space on your memory stick.

Here is some numbers for reference:

Size 	# of 256x256pixel tiles				size on MS
------------------------------------------------------------------
16			340				~3   MB
32			1364				~5   MB
64			5460				~30  MB
128			21844				~170 MB
256			87380				~550 MB
	
You can manually zip up the small maps to save some space, but zipping maps over size 16 will slowdown the mapviewer performance.

MAKE SURE THAT WIFI SWITCH IS ON & ALL POWER SAVE SETTINGS ARE DISABLED IN PSP SYSTEM MENU BEFORE YOUR PRESS [START] 
TO START THE DOWNLOAD.

Currently, there is no recovery in case something happened during WIFI download. So if you have a weak signal, 
some map tiles may be missing or downloaded incorrectly. So if your new map does not work and/or hangs the viewer, 
try re-downloading it again or using the provided cygwin script or GMDL program to generate it.


As said above, you can use the getmaps.sh script in scripts directory to "generate" the maps.
To use it, you'll need to install cygwin (www.cygwin.com) - freeware linux-like environment for PC.
You can try to run it under linux too...
Make sure you have following commands installed: wget, bc
Running ./getmap.sh without arguments will print out the usage options.

Once getmaps.sh is finished you should see the new folder: _MAP_FOR......
Copy it to the maps directory on your PSP and your should be able to see in the viewer.

NOTE: all map folders and zips must start with '_' character.

Alternatively, gethybrid.sh script will generate the "hybrid" maps. In order to run it, you must install
JRE, ImageMagic and Netpbm library (all freeware). I have also put in a few sample scripts to show how to "fish" POI data from different online vendors.
Perhaps other people can enhance those and create new ones..

TESTING MAPS
------------
./testmap.sh [map size] {jpg} - creates html files for viewing/testing maps in the browser.
Just drop the files in the folder for the map that you want to test and view them trough the browser.
This technique can be used for displaying maps through PSP's browser for FW2.6+ users.
I also provided pre-made html files for regular 8x8, 16x16, 32x32 & 64x64 maps in scripts/html4testing folder  

CONFIGURATION
-------------

You can tweak some options and parameters in config.txt in system folder.
The comments explain different options...

POI/ATTRACTIONS/WAYPOINTS
---------------

Each attraction type is defined in a separate file inside the applicable map folder.
The file must starts with '_' character in order to be read by the selector menu.

Data format:
[lat],[lon],[description1 37 characters long],[description2 60 char long],

You can specify a png image to be displayed as an POI icon.
The image must consist of 2 frames, lined up horizontally "cursor on display" & "cursor off display"

Format:
!IMAGE:[image name (must be png file)],[image width][image height]

You can declare the list as a collection of waypoints by adding following line:
!WAYPOINT
The waypoints are displayed in bigger font with an alert icon next to them. 
(Useful for display of directions/road hazards/speed traps/etc)

Below is an example of a Gas station file:
#
!IMAGE:gas_icon.png,40,20,
#
41.003630,-74.519120,TJ Gulf; Oak Ridge, NJ,
40.596870,-74.365130,Texeco; Edison, NJ,
39.795585,-75.040591,Super Wawa GasStation; Blackwood, NJ,
...................................

Only 400 closest POIs/WAYPOINTS are displayed on screen, so you ,may need to refresh the display ocasionally.


KNOWN BUGS, PROBLEMS
--------------------
- Hybrid jpg maps sometimes don't load in unzipped state. If this happens, try zipping them.
- zip implementation has a memory leak, which may hang the program after a few loads of zipped maps.
- WIFI part has no retry/correction logic for dropped requests. It is advised to use provided scripts instead of it.

Enjoy,

DENIS
