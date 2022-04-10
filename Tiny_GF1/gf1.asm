; SAMPLE PRECISION:	32:32
; MIXER  PRECISION:	32bit with LINEAR INTERPOLATION

;============
SECTION .bss
;============

;********** V O L U M E ************
; X of 128

GF1_MASTERVOL	equ 130

;************* P A N ***************
; Left		Right
; 000h <------> 0FFh

GF1_PAN1	equ 040h
GF1_PAN2	equ 0D0h
GF1_PAN3	equ 0C0h
GF1_PAN4	equ 050h

;***********************************


GF1_MIXFREQ	equ 44100
GF1_NUMCHN	equ 4					; 4 Channels
GF1_MAXVOL	equ 64

GF1_CLOCK	equ 3546895 ; PAL (NTSC = 3579545)	; GF1 Master Clock
GF1_BUFLEN	equ 882

GF1_MSAMPLESIZ	equ 8					; MIX: 32bit,Stereo = 8bytes x Sample
GF1_MBUFFSIZ	equ (GF1_BUFLEN*GF1_MSAMPLESIZ)

GF1_SAMPLESIZ	equ 4					; OUT: 16bit,Stereo = 4bytes x Sample
GF1_BUFSIZ	equ (GF1_BUFLEN*GF1_SAMPLESIZ)




struc KMixChn
	.Active		resd 1
	.VoiceN		resd 1
	
	.VolL		resd 1
	.VolR		resd 1
	.Volume		resd 1
	.Pan		resd 1

	.DRAMbase	resd 1
	
	.Loop		resd 1
	.SampleSta	resd 1
	.SampleEnd	resd 1
	.SampleLS	resd 1

	.Freq		resd 1
	.PosIndex_I	resd 1
	.PosIndex_F	resd 1

	.PosIncr_I	resd 1
	.PosIncr_F	resd 1

	.size
endstruc


Opened		resd 1
Playing		resd 1
Callback	resd 1
MixPtr		resd 1
MasterVol	resd 1
ActiveVoice	resd 1

MixChn		resb (GF1_NUMCHN * KMixChn.size)
align 16, resb 1
MixBuf		resb (GF1_BUFSIZ*10)	; 8820
align 16, resb 1
TmpBuf		resb GF1_MBUFFSIZ
allSize		equ $-Opened

DRAM		resb (256*1024)


extern	_DOut_Open@8
extern	_DOut_Start@0
extern	_DOut_Close@0
extern	_DOut_BufferCopy@4

global	_gf1Open@8
global	_gf1MasterVol@4
global	_gf1Start@0
global	_gf1Close@0
global	_gf1Reset@0
global	_gf1PokeBlock@12

global	_gf1SelectVoice@4
global	_gf1SetPeriod@4
global	_gf1SetVolume@4
global	_gf1SetSample@12
global	_gf1SetVoice@4


;============
SECTION .text
;============


_chMix_Pan	dd 255-GF1_PAN1, 255-GF1_PAN2, 255-GF1_PAN3, 255-GF1_PAN4




align 4
;----------------------------------
_gf1Open@8:
;----------------------------------
	push	ebp
	mov	ebp,esp
	pushad
	
	call	_gf1Reset@0

	mov	eax,[ebp+8]		; Play Routine
	mov	[Callback],eax

	mov	eax,[ebp+12]		; HWnd
	push	eax
	lea	eax,[_gf1Callback]
	push	eax
	call	_DOut_Open@8
	or	eax,eax
	jnz	.exit

	xor	ecx,ecx
	mov	cl,GF1_MASTERVOL
	mov	[MasterVol],ecx


	inc	eax	
	mov	[Opened],eax
	
	
.exit:
	popad
	pop	ebp
	ret	8h





; gf1PokeBlock(int dst, char *src, int size)
align 4
;----------------------------------
_gf1PokeBlock@12:
;----------------------------------
	push	ebp
	mov	ebp,esp
	pushad

	mov	edx,3FFFFh	; 256K-1

	mov	esi,[ebp+12]	; get src address
	lea	edi,[DRAM]
	mov	ebx,[ebp+8]	; get dstofs address

	mov	ecx,[ebp+16]	; get size
	and	ecx,edx		; mask size
	
	xor	eax,eax
	cld
