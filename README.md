MINIX 1.0 (OpenWATCOM + NASM + Bochs)  
  
简介：  
  Minix1.0是一个16位的微内核操作系统，原作者最先使用PCIX操作系统开发了  
minix，后来便在minix上继续开发，也有在dos下使用C86编译器和MASM4.0汇编  
器编译minix1.0项目。上述三个开发环境已经过时对学习和实践造成了不小的  
障碍，为此本人对minix1.0做了相应的修改，能使其在现代linux、windows上  
使用openwatcom编译器和nasm汇编器构建系统，并可使用bochs模拟器运行和  
调试系统。在此把修改后的源代码上传github，为后来者提供学习的便利，如  
有疑问可联系我e-mail:mumu3w@outlook.com。  
  
1、安装NASM  
  CentOS7：  
  yum install nasm  
  Ubuntu:  
  sudo apt install nasm  
  
2、安装openwatcom  
  https://github.com/open-watcom/travis-ci-ow-builds/archive/master.zip  
  Rename to watcom  
  mv ./watcom /opt/watcom  
    
  export WATCOM=/opt/watcom  
  export PATH=$WATCOM/binl64:$PATH (32:export PATH=$WATCOM/binl:$PATH)  
    
3、构建  
  cd MINIX1  
  ./build.sh  
    
4、清理  
  cd MINIX1  
  ./clean.sh  

![image](https://github.com/mumu3w/MINIX1/blob/master/tools/2.PNG)

![image](https://github.com/mumu3w/MINIX1/blob/master/tools/1.PNG)
