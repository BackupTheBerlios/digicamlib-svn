/****************************************************************************/
/*                    Canon PowerShot I/O library 1.0                       */
/****************************************************************************/
/* Created: 25.9.2004 [BORLAND C 3.1]                                       */
/* Last modified: 27.9.2004 [BORLAND C 3.1]	                            */
/* Copyright (C) 2004 by RayeR                                              */
/* Contact: http://www.volny.cz/rayer                                       */
/* Compiler: Borland C++ 3.1, disable Word Alignment, set 80186 code gen.   */
/****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>

#define PS_GFX_FUNC_NO 7	       // define graphics function number

typedef unsigned char Byte;            // size 8 bit, 0-255
typedef unsigned int  Word;            // size 16 bit, 0-65535
typedef unsigned long DWord;           // size 32 bit, 0-4294967295

typedef struct                         // text message structure
{
  Word x;                              // x-position [0-639]
  Word y;                              // y-position [0-479]
  DWord strptr;                        // RM pointer to 0-terminated string
  Word bkcolor;                        // background color
  Word txtcolor;                       // text color
  Word unknown;                        // ?, let it be 0
} PS_TEXT_MSG;

void log(char *str);
void dumplog(void far*pt,DWord size);

//**************** return 16bit segmnet part of 32bit RM pointer ************
Word hlp_ptr2seg(void *pointer)
{
  return((Word)((DWord)pointer>>16));
}

//**************** return 16bit offset part of 32bit RM pointer *************
Word hlp_ptr2ofs(void *pointer)
{
  return((Word)((DWord)pointer&0xFFFF));
}

//**************** get pointer to system function FUNCNO into SEG:OFS *******
void ps_get_func_ptr(Byte funcno, Word *seg, Word *ofs)
{
  union REGS regs;                     // generic registers
  struct SREGS segregs;                // segment registers
  regs.h.ah = funcno;                  // put required funcno to AH
  int86x(0xFF,&regs,&regs,&segregs);   // do int FFh with regs structures
  *seg=segregs.ds;                     // read segment from DS
  *ofs=regs.x.dx;                      // read offset from DX
}

//**************** load text message from structure to camera internal regs *
void ps_gfx_load_text_msg(PS_TEXT_MSG *tm)
{
  Word fs,fo,ts,to;                    // get pointer to gfx func
  ts=hlp_ptr2seg(tm);                  // get segment part of tm pointer
  to=hlp_ptr2ofs(tm);                  // get offset part of tm pointer
  ps_get_func_ptr(PS_GFX_FUNC_NO,&fs,&fo);
  asm {
    mov bx,fs                          // store func gfx ptr to ES:BX
    mov es,bx
    mov bx,fo
    push ts
    push to
    call DWORD PTR es:[bx+0x78]        // call func 7 / subfunc 78h
  }//*/
}

//**************** execute graphics function with previously prepared data **
void ps_gfx_exec_func(void)
{
  Word fs,fo;
  ps_get_func_ptr(PS_GFX_FUNC_NO,&fs,&fo);	   // get pointer to gfx func
  asm {
    mov bx,fs                          // store func gfx ptr to ES:BX
    mov es,bx
    mov bx,fo
    push 0 			       // ?, let it be 0, 80186 instruction
    call DWORD PTR es:[bx+0x20]        // call func 7 / subfunc 20h
  }
}
void ps_exec_func34(void)
{
  Word fs,fo;                          // get pointer to gfx func
  ps_get_func_ptr(PS_GFX_FUNC_NO,&fs,&fo);
  asm {
    mov bx,fs                          // store func gfx ptr to ES:BX
    mov es,bx
    mov bx,fo
    push 0
    push 0	       // ?, let it be 0, 80186 instruction
    call DWORD PTR es:[bx+0x34]        // call func 7 / subfunc 20h
  }

}

//**************** display single-line text message on LCD ("\n" not allowed!)
void ps_print(char *str, Word x, Word y, Word txtcolor, Word bkcolor)
{
  PS_TEXT_MSG tm;                      // text message structure
  FILE *f;
  char buffer[150];
  tm.x=x;                              // fill the structure by params
  tm.y=y;
  tm.txtcolor=txtcolor;
  tm.bkcolor=bkcolor;
  tm.unknown=0;
  tm.strptr=(DWord)str;                // convert pointer to DWord

//  sprintf(buffer,"before &tm: %lu str: \"%s\" back: %lu back2: %lu\n",&tm,str,&ps_print,ps_print);

  ps_gfx_load_text_msg(&tm);           // load structure to camera
//  ps_gfx_exec_func();                  // execute display command
}

