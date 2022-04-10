#include "MODPlayer.h"
#include "Paula.h"
#include "DOut.h"



#define	MAX_INSTR	32
#define	MAX_CHANS	4

#define INITGVOL	54		// 0 -> 64
#define INITORD		0




//-----------------------------------------------------------------
// TYPES
//-----------------------------------------------------------------

typedef struct			// Song type - contains info on song
{
	UWORD songlength;	// song length
	UWORD numpats;		// number of physical patterns
	UBYTE numinstr;		// number of instruments
	UBYTE order[256];	// pattern playing orders
	UBYTE patlen[256];	// length of each pattern
	UBYTE channels;		// number of channels
	UBYTE restart;		// restart position
	UBYTE flags;		// flags - see below
	UBYTE playing;		// if song is playing.  0 = no, 1 = yes

} Song;


typedef	struct		// INSTRUMENT
{
	SBYTE*	Address;	// address of sample
	UWORD	Length;		// length of sample in words

	SBYTE*	LoopSta;
	UWORD	LoopLen;

	UBYTE	Volume;
	UWORD	C2Spd;

} tsInstr;



typedef	struct		// CHANNEL
{
// general variables
 UBYTE	mute;		// toggle whether to mute channel or not
 SBYTE	volume;		// current volume of each channel
 SWORD	freq;		// amiga frequency of each channel
 SWORD	panval;		// pan values for each channel
 SBYTE	lastins;	// instrument # for each channel (to remember)
 UBYTE	lastnot;	// last note set in channel (to remember)
 UWORD	lastper;	// last period set in channel (to remember)
 UBYTE	restart;	// flag whether to play sample or not
 SBYTE*	start;		// where to start in sample

// effect variables
 ULONG	soffset;	// amount to sample offset by

 SWORD	porto;		// note to port to value (signed word)
 UBYTE  portsp;		// porta speed

 SBYTE	vibpos;		// vibrato position
 UBYTE  vibspe;		// vibrato speed
 UBYTE  vibdep;		// vibrato depth

 SBYTE	trempos;	// tremolo position
 UBYTE	tremspe;	// tremolo speed
 UBYTE	tremdep;	// tremolo depth

 UBYTE	patlooprow;
 UBYTE	patloopno;	// pattern loop variables for effect  E6x

 UBYTE	wavecon;	// waveform type for vibrato
			// and tremolo (4bits each)

} tsChannel;




typedef struct		// Single note type - contains info on 1 note
{
	UBYTE note;		// note to play at     (0-133) 132=none,133=keyoff
	UBYTE number;	// sample being played (0-99)
	UBYTE effect;	// effect number       (0-1Ah)
	UBYTE eparm;	// effect parameter    (0-255)

} Note;





#define	PERIODSIZ	134
#define NONOTE		(PERIODSIZ-2)
#define KEYOFF		(PERIODSIZ-1)
#define NOVOL		255

void	MOD_UpdateNote(void);
void	MOD_UpdateEffect(UBYTE tick);

// SINE TABLE FOR TREMOLO AND VIBRATO 
// (from protracker so 100% compatible)
UBYTE Sintab[32] = 
{
	   0, 24, 49, 74, 97,120,141,161,
	 180,197,212,224,235,244,250,253,
	 255,253,250,244,235,224,212,197,
	 180,161,141,120, 97, 74, 49, 24
};

// AMIGA TYPE PERIOD TABLE, FOR 11 OCTAVES 
// (octave 9 and 10 are only for .MOD's that use these sort 
// of periods when loading, i.e dope.mod)

UWORD Periodtab[PERIODSIZ] = 
{
  0,

  27392,25856,24384,23040,21696,20480,19328,18240,17216,16256,15360,14496,//0
  13696,12928,12192,11520,10848,10240, 9664, 9120, 8608, 8128, 7680, 7248,//1
   6848, 6464, 6096, 5760, 5424, 5120, 4832, 4560, 4304, 4064, 3840, 3624,//2
   3424, 3232, 3048, 2880, 2712, 2560, 2416, 2280, 2152, 2032, 1920, 1812,//3
   1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016,  960,  906,//4

	856,  808,  762,  720,  678,  640,  604,  570,  538,  508,  480,  453,//5
	428,  404,  381,  360,  339,  320,  302,  285,  269,  254,  240,  226,//6
	214,  202,  190,  180,  170,  160,  151,  143,  135,  127,  120,  113,//7

	107,  101,   95,   90,   85,   80,   75,   71,   67,   63,   60,   56,//8
	 53,   50,   47,   45,   42,   40,   37,   35,   33,   31,   30,   28,//9
	 26,   25,   23,   22,   21,   20,   18,   17,   16,   15,   15,   14,//10

	 0		// <- these last 2 are for no note (132) 
			// and keyoff (133)
};

