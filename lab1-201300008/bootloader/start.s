/* Real Mode Hello World */
#  .code16
#  .global start
#  start:
#  	movw %cs, %ax
#  	movw %ax, %ds
#  	movw %ax, %es
#  	movw %ax, %ss
#  	movw $0x7d00, %ax
#  	movw %ax, %sp # setting stack pointer to 0x7d00
#  	pushw $13 # pushing the size to print into stack
#  	pushw $message # pushing the address of message into stack
#  	callw displayStr # calling the display function
#  loop:
#  	jmp loop
#
#  message:
#  	.string "Hello, World!\n\0"
#
#  displayStr:
#  	pushw %bp
#  	movw 4(%esp), %ax
#  	movw %ax, %bp
#  	movw 6(%esp), %cx
#  	movw $0x1301, %ax
#  	movw $0x000c, %bx
#  	movw $0x0000, %dx
#  	int $0x10
#  	popw %bp
#  	ret


/* Protected Mode Hello World */
.code16
.global start
start:
   #关闭中断
	cli                           

   #启动A20总线
	inb $0x92, %al
   orb $0x2, %al
   outb %al, $0x92



   #加载GDTR
   data32 addr32 lgdt gdtDesc     
                                   #启动保护模式
	
	#设置CR0的PE位（第0位）为1
	movl %cr0, %eax
	orl $0x1, %eax
	movl %eax, %cr0

	#长跳转切换至保护模式
	data32 ljmp $0x08, $start32     

.code32
start32:
	#初始化DS ES FS GS SS 初始化栈顶指针ESP
	movw $0x10, %ax                       
	movw %ax, %ds    #  Data Segment
	movw %ax, %es    #  Extra Segment
	movw %ax, %fs    #  FS
	movw %ax, %ss    #  Stack Segment                             
	movw $0x18, %ax  
	movw %ax, %gs    #  GS
	movl $0x7c00, %ebp
	movl %ebp, %esp 

	pushl $13 # pushing the size to print into stack
	pushl $message   # pushing the address of message into stack
	calll displayStr # calling the display function
loop:
	jmp loop

message:
	 .string "Hello, World!\n\0"

displayStr:
	 movl 4(%esp), %ebx
	 movl 8(%esp), %ecx
	 movl $((80*5+0)*2), %edi
	 movb $0x0c, %ah
nextChar:
	 movb (%ebx), %al
	 movw %ax, %gs:(%edi)
	 addl $2, %edi
	 incl %ebx
	 loopnz nextChar # loopnz decrease ecx by 1
	 ret

gdt:
   .hword 0,0,0,0   # GDT第一个表项必须为空
   
	#代码段描述符
	.hword  0xffff   # Limit
	.hword 0x0       # Base address[15:0]
	.byte 0x0        # Base address[23:16]
	.byte 0x9a       # 10011010b P:1,描述符在内存中; DPL:00,ring0; S:1,存储段[代码数据段]; 1010:代码段,执行,可读
	.byte 0xcf       # 11001111b G:Limit:1,4k; D/B:1,使用32位地址; 未使用:00,Limit:1111
	.byte 0x0        # Base address[31:24]:0

    
   #数据段描述符
	.hword 0xffff
	.hword 0x0 
	.byte 0x0 
	.byte 0x92       # 0010:数据段，读写
	.byte 0xcf 
	.byte 0x0 
       
   #视频段描述符
	.hword 0xffff 
	.hword 0x8000
	.byte 0x0b 
	.byte 0x92 
	.byte 0xcf
	.byte 0x0 
    
	
gdtDesc:
   .word (gdtDesc - gdt -1)
   .long gdt



/* Protected Mode Loading Hello World APP */
#.code16
#.global start
#start:
#	#关中断
#	cli                           
#
#	#启动A20总线
#	inb $0x92, %al
#	orb $0x2, %al
#	outb %al, $0x92
#
#	#加载GDTR
#	data32 addr32 lgdt gdtDesc     
#	
#	#设置CR0的PE位（第0位）为1
#	movl %cr0, %eax
#	orl $0x1, %eax
#	movl %eax, %cr0
#
#	#长跳转切换至保护模式
#	data32 ljmp $0x08, $start32     
#
#.code32
#start32:
#	#初始化DS ES FS GS SS
#	movw $0x10, %ax                       
#	movw %ax, %ds    
#	movw %ax, %es   
#	movw %ax, %fs  
#	movw %ax, %ss  
#	movw $0x18, %ax  
#	movw %ax, %gs 
#	
#	#初始化栈顶指针ESP
#	movl $0x7c00, %ebp
#	movl %ebp, %esp 
#	
#	# wk add
#	jmp bootMain
#
#gdt:
#	#GDT第一个表项必须为空
#	.hword 0,0,0,0  
#   
#	#代码段描述符
#	.hword  0xffff,0x0
#	.byte 0x0,0x9a,0xcf,0x0
#       
#   #数据段描述符
#	.hword 0xffff,0x0 
#	.byte 0x0,0x92,0xcf,0x0 
#       
#   #视频段描述符
#	.hword 0xffff,0x8000
#	.byte 0x0b,0x92,0xcf,0x0
#    
#	
#gdtDesc:
#   .word (gdtDesc - gdt -1)
#   .long gdt