void KeyboardDispatcher();

DWord i;
FILE *f;

char smsg[]="RayeR HacK!!!";
Word hlp_ptr2seg2(DWord pointer)
{
  pointer=pointer>>16;
  return (Word)pointer;
}

struct KeyData{		// offset
	Word event;	// 0
	Word res1;	// 2
	Byte res2;	// 4
	DWord keyMap;	// 5
};

#define KEY_RELEASE_HALF 	1
#define KEY_RELEASE_FULL 	2
#define KEY_SET			8
#define KEY_MENU		16
#define KEY_MACRO		32
#define KEY_DISPLAY		64
#define KEY_FLASH		128
#define KEY_WB			512
#define KEY_RIGHT		1024
#define KEY_LEFT		2048
#define KEY_ZOOM_DEC		4096
#define KEY_ZOOM_INC		8192
#define KEY_BATTERY		16384
#define KEY_VIDEO_OUT		32768
#define KEY_USB		        ((DWord)1<<16)
#define KEY_UP			((DWord)2<<16)
#define KEY_DOWN		((DWord)4<<16)
#define KEY_MF			((DWord)32<<16)
#define KEY_LENS		((DWord)1024<<16)
#define KEY_MODE		((DWord)2048<<16)
#define KEY_SPOTFOCUS		((DWord)4096<<16)

int far pascal back(struct KeyData far*keys,Word far*wparam)
{
	Word backs,backo;
	char buffer2[100];
	asm{
		les bx,[bp+0x2]
		mov backo,bx
		mov bx,es
		mov backs,bx
	}


	if (keys->keyMap&KEY_SPOTFOCUS)
	{
		ps_exec_func34();
		itoa(backs,buffer2,16);
		ps_print(buffer2,200,10,14,0);
		itoa(backo,buffer2,16);
		ps_print(buffer2,200,40,14,0);
		ps_gfx_exec_func();
		for(i=0;i<500000;i++);
	}

	if ((keys->keyMap&KEY_RELEASE_FULL)||(keys->keyMap&KEY_MF))
		*wparam=1;

	return 0;
}

Byte real(Byte b)
{
	Byte high,low;
	high=b>>4;
	low=b&0xF;
	return high*10+low;
}

void KeyboardDispatcher()
{
 Word var,fs1,fo1;
 var=0;
  ps_get_func_ptr(1,&fs1,&fo1);
 asm{
 push cs
 push offset back
 push ss
 lea ax,var
 push ax
 mov bx,fs1
 mov es,bx
 mov bx,fo1
 call dword ptr es:[bx+0x8]
 }
 if (var)
	exit(0);
}
char logfilename[]="D:\\log.txt";
FILE *OpenLog()
{
	return fopen(logfilename,"a+");
}
void log(char *str)
{
	FILE *f=OpenLog();
	fprintf(f,"%s\n",str);
	fclose(f);
}
void log(DWord d)
{
	FILE *f=OpenLog();
	fprintf(f,"DWord: %lu\n",d);
	fclose(f);
}
Word dumpnr=0;
#define DUMP_FILE 1
#define DUMP_LOG 2
void dumplog2(void far*pt,DWord size,int savetofile)
{
	DWord i;
	FILE *f=OpenLog();
	Byte far*buf=(Byte far*)pt;
	char strbuf[17];
	FILE *f2;
	if (savetofile&DUMP_FILE)
	{
		dumpnr++;
		sprintf(strbuf,"D:\\b%04X%u.txt",hlp_ptr2seg(pt),dumpnr);
		f2=fopen(strbuf,"wb");
	}

	fprintf(f,"Dumping data Ptr: 0x%08X Size: 0x%08X\n\n",(DWord)pt,(DWord)size);

	strbuf[0]=0;
	for(i=0;i<size+16-(size%16);i++)
	{
		if (i%16==0)
			fprintf(f,"%04X:%04X\t",hlp_ptr2seg((void *)(((DWord)pt)+i)),hlp_ptr2ofs((void *)(((DWord)pt)+i)));

		if (i<size)
		{
			fprintf(f,"%02X ",buf[i]);
			if (savetofile&DUMP_FILE)
				fprintf(f2,"%c",buf[i]);
		}
		else
			fprintf(f,"   ");



		if (i%4==3)
			fprintf(f,"  ");


		if (i<size&&buf[i]>32&&buf[i]<128)
			strbuf[i%16]=buf[i];
		else if (i<size)
			strbuf[i%16]='.';
		else
			strbuf[i%16]=' ';

		if (i%16==15)
		{
			strbuf[16]=0;
			fprintf(f,"\t%s\n",strbuf);
		}
	}
	fprintf(f,"\nDump finished.\n\n");
	fclose(f);
	if (savetofile&DUMP_FILE)
		fclose(f2);
}
void dumplog(void far*pt,DWord size)
{
	dumplog2(pt,size,DUMP_LOG);
}


  Word ka=0;
  char buffer[20];