static	int FineHz[16] = 
{
	8363, 8413, 8463, 8529, 8581, 8651, 8723, 8757, 
	7895, 7941, 7985, 8046, 8107, 8169, 8232, 8280 
};

//-----------------------------------------------------------------
// VARIABLES
//-----------------------------------------------------------------

// PLAY TIME VARIABLES
static	UBYTE pause = 0;
static	UBYTE speed, bpm;	// speed & bpm, figure it out.
static	UBYTE patdelay;		// # of notes to delay pattern for in EEx
static	SBYTE row;			// current row being played
static	SBYTE gvol = INITGVOL;	// master volume
static	UBYTE defpan =0x30;	// default pan value, L=defpan, R=0xFF-defpan
static	SWORD ord = INITORD;	// current order being played

 // PATTERN AND SONG DATA VARIABLES
static	Song		FM;					// the song header information
static	char		*patbuff[256];		// pattern data pointers
static	tsInstr		Instr[MAX_INSTR];
static	tsChannel	Chans[MAX_CHANS];
static	Note		*current;			// current - a global 'note'

static	BOOL	MOD_Opened;
static	BOOL	UsePaula;






//-----------------------------------------------------------------
// FUNCTIONS
//-----------------------------------------------------------------



/*-----------------------------------------------------------------------*/
int FinetoHz(UBYTE ft) 
/*-----------------------------------------------------------------------*/
{
	return FineHz[ft & 15];
}


/*-----------------------------------------------------------------------*/
void	doporta(UBYTE track) 
/*-----------------------------------------------------------------------*/
{
	// slide pitch down if it needs too.
	if (Chans[track].freq < Chans[track].porto) 
	{
		Chans[track].freq += (Chans[track].portsp << 2);
		if (Chans[track].freq > Chans[track].porto) 
			Chans[track].freq = Chans[track].porto;
	}

	// slide pitch up if it needs too.
	if (Chans[track].freq > Chans[track].porto) 
	{
		Chans[track].freq -= (Chans[track].portsp << 2);
		if (Chans[track].freq < Chans[track].porto) 
			Chans[track].freq = Chans[track].porto;
	}

	 //
	 // if (glissando[track]) {
	 //}
	 //
	paulaSetPeriod(track,Chans[track].freq);
}




/*-----------------------------------------------------------------------*/
void	dovibrato(UBYTE track) 
/*-----------------------------------------------------------------------*/
{
register UWORD delta;
register UBYTE temp;

	temp = (Chans[track].vibpos & 31);

	switch(Chans[track].wavecon & 3)
	{
		case 0: delta = Sintab[temp];			// sine
				break;
		case 1: temp <<= 3;						// ramp down
				if (Chans[track].vibpos<0) temp=255-temp;
				delta=temp;
				break;
		case 2: delta = 255;					// square
				break;
		case 3: delta = Sintab[temp];			// random
				break;
	};

	delta *= Chans[track].vibdep;
	delta >>=7;
	delta <<=2;   // we use 4*periods so make vibrato 4 times bigger

	if (Chans[track].vibpos >= 0) 
			paulaSetPeriod(track, Chans[track].freq + delta);
	else	paulaSetPeriod(track, Chans[track].freq - delta);

	Chans[track].vibpos += Chans[track].vibspe;
	if (Chans[track].vibpos > 31) Chans[track].vibpos -= 64;
}




/*-----------------------------------------------------------------------*/
void	dotremolo(UBYTE track) 
/*-----------------------------------------------------------------------*/
{
register UWORD delta;
register UBYTE temp;

	temp = (Chans[track].trempos & 31);

	switch((Chans[track].wavecon >> 4) & 3)
	{
		case 0: delta = Sintab[temp];             // sine
				break;
		case 1: temp <<= 3;                     // ramp down
				if(Chans[track].vibpos < 0) temp=255-temp;
				delta=temp;
				break;
		case 2: delta = 255;                      // square
				break;
		case 3: delta = Sintab[temp];             // random (just use sine)
				break;
	};

	delta *= Chans[track].tremdep;
	delta >>= 6;

	if (Chans[track].trempos >= 0) 
	{
		if (Chans[track].volume + delta > 64) 
			delta = 64 - Chans[track].volume;
		paulaSetVolume(track, (Chans[track].volume + delta) * gvol / 64);
	}
	else {
		if ((WORD)(Chans[track].volume - delta) < 0) 
			delta = Chans[track].volume;
		paulaSetVolume(track, (Chans[track].volume - delta) * gvol / 64);
	}

	Chans[track].trempos += Chans[track].tremspe;
	if (Chans[track].trempos > 31) Chans[track].trempos -= 64;
}





