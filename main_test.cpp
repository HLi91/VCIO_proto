#include "cio.h"
#include <chrono>


int main(int argc, char *argv[]){
	
	VCIO * cio = new VCIO();
	cio->init("/cio", 1); 
	char buff[1024];
	memset(buff, 0, 1024);


	for (int j = 0; j < 3; j++) {
		int fd = cio->cio_fopen("1", "r");
		
		using namespace std::chrono;
		milliseconds ms = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch()
			);
		
		for (int i = 0; i < 1024 * 1024; i++)
			cio->cio_fwrite(buff, 1, 1024, fd);
		cio->cio_fclose(fd);

		using namespace std::chrono;
		milliseconds ms2 = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch()
			);
		cout << (ms2.count() - ms.count()) << endl;
		
	}


	


	for (int j = 0; j < 3; j++) {
		using namespace std::chrono;
		milliseconds ms = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch()
			);
		FILE* f = fopen("/mnt/tmpfs/testout", "w");
		
		for (int i = 0; i < 1024 * 1024; i++)
			fwrite(buff, 1, 1024, f);
		fclose(f);
		milliseconds ms2 = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch()
			);
		cout << (ms2.count() - ms.count()) << endl;
		

	}
}