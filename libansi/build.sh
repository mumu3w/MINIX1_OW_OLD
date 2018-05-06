#!/bin/bash


cd assert   
rm *.obj 
wmake
cd ..

cd ctype  
rm *.obj 
wmake
cd ..

cd float 
rm *.obj 
wmake
cd ..

cd locale  
rm *.obj 
wmake
cd ..

cd stdio   
rm *.obj 
wmake
cd ..

cd string  
rm *.obj 
wmake
cd ..

cd time
rm *.obj 
wmake
cd ..

cd errno 
rm *.obj 
wmake
cd ..

cd math
rm *.obj 
wmake
cd ..

cd stdlib
rm *.obj 
wmake
cd ..