/*-----------------------------------------------------------------------*/
void	MOD_PlaySample(int track)
/*-----------------------------------------------------------------------*/
{
tsChannel *p = &Chans[track];

	paulaSetSamplePos(track, Instr[p->lastins].Address);
	paulaSetSampleLen(track, Instr[p->lastins].Length);
	paulaPlayVoice(track);
	paulaSetSamplePos(track, Instr[p->lastins].LoopSta);
	paulaSetSampleLen(track, Instr[p->lastins].LoopLen);
}




/*-----------------------------------------------------------------------*/
void	MOD_UpdateNote(void)
/*-----------------------------------------------------------------------*/
{
register UBYTE track, eparmx, eparmy;
register UBYTE breakflag=0,jumpflag=0,lastvoice=0;
tsChannel *p;



	current = (Note *)( patbuff[FM.order[ord]] + 
						(sizeof(Note) * row * FM.channels) );

	for (track = 0; track < FM.channels; track++) 
	{
	  p = &Chans[track];	
	  eparmx = current -> eparm >> 4;	// get effect param x
	  eparmy = current -> eparm & 0xF;	// get effect param y

	  // if an INSTRUMENT NUMBER is given
	  if (current->number) 
	  {
		// remember the Instrument #
		p->lastins = current->number-1;

		if (!(current -> effect == 0xE && eparmx == 0xD))
			p->volume = Instr[p->lastins].Volume;
	  }

		// if a NOTE is given

//		if ( (current->note >= 0) && (current->note < NONOTE) )

		if (current->note > 0)
		{
			p->restart = 1;			// retrigger sample

			// get period according to relative note, c2spd and finetune
			p->lastper = 
					8363L * Periodtab[current->note] / 
					Instr[p->lastins].C2Spd;

			p->lastnot = current->note;	// now remember the note
			p->start = Instr[p->lastins].Address;// store sample start
			lastvoice = track;                      // last voiced used

			// retrigger tremolo and vibrato waveforms

			if ((p->wavecon & 0xF) < 4) p->vibpos = 0;
			if ((p->wavecon >> 4) < 4) 	p->trempos = 0;

			// frequency only changes if there are no portamento effects

			if (current->effect != 0x3 && current->effect != 0x5 &&
				current->effect != 0x16)
				p->freq = p->lastper;
		}
		else p->restart = 0;      // else dont retrigger a sample

		// process volume byte
//		if (current->volume <=0x40) p->volume = current->volume;

		// process keyoff note
		if (current->note == KEYOFF) p->volume = 0;

		// TICK 0 EFFECTS

		switch (current -> effect) 
		{
			case 0x0: break;                // dont waste my time in here!!!
			case 0x1: break;                // not processed on tick 0
			case 0x2: break;                // not processed on tick 0

		// Set Volume
			case 0xC: p->volume = current -> eparm;
					  if (p->volume > 64) p->volume = 64;
					  break;

		// Porta to Note
			case 0x3: if (current->eparm) 
						  p->portsp = current->eparm;

		// Porta + Volume Slide
			case 0x5: p->porto = p->lastper;
					  p->restart = 0;
					  break;

		// Vibrato
			case 0x4: if (eparmx) p->vibspe = eparmx;
					  if (eparmy) p->vibdep = eparmy;

		// Vibrato + Volume Slide
			case 0x6: break;		// not processed on tick 0

		// Tremolo
			case 0x7: if (eparmx) p->tremspe = eparmx;
					  if (eparmy) p->tremdep = eparmy;
					  break;

		// Pan
			case 0x8: p->panval = current -> eparm;
//					  GUSSetBalance(track, panval[track]);
					  break;

		// Sample Offset
			case 0x9: if (current->eparm) 
						  p->soffset = current->eparm << 8;
					  if (p->soffset > Instr[p->lastins].Length)
						  p->soffset = Instr[p->lastins].Length;
					  p->start += p->soffset;
					  break;

		// Volume Slide
			case 0xA: break;	// not processed on tick 0

		// Jump To Pattern
			// --- 00 B00 : --- 00 D63 , should put us at ord=0, row=63
			case 0xB: ord = current->eparm;
					  row = -1;
					  if (ord >= FM.songlength) ord=0;
					  jumpflag = 1;
					  break;

		// Pattern Break
			case 0xD: row = (eparmx*10) + eparmy -1;
					  if (row > 63) row = -1;
					  if (!breakflag && !jumpflag) ord++;
					  if (ord >= FM.songlength) ord=0;
					  breakflag = 1;
					  break;

		// Set Speed
			case 0xF: if (current->eparm < 0x20) 
						  speed = current->eparm;
					  else 
						{ 
							bpm = current->eparm; 
//							SetTimerSpeed(); 
						}
					  break;

		// extended PT effects
			case 0xE: 
				switch (eparmx) 
				{
					case 0x1: p->freq -= (eparmy << 2);	break;
					case 0x2: p->freq += (eparmy << 2);	break;
					case 0x3: break;
					case 0x4: p->wavecon &= 0xF0;
							  p->wavecon |= eparmy;		break;

					case 0x5: Instr[p->lastins].C2Spd = 
								  FinetoHz(eparmy);		break;

					case 0x6: if (eparmy == 0) 
									p->patlooprow = row;
							  else 
							  {
							   if (!p->patloopno) 
									p->patloopno = eparmy;
							   else p->patloopno--;

							   if (p->patloopno) 
									row = p->patlooprow-1;
							  } 
							  break;

					case 0x7:	p->wavecon &= 0xF;
								p->wavecon |= (eparmy<<4);
								break;
					case 0x8:	p->panval = (eparmy << 4);
//								GUSSetBalance(track, panval[track]);
								break;
					case 0x9:	break;
					case 0xA:	p->volume += eparmy;
								if (p->volume > 64) p->volume=64;
								break;
					case 0xB:	p->volume -= eparmy;
								if (p->volume < 0) p->volume = 0;
								break;
					case 0xC:	break;  // not processed on tick 0
					case 0xD:	p->restart = 0;	goto nofreq;
					case 0xE:	patdelay = eparmy; break;
				};
				break;

		};

		if (current->effect !=7) 
			paulaSetVolume(track, p->volume * gvol / 64);

		if (p->freq)
			paulaSetPeriod(track, p->freq);
nofreq:
		current++;
	}

	// now play the samples
	for (track=0; track <= lastvoice; track++) 
	{
		if (Chans[track].restart)     
			MOD_PlaySample(track);
	}
}



















