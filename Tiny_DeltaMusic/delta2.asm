;============
SECTION .bss
;============


struc track
	.Data	resd 1
	.Len	resd 1
	.Rst	resd 1
	.size
endstruc


struc instr
	.i_sample_size		resw 1	; $00
	.i_repsta		resw 1	; $02
	.i_replen		resw 1	; $04
	
	.i_volume_table		resb 15	; $06
	.i_vibrato_table	resb 15	; $15
	.i_bendrate		resw 1	; $24

	.i_type			resb 1	; $25
	.i_wavetable		resb 48	; $26
	.i_filler		resb 41
	.size		; 	(128 bytes)
endstruc


struc chan
	.c_voiceN         	resd 1
	.c_kick           	resw 1
	.c_instr_data     	resd 1
	.c_track_data       	resd 1
	.c_block_data       	resd 1
	.c_track_pos        	resw 1
	.c_block_pos        	resw 1
	.c_waveform_tabpos  	resb 1
	.c_period_transp    	resw 1
	.c_period	   	resw 1
	.c_note		   	resb 1
	.c_chn_volume       	resb 1
	.c_chn_bendrate     	resw 1
	.c_volume           	resw 1
	.c_volume_tabpos    	resw 1
	.c_volume_sustain   	resb 1
	.c_porta_speed      	resb 1
	.c_waveform_tabspd	resb 1
	.c_waveform_tabspdcnt	resb 1
	.c_vibrato_dir     	resb 1
	.c_final_bendrate   	resw 1
	.c_vibrato_len     	resb 1
	.c_note_transp      	resb 1
	.c_arpeggio_data	resd 1
	.c_arpeggio_tabpos  	resw 1
	.c_plmode 	   	resb 1
	.c_instr_type 	   	resb 1
	.c_vibrato_tabpos  	resb 1
	.c_vibrato_sust    	resb 1
	.c_track_len	   	resw 1
	.c_track_rst	   	resw 1

	.size		;	(64 bytes)
endstruc




;<<<<<<<<<
; d0 = eax
; d1 = ebx
; d2 = ecx
; a0 = ebp
; a2 = esi
; a3 = edi
; a4 = edx
;<<<<<<<<<


;- - - - - - - - - - - - - - - - - - - - - - - 

extern	_paulaOpen@8
extern	_paulaStart@0
extern	_paulaClose@0
extern	_paulaReset@0
extern	_paulaSelectVoice@4
extern	_paulaSetPeriod@4
extern	_paulaSetVolume@4
extern	_paulaSetSamplePos@4
extern	_paulaSetSampleLen@4
extern	_paulaSetVoice@4
;extern	_paulaDMACON@4

global	_DM2_Open@4
global	_DM2_InitModule@4
global	_DM2_Start@0
global	_DM2_Close@0


channel_A	resb chan.size*4
trackInfo	resb track.size*4
Arpeggio	resb 1024
Tracks		resb (8*256)
Blocks		resb ((4*16)*256)
InstrData	resb (128*128)
Wavs		resb 65536
SampleInfos	resd 8
SampleLens	resd 8
DataSize	equ $-channel_A		

;============
SECTION .code
;============


;------------------------------------------------------------------------
_DM2_Open@4:
;------------------------------------------------------------------------
	push	ebp
	mov	ebp,esp
	push	eax
	
	mov	eax,[ebp+8]
	push	eax
	lea	eax,[DM_play]
	push	eax
	call	_paulaOpen@8


	lea	edi,[channel_A]
	mov	ecx,DataSize/4
	xor	eax,eax
	cld
	rep	stosd
	

	xor	ecx,ecx
	lea	ebp,[channel_A]
.initCHN:	
	mov	[ebp + chan.c_voiceN],ecx
	add	ebp,chan.size
	inc	cl
	cmp	cl,4
	jl	.initCHN
	
	mov	byte [Opened],1

	pop	eax
	pop	ebp
	ret	4h


;---------------------------------
_DM2_Close@0:
;---------------------------------
	pushad
	call	_paulaClose@0
	popad
	ret


