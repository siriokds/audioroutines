;============
SECTION .bss
;============


struc	instr
	.Start		resd 1
	.Len		resd 1
	.C2Spd		resd 1
	.Volume		resd 1
	.LoopSta	resd 1
	.LoopEnd	resd 1
	.Mode		resd 1
	.Filler		resd 1
	.size
endstruc


struc	chan
	.VoiceN		resd 1
	.ON		resd 1
	.Volume		resd 1
	.Period		resd 1

	.PanVal		resd 1
	.lastins	resd 1 	; instrument # for each channel (to remember)
	.lastnot	resd 1	; last note set in channel (to remember)
	.lastper	resd 1	; last period set in channel (to remember)

	.restart	resd 1	; flag whether to play sample or not
	.start		resd 1	; where to start in sample
	.soffset	resd 1	; amount to sample offset by
	.porto		resd 1	; note to port to value (signed word)

	.portsp		resd 1	; porta speed
	.vibpos		resd 1	; vibrato position
	.vibspe		resd 1	; vibrato speed
	.vibdep		resd 1	; vibrato depth

	.trempos	resd 1	; tremolo position
	.tremspe	resd 1	; tremolo speed
	.tremdep	resd 1	; tremolo depth
	.patlooprow	resd 1  ;

	.patloopno	resd 1	; pattern loop variables for effect  E6x
	.wavecon	resd 1	; waveform type for vibrato and tremolo (4bits each)
	
	.filler		resd 10
	.size
endstruc



struc	module
	.ModAddress	resd 1
	.Opened		resd 1
	.Playing	resd 1
	
	.tick		resd 1
	.speed		resd 1
	.row		resd 1
	.ord		resd 1
	.patdelay	resd 1

	.POSnum		resd 1
	.PATnum		resd 1
	.INSnum		resd 1
	.SMPptr		resd 1
	
	.Restart	resd 1
	.Flags		resd 1
	.size
endstruc


;- - - - - - - - - - - - - - - - - - - - - - - 

extern	_paulaOpen@8
extern	_paulaMasterVol@4
extern	_paulaStart@0
extern	_paulaClose@0
extern	_paulaReset@0
extern	_paulaSelectVoice@4
extern	_paulaSetPeriod@4
extern	_paulaSetVolume@4
extern	_paulaSetSamplePos@4
extern	_paulaSetSampleLen@4
extern	_paulaSetVoice@4

global	_CHP_Open@8
global	_CHP_InitModule@4
global	_CHP_Start@0
global	_CHP_Close@0



CHPinfo: resb module.size
VOCdata: resb (chan.size*4)
POSinfo: resb (128)
INSinfo: resb (instr.size*32)
PATinfo: resb (1024*256)
SMPinfo: resb (256*1024)
sizeAll  equ $-CHPinfo



;============
SECTION .code
;============


Periods	dw 6B00h, 6500h, 5F40h, 5A00h, 54C0h, 5000h, 4B80h, 4740h, 4340h, 3F80h, 3C00h, 38A0h 
	dw 3580h, 3280h, 2FA0h, 2D00h, 2A60h, 2800h, 25C0h, 23A0h, 21A0h, 1FC0h, 1E00h, 1C50h 
	dw 1AC0h, 1940h, 17D0h, 1680h, 1530h, 1400h, 12E0h, 11D0h, 10D0h, 0FE0h, 0F00h, 0E28h 
	dw 0D60h, 0CA0h, 0BE8h, 0B40h, 0A98h, 0A00h, 0970h, 08E8h, 0868h, 07F0h, 0780h, 0714h 
	dw 06B0h, 0650h, 05F4h, 05A0h, 054Ch, 0500h, 04B8h, 0474h, 0434h, 03F8h, 03C0h, 038Ah 
	dw 0358h, 0328h, 02FAh, 02D0h, 02A6h, 0280h, 025Ch, 023Ah, 021Ah, 01FCh, 01E0h, 01C5h 
	dw 01ACh, 0194h, 017Dh, 0168h, 0153h, 0140h, 012Eh, 011Dh, 010Dh, 00FEh, 00F0h, 00E2h 
	dw 00D6h, 00CAh, 00BEh, 00B4h, 00AAh, 00A0h, 0097h, 008Fh, 0087h, 007Fh, 0078h, 0071h 
	dw 006Bh, 0065h, 005Fh, 005Ah, 0055h, 0050h, 004Bh, 0047h, 0043h, 003Fh, 003Ch, 0038h 
	dw 0035h, 0032h, 002Fh, 002Dh, 002Ah, 0028h, 0025h, 0023h, 0021h, 001Fh, 001Eh, 001Ch 
	dw 001Ah, 0019h, 0017h, 0016h, 0015h, 0014h, 0012h, 0011h, 0010h, 000Fh, 000Fh, 000Eh
	dw 0000h, 0000h, 0000h, 0000h, 0000h, 0000h, 0000h, 0000h, 0000h, 0000h, 0000h, 0000h

