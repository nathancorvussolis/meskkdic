
#include "stdafx.h"

#define VERSION		L"1.1.0"

#ifdef _UNICODE
#define BUFSIZE 0x8000
LPCWSTR modeR = L"r,ccs=UNICODE";
LPCWSTR modeW = L"w,ccs=UNICODE";
LPCWSTR EntriesAri = L";; okuri-ari entries.\n";
LPCWSTR EntriesNasi = L";; okuri-nasi entries.\n";
#define XSTRING std::wstring
#define XREGEX std::wregex
#define XSMATCH std::wsmatch
#else
#define BUFSIZE 0x10000
LPCWSTR modeR = L"rb";
LPCWSTR modeW = L"wb";
LPCSTR EntriesAri = ";; okuri-ari entries.\n";
LPCSTR EntriesNasi = ";; okuri-nasi entries.\n";
#define XSTRING std::string
#define XREGEX std::regex
#define XSMATCH std::smatch
#endif

CONST WCHAR plus = L'+';
CONST WCHAR minus = L'-';

//候補   pair< candidate, annotation >
typedef std::pair< XSTRING, XSTRING > SKKDICCANDIDATE;
typedef std::vector< SKKDICCANDIDATE > SKKDICCANDIDATES;
//辞書   pair< key, candidates >
typedef std::pair< XSTRING, SKKDICCANDIDATES > SKKDICENTRY;
typedef std::map< XSTRING, SKKDICCANDIDATES > SKKDIC;

BOOL LoadSKKDic(CONST WCHAR op, LPCWSTR path, SKKDIC &entries_a, SKKDIC &entries_n);
void AddDic(SKKDIC &skkdic, const XSTRING &key, const XSTRING &candidate, const XSTRING &annotation);
void DelDic(SKKDIC &skkdic, const XSTRING &key, const XSTRING &candidate, const XSTRING &annotation);
BOOL SaveSKKDic(LPCWSTR path, const SKKDIC &entries_a, const SKKDIC &entries_n);
void WriteSKKDicEntry(FILE *fp, const XSTRING &key, const SKKDICCANDIDATES &candidates);
XSTRING ParseConcat(const XSTRING &s);
XSTRING MakeConcat(const XSTRING &s);

int wmain(int argc, wchar_t* argv[])
{
	SKKDIC entries_a, entries_n;
	WCHAR op;
	int i;

	_tsetlocale(LC_ALL, _T("JPN"));

	if(argc < 3)
	{
		fwprintf(stderr, L"\nmeskkdic (%s)\n\n", VERSION);
		fwprintf(stderr, L"usage : meskkdic <input file 1> [[+-] <input file 2> ...] <output file>\n");
		return -1;
	}

	for(i=1; i<argc-1; i+=2)
	{
		if(i==1)
		{
			op = plus;
		}
		else
		{
			if(wcslen(argv[i-1]) != 1 && argv[i-1][0] != plus && argv[i-1][0] != minus)
			{
				fwprintf(stderr, L"found irregal string : %s\n");
				return -1;
			}
			op = argv[i-1][0];
		}
		
		if(!LoadSKKDic(op, argv[i], entries_a, entries_n))
		{
			return -1;
		}
	}

	if(!SaveSKKDic(argv[argc-1], entries_a, entries_n))
	{
		return -1;
	}

	return 0;
}

BOOL LoadSKKDic(const WCHAR op, LPCWSTR path, SKKDIC &entries_a, SKKDIC &entries_n)
{
	size_t i, is, ie;
	FILE *fp;
	TCHAR buf[BUFSIZE];
	XSTRING s;
	XREGEX re;
	XSTRING fmt;
	std::vector<XSTRING> es;
	int okuri = -1;
	XSTRING key;
	XSTRING candidate;
	XSTRING c, a;

	_wfopen_s(&fp, path, modeR);
	if(fp == NULL)
	{
		fwprintf(stderr, L"cannot open : %s\n", path);
		return FALSE;
	}

	while(true)
	{
		if(_fgetts(buf, _countof(buf), fp) == NULL)
		{
			break;
		}

		if(_tcscmp(EntriesAri, buf) == 0)
		{
			okuri = 1;
			continue;
		}
		else if(_tcscmp(EntriesNasi, buf) == 0)
		{
			okuri = 0;
			continue;
		}
		else if(okuri == -1)
		{
			continue;
		}

		s.assign(buf);
		re.assign(_T("[\\x00-\\x19]"));
		fmt.assign(_T(""));
		s = std::regex_replace(s, re, fmt);
		//送りありエントリのブロック形式を除去
		re.assign(_T("\\[.+?/.+?/\\]/"));
		fmt.assign(_T(""));
		s = std::regex_replace(s, re, fmt);

		is = s.find_first_of(_T('\x20'));
		if(is == XSTRING::npos)
		{
			continue;
		}
		key = s.substr(0, is);

		is = s.find_first_of(_T('/'), is);
		if(is == XSTRING::npos)
		{
			continue;
		}
		candidate = s.substr(is);

		//エントリを「/」で分割
		es.clear();
		i = 0;
		s = candidate;
		while(i < s.size())
		{
			is = s.find_first_of(_T('/'), i);
			ie = s.find_first_of(_T('/'), is + 1);
			if(ie == XSTRING::npos)
			{
				break;
			}
			es.push_back(s.substr(i + 1, ie - is - 1));
			i = ie;
		}

		//候補と注釈を分割
		for(i=0; i<es.size(); i++)
		{
			s = es[i];
			ie = s.find_first_of(_T(';'));

			if(ie == XSTRING::npos)
			{
				c = s;
				a = _T("");
			}
			else
			{
				c = s.substr(0, ie);
				a = s.substr(ie + 1);
			}

			switch(op)
			{
			case plus:
				AddDic((okuri == 0 ? entries_n : entries_a), key, c, a);
				break;
			case minus:
				DelDic((okuri == 0 ? entries_n : entries_a), key, c, a);
				break;
			default:
				break;
			}
		}
	}

	fclose(fp);

	return TRUE;
}

