;============
SECTION .bss
;============


struc	instr
	.Start		resd 1
	.Len		resd 1
	.LoopSta	resd 1
	.LoopLen	resd 1
	.Volume		resd 1
	.C2Spd		resd 1
	.Filler		resd 2
	.size
endstruc


struc	chan
	.VoiceN		resd 1
	.ON		resd 1
	.Volume		resd 1
	.Period		resd 1
	.freq		resd 1

	.current	resd 1

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
	
	.filler		resd 8
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
global	_CHP_Update@0
global	_CHP_GetSamplesPtr@0

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


	dw 6B00h, 6500h, 5F40h, 5A00h, 54C0h, 5000h, 4B80h, 4740h, 4340h, 3F80h, 3C00h, 38A0h 
	dw 3580h, 3280h, 2FA0h, 2D00h, 2A60h, 2800h, 25C0h, 23A0h, 21A0h, 1FC0h, 1E00h, 1C50h 
	
	dw 1AC0h, 1940h, 17D0h, 1680h, 1530h, 1400h, 12E0h, 11D0h, 10D0h, 0FE0h, 0F00h, 0E28h 
	dw 0D60h, 0CA0h, 0BE8h, 0B40h, 0A98h, 0A00h, 0970h, 08E8h, 0868h, 07F0h, 0780h, 0714h 
	dw 06B0h, 0650h, 05F4h, 05A0h, 054Ch, 0500h, 04B8h, 0474h, 0434h, 03F8h, 03C0h, 038Ah 
	
Periods	dw 0358h, 0328h, 02FAh, 02D0h, 02A6h, 0280h, 025Ch, 023Ah, 021Ah, 01FCh, 01E0h, 01C5h 
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
align 4
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


align 4
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


align 4
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


align 4
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
_CHP_GetSamplesPtr@0:
;------------------------------------------------------------------------
	push	ebp
	mov	ebp,esp
	pop	ebp
	lea	eax,[SMPinfo]
	ret


;------------------------------------------------------------------------
_CHP_Update@0:
;------------------------------------------------------------------------
	push	ebp
	mov	ebp,esp
	pushad

	call	CHP_Update
	
	popad
	pop	ebp
	ret


align 4
;------------------------------------------------------------------------
_CHP_Open@8:
;------------------------------------------------------------------------
	push	ebp
	mov	ebp,esp
	pushad
	
	lea	ebx,[CHPinfo]
	
	call	ClearGlobal

	mov	eax,[ebp+8]
	push	eax
	lea	eax,[CHP_Update]
	push	eax
	call	_paulaOpen@8
	mov	eax,[ebp+12]
	push	eax
	call	_paulaMasterVol@4
	
	mov	byte [ebx + module.Opened],1

	popad
	pop	ebp
	ret	8h


align 4
;---------------------------------
_CHP_Close@0:
;---------------------------------
	pushad

	call	_paulaClose@0

	call	ClearGlobal

	popad
	ret



align 4
;---------------------------------
_CHP_Start@0:
;---------------------------------
	pushad
	lea	ebx,[CHPinfo]

	cmp	byte [ebx + module.Opened],0
	jz	short .exit
	
	mov	byte [ebx + module.Playing],1
	
	call	_paulaStart@0
	
.exit:	
	popad
	ret










align 4
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
	

	mov	al,[esi+0Fh]
	and	al,7Fh
	mov	[ebp + module.POSnum],eax
	mov	cl,al

	mov	al,[esi+06h]
	cmp	al,cl
	jb	short .noRSTAj
	xor	eax,eax
.noRSTAj:
	mov	[ebp + module.Restart],eax

	mov	al,[esi+07h]
	mov	[ebp + module.PATnum],eax

	mov	eax,[esi+08h]
	add	eax,esi
	mov	[ebp + module.SMPptr],eax
	
	
	movzx	eax,byte [esi+0Eh]
	mov	[ebp + module.INSnum],eax
	mov	bl,al


;---- POSITIONS

	mov	ecx, [ebp + module.POSnum]
	add	esi,10h			; skip header
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
	mov	[edi + instr.Len],eax
	add	eax,eax
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
	add	eax,[edi + instr.Start]
	mov	[edi + instr.LoopSta],eax
	
	movzx	eax,word [esi+6]	; LoopSize / 2
	mov	[edi + instr.LoopLen],eax
	
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
	lea	ebx,[VOCdata]
	xor	eax,eax
.inVoc:	mov	[ebx + chan.VoiceN],al
	add	ebx, chan.size
	inc	al
	cmp	al,4
	jl	short .inVoc


	
	xor	eax,eax
	mov	al,6
	mov	[ebp + module.tick],eax
	mov	[ebp + module.speed],eax
	mov	al,64
	mov	[ebp + module.row],eax
	mov	eax,[ebp + module.POSnum]
	mov	[ebp + module.ord],eax


	popad
	pop	ebp
	ret	4




