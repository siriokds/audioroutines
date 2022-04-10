;============
SECTION .bss
;============


struc	instr
	.Start		resd 1
	.Len		resd 1
	.RepSta		resd 1
	.RepLen		resd 1
	.size
endstruc


struc	chan
	.ON		resd 1
	.PatPtr		resd 1
	.PatOfs		resd 1
	.Period		resw 1
	.Volume		resb 1
	.NoteTR		resb 1
	.SampTR		resb 1
	.VoiceN		resb 1
	.Note		resb 1
	.Info		resb 1
	.PortaV		resb 1
	.PortaX		resb 1
	.VolTspd	resb 1
	.VolTcnt	resb 1
	.VolPtr		resd 1
	.VolOfs		resd 1
	.FrqPtr		resd 1
	.FrqOfs		resd 1
	.VolSus		resb 1
	.FrqSus		resb 1
	.VibSpd		resb 1
	.VibAmp		resb 1
	.VibAmpC	resb 1
	.VibDel		resb 1
	.VibXOR		resb 1
	.PBend		resb 1
	.PBendC		resb 1
	.PBendX		resb 1
	.FreqTR		resb 1
	.VBendS		resb 1
	.VBendC		resb 1
	.VBendX		resb 1
	.SamplN		resb 1
	.size
endstruc



struc	module
	.ModAddress	resd 1
	.Opened		resd 1
	.Playing	resd 1
	.REPcnt		resd 1
	.REPspd		resd 1

	.SEQendp	resd 1
	.SEQoffs	resd 1
	.SEQnext	resd 1 

	.SEQnum		resd 1
	.PATnum		resd 1
	.VOLnum		resd 1
	.FRQnum		resd 1
	.SMPaddr 	resd 1
	.WAVaddr 	resd 1
	.size
endstruc


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

global	_FCp_Open@4
global	_FCp_InitModule@4
global	_FCp_Start@0
global	_FCp_Close@0


fcdat:	 resb module.size
VXdata:	 resb (chan.size*4)
SNDinfo: resb (instr.size*128)
SEQinfo: resb (256*16)
PATinfo: resb (256*64)
VOLinfo: resb (256*64)
FRQinfo: resb (256*64)
SMPinfo: resb (80*1024)
sizeAll  equ $-fcdat


;============
SECTION .code
;============


	dw 1ac0h,1940h,17d0h,1680h,1530h,1400h,12e0h,11d0h,10d0h,0fe0h,0f00h,0e28h
PERIODS	dw 06b0h,0650h,05f4h,05a0h,054ch,0500h,04b8h,0474h,0434h,03f8h,03c0h,038ah
	dw 0358h,0328h,02fah,02d0h,02a6h,0280h,025ch,023ah,021ah,01fch,01e0h,01c5h
	dw 01ach,0194h,017dh,0168h,0153h,0140h,012eh,011dh,010dh,00feh,00f0h,00e2h
	dw 00d6h,00cah,00beh,00b4h,00aah,00a0h,0097h,008fh,0087h,007fh,0078h,0071h
	dw 0071h,0071h,0071h,0071h,0071h,0071h,0071h,0071h,0071h,0071h,0071h,0071h
	
;	dw 1ac0h,1940h,17d0h,1680h,1530h,1400h,12e0h,11d0h,10d0h,0fe0h,0f00h,0e28h
	dw 0d60h,0ca0h,0be8h,0b40h,0a98h,0a00h,0970h,08e8h,0868h,07f0h,0780h,0714h
	dw 06b0h,0650h,05f4h,05a0h,054ch,0500h,04b8h,0474h,0434h,03f8h,03c0h,038ah
	dw 0358h,0328h,02fah,02d0h,02a6h,0280h,025ch,023ah,021ah,01fch,01e0h,01c5h
	dw 01ach,0194h,017dh,0168h,0153h,0140h,012eh,011dh,010dh,00feh,00f0h,00e2h
	dw 00d6h,00cah,00beh,00b4h,00aah,00a0h,0097h,008fh,0087h,007fh,0078h,0071h
	dw 0071h,0071h,0071h,0071h,0071h,0071h,0071h,0071h,0071h,0071h,0071h,0071h





