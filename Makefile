all: cio.o fileDescriptor.o fix_slab.o HashManager.o InodeManager.o RegSlabManager.o regular_slab.o main_test.o 
	g++ -o output cio.o fileDescriptor.o fix_slab.o HashManager.o InodeManager.o RegSlabManager.o regular_slab.o  main_test.o -pthread -g -lrt

cio.o: cio.cpp
	g++ -c -std=c++11 -pthread cio.cpp -g
    
fileDescriptor.o: fileDescriptor.cpp
	g++ -c -std=c++11 -pthread fileDescriptor.cpp -g

fix_slab.o: fix_slab.cpp
	g++ -c -std=c++11 -pthread fix_slab.cpp -g -lrt
	
HashManager.o: HashManager.cpp
	g++ -c -std=c++11 -pthread HashManager.cpp -g
    
InodeManager.o: InodeManager.cpp
	g++ -c -std=c++11 -pthread InodeManager.cpp -g
RegSlabManager.o: RegSlabManager.cpp
	g++ -c -std=c++11 -pthread RegSlabManager.cpp -g

regular_slab.o: regular_slab.cpp
	g++ -c -std=c++11 -pthread regular_slab.cpp -g -lrt
	
main_test.o:
	g++ -c -std=c++11 -pthread main_test.cpp -g
    
reset:
	rm -f /dev/shm/cio*
	rm -f testout
	
	
clean:
	rm -f *o output testout
	rm -f /dev/shm/cio*
    
  