;---------------------------------
_DM2_Start@0:
;---------------------------------
	pushad
	cmp	byte [Opened],0
	jz	.exit
	cmp	byte [Playing],0
	jnz	.exit
	mov	byte [Playing],1
	call	_paulaStart@0
.exit:	
	popad
	ret


;-----------------------------------------------------
_DM2_InitModule@4:
;-----------------------------------------------------
	push	ebp
	mov	ebp,esp
	pushad

	mov	eax,[ebp+8h]			; get module address from stack
	cmp	dword [eax],4C4E462Eh		; check for '.FNL' at 0000h
	je	.OKValidModule
	add	eax,0BC6h			; skip playroutine
	cmp	dword [eax],4C4E462Eh		; check for '.FNL' at 0BC6h
	je	.OKValidModule
	
	popad
	pop	ebp
	mov	al,1
	ret	4h

.OKValidModule:

	cld

	lea	esi,[eax+4]			; skip '.FNL'
	lea	edi,[Arpeggio]
	mov	ecx,1024/4
	rep	movsd				; Copy Arpeggios
	
	
	xor	edx,edx

	lea	edi,[trackInfo]
	mov	cl,4
.iniLenRst:
	mov	eax,[esi]
	bswap	eax
	mov	[edi + track.Len],ax
	shr	eax,16
	mov	[edi + track.Rst],ax
	lea	esi,[esi+4]
	add	edi,track.size
	dec	cl
	jnz	.iniLenRst



	lea	ebx,[trackInfo]
	lea	edi,[Tracks]
	mov	dl,4				; Point to Track infos
.iniDatLen:
	mov	[ebx + track.Data],edi
	mov	ecx,[ebx + track.Len]
	rep	movsb
	add	ebx,track.size
	dec	dl
	jnz	.iniDatLen




	mov	ecx,[esi]			; get PATlen
	bswap	ecx
	mov	ebx,ecx
	lea	esi,[esi+4]			; point to PAT
	lea	edi,[Blocks]
	mov	[PATinfo],edi			; save PAT address
	rep	movsb



;	pushad
;	lea	edi,[Blocks]
;	mov	ecx,ebx
;	shr	ecx,2
;.CheckZI:
;	mov	eax,[edi]
;	or	al,al
;	jz	.noNote
;	or	ah,ah
;	jnz	.noNote
;	mov	byte [ZeroInstr],1
;	jmp	.EndZI
;.noNote:
;	add	edi,4
;	loop	.CheckZI	
;.EndZI:	
;	popad





	lea	esi,[esi+256]			; skip info offs


	;- Get Infos -
	cld
	
	movzx	eax,word [esi-2]		; get TOT instr infos size
	xchg	ah,al
	mov	ecx,88
	xor	edx,edx
	div	ecx
	mov	[WaveN],al
	mov	ebx,eax
	lea	edi,[InstrData]
	mov	[InstrTable],edi		; first instr info
	xor	eax,eax
.cinstr:	
	push	edi
	
	mov	ecx,3
.ci_s:	lodsw					; copy i_samples_size, i_repsta, i_replen
	xchg	ah,al
	stosw
	loop	.ci_s

	mov	ecx,(15+15)			; copy i_volume_table, i_vibrato_table
	rep	movsb
	
	lodsw					; copy i_bendrate
	xchg	ah,al
	stosw
	
	mov	ecx,(1+48)			; copy i_type, i_wavetable
	rep	movsb

	inc	esi				; skip filler

	pop	edi
	add	edi,instr.size
	
	dec	ebx
	jnz	.cinstr
	
	
	
	mov	ecx,[esi]
	bswap	ecx				; get wav len
	lea	esi,[esi+4]
	lea	edi,[Wavs]
	mov	[NoiseAddr],edi
	rep	movsb

	

	lea	edi,[SampleLens]
	mov	cl,8
.copySlens:
	mov	eax,[esi]
	bswap	eax
	mov	[edi],eax
	add	esi,4
	add	edi,4
	dec	cl
	jnz	.copySlens
	
	lea	esi,[esi+32]

	lea	edi,[SampleInfos]
	mov	[SampleInfo],edi
	mov	cl,8
