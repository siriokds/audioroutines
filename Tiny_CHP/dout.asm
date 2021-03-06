; DOUT.ASM x NASM (SLEEP MODE, 672 bytes)


DOUT_PRIMARY_FLAGS	equ 0x00000001	;(DSBCAPS_PRIMARYBUFFER)
DOUT_SECONDARY_FLAGS	equ 0x00018000	;(DSBCAPS_GETCURRENTPOSITION2 OR DSBCAPS_GLOBALFOCUS)

DOUT_FREQUENCY		equ 44100
DOUT_SAMPLESIZ		equ 4
DOUT_LATENCY		equ 200					; Latency = 200ms
DOUT_CALLFRQ		equ 40					; (DOUT_LATENCY/5)
DOUT_BUFFERLEN		equ 8820				; ((DOUT_LATENCY*DOUT_FREQUENCY)/1000)
DOUT_BUFFERSIZ		equ DOUT_BUFFERLEN * DOUT_SAMPLESIZ
DOUT_BUFFERNUM		equ 2
DOUT_BUFFERTOT		equ DOUT_BUFFERSIZ * DOUT_BUFFERNUM
DOUT_WFXSIZE		equ 18


;============
SECTION .bss
;============


struc dsdata
  
	.lpDSOUND	resd 1		; ptr to the IDirectSound instance
	.lpdsPrimary	resd 1		; ptr to the primary   IDirectSoundBuffer
	.lpdsBuffer	resd 1		; ptr to the secondary IDirectSoundBuffer
	.ThreadID	resd 1		; dummy dword for thread ID
	.ThreadHandle	resd 1		; handle of sound thread
	.ThreadXTReq	resd 1		; exit request flag for sound thread
	.rPos		resd 1		; Read/Play Position
	.wPos		resd 1		; Write Position
	.nBuffer	resd 1		; Actual Buffer Number
	.Callback	resd 1		; External Callback
	.buf1      	resd 1          ; 1st locked buffer address for streaming
	.len1      	resd 1          ; length of 1st buffer
	.buf2      	resd 1          ; 2st locked buffer address for streaming
	.len2      	resd 1          ; length of 2st buffer
	.bOpened	resd 1
	.Filler		resd 1
	.tempwfx   	resb DOUT_WFXSIZE

	.size
endstruc



;- - - - - - - - - - - - - - - - - - - - - - - 

extern _DirectSoundCreate@12
extern _CreateThread@24
extern _SetThreadPriority@8
extern _ResumeThread@4
extern _WaitForSingleObject@8
extern _Sleep@4



global _DOut_Open@8
global _DOut_Start@0
global _DOut_BufferCopy@4
global _DOut_Close@0

globdat: resb dsdata.size

;============
SECTION .code
;============


;---------------------------------------------
CallbackRoutine:
;---------------------------------------------
	pushad
	mov	ebx, globdat	
	mov	eax, [ebx + dsdata.Callback]
	or	eax,eax
	jz	.noCallback
	call	eax
.noCallback:
	popad
	ret
	
	
;---------------------------------------------
ThreadProcDS:
;---------------------------------------------
	pushad
 	mov	ebx, globdat
luup: 	
.looping:
	; check for exit request
	cmp     byte [ebx + dsdata.ThreadXTReq], 0
	jz      .loopok
	popad
	xor     eax,eax
	ret     4h
.loopok:

	xor	eax,eax
	push	eax
	lea	eax, [ebx + dsdata.rPos]
	push	eax
	mov	ebp, [ebx + dsdata.lpdsBuffer]
	push	ebp
	mov	edx, [ebp]				; vtbl
	call	[edx + 10h]				; IDirectSoundBuffer::GetCurrentPosition


	mov	eax, [ebx + dsdata.rPos]
	mov	ecx,DOUT_BUFFERSIZ
	xor	edx,edx
	div	ecx
	and	eax,1					; ActBuffer = (rPos / DOUT_BUFFERSIZ) & 1;
	
	mov	edx,[ebx + dsdata.nBuffer]
	cmp	eax,edx
	jne	noCall
	inc	eax
	and	eax,1
	mov	[ebx + dsdata.nBuffer],eax		
	call	CallbackRoutine
noCall:	

	xor	eax,eax
	mov	al, DOUT_CALLFRQ
	push	eax
	call	_Sleep@4

	jmp	luup


		

		



;---------------------------------------------
DSClose:
;---------------------------------------------
	pushad
	mov	ebx, globdat
	xor	ecx, ecx

	mov	eax, [ebx + dsdata.lpDSOUND]
	or	eax,eax
	jnz	.noDSOUNDRelease

	mov	esi, [eax]  				; vtbl
	push	eax					; lpdSound
	call	[esi + 08h]   				; IDirectSound::Release
	
	mov	[ebx + dsdata.lpDSOUND], ecx

