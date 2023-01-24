#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AES32.h"
#include "file_Padding.h"

using namespace std;

//패딩 제거 함수
void erase_padding(const char* source, const char* dest) //패딩파일, 패딩을 제거한 부분을 저장할 파일
{
	ifstream fin;
	ofstream fout;

	fin.open(source, ios::binary);
	if (fin.fail()) {
		cout << "Input File Open Error" << endl;
		return;
	}

	fout.open(dest, ios::binary);
	if (fout.fail()) {
		cout << "Output File Open Error" << endl;
		return;
	}

	int file_len;
	file_len = FileSize(source);
	int num_blocks = file_len / 16 + 1;

	byte buffer[16];

	for (int i = 0; i < num_blocks - 2; i++) {
		for (int j = 0; j < 16; j++) {
			fin.read((char*)&buffer[j], 1);
			fout.write((char*)&buffer[j], 1); //패딩된 부분의 전 블록까지 dest에 저장
		}
	}

	byte last_block[16];
	for (int i = 0; i < 16; i++) {
		fin.read((char*)&last_block[i], 1); //마지막 블록을 last_block에 저장
	}

	for (int i = 0; i < 16; i++) { //마지막 블록에서 패딩된 부분이 나올때까지만 dest에 저장함
		if(last_block[i] == 0x80){
			if (i == 15) break; 
			else {
				if (last_block[i + 1] == 0x00) break; 
				else {
					fout.write((char*)&last_block[i], 1);
				}
			}
		}
		else {
			fout.write((char*)&last_block[i], 1);
		}
	}
}

//AES파일암호화(ECB모드)
void file_AES_ECB(const char* source, byte key[16], const char* dest) //평문파일, 키, 암호문파일
{
	u32 rk[11][4];
	ifstream fin;
	ofstream fout;

	AES32_Enc_KeySchedule(key, rk); //암호화를 위한 라운드키를 획득

	const char* padput_file = "AES_ECB-Padded.bin"; 
	FilePadding(source, padput_file); //파일을 패딩하여 저장

	int file_len;
	file_len = FileSize(padput_file); //패딩된 파일의 길이를 재서 file_len에 저장
	int num_blocks = file_len / 16 + 1;

	byte pt[16];
	byte ct[16];

	fin.open(padput_file, ios::binary);
	if (fin.fail()) {
		cout << "Input File Open Error" << endl;
		return;
	}

	fout.open(dest, ios::binary);
	if (fout.fail()) {
		cout << "Output File Open Error" << endl;
		return;
	}

	for (int i = 0; i < num_blocks-1; i++) {
		for (int j = 0; j < 16; j++) {
			fin.read((char*)&pt[j], 1); //pt에 평문파일들을 1블록씩 입력
		}
		AES32_Encrypt(pt, rk, ct); //pt를 암호화하여 ct에 암호문 1블럭 입력
		for (int j = 0; j < 16; j++) {
			fout.write((char*)&ct[j], 1); //암호화파일에 ct를 1블록씩 입력
		}
	}
}

//AES파일복호화(ECB모드)
void Inv_file_AES_ECB(const char* source, byte key[16], const char* dest) //암호문파일, 키, 복호화파일
{
	u32 rk[11][4];
	ifstream fin;
	ofstream fout;

	AES32_Dec_KeySchedule(key, rk); //복호화를 위한 라운드키 획득

	const char* padput_file = "Inv_AES_ECB-Padded.bin";

	int file_len;
	file_len = FileSize(source);
	int num_blocks = file_len / 16 + 1;

	byte pt[16];
	byte ct[16];

	fin.open(source, ios::binary);
	if (fin.fail()) {
		cout << "Input File Open Error" << endl;
		return;
	}

	fout.open(padput_file, ios::binary);
	if (fout.fail()) {
		cout << "Output File Open Error" << endl;
		return;
	}

	for (int i = 0; i < num_blocks - 1; i++) {
		for (int j = 0; j < 16; j++) {
			fin.read((char*)&ct[j], 1); //ct에 암호문파일을 1블록씩 입력
		}
		AES32_EqDecrypt(ct, rk, pt); //ct를 복호화하여 pt에 복호화문 1블럭 입력
		for (int j = 0; j < 16; j++) {
			fout.write((char*)&pt[j], 1); //padout_file에 패딩이 포함된 평문 입력
		}
	}

	fin.seekg(0, ios::beg);
	fout.seekp(0, ios::beg);

	erase_padding(padput_file, dest); //erase_padding에 padout_file을 넣어 패딩부분을 지우기
}

