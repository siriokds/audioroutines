;----------------------------------------------------------
; Delta Music 2.0 Replayer
; coded by:
;
; BENT NIELSEN   
; KYRADSERVEJ 19 B, 8700 HORSENS,   
; DENMARK,   TLF. 75-601-868
; COPYRIGHT 1990
;----------------------------------------------------------
; Resourced by SiRioKD
; Latest update : 25/02/2000
;----------------------------------------------------------



;---------------------------------------------------------------------------------------
; Every DeltaMusic 2.0 module have 
; the relative playroutine at TOP, 
; and then the REAL module.
;
; Delta Music 2.0 Module Structure
;---------------------------------------------------------------------------------------
; Offset 	Length  	Name
;---------------------------------------------------------------------------------------
; $0000 	 $0BC6  	PlayRoutine
; $0BC6 	     4  	'.FNL' (DeltaMusic 2.0 FINAL version of module!)
; $0BCA 	 $0400  	Arpeggio Values (NumOfTableMax * SizeOfTable => 64*16 = $0400)
; $0FCA		     2		Track1Data * 2
; $0FCC 	     2  	Track1Len
; $0FCE		     2		Track2Data * 2
; $0FD0 	     2  	Track2Len
; $0FD2		     2		Track3Data * 2
; $0FD4 	     2  	Track3Len
; $0FD6		     2		Track4Data * 2
; $0FD8 	     2  	Track4Len
; $0FDA 	 Track1Len	Track1Data
; ADDR1 	 Track2Len	Track2Data	(ADDR1 = $0FD8 + Track1Len)
; ADDR2 	 Track3Len	Track3Data	(ADDR2 = ADDR1 + Track2Len)
; ADDR3 	 Track4Len	Track4Data	(ADDR3 = ADDR2 + Track3Len)
; ADDR4		     4		PATsize		(ADDR4 = ADDR3 + Track4Len)
; ADDR5		 PATsize	Patterns	(ADDR5 = ADDR4 + 4)
; ADDR6		   256		InstrTableOfs	(ADDR6 = ADDR5 + PATsize)
; ADDR7						(ADDR7 = ADDR6 + 256)
;---------------------------------------------------------------------------------------

;---------------------------------------------------------------------------------------
; Instrument Structure (88 bytes)
;---------------------------------------------------------------------------------------
; Offset 	Length  	Name
;---------------------------------------------------------------------------------------
; $0000 	2		Wave/Sample Size (n. of words)
; $0002 	2		Repeat Start  (n. of words)
; $0004		2		Repeat Length (n. of words)
; $0006 	15		Volume Table
; $0015 	15		Vibrato Table
; $0024 	2		BendRate
; $0026 	1		Instrument Type
; $0027 	48		WaveTable
; $0057 	1		(Word Align Filler)
;---------------------------------------------------------------------------------------

; ////  hardware  \\\\
h_sound     		= 0
h_length    		= 4
h_period		= 6
h_volume    		= 8


; ////  instrument  \\\\
i_sample_size		= $00
i_repsta		= $02
i_replen		= $04
i_volume_table		= $06
i_vibrato_table		= $15
i_bendrate		= $24
i_type			= $26
i_wavetable		= $27


; ////  channel  \\\\
c_hardware         	= $00
c_dmacon           	= $04
c_instr_data     	= $06
c_track_data       	= $0A
c_block_data       	= $0E
c_track_pos        	= $12
c_block_pos        	= $14
c_sample_num       	= $16
c_waveform_tabpos  	= $17
c_period_transp    	= $18
c_period	   	= $1A
c_note		   	= $1C
c_chn_volume       	= $1D
c_chn_bendrate     	= $1E
c_volume           	= $20
c_volume_tabpos    	= $22
c_volume_sustain   	= $24
c_porta_speed      	= $25
c_boh		   	= $26
c_vibrato_dir     	= $27
c_final_bendrate   	= $28
c_vibrato_len     	= $2A
c_note_transp      	= $2B
c_arpeggio_data	   	= $2C
c_arpeggio_tabpos  	= $30
c_playmode 	   	= $32
c_instr_type 	   	= $33
c_vibrato_tabpos  	= $34
c_vibrato_sust    	= $35
c_track_len	   	= $36
c_track_end	   	= $38








;-------------------------------------	
DM_Command
;-------------------------------------	
	and.b	#$03,d0
	
	tst.b	d0
	beq	DM_play
	
	cmp.b	#$01,d0
	beq	DM_Init
	
	cmp.b	#$02,d0
	beq.s	DM_SongVolume
	
	cmp.b	#$03,d0
	beq.s	DM_PlaySoundFX
	
	moveq	#$00,d0
	rts