Sintab	db   0, 24, 49, 74,  97,120,141,161, 180,197,212,224, 235,244,250,253
	db 255,253,250,244, 235,224,212,197, 180,161,141,120,  97, 74, 49, 24



FineHz	dw 8363, 8413, 8463, 8529, 8581, 8651, 8723, 8757
	dw 7895, 7941, 7985, 8046, 8107, 8169, 8232, 8280
	
LOGtab	db 000h, 081h, 0C1h, 0E1h, 0F1h, 0F9h, 0FDh, 0FFh, 000h, 001h, 003h, 007h, 00Fh, 01Fh, 03Fh, 07Fh


;---------------------------------
;UnPacked Note are 3bytes long.
;---------------------------------
;0000-0000  0000-0000  0000-0000
;|\     /\     / \  /   \      /
;| note.  sampl   Fx     Fx Arg
;| numbr  numbr  (4)      (8)
;|  (6)   (5)
;|
;description bit 
;set to 0 (if note) or 1 (if rep)
;---------------------------------

; in:  esi = ptr packed note
; out: eax = unpacked note   ( FXA FXC SMP NOT )
;------------------------------------------------------------------------
UnpackNote:
;------------------------------------------------------------------------
	push	ebx
	
	
	mov	bl,[esi]
	shr	bl,1
	and	bl,3Fh
	ror	ebx,8			; get note num

	mov	al,[esi]
	and	al,1
	shl	al,4
	mov	bl,[esi+1]
	shr	bl,4
	or	bl,al		
	ror	ebx,8			; get sample num

	mov	bl,[esi+1]
	and	bl,0Fh
	ror	ebx,8

	mov	bl,[esi+2]
	ror	ebx,8
	
	mov	eax,ebx
	pop	ebx
	ret


;------------------------------------------------------------------------
UnpackChn:
;------------------------------------------------------------------------
	push	ebx
	push	ecx
	push	edx
	push	edi
	
	xor	edx,edx
	mov	ecx,64
.anNote:
	mov	al,[esi]
	cmp	al,7Fh
	ja	short .RepLast	

	call	UnpackNote
	mov	edx,eax
	mov	[edi],eax
	add	edi,16
	dec	ecx
	add	esi,3
	
	jmp	short .nxtNote	

.RepLast:
	and	eax,7Fh
	sub	ecx,eax
	
.rpL:	mov	[edi],edx
	add	edi,16
	dec	al
	jnz	short .rpL
	
	inc	esi
.nxtNote:
	or	ecx,ecx
	jg	short .anNote
	
	pop 	edi
	pop 	edx
	pop 	ecx
	pop 	ebx
	ret


;------------------------------------------------------------------------
ClearGlobal:
;------------------------------------------------------------------------
	pushad
	lea	edi,[CHPinfo]
	mov	ecx, sizeAll
	xor	eax, eax
	cld
	rep	stosb
	popad
	ret

;------------------------------------------------------------------------
CopyMem:
;------------------------------------------------------------------------
	push	ecx
	push	esi
	push	edi
	
	push	ecx
	cld
	shr	ecx,2
	rep	movsd
	pop	ecx
	and	ecx,3
	rep	movsb

	pop	edi
	pop	esi
	pop	ecx
	ret






