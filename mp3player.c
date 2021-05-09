#include <pspkernel.h>
#include <pspthreadman.h>
#include <pspaudio.h>
#include <psppower.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "mad.h"
#include "utils.h"
#include "main.h"
#include "basic.h"
#include "mp3player.h"
volatile int MP3Thread_Active = 1;

char MP3_file[256]={0,0,0};
char MP3_file_2[256];
int MP3_newfile;
int MP3_pause;
int MP3_file_offset=0;
int MP3_currentOffset=-1;
int MP3_savedOffset=-1;
int mp3_thread=-1;
int MP3_FD=-1;
int MP3_presentlyPlaying=0;

int MP3_resident=1;
int MP3Thread_Started=0;
#define OUTPUT_BUFFER_SIZE	4096*4 /* Must be an integer multiple of 4. */
#define INPUT_BUFFER_SIZE	(2*OUTPUT_BUFFER_SIZE)

#define MAXVOLUME	0x8000
int mp3_buffer_size=OUTPUT_BUFFER_SIZE/4;
static int mp3_handle;

static unsigned char		InputBuffer[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD],
						OutputBuffer[2][OUTPUT_BUFFER_SIZE],
						*OutputPtr=OutputBuffer[0],
						*GuardPtr=NULL;
volatile int OutputBuffer_flip=0;
volatile static const unsigned char	*OutputBufferEnd=OutputBuffer[0]+OUTPUT_BUFFER_SIZE;
static struct mad_stream	Stream;
static struct mad_frame	Frame;
static struct mad_synth	Synth;
static mad_timer_t	Timer;

int	MusicBitRate;
int	MusicSampleRate;



enum {
  ID3_TAG_FLAG_UNSYNCHRONISATION     = 0x80,
  ID3_TAG_FLAG_EXTENDEDHEADER        = 0x40,
  ID3_TAG_FLAG_EXPERIMENTALINDICATOR = 0x20,
  ID3_TAG_FLAG_FOOTERPRESENT         = 0x10,

  ID3_TAG_FLAG_KNOWNFLAGS            = 0xf0
};

enum tagtype {
  TAGTYPE_NONE = 0,
  TAGTYPE_ID3V1,
  TAGTYPE_ID3V2,
  TAGTYPE_ID3V2_FOOTER
};

static
enum tagtype tagtype(unsigned char const *data, unsigned long length)
{
  if (length >= 3 &&
      data[0] == 'T' && data[1] == 'A' && data[2] == 'G')
    return TAGTYPE_ID3V1;

  if (length >= 10 &&
      ((data[0] == 'I' && data[1] == 'D' && data[2] == '3') ||
       (data[0] == '3' && data[1] == 'D' && data[2] == 'I')) &&
      data[3] < 0xff && data[4] < 0xff &&
      data[6] < 0x80 && data[7] < 0x80 && data[8] < 0x80 && data[9] < 0x80)
    return data[0] == 'I' ? TAGTYPE_ID3V2 : TAGTYPE_ID3V2_FOOTER;

  return TAGTYPE_NONE;
}

unsigned long id3_parse_uint(unsigned char const **ptr, unsigned int bytes)
{
unsigned long value = 0;

  switch (bytes) {
  case 4: value = (value << 8) | *(*ptr)++;
  case 3: value = (value << 8) | *(*ptr)++;
  case 2: value = (value << 8) | *(*ptr)++;
  case 1: value = (value << 8) | *(*ptr)++;
  }

  return value;
}

unsigned long id3_parse_syncsafe(unsigned char const **ptr, unsigned int bytes)
{
unsigned long value = 0;

  switch (bytes) {
  case 5: value = (value << 4) | (*(*ptr)++ & 0x0f);
  case 4: value = (value << 7) | (*(*ptr)++ & 0x7f);
          value = (value << 7) | (*(*ptr)++ & 0x7f);
	  value = (value << 7) | (*(*ptr)++ & 0x7f);
	  value = (value << 7) | (*(*ptr)++ & 0x7f);
  }

  return value;
}

static void parse_header(unsigned char const **ptr, unsigned int *version, int *flags, unsigned long *size)
{
  *ptr += 3;

  *version = id3_parse_uint(ptr, 2);
  *flags   = id3_parse_uint(ptr, 1);
  *size    = id3_parse_syncsafe(ptr, 4);
}

