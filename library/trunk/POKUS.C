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
//#include <bcd.h>

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

int (pascal far*ps_ltm)(PS_TEXT_MSG *tm)=0;


//**************** load text message from structure to camera internal regs *
void ps_gfx_load_text_msg(PS_TEXT_MSG *tm)
{
 if (ps_ltm==0)
 {
//	void far *addr=&ps_ltm;
	Word fs,fo;
	asm{
	push ds
	mov ah,7
	int 0xff
//	les bx,addr
//	mov fo,dx
//	mov dx,ds
//	mov fs,dx
	mov bx,ds
	mov es,bx
	mov bx,dx
	mov ax,es:[bx+0x78]
	mov cx,es:[bx+0x7a]
	mov fo,ax
	mov fs,cx
	pop ds
	}
	ps_ltm=(int (pascal far*)(PS_TEXT_MSG*))((DWord)fs*256*256+fo);
 }
// ps_ltm(tm);/*
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
  //f=fopen("D:\\prtlog.txt","a+");

//  sprintf(buffer,"before &tm: %lu str: \"%s\" back: %lu back2: %lu\n",&tm,str,&ps_print,ps_print);
//  log(buffer);
  //fclose(f);

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
  //return((Word)((DWord)pointer>>16));
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
/*	char buffer[10];
	ultoa(keys->keyMap,buffer,10);
	ps_exec_func34();
	ps_print(buffer,10,10,14,0);
	ps_gfx_exec_func();*/
	Word backs,backo;
	char buffer2[100];
	asm{
		les bx,[bp+0x2]
		mov backo,bx
		mov bx,es
		mov backs,bx
	}

	//dumplog(MK_FP(_SS,_BP),100);


	if (keys->keyMap&KEY_SPOTFOCUS)
	{
		//sprintf(buffer2,"Wurst %X:%X",backs,backo);
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

/*int far pascal oldback(DWord d1,Word far*d2)
{
	FILE *f;
		DWord j;
	char buffer[10];
	Word wert;

	asm{
	les bx,d1
	mov ax,es:[bx]
	mov wert,ax
	}

	ps_exec_func34();

	if (wert==5)
	{
		ps_print("komisches ding 5",200,100,14,0);
		for (j=0; j<500000; j++); // delay ~2
	}

	itoa(wert,buffer,10);
	ps_print(buffer,200,10,14,0);
	asm{
	les bx,d1
	mov ax,es:[bx+5]
	mov wert,ax
	}

	itoa(wert,buffer,10);
	ps_print(buffer,200,40,14,0);

	if (wert&2)
	{
		*d2=1;
	}

	asm{
	les bx,d1
	mov ax,es:[bx+7]
	mov wert,ax
	}

	itoa(wert,buffer,10);
	ps_print(buffer,200,70,14,0);

	ps_gfx_exec_func();
} */

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

//void far(*oldfunc)();

int pascal far my_ltm(PS_TEXT_MSG *tm)
{
	if (((char far*)tm->strptr)[0]!=0)
		((char far*)tm->strptr)[0]='j';

//	return ps_ltm(tm);
	return 0;
}

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
	i=allocmem(200,&segp);//new ADD_INFO[500];
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
		Word j;
		char far*pt;
		FILE *f;
		asm{
		mov ax,[bp+0x14]
		mov fs2,ax
		mov ax,[bp+0x12]
		mov fo2,ax
		}
		ADD_INFO *ai=findInfo(MK_FP(fs2,fo2),1);
		if(ai)
		{
			char buffer[100];
			asm{
				mov bx,0x440
				mov es,bx
				mov bx,2
				mov ax,es:[bx]
				mov bx,ax}
				marke:asm {
				mov ax,es:[bx+2]
				mov fs,ax
				mov ax,es:[bx+4]
				mov fo,ax
				cmp ax,0x269
				jz marke
			}


			if (!ai->name)
			{
				sprintf(buffer,"func%X_%X : %X:%X",(Word)ai->tablenumber,(Word)ai->offset,fs,fo);
				log(buffer);
			}
			else
			{
				sprintf(buffer,"%X:%X",fs,fo);
				log(ai->name);
				log(buffer);
			}

				//mov cx,ax
				//mov ax,es:[bx+24]

			/*	add ax,22
				mov bx,ax
				mov ax,es:[bx]
				mov fs,ax
				mov ax,es:[bx+2]
				mov fo,ax*/

			//log((DWord)_AX);
			//log((DWord)MK_FP(_CX,_AX));
			//sprintf(buffer,"%Fp",MK_FP(fs,fo));
			//log(buffer);
		}
/*		addressbuffer[addCount]=(DWord)MK_FP(fs,fo);
		addCount++;

		if (addCount>=100)
			saveAddresses();*/
/*		f=fopen("ret.txt","w");
		//fwrite(pt,100,1,f);
		fprintf(f,"%X:%X",fs,fo);
		fclose(f);               */

/*		if (fs==0xdad9||fs==0xdc80||fs==0xda0c||fs==0x9cb7||fs==0x976f||fs==0xd981||fs==0xd9c6||fs==0xd8e9||fs==0xb4ba)
		//if (fs==0xccea||fs==0xcb47||fs==0xce0c)
		{
		for (j=1;j<0xb;j++)
		{
			if (j==3||j==7)
				continue;
			DWord i;
			DWord *functable=(DWord*)LoadFuncTable(j);
			DWord grenze[]={0,52,19,0,8,25,3,37,31,4,20,26,46,4};

			char buffer[100];
			for (i=0;i<grenze[j];i++)
			{
				Word off=FP_OFF(functable[i]);
				if(fo>off&&fo<off+12&&FP_SEG(functable[i])==fs)
				{
					sprintf(buffer,"func%X_%lX",j,i*4,functable[i]);
					log(buffer);
					break;
				}
			}
			if (i<grenze[j])
				break;
		}
		}*/
/*			{
				log("Function called in seg: dad9h");
				log((DWord)fo);
			}    */


		if (fs2==0xdc80)
		{

		if (fo2==0x1ed)
		{
		       //	dumplog(MK_FP(_SS,_BP),200);
		       //	log((DWord)_BP);

/*		       DWord test;
		       asm{
		       mov bx,offset fs
		       mov fs2,bx
		       mov bx,seg fs
		       mov fo2,bx*/

/*		       mov ax,ss:[bx+4]
		       mov fs2,ax
		       mov ax,ss:[bx+2]
		       mov fo2,ax
		       }               */
//		       log("int 90h called from dc80:01ed");
/*		       dumplog(&fs,500);
//		       dumplog(((Byte far*)&fs)+0x54,300);

//		       dumplog((void far*)*((DWord*)(((Byte far*)&fs)+0x54)),100);
//		       log("&fs");
//		       log((DWord)&fs);//*/
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
		       log(*((DWord*)(((Byte far*)&fs)+0x4a)));

		       asm{
		       mov ax,ds
		       mov fo2,ax
		       }

		       dumplog2((void far*)(((Byte far*)(*((DWord*)(((Byte far*)&fs)+0x4a))))-100),200,DUMP_FILE);
		       log("bp");
		       asm{
		       mov bx,bp
		       mov fo2,bx
		       }
		       log((DWord)fo2);
		       log("\n int90handler finished");*/
		       }
/*		       test=(DWord)(void far*)&fs+0x58;

		       test=fs+fo*256*256;
		       f=fopen("D:\\ret.txt","wb");
		       fwrite(&fs,200,1,f);
		       fclose(f);
		       f=fopen("D:\\call.txt","a+");
		       fprintf(f,"%lu %lu\n %s \n",fs,test,((PS_TEXT_MSG far*)test)->strptr);
		       fwrite((void far*)(((PS_TEXT_MSG far*)(((DWord)(void far*)(&fs))+0x54))->strptr),5,1,f);
		       fprintf(f," \n");
		       fclose(f);       */
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

int main(void)
{
	Word fso,foo;
	Byte far* ptr;
	DWord i;
	char buffer[100];

//	log("------------------------------------------------------");
	log("\n\n");
	log("               Starting new session");
	log("------------------------------------------------------");

//	dumpstacktest();

//	log("writing fast address table");
	//addCount=0;
	initInfo();
	for(i=1;i<15;i++)
	{
		DWord j;
		DWord *functable=(DWord*)LoadFuncTable(i);
		DWord grenze[]={0,52,19,0,8,25,3,37,31,4,20,26,0,0,0,0};//,46,4,10};

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

	addInfoName(7,0x78,"PrintStr");
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
	addInfoName(8,0x34,"DeleteDlg");

//	log("Kollisionen");

	//dumplog2((void far*)0x1e44000,0x1000,DUMP_FILE);

//	logfunctables();
/*	ps_exec_func34();
	ps_print("Reloading Pointer...",100,10,14,0);
	ps_gfx_exec_func();*/

//	int0x90_handler=getvect(0x90);
//	h=int0x90_handler;

//	ptr=(Byte far*)my90handler;
//	ptr+=0x10;

//        h=&jmpptr;

//	setvect(0x90,(void interrupt(*)(...))ptr);
	setvect(0x90,my90handler);
//	log("Reloaded Pointer");
//for(i=0;i<500000;i++);
//ps_print("Done...",100,40,14,0);
//ps_gfx_exec_func();
//for(i=0;i<500000;i++);
//exit(0);

//	asm int 0x90;
//	exit(0);

/*	ptr=(Byte far*)0xdc8001e4;

	if (*ptr==0xc8)
	{
		ps_print("Pointer OK... Rewriting...",100,40,14,0);
		ps_gfx_exec_func();

		*ptr=0xcb;
	}

	if (*ptr!=0xcb)
	{
		ps_print("write protected?",100,70,14,0);
		ps_gfx_exec_func();
	}
/*	ps_get_func_ptr(7,&fso,&foo);
	foo+=0x78;

	ptr=(DWord far*)((DWord)fso*256*256+foo);

		sprintf(buffer,"%lu",*ptr);
	ps_print(buffer,150,100,12,0);
	*ptr=(DWord)my_ltm;
		sprintf(buffer,"%lu",*ptr);
	ps_print(buffer,150,130,12,0);

   {
	Word fs,fo;
	asm{
	push ds
	mov ah,7
	int 0xff
//	les bx,addr
//	mov fo,dx
//	mov dx,ds
//	mov fs,dx
	mov bx,ds
	mov es,bx
	mov bx,dx
	mov ax,es:[bx+0x78]
	mov cx,es:[bx+0x7a]
	mov fo,ax
	mov fs,cx
	pop ds
	}
/*	ultoa((DWord)fs*256*256+fo,buffer,10);
 //	ps_exec_func34();
	ps_print(buffer,150,40,12,0);
	sprintf(buffer,"%X:%X",fs,fo);
	ps_print(buffer,150,70,12,0);
	sprintf(buffer,"%lu",*ptr);
	ps_print(buffer,150,100,12,0);
	sprintf(buffer,"%lu",(DWord)my_ltm);
	ps_print(buffer,150,130,12,0);
	sprintf(buffer,"%lu",fso*256*256+foo+78);
	ps_print(buffer,150,160,12,0);
}            */

	ps_print("Done.",200,190,14,0);
	ps_gfx_exec_func();

	for (i=0;i<500000;i++);

	//for (i=0;i<500000;i++);for (i=0;i<500000;i++);

	//setvect(0x90,int0x90_handler);
	log("going into deep standby....");
	keep(0, (_SS + (_SP/16) - _psp));
	return 0;
}

int oldmain(void)
{
//	char *buffer;
  //void far *p;
  //Word fs1;
  //DWord g;
  //unsigned long int h;
  //char far *cp;
  Word fs,fo,fs2,fo2,fs1,fo1,fsa,foa,fsg,fog;
    Word var=0;
//  Word a,b;
  void far* add;
  DWord a=0,j;
  FILE *f;
  ps_get_func_ptr(8,&fs,&fo);
  ps_get_func_ptr(7,&fsg,&fog);
  ps_get_func_ptr(0xa,&fsa,&foa);
  ps_get_func_ptr(1,&fs1,&fo1);

  f=fopen("D:\\vect.txt","w");
  for(i=0;i<256;i++)
  {
	char buffer[100];
	add=getvect(i);
	fs=hlp_ptr2seg((void*)add);
	fo=hlp_ptr2ofs((void*)add);
	ultoa((DWord)add,buffer,10);
	fprintf(f,buffer);
	fprintf(f," 0x%X: %04X:%04X %lu\n",i,fs,fo,add);

//  ps_exec_func34();
//  ps_print(buffer,320,10,12,0);
  }//  */
  fclose(f);
/* asm{
// mov ax,0xff00
// int 0x10
//mov bx,2
//mov ax,0x5801
//int 0x21
push cs
push offset back
push ss
push 0
mov bx, fs
mov es,bx
mov bx,fo
call dword ptr es:[bx+8]
mov bx,fo
call dword ptr es:[bx+0x14]
 }                         */
/* asm{
 push 2
 push 2
 mov bx,fs
 mov es,bx
 mov bx,fo
 call dword ptr es:[bx+0x30]
 mov fs2,dx
 mov fo2,ax
 mov es,dx
 mov bx, ax
 mov es:[bx+8],ds
 mov word ptr es:[bx+6],offset ka
 push word ptr es:[bx+0x23]
 push word ptr es:[bx+0x21]
 push 1
 mov bx,fs
 mov es,bx
 mov bx,fo
 call dword ptr es:[bx+4]
 mov bx,fs2
 mov es,bx
 mov bx,fo2
 push word ptr es:[bx+0x23]
 mov ax,es:[bx+0x21]
 push ax
 push ds
 push offset smsg
 mov bx, fs
 mov es,bx
 mov bx,fo
 call dword ptr es:[bx+0x10]
 mov bx,fs2
 mov es,bx
 mov bx,fo2
 push word ptr es:[bx+0x23]
 push 0
 push 100
 mov bx, fs
 mov es,bx
 mov bx,fo
 call dword ptr es:[bx+0x14]
 push fs2
 push fo2
  mov bx, fs
 mov es,bx
 mov bx,fo

 call dword ptr es:[bx+0x68]
 push 0
  mov bx, fs
 mov es,bx
 mov bx,fo

 call dword ptr es:[bx+0x78]
 }
 ps_exec_func34();
 asm{
 push fs2
 push fo2
 mov bx, fs
 mov es,bx
 mov bx,fo
 call dword ptr es:[bx+0x38]
 mov bx, fsa
 mov es,bx
 mov bx,foa
 push 0
 push 0x58
 push 0x2f0
 push 0x172
 push 0
 push 1
 call dword ptr es:[bx+0x38]
 }*/
/*while(!var)
 {
 Word rue,rue2;
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

   mov rue,ax
 mov bx,fs1
 mov es,bx
 mov bx,fo1
 call dword ptr es:[bx+0x14]
 mov rue2,ax
 }
  ps_exec_func34();
 itoa(rue,buffer,10);
 ps_print(buffer,320,10,12,0);
 itoa(rue2,buffer,10);
 ps_print(buffer,320,40,12,0);
 itoa(i,buffer,10);
 ps_print(buffer,320,70,12,0);

 }                            */
/* itoa(ka,buffer,10);
 ps_print(buffer,320,10,12,0);
   for (i=0; i<500000; i++);*/
// return 0;
/*  asm{
  mov bx,fs
  mov es,bx
  mov bx,fo
  mov ax,WORD PTR es:[bx+0x34]
  mov a,ax
  mov ax,WORD PTR es:[bx+0x36]
  mov b,ax
   }          */
   {
	Word fs,fo;
	asm{
	push ds
	mov ah,7
	int 0xff
//	les bx,addr
//	mov fo,dx
//	mov dx,ds
//	mov fs,dx
	mov bx,ds
	mov es,bx
	mov bx,dx
	mov ax,es:[bx+0x78]
	mov cx,es:[bx+0x7a]
	mov fo,ax
	mov fs,cx
	pop ds
	}
	ultoa((DWord)fs*256*256+fo,buffer,10);
	ps_exec_func34();
	ps_print(buffer,150,10,12,0);
	sprintf(buffer,"%X:%X",fs,fo);
	ps_print(buffer,150,40,12,0);
}
  //buffer=(char*)malloc(30000);
//  ps_print(smsg,320,10,12,0);
  ps_gfx_exec_func();
//DWord p=0x943b*256*256+0x21cc;
  //fs1=hlp_ptr2seg2(p);
//ps_get_func_ptr(PS_GFX_FUNC_NO,&fs,&fo);

  //p=(char far *)(unsigned long)(0x90070000);
  //g=2486894592;
  //h=2486894592;
  //p=(void far *)g;
  //cp=(char far *)g;
 // fs1=hlp_ptr2seg(p);
  //_fmemcpy(buffer,(void far*)(0x943b0000),30000);
/*  asm{
  mov es,fs
  mov bx,fo
  mov ax,es:[bx+20]
  mov fo,ax
  }*/

//  getch();

  for (i=0; i<500000; i++); // delay ~2

/*for (i=0;i<500000;i++)
{
  itoa(i,buffer,10);
  ps_exec_func34();
  ps_print(buffer,320,10,12,0);
} */
//for (i=0; i<500000; i++); // delay ~2

  // b=(void*)buffer;
/*asm{
	mov bp,offset buffer
//	mov es,seg b
	sidt ds:[bp]
//	sidt ptr b
}   */

/*	f=fopen("d:\\dump.txt","wb");
  for (i=0;i<0x10000;i+=0x1000)
      {
	DWord ptr;
	DWord written=0;
	char filename2[100];
	char buffer[10];
	char filename[]="D:\\log";
	filename2[0]=0;
	ultoa(i,buffer,16);
	strcat(filename2,filename);
	strcat(filename2,buffer);
	strcat(filename2,".txt");



	ptr=i*256*256;
	//written=fwrite((void far*)ptr,0x1000,16,f);
	for (j=0;j<0x10000;j++)
	{
		ptr++;written++;
		putc(*((Byte far*)ptr),f);
	}
	a+=written;
	ps_exec_func34();
	ps_print(buffer,200,10,14,0);
	ultoa(written,buffer,16);
	ps_print(buffer,200,40,14,0);
	ultoa(a,buffer,16);
	ps_print(buffer,200,70,14,0);
	ps_gfx_exec_func();

	KeyboardDispatcher();
  }
  fclose(f);*/
  {
  Word segm,ofs,segm2,ofs2;
  char bufferstr[100];
  KeyboardDispatcher();
  asm  {
  push ds
  mov bx,fsg
  mov es,bx
  mov bx,fog
  lds cx,dword ptr es:[bx+0x20]
  mov ofs,cx
  mov cx,ds
  mov segm,cx
  mov es,cx
  mov bx,ofs
  lds cx,dword ptr es:[bx]
  mov ofs2,cx
  mov cx,ds
  mov segm2,cx
  pop ds
  }
  ps_exec_func34();
  sprintf(bufferstr,"%X:%X",fsg,fog);
  ps_print(bufferstr,200,10,14,0);
  sprintf(bufferstr,"%X:%X",segm,ofs);
  ps_print(bufferstr,200,40,14,0);
  sprintf(bufferstr,"%X:%X",segm2,ofs2);
  ps_print(bufferstr,200,70,14,0);
  ps_gfx_exec_func();
  for (i=0;i<500000;i++);
  }
  while(1)
  {
  Byte stunde,min,sec;
  char bufferstr[30];
  asm{
     mov ah,2
     int 0x1a
     mov ax,cx
     mov stunde,ah
     mov min,al
     mov sec,dh
  }
  ps_exec_func34();
  sprintf(bufferstr,"%d:%d:%d",real(stunde),real(min),real(sec));
  ps_print(bufferstr,200,10,14,0);
  ps_gfx_exec_func();

  KeyboardDispatcher();
for (i=0;i<200000;i++);
  }
  //fprintf(f,"%X:%X\n",sseg,sofs);
  //fprintf(f,"%X\n",(DWord)bp);
  /*for(i=0;i<256;i++)
  {
	add=getvect(i);
	fs=hlp_ptr2seg((void*)add);
	fo=hlp_ptr2ofs((void*)add);
	ultoa((DWord)add,buffer,10);
	fprintf(f,buffer);
	fprintf(f," 0x%X: %04X:%04X %lu\n",i,fs,fo,add);

//  ps_exec_func34();
//  ps_print(buffer,320,10,12,0);
  }  */
  //fprintf(f,"%04X:%04X\n",a,b);
  //fwrite((Byte*)s.strptr,sizeof(smsg),1,f);
//  fwrite((void far*)(0x965b0),65535,1,f);
/*for (a=0;a<131070;a+=65535)
{
  a=(DWord)(void far*)a;
  fwrite((void far*)a,65535,1,f);
 // fflush(f);
  }
  fclose(f);      */
  /*save_segment("D:\\testfd80.bin",0xfd80);
  save_segment("D:\\testce6b.bin",0xce6b);
  save_segment("D:\\test0dbf.bin",0x0dbf);*/
  //save_segment("d:\\test943b.bin",0x943b);
  //save_segment("d:\\testdad9.bin",0xdad9);
  //save_segment("D:\\testd9da.bin",0xd9da);
  //save_segment("",0);
  return(0);
}