;------------------------------------------------------------------------
_CHP_Open@8:
;------------------------------------------------------------------------
	push	ebp
	mov	ebp,esp
	pushad
	
	lea	ebx,[CHPinfo]
	
	call	ClearGlobal

;	mov	eax,[ebp+8]
;	push	eax
;	lea	eax,[CHP_Update]
;	push	eax
;	call	_paulaOpen@8
;	mov	eax,[ebp+12]
;	push	eax
;	call	_paulaMasterVol@4
	
	mov	byte [ebx + module.Opened],1

	popad
	pop	ebp
	ret	8h


;---------------------------------
_CHP_Close@0:
;---------------------------------
	pushad

;	call	_paulaClose@0

	call	ClearGlobal

	popad
	ret



;---------------------------------
_CHP_Start@0:
;---------------------------------
	pushad
	lea	ebx,[CHPinfo]

	cmp	byte [ebx + module.Opened],0
	jz	.exit
	call	_paulaStart@0
	mov	byte [ebx + module.Playing],1
.exit:	
	popad
	ret










;-------------------------------------------------
_CHP_InitModule@4:
;-------------------------------------------------
	push	ebp
	mov	ebp,esp
	pushad
	lea	ebx,[CHPinfo]
	
	mov	esi,[ebp+8]
	mov	[ebx + module.ModAddress],esi
	mov	ebp,ebx

	xor	eax,eax
	
	mov	al,[esi+06h]
	mov	[ebp + module.Restart],eax

	mov	al,[esi+07h]
	mov	[ebp + module.PATnum],eax

	mov	eax,[esi+08h]
	add	eax,esi
	mov	[ebp + module.SMPptr],eax
	
	
	movzx	eax,byte [esi+0Eh]
	mov	[ebp + module.INSnum],eax
	mov	bl,al
	
	movzx	eax,byte [esi+0Fh]
	and	al,7Fh
	mov	[ebp + module.POSnum],eax


;---- POSITIONS

	add	esi,10h			; skip header
	movzx	ecx,al
	lea	edi,[POSinfo]
	cld
	rep	movsb
	


;---- INSTRUMENTS
	push	ebp	
	
	movzx	ecx,bl
	lea	edi,[INSinfo]
	xor	eax,eax
	lea	ebp,[SMPinfo]
	lea	ebx,[FineHz]

.SetInstr:
	mov	[edi + instr.Start],ebp
	
	mov	ax,[esi]		; get SampleSize / 2
	add	eax,eax
	mov	[edi + instr.Len],eax
	add	ebp,eax
	
	movzx	eax,byte [esi+2]	; get Finetune
	movzx	eax,word [ebx + eax*2]
	mov	[edi + instr.C2Spd],eax
	
	xor	eax,eax
	mov	al,[esi+3]		; get Volume
	and	al,7Fh
	cmp	al,40h
	jle	.najVl
	mov	al,40h
.najVl:	mov	[edi + instr.Volume],eax

	
	movzx	eax,word [esi+4]	; LoopSta / 2
	add	eax,eax
	mov	[edi + instr.LoopSta],eax
	
	movzx	eax,word [esi+6]	; LoopSize / 2
	add	eax,eax
	add	eax,[edi + instr.LoopSta]
	dec	eax
	mov	[edi + instr.LoopEnd],eax
	
	add	esi,8
	add	edi,instr.size
	
	loop	.SetInstr
	

	pop	ebp
	
;---- PATTERNS
	
	lea	edi,[PATinfo]
	mov	ecx,[ebp + module.PATnum]

.unpackPAT:	
	push	edi
	call	UnpackChn
	add	edi,4
	call	UnpackChn
	add	edi,4
	call	UnpackChn
	add	edi,4
	call	UnpackChn
	pop	edi
	add	edi,1024
	loop	.unpackPAT


;---- SAMPLES

; RL-Delta (RLE/Log/Delta) UnPacking

	mov	ecx,[ebp + module.INSnum]
	push	ebp
	
	lea	ebx,[INSinfo]
	lea	edi,[SMPinfo]
	lea	ebp,[LOGtab]
	cld
