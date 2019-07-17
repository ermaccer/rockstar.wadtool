// rockstar_wadtool.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>
#include <memory>
#include <filesystem>

#include "stdafx.h"
#include "zlib\zlib.h"
#include "wad.h"
#include "filef.h"

#pragma comment (lib, "zlib/zdll.lib" )

enum eMode {
	MODE_EXTRACT = 1,MODE_CREATE,
	PARAM_SCAN_NAME,
	GAME_TW_PSP, GAME_TW_PS2, GAME_TW_PS2_PAK, GAME_MP_PS2
};

int main(int argc, char* argv[])
{
	if (argc < 3) {
		std::cout << "Rockstar Wadtool - work with R* wad formats\n"
			<< "Usage: rwadtool <params> <file/folder>\n"
			<< "    -c              Creates archive from a folder for supported games\n"
			<< "    -e              Extracts archive\n"
			<< "    -o              Specifies a folder for extraction/output file\n"
			<< "    -t              Specifies table file/ output too\n"
			<< "    -s              Name pak textures after texture name\n"
			<< "    -g  <game>      Specifies game:\n"
			<< "                      tw_ps2     - The Warriors PS2\n"
			<< "                      tw_ps2_pak - The Warriors PS2 Pak (experimental)\n"
			<< "                      tw_psp     - The Warriors PSP (Creation supported)\n"
			<< "                      mp_ps2     - Max Payne PS2 (MEGAIMAG.WAD)\n"
			<< "Examples: \n"
			<< "extract archive (psp) - rwadtool -e -g tw_psp -t zwarriors.dir zwarriors.wad\n"
			<< "create archive (psp) - rwadtool -c -g tw_psp -t newdir.dir -o newwad.wad folder\n";
		return 1;
	}

	int mode = 0;
	int param = 0;
	int game = 0;
	int extra = 0;
	std::string o_param;
	std::string g_param;
	std::string t_param;

	// params
	for (int i = 1; i < argc - 1; i++)
	{
		if (argv[i][0] != '-' || strlen(argv[i]) != 2) {
			return 1;
		}
		switch (argv[i][1])
		{
		case 'e': mode = MODE_EXTRACT;
			break;
		case 'c': mode = MODE_CREATE;
			break;
		case 's': param = PARAM_SCAN_NAME;
			break;
		case 'o':
			i++;
			o_param = argv[i];
			break;
		case 'g':
			i++;
			g_param = argv[i];
			break;
		case 't':
			i++;
			t_param = argv[i];
			break;
		default:
			std::cout << "ERROR: Param does not exist: " << argv[i] << std::endl;
			break;
		}
	}
	// check game

	if (g_param == "tw_psp") game = GAME_TW_PSP;
	if (g_param == "tw_ps2") game = GAME_TW_PS2;
	if (g_param == "mp_ps2") game = GAME_MP_PS2;
	if (g_param == "tw_ps2_pak") game = GAME_TW_PS2_PAK;

	if (mode == MODE_EXTRACT)
	{
		if (game == GAME_TW_PSP)
		{
			if (t_param.empty())
			{
				std::cout << "ERROR: Table file was not specified!" << std::endl;
				return 1;
			}
		
			std::ifstream pTable(t_param, std::ifstream::binary);

			if (!pTable)
			{
				std::cout << "ERROR: Could not open: " << t_param << "!" << std::endl;
				return 1;
			}
			if (pTable)
			{
				std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

				if (!pFile)
				{
					std::cout << "ERROR: Could not open: " << argv[argc - 1] << "!" << std::endl;
					return 1;
				}

				header_wad_main_tw wad;

				// can't do any checks here 
				pTable.read((char*)&wad, sizeof(header_wad_main_tw));

				std::unique_ptr<wad_main_tw_psp[]> wad_entry = std::make_unique<wad_main_tw_psp[]>(wad.files);

				for (int i = 0; i < wad.files; i++)
					pTable.read((char*)&wad_entry[i], sizeof(wad_main_tw_psp));


				std::ofstream oDump("!psp_wad.bin", std::ofstream::binary);
				for (int i = 0; i < wad.files; i++)
				{
					oDump.write((char*)&wad_entry[i].pad, sizeof(int));
				}

				// get files

				if (!o_param.empty())
				{
					std::experimental::filesystem::create_directory(o_param);
					std::experimental::filesystem::current_path(
						std::experimental::filesystem::system_complete(std::experimental::filesystem::path(o_param)));
				}



				for (int i = 0; i < wad.files; i++)
				{
					pFile.seekg(wad_entry[i].offset, pFile.beg);

					int dataSize = wad_entry[i].size;
					std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);
					pFile.read(dataBuff.get(), dataSize);

					std::unique_ptr<char[]> rawBuff = std::make_unique<char[]>(wad_entry[i].rawSize);
					unsigned long uncompressedSize = wad_entry[i].rawSize;

					int zlib_output = uncompress((Bytef*)rawBuff.get(), 
						&uncompressedSize,
						(Bytef*)dataBuff.get(),
						wad_entry[i].size);


					if (zlib_output == Z_MEM_ERROR)
					{
						std::cout << "ERROR: ZLIB: Out of memory!" << std::endl;
						return 1;
					}


					std::string output = std::to_string(i) + ".dat";

					std::cout << "Processing: " << output << std::endl;
					std::ofstream oFile(output, std::ofstream::binary);
					oFile.write(rawBuff.get(), wad_entry[i].rawSize);


				}

			}
		}
		if (game == GAME_TW_PS2)
		{
			if (t_param.empty())
			{
				std::cout << "ERROR: Table file was not specified!" << std::endl;
				return 1;
			}

			std::ifstream pTable(t_param, std::ifstream::binary);

			if (!pTable)
			{
				std::cout << "ERROR: Could not open: " << t_param << "!" << std::endl;
				return 1;
			}
			if (pTable)
			{
				std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

				if (!pFile)
				{
					std::cout << "ERROR: Could not open: " << argv[argc - 1] << "!" << std::endl;
					return 1;
				}

				header_wad_main_tw wad;

				// can't do any checks here 
				pTable.read((char*)&wad, sizeof(header_wad_main_tw));

				std::unique_ptr<wad_main_tw_ps2[]> wad_entry = std::make_unique<wad_main_tw_ps2[]>(wad.files);

				for (int i = 0; i < wad.files; i++)
					pTable.read((char*)&wad_entry[i], sizeof(wad_main_tw_ps2));


				// get files

				if (!o_param.empty())
				{
					std::experimental::filesystem::create_directory(o_param);
					std::experimental::filesystem::current_path(
						std::experimental::filesystem::system_complete(std::experimental::filesystem::path(o_param)));
				}



				for (int i = 0; i < wad.files; i++)
				{
					pFile.seekg(wad_entry[i].offset, pFile.beg);

					int dataSize = wad_entry[i].size;
					std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);
					pFile.read(dataBuff.get(), dataSize);

					std::string output = std::to_string(i) + ".dat";

					std::cout << "Processing: " << output << std::endl;
					std::ofstream oFile(output, std::ofstream::binary);
					oFile.write(dataBuff.get(), wad_entry[i].size);


				}

			}
		}
		if (game == GAME_TW_PS2_PAK)
		{
			std::cout << "INFO: Experimental" << std::endl;
			std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

			if (!pFile)
			{
				std::cout << "ERROR: Could not open: " << argv[argc - 1] << "!" << std::endl;
				return 1;
			}

			if (pFile)
			{
				int files;
				pFile.read((char*)&files, sizeof(int));

				std::unique_ptr<int[]> size = std::make_unique<int[]>(files);

				for (int i = 0; i < files; i++)
				{
	
					int temp;
					pFile.read((char*)&temp, sizeof(int));
					size[i] = temp;
					pFile.seekg(sizeof(int) * 2, pFile.cur);
					if (i != 0)
						pFile.seekg(sizeof(int), pFile.cur);

				}

				if (files == 1)
					pFile.seekg(sizeof(int) * 4, pFile.cur);

				if (!o_param.empty())
				{
					std::experimental::filesystem::create_directory(o_param);
					std::experimental::filesystem::current_path(
						std::experimental::filesystem::system_complete(std::experimental::filesystem::path(o_param)));
				}

				for (int i = 0; i < files; i++)
				{
					int dataSize = size[i];
					std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);
					pFile.read(dataBuff.get(), dataSize);

					std::string extension = ".dat";
					std::string filename = std::to_string(i);
					if (dataBuff[0] == 0x16 && dataBuff[1] == 0 && dataBuff[2] == 0 && dataBuff[3] == 0)
					{
						extension = ".txd";
						if (param == PARAM_SCAN_NAME)
						{
							int i = 0;
							std::unique_ptr<char[]> strBuff = std::make_unique<char[]>(_MAX_PATH);
							do
							{
								strBuff[i] = dataBuff[i + 0x48];
								i++;
							} while (dataBuff[i + 0x48] != 0);
							std::string result(strBuff.get(), i);
							filename = result;
						}

					}
						
					std::string output = filename + extension;
					
					std::cout << "Processing: " << output << std::endl;
					std::ofstream oFile(output, std::ofstream::binary);
					oFile.write(dataBuff.get(), dataSize);
				}
				
			}
		}
		if (game == GAME_MP_PS2)
		{
			std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

			if (!pFile)
			{
				std::cout << "ERROR: Could not open: " << argv[argc - 1] << "!" << std::endl;
				return 1;
			}
			if (pFile)
			{
				// can't do any checks either
				int files;
				pFile.read((char*)&files, sizeof(int));

				std::unique_ptr<wad_mega_mp_ps2[]> wad_entry = std::make_unique<wad_mega_mp_ps2[]>(files);

				for (int i = 0; i < files; i++)
				{
					pFile.read((char*)&wad_entry[i], sizeof(wad_mega_mp_ps2));
				}

				if (!o_param.empty())
				{
					std::experimental::filesystem::create_directory(o_param);
					std::experimental::filesystem::current_path(
						std::experimental::filesystem::system_complete(std::experimental::filesystem::path(o_param)));
				}

				for (int i = 0; i < files; i++)
				{
					std::string output = wad_entry[i].name;
					int dataSize = wad_entry[i].size;

					std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);
					pFile.read(dataBuff.get(), dataSize);
				
					std::cout << "Processing: " << wad_entry[i].name << std::endl;

					std::ofstream oFile(output, std::ofstream::binary);
					oFile.write(dataBuff.get(), dataSize);
				}
			}
		}
	}
	if (mode == MODE_CREATE)
	{

		std::experimental::filesystem::path folder(argv[argc - 1]);
		if (!std::experimental::filesystem::exists(folder))
		{
			std::cout << "ERROR: Could not open directory: " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}

		if (std::experimental::filesystem::exists(folder))
		{


			int filesFound = 0;
			int foldersFound = 0;
			// get files number
			for (const auto & file : std::experimental::filesystem::recursive_directory_iterator(folder))
			{
				filesFound++;
				if (std::experimental::filesystem::is_directory(file)) foldersFound++;

			}
			filesFound -= foldersFound;

			if (game == GAME_TW_PSP)
			{
				if (t_param.empty())
				{
					std::cout << "ERROR: Output table file was not specified!" << std::endl;
					return 1;
				}

				if (o_param.empty())
				{
					std::cout << "ERROR: Output file was not specified!" << std::endl;
					return 1;
				}
				// no filenames, just grab 1.dat,2.dat etc

				std::ofstream oDir(t_param, std::ofstream::binary);
				header_wad_main_tw wad;
				wad.files = filesFound;
				oDir.write((char*)&wad, sizeof(header_wad_main_tw));


				std::unique_ptr<unsigned int[]> pad = std::make_unique<unsigned int[]>(wad.files);
				std::unique_ptr<int[]> size = std::make_unique<int[]>(wad.files);
				std::unique_ptr<int[]> rawSize = std::make_unique<int[]>(wad.files);





				// grab dump

				std::ifstream pDir("!psp_wad.bin", std::ifstream::binary);

				if (!pDir) {
					std::cout << "ERROR: Could not open !psp_wad.bin!" << std::endl;
					return 1;
				}

				for (int i = 0; i < wad.files; i++)
				{
					pDir.read((char*)&pad[i], sizeof(int));
				}

				std::ofstream oArchive(o_param, std::ofstream::binary);

				for (int i = 0; i < wad.files; i++)
				{
					std::string input = argv[argc - 1];
					input += "\\" + std::to_string(i) + ".dat";

					std::ifstream pTemp(input, std::ifstream::binary);

					if (!pTemp)
					{
						std::cout << "\nERROR: Could not open " << input << "!" << std::endl;
						return 1;
					}

					std::cout << "\rProcessing: " << i+ 1 << "/" << wad.files << std::flush;

					int dataSize = (int)getSizeToEnd(pTemp);
					rawSize[i] = dataSize;
					std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);
					pTemp.read(dataBuff.get(), dataSize);

					unsigned long cmpSize = dataSize * 1.2 + 12;
					std::unique_ptr<char[]> cmpBuff = std::make_unique<char[]>(cmpSize);

					int zlib_output = compress((Bytef*)cmpBuff.get(), &cmpSize, (Bytef*)dataBuff.get(), dataSize);

					if (zlib_output == Z_MEM_ERROR) {
						std::cout << "ERROR: ZLIB: Out of memory!" << std::endl;
						return 1;
					}

					size[i] = cmpSize;
					oArchive.write(cmpBuff.get(), cmpSize);

					// write pad
					std::unique_ptr<char[]> padBuff = std::make_unique<char[]>(calcOffsetFromPad(size[i], PSP_PAD_VALUE) - size[i]);
					oArchive.write(padBuff.get(), calcOffsetFromPad(size[i], PSP_PAD_VALUE) - size[i]);
					
				}

				int baseOffset = 0;
				for (int i = 0; i < wad.files; i++)
				{
					wad_main_tw_psp ent;
					ent.size = size[i];
					ent.pad = pad[i];
					ent.rawSize = rawSize[i];
					ent.offset = baseOffset;
					oDir.write((char*)&ent, sizeof(wad_main_tw_psp));
					baseOffset += calcOffsetFromPad(size[i], PSP_PAD_VALUE);
				}

			}
		}
	}

    return 0;
}