.copySinfo:
	mov	eax,[esi]
	bswap	eax
	mov	[edi],eax
	add	esi,4
	add	edi,4
	dec	cl
	jnz	.copySinfo
	
	mov	[SampleBaseAddr],esi
	


	xor	edx,edx
	
	lea	ebp,[channel_A]
	lea	edi,[trackInfo]
	mov	cl,4
.ini_A:	
	mov	eax,[edi + track.Data]
	mov	[ebp + chan.c_track_data],eax
	
	mov	ax,[edi + track.Len]
	mov	[ebp + chan.c_track_len],ax
	
	mov	ax,[edi + track.Rst]
	mov	[ebp + chan.c_track_rst],ax
	
	call	InitVoice
	add	ebp,chan.size
	add	edi,track.size
	dec	cl
	jnz	.ini_A


	mov	byte [REPspd],6
	mov	byte [REPcnt],0

	popad
	pop	ebp
	xor	eax,eax
	ret	4h



InitVoice:
	mov	[ebp + chan.c_kick],dx

	mov	word [ebp + chan.c_track_pos],-2	; track pos
	mov	word [ebp + chan.c_block_pos],64	; block pos
	
	mov	eax,[InstrTable]
	mov	[ebp + chan.c_instr_data],eax		; first instr
	
	mov	[ebp + chan.c_instr_type],dl
	mov	[ebp + chan.c_arpeggio_tabpos],dx	; arpeggio tab pos

	lea	eax,[Arpeggio]
	mov	[ebp + chan.c_arpeggio_data],eax	; first arpeggio data
	
	mov	[ebp + chan.c_volume],dx		; volume
	mov	[ebp + chan.c_volume_tabpos],dx		; volume tab pos
	mov	[ebp + chan.c_volume_sustain],dl	; volume sustain
	mov	[ebp + chan.c_porta_speed],dl		; porta speed
	mov	[ebp + chan.c_chn_bendrate],dx		; channel bendrate
	mov	byte [ebp + chan.c_chn_volume],3fh	; channel volume
	ret
	
	

;-------------------------------------------------------------------------------
DM_play:
;-------------------------------------------------------------------------------
	cmp	byte [Opened],0
	jz	.exit
	cmp	byte [Playing],0
	jz	.exit
	jmp	.okplay
.exit:	ret

.okplay:
	pushad
;---------------------------------
; Random Sample
	mov	eax,[RandomSeed]
	mov	edi,[NoiseAddr]
	mov	ecx,16			; 64 random samples = 16*4
	cld
.RandomGen:
	rol	eax,7
	add	eax,6ECA756Dh
	xor	eax,9E59A92Bh
	stosd
	loop	.RandomGen
	mov	[RandomSeed],eax


;---------------------------------
; Check REPcnt
	
	cmp	byte [REPcnt],0
	jnz	.doEffects


	lea	ebp,[channel_A]
	call	NewNote

	add	ebp,chan.size
	call	NewNote

	add	ebp,chan.size
	call	NewNote

	add	ebp,chan.size
	call	NewNote


	mov	al,[REPspd]
	mov	[REPcnt],al

.doEffects:

	lea	ebp,[channel_A]
	call	Effects

	add	ebp,chan.size
	call	Effects

	add	ebp,chan.size
	call	Effects

	add	ebp,chan.size
	call	Effects

	dec	byte [REPcnt]



;	mov	cl,4
;	lea	ebp,[channel_A]
;	add	ebp,chan.size*1
;.doNote:	
;	call	NewNote
;	add	ebp,chan.size
;	dec	cl
;	jnz	.doNote

;.doEffects:

;	mov	cl,4
;	lea	ebp,[channel_A]
;	add	ebp,chan.size*1
;.doFX:	
;	call	Effects
;	add	ebp,chan.size
;	dec	cl
;	jnz	.doFX

;---------------------------------

	popad
	ret