signed long id3_tag_query(unsigned char const *data, unsigned long length)
{
unsigned int version;
int flags;
unsigned long size;

	switch (tagtype(data, length)) {
  case TAGTYPE_ID3V1:
    return 128;

  case TAGTYPE_ID3V2:
    parse_header(&data, &version, &flags, &size);

    if (flags & ID3_TAG_FLAG_FOOTERPRESENT)
      size += 10;

    return 10 + size;

  case TAGTYPE_ID3V2_FOOTER:
    parse_header(&data, &version, &flags, &size);
    return -size - 10;

  case TAGTYPE_NONE:
    break;
  }

  return 0;
}




void MP3_OpenAudio(void)
{
	mp3_handle = sceAudioChReserve( 7, mp3_buffer_size, 0 );
	if (mp3_handle < 0)
		mp3_handle = sceAudioChReserve( PSP_AUDIO_NEXT_CHANNEL, mp3_buffer_size, 0 );
}

void MP3_CloseAudio(vpod)
{
	sceAudioChRelease( mp3_handle );
	mad_synth_finish(&Synth);
	mad_frame_finish(&Frame);
	mad_stream_finish(&Stream);
}


void MP3_Pause(int pause)
{
	if(mp3_thread==-1) return;
	MP3_pause=pause;
	if(pause)
		sceKernelSuspendThread(mp3_thread);
	else
		sceKernelResumeThread(mp3_thread);
}

typedef struct _playlist {
        char		 name[252];
        struct _playlist *next;
} PLAYLIST;
PLAYLIST *play_list=NULL;
PLAYLIST *current_entry=NULL;

void MP3_Play(char *name, int offset)
{
	strncpy(MP3_file, name,256);

	//ms_write_log("\nPLAY : '%s'  MP3_presentlyPlaying=%d",MP3_file,MP3_presentlyPlaying);

	MP3_newfile=1;
	MP3_playing=0;
	MP3_file_offset = offset;
	sceKernelWakeupThread(mp3_thread);
	MP3_Pause(0);
}


void MP3_PlaySnipet(char *name)		// This is used to interrupt whatever is playing, save the state and start the new one.
{
	if(MP3_presentlyPlaying!=2)		// Not a Snipet Playing, which means we need to save...
	{
		strncpy(MP3_file_2,MP3_file,256);	// Save presently played mp3
		MP3_savedOffset=MP3_currentOffset;	// and it's offset...
	}
	MP3_presentlyPlaying=2;
	MP3_Play(name,0);
}

// #define DEBUGGINGM3UPROCESS	1

void CheckSongStack(void)		// This gets called when a sample finishes playing
{
PLAYLIST	*i;

#ifdef DEBUGGINGM3UPROCESS
	ms_write_log("\nM3U : CheckSongStack called (MP3_presentlyPlaying=%d)  current_entry=%p  (%p)",MP3_presentlyPlaying,current_entry,play_list);
#endif

	if(MP3_presentlyPlaying==2)	// Snipet finished playing, let's play what was on the stack
	{
		if(MP3_savedOffset!=-1)
		{
			MP3_Play(MP3_file_2,MP3_savedOffset);
			MP3_presentlyPlaying=1;
		}
		else
		{
			MP3_presentlyPlaying=0;	// Nothing was playing...
			MP3_Pause(1);
		}
	}
	else
	{
		if((i=current_entry)!=NULL)	
		{
			if((i=current_entry->next)==NULL) i=play_list;
			current_entry=i;
		}
		else
		{
			i=play_list;
			current_entry=i;
		}
		if(i!=NULL)
		{
#ifdef DEBUGGINGM3UPROCESS
	ms_write_log("\nM3U : Calling Play with file '%s'",i->name);
#endif
			MP3_Play(i->name,0);
		}
		else
		{
#ifdef DEBUGGINGM3UPROCESS
	ms_write_log("\nM3U : End of List");
#endif
			MP3_Pause(1);
		}
	}
}

void	CleanUpMusicList(void)
{
PLAYLIST	*i=play_list,*n;

	while(i!=NULL)
	{
		n=i->next;
		free((void*)i);
		i=n;
	}
	play_list=NULL;
	current_entry=NULL;
}

