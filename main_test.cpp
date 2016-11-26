#include "cio.h"
#include <chrono>


int main(int argc, char *argv[]){
	
	VCIO * cio = new VCIO();
	cio->init("/cio", 1); 
	char buff[1024*1024];
	memset(buff, 0, 1024*1024);
	int rounds;
	int repeat = 10;
	int buff_size = 512;
	//cin >> buff_size;
	rounds = 1024 * 1024 * 1024 / buff_size;
	cout << buff_size << " btyes write " << rounds << " times totally 1G" << endl;

	cout << "VCIO write" << endl;
	
	
	for (int j = 0; j < repeat; j++) {
		
		
		using namespace std::chrono;
		milliseconds ms = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch()
			);
		int fd = cio->cio_fopen("1", "r");
		for (int i = 0; i < rounds; i++)
			cio->cio_fwrite(buff, 1, buff_size, fd);
		cio->cio_fclose(fd);

		using namespace std::chrono;
		milliseconds ms2 = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch()
			);
		cout << (ms2.count() - ms.count()) << endl;
		
	}


	cout << "VCIO read" << endl;
	for (int j = 0; j < repeat; j++) {


		using namespace std::chrono;
		milliseconds ms = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch()
			);
		int fd = cio->cio_fopen("1", "r");
		for (int i = 0; i < rounds; i++)
			cio->cio_fread(buff, 1, buff_size, fd);
		cio->cio_fclose(fd);

		using namespace std::chrono;
		milliseconds ms2 = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch()
			);
		cout << (ms2.count() - ms.count()) << endl;

	}

	return 0;
	cout << "tmpfs write" << endl;

	for (int j = 0; j < repeat; j++) {
		using namespace std::chrono;
		milliseconds ms = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch()
			);
		FILE* f = fopen("/mnt/tmpfs/testout", "w");
		
		for (int i = 0; i < rounds; i++)
			fwrite(buff, 1, buff_size, f);
		fclose(f);
		milliseconds ms2 = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch()
			);
		cout << (ms2.count() - ms.count()) << endl;
		

	}

	cout << "tmpfs read" << endl;

	for (int j = 0; j < repeat; j++) {
		using namespace std::chrono;
		milliseconds ms = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch()
			);
		FILE* f = fopen("/mnt/tmpfs/testout", "r");

		for (int i = 0; i < rounds; i++)
			fread(buff, 1, buff_size, f);

		fclose(f);
		milliseconds ms2 = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch()
			);
		cout << (ms2.count() - ms.count()) << endl;


	}

	

	cout << "ssd write" << endl;
	for (int j = 0; j < repeat; j++) {
		using namespace std::chrono;
		milliseconds ms = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch()
			);
		FILE* f = fopen("testout", "w");

		for (int i = 0; i < rounds; i++)
			fwrite(buff, 1, buff_size, f);
		fclose(f);
		milliseconds ms2 = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch()
			);
		cout << (ms2.count() - ms.count()) << endl;


	}

	cout << "ssd read" << endl;
	for (int j = 0; j < repeat; j++) {
		using namespace std::chrono;
		milliseconds ms = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch()
			);
		FILE* f = fopen("testout2", "r");

		for (int i = 0; i < rounds; i++)
			fread(buff, 1, buff_size, f);
		fclose(f);
		milliseconds ms2 = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch()
			);
		cout << (ms2.count() - ms.count()) << endl;


	}
}