;-------------------------------------------------------------------------------
DM_SongVolume
;-------------------------------------------------------------------------------
; d1 = (0-63)	 The new song volume want

	and.b	#$3F,d1
	move.b	d1,SongVolume
	rts



;-------------------------------------------------------------------------------
DM_PlaySoundFX
;-------------------------------------------------------------------------------
; d1 = (0-3) 	 Which channel to play sound fx in.
; d2 = (0-127)	 Which instrument to use as fx.
; d3 = (1-72)	 Which note to play the fx with.
; d4 = (0-65535) How long to hold the sound FX.
; d5 = (0-63)	 Which arpeggio effect to use on the FX

	and.l	#127,d2				; instrument number (0-127)
	and.w	#127,d3				; note (1-72)
	and.l	#63,d5				; arpeggio effect
	
	
	tst.b	d1				; channel (0-3)
	bne.s	lbC00005A
	move.w	d4,Track1LenCnt
	lea	channel1_A,a0			; channel #0
	clr.b	c_playmode(a0)			; play mode = soundfx
	lea	channel1_B,a0
	bra.s	lbC00009A

lbC00005A
	cmp.b	#$01,d1
	bne.s	lbC000072
	move.w	d4,Track2LenCnt
	lea	channel2_A,a0			; channel #1
	clr.b	c_playmode(a0)			; play mode = soundfx
	lea	channel2_B,a0
	bra.s	lbC00009A

lbC000072
	cmp.b	#$01,d1			
	bne.s	lbC00008A
	move.w	d4,Track3LenCnt
	lea	channel3_A,a0			; channel #2
	clr.b	c_playmode(a0)			; play mode = soundfx
	lea	channel3_B,a0
	bra.s	lbC00009A

lbC00008A
	move.w	d4,Track4LenCnt
	lea	channel4_A,a0			; channel #3
	clr.b	c_playmode(a0)			; play mode = soundfx
	lea	channel4_B,a0
	
	
	
lbC00009A
	move.l	c_hardware(a0),a1		; dff0a0, dff0b0...
	move.w	c_dmacon(a0),$00DFF096
	
	move.b	d3,c_note(a0)			; note
	lea	Periods,a4			; period table
	add.w	d3,d3
	move.w	$00(a4,d3.w),c_period(a0)	; period
	subq.w	#$01,d2				; instrument
	add.l	d2,d2
	move.l	InstrTableOfs,a4		; instrument offset table
	add.l	d2,a4
	moveq	#$00,d2
	move.w	(a4),d2				; relative instrument address
	add.l	InstrTable,d2
	move.l	d2,c_instr_data(a0)		; instr info address
	move.l	d2,a2
	move.b	i_type(a2),c_instr_type(a0)	; synth ?
	bpl.s	lbC000106			; yes
	moveq	#$00,d0
	move.l	SampleInfo,a4			; sample offset table
	move.b	i_wavetable(a2),d0		; instrument sample number	
	and.b	#$07,d0
	asl.l	#$02,d0
	add.l	d0,a4				; pos in table
	move.l	(a4),a4				; relative address
	add.l	SampleBaseAddr,a4		; sample address
	move.l	a4,h_sound(a1)			; A0 register
	move.w	i_sample_size(a2),h_length(a1)	; A4 register = len
	move.w	c_dmacon(a0),d0			; channel bit
	or.w	#$8000,d0	
	move.w	d0,$00DFF096			; set dmacon

lbC000106
	move.l	ModAddress,a4			; arpeggio table
	asl.w	#$04,d5				; *16
	add.l	d5,a4
	move.l	a4,c_arpeggio_data(a0)		; chosen arpeggio
	clr.b	c_sample_num(a0)		; sample number
	clr.b	c_waveform_tabpos(a0)		; waveform tab pos		
	clr.w	c_volume(a0)			; volume
	clr.b	c_volume_sustain(a0)		; volume sustain
	clr.w	c_volume_tabpos(a0)		; volume tab pos
	clr.w	c_arpeggio_tabpos(a0)		; arpeggio tab pos
	clr.b	c_boh(a0)			; ???
	clr.b	c_vibrato_dir(a0)		; vibrator dir
	clr.w	c_final_bendrate(a0)		; final bendrate
	move.b	$0016(a2),c_vibrato_len(a0)	; vibrato len
	clr.b	c_vibrato_tabpos(a0)		; vibrator tab pos
	move.b	$0017(a2),c_vibrato_sust(a0)	; vibrato sustain
	rts











;	dc.b	'THIS PIECE OF MUSIC, WAS CREATED ON DELTA MUSIC V2.0'
;	dc.b	' ...  CODED BY :  BENT NIELSEN,   KYRADSERVEJ 19 B, '
;	dc.b	'  8700 HORSENS,   DENMARK,   TLF. 75-601-868 ..... C'
;	dc.b	'OPYRIGHT 1990 .......  ',0