/*-----------------------------------------------------------------------*/
void	MOD_UpdateEffect(UBYTE tick)
/*-----------------------------------------------------------------------*/
{
UBYTE track, effect, eparmx, eparmy;

	// rewind back 1 row (as we just incremented the row 
	// in modhandler() function)

	current -= FM.channels;

	for (track=0; track < FM.channels; track++)
	{
		// no freq?, so dont do anything!
		if (!Chans[track].freq) { paulaStopVoice(track); goto skip; }
		
		// grab the effect number
		effect = current -> effect;

		// grab the effect parameter x
		eparmx = current -> eparm >> 4;

		// grab the effect parameter y
		eparmy = current -> eparm & 0xF;


		switch(effect) 
		{
		// Arpeggio
		case 0x0: if (current -> eparm > 0) switch (tick % 3) 
				  {
					case 0: paulaSetPeriod(track, Chans[track].freq);
							break;
					case 1: paulaSetPeriod(track, Periodtab[Chans[track].lastnot+eparmx]);
							break;
					case 2: paulaSetPeriod(track, Periodtab[Chans[track].lastnot+eparmy]);
							break;
				  };
				  break;

		// Porta Up
		case 0x1: 
				Chans[track].freq -= (current -> eparm << 2); // subtract freq
				if (Chans[track].freq < 113) Chans[track].freq = 113;
				paulaSetPeriod(track, Chans[track].freq);
				break;

		// Porta Down
		case 0x2:

				Chans[track].freq += (current -> eparm << 2);
				if (Chans[track].freq > 856) Chans[track].freq = 856;
				paulaSetPeriod(track, Chans[track].freq);
				break;

		// Porta to Note
		case 0x3: doporta(track);
				  break;

		// Vibrato
		case 0x4: dovibrato(track);
				  break;

		// Porta to Note + Volume Slide
		case 0x5: doporta(track);
				  Chans[track].volume += eparmx - eparmy;
				  if (Chans[track].volume < 0) Chans[track].volume = 0;
				  if (Chans[track].volume > 64) Chans[track].volume= 64;
				  paulaSetVolume(track, Chans[track].volume * gvol / 64);
				  break;

		// Vibrato + Volume Slide
		case 0x6: dovibrato(track);
				  Chans[track].volume += eparmx - eparmy;
				  if (Chans[track].volume < 0) Chans[track].volume = 0;
				  if (Chans[track].volume > 64) Chans[track].volume= 64;
				  paulaSetVolume(track, Chans[track].volume * gvol / 64);
				  break;

		// Tremolo
		case 0x7: dotremolo(track);
				  break;

		// Volume Slide
		case 0xA: Chans[track].volume += eparmx - eparmy;
				  if (Chans[track].volume < 0) Chans[track].volume = 0;
				  if (Chans[track].volume > 64) Chans[track].volume= 64;
				  paulaSetVolume(track, Chans[track].volume * gvol / 64);
				  break;

		// extended PT effects
		case 0xE: 
			switch(eparmx) 
			{
				// Note Cut - 0 volume after x ticks
				case 0xC: 
					if (tick==eparmy) 
					{
						Chans[track].volume = 0;
						paulaSetVolume(track, Chans[track].volume);
					}
					break;

				// Retrig Note - retrigger every x ticks
				case 0x9: 
					if (!eparmy) break; // divide by 0 bugfix
					if (!(tick % eparmy)) 
					{
						MOD_PlaySample(track);
					}
					break;

				// Note Delay - wait x ticks then play - quite more complex
				case 0xD:
					if (tick==eparmy)	// than it seems.
					{             
						if (current->number) 
							Chans[track].volume = 
							  Instr[Chans[track].lastins].Volume;
//						if (current->volume <= 0x40)
//							Chans[track].volume = current->volume;

						paulaSetPeriod(track, Chans[track].freq);
						paulaSetVolume(track, Chans[track].volume * gvol / 64);

						MOD_PlaySample(track);
					}
					break;
			  };
			  break;
		};
skip:
		current++;
	}
}



