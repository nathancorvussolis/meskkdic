
#include <Windows.h>
#include <clocale>
#include <string>
#include <vector>
#include <map>
#include <regex>

#define VERSION		L"2.5.0"

LPCWSTR modeR = L"rt";
LPCWSTR modeW = L"wb";
LPCWSTR modeRL = L"rt,ccs=UTF-16LE";
LPCWSTR modeWL = L"wt,ccs=UTF-16LE";

#define BUFSIZE 0x400
#define WBUFSIZE 0x100

LPCSTR EntriesAri = ";; okuri-ari entries.\n";
LPCSTR EntriesNasi = ";; okuri-nasi entries.\n";
LPCWSTR EntriesAriL = L";; okuri-ari entries.\n";
LPCWSTR EntriesNasiL = L";; okuri-nasi entries.\n";

//変換済み検索結果
typedef std::pair< std::string, std::string > CANDIDATEBASE; //候補、注釈
typedef std::pair< CANDIDATEBASE, CANDIDATEBASE > CANDIDATE; //表示用、辞書登録用
typedef std::vector< CANDIDATE > CANDIDATES;

//検索結果
typedef std::pair< std::string, std::string > SKKDICCANDIDATE; //候補、注釈
typedef std::vector< SKKDICCANDIDATE > SKKDICCANDIDATES;

//送りありエントリのブロック
typedef std::pair< std::string, SKKDICCANDIDATES > SKKDICOKURIBLOCK; //送り仮名、候補
typedef std::vector< SKKDICOKURIBLOCK > SKKDICOKURIBLOCKS;
struct OKURIBLOCKS { //avoid C4503
	SKKDICOKURIBLOCKS o;
};
typedef std::pair< std::string, OKURIBLOCKS > USEROKURIENTRY; //見出し語、送りブロック
typedef std::map< std::string, OKURIBLOCKS > USEROKURI;

//見出し語順序
typedef std::vector< std::string > KEYORDER;

//辞書
typedef std::pair< std::string, SKKDICCANDIDATES > SKKDICENTRY; //見出し語、候補
typedef std::map< std::string, SKKDICCANDIDATES > SKKDIC;

#define FORWARD_ITERATION_I(iterator, container) \
	for(auto (iterator) = (container).begin(); (iterator) != (container).end(); ++(iterator))
#define FORWARD_ITERATION(iterator, container) \
	for(auto (iterator) = (container).begin(); (iterator) != (container).end(); )
#define REVERSE_ITERATION_I(reverse_iterator, container) \
	for(auto (reverse_iterator) = (container).rbegin(); (reverse_iterator) != (container).rend(); ++(reverse_iterator))
#define REVERSE_ITERATION(reverse_iterator, container) \
	for(auto (reverse_iterator) = (container).rbegin(); (reverse_iterator) != (container).rend(); )

CONST WCHAR plus = L'+';
CONST WCHAR minus = L'-';

BOOL widechar = FALSE;		// UTF-16LE
BOOL privatedic = FALSE;	// 個人辞書、送りありエントリの角括弧のブロックを保持。

SKKDIC skkdic_a;			//辞書 送りありエントリ
SKKDIC skkdic_n;			//辞書 送りなしエントリ
USEROKURI userokuri;		//送りブロック
KEYORDER complements;		//送りなしエントリ 見出し語順序
KEYORDER accompaniments;	//送りありエントリ 見出し語順序

