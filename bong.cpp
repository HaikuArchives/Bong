/*****************************************************************

 BONG for BeOS, based on the old classic PONG

 FreeWare

 (c) 1997 by Gertjan van Ratingen
 

*****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <AppKit.h>
#include <InterfaceKit.h>
#include <KernelKit.h>
//#include <MediaKit.h>
#include <media/MediaDefs.h>
#include <media/SoundPlayer.h>
#include <app/Message.h>
#include <device/Joystick.h>
#include <interface/Alert.h>
#include <interface/View.h>
#include <interface/InterfaceDefs.h>  // rgb_color, key_info
#include <interface/Menu.h>
#include <interface/MenuItem.h>
#include <support/SupportDefs.h>
#include <storage/Path.h>

#define DEBUG_


#if 0

In the routines UpdateLeftBat() and UpdateRightBat() the
values of the joysticks are read; the maximum and minimum
values are stored to determine the active range of the stick.


The playfield consists of DISPLAY_W x DISPLAY_H 'field-units'.

#######################################
.......................................
.......###.###.....#.....###.###.......
.......#.#.#.#...........#.#.#.#.......
.......###.###.....#.....###.###.......
.......#.#.#.#...........#.#.#.#.......
.......###.###.....#.....###.###.......
.......................................
...................#...................
.......................................
.%.................#...................
.%.....................................
.%.................#...................
.%...................................%.
...................#.................%.
.....................................%.
...................#.................%.
.......................................
...................#...................
.......................................
...................#...................
.......................................
...................#...................
.......................................
...................#...................
.......................................
...................#...................
.......................................
#######################################

At the start (and after resizing the window) the width and height
of each field-unit is calculated in pixels (variables pw and ph).
Using these values, the on-screen elements such as scores,
lines, bats and ball can be drawn.

The positions of the bats and the ball are in BeView's units
(pixels). This illustration shows the use of some variables:

##################### ph
          :         ^          ^
          :         |          |
 %        :         |bat_space |ball_yspace
 %        :         |          |
      O   :         V          |
          :        %           V
          :        %
##################### ph
<------------------>
    ball_xspace

#endif

#define DISPLAY_W 41
#define DISPLAY_H 31

#define LEFT_BAT_X 1
#define RIGHT_BAT_X (DISPLAY_W - 2)

#define FOOTBALL_TOP 9
#define FOOTBALL_BOTTOM (DISPLAY_H-FOOTBALL_TOP)

#define LEFT_SCORE_X 10
#define RIGHT_SCORE_X 22

/*---------------------------------------------------------------*/

// frequencies for 'classic' samples
#define BAT_FREQ 800
#define OUT_FREQ 104
#define WALL_FREQ 208
#define OVER_FREQ 500

// 
#define BAT_TIME 0.05
#define OUT_TIME 0.4
#define WALL_TIME 0.05
#define OVER_TIME 1.0

#define INTRO_SND 0
#define OUT_SND 1
#define OVER_SND 2
#define WALL_SND 3
#define NUM_WALL_SND 8
#define LBAT_SND WALL_SND+NUM_WALL_SND
#define NUM_LBAT_SND 8
#define RBAT_SND LBAT_SND+NUM_LBAT_SND
#define NUM_RBAT_SND 9

#define NUM_SAMPLES RBAT_SND+NUM_RBAT_SND

struct {
	char *name;
	char *data;
	long size;
	int freq;
	float time;
	char *curr_data;
	long samples_left;
	float panning;
} samples[] = {
{ "intro.raw", NULL, 0, -1, 0, NULL,0, 0.0 },
{ "out.raw", NULL, 0, OUT_FREQ, OUT_TIME, NULL,0, 0.0 },
{ "over.raw", NULL, 0, OVER_FREQ, OVER_TIME, NULL,0, 0.0 },

{ "wall1.raw", NULL, 0, WALL_FREQ, WALL_TIME, NULL,0, 0.0 },
{ "wall2.raw", NULL, 0, WALL_FREQ, WALL_TIME, NULL,0, 0.0 },
{ "wall3.raw", NULL, 0, WALL_FREQ, WALL_TIME, NULL,0, 0.0 },
{ "wall4.raw", NULL, 0, WALL_FREQ, WALL_TIME, NULL,0, 0.0 },
{ "wall5.raw", NULL, 0, WALL_FREQ, WALL_TIME, NULL,0, 0.0 },
{ "wall6.raw", NULL, 0, WALL_FREQ, WALL_TIME, NULL,0, 0.0 },
{ "wall7.raw", NULL, 0, WALL_FREQ, WALL_TIME, NULL,0, 0.0 },
{ "wall8.raw", NULL, 0, WALL_FREQ, WALL_TIME, NULL,0, 0.0 },

{ "lbat1.raw", NULL, 0, BAT_FREQ, BAT_TIME, NULL,0, 0.0 },
{ "lbat2.raw", NULL, 0, BAT_FREQ, BAT_TIME, NULL,0, 0.0 },
{ "lbat3.raw", NULL, 0, BAT_FREQ, BAT_TIME, NULL,0, 0.0 },
{ "lbat4.raw", NULL, 0, BAT_FREQ, BAT_TIME, NULL,0, 0.0 },
{ "lbat5.raw", NULL, 0, BAT_FREQ, BAT_TIME, NULL,0, 0.0 },
{ "lbat6.raw", NULL, 0, BAT_FREQ, BAT_TIME, NULL,0, 0.0 },
{ "lbat7.raw", NULL, 0, BAT_FREQ, BAT_TIME, NULL,0, 0.0 },
{ "lbat8.raw", NULL, 0, BAT_FREQ, BAT_TIME, NULL,0, 0.0 },

{ "rbat1.raw", NULL, 0, BAT_FREQ, BAT_TIME, NULL,0, 0.0 },
{ "rbat2.raw", NULL, 0, BAT_FREQ, BAT_TIME, NULL,0, 0.0 },
{ "rbat3.raw", NULL, 0, BAT_FREQ, BAT_TIME, NULL,0, 0.0 },
{ "rbat4.raw", NULL, 0, BAT_FREQ, BAT_TIME, NULL,0, 0.0 },
{ "rbat5.raw", NULL, 0, BAT_FREQ, BAT_TIME, NULL,0, 0.0 },
{ "rbat6.raw", NULL, 0, BAT_FREQ, BAT_TIME, NULL,0, 0.0 },
{ "rbat7.raw", NULL, 0, BAT_FREQ, BAT_TIME, NULL,0, 0.0 },
{ "rbat8.raw", NULL, 0, BAT_FREQ, BAT_TIME, NULL,0, 0.0 },
{ "rbat9.raw", NULL, 0, BAT_FREQ, BAT_TIME, NULL,0, 0.0 }
};

/*---------------------------------------------------------------*/


// Window/Application messages
#define MSG_UPDATE 'updt'
#define MSG_RESTART 'strt'
#define MSG_SHOWMENU 'shmn'

#define MSG_TENNIS 'tenn'
#define MSG_FOOTBALL 'foot'
#define MSG_SQUASH 'squa'

#define MSG_SPEED_LOW 'spdl'
#define MSG_SPEED_NORMAL 'spdn'
#define MSG_SPEED_HIGH 'spdh'
#define MSG_SPEED_LUDICROUS 'spdx'

#define MSG_ANGLE_LOW 'angl'
#define MSG_ANGLE_NORMAL 'angn'
#define MSG_ANGLE_HIGH 'angh'

#define MSG_BAT_SMALL 'bats'
#define MSG_BAT_NORMAL 'batn'
#define MSG_BAT_LARGE 'batl'

#define MSG_PLAYER_0 'ply0'
#define MSG_PLAYER_L 'plyl'
#define MSG_PLAYER_R 'plyr'
#define MSG_PLAYER_2 'ply2'

#define MSG_COLOR 'colr'
#define MSG_BW 'bw  '