;------------------------------------------------------------------------
ClearGlobal:
;------------------------------------------------------------------------
	pushad
	mov	edi, fcdat
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
_FCp_Open@4:
;------------------------------------------------------------------------
	push	ebp
	mov	ebp,esp
	pushad
	
	mov	ebx, fcdat
	
	call	ClearGlobal

	mov	eax,[ebp+8]
	push	eax
	lea	eax,[FCp_Update]
	push	eax
	call	_paulaOpen@8
	
	mov	byte [ebx + module.Opened],1

	popad
	pop	ebp
	ret	4h


;---------------------------------
_FCp_Close@0:
;---------------------------------
	pushad

	call	_paulaClose@0
	call	ClearGlobal

	popad
	ret



;---------------------------------
_FCp_Start@0:
;---------------------------------
	pushad
	mov	ebx, fcdat

	cmp	byte [ebx + module.Opened],0
	jz	.exit
	call	_paulaStart@0
	mov	byte [ebx + module.Playing],1
.exit:	
	popad
	ret










;-------------------------------------------------
_FCp_InitModule@4:
;-------------------------------------------------
	push	ebp
	mov	ebp,esp
	pushad
	mov	ebx, fcdat
	
	mov	esi,[ebp+8]
	mov	[ebx + module.ModAddress],esi
	mov	ebp,ebx


	mov	eax,[esi+12]
	bswap	eax
	shr	eax,6
	mov	[ebp + module.PATnum],al

	
	mov	eax,[esi+20]
	bswap	eax
	shr	eax,6
	mov	[ebp + module.FRQnum],al

	
	mov	eax,[esi+28]
	bswap	eax
	shr	eax,6
	mov	[ebp + module.VOLnum],al

	
	mov	eax,[esi+4]
	bswap	eax
	mov	ecx,13
	xor	edx,edx
	div	ecx
	mov	[ebp + module.SEQnum],eax
	shl	eax,4
	mov	[ebp + module.SEQendp],eax
		


	mov	al,0E1h
	lea	edi,[VOLinfo]
	mov	[edi+(64*255)], al
	lea	edi,[FRQinfo]
	mov	[edi+(64*255)], al


	push	esi
	
	lea	esi,[esi+180]
	lea	edi,[SEQinfo]
	mov	edx,[ebp + module.SEQnum]
.CopySEQ:	
	mov	ecx,13
	call	CopyMem
	
	add	edi,16
	add	esi,13
	dec	edx
	jnz	.CopySEQ

	pop	ebx

	
	mov	esi,[ebx+8]
	bswap	esi
	add	esi,ebx
	lea	edi,[PATinfo]
	mov	ecx,[ebp + module.PATnum]
	shl	ecx,6
	call	CopyMem
	
	mov	esi,[ebx+16]
	bswap	esi
	add	esi,ebx
	lea	edi,[FRQinfo]
	mov	ecx,[ebp + module.FRQnum]
	shl	ecx,6
	call	CopyMem

	mov	esi,[ebx+24]
	bswap	esi
	add	esi,ebx
	lea	edi,[VOLinfo]
	mov	ecx,[ebp + module.VOLnum]
	shl	ecx,6
	call	CopyMem

	mov	eax,[ebx+32]
	bswap	eax
	add	eax,ebx
	mov	[ebp + module.SMPaddr],eax


;//--------------------------------------------------------
;// Load Samples

	push	ebx

	lea	ebx,[ebx+40]
	lea	edi,[SNDinfo]
	xor	eax,eax
	mov	cl,10
	xor	edx,edx				; edx = Start

.LoadSMP:
	mov	[edi + instr.Start],edx

	mov	ax,[ebx]
	xchg	ah,al
	mov	[edi + instr.Len],ax

	cmp	ax,1
	jle	.noadj
	
	lea	eax,[(eax*2)+2]
	add	edx,eax