.noDSOUNDRelease:

	popad
	ret











;int DOut_Open(void (*UserF)(void), DWORD hWnd);

;---------------------------------------------
_DOut_Open@8:
;---------------------------------------------
	push	ebp
	mov	ebp,esp
	pushad
	mov	ebx, globdat


	; clear global data
	mov	ecx, dsdata.size
	mov	edi, ebx
	xor	eax, eax
	cld
	rep	stosb


	mov	ecx, [ebp + 8]			; first parm (callback)
	mov	[ebx + dsdata.Callback],ecx


	; DirectSound Object create and set
	
	push    eax				; NULL
	lea     ecx, [ebx + dsdata.lpDSOUND]
	push	ecx				; LPDIRECTSOUND*
	push    eax				; NULL for Default Device
	call    _DirectSoundCreate@12

	or      eax, eax
	jnz     .initfailgate
	

	; set DSound cooperative level
	
	mov     al, 3       ; exclusive mode
	push    eax
	push    dword [ebp + 12]		; second parm (hwnd)
	mov     esi, [ebx + dsdata.lpDSOUND]
	push	esi
	mov	edi, [esi]  			; edi = vtbl
	call	[edi + 18h] 			; IDirectSound::SetCooperativeLevel

	or	eax, eax
	jnz	.initfailgate
		

	; obtain primary buffer
	
	xor	eax,eax
	push    eax
	lea     ebp, [ebx + dsdata.lpdsPrimary]
	push    ebp
	push    dword primdesc
	push    esi
	call    [edi + 0ch] 			; IDirectSound::CreateSoundBuffer

	or       eax, eax
.initfailgate:
	jnz      .gate2

	; obtain secondary buffer
	
	push     eax
	lea      edx, [ebx + dsdata.lpdsBuffer]
	push     edx
	push     dword buffdesc
	push     esi
	call     [edi + 0ch] 			; IDirectSound::CreateSoundBuffer

	or       eax, eax
.gate2
	jnz      near .InitFailed




	; set primary buffer format

	lea     edi, [ebx + dsdata.tempwfx]
	push    edi

	lea     esi, [wfxprimary]
	mov	ecx, DOUT_WFXSIZE
	rep     movsb

	mov     esi, [ebp]
	push    esi
	mov     edi, [esi]  			; edx = vtbl
	call    [edi + 38h] 			; IDirectSoundBuffer::SetFormat






	; Create MainThread

	lea     edx, [ebx + dsdata.ThreadID]
   	push    edx
	xor	eax,eax
	mov	al,4				; CREATE_SUSPENDED
    	push	eax
	xor	eax,eax
    	push	eax
	push    dword ThreadProcDS
	push    eax
	push    eax
	call     _CreateThread@24
	mov     [ebx + dsdata.ThreadHandle], eax
	push	dword 15
	push	eax
	call    _SetThreadPriority@8



	; ok, everything's done
	
	mov	dword [ebx + dsdata.bOpened],1	
	jmp     short .initends

.InitFailed:
	call     DSClose
	
.initends:
	popad
	pop	ebp
	xor	eax,eax
	ret     8h




	




;---------------------------------------------
_DOut_Start@0:
;---------------------------------------------
	pushad
	mov	ebx,globdat
	xor	eax,eax
	
	cmp	[ebx + dsdata.bOpened],eax
	jnz	.doStart

	inc	eax
	popad
	ret
	
.doStart:


; * Fill Pre-Buffers
	xor	eax,eax
	mov	[ebx + dsdata.nBuffer],eax
	call	CallbackRoutine
	inc	eax
	mov	[ebx + dsdata.nBuffer],eax
	call	CallbackRoutine
	

; * Resume Thread  +  Rewind to 0  +  Play

	xor	eax,eax
	push	eax					; position = 0
	mov	ebp, [ebx + dsdata.lpdsBuffer]
	mov	esi, [ebp]  				; vtbl
	push	ebp					; lpdsBuffer
	call	[esi + 34h]   				; IDirectSoundBuffer::SetCurrentPosition


	mov	ebp, [ebx + dsdata.ThreadHandle]
	push	ebp
	call	_ResumeThread@4


	xor	eax,eax
	inc	al
	push	eax					; DSBPLAY_LOOPING = 1
	dec	al
	push	eax					; RESERVED 
	push	eax					; PRIORITY
	mov	ebp, [ebx + dsdata.lpdsBuffer]
	mov	esi, [ebp]  				; vtbl
	push	ebp					; lpdsBuffer
	call	[esi + 30h]   				; IDirectSoundBuffer::Play
	
	xor	eax,eax
	popad
	ret