;-------------------------------------------------------------------------------
NewNote:
;-------------------------------------------------------------------------------
	pushad
	
	mov	eax,[ebp + chan.c_voiceN]
	push	eax
	call	_paulaSelectVoice@4
	

	; check for PATend
	cmp	word [ebp + chan.c_block_pos],64
	jl	.sameBlock
	
	
	mov	word [ebp + chan.c_block_pos],0

	; check for SEQend

	movzx	eax,word [ebp + chan.c_track_pos]
	add	ax,2
	cmp	ax,[ebp + chan.c_track_len]
	jl	.sameTrack
	movzx	eax,word [ebp + chan.c_track_rst]
.sameTrack: 
	mov	[ebp + chan.c_track_pos],ax
	movzx	ebx,ax
	mov	edi,[ebp + chan.c_track_data]		; track adr
	mov	al,[edi + ebx + 1]			; note transpose
	mov	[ebp + chan.c_note_transp],al

	movzx	eax,byte [edi + ebx]			; block number
	sal	eax,6					; *64 (block offset)
	add	eax,[PATinfo]				; + block base address
	mov	[ebp + chan.c_block_data],eax


.sameBlock:
	mov	edi,[ebp + chan.c_block_data]		; current block
	movzx	eax,word [ebp + chan.c_block_pos]	; pos in block
	add	edi,eax
	movzx	eax,byte [edi]				; 1) get note
	or	al,al
	jz	NEAR .noNewNote

; *** PLAY NOTE ***

	mov	[ebp + chan.c_note],al			; put note
	lea	edx,[Periods]				; period table
	add	al,[ebp + chan.c_note_transp]		; note transposition
	mov	ax,[edx + eax*2]
	mov	[ebp + chan.c_period],ax		; put period


	movzx	eax,byte [edi+1]			; 2) get instrument
	cmp	byte [ZeroInstr],0
	jnz	.noDecInstr
	dec	eax
.noDecInstr:
	js	NEAR .noNewNote
	shl	eax,7					; *128 = instr.size
	add	eax,[InstrTable]
	mov	[ebp + chan.c_instr_data],eax		; current instr ptr
	mov	esi,eax
	mov	al,[esi + instr.i_type]
	mov	[ebp + chan.c_instr_type],al


; *** RESET CHAN ***

	xor	edx,edx
	
	mov	[ebp + chan.c_period_transp],dx
	
	mov	[ebp + chan.c_waveform_tabpos],dl
	mov	[ebp + chan.c_arpeggio_tabpos],dx

	mov	[ebp + chan.c_volume],dx
	mov	[ebp + chan.c_volume_tabpos],dx
	mov	[ebp + chan.c_volume_sustain],dl

	mov	[ebp + chan.c_final_bendrate],dx

	mov	[ebp + chan.c_vibrato_tabpos],dl
	mov	[ebp + chan.c_vibrato_dir],dl
	mov	al,[esi+16h]				; get vibrato_len in vibrato table!
	mov	[ebp + chan.c_vibrato_len],al
	mov	al,[esi+17h]				; get vibrato_sust in vibrato table!
	mov	[ebp + chan.c_vibrato_sust],al


	push	dword 0
	call	_paulaSetVoice@4			; Stop Voice!


; *** PLAY SAMPLE OR SYNTH ? ***

	test	byte [ebp + chan.c_instr_type],80h	; is synth ?
	jz	.playSynth				; yes! skip!


	movzx	eax,byte [esi + instr.i_wavetable]	; 1st byte of wavetable = sample number
	and	al,7
	mov	edi,[SampleInfo]			; sample offset table

	mov	ecx,[edi+(eax*4)]			; relative sample adr
	add	ecx,[SampleBaseAddr]
	mov	ebx,ecx
	push	ecx
	call	_paulaSetSamplePos@4			; sample address

	add	edi,32
	mov	ecx,[edi+(eax*4)]			; relative sample adr
	shr	ecx,1
	push	ecx
	call	_paulaSetSampleLen@4


	push	dword 1
	call	_paulaSetVoice@4
	

	movzx	eax,word [esi + instr.i_repsta]
	add	eax,ebx
	push	eax
	call	_paulaSetSamplePos@4			; sample address


	movzx	eax,word [esi + instr.i_replen]
	push	eax
	call	_paulaSetSampleLen@4

