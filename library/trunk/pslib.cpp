#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <string.h>

typedef unsigned char Byte;            // size 8 bit, 0-255
typedef unsigned int  Word;            // size 16 bit, 0-65535
typedef unsigned long DWord;           // size 32 bit, 0-4294967295

// Key definitions for Powershot S30
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

typedef struct
{
	Word event;	// 0
	Word res1;	// 2
	Byte res2;	// 4
	DWord keyMap;	// 5
}KeyState;

typedef int (far pascal KEY_STATE_CALLBACK_FUNC)(KeyState far*keys,DWord lparam);

/*
GetKeyState	-	Calls the callback function for every state change since the last call once
SpecialAbortConditionArised
		-	true, if battery is opened or switched from playback to record mode
GetTicks	-	gets the milliseconds since start of the cam
*/
typedef struct
{
	void far*(far pascal *func0)();						//0x00
	DWord res1;								//0x04
	int (far pascal *GetKeyState)(KEY_STATE_CALLBACK_FUNC far*,DWord lparam);	//0x08
	char res2[8];								//0x0c
	int (far pascal *SpecialAbortConditionArised)();			//0x14
	char res3[0xa4];								//0x18
	void (far pascal *GetTicks)(DWord *d);					//0xbc
} FUNCS1_TABLE;

typedef struct                         // text message structure
{
  Word x;                              // x-position [0-639]
  Word y;                              // y-position [0-479]
  Byte far*strptr;                     // RM pointer to 0-terminated string
  Word bkcolor;                        // background color
  Word txtcolor;                       // text color
  Word unknown;                        // ?, let it be 0
} PS_TEXT_MSG;


/*
Draw	-	Puts output buffer to screen
Clear	-	Clears buffer with selected background color
		0 is transparent
Print	-	Prints message to buffer
*/
typedef struct
{
	char res[0x20];					//0x00
	int (far pascal*Draw)(Word res);		//0x20
	char res2[0x10];				//0x24
	int (far pascal*Clear)(Word res1,Word color);	//0x34
	char res3[0x08];				//0x38
	void (far pascal*FillRectangle)(Word left,Word top,Word width,Word height,Word unk,Word color);	//0x40
	char res4[0x34];				//0x44
	int (far pascal*Print)(PS_TEXT_MSG far*msg);	//0x78
} FUNCS7_TABLE;

void far*LoadFuncTable(Byte number)
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

FUNCS1_TABLE far*funcs1;
FUNCS7_TABLE far*funcs7;
void far*funcs8;
void far*funcsA;

void LoadFuncTables()
{
	funcs1=(FUNCS1_TABLE far*)LoadFuncTable(1);
	funcs7=(FUNCS7_TABLE far*)LoadFuncTable(7);
	funcs8=LoadFuncTable(8);
	funcsA=LoadFuncTable(0xA);
}