;-------------------------------------------------------------------------------
DM_Init
;-------------------------------------------------------------------------------
;	bset	#$01,$00BFE001			;led/filter off

	move.l	ModAddress,a0
	lea	$0400(a0),a0
	
	clr.w	Track1LenCnt
	clr.w	Track2LenCnt
	clr.w	Track3LenCnt
	clr.w	Track4LenCnt
	
	move.l	a0,a2
	
	lea	Track1Len,a1			; copy tracks lengths here
	moveq	#$07,d7				; 8 words
lbC00021C
	move.w	(a0)+,(a1)+
	dbra	d7,lbC00021C
	
	
	move.l	a0,Track1Data			; track1 adr
	moveq	#$00,d0
	move.w	$0002(a2),d0			; track1 len
	add.l	d0,a0
	
	move.l	a0,Track2Data			; track2 adr
	moveq	#$00,d0
	move.w	$0006(a2),d0			; track2 len
	add.l	d0,a0
	
	move.l	a0,Track3Data			; track3 adr
	moveq	#$00,d0
	move.w	$000A(a2),d0			; track3 len
	add.l	d0,a0
	
	move.l	a0,Track4Data			; track4 adr
	moveq	#$00,d0
	move.w	$000E(a2),d0			; track4 len
	add.l	d0,a0

	
	move.l	(a0)+,d0
	move.l	a0,PATinfo			; first block adr
	add.l	d0,a0
	move.l	a0,InstrTableOfs		; instr offset table
	add.l	#$000000FE,a0
	moveq	#$00,d0
	move.w	(a0)+,d0			; highest offset
	move.l	a0,InstrTable			; first instr info
	add.l	d0,a0
	move.l	(a0)+,d0
	move.l	a0,NoiseAddr			; first synth adr
	add.l	d0,a0
	move.l	a0,SampleInfo			; sample offset table
	add.l	#64,SampleInfo
	add.l	#96,a0
	move.l	a0,SampleBaseAddr		; sample base adr 
	
	
	lea	$DFF000,a5		
	move.w	#$000F,$0096(a5)		; dmacon
	
	move.w	#$0001,$00A4(a5)		; len
	move.w	#$0001,$00B4(a5)
	move.w	#$0001,$00C4(a5)
	move.w	#$0001,$00D4(a5)
	
	move.w	#$0000,$00A8(a5)		; vol
	move.w	#$0000,$00B8(a5)
	move.w	#$0000,$00C8(a5)
	move.w	#$0000,$00D8(a5)
	
	
	
	lea	channel1_A,a0			; voice1 data
	move.l	Track1Data,c_track_data(a0)	; track1 adr
	move.l	Track1Len,c_track_len(a0)	; track1 len
	bsr	InitVoice
	
	lea	channel2_A,a0			; voice2 data
	move.l	Track2Data,c_track_data(a0)	; track2 adr
	move.l	Track2Len,c_track_len(a0)	; track2 len
	bsr	InitVoice
	
	lea	channel3_A,a0			; voice3 data
	move.l	Track3Data,c_track_data(a0)	; track3 adr
	move.l	Track3Len,c_track_len(a0)	; track3 len
	bsr	InitVoice
	
	lea	channel4_A,a0			; voice4 data
	move.l	Track4Data,c_track_data(a0)	; track4 adr
	move.l	Track4Len,c_track_len(a0)	; track4 len
	bsr	InitVoice
	

	
	
	lea	channel1_B,a0
	move.l	Track1Data,c_track_data(a0)
	move.l	Track1Len,c_track_len(a0)
	bsr	InitVoice
	
	lea	channel2_B,a0
	move.l	Track2Data,c_track_data(a0)
	move.l	Track2Len,c_track_len(a0)
	bsr	InitVoice
	
	lea	channel3_B,a0
	move.l	Track3Data,c_track_data(a0)
	move.l	Track3Len,c_track_len(a0)
	bsr	InitVoice
	
	lea	channel4_B,a0
	move.l	Track4Data,c_track_data(a0)
	move.l	Track4Len,c_track_len(a0)
	bsr	InitVoice

	
	
	move.b	#5,REPspd
	move.b	#1,REPcnt
	rts





