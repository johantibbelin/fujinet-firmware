/*
 * EdUrlParser.h
 *
 *  Created on: Nov 25, 2014
 *      Author: netmind
 */

#ifndef EDURLPARSER_H_
#define EDURLPARSER_H_

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

using namespace std;

typedef struct {
	string key;
	string val;
} query_kv_t;

typedef int (*__kv_callback)(void* list, string k, string v);

class EdUrlParser {
private:
	EdUrlParser();
public:
	virtual ~EdUrlParser();
	static EdUrlParser* parseUrl(const string &urlstr);
	static int parsePath(vector<string> *pdirlist, string pathstr);
	static string urlDecode(const string &str);
	static bool toChar(const char* hex, char *result);
	static string urlEncode(const string &s);
	static void toHex(char *desthex, char c);
	static size_t parseKeyValueMap(unordered_map<string, string> *kvmap, const string &str, bool strict=true);
	static size_t parseKeyValueList(vector< query_kv_t > *kvmap, const string &rawstr, bool strict=true);
	static size_t parseKeyValue(const string &rawstr, __kv_callback kvcb, void* obj, bool strict);
	bool isValidUrl();

private:
	void parse();

public:
	string mRawUrl;
	string scheme;
	string hostName;
	string port;
	string path;
	string query;
	string fragment;
	string toString() { return scheme + "://" + hostName + (port.empty() ? "" : (":" + port)) + path + (query.empty() ? "" : "?" + query) + (fragment.empty() ? "" : "#" + fragment); }
};

#endif /* EDURLPARSER_H_ */