char logfilename[]="D:\\lognew.txt";
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
		sprintf(strbuf,"D:\\b%04X%u.txt",FP_SEG(pt),dumpnr);
		f2=fopen(strbuf,"wb");
	}

	fprintf(f,"Dumping data Ptr: 0x%08X Size: 0x%08X\n\n",(DWord)pt,(DWord)size);

	strbuf[0]=0;
	for(i=0;i<size+16-(size%16);i++)
	{
		if (i%16==0)
			fprintf(f,"%04X:%04X\t",FP_SEG((void *)(((DWord)pt)+i)),FP_OFF((void *)(((DWord)pt)+i)));

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

void ColorTest()
{
	PS_TEXT_MSG tm;
	DWord i,j;

	for (i=0;i<16;i++)
	{
	tm.x=100;
	tm.y=240;
	tm.strptr="Die wurst";
	tm.bkcolor=0;
	tm.txtcolor=12;
	tm.unknown=0;
	funcs7->Clear(10000,i);
	funcs7->Print(&tm);
	if (funcs1->SpecialAbortConditionArised())
	{
		tm.y=270;
		tm.strptr="Kamelhaarsuppe";
		funcs7->Print(&tm);
	}
	funcs7->Draw(0);
	for(j=0;j<200000;j++);
	}
	for(i=0;i<500000;i++);
}

int far pascal WaitForReleaseKeyCB(KeyState far*keys,Word *wparam)
{
	if (keys->keyMap&KEY_RELEASE_HALF)
		*wparam=1;
	else if (keys->keyMap&KEY_ZOOM_DEC)
		*wparam=2;
	else if (keys->keyMap&KEY_ZOOM_INC)
		*wparam=3;
	else
		*wparam=0;
	return 0;
}

Word WaitForKey()
{
	Word wait=0;
	while (!wait)
		funcs1->GetKeyState((KEY_STATE_CALLBACK_FUNC far*)WaitForReleaseKeyCB,(DWord)&wait);

	return wait;
}

typedef struct
{
	Word far*wparam;
	Word ds;
}CB_INFO;
int far pascal mykeyhandler(KeyState far*keys,DWord lparam)
{
	char buffer[50];
	PS_TEXT_MSG tm;
	CB_INFO *pcbi=(CB_INFO*)lparam;
	void far*stack;
	asm push ds;		// save old DS
	_DS=pcbi->ds;		// DS is wrong, so we have to adjust it, if we want
				// want to access global variables (like funcs7)
	stack=MK_FP(_SS,_BP);


	if (keys->keyMap&KEY_WB&&(keys->keyMap&KEY_RELEASE_HALF))
		*(pcbi->wparam)=30000;
	else
		(*(pcbi->wparam))++;

	funcs7->Clear(0,0);

	tm.unknown=0;
	tm.txtcolor=12;
	tm.bkcolor=0;
	tm.x=40;
	tm.y=10;

	ultoa(keys->keyMap,buffer,2);
	sprintf(buffer,"%032s",buffer);
	tm.strptr=buffer;
	funcs7->Print(&tm);
	tm.y+=30;
	itoa(keys->res1,buffer,10);
	funcs7->Print(&tm);
	tm.y+=30;
	itoa((Word)keys->res2,buffer,10);
	funcs7->Print(&tm);
	tm.y+=30;
	itoa(*(pcbi->wparam),buffer,10);
	funcs7->Print(&tm);
	tm.y+=30;
	itoa(keys->event,buffer,10);
	funcs7->Print(&tm);

	funcs7->Draw(0);

	sprintf(buffer,"Keymap %lu",keys->keyMap);
	log(buffer);
	log((DWord)keys->res1);
	log((DWord)keys->res2);
	dumplog(stack,30);

	asm pop ds;		// restore ds

	return 0;
}

void KeyTest()
{
	DWord i;
	Word wparam=0;
	CB_INFO cbi;
	cbi.wparam=&wparam;
	cbi.ds=_DS;
	while(wparam<30000)
	{
		log("KeyTest");
		funcs1->GetKeyState(&mykeyhandler,(DWord)&cbi);
		for(i=0;i<50000;i++);
	}
}

void print(Byte *buffer,Word top,Word left)
{
	PS_TEXT_MSG tm;
	tm.unknown=0;
	tm.txtcolor=14;
	tm.bkcolor=0;
	tm.x=top;
	tm.y=left;
	tm.strptr=buffer;
	funcs7->Print(&tm);
}

class Reader
{
public:
	virtual DWord GetSize()=0;
	virtual DWord Seek(DWord pos)=0;
	virtual Byte *GetBytes(DWord length)=0;
	virtual Byte IsEOF()=0;
	Byte GetByte()
	{
		return *GetBytes(1);
	}
	Word GetWord()
	{
		return *(Word*)GetBytes(2);
	}
	DWord GetDWord()
	{
		return *(DWord*)GetBytes(4);
	}
};

class BufferReader:public Reader
{
private:
	Byte far*m_buffer;
	DWord m_length;
	DWord m_pos;
public:
	BufferReader(Byte *buffer,DWord len):m_buffer(buffer),m_length(len),m_pos(0)
	{}
	virtual DWord GetSize()
	{
		return m_length;
	}
	virtual Byte*GetBytes(DWord len)
	{
		if (m_pos+len<m_length)
		{
			Byte*b=m_buffer+m_pos;
			m_pos+=len;
			return b;
		}
		else
			return 0;
	}
	virtual Byte IsEOF()
	{
		if (m_pos<m_length)
			return 0;
		else
			return 1;
	}
	virtual DWord Seek(DWord pos)
	{
		if (pos<m_length)
		{
			DWord old=m_pos;
			m_pos=pos;
			return old;
		}
		else
			return (DWord)-1;
	}
};

class LineReader
{
public:
//	virtual DWord GetLine(Byte *buffer,int size)=0;
	virtual Byte *GetLine()=0;
	virtual DWord Seek(DWord line)=0;
	virtual DWord GetLineCount()=0;
	virtual Byte IsEOF()=0;
};

typedef struct LineItemS
{
	struct LineItemS *next;
	struct LineItemS *prev;
	Byte *Line;
}LineItem;

class Log:public LineReader
{
private:
	LineItem *m_firstLine;
	LineItem *m_lastLine;
	LineItem *m_curLine;
	DWord m_curLineNo;
	DWord m_cLines;
public:
	Log()
	{
		m_firstLine=0;
		m_lastLine=0;
		m_curLine=0;
		m_cLines=0;
	}
	virtual ~Log()
	{
		LineItem *li=m_firstLine;
		while(li!=0)
		{
			LineItem *next;
			delete li->Line;
			next=li->next;
			delete li;
			li=next;
		}
	}
	virtual DWord GetLineCount()
	{
		return m_cLines;
	}
	virtual DWord Seek(DWord line)
	{
		if (line<m_cLines)
		{
			DWord old=m_curLineNo;
			if (line>=m_curLineNo&&m_curLine)
			{
				while(line!=m_curLineNo)
					next();
			}
			else
			{
				if (line<m_curLineNo-line||!m_curLine)
				{
					m_curLineNo=0;
					m_curLine=m_firstLine;
					while(line!=m_curLineNo)
						next();
				}
				else
				{
					while(line!=m_curLineNo)
					{
						m_curLine=m_curLine->prev;
						m_curLineNo--;
					}
				}
			}
			return old;
		}
		else
			return (DWord)-1;
	}
	void next()
	{
		if (m_curLineNo<m_cLines)
		{
			m_curLineNo++;
			m_curLine=m_curLine->next;
		}
	}
	virtual Byte *GetLine()
	{
		Byte *back;
		if (!m_curLine)
			return 0;

		back=m_curLine->Line;
		next();
		return back;
	}
	virtual Byte IsEOF()
	{
		return (m_curLineNo>=m_cLines);
	}
	void Add(Byte *ent)
	{
		LineItem *li=new LineItem;
		DWord len=strlen(ent);
		li->Line=new Byte[len+1];
		strcpy(li->Line,ent);
		li->prev=m_lastLine;
		li->next=0;
		m_lastLine->next=li;
		m_lastLine=li;
		m_cLines++;
		if(!m_firstLine)
			m_firstLine=li;
	}

};

/*class ArrayList
{
	DWord m_curSize;
	DWord m_maxSize;
	void m_array;
public:
	ArrayList(DWord maxsize)
	{
		m_maxSize=maxsize;
		m_cursize=0;
		m_array=new DWord[maxsize];
	}
	DWord
};*/

class WrappingReader:public LineReader
{
	LineReader *m_reader;
	Word m_maxChars;
	DWord m_curLine;
	DWord m_curOffset;
public:
	WrappingReader(LineReader *r,Word maxC)
	{
		m_reader=r;
		m_maxChars=maxC;
	}
	virtual Byte* GetLine()
	{
		Byte *buffer=new Byte[m_maxChars+1];
		Byte *line;
		DWord len;
		m_reader->Seek(m_curLine);
		line=m_reader->GetLine();
		if (!line)
			return 0;

		len=strlen(line);
		if (m_curOffset+m_maxChars>=len)
		{
			strcpy(buffer,line+m_curOffset);
			m_curOffset=0;
			m_curLine++;
		}
		else
		{
			memcpy(buffer,line+m_curOffset,m_maxChars);
			buffer[m_maxChars]=0;
			m_curOffset+=m_maxChars;
		}
		return buffer;
	}
	virtual DWord GetLineCount()
	{
		DWord size=0;
		m_reader->Seek(0);
		for(;!m_reader->IsEOF();)
		{
			Byte *line=m_reader->GetLine();
			DWord len=strlen(line);
			size+=(len/m_maxChars)+1;
		}
		return size;
	}
	virtual DWord Seek(DWord linenumber)
	{
		DWord i=0;
		DWord readerLine=0;
		m_reader->Seek(0);
		for(;!m_reader->IsEOF();readerLine++)
		{
			Byte *line=m_reader->GetLine();
			DWord len=strlen(line);
			DWord offset=0;
			while(offset<len)
			{
				if (i==linenumber)
				{
					DWord oldline=m_curLine;
					m_curLine=readerLine;
					m_curOffset=offset;
					return oldline;
				}
				i++;
				offset+=m_maxChars;
			}
		}
		return (DWord)-1;
	}
	virtual Byte IsEOF()
	{
		if (m_curLine>=m_reader->GetLineCount())
			return 1;
		else
			return 0;
	}
};

#define CHAR_WIDTH 18
#define CHAR_HEIGHT 34

class ScrollDisplay
{
private:
	Word m_width;
	Word m_height;
	WrappingReader *m_reader;
	Word m_charsperline;
	Word m_cLines;
	Word m_linespacing;
//	Byte m_wrap;
	Word m_top;
	Word m_left;
	Word m_pos;
public:
	ScrollDisplay(LineReader *reader,Word left,Word top,Word width,Word height)
	{
		m_top=top;
		m_left=left;
		m_width=width;
		m_height=height;
		m_linespacing=0;

		m_cLines=m_height/(CHAR_HEIGHT+m_linespacing);
		m_charsperline=m_width/(CHAR_WIDTH);

		m_reader=new WrappingReader(reader,m_charsperline);
  //		m_wrap=1;
		m_pos=0;
	}
	virtual ~ScrollDisplay()
	{
		delete m_reader;
	}
	void Scroll(int count)
	{
		if (count+(int)m_pos<0)
			m_pos=0;
		else if (count+(int)m_pos+m_cLines<=m_reader->GetLineCount())
                	m_pos+=count;
	}
	void Draw()
	{
		DWord i;
		m_reader->Seek(m_pos);
		for(i=0;i<m_cLines&&!m_reader->IsEOF();i++)
		{
			Byte *line=m_reader->GetLine();
			if (!line)
				return;
			print(line,m_left,m_top+i*(CHAR_HEIGHT+m_linespacing));
			delete line;
		}
	}
};


void ScrollDisplayTest()
{
	Word key;
	Log *l=new Log();
	ScrollDisplay sd(l,20,20,600,440);
	l->Add("TestText");
	l->Add("Die Wurst");
	l->Add("also des isch jetzt mal wirklich ein extrem langer Text, aber da kann man nichts machen");
	l->Add("und noch  mehr Text");
	l->Add("noch viel mehr Text");
	l->Add("oh mein Gott das wird jetzt richtig viel");
	l->Add("also so richtig extrem viel Text wird erst jetzt angezeigt, aber das war ja auch mal n”tig");
	l->Add("Da passt jetzt so krass viel Text drauf, dass ist einfach nicht mehr so ganz normal, aber was will man machen ;)?!?");

	funcs7->Clear(0,7);
	sd.Draw();
	funcs7->Draw(0);

	key=WaitForKey();

	while (key!=1)
	{
		switch(key)
		{
			case 2:
				sd.Scroll(-1);
				break;
			case 3:
				sd.Scroll(1);
				break;
		}
		funcs7->Clear(0,7);
		sd.Draw();
		funcs7->Draw(0);

		key=WaitForKey();
	}

	delete l;
}

typedef struct
{
	void (pascal far*func0)(void far*);			//0x00
	void (pascal far*func4)(void far*,Word);      //0x04
	char res2[0x8];			//0x08
	void (pascal far*func10)(void far*,void far*);			//0x10
	void (pascal far*func14)(void far*,Word left);			//0x14
	char res3[0x18];		//0x18
	Byte *(pascal far *func30)(Word unk1,Word unk2);     //0x30
	void (pascal far*func34)(void far*);			//0x34
	void (pascal far*func38)(void far*);			//0x38
	char res5[0x2c];		//0x3c
	void (pascal far*func68)(void far*);			//0x68
	char res6[0xc];			//0x6c
	void (pascal far*func78)(Word);			//0x78
}FUNCS8_TABLE;

void funcATest()
{
	FUNCS8_TABLE*f=(FUNCS8_TABLE*)funcs8;
	Byte *b;

	funcs7->Clear(0,0);

	asm{
	les bx,dword ptr funcsA
	push 0	//x
	push 88 //y
	push 752//width
	push 370//height
	push 0
	push 5//border color
	call dword ptr es:[bx+38h]     // draw frame
	}

	b=f->func30(2,2);//alloc DlgBox

	f->func4(*((void**)(b+0x21)),1);   // release button
	f->func10(*((void**)(b+0x21)),"Test"); // set button text
	f->func14(*((void**)(b+0x21)),60); // set button position


	//f->func68(b);
	//f->func78(0);
	f->func38(b);//draw dlg

//	funcs7->FillRectangle(0,88,752,370,0,7);

	funcs7->Draw(0);

	WaitForKey();

	f->func4(*((void**)(b+0x21)),2);   //push button
	f->func0(*((void**)(b+0x21)));     //redraw button

	funcs7->Draw(0);


	WaitForKey();

        f->func34(b);// deleteDlgBox
}

void main()
{
	DWord i,j;

	LoadFuncTables();

	funcATest();

//	ScrollDisplayTest();

//	KeyTest();
}