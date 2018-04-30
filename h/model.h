

segment _TEXT class=CODE align=1
segment _DATABEG class=DATA align=16
segment _DATA class=DATA 
segment CONST class=DATA 
segment CONST2 class=DATA 
segment _DATAEND class=DATA
segment _BSS class=BSS align=2
segment _BSSEND class=BSS align=1
segment STACK class=STACK align=16

group   DGROUP _DATABEG _DATA _DATAEND _BSS _BSSEND STACK

%ifdef _COM_
segment _TEXT class=CODE align=1
segment _TEXTEND class=CODE align=1
segment _DATABEG class=DATA align=2
segment _DATA class=DATA 
segment CONST class=DATA 
segment CONST2 class=DATA 
segment _DATAEND class=DATA
segment _BSS class=BSS align=2
segment _BSSEND class=BSS align=1
segment STACK class=STACK align=16

group   DGROUP _TEXT _TEXTEND _DATABEG _DATA _DATAEND _BSS _BSSEND STACK	

%endif