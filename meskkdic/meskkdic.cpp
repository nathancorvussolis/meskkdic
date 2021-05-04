
#include <tchar.h>

#include <string>
#include <vector>
#include <map>
#include <regex>

#include <Windows.h>

#define VERSION		L"2.5.0"

#ifdef _UNICODE
#define BUFSIZE 0x100
LPCWSTR modeR = L"rt,ccs=UTF-16LE";
LPCWSTR modeW = L"wt,ccs=UTF-16LE";
LPCWSTR EntriesAri = L";; okuri-ari entries.\n";
LPCWSTR EntriesNasi = L";; okuri-nasi entries.\n";
#define XSTRING std::wstring
#define XREGEX std::wregex
#define XSMATCH std::wsmatch
#else
#define BUFSIZE 0x200
LPCWSTR modeR = L"rt";
LPCWSTR modeW = L"wb";
LPCSTR EntriesAri = ";; okuri-ari entries.\n";
LPCSTR EntriesNasi = ";; okuri-nasi entries.\n";
#define XSTRING std::string
#define XREGEX std::regex
#define XSMATCH std::smatch
#endif

//変換済み検索結果
typedef std::pair< XSTRING, XSTRING > CANDIDATEBASE; //候補、注釈
typedef std::pair< CANDIDATEBASE, CANDIDATEBASE > CANDIDATE; //表示用、辞書登録用
typedef std::vector< CANDIDATE > CANDIDATES;

//検索結果
typedef std::pair< XSTRING, XSTRING > SKKDICCANDIDATE; //候補、注釈
typedef std::vector< SKKDICCANDIDATE > SKKDICCANDIDATES;

//送りありエントリのブロック
typedef std::pair< XSTRING, SKKDICCANDIDATES > SKKDICOKURIBLOCK; //送り仮名、候補
typedef std::vector< SKKDICOKURIBLOCK > SKKDICOKURIBLOCKS;
struct OKURIBLOCKS { //avoid C4503
	SKKDICOKURIBLOCKS o;
};
typedef std::pair< XSTRING, OKURIBLOCKS > USEROKURIENTRY; //見出し語、送りブロック
typedef std::map< XSTRING, OKURIBLOCKS > USEROKURI;

//見出し語順序
typedef std::vector< XSTRING > KEYORDER;

//辞書
typedef std::pair< XSTRING, SKKDICCANDIDATES > SKKDICENTRY; //見出し語、候補
typedef std::map< XSTRING, SKKDICCANDIDATES > SKKDIC;

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

BOOL privatedic = FALSE;

SKKDIC skkdic_a;			//辞書 送りありエントリ
SKKDIC skkdic_n;			//辞書 送りなしエントリ
USEROKURI userokuri;		//送りブロック
KEYORDER complements;		//送りなしエントリ 見出し語順序
KEYORDER accompaniments;	//送りありエントリ 見出し語順序

void usage();
void AddDic(int okuri, const XSTRING &searchkey, const XSTRING &candidate, const XSTRING &annotation);
void DelDic(int okuri, const XSTRING &searchkey, const XSTRING &candidate);
void AddKeyOrder(const XSTRING &searchkey, KEYORDER &keyorder);
void DelKeyOrder(const XSTRING &searchkey, KEYORDER &keyorder);
void AddOkuriBlock(const std::wstring &key, const SKKDICCANDIDATES &sc, SKKDICOKURIBLOCKS &so);
BOOL LoadSKKDic(CONST WCHAR op, LPCWSTR path);
void WriteSKKDicEntry(FILE *fp, const XSTRING &key, const SKKDICCANDIDATES &sc, const SKKDICOKURIBLOCKS &so);
BOOL SaveSKKDic(LPCWSTR path);
int ReadSKKDicLine(FILE *fp, int &okuri, XSTRING &key, SKKDICCANDIDATES &c, SKKDICOKURIBLOCKS &o);
void ParseSKKDicCandiate(const XSTRING &s, SKKDICCANDIDATES &c);
void ParseSKKDicOkuriBlock(const XSTRING &s, SKKDICOKURIBLOCKS &o);
XSTRING ParseConcat(const XSTRING &s);
XSTRING MakeConcat(const XSTRING &s);

int wmain(int argc, wchar_t* argv[])
{
	WCHAR op;
	int i, oi;
	SKKDIC entries_a, entries_n;

	_tsetlocale(LC_ALL, _T(""));

	if(argc < 3)
	{
		usage();
		return -1;
	}

	if(wcscmp(argv[1], L"-O") == 0)
	{
		privatedic = TRUE;
		oi = 2;
	}
	else
	{
		oi = 1;
	}

	for(i = oi; i < argc - 1; i += 2)
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

	for(i = oi; i < argc - 1; i += 2)
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
		L"usage : meskkdic [-O] <input file 1> [[+-] <input file 2> ...] <output file>\n");
}