void usage();
void AddDic(int okuri, const std::string &searchkey, const std::string &candidate, const std::string &annotation);
void DelDic(int okuri, const std::string &searchkey, const std::string &candidate);
void AddKeyOrder(const std::string &searchkey, KEYORDER &keyorder);
void DelKeyOrder(const std::string &searchkey, KEYORDER &keyorder);
void AddOkuriBlock(const std::string &key, const SKKDICCANDIDATES &sc, SKKDICOKURIBLOCKS &so);
BOOL LoadSKKDic(CONST WCHAR op, LPCWSTR path);
std::wstring cesu8_string_to_wstring(const std::string &s);
void WriteSKKDicEntry(FILE *fp, const std::string &key, const SKKDICCANDIDATES &sc, const SKKDICOKURIBLOCKS &so);
BOOL SaveSKKDic(LPCWSTR path);
char *fgets8(char *buffer, int count, FILE *fp);
int ReadSKKDicLine(FILE *fp, int &okuri, std::string &key, SKKDICCANDIDATES &c, SKKDICOKURIBLOCKS &o);
void ParseSKKDicCandiate(const std::string &s, SKKDICCANDIDATES &c);
void ParseSKKDicOkuriBlock(const std::string &s, SKKDICOKURIBLOCKS &o);
std::string ParseConcat(const std::string &s);
std::string MakeConcat(const std::string &s);

int wmain(int argc, wchar_t* argv[])
{
	WCHAR op;
	int oi = 1;

	_wsetlocale(LC_ALL, L"");

	if(argc < 3)
	{
		usage();
		return -1;
	}

	for (int i = 1; i <= 2; i++)
	{
		if ((argv[i][0] == L'-' || argv[i][0] == L'/') && argv[i][1] != L'\0')
		{
			if (towupper(argv[i][1]) == L'W')
			{
				widechar = TRUE;
				++oi;
			}
			else if (towupper(argv[i][1]) == L'O')
			{
				privatedic = TRUE;
				++oi;
			}
		}
	}

	for(int i = oi; i < argc - 1; i += 2)
	{
		if(i != oi)
		{
			if((wcslen(argv[i - 1]) != 1) || ((argv[i - 1][0] != plus) && (argv[i - 1][0] != minus)))
			{
				fwprintf(stderr, L"found illegal string : %s\n", argv[i - 1]);
				return -1;
			}
		}
	}

	for(int i = oi; i < argc - 1; i += 2)
	{
		if(i == oi)
		{
			op = plus;
		}
		else
		{
			op = argv[i - 1][0];
		}

		if(LoadSKKDic(op, argv[i]) == FALSE)
		{
			return -1;
		}
	}

	if(SaveSKKDic(argv[argc - 1]) == FALSE)
	{
		return -1;
	}

	return 0;
}

void usage()
{
	fwprintf(stderr, L"\nmeskkdic " VERSION L"\n"
		L"usage : meskkdic [-W] [-O] <input file 1> [[+-] <input file 2> ...] <output file>\n");
}

void AddDic(int okuri, const std::string &searchkey, const std::string &candidate, const std::string &annotation)
{
	LPCSTR seps = ",";
	std::string annotation_esc, annotation_seps;
	std::regex re;
	SKKDICENTRY userdicentry;
	USEROKURIENTRY userokurientry;
	SKKDICCANDIDATES okurics;

	if(searchkey.empty() || candidate.empty())
	{
		return;
	}

	if(!annotation.empty())
	{
		annotation_seps = seps + ParseConcat(annotation) + seps;
	}

	auto &skkdic = (okuri == 0 ? skkdic_n : skkdic_a);
	auto skkdic_itr = skkdic.find(searchkey);
	if(skkdic_itr == skkdic.end())
	{
		userdicentry.first = searchkey;
		userdicentry.second.push_back(SKKDICCANDIDATE(candidate, annotation_seps));
		skkdic.insert(userdicentry);
	}
	else
	{
		bool exist = false;
		FORWARD_ITERATION_I(sc_itr, skkdic_itr->second)
		{
			if(sc_itr->first == candidate)
			{
				exist = true;
				annotation_esc = ParseConcat(sc_itr->second);
				if(annotation_esc.find(annotation_seps) == std::string::npos)
				{
					if(annotation_esc.empty())
					{
						sc_itr->second.assign(MakeConcat(annotation_seps));
					}
					else
					{
						annotation_esc.append(ParseConcat(annotation) + seps);
						sc_itr->second.assign(MakeConcat(annotation_esc));
					}
				}
				break;
			}
		}
		if(exist == false)
		{
			skkdic_itr->second.push_back(SKKDICCANDIDATE(candidate, MakeConcat(annotation_seps)));
		}
	}

	if(privatedic)
	{
		AddKeyOrder(searchkey, (okuri == 0 ? complements : accompaniments));
	}
}