#define MSG_BEEP 'beep'
#define MSG_SAMPLE 'smpl'

#define MSG_KEYBOARD 'keyb'
#define MSG_JOYSTICK_1_2 'jy12'
#define MSG_JOYSTICK_1 'joy1'
#define MSG_JOYSTICK_2 'joy2'

// types of games
#define TENNIS 0
#define FOOTBALL 1
#define SQUASH 2
#define NUM_TYPES 3

// types of controls
#define KEYBOARD 0
#define JOYSTICK_1_2 1
#define JOYSTICK_1 2
#define JOYSTICK_2 3
#define NUM_CONTROLS 4

bigtime_t snooze_time = 30000;

// prototype for dac callback function
void dac_func(void *arg, void *buf, size_t count, const media_raw_audio_format &format);

class bong;
class bongTimer;
class bongWindow;
class bongView;

class bong : public BApplication
{
public:
	bong(); // constructor
	~bong(); // destructor
	virtual void ReadyToRun();
	virtual bool QuitRequested();
	virtual void AboutRequested();
	virtual void MessageReceived(BMessage *msg);

private:
	bongWindow *bong_window;
	bongTimer *bong_timer;
};

class bongTimer : public BLooper
{
public:
	bongTimer(BWindow *window);
	~bongTimer();
	virtual bool QuitRequested(void);
	virtual void MessageReceived(BMessage *msg);
private:
	BWindow *target_window;
};

class bongWindow : public BWindow
{
public:
	bongWindow(int width, int height); // constructor
	virtual bool QuitRequested(void);
	virtual void MessageReceived(BMessage *msg);
	virtual void WindowActivated(bool active);
	void Restart(void);

	bongView *bong_view;
	bool active;
private:
	bool quit_forwarded; // Flag: Quit request already forwarded to be_app
};


class bongView : public BView
{
public:
	bongView(BRect frame); // constructor
	~bongView(); // destructor
	virtual void Draw(BRect update);
	virtual void AttachedToWindow(void);
	virtual void FrameResized(float w, float h);
	void SaveSettings();
	void MouseDown(BPoint where);
	void HandlePopupMenu(float x, float y);
	void Pause(bool do_pause);
	void SetType(int type);
	void SetPlayers(bool left, bool right);
	void SetColor(bool is_color);
	void SetSound(bool use_sample);
	void SetSpeed(float speed);
	void SetAngle(float ang);
	void SetBatSize(int size);
	void SetControls(int control);

	int GetType(void);
	int GetPlayers(void);
	int GetColor(void);
	int GetSound(void);
	int GetSpeed(void);
	int GetAngle(void);
	int GetBatSize(void);
	int GetControls(void);

	void Reset(void);
	void Redraw(void);
	void DrawScores(bool force);
	void DrawBats(bool force);
	bool Update(void);
	void Beep(int freq, float sec, float pan);

	int beeps, period, half;
	float panning;
	bool use_samples;
	BPopUpMenu *menu;

private:
	void RecalcVars(void);
	void DrawMiddleLine(void);
	void NewBall(void);
	void DrawScore(int place, int score);
	void DrawDigit(float x, int val);
	void DrawBat(float x, float y, rgb_color col);
	void DrawBall(void);
	void UpdateLeftBat(void);
	void UpdateRightBat(void);
	bool TouchDown(int side);
	void GameOverSound(void);

	BSoundPlayer *soundPlayer;

	BJoystick *joystick1, *joystick2;
	ulong downkey;

	bool paused, game_over;
	int game_over_count;
	bool has_color;
	int input_type; // KEYBOARD or JOYSTICK

	float width, height;
	int game_type; // TENNIS, FOOTBALL, SQUASH
	int bat_size; // 3 or 5 (see menuitem)
	float curr_speed, ball_speed; // speed in field-units
	float angle; // extra delta multiplicator
            	 //(<1 => lower dy, higher dx, >1 => higher dy, lower dx)
	rgb_color field, leftbat_col, rightbat_col;
	int score[2], players[2];
	float pw;  // width of 'field-unit' in screenpixels
	float ph;  // height of 'field-unit' in screenpixels
	float left_bat_y, right_bat_y; // 0...bat_space
	float left_bat_x, right_bat_x;
	float left_bat_dy, right_bat_dy;
	short joy1_min, joy1_max;
	short joy2_min, joy2_max;
	float bat_ysize;
	float new_left_bat, new_right_bat;
	float ball_x, ball_y; // 0...ball_xspace and 0..ball_yspace
	float ball_dx, ball_dy; // 1... depends on speed and angle
	float new_ball_x, new_ball_y;
	float new_x, new_y;
	float ball_xspace;
	float ball_yspace;
	float bat_space;
	float football_top; // y-coord of top of football-goal
	float football_bottom; // y-coord of top of football-goal
};


#ifdef DEBUG
#define debug(x) printf x
#else
#define debug(x)
#endif


/*---------------------------------------------------------------*/

/*
* dac_func: if bongView's beeps are set,
* mix a blockwave with audiostream
*/
void dac_func(void *arg, void *buf, size_t size, const media_raw_audio_format &format)
{
	int i;
	float l, r;
	int32 num;
	float sample;
	float *out_floats;
	float pan;
	bongView *view = (bongView *)arg;

	if (arg==NULL || buf==NULL || size==0) return;

	out_floats = (float *)buf;
	num = size/8; // number of float 44.1 kHz stereo samples
	num /= 4; // because samplerate of samples is 11025 Hz
	while(num-- > 0)
	{
		l = 0.0; r = 0.0; // start with silence
		if (view->use_samples)
		{
			for(i=0;i<NUM_SAMPLES;i++)
			{
				if (samples[i].samples_left > 0) // any samples left to play?
				{
					pan = samples[i].panning;
					samples[i].samples_left--;
					sample = (float) (*(samples[i].curr_data))/128.0;
					l += sample * (1.0-pan);
					r += sample * pan;
					samples[i].curr_data++;
				}
			}
		}
		if (view->beeps > 0)
		{
			pan = view->panning;
			if (view->period < 0)
			{
				l += 0.5 * (1.0 - pan);
				r += 0.5 * pan;
			}
			else
			{
				l -= 0.5 * (1.0 - pan);
				r -= 0.5 * pan;
			}
			view->period++;
			if (view->period >= view->half)
				view->period = -view->half;

			view->beeps--;
		}

		if (l > 1.0) l = 1.0;
		else if (l < -1.0) l = -1.0;
		if (r > 1.0) r = 1.0;
		else if (r < -1.0) r = -1.0;

		// put each 11025 Hz sample 4 times in the buffer to get 44100 Hz
		for(i=0; i<4; i++)
		{
			*out_floats++ = l; // Left channel
			*out_floats++ = r; // Right channel
		}
	}
}

/*---------------------------------------------------------------*/

/*
* random generator, 'borrowed' from Marco Nelissens blanker "Static"
*/
unsigned long idum=0;

inline unsigned long lrand()
{
	idum=1664525L*idum+1013904223L;
	
	if(idum&0x00800200)
		idum=1664525L*idum+1013904223L;
	return idum;
}


/*---------------------------------------------------------------*/

bong *the_app;

// Create application object and start it
int main(void)
{
	the_app = new bong;
	the_app->Run();
	delete the_app;

	return 0;
}

bong::bong() : BApplication("application/x-gjvr-bong")
{
}

bong::~bong()
{
	int i;

	for(i=0;i<NUM_SAMPLES;i++)
	{
		if (samples[i].data) free(samples[i].data);
	}
}

bool bong::QuitRequested(void)
{
//	bong_timer->PostMessage(B_QUIT_REQUESTED);
	bong_timer->Quit();
	if (BApplication::QuitRequested())
	{
		return TRUE;
	}
	return FALSE;
}