void AddDic(int okuri, const XSTRING &searchkey, const XSTRING &candidate, const XSTRING &annotation)
{
	LPCTSTR seps = _T(",");
	XSTRING annotation_esc, annotation_seps;
	XREGEX re;
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
				if(annotation_esc.find(annotation_seps) == std::wstring::npos)
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

void DelDic(int okuri, const XSTRING &searchkey, const XSTRING &candidate)
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

void AddKeyOrder(const XSTRING &searchkey, KEYORDER &keyorder)
{
	keyorder.push_back(searchkey);
}

void DelKeyOrder(const XSTRING &searchkey, KEYORDER &keyorder)
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

void AddOkuriBlock(const XSTRING &key, const SKKDICCANDIDATES &sc, SKKDICOKURIBLOCKS &so)
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
	XSTRING key, empty;
	int okuri = 0; // default okuri-nasi
	int rl;
	SKKDICCANDIDATES sc;
	SKKDICOKURIBLOCKS so;

	_wfopen_s(&fp, path, modeR);
	if(fp == nullptr)
	{
		fwprintf(stderr, L"cannot open file : %s\n", path);
		return FALSE;
	}

#ifndef _UNICODE
	// skip BOM
	UCHAR bom[3] = {0,0,0};
	fread(&bom, sizeof(bom), 1, fp);
	if(bom[0] != 0xEF || bom[1] != 0xBB || bom[2] != 0xBF)
	{
		fseek(fp, 0, SEEK_SET);
	}
#endif

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

void WriteSKKDicEntry(FILE *fp, const XSTRING &key, const SKKDICCANDIDATES &sc, const SKKDICOKURIBLOCKS &so)
{
	XSTRING line, annotation_esc;

	line = key + _T(" /");
	FORWARD_ITERATION_I(sc_itr, sc)
	{
		line += sc_itr->first;
		if(sc_itr->second.size() > 2)
		{
			annotation_esc = ParseConcat(sc_itr->second);
			line += _T(";") + MakeConcat(annotation_esc.substr(1, annotation_esc.size() - 2));
		}
		line += _T("/");
	}

	if(privatedic)
	{
		FORWARD_ITERATION_I(so_itr, so)
		{
			line += _T("[") + so_itr->first + _T("/");
			FORWARD_ITERATION_I(sc_itr, so_itr->second)
			{
				line += sc_itr->first + _T("/");
			}
			line += _T("]/");
		}

	}
	
	_ftprintf(fp, _T("%s\n"), line.c_str());
}

BOOL SaveSKKDic(LPCWSTR path)
{
	FILE *fp;
	SKKDICOKURIBLOCKS so;

	_wfopen_s(&fp, path, modeW);
	if(fp == nullptr)
	{
		fwprintf(stderr, L"cannot open file : %s\n", path);
		return FALSE;
	}

	//送りありエントリ
	_ftprintf(fp, _T("%s"), EntriesAri);

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
	_ftprintf(fp, _T("%s"), EntriesNasi);

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

int ReadSKKDicLine(FILE *fp, int &okuri, XSTRING &key, SKKDICCANDIDATES &c, SKKDICOKURIBLOCKS &o)
{
	TCHAR buf[BUFSIZE];
	XSTRING sbuf;

	c.clear();
	o.clear();

	while(_fgetts(buf, _countof(buf), fp) != nullptr)
	{
		sbuf += buf;

		if(!sbuf.empty() && sbuf.back() == _T('\n'))
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

	XSTRING s = sbuf;

	static const XSTRING  fmt(_T(""));

	static const XREGEX rectrl(_T("[\\x00-\\x19]"));
	s = std::regex_replace(s, rectrl, fmt);

	if(okuri == 1)
	{
		if(privatedic)
		{
			ParseSKKDicOkuriBlock(s, o);
		}

		//送りブロックを除去
		static const XREGEX reblock(_T("\\[[^\\[\\]]+?/[^\\[\\]]+?/\\]/"));
		s = std::regex_replace(s, reblock, fmt);
	}

	size_t is = s.find(_T("\x20/"));
	if(is == std::wstring::npos)
	{
		return 1;
	}

	size_t ie = s.find_last_not_of(_T('\x20'), is);
	if(ie == std::wstring::npos)
	{
		return 1;
	}

	if(s.find_last_of(_T('\x20'), ie) != std::wstring::npos)
	{
		return 1;
	}

	key = s.substr(0, ie + 1);

	s = s.substr(is + 1);

	ParseSKKDicCandiate(s, c);

	return 0;
}

void ParseSKKDicCandiate(const XSTRING &s, SKKDICCANDIDATES &c)
{
	size_t i, is, ie, ia;
	XSTRING candidate, annotation;

	i = 0;
	while(i < s.size())
	{
		is = s.find_first_of(_T('/'), i);
		ie = s.find_first_of(_T('/'), is + 1);
		if(ie == XSTRING::npos)
		{
			break;
		}

		candidate = s.substr(i + 1, ie - is - 1);
		i = ie;

		ia = candidate.find_first_of(_T(';'));

		if(ia == XSTRING::npos)
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

void ParseSKKDicOkuriBlock(const XSTRING &s, SKKDICOKURIBLOCKS &o)
{
	XSTRING so, okurik, okuric, fmt;
	XSMATCH m;
	SKKDICCANDIDATES okurics;

	so = s;

	static const XREGEX reblock(_T("\\[([^\\[\\]]+?)(/[^\\[\\]]+?/)\\]/"));

	while(std::regex_search(so, m, reblock))
	{
		okurics.clear();

		fmt.assign(_T("$1"));
		okurik = std::regex_replace(m.str(), reblock, fmt);
		fmt.assign(_T("$2"));
		okuric = std::regex_replace(m.str(), reblock, fmt);

		ParseSKKDicCandiate(okuric, okurics);

		std::reverse(okurics.begin(), okurics.end());

		o.insert(o.begin(), std::make_pair(okurik, okurics));

		so = m.suffix().str();
	}
}

XSTRING ParseConcat(const XSTRING &s)
{
	XSTRING ret, fmt, numstr, numtmpstr;
	XREGEX re;
	XSMATCH res;
	ULONG u;
#ifdef _UNICODE
	LPCTSTR bsrep = _T("\uf05c");
#else
	LPCTSTR bsrep = _T("\xff");
#endif
	ret = s;

	static const XREGEX reconcat(_T("^\\(\\s*concat\\s+\"(.+)\"\\s*\\)$"));

	if(std::regex_search(ret, reconcat))
	{
		fmt.assign(_T("$1"));
		ret = std::regex_replace(ret, reconcat, fmt);

		re.assign(_T("\"\\s+\""));
		fmt.assign(_T(""));
		ret = std::regex_replace(ret, re, fmt);

		//バックスラッシュ
		re.assign(_T("\\\\\\\\"));
		fmt.assign(bsrep);
		ret = std::regex_replace(ret, re, fmt);

		//二重引用符
		re.assign(_T("\\\\\\\""));
		fmt.assign(_T("\\\""));
		ret = std::regex_replace(ret, re, fmt);

		//空白文字
		re.assign(_T("\\\\s"));
		fmt.assign(_T("\x20"));
		ret = std::regex_replace(ret, re, fmt);

		//制御文字など
		re.assign(_T("\\\\[abtnvfred ]"));
		fmt.assign(_T(""));
		ret = std::regex_replace(ret, re, fmt);

		//8進数表記の文字
		re.assign(_T("\\\\[0-3][0-7]{2}"));
		while(std::regex_search(ret, res, re))
		{
			numstr += res.prefix();
			numtmpstr = res.str();
			numtmpstr[0] = L'0';
			u = _tcstoul(numtmpstr.c_str(), nullptr, 0);
			if(u >= _T('\x20') && u <= _T('\x7E'))
			{
				numstr.append(1, (TCHAR)u);
			}
			ret = res.suffix().str();
		}
		ret = numstr + ret;

		//意味なしエスケープ
		re.assign(_T("\\\\"));
		fmt.assign(_T(""));
		ret = std::regex_replace(ret, re, fmt);

		//バックスラッシュ
		re.assign(bsrep);
		fmt.assign(_T("\\"));
		ret = std::regex_replace(ret, re, fmt);
	}

	return ret;
}

XSTRING MakeConcat(const XSTRING &s)
{
	XSTRING ret, fmt;
	XREGEX re;

	ret = s;

	// "/" -> \057, ";" -> \073
	static const XREGEX respcch(_T("[/;]"));

	if(std::regex_search(ret, respcch))
	{
		// "\"" -> "\\\"", "\\" -> "\\\\"
		re.assign(_T("([\\\"\\\\])"));
		fmt.assign(_T("\\$1"));
		ret = std::regex_replace(ret, re, fmt);

		re.assign(_T("/"));
		fmt.assign(_T("\\057"));
		ret = std::regex_replace(ret, re, fmt);

		re.assign(_T(";"));
		fmt.assign(_T("\\073"));
		ret = std::regex_replace(ret, re, fmt);

		ret = _T("(concat \"") + ret + _T("\")");
	}

	return ret;
}