void DelDic(int okuri, const std::string &searchkey, const std::string &candidate)
{
	auto &skkdic = (okuri == 0 ? skkdic_n : skkdic_a);
	auto skkdic_itr = skkdic.find(searchkey);
	if(skkdic_itr != skkdic.end())
	{
		FORWARD_ITERATION_I(sc_itr, skkdic_itr->second)
		{
			if(sc_itr->first == candidate)
			{
				skkdic_itr->second.erase(sc_itr);
				break;
			}
		}

		if(skkdic_itr->second.empty())
		{
			skkdic.erase(skkdic_itr);

			if(privatedic)
			{
				DelKeyOrder(searchkey, (okuri == 0 ? complements : accompaniments));
			}
		}
	}

	if(privatedic)
	{
		//送りブロック
		auto userokuri_itr = userokuri.find(searchkey);
		if(userokuri_itr != userokuri.end())
		{
			FORWARD_ITERATION(so_itr, userokuri_itr->second.o)
			{
				FORWARD_ITERATION(sc_itr, so_itr->second)
				{
					if(sc_itr->first == candidate)
					{
						sc_itr = so_itr->second.erase(sc_itr);
					}
					else
					{
						++sc_itr;
					}
				}
				if(so_itr->second.empty())
				{
					so_itr = userokuri_itr->second.o.erase(so_itr);
				}
				else
				{
					++so_itr;
				}
			}
			if(userokuri_itr->second.o.empty())
			{
				userokuri.erase(userokuri_itr);
			}
		}
	}
}

void AddKeyOrder(const std::string &searchkey, KEYORDER &keyorder)
{
	keyorder.push_back(searchkey);
}

void DelKeyOrder(const std::string &searchkey, KEYORDER &keyorder)
{
	if(!keyorder.empty())
	{
		FORWARD_ITERATION_I(keyorder_itr, keyorder)
		{
			if(searchkey == *keyorder_itr)
			{
				keyorder.erase(keyorder_itr);
				break;
			}
		}
	}
}

void AddOkuriBlock(const std::string &key, const SKKDICCANDIDATES &sc, SKKDICOKURIBLOCKS &so)
{
	USEROKURIENTRY userokurientry;

	//送り仮名が重複したら1つにまとめる
	FORWARD_ITERATION_I(so_itr, so)
	{
		for(auto so_itr1 = so_itr + 1; so_itr1 != so.end();)
		{
			if(so_itr->first == so_itr1->first)
			{
				FORWARD_ITERATION_I(sc_itr1, so_itr1->second)
				{
					bool exist = false;
					FORWARD_ITERATION_I(sc_itr, so_itr->second)
					{
						if(sc_itr->first == sc_itr1->first)
						{
							exist = true;
							break;
						}
					}
					if(!exist)
					{
						so_itr->second.push_back(*sc_itr1);
					}
				}
				so_itr1 = so.erase(so_itr1);
			}
			else
			{
				++so_itr1;
			}
		}
	}

	//候補にない送りブロックの候補を除外
	FORWARD_ITERATION(so_itr, so)
	{
		FORWARD_ITERATION(soc_itr, so_itr->second)
		{
			bool exist = false;
			FORWARD_ITERATION_I(sc_itr, sc)
			{
				if(soc_itr->first == sc_itr->first)
				{
					exist = true;
					break;
				}
			}
			if(!exist)
			{
				soc_itr = so_itr->second.erase(soc_itr);
			}
			else
			{
				++soc_itr;
			}
		}
		if(so_itr->second.empty())
		{
			so_itr = so.erase(so_itr);
		}
		else
		{
			++so_itr;
		}
	}

	//送りブロックに追加
	if(!so.empty())
	{
		auto userokuri_itr = userokuri.find(key);
		if(userokuri_itr == userokuri.end())
		{
			userokurientry.first = key;
			userokurientry.second.o = so;
			userokuri.insert(userokurientry);
		}
		else
		{
			FORWARD_ITERATION_I(so_itr, so)
			{
				bool exist_o = false;
				FORWARD_ITERATION_I(o_itr, userokuri_itr->second.o)
				{
					if(o_itr->first == so_itr->first)
					{
						exist_o = true;
						FORWARD_ITERATION_I(c_itr, so_itr->second)
						{
							bool exist_c = false;
							FORWARD_ITERATION_I(oc_itr, o_itr->second)
							{
								if(oc_itr->first == c_itr->first)
								{
									exist_c = true;
									break;
								}
							}
							if(!exist_c)
							{
								o_itr->second.push_back(*c_itr);
							}
						}
						break;
					}
				}
				if(!exist_o)
				{
					userokuri_itr->second.o.push_back(*so_itr);
				}
			}
		}
	}
}