void	AddToMusicList(char *name)
{
int		a=0;
PLAYLIST	*tmplst=NULL,*i;

	if((tmplst=(PLAYLIST*)malloc((sizeof(PLAYLIST))))==NULL)
	{
		ms_write_log("\nM3U : Failed to allocate structure for '%s'",name);
		return;
	}
	tmplst->name[0]='m';
	tmplst->name[1]='s';
	tmplst->name[2]='0';
	tmplst->name[3]=':';
	tmplst->name[4]='/';
	while((a<246)&&(name[a]!='\0'))
	{
		tmplst->name[5+a]=name[a];
		++a;
	}
	tmplst->name[5+a]='\0';
	tmplst->next=NULL;

#ifdef DEBUGGINGM3UPROCESS
	ms_write_log("\nM3U : AddToMusicList '%s'  ptr=%p",name,tmplst);
#endif

	i=play_list;
	if(i==NULL)
	{
		play_list=tmplst;
		return;
	}
	while(i->next!=NULL) i=i->next;
	i->next=tmplst;
}

void	MP3_ProcessM3U(char *FileName)
{
char	*SharedTmpBuffer=NULL;
int	fdin,filesize,bs,lb,be;
char	tmpc;

	CleanUpMusicList();
	if(FileName==NULL) return;

#ifdef DEBUGGINGM3UPROCESS
	ms_write_log("\nM3U : Opening m3u file : '%s'",FileName);
#endif

	if((fdin=sceIoOpen(FileName,PSP_O_RDONLY,0777))<0)
	{
		ms_write_log("\nM3U : Wasn't able to open m3u file : '%s'",FileName);
		return;							// Wasn't able to open m3u
	}
	filesize=sceIoLseek(fdin,0,SEEK_END);
	sceIoLseek(fdin,0,SEEK_SET);
	if((SharedTmpBuffer=(char*)malloc((size_t)(filesize+1)))==NULL)
	{
		ms_write_log("\nM3U : Wasn't able to allocate %d bytes for m3u file '%s'.",filesize,FileName);
		sceIoClose(fdin);
		return;							// Wasn't able to allocate m3u buffer
	}
	bs=sceIoRead(fdin,SharedTmpBuffer,filesize);
	sceIoClose(fdin);
	if(bs<filesize) filesize=bs;					// Make sure we're working with the smaller number...
	bs=0;				// EOL	is 0D,0A in DOS or 0A only in Linux
	while(bs<filesize)		// prep the buffer by replacing 'invalid' characters [0-31] and empty chars but preserve EOL
	{
		if((SharedTmpBuffer[bs]<' ')&&(SharedTmpBuffer[bs]!='\0')&&(SharedTmpBuffer[bs]!=0x0A)) SharedTmpBuffer[bs]='\0';
		if(SharedTmpBuffer[bs]=='\\') SharedTmpBuffer[bs]='/';	// from "I:\PSP\MUSIC\02 Requiem - II Kyrie.mp3"  to  "I:/PSP/MUSIC/02 Requiem - II Kyrie.mp3"
		++bs;
	}
	bs=0;lb=0;			// lb= lower boundary for backward searches, usually ptr to the line beginning
	while(bs<filesize)
	{
		while((bs<filesize)&&(SharedTmpBuffer[bs]=='\0')) ++bs;	// position to next valid character
		if(bs>=filesize)
		{
			free((void*)SharedTmpBuffer);
			current_entry=play_list;
			if(play_list!=NULL) MP3_Play(play_list->name,0);	// Process the first entry -> play music
			return;
		}
		if(SharedTmpBuffer[bs]=='#')	// process comment
		{
#ifdef DEBUGGINGM3UPROCESS
	ms_write_log("M3U : Comment at %d  ",bs);
#endif
			while((bs<filesize)&&(SharedTmpBuffer[bs]!='\0')&&(SharedTmpBuffer[bs]!=0x0A))
			{
#ifdef DEBUGGINGM3UPROCESS
	ms_write_log("M3U :  %d:%d:%c ",bs,(int)SharedTmpBuffer[bs],SharedTmpBuffer[bs]);
#endif
				++bs;	// since this is a comment, go to next line
			}
			if(bs>=filesize)
			{
				free((void*)SharedTmpBuffer);
				current_entry=play_list;
				if(play_list!=NULL) MP3_Play(play_list->name,0);	// Process the first entry -> play music
				return;
			}
#ifdef DEBUGGINGM3UPROCESS
	ms_write_log("M3U : Positioned to %d:%d:%c:",bs,(int)SharedTmpBuffer[bs],SharedTmpBuffer[bs]);
#endif
			while((bs<filesize)&&((SharedTmpBuffer[bs]=='\0')||(SharedTmpBuffer[bs]==0x0A)))
			{
#ifdef DEBUGGINGM3UPROCESS
	ms_write_log("M3U :  %d:%d:%c ",bs,(int)SharedTmpBuffer[bs],SharedTmpBuffer[bs]);
#endif
				++bs;	// position to next valid char
			}
			if(bs>=filesize)
			{
				free((void*)SharedTmpBuffer);
				current_entry=play_list;
				if(play_list!=NULL) MP3_Play(play_list->name,0);	// Process the first entry -> play music
				return;
			}
			lb=bs;
#ifdef DEBUGGINGM3UPROCESS
	ms_write_log("M3U : Repositioned to %d :%d:%c:",bs,(int)SharedTmpBuffer[bs],SharedTmpBuffer[bs]);
#endif
		}
		else
		{
#ifdef DEBUGGINGM3UPROCESS
	ms_write_log("M3U : Checking at %d",bs);
#endif
			while((bs<(filesize-4))&&!((SharedTmpBuffer[bs]=='.')&&((SharedTmpBuffer[bs+1]=='m')||(SharedTmpBuffer[bs+1]=='M'))&&((SharedTmpBuffer[bs+2]=='p')||(SharedTmpBuffer[bs+2]=='P'))&&(SharedTmpBuffer[bs+3]=='3'))) ++bs;
			be=bs+4;						// be points to char after '.mp3'
			if(bs>=(filesize-4))
			{
				free((void*)SharedTmpBuffer);
				current_entry=play_list;
				if(play_list!=NULL) MP3_Play(play_list->name,0);	// Process the first entry -> play music
				return;
			}
			while((bs>=lb)&&(SharedTmpBuffer[bs]>=' ')&&(SharedTmpBuffer[bs]!=':')) --bs;
			if((SharedTmpBuffer[bs]==':')||(SharedTmpBuffer[bs]<' ')) ++bs;
			while(bs<(filesize-4)&&(SharedTmpBuffer[bs]=='/')) ++bs;	// move to after initial /
			// bs to be is the drive independent name like "/PSP/MUSIC/This is Music.mp3"  from "ms0:/PSP/MUSIC/This is Music.mp3" or "P:/PSP/MUSIC/This is Music.MP3"

#ifdef DEBUGGINGM3UPROCESS
	ms_write_log("M3U now pointing at '%s': :%d:  bs=%d",&SharedTmpBuffer[bs],SharedTmpBuffer[bs],bs);
#endif
			tmpc=SharedTmpBuffer[be];
			SharedTmpBuffer[be]='\0';
			AddToMusicList(&SharedTmpBuffer[bs]);
			SharedTmpBuffer[be]=tmpc;
			while((bs<filesize)&&(SharedTmpBuffer[bs]!='\0')&&(SharedTmpBuffer[bs]!=0x0A)) ++bs;	// position to next "line"
		}
	}
return;
}