.poke:
	lodsb
	and	ebx,edx
	mov	[edi+ebx],al
	inc	ebx
	loop	.poke	
	
	popad
	pop	ebp
	ret	12
	

align 4
;----------------------------------
_gf1MasterVol@4:
;----------------------------------
	push	ebp
	mov	ebp,esp
	pushad
	
	mov	eax,[ebp+8]
	mov	[MasterVol],eax
	
	popad
	pop	ebp
	ret	4


align 4
;----------------------------------
_gf1Start@0:
;----------------------------------
	xor	eax,eax
	cmp	[Opened],eax
	jnz	.okrun
	ret
.okrun:
	pushad

	inc	eax
	mov	[Playing],eax
	call	_DOut_Start@0
	
	popad
	ret


align 4
;----------------------------------
_gf1Close@0:
;----------------------------------
	xor	eax,eax
	cmp	[Opened],eax
	jnz	.okrun
	ret
.okrun:
	pushad

	mov	[Playing],eax
	call	_DOut_Close@0

	popad
	ret



align 4
;----------------------------------
_gf1Reset@0:
;----------------------------------
	pushad

; init voice
	lea	edi,[Opened]
	mov	ecx,allSize
	xor	eax,eax
	cld
	rep	stosb

	lea	esi,[_chMix_Pan]
	lea	edi,[MixChn]
	lea	ebx,[DRAM]
	mov	cx,0400h
.inich:	
	mov	[edi + KMixChn.VoiceN],cl
	mov	[edi + KMixChn.DRAMbase],ebx
	mov	[edi + KMixChn.Active],eax
	mov	[edi + KMixChn.SampleSta],eax
	mov	[edi + KMixChn.SampleEnd],eax
	mov	[edi + KMixChn.SampleLS],eax
	mov	edx,[esi]
	mov	[edi + KMixChn.Pan],edx
	
	add	esi,byte 4
	add	edi,byte KMixChn.size
	inc	cl
	cmp	cl,ch
	jl	.inich
	
	popad
	ret



align 4
; in: eax = chnNum, out: ebx = chnPtr
;----------------------------------
_SetChnAddr:
;----------------------------------
	push	eax
	push	ecx
	
	mov	eax,[ActiveVoice]	
	and	eax,3
	xor	ecx,ecx
	mov	cl,KMixChn.size
	mul	ecx
	lea	ebx,[MixChn]
	add	ebx,eax
	
	pop	ecx
	pop	eax
	ret


align 4
;----------------------------------
_gf1SelectVoice@4:
;----------------------------------
	push	ebp
	mov	ebp,esp
	push	eax

	mov	eax,[ebp+8]
	and	al,3
	mov	byte [ActiveVoice],al

	pop	eax
	pop	ebp
	ret	4h

align 4
;----------------------------------
_gf1SetPeriod@4:
;----------------------------------
	push	ebp
	mov	ebp,esp
	pushad

	
	call	_SetChnAddr	
	xor	ecx,ecx

	mov	eax,[ebp+8]		; Get Period
	and	eax,0000FFFFh
	jnz	.noZ
	
	mov	[ebx + KMixChn.Active],ecx
	jmp	short .exit
.noZ:
	mov	si,0040h
	cmp	ax,si
	jge	.noadjL
	mov	ax,si
.noadjL:
	mov	si,6B00h
	cmp	ax,si
	jle	.noadjU
	mov	ax,si
.noadjU:
	or	eax,eax
	jz	.exit
	
	mov	ecx,eax
	mov	eax,GF1_CLOCK
	xor	edx,edx
	idiv	ecx

	mov	[ebx + KMixChn.Freq],eax
	
	mov	ecx,GF1_MIXFREQ
	xor	edx,edx
	div	ecx
	mov	[ebx + KMixChn.PosIncr_I],eax
	xor	eax,eax
	div	ecx
	mov	[ebx + KMixChn.PosIncr_F],eax
	
.exit:
	popad
	pop	ebp
	ret	4h
	
	

align 4
;----------------------------------
_gf1SetVolume@4:
;----------------------------------
	push	ebp
	mov	ebp,esp
	pushad

	call	_SetChnAddr	

	movzx	eax,byte [ebp+8]	; Get Volume
	
	mov	cl,GF1_MAXVOL-1
	cmp	al,cl
	jbe	.noadjU
	mov	al,cl