BOOL LoadSKKDic(CONST WCHAR op, LPCWSTR path)
{
	FILE *fp;
	std::string key, empty;
	int okuri = 0; // default okuri-nasi
	int rl;
	SKKDICCANDIDATES sc;
	SKKDICOKURIBLOCKS so;

	_wfopen_s(&fp, path, (widechar ? modeRL : modeR));
	if(fp == nullptr)
	{
		fwprintf(stderr, L"cannot open file : %s\n", path);
		return FALSE;
	}

	if (widechar == FALSE)
	{
		// skip BOM
		UCHAR bom[3] = { 0,0,0 };
		fread(&bom, sizeof(bom), 1, fp);
		if (bom[0] != 0xEF || bom[1] != 0xBB || bom[2] != 0xBF)
		{
			fseek(fp, 0, SEEK_SET);
		}
	}

	while(true)
	{
		rl = ReadSKKDicLine(fp, okuri, key, sc, so);
		if(rl == -1)
		{
			break;
		}
		else if(rl == 1)
		{
			continue;
		}

		if(sc.empty())
		{
			continue;
		}

		switch(op)
		{
		case plus:
			FORWARD_ITERATION_I(sc_itr, sc)
			{
				AddDic(okuri, key, sc_itr->first, sc_itr->second);
			}

			if(privatedic && okuri == 1)
			{
				AddOkuriBlock(key, sc, so);
			}
			break;

		case minus:
			FORWARD_ITERATION_I(sc_itr, sc)
			{
				DelDic(okuri, key, sc_itr->first);
			}
			break;

		default:
			break;
		}
	}

	fclose(fp);

	return TRUE;
}

std::wstring cesu8_string_to_wstring(const std::string &s)
{
	std::wstring ws;
	size_t length = s.length();

	for (size_t i = 0; i < length; i++)
	{
		UCHAR u0 = (UCHAR)s[i];

		// CESU-8
		if (u0 <= 0x7F)
		{
			WCHAR w = (WCHAR)(u0 & 0x7F);
			ws.push_back(w);
		}
		else if (u0 >= 0xC2 && u0 <= 0xDF)
		{
			if (i + 1 >= length) break;

			UCHAR u1 = (UCHAR)s[i + 1];

			WCHAR w =
				((WCHAR)(u0 & 0x1F) << 6) |
				((WCHAR)(u1 & 0x3F));
			ws.push_back(w);
			i += 1;
		}
		else if (u0 >= 0xE0 && u0 <= 0xEF)
		{
			if (i + 2 >= length) break;

			UCHAR u1 = (UCHAR)s[i + 1];
			UCHAR u2 = (UCHAR)s[i + 2];

			WCHAR w =
				((WCHAR)(u0 & 0x0F) << 12) |
				((WCHAR)(u1 & 0x3F) << 6) |
				((WCHAR)(u2 & 0x3F));
			ws.push_back(w);
			i += 2;
		}
	}

	return ws;
}