;---------------------------
;---------------------------
align 4
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



; in: eax = instrument num
;out: eax = instrument ptr
align 4
;---------------------------
getInstrPtr:
;---------------------------
	push	ebx
	push	ecx
	push	edx
	
	mov	ecx,instr.size
	xor	edx,edx
	mul	ecx
	lea	ebx,[INSinfo]
	add	eax,ebx
	
	pop	edx
	pop	ecx
	pop	ebx
	ret


;out: eax = row ptr
;---------------------------
getRowPtr:
;---------------------------
	push	ecx
	push	edi
	
	lea	edi,[CHPinfo]
	lea	eax,[POSinfo]
	add	eax,[edi + module.ord]
	movzx	eax, byte [eax]
	shl	eax,10
	lea	ecx,[PATinfo]
	add	eax,ecx
	mov	ecx,[edi + module.row]
	shl	ecx,4
	add	eax,ecx

	pop	edi
	pop	ecx
	ret


; in: ecx = note
align 4
;---------------------------
updateLastPeriod:
;---------------------------
	pushad

	and	ecx,000000FFh
	add	ecx,ecx
	lea	eax,[Periods]
	add	eax,ecx
	movzx	eax,word [eax]
	mov	ecx,8363
	xor	edx,edx
	mul	ecx

	push	eax
	
	lea	edi,[VOCdata]
	mov	eax,[edi + chan.lastins]
	call	getInstrPtr
	mov	ecx,[eax + instr.C2Spd]

	pop	eax
	
	xor	edx,edx
	div	ecx

	mov	[edi + chan.lastper],eax

	popad
	ret

	
	
	


align 4
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
	call	Effects

.exit:
	popad
	ret





align 4
_CHP_Update:
	pushad
	lea	ebx,[CHPinfo]
	xor	eax,eax
	cmp	[ebx + module.Opened],eax
	jz	.exit
	cmp	[ebx + module.Playing],eax
	jz	.exit

	lea	ebp,[VOCdata]

	cmp	byte [ebp + chan.restart],0
	jnz	short .noSetVoice

	mov	al,0
	call	getInstrPtr
	mov	esi,eax
	
	
	push	dword [esi + instr.Volume]
	call	_paulaSetVolume@4
	
	push	dword 02D0h
	call	_paulaSetPeriod@4


	push	dword [esi + instr.Start]
	call	_paulaSetSamplePos@4
	
	push	dword [esi + instr.Len]
	call	_paulaSetSampleLen@4

	push	dword 1
	call	_paulaSetVoice@4

	push	dword [esi + instr.LoopSta]
	call	_paulaSetSamplePos@4

	push	dword [esi + instr.LoopLen]
	call	_paulaSetSampleLen@4

.exit:
.noSetVoice:
	popad
	ret
	

align 4
;---------------------------------------------------------	
;---------------------------------------------------------	
;---------------------------------------------------------	
NewNote:
;---------------------------------------------------------	
;---------------------------------------------------------	
;---------------------------------------------------------	
	pushad
	
	mov	edi,[CHPinfo]
	lea	ebp,[VOCdata]
	push	ebp
	
	xor	eax,eax
.doVoice:
	push	eax
	call	_paulaSelectVoice@4


	call	NewNote_CHN
	
	add	ebp, chan.size	

	inc	al
	cmp	al,4
	jl	short .doVoice
	
	
;-----------------------------------------------

	pop	ebp
	xor	eax,eax
.doVoice2:
	push	eax
	
	push	eax
	call	_paulaSelectVoice@4

	cmp	byte [ebp + chan.restart],0
	jz	short .noSetVoice

;--- PLAY VOICE STA ---

	mov	eax, [ebp + chan.lastins]
	call	getInstrPtr
	mov	esi,eax
	
	mov	eax,[ebp + chan.start]
	push	eax
	call	_paulaSetSamplePos@4
	
	sub	eax,[esi + instr.Start]
	mov	ebx,[esi + instr.Len]
	sub	ebx,eax
	push	ebx
	call	_paulaSetSampleLen@4
	push	dword 1
	call	_paulaSetVoice@4


	push	dword [esi + instr.LoopSta]
	call	_paulaSetSamplePos@4

	push	dword [esi + instr.LoopLen]
	call	_paulaSetSampleLen@4

;--- PLAY VOICE END ---

.noSetVoice:	
	add	ebp, chan.size	
	pop	eax
	inc	al
	cmp	al,4
	jl	short .doVoice2

	popad
	ret



align 4
;---------------------------------------------------------	
;---------------------------------------------------------	
;---------------------------------------------------------	
NewNote_CHN:
;---------------------------------------------------------	
;---------------------------------------------------------	
;---------------------------------------------------------	
	pushad

	call	getRowPtr
	mov	ecx,[ebp + chan.VoiceN]
	shl	ecx,2
	add	eax,ecx
	mov	ecx,[eax]			; ecx = Note (FXA FXC INS NOT)
	mov	[ebp + chan.current],ecx
	
	or	ch,ch				; Gotta instrument number?
	jz	short .skipIns
	