.nextINS:
	push	ecx
	mov	ecx,[ebx + instr.Len]
	push	ebx
	shr	ecx,1
	xor	edx,edx
	xor	eax,eax
	xor	ebx,ebx			; bl = RLE_last


;--- > UNPACK START < ---
.decINS:
	lodsb
	cmp	al,0Fh
	ja	short .noREP

	inc	al
	sub	ecx,eax
	mov	bh,al
	mov	al,bl
.unpkR:	call	unpackSampleByte
	dec	bh
	jnz	short .unpkR

	or	ecx,ecx
	jmp	short .decData
.noREP:	
	mov	bl,al			; RLElast = val
	call	unpackSampleByte
	
	dec	ecx
.decData:
	jg	short .decINS
	
;--- > UNPACK END < ---

.unpInsEnd:

	pop	ebx
	pop	ecx
	add	ebx,instr.size
.decSMP:
	loop	.nextINS
	pop	ebp

;-----
; VOC RESET HERE!
;-----


	popad
	pop	ebp
	ret	4




;---------------------------
;---------------------------
unpackSampleByte:
;---------------------------
;---------------------------
	push	eax
	shr	al,4
	add	dl,[ebp + eax]
	pop	eax
	mov	[edi],dl
	push	eax
	and	al,00Fh
	add	dl,[ebp + eax]
	pop	eax
	mov	[edi+1],dl
	add	edi,2
	ret












;---------------------------
CHP_Update:
;---------------------------
	pushad
	lea	ebx,[CHPinfo]

	xor	eax,eax
	cmp	[ebx + module.Opened],eax
	jz	.exit
	cmp	[ebx + module.Playing],eax
	jz	.exit
;----
	mov	eax,[ebx + module.tick]
	inc	eax
	cmp	eax,[ebx + module.speed]
	jl	short .doeffects

	xor	eax,eax	
	mov	[ebx + module.tick],eax

;---------

	mov	eax,[ebx + module.row]
	cmp	eax,64
	jl	short .noRowRst

	xor	eax,eax
	mov	[ebx + module.row],eax

	mov	eax,[ebx + module.ord]
	inc	eax
	cmp	eax,[ebx + module.POSnum]
	jl	short .noOrdRst
	mov	eax,[ebx + module.Restart]
.noOrdRst:
	mov	[ebx + module.ord],eax
	
.noRowRst:	
	mov	eax,[ebx + module.patdelay]
	or	eax,eax
	jnz	short .decdelay

	call	NewNote
	inc	byte [ebx + module.row]
	jmp	short .exit

.decdelay:
	dec	eax
	mov	[ebx + module.patdelay],eax
	jmp	short .exit
	

.doeffects:
	mov	[ebx + module.tick],eax

.exit:
	popad
	ret








;----------------------------
NewNote:
;----------------------------
	pushad
	mov	ebp,eax
	lea	edi,[CHPinfo]

	movzx	eax,byte [ebp + chan.VoiceN]
	push	eax
	call	_paulaSelectVoice@4


	popad
	ret








;----------------------------
Effects:
;----------------------------
	pushad
	mov	ebp,eax
	lea	edi,[CHPinfo]
	xor	edx,edx

	movzx	eax,byte [ebp + chan.VoiceN]
	push	eax
	call	_paulaSelectVoice@4

;*************************************************************************
;------------------------------------
	add	ax,[ebp + chan.Period]		; AX = Period
checklimit:
	cmp	ax,0070h			; CHECK LO PERIOD LIMIT (MAX FREQ)
	ja	nn1
	mov	ax,0071h
nn1:
	cmp	ax,0d60h			; CHECK HI PERIOD LIMIT (MIN FREQ)
	jbe	nn2
	mov	ax,0d60h
nn2:
	movzx	ecx,ax
	
	push	ecx
	call	_paulaSetPeriod@4
	
	movzx	ecx,word [ebp + chan.Volume]
	
	push	ecx
	call	_paulaSetVolume@4

	popad
	ret