void AddDic(SKKDIC &skkdic, const XSTRING &key, const XSTRING &candidate, const XSTRING &annotation)
{
	SKKDIC::iterator skkdic_itr;
	SKKDICENTRY skkdicentry;
	SKKDICCANDIDATES::iterator sc_itr;
	LPCTSTR seps = _T(",");
	XSTRING annotation_seps;
	XSTRING annotation_esc;

	if(!annotation.empty())
	{
		annotation_seps = seps + ParseConcat(annotation) + seps;
	}

	skkdic_itr = skkdic.find(key);
	if(skkdic_itr == skkdic.end())
	{
		skkdicentry.first = key;
		skkdicentry.second.push_back(SKKDICCANDIDATE(candidate, annotation_seps));
		skkdic.insert(skkdicentry);
	}
	else
	{
		for(sc_itr = skkdic_itr->second.begin(); sc_itr != skkdic_itr->second.end(); sc_itr++)
		{
			if(sc_itr->first == candidate)
			{
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
		if(sc_itr == skkdic_itr->second.end())
		{
			skkdic_itr->second.push_back(SKKDICCANDIDATE(candidate, MakeConcat(annotation_seps)));
		}
	}
}

void DelDic(SKKDIC &skkdic, const XSTRING &key, const XSTRING &candidate, const XSTRING &annotation)
{
	SKKDIC::iterator skkdic_itr;
	SKKDICCANDIDATES::iterator sc_itr;

	skkdic_itr = skkdic.find(key);
	if(skkdic_itr != skkdic.end())
	{
		for(sc_itr = skkdic_itr->second.begin(); sc_itr != skkdic_itr->second.end(); sc_itr++)
		{
			if(sc_itr->first == candidate)
			{
				skkdic_itr->second.erase(sc_itr);
				if(skkdic_itr->second.empty())
				{
					skkdic.erase(skkdic_itr);
				}
				break;
			}
		}
	}
}

BOOL SaveSKKDic(LPCWSTR path, const SKKDIC &entries_a, const SKKDIC &entries_n)
{
	FILE *fp;
	SKKDIC::const_iterator entries_itr;
	SKKDIC::const_reverse_iterator entries_ritr;
	XSTRING line;

	_wfopen_s(&fp, path, modeW);
	if(fp == NULL)
	{
		fwprintf(stderr, L"cannot open : %s\n", path);
		return FALSE;
	}

	_ftprintf(fp, _T("%s"), EntriesAri);

	for(entries_ritr = entries_a.rbegin(); entries_ritr != entries_a.rend(); entries_ritr++)
	{
		WriteSKKDicEntry(fp, entries_ritr->first, entries_ritr->second);
	}

	_ftprintf(fp, _T("%s"), EntriesNasi);

	for(entries_itr = entries_n.begin(); entries_itr != entries_n.end(); entries_itr++)
	{
		WriteSKKDicEntry(fp, entries_itr->first, entries_itr->second);
	}

	fclose(fp);

	return TRUE;
}

void WriteSKKDicEntry(FILE *fp, const XSTRING &key, const SKKDICCANDIDATES &candidates)
{
	SKKDICCANDIDATES::const_iterator sc_itr;
	XSTRING line;
	XSTRING annotation_esc;

	line = key + _T(" /");
	for(sc_itr = candidates.begin(); sc_itr != candidates.end(); sc_itr++)
	{
		line += sc_itr->first;
		if(sc_itr->second.size() > 2)
		{
			annotation_esc = ParseConcat(sc_itr->second);
			line += _T(";") + MakeConcat(annotation_esc.substr(1, annotation_esc.size() - 2));
		}
		line += _T("/");
	}

	_ftprintf(fp, _T("%s\n"), line.c_str());
}

XSTRING ParseConcat(const XSTRING &s)
{
	XSTRING ret = s;
	XREGEX re;
	XSMATCH res;
	XSTRING numstr, tmpstr, fmt;
	TCHAR u;

	tmpstr = s;
	re.assign(_T("^\\(concat \".+?\"\\)$"));
	if(std::regex_search(tmpstr, re))
	{
		ret.clear();
		fmt = _T("$1");

		re.assign(_T("\\(concat \"(.+)\"\\)"));
		tmpstr = std::regex_replace(tmpstr, re, fmt);

		re.assign(_T("\\\\([\\\"|\\\\])"));
		tmpstr = std::regex_replace(tmpstr, re, fmt);

		re.assign(_T("\\\\[0-3][0-7]{2}"));
		while(std::regex_search(tmpstr, res, re))
		{
			ret += res.prefix();
			numstr = res.str();
			numstr[0] = _T('0');
			u = (TCHAR)_tcstoul(numstr.c_str(), NULL, 0);
			if(u >= _T('\x20') && u <= _T('\x7E'))
			{
				ret.append(1, u);
			}
			tmpstr = res.suffix();
		}
		ret += tmpstr;
	}

	return ret;
}

XSTRING MakeConcat(const XSTRING &s)
{
	XSTRING ret = s;
	XREGEX re;
	XSTRING fmt;

	// "/" -> \057, ";" -> \073
	re.assign(_T("[/;]"));
	if(std::regex_search(ret, re))
	{
		// "\"" -> "\\\"", "\\" -> "\\\\"
		re.assign(_T("([\\\"|\\\\])"));
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
