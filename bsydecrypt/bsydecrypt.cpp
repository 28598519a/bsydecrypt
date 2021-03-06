#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <vector>
#include <io.h>
#include <direct.h>
#include <windows.h>
#define __PAIR__(high, low) (((unsigned long)(high)<<sizeof(high)*8)|low)

using namespace std;

int char2uint(unsigned char* a2, int a3)
{
    return (a2[a3 + 2] << 16) | (a2[a3 + 3] << 24) | a2[a3] | (a2[a3 + 1] << 8);
}

char* decodeXOR(unsigned char* a2, int a3)
{
    int v4 = a3;
    unsigned char* v7 = a2;
    if (a3 <= 19)
        return 0;
    unsigned int v8 = memcmp("gemtrade", a2, 8u);
    if (v8)
        return 0;
    int v9 = char2uint(v7, 8);
    int v10 = char2uint(v7, 12);
    int v11 = char2uint(v7, 16);
    unsigned int v12 = (unsigned int)(v11 + v10) >> 1;
    unsigned int v13 = (unsigned int)(v10 - v11) >> 1;
    size_t v14 = v4 - 20;
    int v15 = v9 - v12 * v13;
    char* v16 = (char *)malloc(v14);
    memcpy(v16, v7 + 20, v14);
    while (1)
    {
        int v17 = char2uint((unsigned char*)v16, v8);
        short v18 = HIWORD(v17) ^ HIWORD(v15);
        v16[v8] = v17 ^ v15;
        int v19 = (int)&v16[v8];
        v8 += 4;
        unsigned int v20 = (v17 ^ (unsigned int)v15) >> 8;
        unsigned int v21 = v14 - v8;
        --v15;
        *(unsigned char*)(v19 + 1) = v20;
        *(unsigned short*)(v19 + 2) = __PAIR__(HIBYTE(v18), (unsigned char)v18);
        if (v14 <= v8 || v21 <= 3) break;
        if (v12 <= v8 && v12 < v21)
        {
            v8 += v13;
            if (v14 < v8 || v12 > v14 - v8) v8 = v14 - v12;
        }
    }
    return v16;
}
/*
//獲取當前目錄
static string curdir()
{
    char exe_full_path[MAX_PATH];
    GetModuleFileNameA(NULL, exe_full_path, MAX_PATH);
    std::string current_path(exe_full_path);
    int pos = current_path.find_last_of('\\', current_path.length());
    return current_path.substr(0, pos);
}
*/
//文件名分解
static array<wstring, 2> splitext(const wstring &file_path)
{
    auto pos = file_path.rfind(L'.');
    array<wstring, 2> text;
    if (wstring::npos != pos)
    {
		text[1] = file_path.substr(pos);
        text[0] = file_path.substr(0, pos);
    }
    else
    {
        text[0] = file_path;
    }
    return text;
}

//列出子目錄下所有文件
static vector<wstring> walk(const wstring &start_path)
{
    _wfinddata_t file_info;
    vector<wstring> file_list;
    wstring find_path = start_path + L"\\*";
	intptr_t handle = _wfindfirst(find_path.c_str(), &file_info);

    if (handle == -1L) return file_list;

    do
    {
        if (file_info.attrib & _A_SUBDIR)
        {
            if ((wcscmp(file_info.name, L".") != 0) && (wcscmp(file_info.name, L"..") != 0))
            {
                wstring new_path = start_path + L"\\" + file_info.name;
                for (auto filename : walk(new_path)) file_list.push_back(filename);
            }
        }
        else
        {
            wstring new_path = start_path + L"\\";
            new_path += file_info.name;
            file_list.push_back(new_path);
        }
    } while (_wfindnext(handle, &file_info) == 0);

    _findclose(handle);

    return file_list;
}

int wmain()
{
	/*
	1. locale("")：調用構造函數創建一個local，其中的空字串具有特殊含義：
	使用客戶環境中缺省的locale。
	例如在簡體中文系統上，會返回簡體中文的locale。
	2. locale::global(locale(""))：將“C++標準IO庫的全域locale”設為“客戶環境中缺省的locale”。
	注意它還會設置C標準庫的locale環境，造成與“setlocale(LC_ALL, "")”類似的效果。
	3. wcout.imbue(locale(""))：使wcout使用“客戶環境中缺省的locale”。
	*/
	locale::global(locale(""));
	wcout.imbue(locale(""));
	wstring filepath, filetype;
	int i = 0;
	wcout << L"******************\n" << L"寶石研物語檔案解密\n" << L"******************\n";
    wcout << L"輸入目錄 :\n";
    wcin >> filepath;
	wcout << L"輸入目標副檔名(例如: png) :\n";
	wcin >> filetype;
    vector<wstring> files;
	vector<wstring> all_files = walk(filepath);
    //vector<string> all_files = walk(curdir());
    for (auto filename : all_files)
    {
        if (splitext(filename)[1] == L"." + filetype) files.push_back(filename);
    }
    for (auto &filename : files)
    {
        ifstream in_file(filename, ios::binary | ios::ate);
        if (!in_file.is_open())
        {
            wcout << L"打開" << filename << L" 失敗！\n";
            return 0;
        }
        // 取得數據
		long data_size = in_file.tellg();
        unsigned char* data = new BYTE[data_size];
        in_file.seekg(0, ios::beg);
        in_file.read((char*)data, data_size);
        in_file.close();

        // 解密數據
        char* v16 = decodeXOR(data, data_size);

        // 寫入數據
        ofstream out_file(splitext(filename)[0] + L"." + filetype, ios::binary);
        if (!out_file.is_open())
        {
			wcout << "創建" << filename << " 失敗！\n";
            continue;
        }
        out_file.write(v16, data_size - 20);
        out_file.close();
		i++;
    }
	wcout << L"所有 " << filetype << L"已解密完成，共" << i << L"個檔案\n";
	system("pause");
    return 0;
}