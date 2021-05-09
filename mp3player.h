int MP3_playing;

void MP3_Play(char *name, int offset);
int	MP3_module_start(SceSize args, void *argp);
void	MP3_ProcessM3U(char *name);
void	MP3_PlaySnipet(char *name);
void	MP3_Stop(void);
void	MP3_Pause(int pause);	// 0 to continue, 1 to pause

void beep();
void playfile(char * filename) ;
void play_alert(int which);
