#include "x86.h"
#include "device.h"

extern int displayRow;
extern int displayCol;

extern uint32_t keyBuffer[MAX_KEYBUFFER_SIZE];
extern int bufferHead;
extern int bufferTail;


void GProtectFaultHandle(struct TrapFrame *tf);
void moveStr(char *buf,int len, char *dst);
void KeyboardHandle(struct TrapFrame *tf);
void syscallHandle(struct TrapFrame *tf);
void syscallWrite(struct TrapFrame *tf);
void syscallPrint(struct TrapFrame *tf);
void syscallRead(struct TrapFrame *tf);
void syscallGetChar(struct TrapFrame *tf);
void syscallGetStr(struct TrapFrame *tf);

void irqHandle(struct TrapFrame *tf) { // pointer tf = esp
	/* Reassign segment register */
	asm volatile("movw %%ax, %%ds"::"a"(KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax, %%es"::"a"(KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax, %%fs"::"a"(KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax, %%gs"::"a"(KSEL(SEG_KDATA)));
	switch(tf->irq) {
		case -1:
			break;
		case 0xd:
			GProtectFaultHandle(tf);
			break;
		case 0x21:
			KeyboardHandle(tf);
			break;
		case 0x80:
			syscallHandle(tf);
			break;
		default:assert(0);
	}
}


void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}


void displat_ch(char character){
	uint16_t data = character | (0x0c << 8 );
	uint16_t pos = ( 80 *displayRow+displayCol)* 2 ;
	asm volatile("movw %0, (%1)"::"r"(data),"r"(pos+0xb8000));
}

void backspace(){
	if(displayCol > 0 && bufferTail > 0){
		displayCol--;
		bufferTail--;
		displat_ch(' ');
	}
}

void newline(){
	displayRow++;
	displayCol = 0;
	if (displayRow == 25)
	{
		displayRow = 24;
		scrollScreen();
	}
}

int Undisplayable(uint32_t code){
	uint32_t undis[] = {0x01, 0xf, 0x1d, 0x2a, 0x36, 0x37, 0x38, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, \
		0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x57, 0x58};
	int ret = 0;
	for(int k = 0; k < 35; k++){
		ret = ret || (code == undis[k]);
	}
	return ret;
}

void KeyboardHandle(struct TrapFrame *tf){
	uint32_t code = getKeyCode();
	if(code == 0xe){ // backspace
		backspace();
	}else if(code == 0x1c){ // enter
		/* newline(); */
		if(displayRow==24){
			newline();
		}
		else{	
			displat_ch('\n');
		}
	}else if(code < 0x81){ 
		if(Undisplayable(code)) return;
		char ascii = getChar(code);
		displat_ch(ascii);
		displayCol++;
		if (displayCol == 80)
		{
			newline();
		}
		keyBuffer[bufferTail++] = ascii;
	}
	updateCursor(displayRow, displayCol);
}

void syscallHandle(struct TrapFrame *tf) {
	switch(tf->eax) { // syscall number
		case 0:
			syscallWrite(tf);
			break; // for SYS_WRITE
		case 1:
			syscallRead(tf);
			break; // for SYS_READ
		default:break;
	}
}

void syscallWrite(struct TrapFrame *tf) {
	switch(tf->ecx) { // file descriptor
		case 0:
			syscallPrint(tf);
			break; // for STD_OUT
		default:break;
	}
}

void syscallPrint(struct TrapFrame *tf) {
	int sel =  USEL(SEG_UDATA); // wk: segment selector for user data, need further modification
	char *str = (char*)tf->edx;
	int size = tf->ebx;
	int pos = 0;
	char character = 0;
	uint16_t data = 0;
	asm volatile("movw %0, %%es"::"m"(sel));
	for (int i = 0; i < size; i++) {
		asm volatile("movb %%es:(%1), %0":"=r"(character):"r"(str+i));
		if (character == '\n')
		{
			newline();
		}
		else
		{
			data = character | (0x0c << 8);
			pos = (80 * displayRow + displayCol) * 2;
			asm volatile("movw %0, (%1)" ::"r"(data), "r"(pos + 0xb8000));
			displayCol++;
			if (displayCol == 80)
			{
				newline();
			}
		}
	}
	updateCursor(displayRow, displayCol);
	return;
}

void syscallRead(struct TrapFrame *tf){
	switch(tf->ecx){ //file descriptor
		case 0:
			syscallGetChar(tf);
			break; // for STD_IN
		case 1:
			syscallGetStr(tf);
			break; // for STD_STR
		default:break;
	}
}

void syscallGetChar(struct TrapFrame *tf){
	// wk add
	uint32_t code = getKeyCode();
	while(!code){
		code = getKeyCode();
	}
	char asc = getChar(code);
	displat_ch(asc);
	displayCol++;
	if (displayCol == 80) newline();
	updateCursor(displayRow, displayCol);
	while(1){
		code = getKeyCode();
		char character = getChar(code);
		if (character == '\n')
		{
			newline();
			updateCursor(displayRow, displayCol);
			break;
		}
	}
	tf->eax = asc;
}

void syscallGetStr(struct TrapFrame *tf){ 
	char buf[256];
	int len = 0;                          
	uint32_t pre, cur;
	pre = getKeyCode();
	do{ // ignore last enter
	cur = getKeyCode();
	}while(cur == 0x1c || cur == 0x1c + 0x80);
	while(1){
		do{
			cur = getKeyCode();
		} while(cur == pre);
		pre = cur;
		if(cur == 0x1c){
			newline();
			break;
		}
		else if(cur == 0xe){ // backspace
			if(len > 0){
				len--;
				if(displayCol > 0){
				displayCol--;
				displat_ch(' ');
			}
				else{
				displayRow--;
				displayCol = 79;
				displat_ch(' ');
				}
			}
		}
		else if(cur < 0x81){
			if(!Undisplayable(cur)) {
				char asc = getChar(cur);
				displat_ch(asc);
				if(cur){
					displayCol += 1;
					buf[len++] = asc;
				}
				if (displayCol == 80) newline();
			}
		}
		updateCursor(displayRow, displayCol);
	}
	putStr(buf);
	for(int k = 0; k < len; k++) ((char*)(tf->edx))[k] = buf[k];
	((char*)(tf->edx))[len] = 0;
}