/*---------------------------------------------------------------*/
int	MOD_PLAY_MUSIC(void)
/*---------------------------------------------------------------*/
{
static UBYTE tick=0;

 if ( (!pause) && (FM.playing) )
 {	
	tick++;
	if (tick >= speed) 
	{
		tick = 0;
		if (row >= FM.patlen[FM.order[ord]])
		{ 
		  row = 0;
		  ord++;
		  if (ord >= FM.songlength) ord = FM.restart;
		}

		if (!patdelay) { MOD_UpdateNote(); row++; }
		else patdelay--;
	}
	else MOD_UpdateEffect(tick);
 }
 return 0;
}





/*----------------------------------------------------------------------*/
int		__stdcall MOD_Open(int MixFreq, int BufferSize, int BufferNum)
/*----------------------------------------------------------------------*/
{
	MOD_Opened = FALSE;
	FM.playing = 0;				// Song is now playing
	
	if (MixFreq > 0)
	{
		UsePaula = TRUE;
		paulaOpen(MixFreq, MOD_PLAY_MUSIC, 50, BufferSize, BufferNum);
	}
	else
	{
		UsePaula = FALSE;
	}


	for (int i = 0; i < FM.channels; i++) 
		ZeroMemory(&Chans[i],sizeof(tsChannel));

	row = 0;
	ord = INITORD;
	patdelay = 0;				// notes to delay pattern for EEx


    	MOD_Opened = TRUE;
	return 0;
}



/*-----------------------------------------------------------------------*/
void	__stdcall MOD_Prepare(void)
/*-----------------------------------------------------------------------*/
{
	if (MOD_Opened == FALSE) return;
        
	if (UsePaula) 
	{
		FM.playing = 1;			// Song is now playing
		paulaReset();
		DOut_Prepare();
	}
	
	FM.playing = 0;				// Song is now stopped
}

/*-----------------------------------------------------------------------*/
void	__stdcall MOD_Begin(void)
/*-----------------------------------------------------------------------*/
{
	if (MOD_Opened == FALSE) return;
        
	if (UsePaula) 
	{
		FM.playing = 1;				// Song is now playing
		DOut_Begin();
	}
}