; INSTRUMENT GIVEN!

	movzx	eax,ch
	dec	eax
	mov	[ebp + chan.lastins],eax
	
	bswap	ecx
	mov	ax,cx				; ax = FXC FXA
	bswap	ecx

;	cmp	ah,0Eh
;	jne	short .skipIns
;	shr	al,4
;	cmp	al,0Dh
;	je	short .skipIns
	
	mov	eax,[ebp + chan.lastins]
	call	getInstrPtr
	mov	eax,[eax + instr.Volume]
	moV	[ebp + chan.Volume],eax
	
.skipIns:

;---

	xor	eax,eax
	mov	[ebp + chan.restart],eax	; default = dont retrig note!
	
	or	cl,cl
	jz	short .nonote
	dec	cl
	cmp	cl,36
	jae	short .nonote

	mov	[ebp + chan.lastnot],cl
	call	updateLastPeriod

	inc	al
	mov	[ebp + chan.restart],eax	; restart = 1, retrig note!

	mov	eax,[ebp + chan.lastins]
	call	getInstrPtr
	mov	eax,[eax + instr.Start]
	mov	[ebp + chan.start],eax





	xor	ebx,ebx
	mov	bl,0Fh
	mov	al,[ebp + chan.wavecon]
	push	eax
	and	al,bl
	cmp	al,4
	jae	short .noRSTvibpos
	mov	byte [ebp + chan.vibpos],bh
.noRSTvibpos:
	pop	eax
	shr	al,4
	cmp	al,4
	jae	short .noRSTtrmpos
	mov	byte [ebp + chan.trempos],bh
.noRSTtrmpos:




	bswap	ecx
	mov	al,ch
	bswap	ecx
	cmp	al,03h
	je	short .nonote
	cmp	al,05h
	je	short .nonote
	cmp	al,16h
	je	short .nonote
	
	mov	eax,[ebp + chan.lastper]
	mov	[ebp + chan.freq],eax
	
.nonote:
	
	
	push	eax
	push	ecx

	bswap	ecx
	movzx	eax,ch				; al = FXC
	
	lea	ecx,[FXtable1]			; point to FXtable1
	mov	eax,[ecx + eax*4]		; get function address
	call	eax				; call it!
	
	pop	ecx
	pop	eax


	bswap	ecx
	cmp	ch,07h
	je	short .noSetVol

	push	dword [ebp + chan.Volume]
	call	_paulaSetVolume@4

.noSetVol:

	mov	eax,[ebp + chan.freq]
	or	eax,eax
	jz	short .noSetFrq	
	push	eax
	call	_paulaSetPeriod@4
.noSetFrq:
	
	popad
	ret





align 4
;----------------------------
Effects:
;----------------------------
	pushad
	lea	edi,[CHPinfo]
	lea	ebp,[VOCdata]
	
	xor	eax,eax
.doVoice2:	
	push	eax

	push	eax
	call	_paulaSelectVoice@4
;----	
	
	

;----	
	add	ebp, chan.size	
	pop	eax
	inc	al
	cmp	al,4
	jl	short .doVoice2
	
	popad
	ret











align	16
FXtable1	dd      fx_null		; 0 - arpeggio
        	dd      fx_null         ; 1 - porta up
        	dd      fx_null         ; 2 - porta down
        	dd      fx_null		; 3 - tone porta
        	dd      fx_null		; 4 - vibrato
        	dd      fx_null         ; 5 - tone+slide
        	dd      fx_null         ; 6 - vibrato+slide
        	dd      fx_null         ; 7 - tremolo
	        dd      fx_null         ; 8 - unused
        	dd      fx_null         ; 9 - sample offset
	        dd      fx_null         ; A - volume slide
        	dd      fx_null         ; B - pattern jump
	        dd      fx_null         ; C - set volume
        	dd      fx_null         ; D - break pattern
        	dd      fx_null         ; E - extra effects
        	dd      fx_null         ; F - set speed

FXtable2	dd      fx_null		; 0 - arpeggio
        	dd      fx_null         ; 1 - porta up
        	dd      fx_null         ; 2 - porta down
        	dd      fx_null		; 3 - tone porta
        	dd      fx_null		; 4 - vibrato
        	dd      fx_null         ; 5 - tone+slide
        	dd      fx_null         ; 6 - vibrato+slide
        	dd      fx_null         ; 7 - tremolo
	        dd      fx_null         ; 8 - unused
        	dd      fx_null         ; 9 - sample offset
	        dd      fx_null         ; A - volume slide
        	dd      fx_null         ; B - pattern jump
	        dd      fx_null         ; C - set volume
        	dd      fx_null         ; D - break pattern
        	dd      fx_null         ; E - extra effects
        	dd      fx_null         ; F - set speed

align 4
;----------------------
fx_null:
;----------------------
	ret