void MP3_Stop(void)
{
	MP3_playing = 0;
	MP3Thread_Active = 0;
	MP3_Pause(0);
}

void MP3_Exit(void)
{
	if(MP3Thread_Started==0)	// Did not use this thread at all
	{
		sceKernelTerminateDeleteThread(mp3_thread);
		return;
	}
	MP3_resident = 0;
	sceKernelWakeupThread(mp3_thread);
}

static signed short MadFixedToSshort(mad_fixed_t Fixed)
{
	if(Fixed>=MAD_F_ONE)
		return(32767);
	if(Fixed<=-MAD_F_ONE)
		return(-32767);
	Fixed=Fixed>>(MAD_F_FRACBITS-15);
	return((signed short)Fixed);
}


#if 0
Just to clarify, libmad returns this error whenever it expected to find an 
MPEG audio sync word (marking the beginning of a frame) but found 
something else instead. It's mostly an informational error; when you call 
the decoding routine again it will automatically search for the next sync 
word in the stream and resynchronize.

There are two cases when libmad expects to find a sync word:

1. After a call to mad_stream_buffer(), a sync word is expected to be 
found at the beginning of the buffer.

2. After successfully decoding a frame header and calculating the frame's 
size, another sync word is expected immediately to follow the frame.

When this error is returned, stream->this_frame points to the place in the 
stream where a sync word was expected.

Usually this error is an indication that something else is in the 
bitstream, such as an ID3 tag. In this case you might call id3_tag_query() 
to see if it really is a tag and either read or skip it.



#endif


// #define DEBUGGINGMP3PROCESS	1