;	push	dword 1
;	call	_paulaSetSampleLen@4

	jmp	.noNewNote

.playSynth:
	mov	al,[esi + instr.i_wavetable]		; 1st byte of wavetable = table speed
	inc	al
	mov	[ebp + chan.c_waveform_tabspd],al

	mov	byte [ebp + chan.c_waveform_tabspdcnt],1

	mov	byte [ebp + chan.c_waveform_tabpos],1	; point to 1st waveN
	mov	byte [ebp + chan.c_kick],1

	

.noNewNote:
	movzx	eax,byte[edi+2]				; 3) get effect number
	dec	eax
	js	.noEffect

	and	al,7
	shl	eax,2
	add	eax,effect_table
	mov	eax,[eax]				; get function address
	or	eax,eax					; !Protection!
	jz	.noEffect

	movzx	ebx,byte [edi+3]			; 4) get effect data
	call	eax					; call it!
	
.noEffect:
	add	word [ebp + chan.c_block_pos],4		; next block pos (every row = 4 bytes)

	popad
	ret	




;-------------------------------------------------------------------------------
Effects:
;-------------------------------------------------------------------------------
	pushad
	
	mov	eax,[ebp + chan.c_voiceN]
	push	eax
	call	_paulaSelectVoice@4

	mov	esi,[ebp + chan.c_instr_data]

	
	test	byte [ebp + chan.c_instr_type],80h	; is synth ?
	jnz	DM_vibrato_handler			; yes! skip!


;-------------------------------------	
DM_wavetable_handler:
;-------------------------------------	
	dec	byte [ebp + chan.c_waveform_tabspdcnt]
	jnz	DM_vibrato_handler

	mov	al,[ebp + chan.c_waveform_tabspd]
	mov	[ebp + chan.c_waveform_tabspdcnt],al

	lea	edi,[esi + instr.i_wavetable]
.GetWF:
	movzx	eax,byte [ebp + chan.c_waveform_tabpos]
	mov	cx, [edi + eax]
.checkWTend:
	cmp	cl,0FFh					; loop (in waveform table) ?
	jne	.noWTloop
	inc	ch
	and	ch,15
	mov	[ebp + chan.c_waveform_tabpos],ch
	jmp	.GetWF

.noWTloop:
	movzx	eax,cl					; get wavefN
	sal	eax,8					; *256 (synth len)
	add	eax,[NoiseAddr]				; Synth Base Address
	push	eax
	call	_paulaSetSamplePos@4			; SET sample address
	
	movzx	eax,word [esi + instr.i_sample_size]
	push	eax
	call	_paulaSetSampleLen@4			; SET sample length


	cmp	byte [ebp + chan.c_kick],0
	jz	.nokick
	mov	byte [ebp + chan.c_kick],0
	push	dword 1
	call	_paulaSetVoice@4
.nokick:

	inc	byte [ebp + chan.c_waveform_tabpos]
	and	byte [ebp + chan.c_waveform_tabpos],3Fh
	


;-------------------------------------	
DM_vibrato_handler:
;-------------------------------------	

	lea	edi,[esi + instr.i_vibrato_table]
	movzx	eax,byte [ebp + chan.c_vibrato_tabpos]
	add	edi,eax
	movzx	eax,byte [edi]

	cmp	byte [ebp + chan.c_vibrato_dir],0
	jnz	.vibraDown
	add	[ebp + chan.c_final_bendrate],ax
	jmp	.vibraUp
.vibraDown:
	sub	[ebp + chan.c_final_bendrate],ax
.vibraUp:
	dec	byte [ebp + chan.c_vibrato_len]
	jnz	.noChgDir
	mov	al,[edi+1]				; get vibrator len
	mov	[ebp + chan.c_vibrato_len],al
	not	byte [ebp + chan.c_vibrato_dir]		; change dir
.noChgDir
	cmp	byte [ebp + chan.c_vibrato_sust],0	
	jnz	.doVibSust				; vibrator sustain

	dec	byte [ebp + chan.c_vibrato_sust]
	jmp	DM_volume_handler

