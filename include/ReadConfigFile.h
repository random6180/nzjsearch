#ifndef __READCONFIGFILE_H__
#define __READCONFIGFILE_H__

#include<string>
#include<map>
using std::string;
using std::map;

namespace nzj
{

#define CONFIGFILEPATH "/home/nzj/wkspace2/search/conf/my.conf"

class ReadConfigFile
{
public:
	static ReadConfigFile * getInstance();
	const string & find(const string &key);
	int printMap();//for debug
private:
	ReadConfigFile(const string &filename);
	static ReadConfigFile *_pInstance;
	class Recycle
	{
	public:
		~Recycle()
		{
			if(_pInstance)delete _pInstance;
		}
	};
	static Recycle rec;
	map<string, string> _content;
};
}
#endif