;---------------------------------------------
_DOut_Close@0:
;---------------------------------------------
	pushad
	mov	ebx,globdat
	xor	eax,eax
	
	cmp	[ebx + dsdata.bOpened],eax
	jnz	.doStart

	inc	eax
	popad
	ret
.doStart:

	mov	ebp, [ebx + dsdata.lpdsBuffer]
	mov	esi, [ebp]  				; vtbl
	push	ebp					; lpdsBuffer
	call	[esi + 48h]   				; IDirectSoundBuffer::Stop


	mov	byte [ebx + dsdata.ThreadXTReq],1	; signal thread to quit!

	; give the thread a chance to finish

	push    dword 2000
	mov	eax, [ebx + dsdata.ThreadHandle]
	push    eax
	call    _WaitForSingleObject@8

	call	DSClose

	mov	dword [ebx + dsdata.bOpened],0
	xor	eax,eax
	popad
	ret





;---------------------------------------------
_DOut_BufferCopy@4
;---------------------------------------------
	push	ebp
	mov	ebp,esp
	pushad
	mov	ebx,globdat
	
	mov	eax, [ebx + dsdata.bOpened]
	or	eax,eax
	jnz	.TryLock
	
	popad
	pop	ebp
	ret	4h



.TryLock:

	xor	eax,eax
	push	eax

	lea	eax, [ebx + dsdata.len2]
	push	eax
	
	lea	eax, [ebx + dsdata.buf2]
	push	eax
	
	lea	eax, [ebx + dsdata.len1]
	push	eax
	
	lea	eax, [ebx + dsdata.buf1]
	push	eax
	
	mov	eax, DOUT_BUFFERSIZ
	push	eax
	
	mov	ecx, [ebx + dsdata.nBuffer]
	mul	ecx
	push	eax

	
	mov	edi, [ebx + dsdata.lpdsBuffer]
	mov	esi, [edi]    			; vtbl
	push	edi
	call	[esi + 2ch]   			; IDirectSoundBuffer::Lock

	
	cmp     eax, 88780096h  		; DSERR_BUFFERLOST
	jne	.locked
	
	mov	edi, [ebx + dsdata.lpdsBuffer]
	mov	esi, [edi]    			; vtbl
	push	edi
   	call    [esi + 50h]   			; IDirectSoundBuffer::Restore
    	jmp     .TryLock

.locked:


	or	eax,eax
	jnz	.Exit
	
	mov	esi, [ebp + 8]			; Source
	mov	edi, [ebx + dsdata.buf1]	; Destination
	mov	ecx,DOUT_BUFFERSIZ

	

	push	ecx
	cld
	shr	ecx,2
	rep	movsd
	pop	ecx
	and	ecx,3
	rep	movsb


	push	dword [ebx + dsdata.len2]
	push	dword [ebx + dsdata.buf2]
	push	dword [ebx + dsdata.len1]
	push	dword [ebx + dsdata.buf1]

	mov	edi, [ebx + dsdata.lpdsBuffer]
	mov	esi, [edi]    			; vtbl
	push	edi
    	call    [esi + 4ch]   			; IDirectSoundBuffer::Unlock

.Exit:
	popad
	pop	ebp
	ret	4h





align 4

DSound_IID:
    DD 0b0210783h
    DW 089cdh
    DW 011d0h
    DB 0afh, 08h, 00h, 0a0h, 0c9h, 025h, 0cdh, 016h


wfxprimary:
    DW	1				; wFormatTag	  = 1 (WAVE_FORMAT_PCM)
    DW  2				; nChannels	  = 2 (STEREO)
    DD	DOUT_FREQUENCY			; nSamplesPerSec  = 44100Hz
    DD	DOUT_FREQUENCY*DOUT_SAMPLESIZ	; nAvgBytesPerSec = 176400 = (44100 * 2 (16bit) * 2 (stereo))
    DW  4				; nBlockAlign	  = 4
    DW  16				; wBitsPerSample  = 16 (16bit)
    DW	0				; cbSize	  = 0
    DW  0				; (Dword Align)


buffdesc:
    DD	20			; dwSize = 20 (struct size)
    DD	DOUT_SECONDARY_FLAGS	; dwFlags
    DD  DOUT_BUFFERTOT		; dwBufferBytes
    DD  0			; dwReserved
    DD  wfxprimary		; lpwfxFormat = wfxprimary

primdesc:
    DD	20			; dwSize = 20 (struct size)
    DD  DOUT_PRIMARY_FLAGS	; dwFlags = 1 (DSBCAPS_PRIMARYBUFFER)
    DD	0			; dwBufferBytes
    DD  0			; dwReserved
    DD	0			; lpwfxFormat