void bong::AboutRequested()
{
	BAlert* about = new BAlert("",
		"Bong 1.4 for R4 (16-jun-1999)\n" \
		"Author: Gertjan van Ratingen\n\n" \
		"E-mail: gertjan@iae.nl\n\n" \
		"Use joysticks (or SHIFT and CTRL keys) to control each bat",
		"Oui, droite!");
	about->Go();
}


void bong::ReadyToRun()
{
	app_info ai;
	BEntry entry, parent;
	BPath bpath;

	FILE *f;
	int i;
	long size;
	char *path = new char[1000];
	char *name = new char[1000];

	debug(("bong::ReadyToRun()\n"));

	strcpy(path, "."); // default
	if (be_app->GetAppInfo(&ai) == B_OK)
		if (entry.SetTo(&ai.ref) == B_OK)
			if (entry.GetParent(&parent) == B_OK)
				if (parent.GetPath(&bpath) == B_OK)
					strcpy(path, bpath.Path());

	// read the audiofiles
	for (i=0;i<NUM_SAMPLES;i++)
	{
		samples[i].size = 0;
		samples[i].samples_left = 0;
		samples[i].data = NULL;
		samples[i].curr_data = NULL;
		sprintf(name, "%s/bongdata/%s", path, samples[i].name);
		f = fopen(name, "rb");
		if (f != NULL)
		{
			fseek(f, 0, 2); // seek to end
			size = ftell(f);
			fseek(f, 0, 0); // seek to start
			samples[i].data = (char *)malloc(size);
			if (samples[i].data != NULL)
			{
				samples[i].size = size;
				if (fread(samples[i].data, size, 1, f) != 1)
					samples[i].size = 0;
				debug(("%s: %d bytes\n", samples[i].name, samples[i].size));
			}
			fclose(f);
		}
	}

	delete[] path;
	delete[] name;

	bong_window = new bongWindow(DISPLAY_W*10, DISPLAY_H*10);

	bong_timer = new bongTimer(bong_window);
	bong_timer->PostMessage(MSG_UPDATE); // get the timer going !!
}

void bong::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{
		default:
			BApplication::MessageReceived(msg);
			break;
	}
}


/*********************************************************************
 *  Timer
 *********************************************************************/
bongTimer::bongTimer(BWindow *window) : BLooper()
{
	debug(("bongTimer::bongTimer()\n"));

	target_window = window;
	Run();
}

bongTimer::~bongTimer()
{
	debug(("bongTimer::~bongTimer()\n"));
}

void bongTimer::MessageReceived(BMessage *msg)
{
//	debug(("bongTimer::MessageReceived()\n"));
	switch(msg->what)
	{
		case MSG_UPDATE:
			if (((bongWindow *)target_window)->active)
				target_window->PostMessage(MSG_UPDATE);
			snooze(snooze_time);
			PostMessage(MSG_UPDATE);
			break;
		default:
			BLooper::MessageReceived(msg);
	}
}

bool bongTimer::QuitRequested(void)
{
	debug(("bongTimer::QuitRequested()\n"));
	return BLooper::QuitRequested();
}



/*********************************************************************
 *  Window
 *********************************************************************/

bongWindow::bongWindow(int width, int height) :
	BWindow(BRect(0, 0, width-1, height-1), "Bong",
				B_TITLED_WINDOW, 0)
{
	quit_forwarded = FALSE;
	active = TRUE;
	MoveTo(100, 100);
	SetSizeLimits(DISPLAY_W, 1000000, DISPLAY_H, 1000000);
	bong_view = new bongView(BRect(0,0, width-1, height-1));

	Lock();
	AddChild(bong_view);
	Unlock();

	// read initial settings from a settingsfile
	FILE *f = fopen("/boot/home/config/settings/Bong_prefs", "r");
	if (f)
	{
		char temp[100];
		int i1, i2;

		while(fgets(temp, 100, f))
		{
			if (strncmp(temp, "pos=", 4)==0)
			{
				sscanf(temp+4, "%d,%d", &i1, &i2);
				MoveTo(i1, i2);
			}
			if (strncmp(temp, "size=", 5)==0)
			{
				sscanf(temp+5, "%d,%d", &i1, &i2);
				ResizeTo(i1, i2);
			}
			if (strncmp(temp, "players=", 8)==0)
			{
				sscanf(temp+8, "%d,%d", &i1, &i2);
				bong_view->SetPlayers(i1?TRUE:FALSE, i2?TRUE:FALSE);
			}
			if (strncmp(temp, "game=", 5)==0)
			{
				sscanf(temp+5, "%d", &i1);
				bong_view->SetType(i1);
			}
			if (strncmp(temp, "input=", 6)==0)
			{
				sscanf(temp+6, "%d", &i1);
				bong_view->SetControls(i1);
			}
			if (strncmp(temp, "batsize=", 8)==0)
			{
				sscanf(temp+8, "%d", &i1);
				bong_view->SetBatSize(i1);
			}
			if (strncmp(temp, "angle=", 6)==0)
			{
				sscanf(temp+6, "%d", &i1);
				bong_view->SetAngle((float)i1/10.0);
			}
			if (strncmp(temp, "color=", 6)==0)
			{
				sscanf(temp+6, "%d", &i1);
				Lock();
				bong_view->SetColor(i1?TRUE:FALSE);
				Unlock();
			}
			if (strncmp(temp, "sound=", 6)==0)
			{
				sscanf(temp+6, "%d", &i1);
				Lock();
				bong_view->SetSound(i1?TRUE:FALSE);
				Unlock();
			}
			if (strncmp(temp, "ballspeed=", 10)==0)
			{
				sscanf(temp+10, "%d", &i1);
				bong_view->SetSpeed((float)i1/10.0);
			}
		}
		fclose(f);
	}

	BMenuItem *item;
	BMenu *submenu;
	BPopUpMenu *menu;
	int i;

	menu = new BPopUpMenu("", FALSE, FALSE);

	item = new BMenuItem("New game", new BMessage(MSG_RESTART));
	item->SetTarget(this);
	menu->AddItem(item);

	menu->AddSeparatorItem();

	i = bong_view->GetType();

	submenu = new BMenu("Gametype");
	item = new BMenuItem("Tennis", new BMessage(MSG_TENNIS));
	if (i == TENNIS) item->SetMarked(TRUE);
	submenu->AddItem(item);
	item = new BMenuItem("Football", new BMessage(MSG_FOOTBALL));
	if (i == FOOTBALL) item->SetMarked(TRUE);
	submenu->AddItem(item);
	item = new BMenuItem("Squash", new BMessage(MSG_SQUASH));
	if (i == SQUASH) item->SetMarked(TRUE);
	submenu->AddItem(item);
	submenu->SetTargetForItems(this);
	submenu->SetRadioMode(TRUE);

	menu->AddItem(submenu);

	i = bong_view->GetPlayers();
	submenu = new BMenu("Players");
	item = new BMenuItem("left & right", new BMessage(MSG_PLAYER_2));
	if (i == 3) item->SetMarked(TRUE);
	submenu->AddItem(item);
	item = new BMenuItem("left", new BMessage(MSG_PLAYER_L));
	if (i == 1) item->SetMarked(TRUE);
	submenu->AddItem(item);
	item = new BMenuItem("right", new BMessage(MSG_PLAYER_R));
	if (i == 2) item->SetMarked(TRUE);
	submenu->AddItem(item);
	item = new BMenuItem("no players", new BMessage(MSG_PLAYER_0));
	if (i == 0) item->SetMarked(TRUE);
	submenu->AddItem(item);
	submenu->SetTargetForItems(this);
	submenu->SetRadioMode(TRUE);

	menu->AddItem(submenu);

	i = bong_view->GetColor();
	submenu = new BMenu("Display");
	item = new BMenuItem("Black & white", new BMessage(MSG_BW));
	if (i == 0) item->SetMarked(TRUE);
	submenu->AddItem(item);
	item = new BMenuItem("Color", new BMessage(MSG_COLOR));
	if (i == 1) item->SetMarked(TRUE);
	submenu->AddItem(item);
	submenu->SetTargetForItems(this);
	submenu->SetRadioMode(TRUE);

	menu->AddItem(submenu);

	i = bong_view->GetSound();
	submenu = new BMenu("Sound");
	item = new BMenuItem("Beeps", new BMessage(MSG_BEEP));
	if (i == 0) item->SetMarked(TRUE);
	submenu->AddItem(item);
	item = new BMenuItem("Samples", new BMessage(MSG_SAMPLE));
	if (i == 1) item->SetMarked(TRUE);
	submenu->AddItem(item);
	submenu->SetTargetForItems(this);
	submenu->SetRadioMode(TRUE);

	menu->AddItem(submenu);

	i = bong_view->GetSpeed();
	submenu = new BMenu("Speed");
	item = new BMenuItem("Slow", new BMessage(MSG_SPEED_LOW));
	if (i < 5) item->SetMarked(TRUE);
	submenu->AddItem(item);
	item = new BMenuItem("Normal", new BMessage(MSG_SPEED_NORMAL));
	if (i >= 5 && i < 15) item->SetMarked(TRUE);
	submenu->AddItem(item);
	item = new BMenuItem("High", new BMessage(MSG_SPEED_HIGH));
	if (i >= 15 && i < 30) item->SetMarked(TRUE);
	submenu->AddItem(item);
	item = new BMenuItem("Ludicrous", new BMessage(MSG_SPEED_LUDICROUS));
	if (i >= 30) item->SetMarked(TRUE);
	submenu->AddItem(item);
	submenu->SetTargetForItems(this);
	submenu->SetRadioMode(TRUE);

	menu->AddItem(submenu);

	i = bong_view->GetBatSize();
	submenu = new BMenu("Batsize");
	item = new BMenuItem("Small", new BMessage(MSG_BAT_SMALL));
	if (i <= 3) item->SetMarked(TRUE);
	submenu->AddItem(item);
	item = new BMenuItem("Normal", new BMessage(MSG_BAT_NORMAL));
	if (i >3 && i < 5) item->SetMarked(TRUE);
	submenu->AddItem(item);
	item = new BMenuItem("Large", new BMessage(MSG_BAT_LARGE));
	if (i >= 5) item->SetMarked(TRUE);
	submenu->AddItem(item);
	submenu->SetTargetForItems(this);
	submenu->SetRadioMode(TRUE);

	menu->AddItem(submenu);

	i = bong_view->GetAngle();
	submenu = new BMenu("Angle");
	item = new BMenuItem("Low", new BMessage(MSG_ANGLE_LOW));
	if (i < 8) item->SetMarked(TRUE);
	submenu->AddItem(item);
	item = new BMenuItem("Normal", new BMessage(MSG_ANGLE_NORMAL));
	if (i >= 8 && i < 12) item->SetMarked(TRUE);
	submenu->AddItem(item);
	item = new BMenuItem("High", new BMessage(MSG_ANGLE_HIGH));
	if (i >= 12) item->SetMarked(TRUE);
	submenu->AddItem(item);
	submenu->SetTargetForItems(this);
	submenu->SetRadioMode(TRUE);

	menu->AddItem(submenu);

	i = bong_view->GetControls();
	submenu = new BMenu("Controls");
	item = new BMenuItem("Keyboard", new BMessage(MSG_KEYBOARD));
	if (i == KEYBOARD) item->SetMarked(TRUE);
	submenu->AddItem(item);
	item = new BMenuItem("Joysticks port 1 & 2", new BMessage(MSG_JOYSTICK_1_2));
	if (i == JOYSTICK_1_2) item->SetMarked(TRUE);
	submenu->AddItem(item);
	item = new BMenuItem("Split joystick port 1", new BMessage(MSG_JOYSTICK_1));
	if (i == JOYSTICK_1) item->SetMarked(TRUE);
	submenu->AddItem(item);
	item = new BMenuItem("Split joystick port 2", new BMessage(MSG_JOYSTICK_2));
	if (i == JOYSTICK_2) item->SetMarked(TRUE);
	submenu->AddItem(item);
	submenu->SetTargetForItems(this);
	submenu->SetRadioMode(TRUE);

	menu->AddItem(submenu);

	menu->AddSeparatorItem();

	item = new BMenuItem("About Bong...", new BMessage(B_ABOUT_REQUESTED));
	item->SetTarget(be_app);
	menu->AddItem(item);

	menu->AddSeparatorItem();

	item = new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q');
	item->SetTarget(be_app);
	menu->AddItem(item);

	bong_view->menu = menu;

	Show();
}