InitVoice:
	clr.w	c_block_pos(a0)			; block pos
	clr.w	c_track_pos(a0)			; track pos
	move.l	InstrTable,c_instr_data(a0)	; first instr
	clr.b	c_instr_type(a0)
	clr.w	c_arpeggio_tabpos(a0)		; arpeggio tab pos
	move.l	ModAddress,a3
	move.l	a3,c_arpeggio_data(a0)		; first arp
	clr.w	c_volume(a0)			; volume
	clr.w	c_volume_tabpos(a0)		; volume tab pos
	clr.b	c_volume_sustain(a0)		; volume sustain
	clr.b	c_porta_speed(a0)		; porta speed
	clr.w	c_chn_bendrate(a0)		; channel bendrate
	move.b	#$3F,c_chn_volume(a0)		; channel volume
	move.b	#$01,c_playmode(a0)		; play mode (1 = normal)
	rts











;-------------------------------------------------------------------------------
DM_play
;-------------------------------------------------------------------------------
	movem.l	d0-d7/a0-a6,-(sp)	; push all register!!!
	
	clr.b	MasterDMA

	
	move.l	NoiseAddr,a3
	move.l	RandomSeed,d1
	moveq	#$15,d0			; 64 random samples = 16*4
RandomGen:
	rol.l	#$07,d1
	add.l	#$6ECA756D,d1
	eor.l	#$9E59A92B,d1
	move.l	d1,(a3)+
	dbra	d0,RandomGen
	
	move.l	d1,RandomSeed

	
	
	subq.b	#$01,REPcnt
	bpl.s	@@skipRepReset
	move.b	REPspd,REPcnt		; speed counter
@@skipRepReset:




	clr.b	PlayMode		; play mode = normal
	
	lea	channel1_A,a0
	bsr	PlayVoice
	
	lea	channel2_A,a0
	bsr	PlayVoice
	
	lea	channel3_A,a0
	bsr	PlayVoice
	
	lea	channel4_A,a0
	bsr	PlayVoice
	
	move.b	#$01,PlayMode		; play mode = same track pos, same block pos




	tst.w	Track1LenCnt		; track1 len counter
	beq.s	lbC000432
	
	subq.w	#$01,Track1LenCnt
	lea	channel1_B,a0		; voice1 data
	bsr.s	PlayVoice
	bra.s	lbC00043C
lbC000432
	lea	channel1_A,a0
	move.b	#$01,c_playmode(a0)	; music mode = normal
lbC00043C




	tst.w	Track2LenCnt		; track2 len counter
	beq.s	lbC00044E
	subq.w	#$01,Track2LenCnt
	lea	channel2_B,a0		; voice2 data
	bsr.s	PlayVoice
	bra.s	lbC000458
lbC00044E
	lea	channel2_A,a0
	move.b	#$01,c_playmode(a0)	; music mode = normal
lbC000458




	tst.w	Track3LenCnt		; track3 len counter
	beq.s	lbC00046A
	subq.w	#$01,Track3LenCnt
	lea	channel3_B,a0		; voice3 data
	bsr.s	PlayVoice
	bra.s	lbC000474
lbC00046A
	lea	channel3_A,a0
	move.b	#$01,c_playmode(a0)	; music mode = normal
lbC000474





	tst.w	Track4LenCnt		; track4 len counter
	beq.s	lbC000486
	subq.w	#$01,Track4LenCnt
	lea	channel4_B,a0		; voice4 data
	bsr.s	PlayVoice
	bra.s	lbC000490
lbC000486
	lea	channel3_A,a0
	move.b	#$01,c_playmode(a0)	; music mode = normal
lbC000490




	move.w	MasterDMA,$00DFF096


	movem.l	(sp)+,d0-d7/a0-a6	; pop all register!!!
	rts








;-------------------------------------------------------------------------------
PlayVoice:
;-------------------------------------------------------------------------------
	move.l	c_hardware(a0),a1		; dff0a0, dff0b0...
	move.l	c_instr_data(a0),a2		; instr info
	tst.b	c_instr_type(a0)		; synth ?
	bpl.s	.soundfx2			; yes
	
	clr.b	c_instr_type(a0)		; set to sample
	moveq	#$00,d0
	move.l	SampleInfo,a3			; sample offset table
	move.b	i_wavetable(a2),d0		; instrument sample number
	and.b	#$07,d0
	asl.l	#$02,d0				; *4
	add.l	d0,a3
	move.l	(a3),a4				; relative adr
	add.l	SampleBaseAddr,a4		; absolute adr
	
	moveq	#$00,d0
	tst.b	c_playmode(a0)			; replay mode == soundfx ?
	beq.s	.soundfx			; yes
	move.w	i_replen(a2),h_length(a1)	; sample len
.soundfx


	move.w	i_repsta(a2),d0			; sample repeat
	clr.w	(a4)
	add.l	d0,a4
	tst.b	c_playmode(a0)			; replay mode == soundfx ?
	beq.s	.soundfx2			; yes
	move.l	a4,h_sound(a1)			; sample adr
	
	
