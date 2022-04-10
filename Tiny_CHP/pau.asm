; SAMPLE PRECISION:	32:32
; MIXER  PRECISION:	32bit with LINEAR INTERPOLATION

;============
SECTION .bss
;============

;********** V O L U M E ************
; X of 128

;PAU_MASTERVOL	equ 128

;************* P A N ***************
; Left		Right
; 000h <------> 0FFh

PAU_PAN1	equ 0D0h
PAU_PAN2	equ 040h
PAU_PAN3	equ 050h
PAU_PAN4	equ 0C0h

;***********************************


PAU_MIXFREQ	equ 44100
PAU_NUMCHN	equ 4					; 4 Channels
PAU_MAXVOL	equ 64

PAU_CLOCK	equ 3546895 ; PAL (NTSC = 3579545)	; Paula Master Clock
PAU_BUFFLEN	equ 882

PAU_MSAMPLESIZ	equ 8					; MIX: 32bit,Stereo = 8bytes x Sample
PAU_MBUFFSIZ	equ (PAU_BUFFLEN*PAU_MSAMPLESIZ)

PAU_SAMPLESIZ	equ 4					; OUT: 16bit,Stereo = 4bytes x Sample
PAU_BUFFSIZ	equ (PAU_BUFFLEN*PAU_SAMPLESIZ)




struc KMixChn
	.Active		resd 1
	.VoiceN		resd 1
	
	.VolL		resd 1
	.VolR		resd 1
	.Volume		resd 1
	.Pan		resd 1

	.SamplePos	resd 1
	.SampleLen	resd 1
	.SamplePos_i	resd 1
	.SampleLen_i	resd 1

	.Freq		resd 1
	.PosIndex_I	resd 1
	.PosIndex_F	resd 1
	.PosIncr_I	resd 1
	
	.PosIncr_F	resd 1
	.PosLimit	resd 1	

	.size
endstruc


Opened		resd 1
Playing		resd 1
Callback	resd 1
MixPtr		resd 1
MasterVol	resd 1

MixChn		resb (PAU_NUMCHN * KMixChn.size)
nullDATA	resb 16
align 16, resb 1
MixBuf		resb (PAU_BUFFSIZ*10)	; 8820
align 16, resb 1
TmpBuf		resb PAU_MBUFFSIZ
ActiveVoice	resd 1
allSize		equ $-Opened



extern	_DOut_Open@8
extern	_DOut_Start@0
extern	_DOut_Close@0
extern	_DOut_BufferCopy@4

global	_paulaOpen@8
global	_paulaMasterVol@4
global	_paulaStart@0
global	_paulaClose@0
global	_paulaReset@0

global	_paulaSelectVoice@4
global	_paulaSetPeriod@4
global	_paulaSetVolume@4
global	_paulaSetSamplePos@4
global	_paulaSetSampleLen@4
global	_paulaSetVoice@4
;global	_paulaDMACON@4


;============
SECTION .code
;============


_chMix_Pan	dd PAU_PAN1, PAU_PAN2, PAU_PAN3, PAU_PAN4


align 4
;----------------------------------
_paulaOpen@8:
;----------------------------------
	push	ebp
	mov	ebp,esp
	pushad
	
	call	_paulaReset@0

	mov	eax,[ebp+8]		; Play Routine
	mov	[Callback],eax

	mov	eax,[ebp+12]		; HWnd
	push	eax
	lea	eax,[_paulaCallback]
	push	eax
	call	_DOut_Open@8
	or	eax,eax
	jnz	.exit

	inc	eax	
	mov	[Opened],eax

.exit:
	popad
	pop	ebp
	ret	8h


align 4
;----------------------------------
_paulaMasterVol@4:
;----------------------------------
	push	ebp
	mov	ebp,esp
	pushad
	
	mov	eax,[ebp+8]
	mov	[MasterVol],eax
	
	popad
	pop	ebp
	ret	4h


align 4
;----------------------------------
_paulaStart@0:
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
_paulaClose@0:
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
_paulaReset@0:
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
	lea	eax,[nullDATA]
	mov	cx,0400h