/*
 *  Closing the window quits the program
 */

bool bongWindow::QuitRequested(void)
{
	// Forward quit request to application if not already done
	if (!quit_forwarded) {
		bong_view->SaveSettings();
		quit_forwarded = TRUE;
		be_app->PostMessage(B_QUIT_REQUESTED);
		return FALSE;
	}
	return TRUE;
}

void bongWindow::WindowActivated(bool act)
{
	active = act;
	if (active == FALSE)
	{
		// some jerk just made another window active, so let's drop
		// all activity immediately
//		bong_view->Pause(TRUE);
	}
	else
	{
		// generate an UPDATE message to continue the game 
//		bong_view->Pause(FALSE);
	}
}


/*
 *  Handles messages
 */

void bongWindow::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case MSG_RESTART:
			bong_view->Reset();
			break;
		case MSG_UPDATE:
			bong_view->Update();
			break;
		case MSG_TENNIS:
			bong_view->SetType(TENNIS);
			bong_view->Redraw();
			break;
		case MSG_FOOTBALL:
			bong_view->SetType(FOOTBALL);
			bong_view->Redraw();
			break;
		case MSG_SQUASH:
			bong_view->SetType(SQUASH);
			bong_view->Redraw();
			break;
		case MSG_PLAYER_0:
			bong_view->SetPlayers(FALSE,FALSE);
			break;
		case MSG_PLAYER_L:
			bong_view->SetPlayers(TRUE,FALSE);
			break;
		case MSG_PLAYER_R:
			bong_view->SetPlayers(FALSE,TRUE);
			break;
		case MSG_PLAYER_2:
			bong_view->SetPlayers(TRUE,TRUE);
			break;
		case MSG_COLOR:
			bong_view->SetColor(TRUE);
			bong_view->Redraw();
			break;
		case MSG_BW:
			bong_view->SetColor(FALSE);
			bong_view->Redraw();
			break;
		case MSG_BEEP:
			bong_view->SetSound(FALSE);
			break;
		case MSG_SAMPLE:
			bong_view->SetSound(TRUE);
			break;
		case MSG_SPEED_LOW:
			bong_view->SetSpeed(0.5);
			break;
		case MSG_SPEED_NORMAL:
			bong_view->SetSpeed(1.0);
			break;
		case MSG_SPEED_HIGH:
			bong_view->SetSpeed(2.0);
			break;
		case MSG_SPEED_LUDICROUS:
			bong_view->SetSpeed(50.0);
			break;
		case MSG_ANGLE_LOW:
			bong_view->SetAngle(0.625);
			break;
		case MSG_ANGLE_NORMAL:
			bong_view->SetAngle(1.0);
			break;
		case MSG_ANGLE_HIGH:
			bong_view->SetAngle(1.6);
			break;
		case MSG_BAT_SMALL:
			bong_view->SetBatSize(3);
			bong_view->Redraw();
			break;
		case MSG_BAT_NORMAL:
			bong_view->SetBatSize(4);
			bong_view->Redraw();
			break;
		case MSG_BAT_LARGE:
			bong_view->SetBatSize(5);
			bong_view->Redraw();
			break;
		case MSG_KEYBOARD:
			bong_view->SetControls(KEYBOARD);
			break;
		case MSG_JOYSTICK_1_2:
			bong_view->SetControls(JOYSTICK_1_2);
			break;
		case MSG_JOYSTICK_1:
			bong_view->SetControls(JOYSTICK_1);
			break;
		case MSG_JOYSTICK_2:
			bong_view->SetControls(JOYSTICK_2);
			break;
		case MSG_SHOWMENU:
			{
				float x,y;
				if (msg->FindFloat("x", &x) == B_OK &&
					msg->FindFloat("y", &y) == B_OK)
				{
					active = FALSE;
					bong_view->HandlePopupMenu(x,y);
					active = TRUE;
				}
			}
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}