void WriteSKKDicEntry(FILE *fp, const std::string &key, const SKKDICCANDIDATES &sc, const SKKDICOKURIBLOCKS &so)
{
	std::string line, annotation_esc;

	line = key + " /";
	FORWARD_ITERATION_I(sc_itr, sc)
	{
		line += sc_itr->first;
		if(sc_itr->second.size() > 2)
		{
			annotation_esc = ParseConcat(sc_itr->second);
			line += ";" + MakeConcat(annotation_esc.substr(1, annotation_esc.size() - 2));
		}
		line += "/";
	}

	if(privatedic)
	{
		FORWARD_ITERATION_I(so_itr, so)
		{
			line += "[" + so_itr->first + "/";
			FORWARD_ITERATION_I(sc_itr, so_itr->second)
			{
				line += sc_itr->first + "/";
			}
			line += "]/";
		}

	}

	if (widechar)
	{
		fwprintf(fp, L"%s\n", cesu8_string_to_wstring(line).c_str());
	}
	else
	{
		fprintf(fp, "%s\n", line.c_str());
	}
}

BOOL SaveSKKDic(LPCWSTR path)
{
	FILE *fp;
	SKKDICOKURIBLOCKS so;

	_wfopen_s(&fp, path, (widechar ? modeWL : modeW));
	if(fp == nullptr)
	{
		fwprintf(stderr, L"cannot open file : %s\n", path);
		return FALSE;
	}

	//送りありエントリ
	if (widechar)
	{
		fwprintf(fp, L"%s", EntriesAriL);
	}
	else
	{
		fprintf(fp, "%s", EntriesAri);
	}

	if(privatedic)
	{
		FORWARD_ITERATION_I(keyorder_itr, accompaniments)
		{
			auto skkdic_itr = skkdic_a.find(*keyorder_itr);
			if(skkdic_itr != skkdic_a.end())
			{
				so.clear();
				auto userokuri_itr = userokuri.find(*keyorder_itr);
				if(userokuri_itr != userokuri.end())
				{
					so = userokuri_itr->second.o;
				}
				WriteSKKDicEntry(fp, skkdic_itr->first, skkdic_itr->second, so);

				skkdic_a.erase(skkdic_itr);
			}
		}
	}
	else
	{
		REVERSE_ITERATION_I(skkdic_ritr, skkdic_a)
		{
			WriteSKKDicEntry(fp, skkdic_ritr->first, skkdic_ritr->second, so);
		}
	}

	so.clear();

	//送りなしエントリ
	if (widechar)
	{
		fwprintf(fp, L"%s", EntriesNasiL);
	}
	else
	{
		fprintf(fp, "%s", EntriesNasi);
	}

	if(privatedic)
	{
		FORWARD_ITERATION_I(keyorder_itr, complements)
		{
			auto skkdic_itr = skkdic_n.find(*keyorder_itr);
			if(skkdic_itr != skkdic_n.end())
			{
				WriteSKKDicEntry(fp, skkdic_itr->first, skkdic_itr->second, so);

				skkdic_n.erase(skkdic_itr);
			}
		}
	}
	else
	{
		FORWARD_ITERATION_I(skkdic_itr, skkdic_n)
		{
			WriteSKKDicEntry(fp, skkdic_itr->first, skkdic_itr->second, so);
		}
	}

	fclose(fp);

	return TRUE;
}