.inich:	
	mov	[edi + KMixChn.VoiceN],cl
	mov	[edi + KMixChn.SamplePos],eax
	mov	[edi + KMixChn.SamplePos_i],eax
	mov	ebx,[esi]
	mov	[edi + KMixChn.Pan],ebx
	
	add	esi,4
	add	edi,KMixChn.size
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
	mov	ecx,KMixChn.size
	mul	ecx
	lea	ebx,[MixChn]
	add	ebx,eax
	
	pop	ecx
	pop	eax
	ret


align 4
;----------------------------------
_paulaSelectVoice@4:
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
_paulaSetPeriod@4:
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
	jmp	.exit
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
	mov	eax,PAU_CLOCK
	xor	edx,edx
	idiv	ecx

	mov	[ebx + KMixChn.Freq],eax
.exit:
	popad
	pop	ebp
	ret	4h
	
	

align 4
;----------------------------------
_paulaSetVolume@4:
;----------------------------------
	push	ebp
	mov	ebp,esp
	pushad

	call	_SetChnAddr	

	movzx	eax,byte [ebp+8]	; Get Volume
	
	mov	cl,PAU_MAXVOL-1
	cmp	al,cl
	jbe	.noadjU
	mov	al,cl
.noadjU:
	mov	[ebx + KMixChn.Volume],eax

	popad
	pop	ebp
	ret	4h




align 4
;----------------------------------
_paulaSetSamplePos@4:
;----------------------------------
	push	ebp
	mov	ebp,esp
	pushad
	
	call	_SetChnAddr	

	mov	eax,[ebp+8]			; Get Sample Address

	or	eax,eax
	jz	.exit
	mov	[ebx + KMixChn.SamplePos],eax
.exit:	
	popad
	pop	ebp
	ret	4h

	
align 4
;----------------------------------
_paulaSetSampleLen@4:
;----------------------------------
	push	ebp
	mov	ebp,esp
	pushad

	call	_SetChnAddr	
	xor	ecx,ecx

	mov	eax,[ebp+8]		; Get Len
;	add	eax,eax
	mov	[ebx + KMixChn.SampleLen],eax
.exit:
	popad
	pop	ebp
	ret	4h
	



align 4
;----------------------------------
_paulaSetVoice@4:
;----------------------------------
	push	ebp
	mov	ebp,esp
	pushad

	call	_SetChnAddr	

	xor	ecx,ecx
	mov	eax,[ebp+8]		; Get ON/OFF
	or	eax,eax
	jnz	.PlayVoice
.StopVoice:
	mov	[ebx + KMixChn.Active],ecx
	jmp	.exit
.PlayVoice:
	mov	[ebx + KMixChn.PosIndex_I],ecx
	mov	[ebx + KMixChn.PosIndex_F],ecx

	inc	ecx

	mov	eax,[ebx + KMixChn.SamplePos]
	mov	[ebx + KMixChn.SamplePos_i],eax

	mov	eax,[ebx + KMixChn.SampleLen]
	cmp	eax,4
	jae	short .noStopV
	
	xor	ecx,ecx
	xor	eax,eax
.noStopV:
	mov	[ebx + KMixChn.SampleLen_i],eax
	mov	[ebx + KMixChn.Active],ecx
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
	jmp	.ChkActive
.Deactivate:
	mov	byte [ebp + KMixChn.Active],0
.ExitMix:
	popad
	ret
.ChkActive:	
	xor	eax,eax
	cmp	[ebp + KMixChn.Active],al
	jz	.ExitMix
	cmp	[ebp + KMixChn.Volume],eax
	jz	.ExitMix
	cmp	[ebp + KMixChn.SampleLen_i],eax
	jz	.ExitMix
	cmp	[ebp + KMixChn.Freq],eax
	jz	.ExitMix
	
;------- Source & Destination ----

	mov	esi,[ebp + KMixChn.SamplePos_i]
	lea	edi,[TmpBuf]

;----------- Get Position Increment ---

	mov	ecx,PAU_MIXFREQ
	mov	eax,[ebp + KMixChn.Freq]
	xor	edx,edx
	div	ecx
	mov	[ebp + KMixChn.PosIncr_I],eax
	xor	eax,eax
	div	ecx
	mov	[ebp + KMixChn.PosIncr_F],eax
	
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

	mov	ecx,PAU_BUFFLEN