/*********************************************************************
 *  View
 *********************************************************************/
bongView::bongView(BRect frame) : BView(frame, "",
					B_FOLLOW_ALL_SIDES,
					B_WILL_DRAW | B_PULSE_NEEDED | B_FRAME_EVENTS)
{
	width = frame.right - frame.left + 1;
	height = frame.bottom - frame.top + 1;
	beeps = 0;

	joystick1 = new BJoystick();
	joystick2 = new BJoystick();

	media_raw_audio_format format;
	format = media_raw_audio_format::wildcard;
	format.frame_rate = 44100.0;
	format.channel_count = 2;
	format.format = media_raw_audio_format::B_AUDIO_FLOAT;
#if B_HOST_IS_BENDIAN
	format.byte_order = B_MEDIA_BIG_ENDIAN;
#else
	format.byte_order = B_MEDIA_LITTLE_ENDIAN;
#endif
//	format.buffer_size = 4096;

	soundPlayer = new BSoundPlayer(&format,"Bong", dac_func, NULL, (void *)this);
	if (soundPlayer->Start() == B_OK)
	{
		soundPlayer->SetHasData(true);
	}

	// these are the default settings
	SetType(TENNIS);
	SetPlayers(FALSE, FALSE); // computerized players
	SetColor(FALSE); // black & white display
	SetSound(TRUE); // samples
	SetSpeed(1.0);
	SetAngle(1.0);
	SetBatSize(4);
	SetControls(KEYBOARD);
}

bongView::~bongView()
{
	if (soundPlayer)
	{
		soundPlayer->Stop();
		delete soundPlayer;
	}
	delete joystick1;
	delete joystick2;
}

void bongView::SaveSettings()
{
	FILE *f = fopen("/boot/home/config/settings/Bong_prefs", "w");
	if (f)
	{
		int x,y;
		x = (int)(Window()->Frame().left);
		y = (int)(Window()->Frame().top);
		fprintf(f, "pos=%d,%d\n", x, y);
		fprintf(f, "size=%d,%d\n", (int)width, (int)height);
		fprintf(f, "players=%d,%d\n", players[0], players[1]);
		fprintf(f, "game=%d\n", game_type);
		fprintf(f, "input=%d\n", input_type);
		fprintf(f, "batsize=%d\n", bat_size);
		fprintf(f, "ballspeed=%d\n", (int)(ball_speed*10.0));
		fprintf(f, "angle=%d\n", (int)(angle*10.0));
		fprintf(f, "color=%d\n", has_color);
		fprintf(f, "sound=%d\n", use_samples);
		fclose(f);
	}
}

void bongView::AttachedToWindow(void)
{
	debug(("bongView::AttachedToWindow()\n"));
	MakeFocus(TRUE);
	RecalcVars();
	left_bat_y = 0;
	right_bat_y = 0;
	new_left_bat = (bat_space - bat_ysize) / 2;
	new_right_bat = (bat_space - bat_ysize) / 2;
	ball_x = -1;
	ball_y = -1;
	Reset();
	game_over = TRUE; // start initially in sleepy mode
	game_over_count = 0;
	ball_dx = 0.0;
}

void bongView::Draw(BRect update)
{
	Redraw();
	Flush();
}


void bongView::FrameResized(float w, float h)
{
	// recalculate current positions and sizes:
	// - convert to field-units
	// - calculate the new pw and ph
	// - convert back to screenpixels
	ball_x = ball_x / pw;
	ball_y = ball_y / ph;
	ball_dx = ball_dx / pw;
	ball_dy = ball_dy / ph;
	left_bat_y = left_bat_y / ph;
	right_bat_y = right_bat_y / ph;

	width = w;
	height = h;

	RecalcVars();

	ball_x = ball_x * pw;
	ball_y = ball_y * ph;
	ball_dx = ball_dx * pw;
	ball_dy = ball_dy * ph;
	new_ball_x = ball_x;
	new_ball_y = ball_y;
	left_bat_y = left_bat_y * ph;
	right_bat_y = right_bat_y * ph;
	new_left_bat = left_bat_y;
	new_right_bat = right_bat_y;
	Redraw();
}

void bongView::MouseDown(BPoint where)
{
	BMessage *msg;

	ConvertToScreen(&where);
	msg = new BMessage(MSG_SHOWMENU);
	msg->AddFloat("x", where.x);
	msg->AddFloat("y", where.y);
	Window()->PostMessage(msg);
	delete msg;
}

void bongView::HandlePopupMenu(float x, float y)
{
	BMenuItem *selected;
	BMessage *copy;

	selected = menu->Go(BPoint(x,y));
	if (selected)
	{
		BLooper *looper;
		BHandler *target = selected->Target(&looper);
		if (target == NULL) target = looper->PreferredHandler();
		copy = new BMessage(*(selected->Message()));
		looper->PostMessage(copy, target);
		delete copy;
		selected->SetMarked(TRUE);
	}
}

void bongView::RecalcVars()
{
	pw = width / DISPLAY_W;
	ph = height / DISPLAY_H;
	if (game_type == FOOTBALL)
	{
		left_bat_x = (LEFT_BAT_X + 2) * pw;
		right_bat_x = (RIGHT_BAT_X - 2) * pw;
	}
	else
	{
		left_bat_x = LEFT_BAT_X * pw;
		right_bat_x = RIGHT_BAT_X * pw;
	}
	ball_xspace = width - pw;
	ball_yspace = height - ph - 2*ph;
	bat_ysize = bat_size * ph;
	bat_space = height - bat_ysize - 2*ph;
	football_top = (FOOTBALL_TOP + 1) * ph - 1;
	football_bottom = FOOTBALL_BOTTOM * ph;
}


void bongView::Pause(bool do_pause)
{
	paused = do_pause;
}

int bongView::GetPlayers(void)
{
	return(players[0] + 2*players[1]);
}
int bongView::GetType(void)
{
	return(game_type);
}
int bongView::GetControls(void)
{
	return(input_type);
}
int bongView::GetBatSize(void)
{
	return(bat_size);
}
int bongView::GetSpeed(void)
{
	return((int)(curr_speed*10.0));
}
int bongView::GetAngle(void)
{
	return((int)(angle*10.0));
}
int bongView::GetColor(void)
{
	return(has_color);
}
int bongView::GetSound(void)
{
	return(use_samples);
}


void bongView::SetType(int type)
{
	if (type >= 0 && type < NUM_TYPES)
		game_type = type;
	RecalcVars();
}

void bongView::SetPlayers(bool left, bool right)
{
	players[0] = left;
	players[1] = right;
}

void bongView::SetColor(bool is_color)
{
	has_color = is_color;
	if (is_color)
	{
		field.red=0;field.green=60;field.blue=0;
		leftbat_col.red=230;leftbat_col.green=230;leftbat_col.blue=60;
		rightbat_col.red=100;rightbat_col.green=100;rightbat_col.blue=220;
		SetViewColor(0,60,0);
	}
	else
	{
		field.red=0;field.green=0;field.blue=0;
		leftbat_col.red=255;leftbat_col.green=255;leftbat_col.blue=255;
		rightbat_col.red=200;rightbat_col.green=200;rightbat_col.blue=200;
		SetViewColor(0,0,0);
	}
}

void bongView::SetSound(bool use_sample)
{
	int i, num;

	for(i=0,num=0;i<NUM_SAMPLES;i++)
	{
		if (samples[i].size > 0) num++;
	}
	if (num == 0) use_samples = FALSE;
	else use_samples = use_sample;

	if (use_samples == FALSE)
	{
		for(i=0;i<NUM_SAMPLES;i++)
		{
			samples[i].samples_left = 0;
		}
	}
}