.soundfx2
	tst.b	PlayMode			; play mode = same track pos and same block pos ?
	bne	.PlaySame			; yes
	tst.b	REPcnt				; speed counter > 0
	bne	.PlaySame			; no
	
	tst.w	c_block_pos(a0)			; yes : pos in block == 0 ?
	bne.s	.noPosInBlock			; no
	
	moveq	#$00,d0
	move.l	c_track_data(a0),a3		; track adr
	move.w	c_track_pos(a0),d1		; pos in track
	move.b	$00(a3,d1.w),d0			; block
	move.b	$01(a3,d1.w),c_note_transp(a0)	; note transpose
	asl.l	#$06,d0				; *64 (block len)
	add.l	PATinfo,d0			; first block adr
	move.l	d0,c_block_data(a0)		; current block adr
	addq.w	#$02,d1
	move.w	d1,c_track_pos(a0)		; next pos in track
	
	cmp.w	c_track_end(a0),d1		; end of track ?
	bmi.s	.noPosInBlock
	move.w	c_track_len(a0),c_track_pos(a0)	; track pos = track len : track ended
	
.noPosInBlock
	moveq	#$00,d0
	move.l	c_block_data(a0),a3		; current block
	move.w	c_block_pos(a0),d0		; pos in block
	add.l	d0,a3				; command adr
	moveq	#$00,d0			
	move.b	$0000(a3),d0			; note
	beq	.noNewNote			; no new note
	tst.b	c_playmode(a0)
	beq.s	.soundfx3			; music mode = soundfx
	move.w	c_dmacon(a0),$00DFF096		; dmacon
.soundfx3
	move.b	d0,c_note(a0)			; note
	lea	Periods,a4			; period table
	add.b	c_note_transp(a0),d0		; note transposition
	add.w	d0,d0
	move.w	$00(a4,d0.w),c_period(a0)	; period
	moveq	#$00,d1
	move.b	$0001(a3),d1			; instrument
	add.l	d1,d1
	subq.w	#$02,d1
	move.l	InstrTableOfs,a4		; instr offset table
	add.l	d1,a4
	moveq	#$00,d2
	move.w	(a4),d2				; offset
	add.l	InstrTable,d2			; instr info adr 
	move.l	d2,c_instr_data(a0)		; current instr info
	move.l	d2,a2
	move.b	i_type(a2),c_instr_type(a0)	; is synth ?
	bpl.s	.playSynth			; yes
	tst.b	c_playmode(a0)
	beq.s	.playSynth			; play mode=soundfx
	moveq	#$00,d0
	move.l	SampleInfo,a4
	move.b	i_wavetable(a2),d0		; sample number
	and.b	#$07,d0
	asl.l	#$02,d0
	add.l	d0,a4
	move.l	(a4),a4
	add.l	SampleBaseAddr,a4		; sample base adr
	move.l	a4,h_sound(a1)			; sample adr
	move.w	i_sample_size(a2),h_length(a1)	; sample len
.playSynth
	clr.b	c_sample_num(a0)		; sample number
	clr.b	c_waveform_tabpos(a0)		; waveform tab pos
	clr.w	c_volume(a0)			; volume
	clr.b	c_volume_sustain(a0)		; volume sustain
	clr.w	c_volume_tabpos(a0)		; volume tab pos
	clr.w	c_arpeggio_tabpos(a0)		; arpeggio tab pos
	clr.b	c_boh(a0)			; ???
	clr.b	c_vibrato_dir(a0)		; vibrator dir
	clr.w	c_final_bendrate(a0)		; final bendrate
	move.b	$0016(a2),c_vibrato_len(a0)	; vibrato len
	clr.b	c_vibrato_tabpos(a0)		; vibrator tab pos
	move.b	$0017(a2),c_vibrato_sust(a0)	; vibrato sust
.noNewNote
	move.b	$0002(a3),d0			; effect number
	subq.b	#$01,d0
	bmi.s	.noEffect
	move.b	$0003(a3),d1			; effect data
	lea	SetPlaySpeed,a3			; effect base address
	lea	effect_table,a4			; effect offset table
	and.l	#7,d0
	asl.l	#$02,d0
	add.l	$00(a4,d0.w),a3
	jsr	(a3)				; jump to effect routine
.noEffect
	addq.w	#$04,c_block_pos(a0)		; next block pos (every row = 4 bytes)
	and.w	#$003F,c_block_pos(a0)		; modulo 64 (16 row x block)


.PlaySame
	tst.b	i_type(a2)
	bmi.s	DM_vibrato_handler			; is sample...
	tst.b	c_sample_num(a0)			; sample number
	beq.s	.noSampleNum
	subq.b	#$01,c_sample_num(a0)
	bra.s	DM_vibrato_handler

