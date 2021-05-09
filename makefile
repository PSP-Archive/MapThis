TARGET = mView
PSPSDK=$(shell psp-config --pspsdk-path)
PSPBIN = $(PSPSDK)/../bin

PSP_EBOOT_PIC1 = PIC1.PNG
PSP_EBOOT_ICON = ICON0.PNG


###################################################################
#PSP-290/USB versionSPECIFIC DEFINITIONS::uncomment the lines below
###################################################################
#PSP_FW_VERSION=371
#BUILD_PRX = 1
#CFLAGS = -O2  -G0 -Wall -g -DDANZEFF_SCEGU -DNDEBUG
##LDFLAGS = -mno-crt0 -nostartfiles
#LDFLAGS = -mno-crt0
#LIBS =  -lpspdebug  -lpsprtc -lpspgum -lpspgu  -lpsppower  -lpspusb -lpng -lz  -ljpeg -lm -lc -lpspwlan -lmad -lpspaudiolib -lpspaudio -g

###################################################################
#HOLUX GPSlim236+ version DEFINITIONS::uncomment the lines below
###################################################################
CFLAGS = -O2  -G0 -Wall -g -DDANZEFF_SCEGU -DNDEBUG -DGENERIC
LIBS =  -lpspdebug  -lpsphprm_driver  -lpsprtc   -lpspvfpu -lpspgum   -lpsppower   -lpng -lz  -ljpeg -lm -lpspwlan -lmad -lpspaudiolib -lpspaudio -lpspgu



OBJS =  main.o \
        graphics.o \
	font.o \
	utils.o \
	attractions.o \
	line.o \
	nmeap01.o \
	danzeff.o \
	geocalc.o \
	sceUsbGps.o \
	geodata.o \
	mp3player.o \
	basic.o \
	menu.o \
	sioprx.o \
	display.o



CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti 
ASFLAGS = $(CFLAGS)


EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = mView

include $(PSPSDK)/lib/build.mak