void interrupt(* int0x90_handler)(...);
void far*h;

unsigned char jmpptr[]={0xea,0xd0,0x00,0xbf,0xd};

void far* far LoadFuncTable(Byte number)
{
	void far*rueck;
	asm{
		push ds
		push bp
		mov ah,number
		xor al,al
		int 0xff
		lea bp,rueck
		mov [bp],dx
		mov [bp+2],ds
		pop bp
		pop ds
	}
	return rueck;
}

typedef struct ADD_INFOS
{
	DWord address;
	Byte tablenumber;
	Byte offset;
	char *name;
	struct ADD_INFOS *next;
} ADD_INFO;

ADD_INFO *infomem;
Word infomemCount;

DWord infos[2048];//8192];

ADD_INFO *findInfo(void far*add,int reloc)
{
	// Try finding it first by mod hash
	Word off=FP_OFF(add)%2047;
	if (infos[off])
	{
		ADD_INFO *a=(ADD_INFO*)infos[off];
		while(a!=0)
		{
			if (a->address==(DWord)add)
				return a;

			a=a->next;
		}
	}

	// then by region
	off=FP_OFF(add)/32;
	if (infos[off])
	{
		ADD_INFO *a=(ADD_INFO*)infos[off];
		ADD_INFO *lasta=0;
		while(a)
		{
			if (a->address<=(DWord)add&&a->address+10>(DWord)add)
			{// we found a match, now place it into the hashed position

				if (!reloc)// don't fix position
					return a;

				// delete it
				if (lasta==0)
					infos[off]=0;
				else
					lasta->next=a->next;

				//create new
				a->address=(DWord)add;

				off=FP_OFF(add)%2047;
				if (!infos[off])
				{
					infos[off]=(DWord)a;
					a->next=0;
				}
				else
				{
					lasta=(ADD_INFO*)infos[off];
					infos[off]=(DWord)a;
					a->next=lasta;
				}

				return a;
			}
			lasta=a;
			a=a->next;
		}
	}
	return 0;
}

void addInfo(void far*add,Byte table,Byte offset)
{
	char buffer[100];
	Word off=FP_OFF(add)/32;//%2047; // save it first by region
	ADD_INFO *anew;
//	sprintf(buffer,"add: %Fp table:%u offset:%u off:%u",add,table,offset,off);
//	log(buffer);
	anew=&infomem[infomemCount];
	infomemCount++;
	anew->address=(DWord)add;
	anew->tablenumber=table;
	anew->offset=offset;
	anew->next=0;
	anew->name=0;

	if (infos[off]!=0)
	{
		ADD_INFO *a=(ADD_INFO*)infos[off];
		//sprintf(buffer,"infos[off]=%lu",(DWord)infos[off]);
		//log(buffer);

//		while (a!=0)
//			a=a->next;

//		a->next=anew;

		infos[off]=(DWord)anew;
		anew->next=a;
	}
	else
	{
		//sprintf(buffer,"infos[off] noch leer dann=%lu",(DWord)anew);
		//log(buffer);

		infos[off]=(DWord)anew;
	}
}

ADD_INFO *findInfoByTableOff(Byte table,Byte off)
{
	DWord *functable=(DWord*)LoadFuncTable(table);
	return findInfo((void far*)functable[off/4],0);
}