.noSampleNum
	move.b	i_wavetable(a2),c_sample_num(a0)	; sample number
	moveq	#0,d0
	moveq	#0,d2
	lea	$0028(a2),a3				; waveform table
	move.b	c_waveform_tabpos(a0),d0		; pos in waveform table
.checkWTend
	move.b	d0,d1
	move.b	$00(a3,d0.w),d2
	cmp.b	#$FF,d2				; loop (in waveform table) ?
	bne.s	.noWTloop
	move.b	$01(a3,d0.w),d0			; next waveform
	move.b	$00(a3,d0.w),d2
	cmp.b	#$FF,d2
	bne.s	.checkWTend			; loop
	bra.s	DM_vibrato_handler

.noWTloop
	asl.l	#$08,d2				; *128 (synth len)
	add.l	NoiseAddr,d2			; Synth Base Address
	tst.b	c_playmode(a0)		
	beq.s	.soundfx4			; music mode = soundfx
	move.l	d2,h_sound(a1)			; sample adr
	move.w	i_sample_size(a2),h_length(a1)	; sample len
.soundfx4
	addq.b	#$01,d1
	and.b	#$3F,d1
	move.b	d1,c_waveform_tabpos(a0)	; next waveform pos




;-------------------------------------	
DM_vibrato_handler:
;-------------------------------------	
	moveq	#$00,d0
	lea	i_vibrato_table(a2),a3		; vibrator table
	move.b	c_vibrato_tabpos(a0),d0		; vibrator pos
	add.l	d0,a3
	move.b	$0000(a3),d0			; vibrator step
	
	
	tst.b	c_vibrato_dir(a0)		; vib direction
	bne.s	.vibraDown
	add.w	d0,c_final_bendrate(a0)		; +
	bra.s	.vibraUp
.vibraDown
	sub.w	d0,c_final_bendrate(a0)		; -
.vibraUp


	subq.b	#$01,c_vibrato_len(a0)		; vibrator len
	bne.s	.noChgDir
	move.b	$0001(a3),c_vibrato_len(a0)	; vibrator len
	not.b	c_vibrato_dir(a0)		; change direction
.noChgDir

	tst.b	c_vibrato_sust(a0)		; vibrator sustain
	beq.s	.doVibSust
	subq.b	#$01,c_vibrato_sust(a0)
	bra.s	DM_volume_handler

.doVibSust
	addq.b	#$03,c_vibrato_tabpos(a0)	; next vibrator pos
	cmp.b	#$0F,c_vibrato_tabpos(a0)
	bne.s	.noVTend
	move.b	#12,c_vibrato_tabpos(a0)	; vibrator end
.noVTend
	move.b	$0005(a3),c_vibrato_sust(a0)	; vibrator sustain




;-------------------------------------	
DM_volume_handler:
;-------------------------------------	
	tst.b	c_volume_sustain(a0)		; Volume sustain
	beq.s	.noVolSust
	subq.b	#$01,c_volume_sustain(a0)
	bra.s	DM_portamento_handler

.noVolSust
	moveq	#$00,d0
	moveq	#$00,d1
	move.w	c_volume_tabpos(a0),d0		; pos in vol table
	lea	i_volume_table(a2),a3		; vol table
	add.l	d0,a3
	move.b	(a3),d0				; vol rate
	move.b	$0001(a3),d1			; vol level
	cmp.w	c_volume(a0),d1			; vol = target vol?
	bpl.s	.addVol
	sub.w	d0,c_volume(a0)			; vol-
	cmp.w	c_volume(a0),d1
	bmi.s	DM_portamento_handler
	move.w	d1,c_volume(a0)			; yes
	addq.w	#$03,c_volume_tabpos(a0)	; next vol pos
	move.b	$0002(a3),c_volume_sustain(a0)	; vol sustain
	bra.s	DM_portamento_handler

.addVol
	add.w	d0,c_volume(a0)			; vol+
	cmp.w	c_volume(a0),d1			; vol = target vol?
	bpl.s	DM_portamento_handler
	move.w	d1,c_volume(a0)		
	addq.w	#$03,c_volume_tabpos(a0)	; next vol pos 
	
	cmp.w	#15,c_volume_tabpos(a0)
	bne.s	.noVLend
	
	move.w	#12,c_volume_tabpos(a0)		; vol table end
.noVLend
	move.b	$0002(a3),c_volume_sustain(a0)	; vol sustain