.doVibSust
	add	byte [ebp + chan.c_vibrato_tabpos],3
	cmp	byte [ebp + chan.c_vibrato_tabpos],15
	jl	.noVTend
	mov	byte [ebp + chan.c_vibrato_tabpos],12

.noVTend:
	mov	al,[edi+5]				; get vibra sustain
	mov	byte [ebp + chan.c_vibrato_sust],al
	




;-------------------------------------	
DM_volume_handler:
;-------------------------------------	

	cmp	byte [ebp + chan.c_volume_sustain],0	; Volume sustain
	jz	.noVolSust
	dec	byte [ebp + chan.c_volume_sustain]
	jmp	DM_portamento_handler
.noVolSust:

	movzx	eax,word [ebp + chan.c_volume_tabpos]
	lea	edi,[esi + instr.i_volume_table]
	add	edi,eax
	movzx	eax,byte [edi]				; vol rate
	movzx	ebx,byte [edi+1]			; vol level
	cmp	bx,[ebp + chan.c_volume]		; vol = target vol?
	jns	.addVol
	
	sub	word [ebp + chan.c_volume],ax
	cmp	bx,[ebp + chan.c_volume]		; vol = target vol?
	js	DM_portamento_handler

	mov	[ebp + chan.c_volume],bx
	add	word [ebp + chan.c_volume_tabpos],3
	mov	al,[edi + 2]				; vol sust
	mov	[ebp + chan.c_volume_sustain],al
	jmp	DM_portamento_handler

.addVol
	add	word [ebp + chan.c_volume],ax
	cmp	bx,[ebp + chan.c_volume]		; vol = target vol?
	jns	DM_portamento_handler

	mov	[ebp + chan.c_volume],bx
	add	word [ebp + chan.c_volume_tabpos],3
	cmp	word [ebp + chan.c_volume_tabpos],15
	jl	.noVLend
	mov	word [ebp + chan.c_volume_tabpos],12
.noVLend:
	mov	al,[edi + 2]				; vol sust
	mov	[ebp + chan.c_volume_sustain],al



;-------------------------------------	
DM_portamento_handler:
;-------------------------------------	

	movzx	eax,byte [ebp + chan.c_porta_speed]
	or	al,al
	jz	DM_arpeggio_handler
	
	movzx	ebx,word [ebp + chan.c_period]
	cmp	bx,[ebp + chan.c_period_transp]
	jns	.noPerTran

	sub	word [ebp + chan.c_period_transp],ax

	cmp	bx,[ebp + chan.c_period_transp]
	js	DM_arpeggio_handler
	mov	[ebp + chan.c_period_transp],bx
	jmp	DM_arpeggio_handler

.noPerTran:
	add	word [ebp + chan.c_period_transp],ax
	cmp	bx,[ebp + chan.c_period_transp]
	jns	DM_arpeggio_handler
	mov	[ebp + chan.c_period_transp],bx



;-------------------------------------	
DM_arpeggio_handler:
;-------------------------------------	
	mov	edi,[ebp + chan.c_arpeggio_data]
	movzx	ebx,word [ebp + chan.c_arpeggio_tabpos]
	movzx	eax,byte [edi + ebx]			; get arpeggio VAL
	
	or	bl,bl
	jz	lbC00077A
	cmp	al,80h					; repeat ?
	jnz	lbC00077A
	
	mov	word [ebp + chan.c_arpeggio_tabpos],0	; yes!
	jmp	DM_arpeggio_handler

lbC00077A:
	inc	word [ebp + chan.c_arpeggio_tabpos]
	and	word [ebp + chan.c_arpeggio_tabpos],15
	cmp	byte [ebp + chan.c_porta_speed],0
	jz	GetPeriod

	movzx	eax,word [ebp + chan.c_period_transp]	
	jmp	DM_bendrate_handler

GetPeriod:
	lea	edi,[Periods]
	add	al,[ebp + chan.c_note]
	add	al,[ebp + chan.c_note_transp]
	mov	ax, [edi + eax*2]			; period