.noadj:

		
	mov	ax,[ebx+2]
	xchg	ah,al
	mov	[edi + instr.RepSta],ax
	

	mov	ax,[ebx+4]
	xchg	ah,al
	mov	[edi + instr.RepLen],ax
	
	add	edi,instr.size
	add	ebx,6
	
	dec	cl
	jnz	.LoadSMP
	

;//--------------------------------------------------------
;// Load Waves

	pop	ebx
	
	lea	ebx,[ebx+100]
	xor	eax,eax
	xor	esi,esi
	mov	cl,80

.LoadWAV:
	mov	[edi + instr.Start],edx
	mov	al,[ebx]
	cmp	ax,80h
	jle	.nofit
	mov	ax,80h
.nofit:
	mov	[edi + instr.Len],ax
	mov	[edi + instr.RepSta],esi
	mov	[edi + instr.RepLen],ax

	cmp	ax,1
	jle	.noadj2
	
	lea	eax,[eax*2]
	add	edx,eax
.noadj2:

	add	edi,instr.size
	inc	ebx
	
	dec	cl
	jnz	.LoadWAV


;	mov	ecx,(80*1024)
	mov	ecx,edx
	lea	edi,[SMPinfo]
	mov	esi,[ebp + module.SMPaddr]
	call	CopyMem


;//--------------------------------------------------------
;// get replay speed, if 0 use default value (3);

	mov	ax,0301h
	mov	[ebp + module.REPcnt],al
	mov	[ebp + module.REPspd],ah

	lea	esi,[SEQinfo]
	mov	al,[esi]
	or	al,al
	jz	.noz
	mov	[ebp + module.REPspd],al
.noz:

	mov	eax,[ebp + module.SEQendp]
	mov	[ebp + module.SEQoffs],eax	


;-----

	xor	eax,eax	
	lea	edi,[SEQinfo]
	mov	al,[edi]
	shl	eax,6
	lea	esi,[PATinfo]
	add	esi,eax
	mov	bx,[edi+1]

	mov	edx,(64*255)
	lea	edi,[VXdata]
	xor	ecx,ecx
.InitVocs:		
	mov	byte [edi + chan.ON],1
	mov	[edi + chan.VoiceN],cl
	
	mov	byte [edi + chan.VolTspd],1
	mov	byte [edi + chan.VolTcnt],1
	mov	[edi + chan.PatPtr],esi
	mov	dword [edi + chan.PatOfs],64
	mov	[edi + chan.NoteTR],bl
	mov	[edi + chan.NoteTR],bh
	
	lea	eax,[VOLinfo]
	add	eax,edx
	mov	[edi + chan.VolPtr],eax

	lea	eax,[FRQinfo]
	add	eax,edx
	mov	[edi + chan.FrqPtr],eax

	add	edi,chan.size

	inc	cl
	cmp	cl,4
	jl	.InitVocs


	popad
	pop	ebp
	ret	4








;---------------------------
FCp_Update:
;---------------------------
	pushad
	mov	ebx, fcdat

	xor	eax,eax
	cmp	[ebx + module.Opened],eax
	jz	.exit
	cmp	[ebx + module.Playing],eax
	jz	.exit
;----
	dec	byte [ebx + module.REPcnt]
	jnz	.dofx
	
	mov	al,[ebx + module.REPspd]
	mov	[ebx + module.REPcnt],al

	mov	byte [ebx + module.SEQnext],4
	
	xor	ecx,ecx
	lea	eax,[VXdata]
;	add	eax,chan.size*2
	mov	cl,4
.ntchn:	call	NewNote
	add	eax,chan.size
	loop	.ntchn
.dofx:
	lea	eax,[VXdata]
;	add	eax,chan.size*2
	mov	cl,4
.fxchn:	call	Effects
	add	eax,chan.size
	loop	.fxchn