.noadjU:
	mov	[ebx + KMixChn.Volume],eax

	popad
	pop	ebp
	ret	4h




; gf1SetSample(Sta,LS,End)
align 4
;----------------------------------
_gf1SetSample@12:
;----------------------------------
	push	ebp
	mov	ebp,esp
	pushad
	
	call	_SetChnAddr	

	mov	ecx,3FFFFh

	mov	eax,[ebp+8]			; Get Sample Sta
	and	eax,ecx
	mov	[ebx + KMixChn.SampleSta],eax

	mov	eax,[ebp+16]			; Get Sample End
	and	eax,ecx
	mov	[ebx + KMixChn.SampleEnd],eax

	mov	eax,[ebp+12]			; Get Sample LS
	and	eax,ecx
	mov	[ebx + KMixChn.SampleLS],eax
	
	mov	byte [ebx + KMixChn.Loop],1
	cmp	eax,[ebx + KMixChn.SampleEnd]
	jl	.looped	
	mov	byte [ebx + KMixChn.Loop],0
.looped:	
	popad
	pop	ebp
	ret	12
	
	
	


align 4
;----------------------------------
_gf1SetVoice@4:
;----------------------------------
	push	ebp
	mov	ebp,esp
	pushad

	call	_SetChnAddr	

	xor	eax,eax
	mov	[ebx + KMixChn.Active],eax
	
	mov	ecx,[ebp+8]		; Get ON/OFF
	or	ecx,ecx
	jz	.exit

	mov	[ebx + KMixChn.PosIndex_F],eax
	inc	eax
	mov	[ebx + KMixChn.Active],eax

	mov	eax,[ebx + KMixChn.SampleSta]
	mov	[ebx + KMixChn.PosIndex_I],eax
.exit:
	popad
	pop	ebp
	ret	4h








align 4
;---------------------------------------
; in: ebp = chnPtr
_asmMix16StereoNormal:
;---------------------------------------
	pushad
	jmp	short .ChkActive
.Deactivate:
	mov	byte [ebp + KMixChn.Active],0
.ExitMix:
	popad
	ret
.ChkActive:
	xor	eax,eax
	cmp	[ebp + KMixChn.Active],eax
	je	.ExitMix
	cmp	[ebp + KMixChn.Freq],eax
	je	.Deactivate
	
;------- Source & Destination ----

	mov	esi,[ebp + KMixChn.DRAMbase]
	lea	edi,[TmpBuf]
	
;----------- Get Volume ---

	mov	ecx,[ebp + KMixChn.Volume]

	mov	eax,[MasterVol]
	imul	ecx
	mov	ebx,eax

	mov	ecx,[ebp + KMixChn.Pan]
	imul	ecx
	shl	eax,2
	mov	[ebp + KMixChn.VolL],eax

	mov	eax,ebx
	mov	ecx,[ebp + KMixChn.Pan]
	xor	cl,0FFh
	imul	ecx
	shl	eax,2
	mov	[ebp + KMixChn.VolR],eax

;----------- Number of samples ---

	mov	ecx,GF1_BUFLEN


;----------- Get Sample Values ---

	jmp	short .ChkLup
.Gate1:
	jmp	short .Deactivate
	align	16
.Looped:
	cmp	byte [ebp + KMixChn.Loop],0
	je	.Gate1


	sub	ebx,[ebp + KMixChn.SampleEnd]
	dec	ebx
	add	ebx,[ebp + KMixChn.SampleLS]
	mov	[ebp + KMixChn.PosIndex_I],ebx


.ChkLup:
	mov	ebx,[ebp + KMixChn.PosIndex_I]
	cmp	ebx,[ebp + KMixChn.SampleEnd]
	jg	.Looped

	push	ecx

; * Linear Interpolation *
	mov	ecx,[ebp + KMixChn.PosIndex_F]		; 1
	shr	ecx,18					; -	get highest 16bits
	movsx	edx,byte [esi+ebx]			; -
	movsx	eax,byte [esi+ebx+1]			; 2
	sub	eax,edx					; 3
	imul	eax,ecx					; 13
	sar	eax,16					; 14
	add	edx,eax					; 15
	mov	eax,edx					; 16