char *fgets8(char *buffer, int count, FILE *fp)
{
	if (widechar)
	{
		WCHAR wbuf[WBUFSIZE] = {};
		char *ret = nullptr;
		int n = 0;
		if (fgetws(wbuf, WBUFSIZE, fp) != nullptr)
		{
			for (int i = 0; i < WBUFSIZE; i++)
			{
				// CESU-8 for sorting in UTF-16
				if (wbuf[i] <= L'\u007F')
				{
					if (n + 1 >= count) break;

					buffer[n++] = (UCHAR)wbuf[i] & 0x7F;

					if (wbuf[i] == L'\0')
					{
						ret = buffer;
						break;
					}
				}
				else if (wbuf[i] <= L'\u07FF')
				{
					if (n + 2 >= count) break;

					buffer[n++] = (UCHAR)0xC0 | (UCHAR)((wbuf[i] >> 6) & 0x1F);
					buffer[n++] = (UCHAR)0x80 | (UCHAR)(wbuf[i] & 0x3F);
				}
				else
				{
					if (n + 3 >= count) break;

					buffer[n++] = (UCHAR)0xE0 | (UCHAR)((wbuf[i] >> 12) & 0x0F);
					buffer[n++] = (UCHAR)0x80 | (UCHAR)((wbuf[i] >> 6) & 0x3F);
					buffer[n++] = (UCHAR)0x80 | (UCHAR)(wbuf[i] & 0x3F);
				}
			}

			return ret;
		}
	}
	else
	{
		return fgets(buffer, count, fp);
	}

	return nullptr;
}

int ReadSKKDicLine(FILE *fp, int &okuri, std::string &key, SKKDICCANDIDATES &c, SKKDICOKURIBLOCKS &o)
{
	CHAR buf[BUFSIZE] = {};
	std::string sbuf;

	c.clear();
	o.clear();

	while(fgets8(buf, _countof(buf), fp) != nullptr)
	{
		sbuf += buf;

		if(!sbuf.empty() && sbuf.back() == '\n')
		{
			break;
		}
	}

	if (ferror(fp) != 0)
	{
		return -1;
	}

	if(sbuf.empty())
	{
		return -1;
	}

	if(sbuf.compare(EntriesAri) == 0)
	{
		okuri = 1;
		return 1;
	}
	else if(sbuf.compare(EntriesNasi) == 0)
	{
		okuri = 0;
		return 1;
	}

	if(okuri == -1)
	{
		return 1;
	}

	std::string s = sbuf;

	static const std::string  fmt("");

	static const std::regex rectrl("[\\x00-\\x19]");
	s = std::regex_replace(s, rectrl, fmt);

	if(okuri == 1)
	{
		if(privatedic)
		{
			ParseSKKDicOkuriBlock(s, o);
		}

		//送りブロックを除去
		static const std::regex reblock("\\[[^\\[\\]]+?/[^\\[\\]]+?/\\]/");
		s = std::regex_replace(s, reblock, fmt);
	}

	size_t is = s.find("\x20/");
	if(is == std::string::npos)
	{
		return 1;
	}

	size_t ie = s.find_last_not_of('\x20', is);
	if(ie == std::string::npos)
	{
		return 1;
	}

	if(s.find_last_of('\x20', ie) != std::string::npos)
	{
		return 1;
	}

	key = s.substr(0, ie + 1);

	s = s.substr(is + 1);

	ParseSKKDicCandiate(s, c);

	return 0;
}

void ParseSKKDicCandiate(const std::string &s, SKKDICCANDIDATES &c)
{
	size_t i, is, ie, ia;
	std::string candidate, annotation;

	i = 0;
	while(i < s.size())
	{
		is = s.find_first_of('/', i);
		ie = s.find_first_of('/', is + 1);
		if(ie == std::string::npos)
		{
			break;
		}

		candidate = s.substr(i + 1, ie - is - 1);
		i = ie;

		ia = candidate.find_first_of(';');

		if(ia == std::string::npos)
		{
			annotation.clear();
		}
		else
		{
			annotation = candidate.substr(ia + 1);
			candidate = candidate.substr(0, ia);
		}

		if(!candidate.empty())
		{
			c.push_back(SKKDICCANDIDATE(candidate, annotation));
		}
	}
}