.exit:
	popad
	ret








;----------------------------
NewNote:
;----------------------------
	pushad
	mov	ebp,eax
	mov	edi,fcdat

	movzx	eax,byte [ebp + chan.VoiceN]
	push	eax
	call	_paulaSelectVoice@4


	xor	edx,edx
	
	mov	esi,[ebp + chan.PatPtr]
	add	esi,[ebp + chan.PatOfs]

	cmp	dword [ebp + chan.PatOfs],64
	jge	patend

	cmp	byte [esi],049h
	jne	samepat


patend:

	mov 	[ebp + chan.PatOfs],edx
	
	inc	byte [edi + module.SEQnext]
	cmp	byte [edi + module.SEQnext],3
	jle	.skip1
	mov	[edi + module.SEQnext],dl

	add	dword [edi + module.SEQoffs],16

	mov	ebx,[edi + module.SEQoffs]
	cmp	ebx,[edi + module.SEQendp]
	jl	.skip2

	mov	[edi + module.SEQoffs],edx
	mov	ebx,edx

.skip2:

	lea	eax,[SEQinfo]
	add	eax,ebx
	mov	al,[eax+12]
	
	or	al,al
	jz	.skip1

	mov	[edi + module.REPspd],al
	mov	[edi + module.REPcnt],al
.skip1:

	movzx	eax,byte [ebp + chan.VoiceN]
	lea	eax,[eax*2+eax]			; VoiceN * 3
	lea	esi,[SEQinfo]
	add	esi,eax				; SEQinfo
	add	esi,[edi + module.SEQoffs]	; esi = SEQinfo + SEQoffs + (Voice * 3)

	movzx	eax,byte [esi]			; Pattern Num
	shl	eax,6				; Pattern Ofs
	lea	ecx,[PATinfo]
	add	eax,ecx				; +PATinfo
	mov	[ebp + chan.PatPtr],eax
	mov	ax,[esi+1]
	mov	[ebp + chan.NoteTR],al
	mov	[ebp + chan.SampTR],ah

;------------------------------------------------
samepat:
	mov	esi,[ebp + chan.PatPtr]
	add	esi,[ebp + chan.PatOfs]
	mov	bl,[esi+1]			; Get info byte #1
	mov	al,[esi]			; Get note
	or	al,al
	jnz	.ww1
	and	bl,11000000b
	jz	.noport
	jmp	.ww11
.ww1:
	mov	[ebp + chan.Period],dx
.ww11:
	mov	[ebp + chan.PortaV],dl
	test	bl,80h
	jz	.noport
	mov	cl,[esi+3]
	mov	[ebp + chan.PortaV],cl
.noport:
	and	ax,007Fh
	jz	NEAR nextnote
	mov	[ebp + chan.Note],al
	mov	bl,[esi+1]
	mov	[ebp + chan.Info],bl

	push	dword 0
	call	_paulaSetVoice@4
	
	and	ebx,0000003Fh
	add	bl,[ebp + chan.SampTR]
	
	mov	[ebp + chan.VolOfs],edx
	mov	[ebp + chan.VolSus],dl
	mov	[ebp + chan.FrqOfs],edx
	mov	[ebp + chan.VolSus],dl

	cmp	bl,byte [edi + module.VOLnum]
	jl	.nozer

	mov	[ebp + chan.VolTspd],dl
	mov	[ebp + chan.VolTcnt],dl
	mov	[ebp + chan.VibSpd],dl
	mov	[ebp + chan.VibXOR],dl
	mov	[ebp + chan.VibAmp],dl
	mov	[ebp + chan.VibDel],dl
	
	lea	eax,[VOLinfo]
	add	eax,(64*255)
	mov	[ebp + chan.VolPtr],eax

	lea	eax,[FRQinfo]
	add	eax,(64*255)
	mov	[ebp + chan.FrqPtr],eax
	jmp	nextnote