void addInfoName(Byte table,Byte off,char *name)
{
	ADD_INFO *ai=findInfoByTableOff(table,off);
	if(ai)
		ai->name=name;
}

void initInfo()
{
	Word i;
	Word segp;
	infomemCount=0;
	i=allocmem(100,&segp);//new ADD_INFO[500];
	log((DWord)i);
	infomem=(ADD_INFOS*)MK_FP(segp,0);
	for (i=0;i<2048;i++)
		infos[i]=0;
}

//DWord addressbuffer[100];
//Byte addCount;

void saveAddresses()
{
/*	Byte i;
	for (i=0;i<addCount;i++)
	{

	}
	addCount=0;*/
//	log("Adressen:");
//	dumplog(addressbuffer,400);
//	addCount=0;
}

void interrupt my90handler(...)
{
	asm{
	pushf
	}
	{
		Word fs,fo,fs2,fo2;
		Word j,i;
		asm{
		mov ax,[bp+0x14]
		mov fs,ax
		mov ax,[bp+0x12]
		mov fo,ax
		}
		ADD_INFO *ai=findInfo(MK_FP(fs,fo),1);
		if(ai)
		{
			char buffer[30];
			Word curBP=_BP;
			Word curCGStack;
			if (!ai->name)
			{
				sprintf(buffer,"func%X_%X",(Word)ai->tablenumber,(Word)ai->offset);
				log(buffer);
			}
			else
			{
				//sprintf(buffer,"%X:%X",fs,fo);
				//log(ai->name);
				//log(buffer);
			}

			asm{
				mov bx,curBP
				mov ax,ss:[bx]
				mov curBP,ax
			}
			asm{
				mov bx,0x440
				mov es,bx
				mov bx,2
				mov ax,es:[bx]
				mov curCGStack,ax
			}
			for (int i=0;i<6;i++)
			{
			       Word newBP;
			asm{
				mov bx,curBP
				mov ax,ss:[bx+2]
				mov fo2,ax
				mov ax,ss:[bx+4]
				mov fs2,ax
			}
			if (fs2==0x440&&fo2==0x269)
			{
				asm{
				mov bx,0x440
				mov es,bx
				mov bx,curCGStack
				mov ax,es:[bx+2]
				mov fs2,ax
				mov ax,es:[bx+4]
				mov fo2,ax
				mov ax,es:[bx+8]
				mov j,ax
				add bx,10
				mov curCGStack,bx
				}
				sprintf(buffer,"bank: %X ptr: %X:%X",j,fs2,fo2);
				log(buffer);
			}
			else
			{
				sprintf(buffer,"ptr: %X:%X",fs2,fo2);
				log(buffer);
			}
				asm{
					mov bx,curBP
					mov ax,ss:[bx]
					mov newBP,ax
				}
				if (newBP>curBP)
					curBP=newBP;
				else
					break;
			}
		}
/*		addressbuffer[addCount]=(DWord)MK_FP(fs,fo);
		addCount++;

		if (addCount>=100)
			saveAddresses();*/

		if (fs==0xdc80)
		{

		if (fo==0x1ed)
		{
//		       log("Text");
		       log((char far*)((PS_TEXT_MSG far*)*((DWord far*)(((Byte far*)&fs)+0x54)))->strptr);
/*		       asm{
				mov ax,0x440
				mov es,ax
				mov bx,2
				mov ax,es:[bx]
		       }
/*		       dumplog(MK_FP(0x440,0),512);*/
/*		       log("Ruecksprungadresse");
		       log(*((DWord*)(((Byte far*)&fs)+0x4a)));*/
		       }
		}

	}
	asm{
	popf
	leave
	pop di
	pop si
	pop ds
	pop es
	pop dx
	pop cx
	pop bx
	pop ax
	db 0xea
	dd 0xdbf00d0
	}
}

void logfunctables()
{
	Word z;
	for(z=1;z<0x10;z++)
	{
		Word fs,fo;
		char buffer[100];
		ps_get_func_ptr(z,&fs,&fo);
		sprintf(buffer,"Function Table %x Addr: %x:%x",z,fs,fo);
		log(buffer);
		dumplog(MK_FP(fs,fo),300);
	}
}