void bongView::SetSpeed(float speed)
{
	curr_speed = speed;

	if (speed > 4) // ludicrous speed
	{
		speed = 4;
		snooze_time = 10000;
	}
	else snooze_time = 30000;

//	speed *= 0.5;

	ball_dx *= speed/ball_speed;
	ball_dy *= speed/ball_speed;

	ball_speed = speed;
}

void bongView::SetAngle(float ang)
{
	angle = ang;
}

void bongView::SetBatSize(int size)
{
	if (size > 0 && size < DISPLAY_H-2)
	{
		bat_size = size;
		bat_ysize = bat_size * ph;
		bat_space = height - bat_ysize - 2*ph;
		if (left_bat_y > bat_space) left_bat_y = bat_space;
		if (right_bat_y > bat_space) right_bat_y = bat_space;
		new_left_bat = left_bat_y;
		new_right_bat = right_bat_y;
	}
}

void bongView::SetControls(int control)
{
	long rc1=B_NO_ERROR, rc2=B_NO_ERROR;

	debug(("bongView::SetControls(%d)\n", control));

	if (control >= 0 && control < NUM_CONTROLS)
	{
		input_type = control;
	}
	else
	{
		input_type = KEYBOARD;
	}

	switch(input_type)
	{
		case JOYSTICK_1_2:
			rc1 = joystick1->Open("joystick3");
			rc2 = joystick2->Open("joystick1");
			break;
		case JOYSTICK_1:
			rc1 = joystick1->Open("joystick3");
			rc2 = joystick2->Open("joystick4");
			break;
		case JOYSTICK_2:
			rc1 = joystick1->Open("joystick1");
			rc2 = joystick2->Open("joystick2");
			break;
	}

	if (rc1 == B_ERROR || rc2 == B_ERROR)
	{
		debug(("error opening joyports, rc1=%d rc2=%d\n", rc1, rc2));
		input_type = KEYBOARD;
	}

	if (input_type != KEYBOARD)
	{
		joy1_min = 4096; joy1_max = 0;
		joy2_min = 4096; joy2_max = 0;
	}
}

void bongView::Reset(void)
{
	debug(("bongView::Reset()\n"));

	Beep(INTRO_SND, 1.0, 0.5);
	game_over = FALSE;
	paused = FALSE;
	score[0] = 0;
	score[1] = 0;
	ball_dx = ball_speed * (1.0 / angle) * pw * 0.5;
	if (lrand()%2)
		ball_dx = -ball_dx;
	NewBall();
	Redraw();
}


void bongView::Redraw(void)
{
	float x1, y1;

//	debug(("bongView::Redraw()\n"));

	x1 = width-1;
	y1 = height-1;

	// fill background
	SetHighColor(field);
	FillRect(Bounds());

	SetHighColor(255,255,255,0);
	FillRect(BRect(0,0, x1,ph-1)); // upper wall
	FillRect(BRect(0,y1-ph+1, x1,y1)); // lower wall
	if (game_type == FOOTBALL)
	{
		FillRect(BRect(0, 0, pw-1, football_top)); // left wall top
		FillRect(BRect(0, football_bottom, pw-1, y1)); // left wall bottom
		FillRect(BRect(x1-pw+1, 0, x1, football_top)); // right wall top
		FillRect(BRect(x1-pw+1, football_bottom, x1, y1)); // right wall bottom
	}
	else if (game_type == SQUASH)
	{
		FillRect(BRect(0, 0, pw-1, y1)); // left wall
	}
	DrawMiddleLine();
	DrawScores(TRUE);
	DrawBats(TRUE);
	DrawBall();
	Sync();
}

void bongView::DrawMiddleLine(void)
{
	float i, x1;

	if (game_type != SQUASH)
	{
		x1 = (width - pw) / 2;
		SetHighColor(255,255,255,0);
		for(i=2; i<DISPLAY_H-2;i+=2)
		{
			FillRect(BRect(x1, i*ph, x1+pw-1, i*ph+ph-1));
		}
	}
}


void bongView::NewBall(void)
{
//	debug(("bongView::NewBall()\n"));
	new_ball_x = ball_xspace / 2;
	new_ball_y = ball_yspace / 2;
	ball_dy = ball_speed * angle * ph * 0.5;
	if ((lrand()/10)%2) ball_dy = -ball_dy;
}

void bongView::DrawScores(bool force)
{
//	debug(("bongView::DrawScores()\n"));
	// erase old scores first
	if (force)
	{
		DrawScore(0, -1);
		DrawScore(1, -1);
	}
	// draw new score
	DrawScore(0, score[0]);
	DrawScore(1, score[1]);
}


// DrawScore: place = 0 (left) or 1 (right), score = 0..15
// (if score == -1, erase both score-digits)
void bongView::DrawScore(int place, int score)
{
	float x;

	if (place == 1 && game_type == SQUASH) return;

	if (place == 0) x = LEFT_SCORE_X * pw;
	else x = RIGHT_SCORE_X * pw;

	if (score == -1)
	{
		DrawDigit(x, -1);
		DrawDigit(x+4*pw, -1);
	}
	else
	{
		if (score>=10) DrawDigit(x, score/10);
		else DrawDigit(x, -1);
		DrawDigit(x+4*pw, score%10);
	}
}

// DrawDigit: draw "0" .. "9" (if val==-1, erase area)
void bongView::DrawDigit(float x, int val)
{
	if (val == -1)
	{
		SetHighColor(field);
		val = 8;
	}
	else
	{
		SetHighColor(255,255,255,0);
	}
	switch(val)
	{
		case 0:
			FillRect(BRect(x, 2*ph, x+pw-1, 7*ph-1));
			FillRect(BRect(x+2*pw, 2*ph, x+3*pw-1, 7*ph-1));

			FillRect(BRect(x, 2*ph, x+3*pw-1, 3*ph-1));
			FillRect(BRect(x, 6*ph, x+3*pw-1, 7*ph-1));
			break;
		case 1:
			FillRect(BRect(x+2*pw, 2*ph, x+3*pw-1, 7*ph-1));
			break;
		case 2:
			FillRect(BRect(x, 4*ph, x+pw-1, 7*ph-1));
			FillRect(BRect(x+2*pw, 2*ph, x+3*pw-1, 5*ph-1));

			FillRect(BRect(x, 2*ph, x+3*pw-1, 3*ph-1));
			FillRect(BRect(x, 4*ph, x+3*pw-1, 5*ph-1));
			FillRect(BRect(x, 6*ph, x+3*pw-1, 7*ph-1));
			break;
		case 3:
			FillRect(BRect(x+2*pw, 2*ph, x+3*pw-1, 7*ph-1));

			FillRect(BRect(x, 2*ph, x+3*pw-1, 3*ph-1));
			FillRect(BRect(x, 4*ph, x+3*pw-1, 5*ph-1));
			FillRect(BRect(x, 6*ph, x+3*pw-1, 7*ph-1));
			break;
		case 4:
			FillRect(BRect(x, 2*ph, x+pw-1, 5*ph-1));
			FillRect(BRect(x+2*pw, 2*ph, x+3*pw-1, 7*ph-1));

			FillRect(BRect(x, 4*ph, x+3*pw-1, 5*ph-1));
			break;
		case 5:
			FillRect(BRect(x, 2*ph, x+pw-1, 5*ph-1));
			FillRect(BRect(x+2*pw, 4*ph, x+3*pw-1, 7*ph-1));

			FillRect(BRect(x, 2*ph, x+3*pw-1, 3*ph-1));
			FillRect(BRect(x, 4*ph, x+3*pw-1, 5*ph-1));
			FillRect(BRect(x, 6*ph, x+3*pw-1, 7*ph-1));
			break;
		case 6:
			FillRect(BRect(x, 2*ph, x+pw-1, 7*ph-1));
			FillRect(BRect(x+2*pw, 4*ph, x+3*pw-1, 7*ph-1));

			FillRect(BRect(x, 2*ph, x+3*pw-1, 3*ph-1));
			FillRect(BRect(x, 4*ph, x+3*pw-1, 5*ph-1));
			FillRect(BRect(x, 6*ph, x+3*pw-1, 7*ph-1));
			break;
		case 7:
			FillRect(BRect(x+2*pw, 2*ph, x+3*pw-1, 7*ph-1));

			FillRect(BRect(x, 2*ph, x+3*pw-1, 3*ph-1));
			break;
		case 8:
			FillRect(BRect(x, 2*ph, x+pw-1, 7*ph-1));
			FillRect(BRect(x+2*pw, 2*ph, x+3*pw-1, 7*ph-1));

			FillRect(BRect(x, 2*ph, x+3*pw-1, 3*ph-1));
			FillRect(BRect(x, 4*ph, x+3*pw-1, 5*ph-1));
			FillRect(BRect(x, 6*ph, x+3*pw-1, 7*ph-1));
			break;
		case 9:
			FillRect(BRect(x, 2*ph, x+pw-1, 5*ph-1));
			FillRect(BRect(x+2*pw, 2*ph, x+3*pw-1, 7*ph-1));

			FillRect(BRect(x, 2*ph, x+3*pw-1, 3*ph-1));
			FillRect(BRect(x, 4*ph, x+3*pw-1, 5*ph-1));
			FillRect(BRect(x, 6*ph, x+3*pw-1, 7*ph-1));
			break;
	}
}