; * No Interpolation *
;	movsx	eax,byte [esi+ebx]			; -
;	mov	edx,eax




	imul	edx,[ebp + KMixChn.VolL]		; 26
	add	[edi],edx				; 27

	imul	eax,[ebp + KMixChn.VolR]		; 37
	add	[edi+(GF1_MSAMPLESIZ/2)],eax		; 38



	mov	eax,[ebp + KMixChn.PosIndex_F]		; 39
	mov	ebx,[ebp + KMixChn.PosIndex_I]		; -
	add	eax,[ebp + KMixChn.PosIncr_F]		; 40
	adc	ebx,[ebp + KMixChn.PosIncr_I]		; -
	mov	[ebp + KMixChn.PosIndex_F],eax		; 41
	mov	[ebp + KMixChn.PosIndex_I],ebx		; -

	add	edi,byte GF1_MSAMPLESIZ			; 42

	pop	ecx					; 43
	dec	ecx					; 44
	jnz	.ChkLup					; -
	
	popad
	ret
	


align 4
;-----------------------------------------------------------------------
_MixBuffer:
;-----------------------------------------------------------------------
	pushad
	
	; 1) Clear Buffer
	lea	edi,[TmpBuf]
	mov	ecx,GF1_MBUFFSIZ/4
	xor	eax,eax
	cld
	rep	stosd

	; 2) Call PlayRoutine
	mov	eax,[Callback]
	or	eax,eax
	jz	.NoCall
	pushad
	call	eax
	popad
.NoCall:

	lea	ebp,[MixChn]
	xor	ecx,ecx
	mov	cl,4
.mixchns:	
	
	call	_asmMix16StereoNormal
	add	ebp,byte KMixChn.size
	
	loop	.mixchns


	popad
	ret
	

align 4
;-----------------------------------------------------------------------
_Conv32to16:
;-----------------------------------------------------------------------
	pushad
	
	lea	esi,[TmpBuf]
	mov	edi,[MixPtr]
	
	xor	eax,eax
	mov	ax,32767
	mov	ebx,eax
	mov	ebp,eax
	neg	ebp

	mov	ecx,GF1_BUFLEN
	add	ecx,ecx
.C32to16:
	mov	eax,[esi]
	sar	eax,16
	
	cmp	eax,ebx
	jle	.njUPL
	mov	eax,ebx
.njUPL:	cmp	eax,ebp
	jge	.njDNL
	mov	eax,ebp
.njDNL:
	mov	[edi],ax
	
	add	esi,byte (GF1_MSAMPLESIZ/2)
	add	edi,byte (GF1_SAMPLESIZ/2)
	loop	.C32to16

	popad
	ret



;-----------------------------------------------------------------------
;_FirMemory:
;-----------------------------------------------------------------------
;	pushad
;	lea	edi,[TmpBuf]
;	add	edi,GF1_MSAMPLESIZ
;	mov	si,2
;.firs:
;	push	edi
;	mov	ecx,GF1_BUFLEN-1
;.fir1:
;	mov	edx, [edi-GF1_MSAMPLESIZ]
;	mov	eax, [edi]
;	mov	ebx, [edi+GF1_MSAMPLESIZ]
;	sar	edx,2
;	sar	eax,1
;	sar	ebx,2
;	add	eax,edx
;	add	eax,ebx
;	mov	[edi],eax
;	add	edi,GF1_MSAMPLESIZ
;	loop	.fir1
;	pop	edi
;	add	edi,GF1_MSAMPLESIZ/2
;	dec	si
;	jnz	.firs
;	popad
;	ret



align 4
;-----------------------------------------------------------------------
_gf1Callback:
;-----------------------------------------------------------------------
	cmp	byte [Opened],0
	jnz	.okrun
	ret
.okrun:
	pushad

	lea	eax,[MixBuf]
	mov	[MixPtr],eax
	
	mov	ecx,10
.MixAll10:

	call	_MixBuffer

;	call	_FirMemory

	call	_Conv32to16
	
	add	dword [MixPtr],GF1_BUFSIZ
	loop	.MixAll10



	lea	eax,[MixBuf]
	push	eax
	call	_DOut_BufferCopy@4
		
	popad		
	ret