;-------------------------------------	
DM_portamento_handler
;-------------------------------------	
	moveq	#$00,d0
	move.b	c_porta_speed(a0),d0		; portamento speed
	beq.s	DM_arpeggio_handler
	move.w	c_period(a0),d1			; period
	cmp.w	c_period_transp(a0),d1		; transposed period
	bpl.s	.noPerTran		
	
	sub.w	d0,c_period_transp(a0)		; period-
	cmp.w	c_period_transp(a0),d1
	bmi.s	DM_arpeggio_handler
	move.w	d1,c_period_transp(a0)		; resulting period
	bra.s	DM_arpeggio_handler

.noPerTran
	add.w	d0,c_period_transp(a0)		; period+
	cmp.w	c_period_transp(a0),d1
	bpl.s	DM_arpeggio_handler
	move.w	d1,c_period_transp(a0)		; resulting period



	
	
;-------------------------------------	
DM_arpeggio_handler
;-------------------------------------	
	moveq	#$00,d0
	move.l	c_arpeggio_data(a0),a3		; arpeggio table
	move.w	c_arpeggio_tabpos(a0),d1	; arpeggio pos
	move.b	$00(a3,d1.w),d0
	tst.b	d1
	beq.s	lbC00077A
	cmp.b	#$80,d0				; repeat ?
	bne.s	lbC00077A
	clr.w	c_arpeggio_tabpos(a0)		; yes
	bra.s	DM_arpeggio_handler

lbC00077A
	addq.w	#$01,c_arpeggio_tabpos(a0)	; next arp pos
	and.w	#$000F,c_arpeggio_tabpos(a0)	; end pos
	tst.b	c_porta_speed(a0)
	beq.s	lbC000790			; portamento = 0
	move.w	c_period_transp(a0),d0
	bra.s	DM_bendrate_handler

lbC000790
	lea	Periods,a3
	add.b	c_note(a0),d0			; note
	add.b	c_note_transp(a0),d0		; note transpose
	add.w	d0,d0
	move.w	$00(a3,d0.w),d0
	move.w	d0,c_period_transp(a0)		; period
	
	
;-------------------------------------	
DM_bendrate_handler
;-------------------------------------	
	move.w	i_bendrate(a2),d1		; instr bendrate
	sub.w	c_chn_bendrate(a0),d1		; chann bendrate
	sub.w	d1,c_final_bendrate(a0)		; final bendrate
	add.w	c_final_bendrate(a0),d0		; modify period
	tst.b	c_playmode(a0)
	beq.s	lbC0007C0
	move.w	d0,h_period(a1)			; period register
lbC0007C0
	move.w	c_volume(a0),d0			; vol / 4
	ror.w	#$02,d0
	and.w	#$003F,d0
	cmp.b	c_chn_volume(a0),d0		; channel vol
	bmi.s	lbC0007D4
	move.b	c_chn_volume(a0),d0
lbC0007D4
	cmp.b	SongVolume,d0			; song volume
	bmi.s	lbC0007DE
	move.b	SongVolume,d0
lbC0007DE
	tst.b	c_playmode(a0)			; play mode
	beq.s	lbC0007F0			; = soundfx
	move.w	d0,h_volume(a1)			; vol register
	move.b	c_dmacon+1(a0),d0
	or.b	d0,MasterDMA			; dma audio bits
lbC0007F0
	rts





;-------------------------------------	
SetPlaySpeed
;-------------------------------------	
	and.b	#15,d1
	move.b	d1,REPspd
	rts


;-------------------------------------	
SetHWFilter
;-------------------------------------	
	tst.b	d1
	bne.s	led_on
	bset	#$01,$00BFE001
	rts
led_on	bclr	#$01,$00BFE001
	rts


;-------------------------------------	
SetBendRateUP
;-------------------------------------	
	and.w	#$00FF,d1
	neg.w	d1
	move.w	d1,c_chn_bendrate(a0)
	rts

;-------------------------------------	
SetBendRateDN
;-------------------------------------	
	and.w	#$00FF,d1
	move.w	d1,c_chn_bendrate(a0)
	rts

;-------------------------------------	
SetPortamento
;-------------------------------------	
	move.b	d1,c_porta_speed(a0)
	rts

;-------------------------------------	
SetChnVolume
;-------------------------------------	
	and.b	#$3F,d1
	move.b	d1,c_chn_volume(a0)
	rts

;-------------------------------------	
SetSongVolume
;-------------------------------------	
	and.b	#$3F,d1
	move.b	d1,SongVolume
	rts

;-------------------------------------	
SetChnArpeggio
;-------------------------------------	
	and.l	#$0000003F,d1
	asl.l	#$04,d1
	move.l	ModAddress,a4
	add.l	d1,a4
	move.l	a4,c_arpeggio_data(a0)
	rts