void pascal far dumpstacktest()
{
	char buffer[20];
	log("Stack");
	dumplog(MK_FP(_SS,_SP),100);
	log("BP");
	log((DWord)_BP);
	log("dumpstacktest address");
	sprintf(buffer,"%Fp",dumpstacktest);
	log(buffer);
}

void bankdump()
{
	Word f1,f2;
	FILE *f;
	char buffer[100];
	asm{
		mov dx,0xfed4
		in ax,dx
		mov f1,ax
		add dx,2
		in ax,dx
		mov f2,ax
	}
	sprintf(buffer,"Dumping current bank: %X %X",f1,f2);
	log(buffer);
	sprintf(buffer,"d:\\bank%X",f1);
	f=fopen(buffer,"wb");
	fwrite(MK_FP(0xe000,0),0x8000,2,f);
	fclose(f);
}


void dumpbank(Word b)
{
	Word f1,f2;
	asm{       // saving old bankaddrs
		mov dx,0xfed4
		in ax,dx
		mov f1,ax
		add dx,2
		in ax,dx
		mov f2,ax
		// changing bank
		mov dx,0xfed4
		mov ax,b
		out dx,ax
		add dx,2
		add ax,4
		out dx,ax
	}
	bankdump();
	asm{
		// restoring old values
		mov dx,0xfed4
		mov ax,f1
		out dx,ax
		add dx,2
		mov ax,f2
		out dx,ax
	}

}

void dumptestmain()
{
	dumpbank(0x19c);
	asm{
		mov al,0
		mov ah,0x4c
		int 0x21
	}
}

void main(void)
{
	Word fso,foo;
	Byte far* ptr;
	DWord i;
	char buffer[100];

//	log("------------------------------------------------------");
	log("\n\n");
	log("               Starting new session");
	log("------------------------------------------------------");


//	dumpbank(0x18c);
//	dumpbank(0xe8);
//	dumpbank(0x184);

//	dumpstacktest();

//	log("writing fast address table");
	//addCount=0;
	initInfo();
	for(i=1;i<15;i++)
	{
		DWord j;
		DWord *functable=(DWord*)LoadFuncTable(i);
		//DWord grenze[]={0,52,19,0,8,25,3,37,31,4,20,26,0,0,0,0};//,46,4,10};
		DWord grenze[]={0,0,0,0,0,0,0,0,31,0,0,0,0,0,0,0};

		//sprintf(buffer,"funcs:%u table:%Fp",(Word)i,functable);
		//log(buffer);

		for (j=0;j<grenze[i];j++)
		{
			//Word off=FP_OFF(functable[j]);
			addInfo((void far*)functable[j],i,j*4);
		}
		sprintf(buffer,"funcs%u",(Word)i);// : %Fp",(Word)i,(Word)(j*4),functable[j]);
			//log(buffer);
		ps_exec_func34();
		ps_print(buffer,100,10,12,0);
		ps_gfx_exec_func();
		KeyboardDispatcher();
	}

/*	addInfoName(7,0x78,"PrintStr");
	addInfoName(7,0x34,"ClearScreen");
	addInfoName(7,0x20,"Draw");
	addInfoName(1,0x08,"KeyHandler");
	addInfoName(1,0x14,"HasCriticalConditionArrised");
	addInfoName(1,0xbc,"GetTicks");
	addInfoName(7,0x40,"FillRectangle");
	addInfoName(7,0x18,"SetPalette");
	addInfoName(0xa,0x38,"DrawFrame");
	addInfoName(8,0x30,"AllocDlg");
	addInfoName(8,0x4,"SetButtonState");
	addInfoName(8,0x10,"SetButtonText");
	addInfoName(8,0x14,"SetButtonTextPosition");
	addInfoName(8,0x8,"SetButtonPosition");
	addInfoName(8,0x38,"DrawDlg");
	addInfoName(8,0,"DrawButton");
	addInfoName(8,0x34,"DeleteDlg");*/
//	addInfoName(7,0x4c,"Komische 4C Funktion");

/*	ps_exec_func34();
	ps_print("Reloading Pointer...",100,10,14,0);
	ps_gfx_exec_func();*/
	setvect(0x90,my90handler);
//	log("Reloaded Pointer");

	ps_print("Done.",200,190,14,0);
	ps_gfx_exec_func();

	log("going into deep standby....");
	keep(0, (_SS + (_SP/16) - _psp));
}

