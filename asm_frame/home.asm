.model tiny

;=====;=====;=====;=====;=====;=====;=====;=====;
;=====;	DEFINE	;=====;
FRAME_WIDTH	= 50
FRAME_HEIGHT	= 7
FRAME_LEFT	= 15
FRAME_UP	= 5

WINDOW_WIDTH	= 80
WINDOW_HEIGHT	= 25

BLACK_BLACK	= 00h
BLACK_BLUE	= 10h
WHITE_BLUE	= 17h

DELTA_LU	= 0
DELTA_LD	= 1
DELTA_RU	= 2
DELTA_RD	= 3
DELTA_HOR	= 4
DELTA_VER	= 5
;=====;=====;=====;=====;=====;=====;=====;=====;

;=====;=====;=====;=====;=====;=====;=====;=====;
;=====;	MAIN	;=====;
.code
org 100h
Start:
;=====; CHOICE	;=====;
	mov	si,	cs:[82h]
	and	si,	0ffh
	sub	si,	'0'
	mov	ax,	si
	mov	bx,	6
	mul	bx
	mov	si,	ax
;=====; VIDEO	;=====;
	mov	bx,	0b800h
	mov	ds,	bx
;=====; CLEAR	;=====;
	mov	ax,	BLACK_BLACK * 256 + ' '
	xor	bx,	bx
	mov	cx,	WINDOW_HEIGHT * WINDOW_WIDTH
	mov	dx,	2
	call	draw_word
;=====;	BLUE	;=====;
	mov	cx,	FRAME_HEIGHT
	mov	ah,	WHITE_BLUE
	mov	bx,	WINDOW_WIDTH * 2 * FRAME_UP + FRAME_LEFT * 2
loop_blue:
	push	cx
	mov	cx,	FRAME_WIDTH
	call	draw_word
	pop	cx
	add	bx,	WINDOW_WIDTH * 2 - FRAME_WIDTH * 2
	loop	loop_blue
;=====;	HOR_UP	;=====;
	mov	bx,	offset STYLE_1 + DELTA_HOR
	add	bx,	si
	mov	al,	cs:[bx]
	mov	cx,	FRAME_WIDTH
	mov	bx,	FRAME_UP * 2 * WINDOW_WIDTH + FRAME_LEFT * 2
	call	draw_word
;=====;	HOR_DOWN;=====;
	mov	cx,	FRAME_WIDTH
	mov	bx,	(FRAME_UP + FRAME_HEIGHT) * 2 * WINDOW_WIDTH + FRAME_LEFT * 2
	call	draw_word
;=====;	VER_L	;=====;
	mov	dx,	WINDOW_WIDTH * 2
	mov	bx,	offset STYLE_1 + DELTA_VER
	add	bx,	si
	mov	al,	cs:[bx]
	mov	cx,	FRAME_HEIGHT
	mov	bx,	FRAME_UP * 2 * WINDOW_WIDTH + FRAME_LEFT * 2
	call	draw_word
;=====;	VER_R	;=====;
	mov	cx,	FRAME_HEIGHT
	mov	bx,	FRAME_UP * 2 * WINDOW_WIDTH + FRAME_LEFT * 2 + FRAME_WIDTH * 2 - 2
	call	draw_word
;=====;	LU	;=====;
	mov	bx,	offset STYLE_1 + DELTA_LU
	add	bx,	si
	mov	al,	cs:[bx]
	mov	bx,	FRAME_UP * 2 * WINDOW_WIDTH + FRAME_LEFT * 2
	mov	[bx],	al
;=====;	LD	;=====;
	mov	bx,	offset STYLE_1 + DELTA_LD
	add	bx,	si
	mov	al,	cs:[bx]
	mov	bx,	(FRAME_UP + FRAME_HEIGHT) * 2 * WINDOW_WIDTH + FRAME_LEFT * 2
	mov	[bx],	al
;=====;	RU	;=====;
	mov	bx,	offset STYLE_1 + DELTA_RU
	add	bx,	si
	mov	al,	cs:[bx]
	mov	bx,	FRAME_UP * 2 * WINDOW_WIDTH + FRAME_LEFT * 2 + FRAME_WIDTH * 2 - 2
	mov	[bx],	al
;=====;	RD	;=====;
	mov	bx,	offset STYLE_1 + DELTA_RD
	add	bx,	si
	mov	al,	cs:[bx]
	mov	bx,	(FRAME_UP + FRAME_HEIGHT) * 2 * WINDOW_WIDTH + FRAME_LEFT * 2 + FRAME_WIDTH * 2 - 2
	mov	[bx],	al
;=====;	EXIT	;=====;
	xor	ah,	ah
	int	16h
	mov	ax,	4c00h
	int	21h
;=====;=====;=====;=====;=====;=====;=====;=====;



;=====;=====;=====;=====;=====;=====;=====;=====;
;=====;	DRAW	;=====;
; ax - value to write
; bx - start ptr
; cx - count
; dx - add to start ptr
draw_word	proc
	mov	[bx],	ax
	add	bx,	dx
	loop	draw_word
	ret
	endp
;=====;=====;=====;=====;=====;=====;=====;=====;


;=====;=====;=====;=====;=====;=====;=====;=====;
;=====;	SYMBOLS	;=====;
.data
STYLE_1:		; one line
	db	0dah	; LU
	db	0c0h	; LD
	db	0bfh	; RU
	db	0d9h	; RD
	db	0c4h	; HOR
	db	0b3h	; VER
STYLE_2:		; two line
	db	0c9h	; LU
	db	0c8h	; LD
	db	0bbh	; RU
	db	0bch	; RD
	db	0cdh	; HOR
	db	0bah	; VER

;=====;=====;=====;=====;=====;=====;=====;=====;
;=====;			;=====;
end Start