;----------- Get Sample Values ---

	mov	eax,[ebp + KMixChn.SampleLen_i]
	cmp	eax,3
	jl	.Deactivate
	mov	[ebp + KMixChn.PosLimit],eax

	jmp	.ChkLup
.Gate1:
	jmp	.Deactivate
	align	16
.Looped:
	mov	eax,[ebp + KMixChn.PosLimit]
	mov	ebx,[ebp + KMixChn.SamplePos]
	sub	[ebp + KMixChn.PosIndex_I],eax
	mov	[ebp + KMixChn.SamplePos_i],ebx

	mov	eax,[ebp + KMixChn.SampleLen]
	cmp	eax,3
	jl	.Gate1

	mov	[ebp + KMixChn.SampleLen_i],eax
	mov	[ebp + KMixChn.PosLimit],eax

.ChkLup:
	mov	ebx,[ebp + KMixChn.PosIndex_I]
	
	cmp	ebx,[ebp + KMixChn.PosLimit]
	jge	.Looped

	push	ecx


; * Linear Interpolation *
;	mov	ecx,[ebp + KMixChn.PosIndex_F]		; 1
;	movsx	edx,byte [esi+ebx]			; -
;	movsx	eax,byte [esi+ebx+1]			; 2
;	shr	ecx,18					; -
;	sub	eax,edx					; 3
;	imul	eax,ecx					; 13
;	sar	eax,16					; 14
;	add	edx,eax					; 15
;	mov	eax,edx					; 16

; * No Interpolation *
	movsx	eax,byte [esi+ebx]			; -
	mov	edx,eax





	imul	edx,[ebp + KMixChn.VolL]		; 26
	add	[edi],edx				; 27

	imul	eax,[ebp + KMixChn.VolR]		; 37
	add	[edi+(PAU_MSAMPLESIZ/2)],eax		; 38



	mov	ecx,[ebp + KMixChn.PosIndex_F]		; 39
	mov	eax,[ebp + KMixChn.PosIndex_I]		; -
	add	ecx,[ebp + KMixChn.PosIncr_F]		; 40
	adc	eax,[ebp + KMixChn.PosIncr_I]		; -
	mov	[ebp + KMixChn.PosIndex_F],ecx		; 41
	mov	[ebp + KMixChn.PosIndex_I],eax		; -

	add	edi,PAU_MSAMPLESIZ			; 42

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
	mov	ecx,PAU_MBUFFSIZ/4
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
	add	ebp,KMixChn.size
	
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

	mov	ecx,PAU_BUFFLEN
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
	
	add	esi,PAU_MSAMPLESIZ/2
	add	edi,PAU_SAMPLESIZ/2
	loop	.C32to16

	popad
	ret



;-----------------------------------------------------------------------
;_FirMemory:
;-----------------------------------------------------------------------
;	pushad
;	lea	edi,[TmpBuf]
;	add	edi,PAU_MSAMPLESIZ
;	mov	si,2
;.firs:
;	push	edi
;	mov	ecx,PAU_BUFFLEN-1
;.fir1:
;	mov	edx, [edi-PAU_MSAMPLESIZ]
;	mov	eax, [edi]
;	mov	ebx, [edi+PAU_MSAMPLESIZ]
;	sar	edx,2
;	sar	eax,1
;	sar	ebx,2
;	add	eax,edx
;	add	eax,ebx
;	mov	[edi],eax
;	add	edi,PAU_MSAMPLESIZ
;	loop	.fir1
;	pop	edi
;	add	edi,PAU_MSAMPLESIZ/2
;	dec	si
;	jnz	.firs
;	popad
;	ret



align 4
;-----------------------------------------------------------------------
_paulaCallback:
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
	
	add	dword [MixPtr],PAU_BUFFSIZ
	loop	.MixAll10



	lea	eax,[MixBuf]
	push	eax
	call	_DOut_BufferCopy@4
		
	popad		
	ret