int MP3_runningthread(SceSize args, void *argp)
{
int filesize=0;
int Status=1, i, process_as_sound;
int offset;
unsigned long FrameCount=0;

#ifdef DEBUGGINGMP3PROCESS
ms_write_log("\nMP3 Thread running.");
#endif
	sceKernelSleepThread();
	MP3Thread_Started=1;
	MP3_OpenAudio();
	MP3_FD=-1;
	while(MP3_resident)
	{
		MP3Thread_Active = 1;
		while(MP3Thread_Active)		// ...until MP3_Stop or problem with fileopen
		{
			if(MP3_newfile)
			{
#ifdef DEBUGGINGMP3PROCESS
ms_write_log("\nMP3 Thread : New File = '%s'  MP3_file_offset=%d",MP3_file,MP3_file_offset);
#endif
				if(MP3_FD >= 0)			// In case there was an other file playing...
				{
					sceIoClose(MP3_FD);
					MP3_FD=-1;
				}

				MP3_newfile=0;
				MP3_FD=sceIoOpen(MP3_file,PSP_O_RDONLY, 0777);
				if(MP3_FD<0)
				{
#ifdef DEBUGGINGMP3PROCESS
ms_write_log(" ERROR opening");
#endif
					MP3_playing=0;
					MP3Thread_Active = 0;
					break;
				}
				filesize=sceIoLseek(MP3_FD,0,PSP_SEEK_END);
				offset=MP3_file_offset;
				filesize-=MP3_file_offset;
#ifdef DEBUGGINGMP3PROCESS
ms_write_log(" \n  Offs=%d",offset);
#endif
				sceIoLseek(MP3_FD,offset,PSP_SEEK_SET);
				MP3_currentOffset=offset;
				mad_stream_init(&Stream);
				mad_frame_init(&Frame);
				mad_synth_init(&Synth);
				mad_timer_reset(&Timer);
				FrameCount=0;
				Status=1;
				OutputBuffer_flip=0;
				OutputPtr=OutputBuffer[0];
				OutputBufferEnd=OutputBuffer[0]+OUTPUT_BUFFER_SIZE;
				MP3_playing=1;
#ifdef DEBUGGINGMP3PROCESS
ms_write_log("\nStarted '%s' with offset %d and filesize=%d (%d : %d)",MP3_file,offset,filesize,Stream.bufend,Stream.next_frame);
#endif
			}
			else
				break;

			while(MP3_playing&&MP3Thread_Active&&Status)	// ...until MP3_Stop or MP3_Play or EndOfFile
			{
				if((Stream.buffer==NULL)||(Stream.error==MAD_ERROR_BUFLEN))
				{
				size_t		ReadSize,Remaining;
				unsigned char	*ReadStart;

					if(Stream.next_frame!=NULL)
					{
						Remaining=Stream.bufend-Stream.next_frame;
						memmove(InputBuffer,Stream.next_frame,Remaining);
						ReadStart=InputBuffer+Remaining;
						ReadSize=INPUT_BUFFER_SIZE-Remaining;
					}
					else
					{
						ReadSize=INPUT_BUFFER_SIZE;
						ReadStart=InputBuffer;
						Remaining=0;
					}
					ReadSize=sceIoRead(MP3_FD, ReadStart, ReadSize);
					if(ReadSize>=0) {filesize-=ReadSize;MP3_currentOffset+=ReadSize;}
					if(filesize==0)
					{
						MP3_currentOffset=-1;	// To signal that current file ended
						if(ReadSize>0)
						{
							GuardPtr=ReadStart+ReadSize;
							memset(GuardPtr,0,MAD_BUFFER_GUARD);
							ReadSize+=MAD_BUFFER_GUARD;
						}
					}
#ifdef DEBUGGINGMP3PROCESS
ms_write_log("\n Last frame : filesize=%d ReadSize = %d Remaining=%d InputBuffer=%p ReadStart=%p GuardPtr=%p",filesize,ReadSize,Remaining,InputBuffer,ReadStart,GuardPtr);
#endif
					if((ReadSize+Remaining)>0)
						mad_stream_buffer(&Stream,InputBuffer,ReadSize+Remaining);
					Stream.error=0;
				}

				process_as_sound=1;
				if(mad_frame_decode(&Frame,&Stream))
				{
					process_as_sound=0;
#ifdef DEBUGGINGMP3PROCESS
ms_write_log("\nStream.error=%x  Recoverable=%d  MAD_ERROR_LOSTSYNC=%x MAD_ERROR_BUFLEN=%x",Stream.error,MAD_RECOVERABLE(Stream.error),MAD_ERROR_LOSTSYNC,MAD_ERROR_BUFLEN);
#endif
					if(Stream.error==MAD_ERROR_LOSTSYNC)
					{
						signed long tagsize = id3_tag_query(Stream.this_frame,Stream.bufend-Stream.this_frame);
#ifdef DEBUGGINGMP3PROCESS
ms_write_log("\nid3_tag_query returned with %d",tagsize);
#endif
						if(tagsize>0)
						{
							mad_stream_skip(&Stream,tagsize);
						}
					}
					if((MAD_RECOVERABLE(Stream.error))==0)
						if(Stream.error!=MAD_ERROR_BUFLEN) Status=0;
					if((Stream.error==MAD_ERROR_BUFLEN)&&(MP3_currentOffset==-1)) Status=0;		// Need more data, but we are at the end of the file, so stop decoding!
				}
				if(process_as_sound)
				{
					if(FrameCount==0)
					{
						MusicBitRate=Frame.header.bitrate;
						MusicSampleRate=Frame.header.samplerate;
					}
					FrameCount++;
					mad_timer_add(&Timer,Frame.header.duration);
					mad_synth_frame(&Synth,&Frame);
#ifdef DEBUGGINGMP3PROCESS
ms_write_log("\nFrame.header.duration=%d  Synth.pcm.length=%d (%d:%d)",Frame.header.duration,Synth.pcm.length,Stream.error,MAD_ERROR_BUFLEN);
#endif
					for(i=0;i<Synth.pcm.length;i++)
					{
					signed short	Sample;
						Sample=MadFixedToSshort(Synth.pcm.samples[0][i]);
						*(OutputPtr++)=Sample&0xff;
						*(OutputPtr++)=(Sample>>8);
						if(MAD_NCHANNELS(&Frame.header)==2)
							Sample=MadFixedToSshort(Synth.pcm.samples[1][i]);
						*(OutputPtr++)=Sample&0xff;
						*(OutputPtr++)=(Sample>>8);
						if(OutputPtr==OutputBufferEnd)
						{
#ifdef DEBUGGINGMP3PROCESS
ms_write_log("\n\tOutputPtr=%p OutputBufferEnd=%p",OutputPtr,OutputBufferEnd);
#endif
							sceAudioOutputPannedBlocking(mp3_handle,MAXVOLUME,MAXVOLUME,(char*)OutputBuffer[OutputBuffer_flip] );
							OutputBuffer_flip^=1;
							OutputPtr=OutputBuffer[OutputBuffer_flip];
							OutputBufferEnd=OutputBuffer[OutputBuffer_flip]+OUTPUT_BUFFER_SIZE;
						}
					}
				}
#ifdef DEBUGGINGMP3PROCESS
ms_write_log("\nEND OF SEND = OutputPtr=%p OutputBufferEnd=%p  = %d",OutputPtr,OutputBufferEnd,(OutputBufferEnd-OutputPtr));
#endif
			}
			if(MP3_FD>=0)
			{
				sceIoClose(MP3_FD);
				MP3_FD=-1;
			}
			
			if(MP3_newfile==0) CheckSongStack();
		}
		MP3Thread_Active = 0;
		MP3_playing = 0;
		sceKernelSleepThread();
	}
	MP3_CloseAudio();
	sceKernelTerminateDeleteThread(mp3_thread);
	return (0);
}