/*-----------------------------------------------------------------------*/
void	__stdcall MOD_Start(void)
/*-----------------------------------------------------------------------*/
{
	MOD_Prepare();
	MOD_Begin();
}

/*-----------------------------------------------------------------------*/
void	__stdcall MOD_Stop(void)
/*-----------------------------------------------------------------------*/
{
	if (MOD_Opened == FALSE) return;
	
	if (UsePaula)
	{
		DOut_Stop();
	}

	FM.playing = 0;				// Song is now stopped
}








/*----------------------------------------------------------------------*/
void	__stdcall MOD_Close(void)
/*----------------------------------------------------------------------*/
{
	if (MOD_Opened == false) return;

	MOD_Opened = FALSE;
	FM.playing = 0;

	if (UsePaula) DOut_Close();

	// free the pattern buffers from memory
	for (int i = 0; i <= FM.numpats; i++) if (patbuff[i]) delete patbuff[i];
}










/*-----------------------------------------------------------------------*/
int		__stdcall MOD_InitModuleCHP(SBYTE *modAddress)
/*-----------------------------------------------------------------------*/
{
int		i;
UBYTE	count, count3, ERR=0;
UWORD	count2;
SBYTE	*p = modAddress,*dp;
UBYTE	SampNum,NoteNum,FxCmd,FxArg,RepVal,Row;

	// set panning values LRRL etc
/*
	for (count = 0; count < 32; count++) 
	{
		switch(count%4) {
			case 0:
			case 3: Chans[count].panval = defpan; break;
		   default: Chans[count].panval = 0xF0 - defpan;
		};
	}
*/

	// set each pattern to have 64 rows
	for (i = 0; i < 256; i++) FM.patlen[i] = 64;

	// set a few default values for this format
	speed = 6;	bpm = 125;	FM.channels = 4;


	FM.restart  = (UBYTE)(p[6]);
	FM.numpats  = (UWORD)(p[7]);
	FM.numinstr = (UBYTE)(p[14]);
	FM.songlength = (UBYTE)(p[15]);
	p += 16;


	for (count = 0; count < FM.songlength; count++) 
		FM.order[count] = *p++;



	// load instrument headers

	for (count = 0; count < FM.numinstr; count++) 
	{

		// read rest of information.
		Instr[count].Length = ((UWORD*)p)[0];
		Instr[count].C2Spd  = FinetoHz(p[2]);
		Instr[count].Volume = p[3];
		Instr[count].LoopSta = (SBYTE*) ((UWORD*)p)[4];
		Instr[count].LoopLen = ((UWORD*)p)[6];

		if (Instr[count].LoopLen < 1) Instr[count].LoopLen = 1;

		p += 8;
	}



	
	dp = p;

	// load pattern data
	for (count = 0; count <= FM.numpats; count++) 
	{
		// allocate memory for 1 pattern
		patbuff[count] = new char [FM.channels *64 *sizeof(Note)];
		if (patbuff[count] == NULL)
		{
			while (count > 0) {	count--; delete patbuff[count]; }
			return 1;
		}


		for (count2 = 0; count2 < FM.channels; count2++)
		{

			// point our little note structure to patbuff
			current = (Note *)patbuff[count];
			current += count2;

			for (Row = 0; Row < 64; )
			{

				if ((dp[0] & 0xC0) != 0xC0)
				{
					SampNum = (dp[0] >> 1) & 0x1F;
					NoteNum = ((dp[1] >> 4) & 0x0F) | ((dp[0] << 4) & 0x10);
					FxCmd	= dp[1] & 0x0F;
					FxArg	= dp[2];

					current->note = NoteNum;
					current->number = SampNum;
					current->effect = FxCmd;
					current->eparm = FxArg;

					current += 4;
					Row++;
					dp += 3;
				}
				else
				{
					RepVal = *dp++ & 0x3F;

					for (count3 = 0; count3 < RepVal; count3++)
					{
						current->note = NoteNum;
						current->number = SampNum;
						current->effect = FxCmd;
						current->eparm = FxArg;

						current += 4;
						Row++;
					}
				}
			}

		}
	}
	

	p = modAddress;
	p += ((ULONG*)p)[2];

	for (count = 0; count < FM.numinstr; count++)
	{
		if (!Instr[count].Length)
		{
			Instr[count].Address = 0;
		}
		else
		{
			Instr[count].Address = p;
			Instr[count].LoopSta += (ULONG)p;
			p += Instr[count].Length*2;
		}
	}
	

	MOD_Opened = true;
	return 0;
}