.nozer:
	cld
	shl	ebx,6
	lea	esi,[VOLinfo]
	add	esi,ebx
		
	mov	al,[esi]
	mov	[ebp + chan.VolTcnt],al
	mov	[ebp + chan.VolTspd],al

	movzx	ecx,byte [esi+1]

	mov	al,[edi + module.FRQnum]
	cmp	cl,al
	jle	.noctrl1
	dec	al
	mov	cl,al
.noctrl1:

	mov	eax,[esi+2]			; eax = (5,4,3,2)
	mov	[ebp + chan.VibSpd],al		; put 2
	mov	byte [ebp + chan.VibXOR],40h
	mov	[ebp + chan.VibAmp],ah		; put 3
	mov	[ebp + chan.VibAmpC],ah		; put 3
	bswap	eax				; eax = (2,3,4,5)
	mov	[ebp + chan.VibDel],ah		; put 4

;	mov	al,[esi+2]
;	mov	[ebp + chan.VibSpd],al
;	mov	byte [ebp + chan.VibXOR],40h
;	mov	al,[esi+3]	
;	mov	[ebp + chan.VibAmp],al
;	mov	[ebp + chan.VibAmpC],al
;	mov	al,[esi+4]
;	mov	[ebp + chan.VibDel],ah

	add	esi,5
	mov	[ebp + chan.VolPtr],esi


	lea	eax,[FRQinfo]
	shl	ecx,6
	add	eax,ecx
	mov	[ebp + chan.FrqPtr],eax

nextnote:
	add	dword [ebp + chan.PatOfs],2
	popad
	ret








;----------------------------
Effects:
;----------------------------
	pushad
	mov	ebp,eax
	mov	edi,fcdat
	xor	edx,edx

	movzx	eax,byte [ebp + chan.VoiceN]
	push	eax
	call	_paulaSelectVoice@4

;*************************************************************************
; FREQUENCY EFFECTS
;*************************************************************************
	xor	ecx,ecx
testsustain:
	cmp	byte [ebp + chan.FrqSus],0
	jz	sustzero	
	dec	byte [ebp + chan.FrqSus]
	jmp	VOLUfx
sustzero:
	mov	esi, [ebp + chan.FrqPtr]
	add	esi, [ebp + chan.FrqOfs]
testeffects:
	cmp	byte [esi],0E1h
	je	NEAR VOLUfx

	mov	cl,[esi]
	
	cmp	cl,0E0h				; E0 = loop to other part of FRQtable
	jne	testnewsound
	
	movzx	eax,byte [esi+1]
	and	al,3Fh
	mov	[ebp + chan.FrqOfs],eax
	mov	esi,[ebp + chan.FrqPtr]
	add	esi,eax
	
	mov	cl,[esi]

testnewsound:
	cmp	cl,0E2h				; E2 = set waveform
	jne	testE4

	movzx	eax,byte [esi+1]		; get smp num
	and	al,3Fh
	mov	[ebp + chan.SamplN],al
	
	
	push	esi

	shl	eax,4				; Instr.size = 16
	lea	ecx,[SNDinfo]
	add	eax,ecx
	mov	esi,eax

	lea	eax,[SMPinfo]
	add	eax,[esi + instr.Start]
	push	eax
	call	_paulaSetSamplePos@4		; SetPos


	mov	eax,[esi + instr.Len]
	push	eax
	call	_paulaSetSampleLen@4		; SetLen


	push	dword 1
	call	_paulaSetVoice@4		; PlayVoice


	lea	eax,[SMPinfo]
	add	eax,[esi + instr.Start]
	add	eax,[esi + instr.RepSta]
	push	eax
	call	_paulaSetSamplePos@4


	mov	eax,[esi + instr.RepLen]
	push	eax
	call	_paulaSetSampleLen@4

	pop	esi

	mov	[ebp + chan.VolOfs],edx
	mov	byte [ebp + chan.VolTcnt],1
	add	word [ebp + chan.FrqOfs],2
	jmp	transpose