//AES파일암호화(CBC모드)
void file_AES_CBC(const char* source, byte IV[16], byte key[16], const char* dest) //평문파일, 키, 암호문파일
{
	u32 rk[11][4];
	ifstream fin;
	ofstream fout;

	AES32_Enc_KeySchedule(key, rk); //암호화를 위한 라운드키 획득

	const char* output_file = "AES_CBC-Padded.bin";
	FilePadding(source, output_file); //파일을 패딩하여 저장

	int file_len;
	file_len = FileSize(output_file);
	int num_blocks = file_len / 16 + 1;

	byte pt[16];
	byte ct[16];

	fin.open(output_file, ios::binary);
	if (fin.fail()) {
		cout << "Input File Open Error" << endl;
		return;
	}

	fout.open(dest, ios::binary);
	if (fout.fail()) {
		cout << "Output File Open Error" << endl;
		return;
	}

	for (int i = 0; i < num_blocks - 1; i++) {
		if (i == 0) { //CBC 첫블록
			for (int j = 0; j < 16; j++) {
				fin.read((char*)&pt[j], 1); //CBC의 첫 블록을 pt에 저장
				pt[j] = pt[j] ^ IV[j]; //pt와 초기벡터를 XOR연산 수행
			}
			AES32_Encrypt(pt, rk, ct); //pt를 암호화하여 ct에 암호문 1블록 입력
			for (int j = 0; j < 16; j++) {
				fout.write((char*)&ct[j], 1); //암호화파일에 ct를 1블록 입력
			}
		}
		else { //첫블록 이후
			for (int j = 0; j < 16; j++) {
				fin.read((char*)&pt[j], 1); //pt에 평문파일들을 1블록씩 입력
				pt[j] = pt[j] ^ ct[j]; //pt와 직전의 암호문 블록을 XOR연산 수행
			}
			AES32_Encrypt(pt, rk, ct); //pt를 암호화하여 ct에 암호문 1블록 입력
			for (int j = 0; j < 16; j++) {
				fout.write((char*)&ct[j], 1); //암호화파일에 ct를 1블록씩 입력
			}
		}
	}

}

//AES파일복호화(CBC모드)
void Inv_file_AES_CBC(const char* source, byte IV[16], byte key[16], const char* dest) //암호문파일, 키, 복호화파일
{
	u32 rk[11][4];
	ifstream fin;
	ofstream fout;

	AES32_Dec_KeySchedule(key, rk); 
	
	const char* padput_file = "Inv_AES_CBC-Padded.bin"; //복호화를 위한 라운드키 획득

	int file_len;
	file_len = FileSize(source);
	int num_blocks = file_len / 16 + 1;

	byte pt[16];
	byte ct1[16];
	byte ct2[16];

	fin.open(source, ios::binary);
	if (fin.fail()) {
		cout << "Input File Open Error" << endl;
		return;
	}

	fout.open(padput_file, ios::binary);
	if (fout.fail()) {
		cout << "Output File Open Error" << endl;
		return;
	}

	for (int i = 0; i < num_blocks - 1; i++) {
		if (i == 0) { //CBC 첫블록
			for (int j = 0; j < 16; j++) {
				fin.read((char*)&ct1[j], 1); //암호문파일의 첫블록을 ct1에 저장
			}
			AES32_EqDecrypt(ct1, rk, pt); //ct1을 복호화하여 pt에 평문(xor연산 필요) 1블록 입력
			for (int j = 0; j < 16; j++) {
				pt[j] = pt[j] ^ IV[j]; //pt와 초기벡터를 XOR 연산 수행
				fout.write((char*)&pt[j], 1); //padput_file에 패딩이 포함된 pt를 1블록 입력
			}
		}
		else { //첫블록 이후
			for (int j = 0; j < 16; j++) {
				fin.read((char*)&ct2[j], 1); //ct2에 암호문파일을 1블록씩 입력
			}
			AES32_EqDecrypt(ct2, rk, pt); //ct2을 복호화하여 pt에 평문(xor연산 필요) 1블록 입력
			for (int j = 0; j < 16; j++) {
				pt[j] = pt[j] ^ ct1[j]; //pt와 직전의 암호문 1블록을 XOR연산 수행
				ct1[j] = ct2[j]; //ct1에 암호문 1블럭을 저장
				fout.write((char*)&pt[j], 1); //padput_file에 패딩이 포함된 pt를 1블록씩 입력
			}
		}
	}

	fin.seekg(0, ios::beg);
	fout.seekp(0, ios::beg);

	erase_padding(padput_file, dest);//erase_padding에 padout_file을 넣어 패딩부분을 지우기
}

int main()
{
	const char *ecb_input_file = "ecb_test_plain.txt";
	const char* ecb_output_file = "ecb_test_cipher.txt";

	byte ECBkey[16] = { 0x28, 0x0a, 0xfe, 0x06, 0x32, 0x16, 0xa1, 0x0b,
						 0x9c, 0xad, 0x9b, 0x20, 0x95, 0x55, 0x2b, 0x16 };

	file_AES_ECB(ecb_input_file, ECBkey, ecb_output_file);

	const char* ecb_output2_file = "ecb_test_decipher.txt";

	Inv_file_AES_ECB(ecb_output_file, ECBkey, ecb_output2_file);
	
	const char *cbc_input_file = "cbc_test_plain.txt";
	const char* cbc_output_file = "cbc_test_cipher.txt";

	byte CBCkey[16] = { 0x89, 0xa5, 0x53, 0x73, 0x04, 0x33, 0xf7, 0xe6,
						0xd6, 0x7d, 0x16, 0xd3, 0x73, 0xbd, 0x53, 0x60 };
	byte IV[16] = { 0xf7, 0x24, 0x55, 0x8d, 0xb3, 0x43, 0x3a, 0x52,
					0x3f, 0x4e, 0x51, 0xa5, 0xbe, 0xa7, 0x04, 0x97 };

	file_AES_CBC(cbc_input_file, IV, CBCkey, cbc_output_file);

	const char* cbc_output2_file = "cbc_test_decipher.txt";

	Inv_file_AES_CBC(cbc_output_file, IV, CBCkey, cbc_output2_file);

	return 0;
}