void bongView::DrawBats(bool force)
{
//	debug(("bongView::DrawBats() at %d and %d\n", new_left_bat, new_right_bat));
	if (game_type != SQUASH)
	{
		if ((new_left_bat != left_bat_y) || force)
		{
			DrawBat(left_bat_x, left_bat_y+ph, field);
			left_bat_y = new_left_bat;
			DrawBat(left_bat_x, left_bat_y+ph, leftbat_col);
		}
	}
	if ((new_right_bat != right_bat_y) || force)
	{
		DrawBat(right_bat_x, right_bat_y+ph, field);
		right_bat_y = new_right_bat;
		DrawBat(right_bat_x, right_bat_y+ph, rightbat_col);
	}
}

void bongView::DrawBat(float x, float y, rgb_color col)
{
//	debug(("bongView::DrawBat(%f, %f)\n", x,y));
	SetHighColor(col);
	FillRect(BRect(x, y, x+pw-1, y+bat_size*ph-1));
}

void bongView::DrawBall(void)
{
	if (ball_y < 0 || ball_y > ball_yspace
		|| new_ball_y < 0 || new_ball_y > ball_yspace)
		debug(("bongView::DrawBall() from %1.f,%1.f to %1.f,%1.f\n",
			ball_x, ball_y, new_ball_x, new_ball_y));

	if (ball_x >= 0)
	{
		// erase ball first
		SetHighColor(field);
		FillRect(BRect(ball_x,ball_y+ph, ball_x+pw-1,ball_y+2*ph-1));

		// redraw parts if needed that were overwritten by the ball
		if (ball_y < 7*ph)
		{
			if (ball_x+pw >= LEFT_SCORE_X*pw && ball_x <= (LEFT_SCORE_X+7)*pw)
				DrawScore(0, score[0]); // left score digits
			if (ball_x+pw >= RIGHT_SCORE_X*pw && ball_x <= (RIGHT_SCORE_X+7)*pw)
				DrawScore(1, score[1]); // right score digits
		}
		if (ball_x+pw >= (width-pw)/2 && ball_x <= (width+pw)/2)
			DrawMiddleLine();
		else if (ball_x >= left_bat_x-pw && ball_x <= left_bat_x+pw
				&& game_type != SQUASH)
			DrawBat(left_bat_x, left_bat_y+ph, leftbat_col);
		else if (ball_x >= right_bat_x-pw && ball_x <= right_bat_x+pw)
			DrawBat(right_bat_x, right_bat_y+ph, rightbat_col);
	}

	// now we can draw ball on new position
	ball_x = new_ball_x;
	ball_y = new_ball_y;
	if (ball_x >= 0)
	{
		SetHighColor(255,255,255,0);
		FillRect(BRect(ball_x,ball_y+ph, ball_x+pw-1,ball_y+2*ph-1));
	}
}

// Update: update position of ball, check and handle collisions
// return FALSE if game should not continue, TRUE otherwise
bool bongView::Update(void)
{
	if (paused)
	{
		// the game is frozen (e.g. because the window got deactivated)
		return FALSE;
	}

	UpdateLeftBat();
	UpdateRightBat();
	DrawBats(FALSE);

	if (game_over)
	{
		Sync();
		GameOverSound();
		return TRUE; // no ball-updates, but bats can be moved
	}

	// determine new ball-position, check if we're gonna hit anything
	new_x = ball_x + ball_dx;
	new_y = ball_y + ball_dy;
	new_ball_x = ball_x;
	new_ball_y = ball_y;

	if (new_x < left_bat_x+pw)
	{
		float coll_dx, coll_y;

		// test if left bat will be hit
		coll_dx = (left_bat_x+pw) - ball_x; // distance ball <-> bat
		coll_y = ball_y + (coll_dx/ball_dx*ball_dy); // vert. movement
		if (game_type != SQUASH &&
			ball_x >= left_bat_x + pw &&
			coll_y+ph >= left_bat_y &&
			coll_y < left_bat_y+bat_ysize)
		{
			Beep(LBAT_SND, 0.05, 0.1);
			new_ball_x = left_bat_x+pw - (ball_x - (left_bat_x+pw));
			ball_dx = -ball_dx;
			// diff is distance (in pixels) from center of bat
			// and is used to adjust ball_dy a bit :)
			float diff = new_ball_y - right_bat_y - bat_ysize/2;
//			ball_dy += (diff*0.5) / ph;
			ball_dy += (left_bat_dy / 2); // spin-effect
			if (ball_dy > 4.0*ball_speed*ph)
				ball_dy = 4.0*ball_speed*ph;
			else if (ball_dy < -4.0*ball_speed*ph)
				ball_dy = -4.0*ball_speed*ph;
		}
		else if (ball_x >= 0+pw && new_x < 0+pw) // left wall hit?
		{
			coll_dx = (0+pw) - ball_x; // distance ball <-> wall
			coll_y = ball_y + (coll_dx/ball_dx*ball_dy); // vert. movement
			if (game_type == FOOTBALL &&
				(coll_y < football_top ||
				 coll_y+ph >= football_bottom))
			{
				Beep(WALL_SND, 0.05, 0.1);
				new_ball_x = 0+pw - (ball_x - (0+pw));
				ball_dx = -ball_dx;
			}
			else if (game_type == SQUASH)
			{
				Beep(WALL_SND, 0.05, 0.1);
				new_ball_x = 0+pw - (ball_x - (0+pw));
				ball_dx = -ball_dx;
			}
		}
		else if (new_x < 0)
		{
			// right player scores!!
			return TouchDown(1);
		}
	}
	else if (new_x+pw >= right_bat_x)
	{
		float coll_dx, coll_y;

		// test if right bat will be hit
		coll_dx = right_bat_x - ball_x - pw; // distance ball <-> bat
		coll_y = ball_y + (coll_dx/ball_dx*ball_dy); // vert. movement
		if (ball_x+pw <= right_bat_x &&
			coll_y+ph >= right_bat_y &&
			coll_y < right_bat_y+bat_ysize)
		{
			Beep(RBAT_SND, 0.05, 0.9);
			new_ball_x = right_bat_x-pw + (right_bat_x - (ball_x+pw));
			ball_dx = -ball_dx;
			// diff is distance (in pixels) from center of bat
			// and is used to adjust ball_dy a bit :)
			float diff = new_ball_y - right_bat_y - bat_ysize/2;
//			ball_dy += (diff*0.5) / ph;
			ball_dy += (right_bat_dy / 2); // spin-effect
			if (ball_dy > 4.0*ball_speed*ph)
				ball_dy = 4.0*ball_speed*ph;
			else if (ball_dy < -4.0*ball_speed*ph)
				ball_dy = -4.0*ball_speed*ph;
		}
		else if (ball_x+pw <= ball_xspace &&
				new_x+pw > ball_xspace) // right wall hit?
		{
			coll_dx = ball_xspace - (ball_x+pw); // distance ball <-> wall
			coll_y = ball_y + (coll_dx/ball_dx*ball_dy); // vert. movement
			if (game_type == FOOTBALL &&
				(coll_y < football_top ||
				 coll_y+ph >= football_bottom))
			{
				Beep(WALL_SND, 0.05, 0.9);
				new_ball_x = ball_xspace-pw + ball_xspace - (ball_x+pw);
				ball_dx = -ball_dx;
			}
		}
		else if (new_x > ball_xspace)
		{
			// left player scores!!
			return TouchDown(0);
		}
	}

	if (new_y < 0)
	{
		// hit upper wall
		Beep(WALL_SND, 0.05, new_x/width);
		new_ball_y = 0 - (ball_y - 0);
		ball_dy = -ball_dy;
	}
	else if (new_y > ball_yspace)
	{
		// hit lower wall
		Beep(WALL_SND, 0.05, new_x/width);
		new_ball_y = ball_yspace + (ball_yspace - ball_y);
		ball_dy = -ball_dy;
	}

	new_ball_x += ball_dx;
	new_ball_y += ball_dy;

	DrawBall();
	Sync();
	
	return TRUE;
}