void ParseSKKDicOkuriBlock(const std::string &s, SKKDICOKURIBLOCKS &o)
{
	std::string so, okurik, okuric, fmt;
	std::smatch m;
	SKKDICCANDIDATES okurics;

	so = s;

	static const std::regex reblock("\\[([^\\[\\]]+?)(/[^\\[\\]]+?/)\\]/");

	while(std::regex_search(so, m, reblock))
	{
		okurics.clear();

		fmt.assign("$1");
		okurik = std::regex_replace(m.str(), reblock, fmt);
		fmt.assign("$2");
		okuric = std::regex_replace(m.str(), reblock, fmt);

		ParseSKKDicCandiate(okuric, okurics);

		std::reverse(okurics.begin(), okurics.end());

		o.insert(o.begin(), std::make_pair(okurik, okurics));

		so = m.suffix().str();
	}
}

std::string ParseConcat(const std::string &s)
{
	std::string ret, fmt, numstr, numtmpstr;
	std::regex re;
	std::smatch res;
	ULONG u;
	LPCSTR bsrep = "\xff";
	ret = s;

	static const std::regex reconcat("^\\(\\s*concat\\s+\"(.+)\"\\s*\\)$");

	if(std::regex_search(ret, reconcat))
	{
		fmt.assign("$1");
		ret = std::regex_replace(ret, reconcat, fmt);

		re.assign("\"\\s+\"");
		fmt.assign("");
		ret = std::regex_replace(ret, re, fmt);

		//バックスラッシュ
		re.assign("\\\\\\\\");
		fmt.assign(bsrep);
		ret = std::regex_replace(ret, re, fmt);

		//二重引用符
		re.assign("\\\\\\\"");
		fmt.assign("\\\"");
		ret = std::regex_replace(ret, re, fmt);

		//空白文字
		re.assign("\\\\s");
		fmt.assign("\x20");
		ret = std::regex_replace(ret, re, fmt);

		//制御文字など
		re.assign("\\\\[abtnvfred ]");
		fmt.assign("");
		ret = std::regex_replace(ret, re, fmt);

		//8進数表記の文字
		re.assign("\\\\[0-3][0-7]{2}");
		while(std::regex_search(ret, res, re))
		{
			numstr += res.prefix();
			numtmpstr = res.str();
			numtmpstr[0] = L'0';
			u = strtoul(numtmpstr.c_str(), nullptr, 0);
			if(u >= '\x20' && u <= '\x7E')
			{
				numstr.append(1, (CHAR)u);
			}
			ret = res.suffix().str();
		}
		ret = numstr + ret;

		//意味なしエスケープ
		re.assign("\\\\");
		fmt.assign("");
		ret = std::regex_replace(ret, re, fmt);

		//バックスラッシュ
		re.assign(bsrep);
		fmt.assign("\\");
		ret = std::regex_replace(ret, re, fmt);
	}

	return ret;
}

std::string MakeConcat(const std::string &s)
{
	std::string ret, fmt;
	std::regex re;

	ret = s;

	// "/" -> \057, ";" -> \073
	static const std::regex respcch("[/;]");

	if(std::regex_search(ret, respcch))
	{
		// "\"" -> "\\\"", "\\" -> "\\\\"
		re.assign("([\\\"\\\\])");
		fmt.assign("\\$1");
		ret = std::regex_replace(ret, re, fmt);

		re.assign("/");
		fmt.assign("\\057");
		ret = std::regex_replace(ret, re, fmt);

		re.assign(";");
		fmt.assign("\\073");
		ret = std::regex_replace(ret, re, fmt);

		ret = "(concat \"" + ret + "\")";
	}

	return ret;
}