;-------------------------------------	
DM_bendrate_handler:
;-------------------------------------	
	mov	bx,[esi + instr.i_bendrate]
	sub	bx,[ebp + chan.c_chn_bendrate]
	sub	[ebp + chan.c_final_bendrate],bx
	add	ax,[ebp + chan.c_final_bendrate]

	push	eax
	call	_paulaSetPeriod@4


doVolume:
	mov	ax,[ebp + chan.c_volume]
	shr	ax,2
	and	ax,63
	cmp	al,[ebp + chan.c_chn_volume]
	jle	lbC0007D4
	mov	al,[ebp + chan.c_chn_volume]
lbC0007D4:
	cmp	al,[SongVolume]
	jle	lbC0007DE
	mov	al,[SongVolume]
lbC0007DE:
	push	eax
	call	_paulaSetVolume@4

	
	popad
	ret




;<<<<<<<<<
; d0 = eax
; d1 = ebx
; d2 = ecx
; a0 = ebp
; a2 = esi
; a3 = edi
; a4 = edx
;<<<<<<<<<


align 4
;--------------
SetPlaySpeed:
;--------------
	push	ebx
	inc	bl
	and	bl,15
	mov	[REPspd],bl
	pop	ebx
SetHWFilter:
	ret


align 4
;--------------
SetBendRateUP:
;--------------
	push	ebx
	and	bx,00FFh
	neg	bx
	mov	[ebp + chan.c_chn_bendrate],bx
	pop	ebx
	ret

align 4
;--------------
SetBendRateDN:
;--------------
	push	ebx
	and	bx,00FFh
	mov	[ebp + chan.c_chn_bendrate],bx
	pop	ebx
	ret


align 4
;--------------
SetPortamento:
;--------------
	mov	[ebp + chan.c_porta_speed],bl
	ret

align 4
;--------------
SetChnVolume:
;--------------
	push	ebx
	and	bl,3Fh
	mov	[ebp + chan.c_chn_volume],bl
	pop	ebx
	ret

align 4
;--------------
SetSongVolume:
;--------------
	push	ebx
	and	bl,3Fh
	mov	[SongVolume],bl
	pop	ebx
	ret

align 4
;-------------------------------------	
SetChnArpeggio
;-------------------------------------	
	push	ebx
	push	edx
	and	ebx,3Fh
	sal	ebx,4
	add	ebx,Arpeggio
	mov	[ebp + chan.c_arpeggio_data],ebx
	pop	edx
	pop	ebx
	ret





align 4
effect_table	dd	SetPlaySpeed
		dd	SetHWFilter
		dd	SetBendRateUP
		dd	SetBendRateDN
		dd	SetPortamento
		dd	SetChnVolume
		dd	SetSongVolume
		dd	SetChnArpeggio



;============
SECTION .data
;============


Periods	dw	$0000
	dw	$1AC0,$1940,$17D0,$1680,$1530,$1400,$12E0,$11D0,$10D0,$0FE0,$0F00,$0E20
	dw	$0D60,$0CA0,$0BE8,$0B40,$0A98,$0A00,$0970,$08E8,$0868,$07F0,$0780,$0710
	dw	$06B0,$0650,$05F4,$05A0,$054C,$0500,$04B8,$0474,$0434,$03F8,$03C0,$0388
	dw	$0358,$0328,$02FA,$02D0,$02A6,$0280,$025C,$023A,$021A,$01FC,$01E0,$01C4
	dw	$01AC,$0194,$017D,$0168,$0153,$0140,$012E,$011D,$010D,$00FE,$00F0,$00E2
	dw	$00D6,$00CA,$00BE,$00B4,$00AA,$00A0,$0097,$008F,$0087,$007F,$0078,$0071
	dw	$0071,$0071,$0071,$0071,$0071,$0071,$0071,$0071,$0071,$0071,$0071,$0071


Opened		dd	0
Playing		dd	0
RandomSeed	dd	76B439FCh
ZeroInstr	dd	0

InstrTable	dd	0
PATinfo		dd	0
NoiseAddr	dd	0
WaveN		dd	0
SampleBaseAddr	dd	0
SampleInfo	dd	0

SongVolume	db	63
REPspd		db	0
REPcnt		db	0