void bongView::UpdateLeftBat(void)
{
	key_info keyInfo;
	short joy, range;

	if (players[0])
	{
		if (input_type != KEYBOARD)
		{
			joystick1->Update();
			joy = joystick1->vertical;
			// adjust the range if needed
			if (joy < joy1_min) joy1_min = joy;
			if (joy > joy1_max) joy1_max = joy;
			joy = joy - joy1_min;
			range = joy1_max-joy1_min+1;
			new_left_bat = (range - joy) * bat_space / range;
		}
		else // input from keyboard
		{
			if (get_key_info(&keyInfo) == B_NO_ERROR)
			{
				downkey = keyInfo.modifiers;
				left_bat_dy = 0;
				if (downkey & B_LEFT_SHIFT_KEY) left_bat_dy -= ph;
				if (downkey & B_LEFT_CONTROL_KEY) left_bat_dy += ph;
			}
			new_left_bat += left_bat_dy;
		}
	}
	else if (ball_dx < 0.0)  // if ball is moving to the left
	{
		left_bat_dy = 0;
		if (new_left_bat < ball_y - 2*ph) new_left_bat += ph;
		if (new_left_bat > ball_y - 2*ph) new_left_bat -= ph;

		if (lrand()%10 < 1) new_left_bat += ph*ball_speed;
		if (lrand()%10 < 1) new_left_bat -= ph*ball_speed;

	}
	if (new_left_bat < 0) new_left_bat = 0;
	if (new_left_bat > bat_space) new_left_bat = bat_space;
	left_bat_dy = new_left_bat - left_bat_y;
}

void bongView::UpdateRightBat(void)
{
	key_info keyInfo;
	short joy, range;

	if (players[1])
	{
		if (input_type != KEYBOARD)
		{
			joystick2->Update();
			joy = joystick2->vertical;
			// adjust the range if needed
			if (joy < joy2_min) joy2_min = joy;
			if (joy > joy2_max) joy2_max = joy;
			joy = joy - joy2_min;
			range = joy2_max-joy2_min+1;
			new_right_bat = (range - joy) * bat_space / range;
		}
		else // input from keyboard
		{
			if (get_key_info(&keyInfo) == B_NO_ERROR)
			{
				downkey = keyInfo.modifiers;
				right_bat_dy = 0;
				if (downkey & B_RIGHT_SHIFT_KEY) right_bat_dy -= ph;
				if (downkey & B_RIGHT_OPTION_KEY) right_bat_dy += ph;
			}
			new_right_bat += right_bat_dy;
		}
	}
	else if (ball_dx > 0.0)  // if ball is moving to the right
	{
		right_bat_dy = 0;
		if (new_right_bat < ball_y - 2*ph) new_right_bat += ph;
		if (new_right_bat > ball_y - 2*ph) new_right_bat -= ph;
	
		if (lrand()%10 < 1) new_right_bat += ph*ball_speed;
		if (lrand()%10 < 1) new_right_bat -= ph*ball_speed;
	}

	if (new_right_bat < 0) new_right_bat = 0;
	if (new_right_bat > bat_space) new_right_bat = bat_space;
	right_bat_dy = new_right_bat - right_bat_y;
}

// side is 0 (left player) or 1 (right player)
// return FALSE if game over, TRUE otherwise
bool bongView::TouchDown(int side)
{
	score[side]++;
	DrawScores(TRUE);
	if (score[side] == 15)
	{
		game_over = TRUE;
		game_over_count = 50;
		new_ball_x = -1;
		ball_dx = 0;
		DrawBall();
		return TRUE;
	}
	else
	{
		Beep(OUT_SND, 0.5, side==0?1.0:0.0);
		ball_dx = ball_speed * (1.0 / angle) * pw * 0.5;
		if (side == 0)
			ball_dx = -ball_dx;
		NewBall();
		DrawBall();
	}
	return TRUE;
}

void bongView::GameOverSound(void)
{
	if (game_over_count > 0)
	{
		if (use_samples && samples[OVER_SND].size > 0)
		{
			Beep(OVER_SND, 0.1, 0.5);
			game_over_count = 0;
		}
		else if (beeps < 1)
		{
			Beep(game_over_count, 0.15, 0.5);
			game_over_count *= 2;
			if (game_over_count > 1000) game_over_count = 0;
		}
	}
}

void bongView::Beep(int snd, float sec, float pan)
{
	debug(("bongView::Beep(%d, %1.3f, %1.3f)\n", snd, sec, pan));
	if (snd >= 0 && snd < NUM_SAMPLES)
	{
		if (use_samples == FALSE)
		{
			Beep(samples[snd].freq, samples[snd].time, pan);
		}
		else
		{
			if (snd == LBAT_SND)
			{
				snd += lrand() % NUM_LBAT_SND;
				if (samples[snd].size == 0 || samples[snd].data == NULL)
					snd = LBAT_SND;
			}
			if (snd == RBAT_SND)
			{
				snd += lrand() % NUM_RBAT_SND;
				if (samples[snd].size == 0 || samples[snd].data == NULL)
					snd = RBAT_SND;
			}
			if (snd == WALL_SND)
			{
				snd += lrand() % NUM_WALL_SND;
				if (samples[snd].size == 0 || samples[snd].data == NULL)
					snd = WALL_SND;
			}

			if (samples[snd].size == 0 || samples[snd].data == NULL)
			{
				Beep(samples[snd].freq, samples[snd].time, pan);
			}
			else
			{
				debug(("data=0x%x, size=%d\n",
						samples[snd].data, samples[snd].size));
				samples[snd].samples_left = samples[snd].size;
				samples[snd].curr_data = samples[snd].data;
				samples[snd].panning = pan;
			}
		}
	}
	else
	{
		if (snd > NUM_SAMPLES)
		{
			beeps = (int)(sec*44100.0/4.0);
			half = (44100 / snd) / 2; // #samples per half period
			half /= 4; // adjust 44100 Hz => 11025 Hz
			period = -half;
			panning = pan;
//			debug(("bongView::Beep(%d, %1.3f)  beeps=%d\n", snd, sec, beeps));
		}
	}
}


/**************************************************************/