testE4:
	cmp	cl,0E4h				; E4 = change waveform
	jne	testE9

	movzx	eax,byte [esi+1]		; get smp num
	and	al,3Fh
	mov	[ebp + chan.SamplN],al

	push	esi
	
	shl	eax,4				; Instr.size = 16
	lea	ecx,[SNDinfo]
	add	eax,ecx
	mov	esi,eax

	lea	eax,[SMPinfo]
	add	eax,[esi + instr.Start]
	push	eax
	call	_paulaSetSamplePos@4

	mov	eax,[esi + instr.Len]
	push	eax
	call	_paulaSetSampleLen@4

	pop	esi

	add	word [ebp + chan.FrqOfs],2
	jmp	transpose

testE9:
	cmp	cl,0E9h				; !!! TO DO !!!
	jne	testpatjmp
	
	push	dword 0
	call	_paulaSetVoice@4		; Stop Voice
	
	add	word [ebp + chan.FrqOfs],3
	
testpatjmp:
	cmp	cl,0E7h
	jne	testpitchbend
	
	movzx	eax,byte [esi+1]
	shl	eax,6
	lea	ebx, [FRQinfo]
	add	eax,ebx
	mov	[ebp + chan.FrqPtr],eax
	mov	[ebp + chan.FrqOfs],edx
	mov	esi,eax
	jmp	testeffects

testpitchbend:
	cmp	cl,0EAh
	jne	testnewsustain
	mov	al,[esi+1]
	mov	[ebp + chan.PBend],al
	mov	al,[esi+2]
	mov	[ebp + chan.PBendC],al
	add	word [ebp + chan.FrqOfs],3
	jmp	transpose
testnewsustain:
	cmp	cl,0E8h
	jne	testnewvib
	mov	al,[esi+1]
	mov	[ebp + chan.FrqSus],al
	add	word [ebp + chan.FrqOfs],2
	jmp	testsustain
testnewvib:
	cmp	cl,0E3h
	jne	transpose
	mov	al,[esi+1]
	mov	[ebp + chan.VibSpd],al
	mov	al,[esi+2]
	mov	[ebp + chan.VibAmp],al
	add	word [ebp + chan.FrqOfs],3
transpose:
	mov	esi,[ebp + chan.FrqPtr]
	add	esi,[ebp + chan.FrqOfs]
	mov	al,[esi]
	mov	[ebp + chan.FreqTR],al
	inc	word [ebp + chan.FrqOfs]

;*************************************************************************
; VOLUME EFFECTS
;*************************************************************************
VOLUfx:
	cmp	byte [ebp + chan.VolSus],0
	jz	volsustzero
	dec	byte [ebp + chan.VolSus]
	jmp	calcperiod
volsustzero:
	cmp	byte [ebp + chan.VBendC],0
	jnz	do_VOLbend
	dec	byte [ebp + chan.VolTcnt]
	jnz	NEAR calcperiod
	mov	al,[ebp + chan.VolTspd]
	mov	[ebp + chan.VolTcnt],al
volu_cmd:
	mov	esi,[ebp + chan.VolPtr]
	add	esi,[ebp + chan.VolOfs]
	mov	cl,[esi]
	
testvoluend:
	cmp	cl,0E1h				; VolSeq END
	je	calcperiod
	
	cmp	cl,0EAh				; VolSeq NEWBEND
	jne	testVOLsustain
	
	mov	al,[esi+1]
	mov	[ebp + chan.VBendS],al
	mov	al,[esi+2]
	mov	[ebp + chan.VBendC],al
	add	word [ebp + chan.VolOfs],3
	jmp	do_VOLbend

testVOLsustain:
	cmp	cl,0E8h				; VolSeq SUSTAIN
	jne	testVOLloop
	add	word [ebp + chan.VolOfs],2
	mov	[ebp + chan.VolSus],ch
	jmp	calcperiod