int MP3_module_start(SceSize args, void *argp)
{
        //mp3_thread = sceKernelCreateThread("WC_MP3P", MP3_runningthread, 8, 0x04000, 0, NULL);        // small stack should be enough
        mp3_thread = sceKernelCreateThread("WC_MP3P", MP3_runningthread, 32, 0x02000, 0, NULL); // small stack should be enough
        if(mp3_thread >= 0)
        {
                sceKernelStartThread(mp3_thread, args, argp);
        }

        return 0;
}

void beep()
{
	char filepath[128];
	sprintf(filepath,"%s/system/sounds/beep.mp3", currentpath);
	MP3_PlaySnipet(filepath);
}

void play_alert(int which)
{
	if (MP3_playing==1 || which==0) return;
	char filepath1[256];
	sprintf(filepath1,"%s/%s/message%03d.mp3", currentpath,zipfile,which);
	if (my_access(filepath1, F_OK) != 0)
		sprintf(filepath1,"%s/system/sounds/message%03d.mp3", currentpath,which);
	if (my_access(filepath1, F_OK) == 0) {
		MP3_Play(filepath1,0);
	}

}

void playfile(char * filename)
{
	char filepath[64];
	sprintf(filepath,"%s/system/sounds/%s", currentpath,filename);
	MP3_PlaySnipet(filepath);
}