;-------------------------------------	
effect_table
;-------------------------------------	
	dc.l	SetPlaySpeed
	dc.l	SetHWFilter
	dc.l	SetBendRateUP
	dc.l	SetBendRateDN
	dc.l	SetPortamento
	dc.l	SetChnVolume
	dc.l	SetSongVolume
	dc.l	SetChnArpeggio











channel1_A
	dc.l	$00DFF0A0	00*	; hardware
	dc.w	$0001		04*	; dma
	dc.l	$00000000	06*	; instr info address
	dc.l	$00000000	0A*	; track address
	dc.l	$00000000	0E*	; block address
	dc.w	$0000		12*	; track pos	(sequence)
	dc.w	$0000		14*	; block pos	(pattern)
	dc.b	$00		16*	; sample number
	dc.b	$00		17*	; waveform tab pos
	dc.l	$0000		18*	; transposed period
	dc.w	$0000		1A*	; period
	dc.b	$00		1C*	; note
	dc.b	$00		1D*	; channel volume
	dc.w	$0000		1E*	; channel bendrate
	dc.w	$0000		20*	; volume
	dc.w	$0000		22*	; volume tab pos
	dc.b	$00		24*	; volume sustain
	dc.b	$00		25*	; porta speed
	dc.b	$00		26*	; ???
	dc.b	$00		27*	; vibrator dir
	dc.w	$0000		28*	; final bendrate
	dc.b	$00		2A*	; vibrator len
	dc.b	$00		2B*	; note transpose
	dc.l	$00000000	2C*	; arpeggio table
	dc.w	$0000		30*	; arpeggio tab pos
	dc.b	$00		32*	; play mode (0 = soundfx, 1 = normal)
	dc.b	$00		33*	; instr type (0 = sample, 1 = synth)
	dc.b	$00		34*	; vibrator tab pos
	dc.b	$00		35*	; vibrator sustain
	dc.w	$0000		36*	; track len
	dc.w 	$0000		38*	; track end




channel2_A
	dc.l	$00DFF0B0	00*
	dc.w	$0002		04*
	dcb.b	34,0

channel3_A
	dc.l	$00DFF0C0	00*
	dc.w	$0004		04*
	dcb.b	34,0

channel4_A
	dc.l	$00DFF0D0	00*
	dc.w	$0008		04*
	dcb.b	34,0







channel1_B
	dc.l	$00DFF0A0	00*
	dc.w	$0001		04*
	dcb.b	34,0

channel2_B
	dc.l	$00DFF0B0	00*
	dc.w	$0002		04*
	dcb.b	34,0

channel3_B
	dc.l	$00DFF0C0	00*
	dc.w	$0004		04*
	dcb.b	34,0

channel4_B
	dc.l	$00DFF0D0	00*
	dc.w	$0008		04*
	dcb.b	34,0








Periods:	dc.w	$0000
		dc.w	$1AC0,$1940,$17D0,$1680,$1530,$1400,$12E0,$11D0,$10D0,$0FE0,$0F00,$0E20
		dc.w	$0D60,$0CA0,$0BE8,$0B40,$0A98,$0A00,$0970,$08E8,$0868,$07F0,$0780,$0710
		dc.w	$06B0,$0650,$05F4,$05A0,$054C,$0500,$04B8,$0474,$0434,$03F8,$03C0,$0388
		dc.w	$0358,$0328,$02FA,$02D0,$02A6,$0280,$025C,$023A,$021A,$01FC,$01E0,$01C4
		dc.w	$01AC,$0194,$017D,$0168,$0153,$0140,$012E,$011D,$010D,$00FE,$00F0,$00E2
		dc.w	$00D6,$00CA,$00BE,$00B4,$00AA,$00A0,$0097,$008F,$0087,$007F,$0078,$0071
		dc.w	$0071,$0071,$0071,$0071,$0071,$0071,$0071,$0071,$0071,$0071,$0071,$0071


MasterDMA	dc.w	$8000


ModAddress	dc.l	0

InstrTable	dc.l	0
InstrTableOfs	dc.l	0


Track1Data	dc.l	0
Track2Data	dc.l	0
Track3Data	dc.l	0
Track4Data	dc.l	0

Track1Len  	dc.l	8
Track2Len  	dc.l	8
Track3Len  	dc.l	8
Track4Len  	dc.l	8

Track1LenCnt	dc.w	0
Track2LenCnt	dc.w	0
Track3LenCnt	dc.w	0
Track4LenCnt	dc.w	0


PATinfo		dc.l	0
NoiseAddr	dc.l	0
SampleBaseAddr	dc.l	0
SampleInfo	dc.l	0
RandomSeed	dc.l	0
SongVolume	dc.b	63
REPspd		dc.b	0
REPcnt		dc.b	0


PlayMode	dc.b	0