testVOLloop:
	cmp	cl,0E0h				; VolSeq LOOP
	jne	setvolume
	movzx	eax,byte [esi+1]
	and	al,3Fh
	sub	al,5
	mov	[ebp + chan.VolOfs],al
	jmp	volu_cmd
do_VOLbend:
	xor	byte [ebp + chan.VBendX],0FFh	; DO VOLBend
	jz	calcperiod
	dec	byte [ebp + chan.VBendC]
	mov	al,[ebp + chan.VBendS]
	add	[ebp + chan.Volume],al
	test	al,80h
	jz	calcperiod
	xor	al,al
	mov	[ebp + chan.VBendC],al
	mov	[ebp + chan.Volume],al
	jmp	calcperiod
setvolume:
	mov	al,[esi]
	mov	[ebp + chan.Volume],al
	inc	word [ebp + chan.VolOfs]

;*************************************************************************
; CALC PERIOD
;*************************************************************************
calcperiod:
	movzx	eax,byte [ebp + chan.FreqTR]
	test	al,80h			; is negative? (bit 7 set) locked note
	jnz	lockednote
	add	al,[ebp + chan.Note]	;  Note
	add	al,[ebp + chan.NoteTR]	; +NoteTR
lockednote:
	and	al,7Fh		; use only lower 7bit
	lea	esi,[PERIODS]
	add	eax,eax
	mov	ebx,eax		; BX = (TABOFS OF NOTE*2)
	add	esi,eax
	mov	ax,[esi]	; AX = PERIOD OF NOTE

;*************************************************************************
; VIBRATOR
;*************************************************************************
;	jmp	novibrato	;!!!!!!

	mov	cl,[ebp + chan.VibXOR]		; D0 = AX, D5 = BX, D4 = CH, 
	cmp	byte [ebp + chan.VibDel],0	; D7 = CL, D1 = DX
	jz	vibrator
	dec	byte [ebp + chan.VibDel]
	jmp	novibrato
vibrator:
	xor	edx,edx
	mov	ch,[ebp + chan.VibAmp]
	add	ch,ch
	mov	dl,[ebp + chan.VibAmpC]
	test	cl,80h
	jz	vib1
	test	cl,1
	jnz	vib4
vib1:	
	test	cl,20h
	jnz	vib2
	sub	dl,[ebp + chan.VibSpd]
	jnc	vib3
	or	cl,20h
	xor	edx,edx
	jmp	vib3
vib2:	
	add	dl,[ebp + chan.VibSpd]
	cmp	dl,ch
	jc	vib3
	and	cl,0DFh
	mov	dl,ch
vib3:
	mov	[ebp + chan.VibAmpC],dl
vib4:
	shr	ch,1
	sub	dl,ch
	jnc	vib5
	sub	dx,0100h
vib5:
	add	bl,0A0h
	jc	vib7
vib6:
	add	dx,dx
	add	bl,018h
	jnc	vib6
vib7:
	add	ax,dx
novibrato:
	xor	cl,1
	mov	[ebp + chan.VibXOR],cl

;*************************************************************************
; PORTAMENTO
;*************************************************************************
	
	xor	byte [ebp + chan.PortaX],0FFh		; do 1 portathing every 2 frames!!!
	jz	noporta
;------
	movzx	ebx,byte [ebp + chan.PortaV]
	or	ebx,ebx
	jz	noporta
	cmp	bl,1Fh
	jbe	portaup
portadown: 
	and	bl,1Fh
	neg	bx
portaup:
	sub	[ebp + chan.Period],bx
noporta:

;*************************************************************************
; PITCH BEND
;*************************************************************************
pitchbend:
	xor	byte [ebp + chan.PBendX],1
	jz	addporta
	cmp	byte [ebp + chan.PBendC],0
	jz	addporta
	dec	byte [ebp + chan.PBendC]
	movzx	ebx,byte [ebp + chan.PBend]
	test	bl,80h
	jz	pitchup
	movsx	bx,bl
pitchup:
	sub	word [ebp + chan.Period],bx
addporta:
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


